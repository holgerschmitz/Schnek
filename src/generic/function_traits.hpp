/*
 * typelist.hpp
 *
 * Created on: 27 Nov 2026
 * Author: hschmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2026 Holger Schmitz
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

#ifndef SCHNEK_GENERIC_FUNCTION_TRAITS_HPP_
#define SCHNEK_GENERIC_FUNCTION_TRAITS_HPP_

namespace schnek::generic {
  template<typename Callable>
  struct FunctionTraits : FunctionTraits<decltype(&Callable::operator())> {};

  template<typename Return, typename... Args>
  struct FunctionTraits<Return(Args...)> {
      using ArgumentTypes = std::tuple<Args...>;
  };

  template<typename Return, typename... Args>
  struct FunctionTraits<Return (*)(Args...)> : FunctionTraits<Return(Args...)> {};

  template<typename Return, typename... Args>
  struct FunctionTraits<Return (*)(Args...) noexcept> : FunctionTraits<Return(Args...)> {};

  template<typename Class, typename Return, typename... Args>
  struct FunctionTraits<Return (Class::*)(Args...)> : FunctionTraits<Return(Args...)> {};

  template<typename Class, typename Return, typename... Args>
  struct FunctionTraits<Return (Class::*)(Args...) const> : FunctionTraits<Return(Args...)> {};

  template<typename Class, typename Return, typename... Args>
  struct FunctionTraits<Return (Class::*)(Args...) noexcept> : FunctionTraits<Return(Args...)> {};

  template<typename Class, typename Return, typename... Args>
  struct FunctionTraits<Return (Class::*)(Args...) const noexcept> : FunctionTraits<Return(Args...)> {};

  template<typename Callable>
  using FunctionParameterTypesT = typename FunctionTraits<std::decay_t<Callable>>::ArgumentTypes;

  template<typename Tuple>
  struct TupleTail;

  template<typename First, typename... Rest>
  struct TupleTail<std::tuple<First, Rest...>> {
      using type = std::tuple<Rest...>;
  };

  template<typename Tuple>
  using TupleTailT = typename TupleTail<Tuple>::type;
}  // namespace schnek::generic

#endif  // SCHNEK_GENERIC_FUNCTION_TRAITS_HPP_