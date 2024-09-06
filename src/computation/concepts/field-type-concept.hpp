/*
 * architecture-concept.hpp
 *
 * Created on: 06 Sep 2024
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

#ifndef SCHNEK_COMPUTATION_CONCEPTS_FIELD_TYPE_CONCEPT_HPP_
#define SCHNEK_COMPUTATION_CONCEPTS_FIELD_TYPE_CONCEPT_HPP_

#include <stddef.h>

#include <type_traits>
#include <utility>
#include <string>

#include "../../grid/gridstorage/single-array-allocation.hpp"
#include "../../generic/concept/typelist-concept.hpp"

namespace schnek::computation::concepts {

  namespace internal::fieldtype {

    // Check if Architecture has a GridStorageType member
    template<typename, typename = std::void_t<>>
    struct has_type : std::false_type {};

    template<typename T>
    struct has_type<T, std::void_t<typename T::template type<schnek::SingleArrayGridStorage> > > : std::true_type {};
  }  // namespace internal::architecture

  /**
   * @brief Concept for a storage dependent field type
   *
   * @tparam FieldType
   */
  template<typename FieldType>
  struct FieldTypeConcept {
      static constexpr bool has_type = internal::fieldtype::has_type<FieldType>::value;
      static_assert(has_type, "FieldType must have a 'type' member that defines a storage dependent field type");

      static constexpr bool value = has_type;
  };

}  // namespace schnek::computation::concepts

#endif  // SCHNEK_COMPUTATION_CONCEPTS_FIELD_TYPE_CONCEPT_HPP_