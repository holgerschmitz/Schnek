/*
 * typelist.hpp
 *
 * Created on: 22 Jun 2023
 * Author: hschmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2023 Holger Schmitz
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
 */

#ifndef SCHNEK_GENERIC_TYPELIST_HPP_
#define SCHNEK_GENERIC_TYPELIST_HPP_

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace schnek::generic {
  template<typename... Types>
  struct TypeList;

  namespace internal {
    template<typename... Lists>
    struct TypeListConcat;

    template<>
    struct TypeListConcat<> {
        using type = TypeList<>;
    };

    template<typename... Ts>
    struct TypeListConcat<TypeList<Ts...>> {
        using type = TypeList<Ts...>;
    };

    template<typename... Ts, typename... Us, typename... Rest>
    struct TypeListConcat<TypeList<Ts...>, TypeList<Us...>, Rest...> {
        using type = typename TypeListConcat<TypeList<Ts..., Us...>, Rest...>::type;
    };

    template<typename TypeListType, typename DefaultType>
    struct TypeSingleElementOrDefault;

    template<typename>
    inline constexpr bool always_false = false;
  }  // namespace internal

  /**
   * A type list
   *
   * This is a type list. It is a list of types that can be accessed by index.
   *
   * @tparam Types The types in the list
   */
  template<typename... Types>
  struct TypeList {
      /**
       * The number of types in the list
       */
      static constexpr size_t size = sizeof...(Types);

      /**
       * Get the type at index n
       *
       * @tparam n The index
       * @return The type at index n
       */
      template<size_t n>
      struct get {
          static_assert(n < size, "Index out of bounds");
          using type = std::tuple_element_t<n, std::tuple<Types...>>;
      };

      /**
       * @brief Push a type to the back of the type list
       *
       * @tparam T The type to push
       */
      template<typename T>
      using push_back = TypeList<Types..., T>;

      /**
       * @brief Push a type to the front of the type list
       *
       * @tparam T The type to push
       */
      template<typename T>
      using push_front = TypeList<T, Types...>;

      /**
       * @brief Apply a metafunction to every type in the list
       *
       * @tparam mapper A template with a nested `type` for each element
       */
      template<template<typename T> typename mapper>
      using map = TypeList<typename mapper<Types>::type...>;

      /**
       * @brief Keep only the types for which `conditional<T>::value` is true
       */
      template<template<typename T> typename conditional>
      using filter = typename internal::TypeListConcat<
          std::conditional_t<conditional<Types>::value, TypeList<Types>, TypeList<>>...>::type;

      /**
       * @brief Get the single type satisfying the condition, or the default if there is none
       *
       * It is an error if more than one type satisfies the condition.
       */
      template<template<typename T> typename conditional, typename DefaultType>
      using getWithDefault = typename internal::TypeSingleElementOrDefault<filter<conditional>, DefaultType>::type;

      template<template<typename... T> typename Dest>
      using apply = Dest<Types...>;
  };

  namespace internal {
    template<typename DefaultType>
    struct TypeSingleElementOrDefault<TypeList<>, DefaultType> {
        using type = DefaultType;
    };

    template<typename T, typename DefaultType>
    struct TypeSingleElementOrDefault<TypeList<T>, DefaultType> {
        using type = T;
    };

    template<typename T1, typename T2, typename... Ts, typename DefaultType>
    struct TypeSingleElementOrDefault<TypeList<T1, T2, Ts...>, DefaultType> {
        static_assert(always_false<TypeList<T1, T2, Ts...>>, "TypeList must have at most one unique element");
    };

    template<typename SourceTuple, typename DestTuple, size_t... indices>
    void tupleAssignImpl(const SourceTuple &source, DestTuple &dest, std::index_sequence<indices...>) {
      ((std::get<indices>(dest) = std::get<indices>(source)), ...);
    }
  }  // namespace internal

  /**
   * @brief Assigns the values of a tuple to another tuple
   *
   * The values in the destination tuple must be default constructible.
   *
   * @tparam SourceTuple The source tuple type
   * @tparam DestTuple The destination tuple type
   * @param source The source tuple
   * @return DestTuple The destination tuple
   */
  template<typename SourceTuple, typename DestTuple>
  DestTuple tupleAssign(SourceTuple source) {
    DestTuple dest;
    internal::tupleAssignImpl(source, dest, std::make_index_sequence<std::tuple_size<SourceTuple>::value>{});
    return dest;
  }
}  // namespace schnek::generic

#endif  // SCHNEK_GENERIC_TYPELIST_HPP_