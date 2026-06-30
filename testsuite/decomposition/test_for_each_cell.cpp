/*
 *  test_for_each_cell.cpp
 *
 *  Created on: 30 Jun 2026
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 *
 *  Tests for GridContext::forEachCell. The host cases exercise the C-order host
 *  iteration path. The Kokkos cases register a Kokkos-backed grid and drive the
 *  same per-cell kernel through the execution-space-aware path. On a host-only
 *  Kokkos build the kernel runs on the Serial backend; the identical code path
 *  dispatches to the device once Kokkos is configured with a GPU backend, which
 *  is why the kernel is written as a SCHNEK_INLINE functor and the results are
 *  read back through a host mirror of the view.
 */

#include "../utility.hpp"

#include <grid/grid.hpp>
#include <grid/range.hpp>
#include <macros.hpp>

#include <decomposition/serial_decomposition.hpp>

#include <boost/test/unit_test.hpp>

#include <cstddef>

namespace {

  // Per-cell kernel that writes a position-dependent value into the grid. The
  // operator() is const and SCHNEK_INLINE so the same functor is callable on the
  // host and (on a CUDA build) on the device.
  template<typename GridT>
  struct AssignKernel {
      SCHNEK_INLINE void operator()(const typename GridT::IndexType &pos, GridT &grid) const {
        double value = 0.0;
        double factor = 1.0;
        for (size_t d = 0; d < static_cast<size_t>(GridT::Rank); ++d) {
          value += factor * static_cast<double>(pos[d]);
          factor *= 100.0;
        }
        grid[pos] = value;
      }
  };

  // Kernel adding the second grid into the first, cell by cell.
  template<typename GridT>
  struct AddKernel {
      SCHNEK_INLINE void operator()(const typename GridT::IndexType &pos, GridT &dst, GridT &src) const {
        dst[pos] = dst[pos] + src[pos];
      }
  };

  template<size_t rank>
  double expectedValue(const schnek::Array<ptrdiff_t, rank> &pos) {
    double value = 0.0;
    double factor = 1.0;
    for (size_t d = 0; d < rank; ++d) {
      value += factor * static_cast<double>(pos[d]);
      factor *= 100.0;
    }
    return value;
  }

}  // namespace

BOOST_AUTO_TEST_SUITE(domain_decomposition)
BOOST_AUTO_TEST_SUITE(for_each_cell)

BOOST_AUTO_TEST_CASE(for_each_cell_assigns_every_cell_1d) {
  using GridType = schnek::Grid<double, 1>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;

  RangeType globalRange(schnek::Array<ptrdiff_t, 1>(0), schnek::Array<ptrdiff_t, 1>(7));
  schnek::Range<double, 1> globalDomain(schnek::Array<double, 1>(0), schnek::Array<double, 1>(1));

  schnek::SerialDomainDecomposition<1> decomposition;
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);
  decomposition.init();

  schnek::GridFactory<GridType> factory;
  schnek::GridRegistration registration = decomposition.registerField(factory);

  auto gridContext = decomposition.getGridContext({registration});

  GridType *grid = nullptr;
  gridContext.forEach([&](const RangeType &, GridType &localGrid) { grid = &localGrid; });
  BOOST_REQUIRE(grid != nullptr);
  *grid = -1.0;

  gridContext.forEachCell(AssignKernel<GridType>{});

  RangeType range(grid->getLo(), grid->getHi());
  for (auto it = range.begin(); it != range.end(); ++it) {
    BOOST_CHECK_CLOSE((*grid)[*it], expectedValue<1>(*it), 1e-12);
  }
}

BOOST_AUTO_TEST_CASE(for_each_cell_assigns_every_cell_2d) {
  using GridType = schnek::Grid<double, 2>;
  using RangeType = schnek::Range<ptrdiff_t, 2>;

  RangeType globalRange(schnek::Array<ptrdiff_t, 2>(0, 0), schnek::Array<ptrdiff_t, 2>(4, 3));
  schnek::Range<double, 2> globalDomain(schnek::Array<double, 2>(0, 0), schnek::Array<double, 2>(1, 1));

  schnek::SerialDomainDecomposition<2> decomposition;
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);
  decomposition.init();

  schnek::GridFactory<GridType> factory;
  schnek::GridRegistration registration = decomposition.registerField(factory);

  auto gridContext = decomposition.getGridContext({registration});

  GridType *grid = nullptr;
  gridContext.forEach([&](const RangeType &, GridType &localGrid) { grid = &localGrid; });
  BOOST_REQUIRE(grid != nullptr);
  *grid = -1.0;

  gridContext.forEachCell(AssignKernel<GridType>{});

  RangeType range(grid->getLo(), grid->getHi());
  for (auto it = range.begin(); it != range.end(); ++it) {
    BOOST_CHECK_CLOSE((*grid)[*it], expectedValue<2>(*it), 1e-12);
  }
}

BOOST_AUTO_TEST_CASE(for_each_cell_multiple_grids_1d) {
  using GridType = schnek::Grid<double, 1>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;

  RangeType globalRange(schnek::Array<ptrdiff_t, 1>(0), schnek::Array<ptrdiff_t, 1>(5));
  schnek::Range<double, 1> globalDomain(schnek::Array<double, 1>(0), schnek::Array<double, 1>(1));

  schnek::SerialDomainDecomposition<1> decomposition;
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);
  decomposition.init();

  schnek::GridFactory<GridType> factory;
  schnek::GridRegistration regDst = decomposition.registerField(factory);
  schnek::GridRegistration regSrc = decomposition.registerField(factory);

  auto dstContext = decomposition.getGridContext({regDst});
  auto srcContext = decomposition.getGridContext({regSrc});

  GridType *dst = nullptr;
  GridType *src = nullptr;
  dstContext.forEach([&](const RangeType &, GridType &g) { dst = &g; });
  srcContext.forEach([&](const RangeType &, GridType &g) { src = &g; });
  BOOST_REQUIRE(dst != nullptr);
  BOOST_REQUIRE(src != nullptr);

  *dst = 10.0;
  *src = 3.0;

  auto bothContext = decomposition.getGridContext({regDst, regSrc});
  bothContext.forEachCell(AddKernel<GridType>{});

  RangeType range(dst->getLo(), dst->getHi());
  for (auto it = range.begin(); it != range.end(); ++it) {
    BOOST_CHECK_CLOSE((*dst)[*it], 13.0, 1e-12);
  }
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

#ifdef SCHNEK_HAVE_KOKKOS

#include <grid/gridstorage/kokkos-storage.hpp>

#include <Kokkos_Core.hpp>

namespace {

  template<typename T, size_t rank>
  using KokkosHostStorage = schnek::KokkosGridStorage<T, rank>;

}  // namespace

BOOST_AUTO_TEST_SUITE(domain_decomposition)
BOOST_AUTO_TEST_SUITE(for_each_cell_kokkos)

BOOST_AUTO_TEST_CASE(for_each_cell_assigns_every_cell_kokkos_1d) {
  using GridType = schnek::Grid<double, 1, KokkosHostStorage>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;

  RangeType globalRange(schnek::Array<ptrdiff_t, 1>(0), schnek::Array<ptrdiff_t, 1>(7));
  schnek::Range<double, 1> globalDomain(schnek::Array<double, 1>(0), schnek::Array<double, 1>(1));

  schnek::SerialDomainDecomposition<1> decomposition;
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);
  decomposition.init();

  schnek::GridFactory<GridType> factory;
  schnek::GridRegistration registration = decomposition.registerField(factory);

  auto gridContext = decomposition.getGridContext({registration});

  GridType *grid = nullptr;
  gridContext.forEach([&](const RangeType &, GridType &localGrid) { grid = &localGrid; });
  BOOST_REQUIRE(grid != nullptr);

  gridContext.forEachCell(AssignKernel<GridType>{});
  Kokkos::fence();

  // Read the result back through a host mirror so the check is correct for both
  // host-accessible and device-resident backends.
  auto view = grid->getKokkosView();
  auto hostView = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(), view);

  ptrdiff_t lo = grid->getLo()[0];
  ptrdiff_t hi = grid->getHi()[0];
  for (ptrdiff_t i = lo; i <= hi; ++i) {
    schnek::Array<ptrdiff_t, 1> pos(i);
    BOOST_CHECK_CLOSE(hostView(i - lo), expectedValue<1>(pos), 1e-12);
  }
}

BOOST_AUTO_TEST_CASE(for_each_cell_assigns_every_cell_kokkos_2d) {
  using GridType = schnek::Grid<double, 2, KokkosHostStorage>;
  using RangeType = schnek::Range<ptrdiff_t, 2>;

  RangeType globalRange(schnek::Array<ptrdiff_t, 2>(0, 0), schnek::Array<ptrdiff_t, 2>(4, 3));
  schnek::Range<double, 2> globalDomain(schnek::Array<double, 2>(0, 0), schnek::Array<double, 2>(1, 1));

  schnek::SerialDomainDecomposition<2> decomposition;
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);
  decomposition.init();

  schnek::GridFactory<GridType> factory;
  schnek::GridRegistration registration = decomposition.registerField(factory);

  auto gridContext = decomposition.getGridContext({registration});

  GridType *grid = nullptr;
  gridContext.forEach([&](const RangeType &, GridType &localGrid) { grid = &localGrid; });
  BOOST_REQUIRE(grid != nullptr);

  gridContext.forEachCell(AssignKernel<GridType>{});
  Kokkos::fence();

  auto view = grid->getKokkosView();
  auto hostView = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(), view);

  ptrdiff_t lo0 = grid->getLo()[0];
  ptrdiff_t hi0 = grid->getHi()[0];
  ptrdiff_t lo1 = grid->getLo()[1];
  ptrdiff_t hi1 = grid->getHi()[1];
  for (ptrdiff_t i = lo0; i <= hi0; ++i) {
    for (ptrdiff_t j = lo1; j <= hi1; ++j) {
      schnek::Array<ptrdiff_t, 2> pos(i, j);
      BOOST_CHECK_CLOSE(hostView(i - lo0, j - lo1), expectedValue<2>(pos), 1e-12);
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

#endif  // SCHNEK_HAVE_KOKKOS
