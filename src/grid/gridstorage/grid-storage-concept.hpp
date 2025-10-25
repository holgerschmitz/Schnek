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

#include <stddef.h>

#include <type_traits>
#include <utility>

namespace schnek {
  namespace concepts {
    template<typename, typename = void>
    struct has_value_type : std::false_type {};

    template<typename T>
    struct has_value_type<T, std::void_t<typename T::value_type>> : std::true_type {};

    template<typename, typename = void>
    struct has_rank : std::false_type {};

    template<typename T>
    struct has_rank<T, std::void_t<decltype(T::rank)>> : std::is_same<decltype(T::rank), const size_t> {};

    namespace internal::grid_storage {
      template<typename, typename = void>
      struct has_index_type : std::false_type {};
      template<typename T>
      struct has_index_type<T, std::void_t<typename T::IndexType>> : std::true_type {};

      template<typename, typename = std::void_t<>>
      struct has_range_type : std::false_type {};
      template<typename T>
      struct has_range_type<T, std::void_t<typename T::RangeType>> : std::true_type {};

      template<typename, typename, typename = void>
      struct has_method_range : std::false_type {};
      template<typename T, typename IndexType>
      struct has_method_range<T, IndexType, std::void_t<decltype(std::declval<T>().range()>> : std::true_type {};

      template<typename, typename, typename = void>
      struct has_method_get : std::false_type {};
      template<typename T, typename IndexType>
      struct has_method_get<
          T,
          IndexType,
          std::void_t<decltype(std::declval<T>().get(std::declval<const IndexType&>()))>> : std::true_type {};

      template<typename, typename, typename = void>
      struct has_method_resize : std::false_type {};
      template<typename T, typename IndexType>
      struct has_method_resize<
          T,
          IndexType,
          std::void_t<
              decltype(std::declval<T>().resize(std::declval<const IndexType&>(), std::declval<const IndexType&>()))>>
          : std::true_type {};

      template<typename, typename = void>
      struct has_method_resize_range : std::false_type {};

      template<typename T>
      struct has_method_resize_range<
          T,
          std::void_t<decltype(std::declval<T>().resize(std::declval<typename T::RangeType>()))>> : std::true_type {};

      // Optional methods
      template<typename, typename = void>
      struct has_method_stride : std::false_type {};

      template<typename T>
      struct has_method_stride<T, std::void_t<decltype(std::declval<const T>().stride(std::declval<size_t>()))>>
          : std::true_type {};

      template<typename, typename = void>
      struct has_method_get_raw_data : std::false_type {};

      template<typename T>
      struct has_method_get_raw_data<T, std::void_t<decltype(std::declval<const T>().getRawData())>> : std::true_type {
      };

    }  // namespace internal::grid_storage

    // Reusable template class to check GridStorage requirements
    template<class GridStorage>
    struct GridStorageConcept {
        static constexpr bool has_value_type = has_value_type<GridStorage>::value;
        static constexpr bool has_rank = has_rank<GridStorage>::value;
        static constexpr bool has_index_type = internal::grid_storage::has_index_type<GridStorage>::value;
        static constexpr bool has_range_type = internal::grid_storage::has_range_type<GridStorage>::value;
        static constexpr bool has_get_method =
            internal::grid_storage::has_method_get<GridStorage, typename GridStorage::IndexType>::value;
        static constexpr bool has_resize_method =
            internal::grid_storage::has_method_resize<GridStorage, typename GridStorage::IndexType>::value;
        static constexpr bool has_resize_range_method =
            internal::grid_storage::has_method_resize_range<GridStorage>::value;

        static constexpr bool has_stride_method = internal::grid_storage::has_method_stride<GridStorage>::value;
        static constexpr bool has_get_raw_data_method =
            internal::grid_storage::has_method_get_raw_data<GridStorage>::value;

        static constexpr bool value = has_value_type && has_rank && has_index_type && has_range_type &&
                                      has_get_method && has_resize_method && has_resize_range_method &&
                                      has_stride_method && has_get_raw_data_method;

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