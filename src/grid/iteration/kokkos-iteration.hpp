/*
 * kokkos-iteration.hpp
 *
 * Created on: 16 Dec 2022
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

#ifndef SCHNEK_GRID_ITERATION_KOKKOSITERATION_HPP_
#define SCHNEK_GRID_ITERATION_KOKKOSITERATION_HPP_

#include "../../config.hpp"
#include "../array.hpp"

#ifdef SCHNEK_HAVE_KOKKOS

#include <Kokkos_Core.hpp>

namespace schnek {
    
    template<
      size_t rank, 
      typename executionSpace = Kokkos::DefaultExecutionSpace
    >
    struct RangeKokkosIterationPolicy
    {
        /**
         * @brief Call a function for each index in the range
         * 
         * The range will be iterated over using Kokkos parallel dispatch
         * 
         * @tparam RangeType The range type. Requires accessor methods `getLo()` and `getHi()`
         *     that return the array-like bounds of the range with length `rank`
         * @tparam Func The function that will be called with an array-like index of length `rank`
         * @param range The range over which to iterate
         * @param func The function that will be called for each position in the range
         */
        template<
            class RangeType,
            typename Func
        >
        static void forEach(const RangeType& range, const Func &func);
    };

    //=================================================================
    //==================== RangeKokkosIterationPolicy =================
    //=================================================================

    namespace implementation {

        template<class ArrayType, size_t... indices>
        inline std::initializer_list<typename ArrayType::value_type> arrayToInitializerListHelper(
            const ArrayType &a, 
            std::index_sequence<indices...>
        ) 
        {
            return std::initializer_list<typename ArrayType::value_type>{a[indices]...};
        }

        template<class ArrayType>
        inline std::initializer_list<typename ArrayType::value_type> arrayToInitializerList(const ArrayType &a)
        {
            return arrayToInitializerListHelper(a, std::make_index_sequence<ArrayType::Length>{});
        }

        // template <typename R, typename... T>
        // std::initializer_list<R> make_initializer_list(T... t) {
        //     return {t...};
        // }


// Here is an example how an initializer list can be created generically

// template<size_t rank>
// void assign(Array<int, rank> a) {
//     std::initializer_list<int> list;
//     assign_helper(a, list, std::make_index_sequence<rank>{});
// }

// template<size_t rank, size_t... indices>
// void assign_helper(Array<int, rank> a, std::initializer_list<int>& list, std::index_sequence<indices...>) {
//     list = {a[indices]...};
// }

// int main() {
//     Array<int, 3> a = {1, 2, 3};
//     assign(a);
//     return 0;
// }

    }

    template<typename executionSpace>
    struct RangeKokkosIterationPolicy<1, executionSpace>
    {
        template<
            class RangeType,
            typename Func
        >
        static void forEach(const RangeType& range, const Func &func)
        {
            typedef typename RangeType::value_type T;
            Kokkos::RangePolicy<
                Kokkos::IndexType<T>,
                executionSpace
            > rangePolicy(range.getLo()[0], range.getHi()[0] + 1);

            Kokkos::parallel_for(rangePolicy, [=](T ind)
            {
                func(typename RangeType::LimitType(ind));
            }); 
        }
    };

    template<
      size_t rank, 
      typename executionSpace
    >
    template<
        class RangeType,
        typename Func
    >
    inline void RangeKokkosIterationPolicy<rank, executionSpace>::forEach(const RangeType& range, const Func &func)
    {
        typedef typename RangeType::value_type T;
        Kokkos::MDRangePolicy< 
            Kokkos::IndexType<T>,
            Kokkos::Rank<rank>, 
            executionSpace 
        > rangePolicy(
            implementation::arrayToInitializerList(range.getLo()), 
            implementation::arrayToInitializerList(range.getHi() + 1) 
        );

        Kokkos::parallel_for(rangePolicy, [=]<typename ...T>(T ... ind)
        {
            func(typename RangeType::LimitType{ind...});
        });
    }

} // namespace schnek

#endif // SCHNEK_HAVE_KOKKOS
#endif // SCHNEK_GRID_ITERATION_RANGEITERATION_HPP_