/*
 * redistribution.hpp
 *
 * Created on: 19 Feb 2026
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

#ifndef SCHNEK_DECOMPOSITION_DETAIL_REDISTRIBUTION_HPP_
#define SCHNEK_DECOMPOSITION_DETAIL_REDISTRIBUTION_HPP_

#include <cstddef>
#include <vector>

#include "../../grid/range.hpp"

namespace schnek {

  /**
   * @brief Describes a data transfer block for grid redistribution.
   *
   * Each block represents a contiguous sub-range of grid data that needs
   * to be transferred between two MPI ranks during load rebalancing.
   * This struct is generic and can be used by any domain decomposition class.
   *
   * @tparam rank The dimensionality of the domain.
   * @tparam CheckingPolicy The array checking policy.
   */
  template<size_t rank, template<size_t> class CheckingPolicy = ArrayNoArgCheck>
  struct TransferBlock {
      using RangeType = Range<ptrdiff_t, rank, CheckingPolicy>;

      /// The MPI rank of the communication partner
      int mpiRank;

      /// The sub-range of grid data to transfer
      RangeType range;
  };

  /**
   * @brief Compute the intersection of two ranges.
   *
   * Returns true if the ranges overlap, and stores the intersection in the
   * output parameter. Returns false if there is no overlap.
   *
   * @tparam rank The dimensionality of the ranges.
   * @tparam CheckingPolicy The array checking policy.
   * @param a The first range.
   * @param b The second range.
   * @param intersection Output: the intersection range (only valid if return is true).
   * @return true if the ranges overlap.
   */
  template<size_t rank, template<size_t> class CheckingPolicy = ArrayNoArgCheck>
  bool rangeIntersection(
      const Range<ptrdiff_t, rank, CheckingPolicy> &a,
      const Range<ptrdiff_t, rank, CheckingPolicy> &b,
      Range<ptrdiff_t, rank, CheckingPolicy> &intersection
  ) {
    for (size_t d = 0; d < rank; ++d) {
      ptrdiff_t lo = std::max(a.getLo()[d], b.getLo()[d]);
      ptrdiff_t hi = std::min(a.getHi()[d], b.getHi()[d]);
      if (lo > hi) {
        return false;
      }
      intersection.getLo()[d] = lo;
      intersection.getHi()[d] = hi;
    }
    return true;
  }

  /**
   * @brief Compute the volume (number of elements) of a range.
   *
   * @tparam rank The dimensionality of the range.
   * @tparam CheckingPolicy The array checking policy.
   * @param range The range.
   * @return The total number of grid points in the range.
   */
  template<size_t rank, template<size_t> class CheckingPolicy = ArrayNoArgCheck>
  size_t rangeVolume(const Range<ptrdiff_t, rank, CheckingPolicy> &range) {
    size_t volume = 1;
    for (size_t d = 0; d < rank; ++d) {
      ptrdiff_t extent = range.getHi()[d] - range.getLo()[d] + 1;
      if (extent <= 0) {
        return 0;
      }
      volume *= static_cast<size_t>(extent);
    }
    return volume;
  }

  /**
   * @brief Compute transfer plans for Cartesian domain redistribution.
   *
   * Given old and new per-dimension per-process ranges, the current process
   * coordinates, and a function to convert coordinates to MPI rank, this
   * computes the set of send and receive blocks needed for redistribution.
   *
   * The send plan contains blocks describing data that the current process
   * needs to send to other processes (or itself). The receive plan contains
   * blocks describing data the current process needs to receive.
   *
   * Blocks where the current process is both sender and receiver (local copies)
   * are identified by having mpiRank == myMpiRank.
   *
   * @tparam rank The dimensionality.
   * @tparam CheckingPolicy The array checking policy.
   * @tparam ProcRanges Type of the per-dimension per-process range arrays.
   * @tparam CoordToRankFunc Callable: (const Array<ptrdiff_t,rank>&) -> int
   * @param oldRanges Old per-dimension per-process ranges.
   * @param newRanges New per-dimension per-process ranges.
   * @param dims Dimensions of the Cartesian process grid.
   * @param myCoord Cartesian coordinates of this process.
   * @param myMpiRank MPI rank of the current process.
   * @param coordToRank Function to convert Cartesian coordinates to MPI rank.
   * @param sendPlan Output: transfer blocks to send.
   * @param recvPlan Output: transfer blocks to receive.
   */
  template<size_t rank, template<size_t> class CheckingPolicy, typename ProcRanges, typename CoordToRankFunc>
  void computeCartesianTransferPlan(
      const ProcRanges &oldRanges,
      const ProcRanges &newRanges,
      const Array<ptrdiff_t, rank> &dims,
      const Array<ptrdiff_t, rank> &myCoord,
      int myMpiRank,
      CoordToRankFunc coordToRank,
      std::vector<TransferBlock<rank, CheckingPolicy>> &sendPlan,
      std::vector<TransferBlock<rank, CheckingPolicy>> &recvPlan
  ) {
    using RangeType = Range<ptrdiff_t, rank, CheckingPolicy>;

    sendPlan.clear();
    recvPlan.clear();

    // Compute my old local range
    RangeType myOldRange;
    for (size_t d = 0; d < rank; ++d) {
      myOldRange.getLo()[d] = oldRanges[d](myCoord[d]).getLo()[0];
      myOldRange.getHi()[d] = oldRanges[d](myCoord[d]).getHi()[0];
    }

    // Compute my new local range
    RangeType myNewRange;
    for (size_t d = 0; d < rank; ++d) {
      myNewRange.getLo()[d] = newRanges[d](myCoord[d]).getLo()[0];
      myNewRange.getHi()[d] = newRanges[d](myCoord[d]).getHi()[0];
    }

    // For each dimension, find which process coordinates in the new layout
    // overlap with my old range (for sends)
    std::vector<std::vector<ptrdiff_t>> sendOverlapCoords(rank);
    for (size_t d = 0; d < rank; ++d) {
      for (ptrdiff_t c = 0; c < dims[d]; ++c) {
        ptrdiff_t newLo = newRanges[d](c).getLo()[0];
        ptrdiff_t newHi = newRanges[d](c).getHi()[0];
        ptrdiff_t oldLo = myOldRange.getLo()[d];
        ptrdiff_t oldHi = myOldRange.getHi()[d];
        if (newLo <= oldHi && newHi >= oldLo) {
          sendOverlapCoords[d].push_back(c);
        }
      }
    }

    // For each dimension, find which process coordinates in the old layout
    // overlap with my new range (for receives)
    std::vector<std::vector<ptrdiff_t>> recvOverlapCoords(rank);
    for (size_t d = 0; d < rank; ++d) {
      for (ptrdiff_t c = 0; c < dims[d]; ++c) {
        ptrdiff_t oldLo = oldRanges[d](c).getLo()[0];
        ptrdiff_t oldHi = oldRanges[d](c).getHi()[0];
        ptrdiff_t newLo = myNewRange.getLo()[d];
        ptrdiff_t newHi = myNewRange.getHi()[d];
        if (oldLo <= newHi && oldHi >= newLo) {
          recvOverlapCoords[d].push_back(c);
        }
      }
    }

    // Enumerate send blocks: iterate over all combinations of overlapping new coords
    // Use recursive enumeration over dimensions
    auto enumerateBlocks = [&](const std::vector<std::vector<ptrdiff_t>> &overlapCoords, const RangeType &myRange,
                               auto getRangeForCoord, std::vector<TransferBlock<rank, CheckingPolicy>> &plan) {
      Array<ptrdiff_t, rank> coord;
      std::vector<size_t> indices(rank, 0);

      // Compute total number of combinations
      size_t totalCombinations = 1;
      for (size_t d = 0; d < rank; ++d) {
        if (overlapCoords[d].empty()) {
          return;
        }
        totalCombinations *= overlapCoords[d].size();
      }

      for (size_t combo = 0; combo < totalCombinations; ++combo) {
        // Decode combination index into per-dimension indices
        size_t remaining = combo;
        for (size_t d = rank; d > 0; --d) {
          indices[d - 1] = remaining % overlapCoords[d - 1].size();
          remaining /= overlapCoords[d - 1].size();
        }

        // Build the coordinate and the partner range
        RangeType partnerRange;
        for (size_t d = 0; d < rank; ++d) {
          coord[d] = overlapCoords[d][indices[d]];
          auto dimRange = getRangeForCoord(d, coord[d]);
          partnerRange.getLo()[d] = dimRange.getLo()[0];
          partnerRange.getHi()[d] = dimRange.getHi()[0];
        }

        // Compute intersection of myRange with partnerRange
        RangeType intersection;
        if (rangeIntersection(myRange, partnerRange, intersection)) {
          int partnerRank = coordToRank(coord);
          plan.push_back({partnerRank, intersection});
        }
      }
    };

    // Send plan: my old range intersected with each destination's new range
    enumerateBlocks(
        sendOverlapCoords, myOldRange, [&](size_t d, ptrdiff_t c) -> decltype(auto) { return newRanges[d](c); },
        sendPlan
    );

    // Receive plan: my new range intersected with each source's old range
    enumerateBlocks(
        recvOverlapCoords, myNewRange, [&](size_t d, ptrdiff_t c) -> decltype(auto) { return oldRanges[d](c); },
        recvPlan
    );

    (void)myMpiRank;
  }

}  // namespace schnek

#endif  // SCHNEK_DECOMPOSITION_DETAIL_REDISTRIBUTION_HPP_
