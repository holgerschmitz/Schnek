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
     * host-accessible storage policy. A future Kokkos specialisation (selected
     * via the `Enable` parameter on the concrete `GridType`) provides the
     * device-aware path without changing any of the calling code.
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

  }  // namespace detail
}  // namespace schnek

#endif  // SCHNEK_DECOMPOSITION_DETAIL_GRID_TRANSFER_HPP_
