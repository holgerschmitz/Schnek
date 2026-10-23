/*
 *  test_device_storage.hpp
 *
 *  Test-only grid storage that keeps Kokkos::HostSpace memory but reports
 *  host_accessible = false. The grid transfer code therefore takes the device
 *  path, which can run on a CPU-only Kokkos build (for example Kokkos::Serial).
 *
 *  Limitations:
 *  - Host memory is valid for Kokkos::Serial, so this storage cannot detect
 *    host-pointer dereferences from device code. Tests that use it guard the
 *    logic of the device path, not the memory-space invariant. CUDA tests with
 *    Kokkos::CudaSpace guard the invariant.
 *  - Tests must read grid contents through Kokkos::create_mirror_view_and_copy,
 *    not through the storage's host accessors.
 */

#pragma once

#include <grid/gridstorage/kokkos-storage.hpp>

#include <Kokkos_Core.hpp>

#include <cstddef>

template<class T, size_t rank>
struct HostBackedDeviceStorage : schnek::KokkosGridStorage<T, rank, Kokkos::HostSpace> {
  using schnek::KokkosGridStorage<T, rank, Kokkos::HostSpace>::KokkosGridStorage;

  static constexpr bool host_accessible = false;  // selects the device transfer path
};
