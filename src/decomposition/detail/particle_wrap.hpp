/*
 * particle_wrap.hpp
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
#ifndef SCHNEK_DECOMPOSITION_DETAIL_PARTICLE_WRAP_HPP_
#define SCHNEK_DECOMPOSITION_DETAIL_PARTICLE_WRAP_HPP_

#include <cmath>
#include <cstddef>
#include <cstdint>

#include "../../macros.hpp"

namespace schnek {
  namespace internal {

    SCHNEK_FUNCTION inline bool isFiniteCell(double value) {
      const double lowerBound = static_cast<double>(PTRDIFF_MIN);
      const double upperBoundExclusive = -lowerBound;
      return value >= lowerBound && value < upperBoundExclusive;
    }

    /// Converts a coordinate already validated by isFiniteCell.
    SCHNEK_FUNCTION inline ptrdiff_t owningCell(double position) {
      return static_cast<ptrdiff_t>(std::floor(position));
    }

    template<class Particle, class Accessor, std::size_t rank>
    struct FinitePosition {
        Accessor accessor;

        SCHNEK_FUNCTION bool operator()(const Particle &particle) const {
          for (std::size_t dim = 0; dim < rank; ++dim) {
            if (!isFiniteCell(accessor(particle)[dim])) {
              return false;
            }
          }
          return true;
        }
    };

    template<class Particle, class Accessor>
    struct FinitePositionValue {
        Accessor accessor;
        std::size_t dim;

        SCHNEK_FUNCTION bool operator()(const Particle &particle) const {
          return isFiniteCell(accessor(particle)[dim]);
        }
    };

    template<class Particle, class Accessor>
    struct WrapDim {
        Accessor accessor;
        std::size_t dim;
        double glo;
        double ghi;
        double length;

        SCHNEK_FUNCTION void operator()(Particle &particle) const {
          const double periods = std::floor((accessor(particle)[dim] - glo) / length);
          if (periods != 0.0) {
            accessor.shift(particle, dim, -periods * length);
          }
        }
    };

    template<class Particle, class Accessor>
    struct PositionCellInRange {
        Accessor accessor;
        std::size_t dim;
        ptrdiff_t lo;
        ptrdiff_t hi;

        SCHNEK_FUNCTION bool operator()(const Particle &particle) const {
          const double position = accessor(particle)[dim];
          if (!isFiniteCell(position)) {
            return false;
          }
          const ptrdiff_t cell = owningCell(position);
          return cell >= lo && cell <= hi;
        }
    };

    template<class Particle, class Accessor>
    struct ValidWrapPeriods {
        Accessor accessor;
        std::size_t dim;
        double glo;
        double length;

        SCHNEK_FUNCTION bool operator()(const Particle &particle) const {
          const double position = accessor(particle)[dim];
          if (!isFiniteCell(position)) {
            return false;
          }

          const double periods = std::floor((position - glo) / length);
          constexpr double maxPeriods = 1073741824.0;
          return periods > -maxPeriods && periods < maxPeriods;
        }
    };

  }  // namespace internal
}  // namespace schnek

#endif  // SCHNEK_DECOMPOSITION_DETAIL_PARTICLE_WRAP_HPP_
