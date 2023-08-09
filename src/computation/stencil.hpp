/*
 * stencil.hpp
 *
 * Created on: 25 Jun 2023
 * Author: hschmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2023 Holger Schmitz
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

#ifndef SCHNEK_COMPUTATION_STENCIL_HPP_
#define SCHNEK_COMPUTATION_STENCIL_HPP_

#include <cstddef>
#include <array>

namespace schnek {
    namespace computation {
        template<size_t rank, int... Values>
        struct StaticIndex {
            static constexpr size_t size = rank;
            static constexpr std::array<int, rank> values = {Values...};
        };

        /**
         * A stencil
         * 
         * This is a stencil. It is a list of values that can be accessed by index.
         * 
         * @tparam Values The values in the stencil
         */
        template<size_t rank, typename... Values>
        struct Stencil {
            /**
             * The number of values in the stencil
             */
            static constexpr std::size_t size = sizeof...(Values);
                        
            /**
             * Get the value at the given index
             * 
             * @tparam index The index of the value to get
             */
            template<size_t index>
            static constexpr std::array<int, rank> value() {
                return TypeList<int, Values...>::get<;
            }
        };
    }
}

#endif // SCHNEK_COMPUTATION_STENCIL_HPP_