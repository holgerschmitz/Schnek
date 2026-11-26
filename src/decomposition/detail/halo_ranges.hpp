#ifndef SCHNEK_DECOMPOSITION_DETAIL_HALO_RANGES_HPP_
#define SCHNEK_DECOMPOSITION_DETAIL_HALO_RANGES_HPP_

#include <cstddef>
#include <optional>

#include "../../util/volume.hpp"

namespace schnek {
  namespace detail {

    template<class RangeType>
    struct HaloRanges {
        std::ptrdiff_t lowerHalo;
        std::ptrdiff_t upperHalo;
        std::optional<RangeType> loGhostRange;
        std::optional<RangeType> hiGhostRange;
        std::optional<RangeType> loSourceRange;
        std::optional<RangeType> hiSourceRange;
    };

    template<class RangeType, class IndexType>
    HaloRanges<RangeType> haloRanges(
        std::size_t dim,
        const IndexType &gridLo,
        const IndexType &gridHi,
        const IndexType &innerLo,
        const IndexType &innerHi
    ) {
      const std::ptrdiff_t lowerHalo = innerLo[dim] - gridLo[dim];
      const std::ptrdiff_t upperHalo = gridHi[dim] - innerHi[dim];

      std::optional<RangeType> loGhostRange;
      if (lowerHalo > 0) {
        IndexType lo = gridLo;
        IndexType hi = gridHi;
        hi[dim] = gridLo[dim] + lowerHalo - 1;
        loGhostRange = RangeType(lo, hi);
      }

      std::optional<RangeType> hiGhostRange;
      if (upperHalo > 0) {
        IndexType lo = gridLo;
        IndexType hi = gridHi;
        lo[dim] = gridHi[dim] - upperHalo + 1;
        hi[dim] = gridHi[dim];
        hiGhostRange = RangeType(lo, hi);
      }

      std::optional<RangeType> loSourceRange;
      if (upperHalo > 0) {
        IndexType lo = gridLo;
        IndexType hi = gridHi;
        lo[dim] = innerLo[dim];
        hi[dim] = innerLo[dim] + upperHalo - 1;
        loSourceRange = RangeType(lo, hi);
      }

      std::optional<RangeType> hiSourceRange;
      if (lowerHalo > 0) {
        IndexType lo = gridLo;
        IndexType hi = gridHi;
        lo[dim] = innerHi[dim] - lowerHalo + 1;
        hi[dim] = innerHi[dim];
        hiSourceRange = RangeType(lo, hi);
      }

      return {lowerHalo, upperHalo, loGhostRange, hiGhostRange, loSourceRange, hiSourceRange};
    }

    struct HaloRangeCounts {
        std::size_t sendLowerCount;
        std::size_t recvLowerCount;
        std::size_t sendUpperCount;
        std::size_t recvUpperCount;
    };

    template<class RangeType>
    std::size_t optionalRangeVolume(const std::optional<RangeType> &range) {
      return range ? schnek::volume(*range) : 0;
    }

    template<class RangeType>
    HaloRangeCounts haloRangeCounts(const HaloRanges<RangeType> &ranges) {
      return {
          optionalRangeVolume(ranges.hiSourceRange), optionalRangeVolume(ranges.loGhostRange),
          optionalRangeVolume(ranges.loSourceRange), optionalRangeVolume(ranges.hiGhostRange)
      };
    }

  }  // namespace detail
}  // namespace schnek

#endif  // SCHNEK_DECOMPOSITION_DETAIL_HALO_RANGES_HPP_
