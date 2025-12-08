/*
 * grid-storage-concept.hpp
 *
 * Created on: 19 Jul 2024
 * Author: Holger Schmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2024 Holger Schmitz
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

#ifndef SCHNEK_GRID_GRIDSTORAGE_GRID_ALLOCATION_CONCEPT_HPP_
#define SCHNEK_GRID_GRIDSTORAGE_GRID_ALLOCATION_CONCEPT_HPP_

#include <stddef.h>

#include <type_traits>
#include <utility>

#include "../../generic/concepts.hpp"
#include "../../generic/is-detected.hpp"

namespace schnek::concepts {
  namespace internal::grid_allocation {
    template<typename T>
    using updater_type_t = typename T::UpdaterType;

    template<typename T>
    using get_data_method_t = decltype(std::declval<T>().getData());

    template<typename T>
    using resize_method_t = decltype(std::declval<T>().resizeImpl(
        std::declval<typename T::IndexType>(), std::declval<typename T::IndexType>()
    ));

    template<typename T>
    using on_update_method_t = decltype(std::declval<T>().onUpdate(std::declval<typename T::UpdaterType>()));
  }  // namespace internal::grid_allocation

  // Reusable template class to check GridAllocation requirements
  template<class GridAllocation>
  struct GridAllocationConceptCondition {
      static constexpr bool has_value_type = is_detected<value_type_t, GridAllocation>::value;
      static constexpr bool has_rank = has_rank<GridAllocation>::value;
      static constexpr bool has_index_type = is_detected<index_type_t, GridAllocation>::value;
      static constexpr bool has_range_type = is_detected<range_type_t, GridAllocation>::value;
      static constexpr bool has_data_method =
          is_detected<internal::grid_allocation::get_data_method_t, GridAllocation>::value;
      static constexpr bool has_updater_type =
          is_detected<internal::grid_allocation::updater_type_t, GridAllocation>::value;
      static constexpr bool has_resize_method =
          is_detected<internal::grid_allocation::resize_method_t, GridAllocation>::value;
      static constexpr bool has_on_update_method =
          is_detected<internal::grid_allocation::on_update_method_t, GridAllocation>::value;

      static constexpr bool value = has_value_type && has_rank && has_index_type && has_range_type && has_data_method &&
                                    has_updater_type && has_resize_method && has_on_update_method;
  };

  template<class GridAllocation>
  struct GridAllocationConcept : public GridAllocationConceptCondition<GridAllocation> {
      typedef GridAllocationConceptCondition<GridAllocation> Concept;
      static_assert(Concept::has_value_type, "GridAllocation must have value_type typedef");
      static_assert(Concept::has_rank, "GridAllocation must define the rank as a const size_t");
      static_assert(Concept::has_index_type, "GridAllocation must have IndexType typedef");
      static_assert(Concept::has_range_type, "GridAllocation must have RangeType typedef");
      static_assert(Concept::has_updater_type, "GridAllocation must have SizeInfo typedef");
      static_assert(Concept::has_data_method, "GridAllocation must define a data attribute");
      static_assert(Concept::has_resize_method, "GridAllocation must define a resize method");
      static_assert(Concept::has_on_update_method, "GridAllocation must define an onUpdate method");
  };
}  // namespace schnek::concepts

#endif  // SCHNEK_GRID_GRIDSTORAGE_GRID_ALLOCATION_CONCEPT_HPP_
