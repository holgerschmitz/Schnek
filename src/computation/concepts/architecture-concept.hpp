/*
 * architecture-concept.hpp
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

#ifndef SCHNEK_COMPUTATION_CONCEPTS_ARCHITECTURE_CONCEPT_HPP_
#define SCHNEK_COMPUTATION_CONCEPTS_ARCHITECTURE_CONCEPT_HPP_

#include <stddef.h>

#include <type_traits>
#include <utility>
#include <string>

#include "../../grid/gridstorage/grid-storage-concept.hpp"
#include "../../generic/concept/typelist-concept.hpp"

namespace schnek::computation::concepts {

  namespace internal::architecture {

    // Check if Architecture has a GridStorageType member
    template<typename, typename = std::void_t<>>
    struct has_grid_storage_type : std::false_type {};

    template<typename T>
    struct has_grid_storage_type<T, std::void_t<typename T::template GridStorageType<int, 1>>> : std::true_type {};

    // Check if Architecture has a string ID
    template<typename, typename = std::void_t<>>
    struct has_id_string : std::false_type {};

    template<typename T>
    struct has_id_string<T, std::void_t<decltype(T::id)>> : std::is_same<decltype(T::id), const std::string> {};

    // Check is Architecture has a PreferredCopySource member
    template<typename, typename = std::void_t<>>
    struct has_preferred_copy_source : std::false_type {};

    template<typename T>
    struct has_preferred_copy_source<T, std::void_t<typename T::PreferredCopySource>> : std::true_type {};

    // Check if Architecture has an AllowedCopySources type list
    template<typename, typename = std::void_t<>>
    struct has_allowed_copy_sources : std::false_type {};

    template<typename T>
    struct has_allowed_copy_sources<T, std::void_t<typename T::AllowedCopySources>> 
      : schnek::generic::concepts::is_typelist<typename T::AllowedCopySources> {};
  }  // namespace internal::architecture

  /**
   * @brief Concept for an architecture
   *
   * @tparam Architecture
   */
  template<typename Architecture>
  struct ArchitectureConcept {
      static constexpr bool has_grid_storage_type = internal::architecture::has_grid_storage_type<Architecture>::value;
      static_assert(has_grid_storage_type, "Architecture must have a GridStorageType member");

      static constexpr bool has_id_string = internal::architecture::has_id_string<Architecture>::value;
      static_assert(has_id_string, "Architecture must have a string ID");

      static constexpr bool has_preferred_copy_source = internal::architecture::has_preferred_copy_source<Architecture>::value;

      static constexpr bool has_allowed_copy_sources = internal::architecture::has_allowed_copy_sources<Architecture>::value;
      static_assert(has_allowed_copy_sources, "Architecture must have an AllowedCopySources type list");

      static constexpr bool value = has_grid_storage_type;
  };

}  // namespace schnek::computation::concepts

#endif  // SCHNEK_COMPUTATION_CONCEPTS_ARCHITECTURE_CONCEPT_HPP_