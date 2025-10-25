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

#ifndef SCHNEK_GRID_GRIDSTORAGE_GRID_STORAGE_CONCEPT_HPP_
#define SCHNEK_GRID_GRIDSTORAGE_GRID_STORAGE_CONCEPT_HPP_

#include "../../generic/is-detected.hpp"

#include <stddef.h>

#include <type_traits>
#include <utility>

namespace schnek {
  namespace concepts {
    // Detection templates for typedefs
    template<typename T>
    using value_type_t = typename T::value_type;

    template<typename T>
    using rank_t = decltype(T::rank);

    template<typename T>
    struct has_rank : std::conjunction<is_detected<rank_t, T>, std::is_same<detected_t<rank_t, T>, const size_t>> {};

    namespace internal::grid_storage {
      // Detection templates for typedefs
      template<typename T>
      using index_type_t = typename T::IndexType;

      template<typename T>
      using range_type_t = typename T::RangeType;

      // Detection templates for methods
      template<typename T>
      using range_method_t = decltype(std::declval<T>().range());

      template<typename T>
      using get_method_t = decltype(std::declval<T>().get(std::declval<typename T::IndexType&>()));

      template<typename T>
      using resize_method_t = decltype(std::declval<T>().resize(std::declval<typename T::IndexType>(), std::declval<typename T::IndexType>()));

      template<typename T>
      using resize_range_method_t = decltype(std::declval<T>().resize(std::declval<typename T::RangeType>()));

      // Optional methods
      template<typename T>
      using stride_method_t = decltype(std::declval<const T>().stride(std::declval<size_t>()));

      template<typename T>
      using get_raw_data_method_t = decltype(std::declval<const T>().getRawData());
    }  // namespace internal::grid_storage

    // Reusable template class to check GridStorage requirements
    template<class GridStorage>
    struct GridStorageConcept {
        static constexpr bool has_value_type = is_detected<value_type_t, GridStorage>::value;
        static constexpr bool has_rank = has_rank<GridStorage>::value;
        static constexpr bool has_index_type = is_detected<internal::grid_storage::index_type_t, GridStorage>::value;
        static constexpr bool has_range_type = is_detected<internal::grid_storage::range_type_t, GridStorage>::value;
        static constexpr bool has_get_method = is_detected<internal::grid_storage::get_method_t, GridStorage>::value;
        static constexpr bool has_resize_method =
            is_detected<internal::grid_storage::resize_method_t, GridStorage>::value;
        static constexpr bool has_resize_range_method =
            is_detected<internal::grid_storage::resize_range_method_t, GridStorage>::value;

        static constexpr bool has_stride_method = is_detected<internal::grid_storage::stride_method_t, GridStorage>::value;
        static constexpr bool has_get_raw_data_method =
            is_detected<internal::grid_storage::get_raw_data_method_t, GridStorage>::value;

        static constexpr bool value = has_value_type && has_rank && has_index_type && has_range_type &&
                                      has_get_method && has_resize_method && has_resize_range_method;

        static_assert(has_value_type, "GridStorage must have value_type typedef");
        static_assert(has_index_type, "GridStorage must have IndexType typedef");
        static_assert(has_range_type, "GridStorage must have RangeType typedef");
        static_assert(has_rank, "GridStorage must define the rank as a const size_t");
        static_assert(has_get_method, "GridStorage must have method T& get(const IndexType&)");
        static_assert(
            has_resize_method, "GridStorage must have method void resize(const IndexType&, const IndexType&)"
        );
    };

    template<typename T, size_t rank, template<typename, size_t> class GridStorage>
    struct GridStorageConceptTempl {
        using Storage = GridStorage<T, rank>;

        static constexpr bool value = GridStorageConcept<Storage>::value;

        static_assert(value, "GridStorage must meet GridStorageConcept requirements");
    };

  }  // namespace concepts

}  // namespace schnek

#endif  // SCHNEK_GRID_GRIDSTORAGE_GRID_STORAGE_CONCEPT_HPP_

// // Grid class template
// template <typename T, size_t rank, template <typename, size_t> class GridStorage>
// class Grid {
//     GridStorageConcept<T, rank, GridStorage> concept_check;

// public:
//     using Storage = GridStorage<T, rank>;

//     static constexpr bool has_stride = GridStorageConcept<T, rank, GridStorage>::has_stride_method;
//     static constexpr bool has_get_raw_data = GridStorageConcept<T, rank, GridStorage>::has_get_raw_data_method;

//     // Implementation of the Grid class
// };