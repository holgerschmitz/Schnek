/*
 * concepts.hpp
 *
 * Created on: 27 Oct 2025
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

#ifndef SCHNEK_GENERIC_CONCEPTS_HPP_
#define SCHNEK_GENERIC_CONCEPTS_HPP_

#include <stddef.h>

#include <type_traits>
#include <utility>

#include "is-detected.hpp"

namespace schnek::concepts {
  template<typename T>
  using value_type_t = typename T::value_type;

  template<typename T>
  using rank_t = decltype(T::rank);

  template<typename T>
  struct has_rank : std::conjunction<is_detected<rank_t, T>, std::is_same<detected_t<rank_t, T>, const size_t>> {};

  template<typename T>
  using size_property_t = decltype(T::size);

  template<typename T>
  struct has_size
      : std::conjunction<is_detected<size_property_t, T>, std::is_same<detected_t<size_property_t, T>, size_t>> {};

  template<typename T>
  using index_type_t = typename T::IndexType;

  template<typename T>
  using range_type_t = typename T::RangeType;

  template<typename T>
  using range_property_t = decltype(T::range);

  template<typename T>
  struct has_range : std::conjunction<
                         is_detected<range_property_t, T>,
                         std::is_same<detected_t<range_property_t, T>, typename T::RangeType>> {};

  template<typename T>
  using dims_property_t = decltype(T::dims);

  template<typename T>
  struct has_dims : std::conjunction<
                        is_detected<dims_property_t, T>,
                        std::is_same<detected_t<dims_property_t, T>, typename T::IndexType>> {};

}  // namespace schnek::concepts

#endif  // SCHNEK_GENERIC_CONCEPTS_HPP_
