/*
 * grid-layout-concept.hpp
 *
 * Created on: 29 Oct 2025
 * Author: Holger Schmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2025 Holger Schmitz
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

#ifndef SCHNEK_GRID_GRIDSTORAGE_GRID_ALLOCATION_LAYOUT_HPP_
#define SCHNEK_GRID_GRIDSTORAGE_GRID_ALLOCATION_LAYOUT_HPP_

#include <stddef.h>

#include <type_traits>
#include <utility>

#include "../../generic/concepts.hpp"
#include "../../generic/is-detected.hpp"

namespace schnek::concepts {
  namespace internal::grid_layout {
    template<typename T>
    using updater_type_t = typename T::UpdaterType;

    template<typename T>
    using get_method_t =
        decltype(std::declval<T>().get(std::declval<typename T::IndexType>(), std::declval<typename T::IndexType>()));

    template<typename T>
    using stride_method_t =
        decltype(std::declval<T>().stride(std::declval<size_t>(), std::declval<typename T::IndexType>()));

    template<typename T>
    using update_size_method_t =
        decltype(std::declval<T>().updateSize(std::declval<typename T::RangeType>(), std::declval<T*>()));
  }  // namespace internal::grid_layout

  // Reusable template class to check GridLayout requirements
  template<class GridLayout>
  struct GridLayoutConceptCondition {
      static constexpr bool has_index_type = is_detected<index_type_t, GridLayout>::value;
      static constexpr bool has_range_type = is_detected<range_type_t, GridLayout>::value;
      static constexpr bool has_get_method = is_detected<internal::grid_layout::get_method_t, GridLayout>::value;
      static constexpr bool has_stride_method = is_detected<internal::grid_layout::stride_method_t, GridLayout>::value;
      static constexpr bool has_update_size_method =
          is_detected<internal::grid_layout::update_size_method_t, GridLayout>::value;

      static constexpr bool value =
          has_index_type && has_range_type && has_get_method && has_stride_method && has_update_size_method;
  };

  template<class GridLayout>
  struct GridLayoutConcept : public GridLayoutConceptCondition<GridLayout> {
      typedef GridLayoutConceptCondition<GridLayout> Concept;
      static_assert(Concept::has_index_type, "GridLayout must have IndexType typedef");
      static_assert(Concept::has_range_type, "GridLayout must have RangeType typedef");
      static_assert(Concept::has_get_method, "GridLayout must define a get method");
      static_assert(Concept::has_stride_method, "GridLayout must define a stride method");
      static_assert(Concept::has_update_size_method, "GridLayout must define an updateSize method");
  };

}  // namespace schnek::concepts

#endif  // SCHNEK_GRID_GRIDSTORAGE_GRID_ALLOCATION_LAYOUT_HPP_
