#ifndef SCHNEK_DECOMPOSITION_DETAIL_WEIGHTED_PARTITION_HPP_
#define SCHNEK_DECOMPOSITION_DETAIL_WEIGHTED_PARTITION_HPP_

#include <cmath>
#include <cstddef>
#include <grid/grid.hpp>
#include <string>
#include <vector>

namespace schnek {
  namespace internal {

    /**
     * Compute process boundaries from an unnormalised cumulative-weight grid.
     * The grid includes a zero-valued sentinel at its lower index; each later
     * index is the cumulative weight through one coarse cell. Cuts must leave
     * at least one coarse cell in every process range. A cut after the first or
     * before the last coarse cell is valid; a cut at either outer edge is not.
     */
    inline bool computeWeightedCuts(
        const Grid<double, 1> &cumulativeWeights,
        ptrdiff_t resolution,
        ptrdiff_t glo,
        ptrdiff_t ghi,
        int nProcs,
        std::vector<ptrdiff_t> &cuts,
        std::string &error
    ) {
      cuts.clear();
      error.clear();

      const ptrdiff_t sentinel = cumulativeWeights.getLo(0);
      const ptrdiff_t lastBin = cumulativeWeights.getHi(0);
      const ptrdiff_t nBins = lastBin - sentinel;
      const ptrdiff_t globalCells = ghi - glo + 1;

      if (nProcs <= 0) {
        error = "process count must be positive";
        return false;
      }
      if (resolution <= 0 || globalCells <= 0 || nBins <= 0) {
        error = "weight grid and global range must be non-empty with positive resolution";
        return false;
      }
      if (globalCells < nProcs || nBins < nProcs) {
        error = "more processes than available coarse weight cells";
        return false;
      }

      double totalWeight = cumulativeWeights(lastBin);
      if (!std::isfinite(totalWeight) || !(totalWeight > 0.0)) {
        error = "total weight must be finite and positive";
        return false;
      }

      double previousWeight = cumulativeWeights(sentinel);
      if (!std::isfinite(previousWeight) || previousWeight != 0.0) {
        error = "cumulative weights must begin with a finite zero sentinel";
        return false;
      }
      for (ptrdiff_t i = sentinel + 1; i <= lastBin; ++i) {
        const double weight = cumulativeWeights(i);
        if (!std::isfinite(weight) || weight < previousWeight) {
          error = "cumulative weights must be finite and non-decreasing";
          return false;
        }
        previousWeight = weight;
      }

      cuts.reserve(static_cast<size_t>(nProcs - 1));
      ptrdiff_t previousOffset = 0;
      for (int cutIndex = 1; cutIndex < nProcs; ++cutIndex) {
        const double target = totalWeight * (double(cutIndex) / double(nProcs));

        // Match findInsertIndex's upper-bound behavior without narrowing grid
        // coordinates to int: find the last cumulative entry <= target.
        ptrdiff_t lo = sentinel;
        ptrdiff_t hi = lastBin;
        while (lo <= hi) {
          const ptrdiff_t mid = lo + (hi - lo) / 2;
          if (cumulativeWeights(mid) <= target) {
            lo = mid + 1;
          } else {
            hi = mid - 1;
          }
        }

        const ptrdiff_t offset = hi - sentinel;
        if (offset <= previousOffset || offset >= nBins - (nProcs - cutIndex - 1)) {
          error = "weighted cut would leave a process with no coarse weight cell";
          cuts.clear();
          return false;
        }

        const ptrdiff_t cut = glo + offset * resolution;
        if (cut <= glo || cut > ghi) {
          error = "weighted cut falls outside the global range";
          cuts.clear();
          return false;
        }

        cuts.push_back(cut);
        previousOffset = offset;
      }

      return true;
    }

  }  // namespace internal
}  // namespace schnek

#endif
