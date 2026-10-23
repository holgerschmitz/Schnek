/*
 *  test_kokkos_redistribution.cpp
 *
 *  Load-balancing redistribution of grids and projected grids on the device
 *  transfer path.
 *
 *  The host-backed tests (rebalance_host_backed_*) use HostBackedDeviceStorage,
 *  which runs on any Kokkos build, including Kokkos::Serial. They guard the
 *  logic of the device path. The CUDA tests use real device allocations
 *  (CudaSpace) and only exist in CUDA builds. They guard the memory-space
 *  invariant. Host-side data is read and written through Kokkos host mirrors.
 */

#include "mpi_test_context.hpp"
#include "test_device_storage.hpp"
#include "../utility.hpp"

#include <grid/range.hpp>
#include <grid/grid.hpp>
#include <grid/field.hpp>

#include <decomposition/mpi_cartesian_decomposition.hpp>

#include <boost/test/unit_test.hpp>

#if defined(SCHNEK_HAVE_KOKKOS) && defined(SCHNEK_HAVE_MPI)

#include <grid/gridstorage/kokkos-storage.hpp>

#include <array>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#ifdef KOKKOS_ENABLE_CUDA
#include <cuda_runtime_api.h>
#endif

// Named rather than anonymous so that lambdas passed to templated
// GridContext::forEach have external linkage (avoids -Wattributes).
namespace schnek_redistribution_test {

#ifdef KOKKOS_ENABLE_CUDA
  template<typename T, size_t rank>
  using DeviceGridStorage = schnek::KokkosGridStorage<T, rank, Kokkos::CudaSpace>;
#endif

  template<typename T>
  std::vector<char> toByteVector(const std::vector<T> &values) {
    std::vector<char> bytes(values.size() * sizeof(T));
    if (!values.empty()) {
      std::memcpy(bytes.data(), values.data(), bytes.size());
    }
    return bytes;
  }

  // Distinct value per global cell, so a misplaced cell is detectable.
  // For Rank 2 this is f(i, j) = 1000 * i + j.
  template<size_t Rank, typename IndexType>
  double encodeIndex(const IndexType &pos) {
    double value = 0.0;
    for (size_t dim = 0; dim < Rank; ++dim) {
      value = value * 1000.0 + static_cast<double>(pos[dim]);
    }
    return value;
  }

  // Element offset of `pos` in a view whose origin is `lo`.
  template<size_t Rank, typename ViewType, typename IndexType, typename PosType>
  size_t viewOffset(const ViewType &view, const IndexType &lo, const PosType &pos) {
    size_t offset = 0;
    for (size_t dim = 0; dim < Rank; ++dim) {
      offset += static_cast<size_t>(pos[dim] - lo[dim]) * view.stride(dim);
    }
    return offset;
  }

  template<size_t Rank, typename MirrorType, typename IndexType, typename PosType>
  double hostValue(const MirrorType &mirror, const IndexType &lo, const PosType &pos) {
    return mirror.data()[viewOffset<Rank>(mirror, lo, pos)];
  }

  // Maps an index that may lie one cell outside [globalLo, globalHi] back
  // into the global range, wrapping periodically.
  template<size_t Rank, typename IndexType>
  IndexType wrapIntoGlobal(IndexType pos, const IndexType &globalLo, const IndexType &globalHi) {
    for (size_t dim = 0; dim < Rank; ++dim) {
      ptrdiff_t extent = globalHi[dim] - globalLo[dim] + 1;
      pos[dim] = ((pos[dim] - globalLo[dim]) % extent + extent) % extent + globalLo[dim];
    }
    return pos;
  }

  template<typename GridT>
  auto copyToHost(GridT &grid) {
    return Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(), grid.getKokkosView());
  }

  // Writes values for the whole allocation of a device grid, including ghost cells.
  template<size_t Rank, typename GridT, typename ValueFn>
  void fillDeviceGrid(GridT &grid, ValueFn valueAt) {
    using RangeType = typename GridT::RangeType;

    auto view = grid.getKokkosView();
    auto host = Kokkos::create_mirror_view(Kokkos::HostSpace(), view);
    RangeType range(grid.getLo(), grid.getHi());
    for (auto pos : range) {
      host.data()[viewOffset<Rank>(host, grid.getLo(), pos)] = valueAt(pos);
    }
    Kokkos::deep_copy(view, host);
    Kokkos::fence();
  }

  void setupMockRank(
      MpiTestContextImpl &context,
      int numProcs,
      int myRank,
      const std::vector<int> &coords,
      MPI_Comm testComm
  ) {
    context.commWorld = (MPI_Comm)(void *)574;
    context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, numProcs));
    context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, myRank));
    context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
    context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));
  }

  // A single process owns the whole layout, so balanceLoad() can only copy
  // device data locally. Covers 1D, 2D and 3D grids.
  template<template<typename, size_t> class Storage, size_t Rank>
  void runSingleRankDeviceRebalance() {
    constexpr std::array<ptrdiff_t, 3> extents{{8, 6, 4}};

    MpiTestContextImpl context;
    setupMockRank(context, 1, 0, std::vector<int>(Rank, 0), (MPI_Comm)(void *)123);

    using GridType = schnek::Grid<double, Rank, Storage>;
    using RangeType = schnek::Range<ptrdiff_t, Rank>;
    using DomainType = schnek::Range<double, Rank>;

    typename RangeType::LimitType globalLo(0);
    typename RangeType::LimitType globalHi(0);
    for (size_t dim = 0; dim < Rank; ++dim) {
      globalHi[dim] = extents[dim] - 1;
    }
    RangeType globalRange(globalLo, globalHi);
    typename DomainType::LimitType domainLo(0.0);
    typename DomainType::LimitType domainHi(1.0);
    DomainType globalDomain(domainLo, domainHi);

    schnek::MpiCartesianDomainDecomposition<Rank> decomposition(context);
    decomposition.setGlobalRange(globalRange);
    decomposition.setGlobalDomain(globalDomain);
    decomposition.init();

    schnek::GridFactory<GridType> factory;
    schnek::GridRegistration registration = decomposition.registerField(factory);

    GridType *grid = nullptr;
    decomposition.getGridContext({registration}).forEach(
        [&](const RangeType &, GridType &localGrid) { grid = &localGrid; }
    );
    BOOST_REQUIRE(grid != nullptr);
    fillDeviceGrid<Rank>(*grid, [](const auto &pos) { return encodeIndex<Rank>(pos); });

    decomposition.balanceLoad();

    int callCount = 0;
    decomposition.getGridContext({registration}).forEach(
        [&](const RangeType &range, GridType &localGrid) {
          ++callCount;
          SCHNEK_CHECK_EQUAL(range, globalRange);
          auto mirror = copyToHost(localGrid);
          for (auto pos : range) {
            BOOST_CHECK_EQUAL(hostValue<Rank>(mirror, localGrid.getLo(), pos), encodeIndex<Rank>(pos));
          }
        }
    );
    BOOST_CHECK_EQUAL(callCount, 1);

    BOOST_CHECK_EQUAL(context.args_MPI_Isend.size(), static_cast<size_t>(0));
    BOOST_CHECK_EQUAL(context.args_MPI_Irecv.size(), static_cast<size_t>(0));
  }

}  // namespace schnek_redistribution_test

using namespace schnek_redistribution_test;

BOOST_AUTO_TEST_SUITE(domain_decomposition)
BOOST_AUTO_TEST_SUITE(kokkos_redistribution)

BOOST_AUTO_TEST_CASE(rebalance_host_backed_device_path_single_rank_1d_2d_3d) {
  runSingleRankDeviceRebalance<HostBackedDeviceStorage, 1>();
  runSingleRankDeviceRebalance<HostBackedDeviceStorage, 2>();
  runSingleRankDeviceRebalance<HostBackedDeviceStorage, 3>();
}

#ifdef KOKKOS_ENABLE_CUDA
BOOST_AUTO_TEST_CASE(rebalance_cuda_space_1d_2d_3d) {
  runSingleRankDeviceRebalance<DeviceGridStorage, 1>();
  runSingleRankDeviceRebalance<DeviceGridStorage, 2>();
  runSingleRankDeviceRebalance<DeviceGridStorage, 3>();
}
#endif

// Two processes split a 1D grid [0..11] into [0..5] | [6..11]. Grow moves the
// cut to [0..7] | [8..11], so rank 0 receives cells [6..7] from rank 1. Shrink
// moves it to [0..3] | [4..11], so rank 0 sends cells [4..5] to rank 1.
// Ghost cells of the old layout hold a sentinel, so a ghost copied into the
// new layout is detected.
template<template<typename, size_t> class Storage>
void runField1DRebalance(bool grow) {
  MpiTestContextImpl context;
  setupMockRank(context, 2, 0, {0}, (MPI_Comm)(void *)123);

  using FieldType = schnek::Field<double, 1, Storage>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;
  using DomainType = schnek::Range<double, 1>;

  RangeType globalRange(schnek::Array<ptrdiff_t, 1>(0), schnek::Array<ptrdiff_t, 1>(11));
  DomainType globalDomain(schnek::Array<double, 1>(0.0), schnek::Array<double, 1>(1.0));

  schnek::MpiCartesianDomainDecomposition<1> decomposition(context);
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);
  decomposition.init();

  typename FieldType::StaggerType noStagger(false);
  constexpr int ghostCells = 1;
  schnek::GridFactory<FieldType> factory(noStagger, ghostCells);
  schnek::GridRegistration registration = decomposition.registerField(factory);

  FieldType *field = nullptr;
  decomposition.getGridContext({registration}).forEach(
      [&](const RangeType &, FieldType &localField) { field = &localField; }
  );
  BOOST_REQUIRE(field != nullptr);
  fillDeviceGrid<1>(*field, [](const auto &pos) {
    return (pos[0] >= 0 && pos[0] <= 5) ? encodeIndex<1>(pos) : -1.0;
  });

  schnek::Grid<double, 1> weights(schnek::Array<ptrdiff_t, 1>(0), schnek::Array<ptrdiff_t, 1>(11));
  for (ptrdiff_t i = 0; i <= 11; ++i) {
    if (grow) {
      weights[schnek::Array<ptrdiff_t, 1>(i)] = (i <= 7) ? 1.0 : 3.0;
    } else {
      weights[schnek::Array<ptrdiff_t, 1>(i)] = (i <= 3) ? 7.0 : 4.0;
    }
  }
  decomposition.setGlobalWeights(weights);

  if (grow) {
    context.ret_MPI_Irecv.push_back(boost::make_tuple(MPI_SUCCESS, toByteVector(std::vector<double>{6.0, 7.0})));
  }

  decomposition.balanceLoad();

  RangeType expectedRange(schnek::Array<ptrdiff_t, 1>(0), schnek::Array<ptrdiff_t, 1>(grow ? 7 : 3));
  int callCount = 0;
  decomposition.getGridContext({registration}).forEach(
      [&](const RangeType &range, FieldType &localField) {
        ++callCount;
        SCHNEK_CHECK_EQUAL(range, expectedRange);
        auto mirror = copyToHost(localField);
        for (auto pos : range) {
          BOOST_CHECK_EQUAL(hostValue<1>(mirror, localField.getLo(), pos), encodeIndex<1>(pos));
        }
      }
  );
  BOOST_CHECK_EQUAL(callCount, 1);

  if (grow) {
    BOOST_REQUIRE_EQUAL(context.args_MPI_Irecv.size(), static_cast<size_t>(1));
    BOOST_CHECK_EQUAL(context.args_MPI_Irecv[0].source, 1);
    BOOST_CHECK_EQUAL(context.args_MPI_Irecv[0].count, 2);
    BOOST_CHECK_EQUAL(context.args_MPI_Isend.size(), static_cast<size_t>(0));
  } else {
    BOOST_REQUIRE_EQUAL(context.args_MPI_Isend.size(), static_cast<size_t>(1));
    BOOST_CHECK_EQUAL(context.args_MPI_Isend[0].dest, 1);
    BOOST_CHECK_EQUAL(context.args_MPI_Isend[0].count, 2);
    BOOST_CHECK_EQUAL(context.args_MPI_Irecv.size(), static_cast<size_t>(0));
  }
}

BOOST_AUTO_TEST_CASE(rebalance_host_backed_device_path_field_with_ghosts_grow) {
  runField1DRebalance<HostBackedDeviceStorage>(true);
}

BOOST_AUTO_TEST_CASE(rebalance_host_backed_device_path_field_with_ghosts_shrink) {
  runField1DRebalance<HostBackedDeviceStorage>(false);
}

#ifdef KOKKOS_ENABLE_CUDA
BOOST_AUTO_TEST_CASE(rebalance_cuda_space_field_with_ghosts_grow) {
  runField1DRebalance<DeviceGridStorage>(true);
}

BOOST_AUTO_TEST_CASE(rebalance_cuda_space_field_with_ghosts_shrink) {
  runField1DRebalance<DeviceGridStorage>(false);
}
#endif

// Two processes in a 2x1 layout split axis 0 of a 2D grid [0..11]x[0..3].
// Grow moves the cut to [0..7] | [8..11], so rank 0 receives the cells
// [6..7]x[0..3] from rank 1. A ghost exchange then fills every allocated cell,
// including the ghost cells of the new layout, with the periodically wrapped
// global index. Ghost cells of the old layout hold a sentinel so that a copy
// which never happened is detected.
template<template<typename, size_t> class Storage>
void runField2DRebalanceAndExchange() {
  MpiTestContextImpl context;
  setupMockRank(context, 2, 0, {0, 0}, (MPI_Comm)(void *)123);

  using FieldType = schnek::Field<double, 2, Storage>;
  using RangeType = schnek::Range<ptrdiff_t, 2>;
  using DomainType = schnek::Range<double, 2>;
  using IndexType = schnek::Array<ptrdiff_t, 2>;

  const IndexType globalLo(0, 0);
  const IndexType globalHi(11, 3);
  RangeType globalRange(globalLo, globalHi);
  DomainType globalDomain(schnek::Array<double, 2>(0.0, 0.0), schnek::Array<double, 2>(1.0, 1.0));

  schnek::MpiCartesianDomainDecomposition<2> decomposition(context);
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);
  decomposition.init();

  typename FieldType::StaggerType noStagger(false);
  constexpr int ghostCells = 1;
  schnek::GridFactory<FieldType> factory(noStagger, ghostCells);
  schnek::GridRegistration registration = decomposition.registerField(factory);

  FieldType *field = nullptr;
  decomposition.getGridContext({registration}).forEach(
      [&](const RangeType &, FieldType &localField) { field = &localField; }
  );
  BOOST_REQUIRE(field != nullptr);
  fillDeviceGrid<2>(*field, [](const auto &pos) {
    bool inner = pos[0] >= 0 && pos[0] <= 5 && pos[1] >= 0 && pos[1] <= 3;
    return inner ? encodeIndex<2>(pos) : -1.0;
  });

  schnek::Grid<double, 2> weights(globalLo, globalHi);
  for (ptrdiff_t i = 0; i <= 11; ++i) {
    for (ptrdiff_t j = 0; j <= 3; ++j) {
      weights[IndexType(i, j)] = (i <= 7) ? 1.0 : 3.0;
    }
  }
  decomposition.setGlobalWeights(weights);

  std::vector<double> received;
  for (ptrdiff_t i = 6; i <= 7; ++i) {
    for (ptrdiff_t j = 0; j <= 3; ++j) {
      received.push_back(encodeIndex<2>(IndexType(i, j)));
    }
  }
  context.ret_MPI_Irecv.push_back(boost::make_tuple(MPI_SUCCESS, toByteVector(received)));

  decomposition.balanceLoad();

  BOOST_REQUIRE_EQUAL(context.args_MPI_Irecv.size(), static_cast<size_t>(1));
  BOOST_CHECK_EQUAL(context.args_MPI_Irecv[0].source, 1);
  BOOST_CHECK_EQUAL(context.args_MPI_Irecv[0].count, 8);
  BOOST_CHECK_EQUAL(context.args_MPI_Isend.size(), static_cast<size_t>(0));

  RangeType expectedRange(IndexType(0, 0), IndexType(7, 3));
  int callCount = 0;
  decomposition.getGridContext({registration}).forEach(
      [&](const RangeType &range, FieldType &) {
        ++callCount;
        SCHNEK_CHECK_EQUAL(range, expectedRange);
      }
  );
  BOOST_CHECK_EQUAL(callCount, 1);

  // Rank 0 has periodic neighbours in dimension 0 (rank 1 on both sides) and
  // itself in dimension 1.
  context.ret_MPI_Cart_shift.push_back(boost::make_tuple(MPI_SUCCESS, 1, 1));
  context.ret_MPI_Cart_shift.push_back(boost::make_tuple(MPI_SUCCESS, 0, 0));

  // Ghost slices in the order the exchange issues them: dim 0 lower, dim 0
  // upper, dim 1 lower, dim 1 upper. Each value is the wrapped global index.
  auto sliceBytes = [&](ptrdiff_t loI, ptrdiff_t hiI, ptrdiff_t loJ, ptrdiff_t hiJ) {
    std::vector<double> values;
    for (ptrdiff_t i = loI; i <= hiI; ++i) {
      for (ptrdiff_t j = loJ; j <= hiJ; ++j) {
        values.push_back(encodeIndex<2>(wrapIntoGlobal<2>(IndexType(i, j), globalLo, globalHi)));
      }
    }
    return toByteVector(values);
  };
  context.ret_MPI_Sendrecv.push_back(boost::make_tuple(MPI_SUCCESS, sliceBytes(-1, -1, -1, 4)));
  context.ret_MPI_Sendrecv.push_back(boost::make_tuple(MPI_SUCCESS, sliceBytes(8, 8, -1, 4)));
  context.ret_MPI_Sendrecv.push_back(boost::make_tuple(MPI_SUCCESS, sliceBytes(-1, 8, -1, -1)));
  context.ret_MPI_Sendrecv.push_back(boost::make_tuple(MPI_SUCCESS, sliceBytes(-1, 8, 4, 4)));

  decomposition.exchange(registration, false);

  BOOST_REQUIRE_EQUAL(context.args_MPI_Sendrecv.size(), static_cast<size_t>(4));
  const int expectedCounts[] = {6, 6, 10, 10};
  for (size_t i = 0; i < 4; ++i) {
    BOOST_CHECK_EQUAL(context.args_MPI_Sendrecv[i].recvCount, expectedCounts[i]);
  }
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_shift.size(), static_cast<size_t>(2));

  decomposition.getGridContext({registration}).forEach(
      [&](const RangeType &, FieldType &localField) { field = &localField; }
  );
  auto mirror = copyToHost(*field);
  RangeType allocation(field->getLo(), field->getHi());
  for (auto pos : allocation) {
    BOOST_CHECK_EQUAL(
        hostValue<2>(mirror, field->getLo(), pos),
        encodeIndex<2>(wrapIntoGlobal<2>(pos, globalLo, globalHi))
    );
  }
}

BOOST_AUTO_TEST_CASE(rebalance_host_backed_device_path_field_2d_exchange) {
  runField2DRebalanceAndExchange<HostBackedDeviceStorage>();
}

#ifdef KOKKOS_ENABLE_CUDA
BOOST_AUTO_TEST_CASE(rebalance_cuda_space_field_2d_exchange) {
  runField2DRebalanceAndExchange<DeviceGridStorage>();
}
#endif

// Two processes in a 2x1 layout split axis 0. The projection keeps axis 1,
// so the projected grid is replicated across both processes. The layout is
// unchanged, so no point-to-point transfer happens. The only remote step is
// the Step 2 MPI_Bcast from the canonical process (rank 0), which packs
// the projected grid from device memory into a buffer.
template<template<typename, size_t> class Storage>
void runProjectedGridRebalance() {
  const MPI_Comm subComm = (MPI_Comm)(void *)456;

  MpiTestContextImpl context;
  setupMockRank(context, 2, 0, {0, 0}, (MPI_Comm)(void *)123);
  context.ret_MPI_Cart_sub.push_back(boost::make_tuple(MPI_SUCCESS, subComm));

  using GridType2D = schnek::Grid<double, 2, Storage>;
  using GridType1D = schnek::Grid<double, 1, Storage>;
  using RangeType2D = schnek::Range<ptrdiff_t, 2>;
  using RangeType1D = schnek::Range<ptrdiff_t, 1>;
  using DomainType2D = schnek::Range<double, 2>;

  RangeType2D globalRange(schnek::Array<ptrdiff_t, 2>(0, 0), schnek::Array<ptrdiff_t, 2>(7, 7));
  DomainType2D globalDomain(schnek::Array<double, 2>(0.0, 0.0), schnek::Array<double, 2>(1.0, 1.0));

  schnek::MpiCartesianDomainDecomposition<2> decomposition(context);
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);
  decomposition.init();

  schnek::GridFactory<GridType2D> factory2d;
  schnek::GridRegistration reg2d = decomposition.registerField(factory2d);

  schnek::GridFactory<GridType1D> factory1d;
  auto projReg = decomposition.registerFieldProjection(factory1d, std::array<size_t, 1>{1});

  GridType2D *grid2d = nullptr;
  decomposition.getGridContext({reg2d}).forEach(
      [&](const RangeType2D &, GridType2D &localGrid) { grid2d = &localGrid; }
  );
  BOOST_REQUIRE(grid2d != nullptr);
  fillDeviceGrid<2>(*grid2d, [](const auto &pos) { return encodeIndex<2>(pos); });

  auto projCtx = decomposition.getProjectedGridContext<1>({projReg});
  GridType1D *projGrid = nullptr;
  projCtx.forEach([&](const RangeType1D &, GridType1D &localGrid) { projGrid = &localGrid; });
  BOOST_REQUIRE(projGrid != nullptr);
  fillDeviceGrid<1>(*projGrid, [](const auto &pos) { return 100.0 + encodeIndex<1>(pos); });

  decomposition.balanceLoad();

  int gridCallCount = 0;
  decomposition.getGridContext({reg2d}).forEach(
      [&](const RangeType2D &range, GridType2D &localGrid) {
        ++gridCallCount;
        auto mirror = copyToHost(localGrid);
        for (auto pos : range) {
          BOOST_CHECK_EQUAL(hostValue<2>(mirror, localGrid.getLo(), pos), encodeIndex<2>(pos));
        }
      }
  );
  BOOST_CHECK_EQUAL(gridCallCount, 1);

  RangeType1D expectedProjRange(schnek::Array<ptrdiff_t, 1>(0), schnek::Array<ptrdiff_t, 1>(7));
  int projCallCount = 0;
  decomposition.getProjectedGridContext<1>({projReg}).forEach(
      [&](const RangeType1D &range, GridType1D &localGrid) {
        ++projCallCount;
        SCHNEK_CHECK_EQUAL(range, expectedProjRange);
        auto mirror = copyToHost(localGrid);
        for (auto pos : range) {
          BOOST_CHECK_EQUAL(hostValue<1>(mirror, localGrid.getLo(), pos), 100.0 + encodeIndex<1>(pos));
        }
      }
  );
  BOOST_CHECK_EQUAL(projCallCount, 1);

  BOOST_REQUIRE_EQUAL(context.args_MPI_Bcast.size(), static_cast<size_t>(1));
  BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<0>(), 8);
  BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<2>(), 0);
  BOOST_CHECK_EQUAL(context.args_MPI_Irecv.size(), static_cast<size_t>(0));
  BOOST_CHECK_EQUAL(context.args_MPI_Isend.size(), static_cast<size_t>(0));
}

BOOST_AUTO_TEST_CASE(rebalance_host_backed_device_path_projected_grid_replicated) {
  runProjectedGridRebalance<HostBackedDeviceStorage>();
}

#ifdef KOKKOS_ENABLE_CUDA
BOOST_AUTO_TEST_CASE(rebalance_cuda_space_projected_grid_replicated) {
  runProjectedGridRebalance<DeviceGridStorage>();
}
#endif

// Mirror of the host split test: the projection runs along axis 0, so the
// projected cut moves with the 2D cut. Rank 0 owns [0..3] before and [0..5]
// after balancing, so it receives the 2D cells [4..5]x[0..7] and the
// projected cells [4..5] from rank 1.
template<template<typename, size_t> class Storage>
void runProjectedGridSplitRebalance() {
  MpiTestContextImpl context;
  setupMockRank(context, 2, 0, {0, 0}, (MPI_Comm)(void *)123);

  using GridType2D = schnek::Grid<double, 2, Storage>;
  using GridType1D = schnek::Grid<double, 1, Storage>;
  using RangeType2D = schnek::Range<ptrdiff_t, 2>;
  using RangeType1D = schnek::Range<ptrdiff_t, 1>;
  using IndexType2D = schnek::Array<ptrdiff_t, 2>;
  using IndexType1D = schnek::Array<ptrdiff_t, 1>;

  RangeType2D globalRange(IndexType2D(0, 0), IndexType2D(7, 7));
  schnek::Range<double, 2> globalDomain(
      schnek::Array<double, 2>(0.0, 0.0), schnek::Array<double, 2>(1.0, 1.0));

  schnek::MpiCartesianDomainDecomposition<2> decomposition(context);
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);
  decomposition.init();

  schnek::GridFactory<GridType2D> factory2d;
  schnek::GridRegistration reg2d = decomposition.registerField(factory2d);

  schnek::GridFactory<GridType1D> factory1d;
  auto projReg = decomposition.registerFieldProjection(factory1d, std::array<size_t, 1>{0});

  GridType2D *grid2d = nullptr;
  decomposition.getGridContext({reg2d}).forEach(
      [&](const RangeType2D &, GridType2D &localGrid) { grid2d = &localGrid; }
  );
  BOOST_REQUIRE(grid2d != nullptr);
  fillDeviceGrid<2>(*grid2d, [](const auto &pos) { return encodeIndex<2>(pos); });

  GridType1D *projGrid = nullptr;
  decomposition.getProjectedGridContext<1>({projReg}).forEach(
      [&](const RangeType1D &, GridType1D &localGrid) { projGrid = &localGrid; }
  );
  BOOST_REQUIRE(projGrid != nullptr);
  fillDeviceGrid<1>(*projGrid, [](const auto &pos) { return 100.0 + encodeIndex<1>(pos); });

  schnek::Grid<double, 2> weights(IndexType2D(0, 0), IndexType2D(7, 7));
  for (ptrdiff_t i = 0; i <= 7; ++i) {
    for (ptrdiff_t j = 0; j <= 7; ++j) {
      weights[IndexType2D(i, j)] = (i <= 5) ? 1.0 : 12.0;
    }
  }
  decomposition.setGlobalWeights(weights);

  std::vector<double> recv2d;
  for (ptrdiff_t i = 4; i <= 5; ++i) {
    for (ptrdiff_t j = 0; j <= 7; ++j) {
      recv2d.push_back(encodeIndex<2>(IndexType2D(i, j)));
    }
  }
  std::vector<double> recvProj;
  for (ptrdiff_t i = 4; i <= 5; ++i) {
    recvProj.push_back(100.0 + encodeIndex<1>(IndexType1D(i)));
  }
  context.ret_MPI_Irecv.push_back(boost::make_tuple(MPI_SUCCESS, toByteVector(recv2d)));
  context.ret_MPI_Irecv.push_back(boost::make_tuple(MPI_SUCCESS, toByteVector(recvProj)));

  decomposition.balanceLoad();

  RangeType2D expected2dRange(IndexType2D(0, 0), IndexType2D(5, 7));
  int gridCallCount = 0;
  decomposition.getGridContext({reg2d}).forEach(
      [&](const RangeType2D &range, GridType2D &localGrid) {
        ++gridCallCount;
        SCHNEK_CHECK_EQUAL(range, expected2dRange);
        auto mirror = copyToHost(localGrid);
        for (auto pos : range) {
          BOOST_CHECK_EQUAL(hostValue<2>(mirror, localGrid.getLo(), pos), encodeIndex<2>(pos));
        }
      }
  );
  BOOST_CHECK_EQUAL(gridCallCount, 1);

  RangeType1D expectedProjRange(IndexType1D(0), IndexType1D(5));
  int projCallCount = 0;
  decomposition.getProjectedGridContext<1>({projReg}).forEach(
      [&](const RangeType1D &range, GridType1D &localGrid) {
        ++projCallCount;
        SCHNEK_CHECK_EQUAL(range, expectedProjRange);
        auto mirror = copyToHost(localGrid);
        for (auto pos : range) {
          BOOST_CHECK_EQUAL(hostValue<1>(mirror, localGrid.getLo(), pos), 100.0 + encodeIndex<1>(pos));
        }
      }
  );
  BOOST_CHECK_EQUAL(projCallCount, 1);

  BOOST_REQUIRE_EQUAL(context.args_MPI_Cart_sub.size(), static_cast<size_t>(1));
  const std::vector<int> &remain = context.args_MPI_Cart_sub[0].get<1>();
  BOOST_REQUIRE_EQUAL(remain.size(), static_cast<size_t>(2));
  BOOST_CHECK_EQUAL(remain[0], 0);
  BOOST_CHECK_EQUAL(remain[1], 1);

  BOOST_REQUIRE_EQUAL(context.args_MPI_Irecv.size(), static_cast<size_t>(2));
  BOOST_CHECK_EQUAL(context.args_MPI_Irecv[0].source, 1);
  BOOST_CHECK_EQUAL(context.args_MPI_Irecv[0].count, 16);
  BOOST_CHECK_EQUAL(context.args_MPI_Irecv[1].source, 1);
  BOOST_CHECK_EQUAL(context.args_MPI_Irecv[1].count, 2);
  BOOST_CHECK_EQUAL(context.args_MPI_Irecv[1].tag, 1);
  BOOST_CHECK_EQUAL(context.args_MPI_Isend.size(), static_cast<size_t>(0));
  BOOST_CHECK_EQUAL(context.args_MPI_Waitall.size(), static_cast<size_t>(2));
}

BOOST_AUTO_TEST_CASE(rebalance_host_backed_device_path_projected_grid_split) {
  runProjectedGridSplitRebalance<HostBackedDeviceStorage>();
}

#ifdef KOKKOS_ENABLE_CUDA
BOOST_AUTO_TEST_CASE(rebalance_cuda_space_projected_grid_split) {
  runProjectedGridSplitRebalance<DeviceGridStorage>();
}
#endif

#ifdef KOKKOS_ENABLE_CUDA
BOOST_AUTO_TEST_CASE(rebalance_cuda_space_two_rank_real_mpi_1d_2d) {
  const bool requireDeviceTests = std::getenv("SCHNEK_REQUIRE_CUDA_DEVICE_TESTS") != nullptr;

  int worldSize = 0;
  BOOST_REQUIRE_EQUAL(MPI_Comm_size(MPI_COMM_WORLD, &worldSize), MPI_SUCCESS);
  if (worldSize != 2) {
    const std::string reason = "requires exactly two MPI ranks (world size is " + std::to_string(worldSize) + ")";
    if (requireDeviceTests) {
      BOOST_FAIL(reason);
    }
    std::cout << "SKIP: " << reason << "; set SCHNEK_REQUIRE_CUDA_DEVICE_TESTS to make this a failure\n"
              << std::flush;
    return;
  }

  int deviceCount = 0;
  const cudaError_t deviceStatus = cudaGetDeviceCount(&deviceCount);
  const int localHasDevice = deviceStatus == cudaSuccess && deviceCount > 0 ? 1 : 0;
  int allRanksHaveDevice = 0;
  BOOST_REQUIRE_EQUAL(
      MPI_Allreduce(&localHasDevice, &allRanksHaveDevice, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD), MPI_SUCCESS
  );
  if (!allRanksHaveDevice) {
    std::string reason = "no CUDA device is available to every MPI rank";
    if (deviceStatus != cudaSuccess) {
      reason += " (" + std::string(cudaGetErrorString(deviceStatus)) + ")";
    }
    if (requireDeviceTests) {
      BOOST_FAIL(reason);
    }
    std::cout << "SKIP: " << reason << "; set SCHNEK_REQUIRE_CUDA_DEVICE_TESTS to make this a failure\n"
              << std::flush;
    return;
  }

  int worldRank = 0;
  BOOST_REQUIRE_EQUAL(MPI_Comm_rank(MPI_COMM_WORLD, &worldRank), MPI_SUCCESS);
  schnek::MpiContextImpl mpiContext(MPI_COMM_WORLD);

  using GridType1D = schnek::Grid<double, 1, DeviceGridStorage>;
  using RangeType1D = schnek::Range<ptrdiff_t, 1>;
  using IndexType1D = schnek::Array<ptrdiff_t, 1>;
  using DomainType1D = schnek::Range<double, 1>;

  const RangeType1D globalRange1D(IndexType1D(0), IndexType1D(11));
  const DomainType1D globalDomain1D(schnek::Array<double, 1>(0.0), schnek::Array<double, 1>(1.0));
  schnek::MpiCartesianDomainDecomposition<1> decomposition1D(mpiContext);
  decomposition1D.setGlobalRange(globalRange1D);
  decomposition1D.setGlobalDomain(globalDomain1D);
  decomposition1D.init();

  schnek::GridFactory<GridType1D> factory1D;
  const schnek::GridRegistration registration1D = decomposition1D.registerField(factory1D);
  GridType1D *grid1D = nullptr;
  decomposition1D.getGridContext({registration1D}).forEach(
      [&](const RangeType1D &, GridType1D &localGrid) { grid1D = &localGrid; }
  );
  BOOST_REQUIRE(grid1D != nullptr);
  fillDeviceGrid<1>(*grid1D, [](const auto &pos) { return encodeIndex<1>(pos); });

  schnek::Grid<double, 1> weights1D(IndexType1D(0), IndexType1D(11));
  for (ptrdiff_t i = 0; i <= 11; ++i) {
    weights1D(i) = i <= 7 ? 1.0 : 3.0;
  }
  decomposition1D.setGlobalWeights(weights1D);
  decomposition1D.balanceLoad();

  const ptrdiff_t expectedLo1D = worldRank == 0 ? 0 : 8;
  const ptrdiff_t expectedHi1D = worldRank == 0 ? 7 : 11;
  int gridCallCount = 0;
  decomposition1D.getGridContext({registration1D}).forEach(
      [&](const RangeType1D &range, GridType1D &localGrid) {
        ++gridCallCount;
        BOOST_CHECK_EQUAL(range.getLo(0), expectedLo1D);
        BOOST_CHECK_EQUAL(range.getHi(0), expectedHi1D);
        auto mirror = copyToHost(localGrid);
        for (auto pos : range) {
          BOOST_CHECK_EQUAL(hostValue<1>(mirror, localGrid.getLo(), pos), encodeIndex<1>(pos));
        }
      }
  );
  BOOST_CHECK_EQUAL(gridCallCount, 1);

  using FieldType2D = schnek::Field<double, 2, DeviceGridStorage>;
  using RangeType2D = schnek::Range<ptrdiff_t, 2>;
  using IndexType2D = schnek::Array<ptrdiff_t, 2>;
  using DomainType2D = schnek::Range<double, 2>;

  const IndexType2D globalLo2D(0, 0);
  const IndexType2D globalHi2D(11, 3);
  const RangeType2D globalRange2D(globalLo2D, globalHi2D);
  const DomainType2D globalDomain2D(
      schnek::Array<double, 2>(0.0, 0.0), schnek::Array<double, 2>(1.0, 1.0)
  );
  schnek::MpiCartesianDomainDecomposition<2> decomposition2D(mpiContext);
  decomposition2D.setGlobalRange(globalRange2D);
  decomposition2D.setGlobalDomain(globalDomain2D);
  decomposition2D.init();

  typename FieldType2D::StaggerType noStagger(false);
  schnek::GridFactory<FieldType2D> factory2D(noStagger, 1);
  const schnek::GridRegistration registration2D = decomposition2D.registerField(factory2D);
  FieldType2D *field2D = nullptr;
  decomposition2D.getGridContext({registration2D}).forEach(
      [&](const RangeType2D &, FieldType2D &localField) { field2D = &localField; }
  );
  BOOST_REQUIRE(field2D != nullptr);
  const ptrdiff_t initialLo2D = worldRank * 6;
  const ptrdiff_t initialHi2D = initialLo2D + 5;
  fillDeviceGrid<2>(*field2D, [&](const auto &pos) {
    const bool inner = pos[0] >= initialLo2D && pos[0] <= initialHi2D && pos[1] >= 0 && pos[1] <= 3;
    return inner ? encodeIndex<2>(pos) : -1.0;
  });

  schnek::Grid<double, 2> weights2D(globalLo2D, globalHi2D);
  for (ptrdiff_t i = 0; i <= 11; ++i) {
    for (ptrdiff_t j = 0; j <= 3; ++j) {
      weights2D(i, j) = i <= 7 ? 1.0 : 3.0;
    }
  }
  decomposition2D.setGlobalWeights(weights2D);
  decomposition2D.balanceLoad();

  const ptrdiff_t expectedLo2D = worldRank == 0 ? 0 : 8;
  const ptrdiff_t expectedHi2D = worldRank == 0 ? 7 : 11;
  int fieldCallCount = 0;
  decomposition2D.getGridContext({registration2D}).forEach(
      [&](const RangeType2D &range, FieldType2D &localField) {
        ++fieldCallCount;
        BOOST_CHECK_EQUAL(range.getLo(0), expectedLo2D);
        BOOST_CHECK_EQUAL(range.getHi(0), expectedHi2D);
        BOOST_CHECK_EQUAL(range.getLo(1), 0);
        BOOST_CHECK_EQUAL(range.getHi(1), 3);
        auto mirror = copyToHost(localField);
        for (auto pos : range) {
          BOOST_CHECK_EQUAL(hostValue<2>(mirror, localField.getLo(), pos), encodeIndex<2>(pos));
        }
      }
  );
  BOOST_CHECK_EQUAL(fieldCallCount, 1);
}
#endif

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

#endif  // SCHNEK_HAVE_KOKKOS && SCHNEK_HAVE_MPI
