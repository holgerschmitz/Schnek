/*
 * particle_binning.hpp
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
#ifndef SCHNEK_DECOMPOSITION_DETAIL_PARTICLE_BINNING_HPP_
#define SCHNEK_DECOMPOSITION_DETAIL_PARTICLE_BINNING_HPP_

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <vector>

#include "../../grid/array.hpp"
#include "../../grid/range.hpp"
#include "../../util/exceptions.hpp"

namespace schnek {

  /**
   * @brief Maps continuous positions in global cell units to bins of rectangular tiles.
   *
   * The region is the inclusive range of global cell indices `[lo, hi]` covered by the bins.
   * It is divided into tiles of `tileCells` cells per dimension. When the region size is not
   * a multiple of the tile size, the last tile along a dimension is smaller.
   *
   * A position belongs to the cell `floor(x)`. Positions outside the region are assigned to the
   * nearest boundary tile, so every position maps to a valid bin. A NaN coordinate maps to the
   * first tile along its dimension.
   *
   * Bins are numbered in row-major order, with the last dimension varying fastest.
   *
   * @tparam rank Number of spatial dimensions.
   */
  template<std::size_t rank>
  class CellBinning {
      static_assert(rank > 0, "CellBinning requires at least one dimension");

    public:
      /**
       * @param region Inclusive range of global cell indices covered by the bins.
       * @param tileCells Tile size in cells for each dimension.
       *
       * @throws ScheckException if the region is empty or a tile size is not positive.
       */
      CellBinning(const Range<std::ptrdiff_t, rank> &region, const Array<std::ptrdiff_t, rank> &tileCells) {
        binCount_ = 1;
        for (std::size_t d = 0; d < rank; ++d) {
          origin_[d] = region.getLo(d);
          cells_[d] = region.getHi(d) - region.getLo(d) + 1;
          tile_[d] = tileCells[d];
          SCHNEK_ASSERT(cells_[d] > 0, "CellBinning region is empty in dimension " << d);
          SCHNEK_ASSERT(tile_[d] > 0, "CellBinning tile size must be positive in dimension " << d);
          tiles_[d] = (cells_[d] + tile_[d] - 1) / tile_[d];
          binCount_ *= static_cast<std::size_t>(tiles_[d]);
        }
        stride_[rank - 1] = 1;
        for (std::size_t d = rank - 1; d > 0; --d) {
          stride_[d - 1] = stride_[d] * static_cast<std::size_t>(tiles_[d]);
        }
      }

      /// Total number of bins.
      std::size_t binCount() const { return binCount_; }

      /// Number of tiles along dimension `dim`.
      std::ptrdiff_t tilesPerDimension(std::size_t dim) const { return tiles_[dim]; }

      /// Bin containing the position. `Position` must be indexable by `std::size_t`.
      template<typename Position>
      std::size_t bin(const Position &position) const {
        std::size_t key = 0;
        for (std::size_t d = 0; d < rank; ++d) {
          key += tileIndex(static_cast<double>(position[d]), d) * stride_[d];
        }
        return key;
      }

      /**
       * @brief Bins that can hold a cell within `haloCells` cells of a cell in bin `bin`.
       *
       * The result contains `bin` itself and every bin reachable by a stencil of that width,
       * clipped to the region. Its size is at most `(2 * ceil(haloCells / tile) + 1)^rank`.
       *
       * @throws ScheckException if `bin` is out of range or `haloCells` is negative.
       */
      std::vector<std::size_t> neighbourhood(std::size_t bin, std::ptrdiff_t haloCells) const {
        SCHNEK_ASSERT(bin < binCount_, "Bin " << bin << " is out of range for " << binCount_ << " bins");
        SCHNEK_ASSERT(haloCells >= 0, "Neighbourhood halo must not be negative");

        std::array<std::ptrdiff_t, rank> lo{};
        std::array<std::ptrdiff_t, rank> hi{};
        std::array<std::ptrdiff_t, rank> cur{};
        for (std::size_t d = 0; d < rank; ++d) {
          const auto tile = static_cast<std::ptrdiff_t>((bin / stride_[d]) % static_cast<std::size_t>(tiles_[d]));
          const std::ptrdiff_t reach = (haloCells + tile_[d] - 1) / tile_[d];
          lo[d] = std::max<std::ptrdiff_t>(0, tile - reach);
          hi[d] = std::min<std::ptrdiff_t>(tiles_[d] - 1, tile + reach);
          cur[d] = lo[d];
        }

        std::vector<std::size_t> result;
        while (true) {
          std::size_t key = 0;
          for (std::size_t d = 0; d < rank; ++d) {
            key += static_cast<std::size_t>(cur[d]) * stride_[d];
          }
          result.push_back(key);

          std::size_t d = rank;
          while (d > 0 && cur[d - 1] == hi[d - 1]) {
            cur[d - 1] = lo[d - 1];
            --d;
          }
          if (d == 0) {
            break;
          }
          ++cur[d - 1];
        }
        return result;
      }

    private:
      /// Tile index along dimension `d` of the cell containing coordinate `x`, clamped to the region.
      std::size_t tileIndex(double x, std::size_t d) const {
        double offset = std::floor(x) - static_cast<double>(origin_[d]);
        if (!(offset >= 0.0)) {
          offset = 0.0;
        }
        const auto top = static_cast<double>(cells_[d] - 1);
        if (offset > top) {
          offset = top;
        }
        return static_cast<std::size_t>(static_cast<std::ptrdiff_t>(offset) / tile_[d]);
      }

      std::array<std::ptrdiff_t, rank> origin_{};
      std::array<std::ptrdiff_t, rank> cells_{};
      std::array<std::ptrdiff_t, rank> tile_{};
      std::array<std::ptrdiff_t, rank> tiles_{};
      std::array<std::size_t, rank> stride_{};
      std::size_t binCount_ = 0;
  };

}  // namespace schnek

#endif /* SCHNEK_DECOMPOSITION_DETAIL_PARTICLE_BINNING_HPP_ */
