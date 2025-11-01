/*
 * is-detected.hpp
 *
 * The code in this file is adapted from code taken from
 * https://en.cppreference.com/
 *
 * Created on: 25 Oct 2025
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
 */

#ifndef SCHNEK_GENERIC_IS_DETECTED_HPP_
#define SCHNEK_GENERIC_IS_DETECTED_HPP_

#include <type_traits>

struct nonesuch {
    ~nonesuch() = delete;
    nonesuch(nonesuch const&) = delete;
    void operator=(nonesuch const&) = delete;
};

namespace internal {
  template<class Default, class AlwaysVoid, template<class...> class Op, class... Args>
  struct detector {
      using value_t = std::false_type;
      using type = Default;
  };

  template<class Default, template<class...> class Op, class... Args>
  struct detector<Default, std::void_t<Op<Args...>>, Op, Args...> {
      using value_t = std::true_type;
      using type = Op<Args...>;
  };
}  // namespace internal

template<template<class...> class Op, class... Args>
using is_detected = typename internal::detector<nonesuch, void, Op, Args...>::value_t;

template<template<class...> class Op, class... Args>
using detected_t = typename internal::detector<nonesuch, void, Op, Args...>::type;

template<class Default, template<class...> class Op, class... Args>
using detected_or = internal::detector<Default, void, Op, Args...>;

template<class Default, template<class...> class Op, class... Args>
using detected_or = internal::detector<Default, void, Op, Args...>;

#endif  // SCHNEK_GENERIC_IS_DETECTED_HPP_
