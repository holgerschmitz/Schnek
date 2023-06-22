/*
 * typelist.hpp
 *
 * Created on: 22 Jun 2023
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


#ifndef SCHNEK_GENERIC_TYPELIST_HPP_
#define SCHNEK_GENERIC_TYPELIST_HPP_

#include <cstddef>

namespace schnek {
    namespace generic {
        template<typename... Types>
        struct TypeList;

        namespace internal {

            template<long n, typename T, typename... Types>
            struct TypeListGet {
                typedef typename TypeListGet<n-1, Types...>::type type;
                static constexpr long value = TypeListGet<n-1, Types...>::value + 1;
            };

            template<typename T, typename... Types>
            struct TypeListGet<0, T, Types...> {
                typedef T type;
                static constexpr long value = 1;
            };
        }

        template<typename... Types>
        struct TypeList {
            static constexpr int size = sizeof...(Types);
            
            template<size_t n>
            struct get
            {
                static_assert(n < size, "Index out of bounds");
                typedef typename internal::TypeListGet<n, Types...>::type type;
            };
            
        };
    }
}

#endif // SCHNEK_GENERIC_TYPELIST_HPP_