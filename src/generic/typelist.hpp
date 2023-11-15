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
#include <tuple>

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
        
        namespace internal {
            template<template <typename T> typename mapper, typename TypeListType, typename... InputTypes>
            struct TypeListMapper {};

            template<template <typename T> typename conditional, typename TypeListType, typename... InputTypes>
            struct TypeListFilter {};
        }

        /**
         * A type list
         * 
         * This is a type list. It is a list of types that can be accessed by index.
         * 
         * @tparam Types The types in the list
         */
        template<typename... Types>
        struct TypeList {
            /**
             * The number of types in the list
             */
            static constexpr int size = sizeof...(Types);
            
            /**
             * Get the type at index n
             * 
             * @tparam n The index
             * @return The type at index n
             */
            template<size_t n>
            struct get
            {
                static_assert(n < size, "Index out of bounds");
                typedef typename internal::TypeListGet<n, Types...>::type type;
            };

            /**
             * @brief Push a type to the back of the type list
             * 
             * @tparam T The type to push
             */
            template<typename T>
            using push_back = TypeList<Types..., T>;

            template<typename T>
            using push_front = TypeList<T, Types...>;

            template<template <typename T> typename mapper>
            using map = typename internal::TypeListMapper<mapper, TypeList<>, Types...>::type;

            template<template <typename T> typename conditional>
            using filter = typename internal::TypeListFilter<conditional, TypeList<>, Types...>::type;

            template<template <typename... T> typename Dest>
            using apply = Dest<Types...>;
        };

        namespace internal {
            template<template <typename T> typename mapper, typename TypeListType, typename Head, typename... InputTypes>
            struct TypeListMapper<mapper, TypeListType, Head, InputTypes...> {
                
                typedef typename TypeListMapper<
                    mapper, 
                    typename TypeListType::push_back<typename mapper<Head>::type>,
                    InputTypes...
                >::type type;
            };

            template<template <typename T> typename mapper, typename TypeListType>
            struct TypeListMapper<mapper, TypeListType> {
                typedef TypeListType type;
            };

            template<bool condition, typename TypeListType, typename Head>
            struct TypeListFilterSelect {};

            template<typename TypeListType, typename Head>
            struct TypeListFilterSelect<true, TypeListType, Head> {
                typedef typename TypeListType::push_back<Head> type;
            };

            template<typename TypeListType, typename Head>
            struct TypeListFilterSelect<false, TypeListType, Head> {
                typedef TypeListType type;
            };

            template<template <typename T> typename conditional, typename TypeListType, typename Head, typename... InputTypes>
            struct TypeListFilter<conditional, TypeListType, Head, InputTypes...> {
                typedef typename TypeListFilter<
                    conditional, 
                    typename TypeListFilterSelect<conditional<Head>::value, TypeListType, Head>::type,
                    InputTypes...
                >::type type;
            };

            template<template <typename T> typename conditional, typename TypeListType>
            struct TypeListFilter<conditional, TypeListType> {
                typedef TypeListType type;
            };

            template<typename SourceTuple, typename DestTuple, int index>
            struct TupleAssignImpl {
                static void assign(const SourceTuple &source, DestTuple &dest) {
                    TupleAssignImpl<SourceTuple, DestTuple, index - 1>::assign(source, dest);
                    std::get<index - 1>(dest) = std::get<index - 1>(source);
                }
            };

            template<typename SourceTuple, typename DestTuple>
            struct TupleAssignImpl<SourceTuple, DestTuple, 0> {
                static void assign(const SourceTuple &, DestTuple &) {}
            };
        }

        template<typename SourceTuple, typename DestTuple>
        DestTuple tupleAssign(SourceTuple source) {
            DestTuple dest;
            internal::TupleAssignImpl<SourceTuple, DestTuple, std::tuple_size<SourceTuple>::value>::assign(source, dest);
            return dest;
        }
    }
}

#endif // SCHNEK_GENERIC_TYPELIST_HPP_