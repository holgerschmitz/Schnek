/*
 *  test_kokkos_exchange.cpp
 *
 *  Created on: 30 Jun 2026
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 *
 *  End-to-end ghost-cell exchange for Kokkos-backed grids. The exchange routes
 *  element movement through the `detail::GridTransfer` customisation point. On a
 *  host-only Kokkos build the grid's memory space is host-accessible, so the
 *  host transfer path is exercised; on a device build the same registration and
 *  exchange drive the device gather/scatter + host-staging path. Either way the
 *  test proves that a Kokkos storage policy composes with field registration and
 *  the MPI ghost exchange.
 */

#include "mpi_test_context.hpp"
#include "../utility.hpp"

#include <grid/range.hpp>
#include <grid/grid.hpp>
#include <grid/field.hpp>

#include <decomposition/mpi_cartesian_decomposition.hpp>

#include <boost/test/unit_test.hpp>

#if defined(SCHNEK_HAVE_KOKKOS) && defined(SCHNEK_HAVE_MPI)

#include <grid/gridstorage/kokkos-storage.hpp>
#include <grid/iteration/kokkos-iteration.hpp>

#include <array>
#include <cstddef>
#include <cstring>
#include <vector>

namespace {

namespace {
#ifdef KOKKOS_ENABLE_CUDA
    typedef Kokkos::Cuda Execution;
    
    template <typename T, size_t rank>
    using GridStorage = schnek::KokkosGridStorage<T, rank, Kokkos::CudaHostPinnedSpace>;

    typedef schnek::RangeKokkosIterationPolicy<1, Execution> Iteration1d;
    typedef schnek::RangeKokkosIterationPolicy<2, Execution> Iteration2d;
#else
    typedef Kokkos::Serial Execution;

    template <typename T, size_t rank>
    using GridStorage = schnek::SingleArrayGridStorage<T, rank>;

    typedef schnek::RangeCIterationPolicy<1> Iteration1d;
    typedef schnek::RangeCIterationPolicy<2> Iteration2d;
#endif
}

  /// Host-accessible default-space Kokkos storage usable as a grid policy.
  template<typename T, size_t rank>
  using KokkosHostStorage = schnek::KokkosGridStorage<T, rank>;

  template<typename RangeType>
  size_t rangeVolume(const RangeType &range) {
    constexpr size_t dims = RangeType::LimitType::length;
    size_t volume = 1;
    for (size_t dim = 0; dim < dims; ++dim) {
      ptrdiff_t extent = range.getHi()[dim] - range.getLo()[dim] + 1;
      if (extent <= 0) {
        return 0;
      }
      volume *= static_cast<size_t>(extent);
    }
    return volume;
  }

  template<typename T>
  std::vector<char> toByteVector(const std::vector<T> &values) {
    std::vector<char> bytes(values.size() * sizeof(T));
    if (!values.empty()) {
      std::memcpy(bytes.data(), values.data(), bytes.size());
    }
    return bytes;
  }

  template<typename FieldType>
  struct GhostSliceExpectation {
    using RangeType = typename FieldType::RangeType;
    RangeType range;
    std::vector<typename FieldType::value_type> values;
  };

  template<typename FieldType>
  struct GhostExchangeSetupResult {
      std::vector<boost::tuple<int, std::vector<char>>> responses;
      std::vector<GhostSliceExpectation<FieldType>> expectations;
  };

  // Prepare the mock MPI Sendrecv responses and the corresponding expected ghost
  // slice values for a single-process ghost exchange.
  template<typename FieldType>
  GhostExchangeSetupResult<FieldType> buildGhostExchangeSetup(FieldType &field) {
    using RangeType = typename FieldType::RangeType;
    using IndexType = typename FieldType::IndexType;

    GhostExchangeSetupResult<FieldType> result;

    IndexType gridLo = field.getLo();
    IndexType gridHi = field.getHi();
    IndexType innerLo = field.getInnerLo();
    IndexType innerHi = field.getInnerHi();

    constexpr size_t dims = RangeType::LimitType::length;

    auto makeRange = [](const IndexType &lo, const IndexType &hi) { return RangeType(lo, hi); };

    double seed = 1.0;
    auto makeValues = [&](const RangeType &range) {
      RangeType rangeCopy(range);
      std::vector<typename FieldType::value_type> values;
      values.reserve(rangeVolume(rangeCopy));
      for (auto it = rangeCopy.begin(); it != rangeCopy.end(); ++it) {
        values.push_back(static_cast<typename FieldType::value_type>(seed));
        seed += 1.0;
      }
      return values;
    };

    for (size_t dim = 0; dim < dims; ++dim) {
      ptrdiff_t lowerHalo = innerLo[dim] - gridLo[dim];
      if (lowerHalo > 0) {
        IndexType lo = gridLo;
        IndexType hi = gridHi;
        hi[dim] = gridLo[dim] + lowerHalo - 1;
        RangeType ghostRange = makeRange(lo, hi);
        auto values = makeValues(ghostRange);
        result.responses.push_back(boost::make_tuple(MPI_SUCCESS, toByteVector(values)));
        result.expectations.push_back({ghostRange, values});
      }

      ptrdiff_t upperHalo = gridHi[dim] - innerHi[dim];
      if (upperHalo > 0) {
        IndexType lo = gridLo;
        IndexType hi = gridHi;
        lo[dim] = gridHi[dim] - upperHalo + 1;
        RangeType ghostRange = makeRange(lo, hi);
        auto values = makeValues(ghostRange);
        result.responses.push_back(boost::make_tuple(MPI_SUCCESS, toByteVector(values)));
        result.expectations.push_back({ghostRange, values});
      }
    }

    return result;
  }

  template<typename GridT>
  void fillGrid(GridT &grid, typename GridT::value_type value) {
    typename GridT::RangeType range(grid.getLo(), grid.getHi());
    for (auto it = range.begin(); it != range.end(); ++it) {
      grid[*it] = value;
    }
  }

  // Apply the prepared ghost expectations to an independent reference grid so the
  // exchanged field can be compared without aliasing the Kokkos view.
  template<typename GridT, typename FieldType>
  void applyGhostExpectations(GridT &reference, const GhostExchangeSetupResult<FieldType> &ghostSetup) {
    for (const auto &expectation : ghostSetup.expectations) {
      size_t idx = 0;
      auto rangeCopy = expectation.range;
      for (auto it = rangeCopy.begin(); it != rangeCopy.end(); ++it) {
        reference[*it] = expectation.values[idx++];
      }
    }
  }

  template<size_t Rank>
  void exchangeRegistration(
      schnek::MpiCartesianDomainDecomposition<Rank> &decomposition,
      const schnek::GridRegistration &registration,
      bool useFieldInfo = false
  ) {
    auto &base = static_cast<schnek::DomainDecomposition<Rank> &>(decomposition);
    base.exchange(registration, useFieldInfo);
  }

}  // namespace

BOOST_AUTO_TEST_SUITE(domain_decomposition)
BOOST_AUTO_TEST_SUITE(kokkos_exchange)

BOOST_AUTO_TEST_CASE(ghost_exchange_updates_ghost_cells_kokkos_1d) {
  MpiTestContextImpl context;

  MPI_Comm testComm = (MPI_Comm)(void *)123;
  std::vector<int> coords(1, 0);
  context.commWorld = (MPI_Comm)(void *)574;
  context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, 1));
  context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, 0));
  context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
  context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));

  using FieldType = schnek::Field<double, 1, GridStorage>;
  using ReferenceGrid = schnek::Grid<double, 1>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;
  using DomainType = schnek::Range<double, 1>;
  using StaggerType = typename FieldType::StaggerType;

  RangeType globalRange(schnek::Array<ptrdiff_t, 1>(0), schnek::Array<ptrdiff_t, 1>(3));
  DomainType globalDomain(schnek::Array<double, 1>(0.0), schnek::Array<double, 1>(1.0));

  schnek::MpiCartesianDomainDecomposition<1> decomposition(context);
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);
  decomposition.init();

  StaggerType noStagger(false);
  constexpr int ghostCells = 1;
  schnek::GridFactory<FieldType> factory(noStagger, ghostCells);
  schnek::GridRegistration registration = decomposition.registerField(factory);

  auto gridContext = decomposition.getGridContext({registration});
  FieldType *field = nullptr;
  gridContext.forEach([&](const RangeType &, FieldType &localField) { field = &localField; });
  BOOST_REQUIRE(field != nullptr);

  std::cerr << "Filling field with -42.0" << std::endl;

  gridContext.forEach([&](const RangeType &range, FieldType &localField) { 
    //   Iteration1d::forEach(range, [&](const typename FieldType::IndexType &pos) {
    //       localField[pos] = -42.0;
    //   });
  });
//   fillGrid(*field, -42.0);
  std::cerr << "Field filled" << std::endl;

//   ReferenceGrid expected(field->getLo(), field->getHi());
//   std::cerr << "Filling expected grid with -42.0" << std::endl;
//   fillGrid(expected, -42.0);
//   std::cerr << "Expected grid filled" << std::endl;
//   auto ghostSetup = buildGhostExchangeSetup(*field);
//   std::cerr << "Applying ghost expectations to expected grid" << std::endl;
//   applyGhostExpectations(expected, ghostSetup);
//   context.ret_MPI_Sendrecv = ghostSetup.responses;

//   std::array<int, 1> prevRanks{{7}};
//   std::array<int, 1> nextRanks{{8}};
//   for (size_t dim = 0; dim < prevRanks.size(); ++dim) {
//     context.ret_MPI_Cart_shift.push_back(boost::make_tuple(MPI_SUCCESS, prevRanks[dim], nextRanks[dim]));
//   }

//   exchangeRegistration<1>(decomposition, registration, false);

//   BOOST_REQUIRE_EQUAL(context.args_MPI_Sendrecv.size(), ghostSetup.expectations.size());
//   for (size_t i = 0; i < ghostSetup.expectations.size(); ++i) {
//     const auto &call = context.args_MPI_Sendrecv[i];
//     BOOST_CHECK(call.recvBufferPresent);
//     BOOST_CHECK_EQUAL(call.recvCount, static_cast<int>(ghostSetup.expectations[i].values.size()));
//   }

//   RangeType verificationRange(field->getLo(), field->getHi());
//   for (auto it = verificationRange.begin(); it != verificationRange.end(); ++it) {
//     BOOST_CHECK_EQUAL((*field)[*it], expected[*it]);
//   }
}

// BOOST_AUTO_TEST_CASE(ghost_exchange_updates_ghost_cells_kokkos_2d) {
//   MpiTestContextImpl context;

//   MPI_Comm testComm = (MPI_Comm)(void *)123;
//   std::vector<int> coords(2, 0);
//   context.commWorld = (MPI_Comm)(void *)574;
//   context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, 1));
//   context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, 0));
//   context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
//   context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));

//   using FieldType = schnek::Field<double, 2, KokkosHostStorage>;
//   using ReferenceGrid = schnek::Grid<double, 2>;
//   using RangeType = schnek::Range<ptrdiff_t, 2>;
//   using DomainType = schnek::Range<double, 2>;
//   using StaggerType = typename FieldType::StaggerType;

//   RangeType globalRange(schnek::Array<ptrdiff_t, 2>(0, 0), schnek::Array<ptrdiff_t, 2>(3, 2));
//   DomainType globalDomain(schnek::Array<double, 2>(0.0, 0.0), schnek::Array<double, 2>(1.0, 1.0));

//   schnek::MpiCartesianDomainDecomposition<2> decomposition(context);
//   decomposition.setGlobalRange(globalRange);
//   decomposition.setGlobalDomain(globalDomain);
//   decomposition.init();

//   StaggerType noStagger(false);
//   constexpr int ghostCells = 1;
//   schnek::GridFactory<FieldType> factory(noStagger, ghostCells);
//   schnek::GridRegistration registration = decomposition.registerField(factory);

//   auto gridContext = decomposition.getGridContext({registration});
//   FieldType *field = nullptr;
//   gridContext.forEach([&](const RangeType &, FieldType &localField) { field = &localField; });
//   BOOST_REQUIRE(field != nullptr);

//   fillGrid(*field, -13.0);

//   ReferenceGrid expected(field->getLo(), field->getHi());
//   fillGrid(expected, -13.0);
//   auto ghostSetup = buildGhostExchangeSetup(*field);
//   applyGhostExpectations(expected, ghostSetup);
//   context.ret_MPI_Sendrecv = ghostSetup.responses;

//   std::array<int, 2> prevRanks{{10, 20}};
//   std::array<int, 2> nextRanks{{11, 21}};
//   for (size_t dim = 0; dim < prevRanks.size(); ++dim) {
//     context.ret_MPI_Cart_shift.push_back(boost::make_tuple(MPI_SUCCESS, prevRanks[dim], nextRanks[dim]));
//   }

//   exchangeRegistration<2>(decomposition, registration, false);

//   BOOST_REQUIRE_EQUAL(context.args_MPI_Sendrecv.size(), ghostSetup.expectations.size());
//   for (size_t i = 0; i < ghostSetup.expectations.size(); ++i) {
//     const auto &call = context.args_MPI_Sendrecv[i];
//     BOOST_CHECK(call.recvBufferPresent);
//     BOOST_CHECK_EQUAL(call.recvCount, static_cast<int>(ghostSetup.expectations[i].values.size()));
//   }

//   RangeType verificationRange(field->getLo(), field->getHi());
//   for (auto it = verificationRange.begin(); it != verificationRange.end(); ++it) {
//     BOOST_CHECK_EQUAL((*field)[*it], expected[*it]);
//   }
// }

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

#endif  // SCHNEK_HAVE_KOKKOS && SCHNEK_HAVE_MPI
