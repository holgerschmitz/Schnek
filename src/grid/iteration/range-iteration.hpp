/*
 * range-iteration.hpp
 *
 * Created on: 1 Dec 2022
 * Author: Holger Schmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2012-2022 Holger Schmitz
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
 */

#ifndef SCHNEK_GRID_ITERATION_RANGEITERATION_HPP_
#define SCHNEK_GRID_ITERATION_RANGEITERATION_HPP_

#include "../../config.hpp"
#include "../array.hpp"

namespace schnek {

    template<
        class RangeType,
        size_t rank
    >
    struct RangeCIterationPolicy {  
    public:
        template<typename Func>
        void forEach(const RangeType& range, Func func)
        {
          typename RangeType::LimitType pos = range.getLo();
          
          do {
            func(pos);
            size_t d = rank;

            while (d>0)
            {
                --d;
                if (++pos[d] > domain.getHi()[d])
                {
                    pos[d] = domain.getLo()[d];
                } else {
                    break;
                }
            }            
        }
    };

} // namespace schnek

#endif // SCHNEK_GRID_ITERATION_RANGEITERATION_HPP_