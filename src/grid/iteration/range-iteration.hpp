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

    /**
     * @brief Iteration policy that iterates over a domain in C-order
     * 
     * @tparam rank the rank of the domain to iterate over
     */
    template<size_t rank>
    struct RangeCIterationPolicy {
        /**
         * @brief Call a function for each index in the range
         * 
         * The range will be iterated over in C-ordering
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
        static void forEach(const RangeType& range, Func func);
    };

    /**
     * @brief Iteration policy that iterates over a domain in Fortran-order
     * 
     * @tparam rank the rank of the domain to iterate over
     */
    template<size_t rank>
    struct RangeFortranIterationPolicy {
        /**
         * @brief Call a function for each index in the range
         * 
         * The range will be iterated over in Fortran-ordering
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
        static void forEach(const RangeType& range, Func func);
    };
   
    //=================================================================
    //==================== RangeCIterationPolicy ======================
    //=================================================================

    namespace internal {
        template<size_t rank, size_t dim>
        struct RangeCIterationPolicyImpl;

        template<size_t rank>
        struct RangeCIterationPolicyImpl<rank, 0> {
            template<
                class RangeType,
                class IndexType,
                typename Func
            >
            static void forEach(
                const RangeType&, 
                IndexType& pos,
                Func func
            )
            {
                func(pos);
            }
        };

        template<size_t rank, size_t dim>
        struct RangeCIterationPolicyImpl {
            template<
                class RangeType,
                class IndexType,
                typename Func
            >
            static void forEach(
                const RangeType& range, 
                IndexType& pos,
                Func func
            )
            {
                constexpr size_t idim = rank - dim;
                auto lo = range.getLo()[idim];
                auto hi = range.getHi()[idim];
                for (pos[idim]=lo; pos[idim]<=hi; ++pos[idim])
                {
                    RangeCIterationPolicyImpl<rank, dim-1>::forEach(range, pos, func);
                }            
            }
        };
    }    

    template<size_t rank>
    template<
        class RangeType,
        typename Func
    >
    inline void RangeCIterationPolicy<rank>::forEach(const RangeType& range, Func func)
    {
        auto pos = range.getLo();
        internal::RangeCIterationPolicyImpl<rank, rank>::forEach(range, pos, func);
    }

    //=================================================================
    //==================== RangeFortranIterationPolicy ================
    //=================================================================

    namespace internal
    {
        template<size_t rank>
        struct RangeFortranIterationPolicyImpl;

        template<>
        struct RangeFortranIterationPolicyImpl<0> {
            template<
                class RangeType,
                class IndexType,
                typename Func
            >
            static void forEach(
                const RangeType&, 
                IndexType& pos,
                Func func
            )
            {
                func(pos);
            }
        };

        template<size_t rank>
        struct RangeFortranIterationPolicyImpl {
            template<
                class RangeType,
                class IndexType,
                typename Func
            >
            static void forEach(
                const RangeType& range, 
                IndexType& pos,
                Func func
            )
            {
                constexpr size_t dim = rank - 1;
                auto lo = range.getLo()[dim];
                auto hi = range.getHi()[dim];
                for (pos[dim]=lo; pos[dim]<=hi; ++pos[dim])
                {
                    RangeFortranIterationPolicyImpl<rank-1>::forEach(range, pos, func);
                }            
            }
        };

    }

    template<size_t rank>
    template<
        class RangeType,
        typename Func
    >
    inline void RangeFortranIterationPolicy<rank>::forEach(const RangeType& range, Func func)
    {
        auto pos = range.getLo();
        internal::RangeFortranIterationPolicyImpl<rank>::forEach(range, pos, func);
    }

} // namespace schnek

#endif // SCHNEK_GRID_ITERATION_RANGEITERATION_HPP_