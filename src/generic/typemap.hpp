/*
 * typemap.hpp
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

#include <cstddef>

namespace schnek {
    namespace generic {

        template<size_t k, typename T>
        struct TypeMapEntry {
            static constexpr size_t key = k;
            typedef T type;
        };

        template<size_t key, typename... /* TypeMapEntries */ Types>
        struct Lookup {};

        template<size_t k, typename T, typename... /* TypeMapEntries */ Types>
        struct Lookup<k, TypeMapEntry<k, T>, Types...>
        {
            typedef T type;
        };

        template<size_t ka, size_t kb, typename T, typename... /* TypeMapEntries */ Types>
        struct Lookup<ka, TypeMapEntry<kb, T>, Types...>
        {
            typedef typename Lookup<ka, Types...>::type type;
        };

        template<typename... /* TypeMapEntries */ Types>
        struct TypeMap : public Types... {
            static constexpr int size = sizeof...(Types);
            
            template<size_t key>
            struct get
            {
                typedef typename Lookup<key, Types...>::type type;
            };

            template<size_t k, typename T>
            struct put
            {
                typedef TypeMap<TypeMapEntry<k, T>, Types...> type;
            };
        };
    }
}

#endif // SCHNEK_GENERIC_TYPEMAP_HPP_