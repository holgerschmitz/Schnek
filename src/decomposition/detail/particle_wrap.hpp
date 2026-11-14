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

#include "../../macros.hpp"

namespace schnek {
  namespace internal {

    template<class Particle, class Accessor>
    struct WrapDim {
        Accessor accessor;
        std::size_t dim;
        double glo;
        double ghi;
        double length;

        SCHNEK_FUNCTION void operator()(Particle &particle) const {
          const double cell = std::floor(accessor(particle)[dim]);
          if (cell < glo) {
            accessor.shift(particle, dim, length);
          } else if (cell > ghi) {
            accessor.shift(particle, dim, -length);
          }
        }
    };

  }  // namespace internal
}  // namespace schnek

#endif  // SCHNEK_DECOMPOSITION_DETAIL_PARTICLE_WRAP_HPP_
