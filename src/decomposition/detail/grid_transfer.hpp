/*
 * grid_transfer.hpp
 *
 * Created on: 30 Jun 2026
 * Author: Holger Schmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2026 Holger Schmitz
 *
 * This file is part of Schnek.
 *
 * Schnek is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Schnek is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Schnek.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef SCHNEK_DECOMPOSITION_DETAIL_GRID_TRANSFER_HPP_
#define SCHNEK_DECOMPOSITION_DETAIL_GRID_TRANSFER_HPP_

#include <cstddef>
#include <type_traits>

#include "config.hpp"
#include "../../grid/gridstorage/grid-storage-concept.hpp"

#ifdef SCHNEK_HAVE_KOKKOS
#include <Kokkos_Core.hpp>

#include "../../grid/gridstorage/kokkos-storage.hpp"
#include "../../grid/iteration/kokkos-iteration.hpp"
#endif

namespace schnek {
  namespace detail {

    /**
     * @brief Customisation point for moving grid elements to and from a
     *        contiguous buffer during domain-decomposition communication.
     *
     * The domain decomposition packs sub-ranges of a registered grid into a
     * contiguous buffer before an MPI transfer and unpacks them on the
     * receiving side. The way this is done depends on where the grid's data
     * lives: for host-accessible storage a plain element loop is sufficient,
     * whereas device-resident (Kokkos) storage needs an execution-space-aware
     * path. `GridTransfer` isolates that decision in a single trait so the MPI
     * orchestration code stays backend agnostic.
     *
     * The primary template implements the host element-loop path used by every
     * host-accessible storage policy (including host-accessible Kokkos memory
     * spaces). The Kokkos specialisation below provides a device-aware path for
     * memory spaces that are not accessible from host code, without changing any
     * of the calling code.
     *
     * @tparam GridType The concrete grid type whose elements are transferred.
     * @tparam Enable   SFINAE hook used by specialisations to select a backend.
     */
    template<class GridType, typename Enable = void>
    struct GridTransfer {
        using ValueType = typename GridType::value_type;
        using RangeType = typename GridType::RangeType;

        /**
         * @brief Copy the elements of `range` out of `grid` into the contiguous
         *        buffer `dst`.
         *
         * @param grid  The grid to read from.
         * @param range The sub-range of grid indices to copy.
         * @param dst   Destination buffer, pre-allocated to the range volume.
         */
        static void pack(const GridType &grid, const RangeType &range, ValueType *dst) {
          std::size_t idx = 0;
          for (auto it = range.begin(); it != range.end(); ++it) {
            dst[idx++] = grid[*it];
          }
        }

        /**
         * @brief Scatter the contiguous buffer `src` back into `grid` over
         *        `range`.
         *
         * @param grid  The grid to write to.
         * @param range The sub-range of grid indices to fill.
         * @param src   Source buffer, holding at least the range volume.
         */
        static void unpack(GridType &grid, const RangeType &range, const ValueType *src) {
          std::size_t idx = 0;
          for (auto it = range.begin(); it != range.end(); ++it) {
            grid[*it] = src[idx++];
          }
        }

        /**
         * @brief Local (no MPI) block copy of `range` from `src` to `dst`.
         *
         * @param src   The grid to read from.
         * @param dst   The grid to write to.
         * @param range The sub-range of grid indices to copy.
         */
        static void copy(const GridType &src, GridType &dst, const RangeType &range) {
          for (auto it = range.begin(); it != range.end(); ++it) {
            dst[*it] = src[*it];
          }
        }
    };

    namespace internal {
      /**
       * @brief Detects whether a storage policy is a device-resident Kokkos
       *        storage, i.e. it exposes a Kokkos view and its memory space is
       *        not accessible from host code.
       *
       * The `host_accessible` member is only inspected for storage policies that
       * actually provide `getKokkosView()`, so the trait is safe to instantiate
       * for every storage policy.
       */
      template<
          class Storage,
          bool = concepts::GridStorageConceptCondition<Storage>::has_get_kokkos_view_method>
      struct is_device_kokkos_storage : std::false_type {};

      template<class Storage>
      struct is_device_kokkos_storage<Storage, true>
          : std::integral_constant<bool, !Storage::host_accessible> {};
    }  // namespace internal

    /// True if `GridType` is backed by a Kokkos storage in a non-host-accessible memory space.
    template<class GridType>
    struct is_device_kokkos_grid
        : internal::is_device_kokkos_storage<typename GridType::storage_type> {};

#ifdef SCHNEK_HAVE_KOKKOS

    /**
     * @brief Device-aware `GridTransfer` specialisation for Kokkos grids whose
     *        memory space is not accessible from host code.
     *
     * Pack and unpack gather/scatter the requested sub-range on the device into
     * a contiguous device staging buffer using `RangeKokkosIterationPolicy`, then
     * move the staging buffer to/from the host MPI buffer with a single
     * `Kokkos::deep_copy`. The local `copy` is a device-to-device sweep.
     *
     * The element ordering inside the contiguous buffer is row-major over the
     * range extents (the last dimension varies fastest), matching the order of
     * the host `Range` iterator used by the primary template, so the two paths
     * are interchangeable across the MPI boundary.
     */
    template<class GridType>
    struct GridTransfer<GridType, std::enable_if_t<is_device_kokkos_grid<GridType>::value>> {
        using ValueType = typename GridType::value_type;
        using RangeType = typename GridType::RangeType;
        using LimitType = typename RangeType::LimitType;
        using StorageType = typename GridType::storage_type;
        using ViewType = typename StorageType::ViewType;
        using MemorySpace = typename StorageType::MemorySpace;
        using ExecSpace = typename ViewType::execution_space;

        static constexpr std::size_t Rank = StorageType::rank;

        using StagingView = Kokkos::View<ValueType *, MemorySpace>;
        using HostBuffer = Kokkos::View<ValueType *, Kokkos::HostSpace, Kokkos::MemoryTraits<Kokkos::Unmanaged>>;
        using ConstHostBuffer =
            Kokkos::View<const ValueType *, Kokkos::HostSpace, Kokkos::MemoryTraits<Kokkos::Unmanaged>>;

        using BoundsArray = Kokkos::Array<ptrdiff_t, Rank>;

        static std::size_t rangeVolume(const RangeType &range) {
          std::size_t volume = 1;
          for (std::size_t d = 0; d < Rank; ++d) {
            ptrdiff_t extent = range.getHi()[d] - range.getLo()[d] + 1;
            if (extent <= 0) {
              return 0;
            }
            volume *= static_cast<std::size_t>(extent);
          }
          return volume;
        }

        /// Fill the per-dimension view strides, grid origin, range origin and range extents.
        static void collectBounds(
            const GridType &grid,
            const ViewType &view,
            const RangeType &range,
            BoundsArray &strides,
            BoundsArray &gridLo,
            BoundsArray &rangeLo,
            BoundsArray &extent
        ) {
          for (std::size_t d = 0; d < Rank; ++d) {
            strides[d] = static_cast<ptrdiff_t>(view.stride(d));
            gridLo[d] = grid.getLo()[d];
            rangeLo[d] = range.getLo()[d];
            extent[d] = range.getHi()[d] - range.getLo()[d] + 1;
          }
        }

        static void pack(const GridType &grid, const RangeType &range, ValueType *dst) {
          const std::size_t volume = rangeVolume(range);
          if (volume == 0) {
            return;
          }

          ViewType view = grid.getKokkosView();
          BoundsArray strides, gridLo, rangeLo, extent;
          collectBounds(grid, view, range, strides, gridLo, rangeLo, extent);

          StagingView staging(Kokkos::view_alloc(Kokkos::WithoutInitializing, "schnek:gridtransfer:pack"), volume);

          ValueType *viewData = view.data();
          ValueType *stagingData = staging.data();

          RangeKokkosIterationPolicy<Rank, ExecSpace>::forEach(
              range,
              KOKKOS_LAMBDA(const LimitType &pos) {
                ptrdiff_t offset = 0;
                std::size_t flat = 0;
                for (std::size_t d = 0; d < Rank; ++d) {
                  offset += (pos[d] - gridLo[d]) * strides[d];
                  flat = flat * static_cast<std::size_t>(extent[d]) +
                         static_cast<std::size_t>(pos[d] - rangeLo[d]);
                }
                stagingData[flat] = viewData[offset];
              }
          );
          Kokkos::fence();

          HostBuffer hostBuf(dst, volume);
          Kokkos::deep_copy(hostBuf, staging);
        }

        static void unpack(GridType &grid, const RangeType &range, const ValueType *src) {
          const std::size_t volume = rangeVolume(range);
          if (volume == 0) {
            return;
          }

          ViewType view = grid.getKokkosView();
          BoundsArray strides, gridLo, rangeLo, extent;
          collectBounds(grid, view, range, strides, gridLo, rangeLo, extent);

          StagingView staging(Kokkos::view_alloc(Kokkos::WithoutInitializing, "schnek:gridtransfer:unpack"), volume);

          ConstHostBuffer hostBuf(src, volume);
          Kokkos::deep_copy(staging, hostBuf);

          ValueType *viewData = view.data();
          ValueType *stagingData = staging.data();

          RangeKokkosIterationPolicy<Rank, ExecSpace>::forEach(
              range,
              KOKKOS_LAMBDA(const LimitType &pos) {
                ptrdiff_t offset = 0;
                std::size_t flat = 0;
                for (std::size_t d = 0; d < Rank; ++d) {
                  offset += (pos[d] - gridLo[d]) * strides[d];
                  flat = flat * static_cast<std::size_t>(extent[d]) +
                         static_cast<std::size_t>(pos[d] - rangeLo[d]);
                }
                viewData[offset] = stagingData[flat];
              }
          );
          Kokkos::fence();
        }

        static void copy(const GridType &src, GridType &dst, const RangeType &range) {
          const std::size_t volume = rangeVolume(range);
          if (volume == 0) {
            return;
          }

          ViewType srcView = src.getKokkosView();
          ViewType dstView = dst.getKokkosView();

          BoundsArray srcStrides, srcLo, srcRangeLo, srcExtent;
          BoundsArray dstStrides, dstLo, dstRangeLo, dstExtent;
          collectBounds(src, srcView, range, srcStrides, srcLo, srcRangeLo, srcExtent);
          collectBounds(dst, dstView, range, dstStrides, dstLo, dstRangeLo, dstExtent);

          ValueType *srcData = srcView.data();
          ValueType *dstData = dstView.data();

          RangeKokkosIterationPolicy<Rank, ExecSpace>::forEach(
              range,
              KOKKOS_LAMBDA(const LimitType &pos) {
                ptrdiff_t srcOffset = 0;
                ptrdiff_t dstOffset = 0;
                for (std::size_t d = 0; d < Rank; ++d) {
                  srcOffset += (pos[d] - srcLo[d]) * srcStrides[d];
                  dstOffset += (pos[d] - dstLo[d]) * dstStrides[d];
                }
                dstData[dstOffset] = srcData[srcOffset];
              }
          );
          Kokkos::fence();
        }
    };

#endif  // SCHNEK_HAVE_KOKKOS

  }  // namespace detail
}  // namespace schnek

#endif  // SCHNEK_DECOMPOSITION_DETAIL_GRID_TRANSFER_HPP_
