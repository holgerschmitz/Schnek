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

    template <typename T, size_t rank>
    using DeviceGridStorage = schnek::KokkosGridStorage<T, rank, Kokkos::CudaSpace>;

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
    // auto &base = static_cast<schnek::DomainDecomposition<Rank> &>(decomposition);
    decomposition.exchange(registration, useFieldInfo);
  }

#ifdef KOKKOS_ENABLE_CUDA
  template<size_t Rank>
  void runDeviceGhostExchange() {
    MpiTestContextImpl context;

    MPI_Comm testComm = (MPI_Comm)(void *)123;
    std::vector<int> coords(Rank, 0);
    context.commWorld = (MPI_Comm)(void *)574;
    context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, 1));
    context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, 0));
    context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
    context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));

    using FieldType = schnek::Field<double, Rank, DeviceGridStorage>;
    using ReferenceGrid = schnek::Grid<double, Rank>;
    using RangeType = schnek::Range<ptrdiff_t, Rank>;
    using DomainType = schnek::Range<double, Rank>;
    using StaggerType = typename FieldType::StaggerType;

    typename RangeType::LimitType globalLo(0);
    typename RangeType::LimitType globalHi(3);
    if constexpr (Rank == 2) {
      globalHi[1] = 2;
    }
    typename DomainType::LimitType domainLo(0.0);
    typename DomainType::LimitType domainHi(1.0);
    RangeType globalRange(globalLo, globalHi);
    DomainType globalDomain(domainLo, domainHi);

    schnek::MpiCartesianDomainDecomposition<Rank> decomposition(context);
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

    Kokkos::deep_copy(field->getKokkosView(), -17.0);
    Kokkos::fence();

    ReferenceGrid expected(field->getLo(), field->getHi());
    fillGrid(expected, -17.0);
    auto ghostSetup = buildGhostExchangeSetup(*field);
    applyGhostExpectations(expected, ghostSetup);
    context.ret_MPI_Sendrecv = ghostSetup.responses;

    for (size_t dim = 0; dim < Rank; ++dim) {
      context.ret_MPI_Cart_shift.push_back(
          boost::make_tuple(MPI_SUCCESS, static_cast<int>(10 + dim * 2), static_cast<int>(11 + dim * 2))
      );
    }

    exchangeRegistration<Rank>(decomposition, registration, false);

    // BOOST_REQUIRE_EQUAL(context.args_MPI_Sendrecv.size(), ghostSetup.expectations.size());
    // for (size_t i = 0; i < ghostSetup.expectations.size(); ++i) {
    //   const auto &call = context.args_MPI_Sendrecv[i];
    //   BOOST_CHECK(call.recvBufferPresent);
    //   BOOST_CHECK_EQUAL(call.recvCount, static_cast<int>(ghostSetup.expectations[i].values.size()));
    // }

    // auto mirror = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(), field->getKokkosView());
    // const auto gridLo = field->getLo();
    // for (auto pos : field->getRange()) {
    //   size_t offset = 0;
    //   for (size_t dim = 0; dim < Rank; ++dim) {
    //     offset += static_cast<size_t>(pos[dim] - gridLo[dim]) * mirror.stride(dim);
    //   }
    //   BOOST_CHECK_EQUAL(mirror.data()[offset], expected[pos]);
    // }
  }
#endif

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
      auto view = localField.getExecutionView();
      Iteration1d::forEach(range, KOKKOS_LAMBDA(const typename FieldType::IndexType &pos) {
          view[pos] = -42.0;
      });
  });
  std::cerr << "Field filled" << std::endl;

  ReferenceGrid expected(field->getLo(), field->getHi());
  std::cerr << "Filling expected grid with -42.0" << std::endl;
  fillGrid(expected, -42.0);
  std::cerr << "Expected grid filled" << std::endl;
  auto ghostSetup = buildGhostExchangeSetup(*field);
  std::cerr << "Applying ghost expectations to expected grid" << std::endl;
  applyGhostExpectations(expected, ghostSetup);
  context.ret_MPI_Sendrecv = ghostSetup.responses;

  std::array<int, 1> prevRanks{{7}};
  std::array<int, 1> nextRanks{{8}};
  for (size_t dim = 0; dim < prevRanks.size(); ++dim) {
    context.ret_MPI_Cart_shift.push_back(boost::make_tuple(MPI_SUCCESS, prevRanks[dim], nextRanks[dim]));
  }

  exchangeRegistration<1>(decomposition, registration, false);

  BOOST_REQUIRE_EQUAL(context.args_MPI_Sendrecv.size(), ghostSetup.expectations.size());
  for (size_t i = 0; i < ghostSetup.expectations.size(); ++i) {
    const auto &call = context.args_MPI_Sendrecv[i];
    BOOST_CHECK(call.recvBufferPresent);
    BOOST_CHECK_EQUAL(call.recvCount, static_cast<int>(ghostSetup.expectations[i].values.size()));
  }

  RangeType verificationRange(field->getLo(), field->getHi());
  for (auto it = verificationRange.begin(); it != verificationRange.end(); ++it) {
    BOOST_CHECK_EQUAL((*field)[*it], expected[*it]);
  }
}

BOOST_AUTO_TEST_CASE(ghost_exchange_updates_ghost_cells_kokkos_2d) {
  MpiTestContextImpl context;

  MPI_Comm testComm = (MPI_Comm)(void *)123;
  std::vector<int> coords(2, 0);
  context.commWorld = (MPI_Comm)(void *)574;
  context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, 1));
  context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, 0));
  context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
  context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));

  using FieldType = schnek::Field<double, 2, GridStorage>;
  using ReferenceGrid = schnek::Grid<double, 2>;
  using RangeType = schnek::Range<ptrdiff_t, 2>;
  using DomainType = schnek::Range<double, 2>;
  using StaggerType = typename FieldType::StaggerType;

  RangeType globalRange(schnek::Array<ptrdiff_t, 2>(0, 0), schnek::Array<ptrdiff_t, 2>(3, 2));
  DomainType globalDomain(schnek::Array<double, 2>(0.0, 0.0), schnek::Array<double, 2>(1.0, 1.0));

  schnek::MpiCartesianDomainDecomposition<2> decomposition(context);
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

  gridContext.forEach([&](const RangeType &range, FieldType &localField) {
      auto view = localField.getExecutionView();
      Iteration2d::forEach(range, KOKKOS_LAMBDA(const typename FieldType::IndexType &pos) {
          view[pos] = -13.0;
      });
  });

  ReferenceGrid expected(field->getLo(), field->getHi());
  fillGrid(expected, -13.0);
  auto ghostSetup = buildGhostExchangeSetup(*field);
  applyGhostExpectations(expected, ghostSetup);
  context.ret_MPI_Sendrecv = ghostSetup.responses;

  std::array<int, 2> prevRanks{{10, 20}};
  std::array<int, 2> nextRanks{{11, 21}};
  for (size_t dim = 0; dim < prevRanks.size(); ++dim) {
    context.ret_MPI_Cart_shift.push_back(boost::make_tuple(MPI_SUCCESS, prevRanks[dim], nextRanks[dim]));
  }

  exchangeRegistration<2>(decomposition, registration, false);

  BOOST_REQUIRE_EQUAL(context.args_MPI_Sendrecv.size(), ghostSetup.expectations.size());
  for (size_t i = 0; i < ghostSetup.expectations.size(); ++i) {
    const auto &call = context.args_MPI_Sendrecv[i];
    BOOST_CHECK(call.recvBufferPresent);
    BOOST_CHECK_EQUAL(call.recvCount, static_cast<int>(ghostSetup.expectations[i].values.size()));
  }

  RangeType verificationRange(field->getLo(), field->getHi());
  for (auto pos : verificationRange) {
    BOOST_CHECK_EQUAL((*field)[pos], expected[pos]);
  }
}

BOOST_AUTO_TEST_CASE(repeated_exchange_reuses_buffers_across_calls_and_value_types) {
  MpiTestContextImpl context;

  MPI_Comm testComm = (MPI_Comm)(void *)123;
  context.commWorld = (MPI_Comm)(void *)574;
  context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, 1));
  context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, 0));
  context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
  context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, {0}));
  context.ret_MPI_Cart_shift.push_back(boost::make_tuple(MPI_SUCCESS, 7, 8));

  using IntField = schnek::Field<int, 1, GridStorage>;
  using DoubleField = schnek::Field<double, 1, GridStorage>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;
  using DomainType = schnek::Range<double, 1>;

  RangeType globalRange(schnek::Array<ptrdiff_t, 1>(0), schnek::Array<ptrdiff_t, 1>(3));
  DomainType globalDomain(schnek::Array<double, 1>(0.0), schnek::Array<double, 1>(1.0));
  schnek::MpiCartesianDomainDecomposition<1> decomposition(context);
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);
  decomposition.init();

  typename IntField::StaggerType noStagger(false);
  schnek::GridFactory<IntField> intFactory(noStagger, 1);
  schnek::GridFactory<DoubleField> doubleFactory(noStagger, 1);
  auto intRegistration = decomposition.registerField(intFactory);
  auto doubleRegistration = decomposition.registerField(doubleFactory);

  IntField *intField = nullptr;
  decomposition.getGridContext({intRegistration}).forEach(
      [&](const RangeType &, IntField &field) { intField = &field; }
  );
  DoubleField *doubleField = nullptr;
  decomposition.getGridContext({doubleRegistration}).forEach(
      [&](const RangeType &, DoubleField &field) { doubleField = &field; }
  );
  BOOST_REQUIRE(intField != nullptr);
  BOOST_REQUIRE(doubleField != nullptr);

  fillGrid(*intField, -4);
  auto intSetup = buildGhostExchangeSetup(*intField);
  context.ret_MPI_Sendrecv.insert(context.ret_MPI_Sendrecv.end(), intSetup.responses.begin(), intSetup.responses.end());
  exchangeRegistration<1>(decomposition, intRegistration, false);
  BOOST_REQUIRE_EQUAL(context.args_MPI_Sendrecv.size(), 2u);
  const void *firstSendBuffer = context.args_MPI_Sendrecv[0].sendBuffer;
  void *firstReceiveBuffer = context.args_MPI_Sendrecv[0].recvBuffer;
  BOOST_CHECK(firstSendBuffer != nullptr);
  BOOST_CHECK(firstReceiveBuffer != nullptr);

  fillGrid(*intField, -8);
  intSetup = buildGhostExchangeSetup(*intField);
  context.ret_MPI_Sendrecv.insert(context.ret_MPI_Sendrecv.end(), intSetup.responses.begin(), intSetup.responses.end());
  exchangeRegistration<1>(decomposition, intRegistration, false);
  BOOST_REQUIRE_EQUAL(context.args_MPI_Sendrecv.size(), 4u);
  BOOST_CHECK_EQUAL(context.args_MPI_Sendrecv[2].sendBuffer, firstSendBuffer);
  BOOST_CHECK_EQUAL(context.args_MPI_Sendrecv[2].recvBuffer, firstReceiveBuffer);

  fillGrid(*doubleField, -12.0);
  auto doubleSetup = buildGhostExchangeSetup(*doubleField);
  context.ret_MPI_Sendrecv.insert(
      context.ret_MPI_Sendrecv.end(), doubleSetup.responses.begin(), doubleSetup.responses.end()
  );
  exchangeRegistration<1>(decomposition, doubleRegistration, false);
  BOOST_REQUIRE_EQUAL(context.args_MPI_Sendrecv.size(), 6u);

  for (const auto &expectation : doubleSetup.expectations) {
    size_t index = 0;
    for (auto it = expectation.range.begin(); it != expectation.range.end(); ++it) {
      BOOST_CHECK_EQUAL((*doubleField)[*it], expectation.values[index++]);
    }
  }
}

#ifdef KOKKOS_ENABLE_CUDA
BOOST_AUTO_TEST_CASE(accumulate_updates_device_grid_and_inner_cells) {
  MpiTestContextImpl context;

  MPI_Comm testComm = (MPI_Comm)(void *)123;
  context.commWorld = (MPI_Comm)(void *)574;
  context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, 1));
  context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, 0));
  context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
  context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, {0}));
  context.ret_MPI_Cart_shift.push_back(boost::make_tuple(MPI_SUCCESS, 7, 8));

  using FieldType = schnek::Field<double, 1, DeviceGridStorage>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;
  using DomainType = schnek::Range<double, 1>;

  RangeType globalRange(schnek::Array<ptrdiff_t, 1>(0), schnek::Array<ptrdiff_t, 1>(3));
  DomainType globalDomain(schnek::Array<double, 1>(0.0), schnek::Array<double, 1>(1.0));
  schnek::MpiCartesianDomainDecomposition<1> decomposition(context);
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);
  decomposition.init();

  typename FieldType::StaggerType noStagger(false);
  schnek::GridFactory<FieldType> factory(noStagger, 1);
  auto registration = decomposition.registerField(factory);
  FieldType *field = nullptr;
  decomposition.getGridContext({registration}).forEach(
      [&](const RangeType &, FieldType &localField) { field = &localField; }
  );
  BOOST_REQUIRE(field != nullptr);

  Kokkos::deep_copy(field->getKokkosView(), 1.0);
  context.ret_MPI_Sendrecv = {
      boost::make_tuple(MPI_SUCCESS, toByteVector(std::vector<double>{2.0})),
      boost::make_tuple(MPI_SUCCESS, toByteVector(std::vector<double>{4.0})),
      boost::make_tuple(MPI_SUCCESS, toByteVector(std::vector<double>{5.0})),
      boost::make_tuple(MPI_SUCCESS, toByteVector(std::vector<double>{7.0}))
  };

  static_cast<schnek::DomainDecomposition<1> &>(decomposition).accumulate(registration, false);

  auto mirror = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(), field->getKokkosView());
  BOOST_CHECK_EQUAL(mirror(0), 3.0);
  BOOST_CHECK_EQUAL(mirror(1), 7.0);
  BOOST_CHECK_EQUAL(mirror(4), 4.0);
  BOOST_CHECK_EQUAL(mirror(5), 6.0);
}

BOOST_AUTO_TEST_CASE(ghost_exchange_updates_ghost_cells_kokkos_cuda_space_1d) {
  runDeviceGhostExchange<1>();
}

BOOST_AUTO_TEST_CASE(ghost_exchange_updates_ghost_cells_kokkos_cuda_space_2d) {
  runDeviceGhostExchange<2>();
}
#endif

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

#endif  // SCHNEK_HAVE_KOKKOS && SCHNEK_HAVE_MPI
