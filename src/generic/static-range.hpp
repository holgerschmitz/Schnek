/*
 * static-range.hpp
 *
 * Created on: 31 Oct 2023
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


#ifndef SCHNEK_GENERIC_TYPEMAP_HPP_
#define SCHNEK_GENERIC_TYPEMAP_HPP_

#include "typelist.hpp"

#include <cstddef>

namespace schnek {
    namespace generic {
        template<ptrdiff_t l, ptrdiff_t h>
        struct StaticRange {
            static constexpr ptrdiff_t lo = l;
            static constexpr ptrdiff_t hi = h;
        };

        template<typename... /* StaticRange */ Types>
        struct StaticGhostCells {
            static constexpr int rank = sizeof...(Types);
            
            template<ptrdiff_t lo, ptrdiff_t hi>
            using put = StaticGhostCells<StaticRange<lo, hi>, Types...>;

            template<size_t count, ptrdiff_t lo, ptrdiff_t hi>
            struct repeat {
                typedef typename StaticGhostCells<StaticRange<lo, hi>, typename repeat<count-1, lo, hi>::type> type;
            };

            template<ptrdiff_t lo, ptrdiff_t hi>
            struct repeat<0, lo, hi> {
                typedef typename StaticGhostCells<StaticRange<lo, hi>> type;
            };

            template<size_t rank>
            struct get {
                typedef typename TypeListGet<rank, Types...>::type type;
                static constexpr ptrdiff_t lo = type::lo;
                static constexpr ptrdiff_t hi = type::hi;
            };
        };
        
    }
}

#endif // SCHNEK_GENERIC_TYPEMAP_HPP_