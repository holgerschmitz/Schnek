/*
 *  test_for_each_cell.cpp
 *
 *  Created on: 30 Jun 2026
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 *
 *  Tests for GridContext::forEachCell.
 */

#include "../utility.hpp"

#include <grid/grid.hpp>
#include <grid/range.hpp>
#include <macros.hpp>

#include <decomposition/serial_decomposition.hpp>

#include <boost/test/unit_test.hpp>

#include <cstddef>

namespace {

  template<typename GridT>
  struct AssignKernel {
      void operator()(
          const typename GridT::IndexType &pos, typename GridT::template ExecutionViewType<void> &grid
      ) const {
        double value = 0.0;
        double factor = 1.0;
        for (size_t d = 0; d < static_cast<size_t>(GridT::Rank); ++d) {
          value += factor * static_cast<double>(pos[d]);
          factor *= 100.0;
        }
        grid[pos] = value;
      }
  };

#ifdef SCHNEK_HAVE_KOKKOS
  template<typename GridT>
  struct AssignKokkosKernel {
      SCHNEK_INLINE void operator()(
          const typename GridT::IndexType &pos, typename GridT::template ExecutionViewType<void> &grid
      ) const {
        double value = 0.0;
        double factor = 1.0;
        for (size_t d = 0; d < static_cast<size_t>(GridT::Rank); ++d) {
          value += factor * static_cast<double>(pos[d]);
          factor *= 100.0;
        }
        grid[pos] = value;
      }
  };
#endif

  template<typename GridT>
  struct AddKernel {
      void operator()(
          const typename GridT::IndexType &pos,
          typename GridT::template ExecutionViewType<void> &dst,
          typename GridT::template ExecutionViewType<void> &src
      ) const {
        dst[pos] = dst[pos] + src[pos];
      }
  };

  template<typename GridT>
  struct CountKernel {
      int *count;

      void operator()(
          const typename GridT::IndexType &, typename GridT::template ExecutionViewType<void> &
      ) const {
        ++*count;
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

  template<typename GridT>
  void checkAssignedGrid(const GridT &grid) {
    typename GridT::RangeType range(grid.getLo(), grid.getHi());
    for (auto it = range.begin(); it != range.end(); ++it) {
      BOOST_CHECK_CLOSE(grid[*it], expectedValue(*it), 1e-12);
    }
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
  const auto registration = decomposition.registerField(factory);
  auto context = decomposition.getGridContext({registration});

  GridType *grid = nullptr;
  context.forEach([&](const RangeType &, GridType &localGrid) { grid = &localGrid; });
  BOOST_REQUIRE(grid != nullptr);
  *grid = -1.0;

  context.forEachCell<GridType>(AssignKernel<GridType>{});
  checkAssignedGrid(*grid);
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
  const auto registration = decomposition.registerField(factory);
  auto context = decomposition.getGridContext({registration});

  GridType *grid = nullptr;
  context.forEach([&](const RangeType &, GridType &localGrid) { grid = &localGrid; });
  BOOST_REQUIRE(grid != nullptr);
  *grid = -1.0;

  context.forEachCell<GridType>(AssignKernel<GridType>{});
  checkAssignedGrid(*grid);
}

BOOST_AUTO_TEST_CASE(for_each_cell_adds_two_grids) {
  using GridType = schnek::Grid<double, 1>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;

  RangeType globalRange(schnek::Array<ptrdiff_t, 1>(0), schnek::Array<ptrdiff_t, 1>(5));
  schnek::Range<double, 1> globalDomain(schnek::Array<double, 1>(0), schnek::Array<double, 1>(1));

  schnek::SerialDomainDecomposition<1> decomposition;
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);
  decomposition.init();

  schnek::GridFactory<GridType> factory;
  const auto dstRegistration = decomposition.registerField(factory);
  const auto srcRegistration = decomposition.registerField(factory);
  auto dstContext = decomposition.getGridContext({dstRegistration});
  auto srcContext = decomposition.getGridContext({srcRegistration});

  GridType *dst = nullptr;
  GridType *src = nullptr;
  dstContext.forEach([&](const RangeType &, GridType &grid) { dst = &grid; });
  srcContext.forEach([&](const RangeType &, GridType &grid) { src = &grid; });
  BOOST_REQUIRE(dst != nullptr);
  BOOST_REQUIRE(src != nullptr);
  *dst = 10.0;
  *src = 3.0;

  auto bothContext = decomposition.getGridContext({dstRegistration, srcRegistration});
  bothContext.forEachCell<GridType, GridType>(AddKernel<GridType>{});

  RangeType range(dst->getLo(), dst->getHi());
  for (auto it = range.begin(); it != range.end(); ++it) {
    BOOST_CHECK_CLOSE((*dst)[*it], 13.0, 1e-12);
  }
}

BOOST_AUTO_TEST_CASE(for_each_cell_skips_non_overlapping_subrange) {
  using GridType = schnek::Grid<double, 1>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;

  RangeType globalRange(schnek::Array<ptrdiff_t, 1>(0), schnek::Array<ptrdiff_t, 1>(7));
  schnek::Range<double, 1> globalDomain(schnek::Array<double, 1>(0), schnek::Array<double, 1>(1));
  schnek::SerialDomainDecomposition<1> decomposition;
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);
  decomposition.init();

  schnek::GridFactory<GridType> factory;
  const auto registration = decomposition.registerField(factory, RangeType(schnek::Array<ptrdiff_t, 1>(20),
                                                                           schnek::Array<ptrdiff_t, 1>(25)));
  auto context = decomposition.getGridContext({registration});

  int calls = 0;
  context.forEachCell<GridType>(CountKernel<GridType>{&calls});
  BOOST_CHECK_EQUAL(calls, 0);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

#ifdef SCHNEK_HAVE_KOKKOS

#include <grid/gridstorage/kokkos-storage.hpp>

#include <Kokkos_Core.hpp>

namespace {

  template<typename T, size_t rank>
  using KokkosHostStorage = schnek::KokkosGridStorage<T, rank, Kokkos::HostSpace>;

}  // namespace

BOOST_AUTO_TEST_SUITE(domain_decomposition)
BOOST_AUTO_TEST_SUITE(for_each_cell_kokkos)

BOOST_AUTO_TEST_CASE(for_each_cell_assigns_every_cell_kokkos_host_space) {
  using GridType = schnek::Grid<double, 1, KokkosHostStorage>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;

  RangeType globalRange(schnek::Array<ptrdiff_t, 1>(0), schnek::Array<ptrdiff_t, 1>(7));
  schnek::Range<double, 1> globalDomain(schnek::Array<double, 1>(0), schnek::Array<double, 1>(1));

  schnek::SerialDomainDecomposition<1> decomposition;
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);
  decomposition.init();

  schnek::GridFactory<GridType> factory;
  const auto registration = decomposition.registerField(factory);
  auto context = decomposition.getGridContext({registration});

  GridType *grid = nullptr;
  context.forEach([&](const RangeType &, GridType &localGrid) { grid = &localGrid; });
  BOOST_REQUIRE(grid != nullptr);

  context.forEachCell<GridType>(AssignKokkosKernel<GridType>{});
  Kokkos::fence();

  auto hostView = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(), grid->getKokkosView());
  const ptrdiff_t lo = grid->getLo()[0];
  for (ptrdiff_t i = grid->getLo()[0]; i <= grid->getHi()[0]; ++i) {
    const schnek::Array<ptrdiff_t, 1> pos(i);
    BOOST_CHECK_CLOSE(hostView(i - lo), expectedValue<1>(pos), 1e-12);
  }
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

#endif  // SCHNEK_HAVE_KOKKOS
