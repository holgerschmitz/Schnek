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

    namespace internal {
        template<typename Func, typename RangeType>
        struct RangeKokkosIterationPolicyFunctor
        {
            Func func;

            template<typename... Indices>
            SCHNEK_INLINE void operator()(Indices... ind) const
            {
                func(typename RangeType::LimitType{ind...});
            }
        };
    }

    // specialization for 1d because Kokkos::MDRangePolicy can only be used for rank>1
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
            typedef Kokkos::RangePolicy<
                Kokkos::IndexType<T>,
                executionSpace
            > ExecutionPolicy;
            
            ExecutionPolicy rangePolicy(range.getLo()[0], range.getHi()[0] + 1);

            internal::RangeKokkosIterationPolicyFunctor<Func, RangeType> functor{func};

            Kokkos::parallel_for("schnek:forEach", rangePolicy, functor); 
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
        T lo[rank];
        T hi[rank];
        const typename RangeType::LimitType& loR = range.getLo();
        const typename RangeType::LimitType& hiR = range.getHi();

        for (size_t i=0; i<rank; ++i)
        {
            lo[i] = loR[i];
            hi[i] = hiR[i] + 1;
        }

        Kokkos::MDRangePolicy<
            Kokkos::IndexType<T>,
            Kokkos::Rank<rank>,
            executionSpace
        > rangePolicy(lo, hi);

        internal::RangeKokkosIterationPolicyFunctor<Func, RangeType> functor{func};

        Kokkos::parallel_for("schnek:forEach", rangePolicy, functor);
    }

} // namespace schnek

#endif // SCHNEK_HAVE_KOKKOS
#endif // SCHNEK_GRID_ITERATION_RANGEITERATION_HPP_
