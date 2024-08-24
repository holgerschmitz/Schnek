/*
 * architecture-concept.hpp
 *
 * Created on: 24 Aug 2024
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

#ifndef SCHNEK_COMPUTATION_CONCEPTS_TYPELIST_CONCEPT_HPP_
#define SCHNEK_COMPUTATION_CONCEPTS_TYPELIST_CONCEPT_HPP_

#include <type_traits>


namespace schnek::generic {
  template<typename... Types>
  struct TypeList;

  namespace concepts {

    // Check if type is typelist
    template<typename, typename = std::void_t<>>
    struct is_typelist : std::false_type {};

    template<typename ...Types>
    struct is_typelist<TypeList<Types...>> : std::true_type {};
  } // namespace concepts

}  // namespace schnek::generic

#endif /* SCHNEK_COMPUTATION_CONCEPTS_TYPELIST_CONCEPT_HPP_ */
