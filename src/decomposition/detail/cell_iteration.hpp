/*
 * cell_iteration.hpp
 *
 * Created on: 30 Jun 2026
 * Author: Holger Schmitz
 * Email: holger@notjustphysics.com
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
#ifndef SCHNEK_DECOMPOSITION_DETAIL_CELL_ITERATION_HPP_
#define SCHNEK_DECOMPOSITION_DETAIL_CELL_ITERATION_HPP_

#include <cstddef>
#include <type_traits>

#include "config.hpp"
#include "../../grid/gridstorage/grid-storage-concept.hpp"
#include "../../grid/iteration/range-iteration.hpp"

#ifdef SCHNEK_HAVE_KOKKOS
#include <Kokkos_Core.hpp>

#include "../../grid/iteration/kokkos-iteration.hpp"
#endif

namespace schnek {
  namespace detail {

    namespace internal {
      /**
       * @brief Detects whether a storage policy is backed by a Kokkos view, i.e.
       *        it exposes a `getKokkosView()` accessor.
       */
      template<
          class Storage,
          bool = concepts::GridStorageConceptCondition<Storage>::has_get_kokkos_view_method>
      struct is_kokkos_storage : std::false_type {};

      template<class Storage>
      struct is_kokkos_storage<Storage, true> : std::true_type {};
    }  // namespace internal

    /// True if `GridType` is backed by a Kokkos storage policy.
    template<class GridType>
    struct is_kokkos_grid : internal::is_kokkos_storage<typename GridType::storage_type> {};

    /**
     * @brief Maps a grid type to the iteration policy used to sweep its cells.
     *
     * For host (non-Kokkos) storage the C-order host policy is selected and the
     * grids are captured by reference, so writes hit the registered grid. For
     * Kokkos storage the execution-space-aware policy is selected and the grids
     * are captured by value: a Kokkos grid copy is shallow (it shares the
     * underlying view handle), which is required so the view can be captured into
     * a device kernel while writes still target the shared device memory.
     */
    template<class GridType, typename Enable = void>
    struct IterationPolicyFor {
        static constexpr std::size_t Rank = static_cast<std::size_t>(GridType::Rank);
        using type = RangeCIterationPolicy<Rank>;
        /// Capture grids by value (true) or by reference (false) for the kernel.
        static constexpr bool device_capture = false;
    };

#ifdef SCHNEK_HAVE_KOKKOS

    template<class GridType>
    struct IterationPolicyFor<GridType, std::enable_if_t<is_kokkos_grid<GridType>::value>> {
        static constexpr std::size_t Rank = static_cast<std::size_t>(GridType::Rank);
        using Storage = typename GridType::storage_type;
        using ExecSpace = typename Storage::ViewType::execution_space;
        using type = RangeKokkosIterationPolicy<Rank, ExecSpace>;
        static constexpr bool device_capture = true;
    };

#endif  // SCHNEK_HAVE_KOKKOS

    namespace internal {
      /**
       * @brief Host launch: sweep `range` with `IterationPolicy`, capturing the
       *        kernel and the grids by reference so writes reach the registered
       *        grids.
       */
      template<class IterationPolicy, class RangeType, class Kernel, class... Grids>
      void forEachCellHost(const RangeType &range, Kernel &kernel, Grids &...grids) {
        IterationPolicy::forEach(range, [&](const typename RangeType::LimitType &pos) { kernel(pos, grids...); });
      }

#ifdef SCHNEK_HAVE_KOKKOS
      /**
       * @brief Invoke the kernel from inside a device lambda.
       *
       * A `KOKKOS_LAMBDA` captures by value as `const`, but the kernel expects
       * non-const grid references. The grids are therefore taken here by value:
       * for a Kokkos grid the copy is shallow (it shares the underlying view), so
       * these non-const copies let the kernel write through the shared (device)
       * memory. The kernel's `operator()` must be const-callable (mark it
       * `SCHNEK_INLINE`).
       */
      template<class Kernel, class Pos, class... Grids>
      SCHNEK_INLINE void invokeCellKernel(const Kernel &kernel, const Pos &pos, Grids... grids) {
        kernel(pos, grids...);
      }

      template<std::size_t I, class T>
      struct PackElement {
        T value;

        SCHNEK_INLINE
        explicit PackElement(const T& v) : value(v) {}
      };

      template<class IndexSeq, class RangeType, class Kernel, class... Grids>
      struct ForEachCellDeviceFunctorImpl;
      
      template<std::size_t... Is, class RangeType, class Kernel, class... Grids>
      struct ForEachCellDeviceFunctorImpl<
          std::index_sequence<Is...>, RangeType, Kernel, Grids...>
        : PackElement<Is, Grids>... {
      
        Kernel kernel;
      
        SCHNEK_INLINE
        ForEachCellDeviceFunctorImpl(Kernel kernel_, Grids... grids_)
          : PackElement<Is, Grids>(grids_)...
          , kernel(kernel_) {}
      
        SCHNEK_INLINE
        void operator()(const typename RangeType::LimitType& pos) const {
          invokeCellKernel(
            kernel,
            pos,
            PackElement<Is, Grids>::value...
          );
        }
      };

      template<class RangeType, class Kernel, class... Grids>
      using ForEachCellDeviceFunctor =
        ForEachCellDeviceFunctorImpl<
          std::index_sequence_for<Grids...>,
          RangeType,
          Kernel,
          Grids...
        >;

      /**
       * @brief Device launch: sweep `range` with the Kokkos `IterationPolicy`.
       *
       * The kernel and grids are passed by value and captured by value into the
       * `KOKKOS_LAMBDA`. For Kokkos grids the copy is shallow (it shares the
       * view), so the kernel writes through the shared (possibly device) memory.
       * On a host Kokkos backend `KOKKOS_LAMBDA` expands to a plain `[=]` lambda,
       * so the same code compiles and runs on CPU-only builds.
       */
      template<class IterationPolicy, class RangeType, class Kernel, class... Grids>
      void forEachCellDevice(const RangeType &range, Kernel kernel, Grids... grids) {
        using Functor =
          ForEachCellDeviceFunctor<
            RangeType,
            Kernel,
            std::decay_t<Grids>...
          >;
        IterationPolicy::forEach(
            range, Functor(kernel, grids...)
        );
      }
#endif  // SCHNEK_HAVE_KOKKOS

    }  // namespace internal

  }  // namespace detail
}  // namespace schnek

#endif  // SCHNEK_DECOMPOSITION_DETAIL_CELL_ITERATION_HPP_
