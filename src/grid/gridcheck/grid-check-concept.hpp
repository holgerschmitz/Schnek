/*
 * grid-storage-concept.hpp
 *
 * Created on: 31 Oct 2025
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

#ifndef SCHNEK_GRID_GRIDCHECK_GRID_CHECK_CONCEPT_HPP_
#define SCHNEK_GRID_GRIDCHECK_GRID_CHECK_CONCEPT_HPP_

#include <stddef.h>

#include <type_traits>
#include <utility>

#include "../../generic/is-detected.hpp"
#include "../../generic/concepts.hpp"

namespace schnek::concepts {
  namespace internal::grid_check {
    template<typename T>
    using check_method_t = decltype(T::check(
        std::declval<typename T::IndexType>(), std::declval<typename T::IndexType>(), std::declval<typename T::IndexType>()
    ));
  }  // namespace internal::grid_check

  // Reusable template class to check GridCheck requirements
  template<class GridCheck>
  struct GridCheckConceptCondition {
      static constexpr bool has_index_type = is_detected<index_type_t, GridCheck>::value;
      static constexpr bool has_check_method =
          is_detected<internal::grid_check::check_method_t, GridCheck>::value;

      static constexpr bool value = has_index_type && has_check_method;
  };

  template<class GridCheck>
  struct GridCheckConcept : public GridCheckConceptCondition<GridCheck> {
      typedef GridCheckConceptCondition<GridCheck> Concept;
      static_assert(Concept::has_index_type, "GridCheck must have IndexType typedef");
      static_assert(Concept::has_check_method, "GridCheck must define a check method");
  };
}  // namespace schnek::concepts

#endif  // SCHNEK_GRID_GRIDCHECK_GRID_CHECK_CONCEPT_HPP_
