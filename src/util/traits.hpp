/*
 * traits.hpp
 *
 * Created on: 1 Dec 2022
 * Author: Holger Schmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2012-2022 Holger Schmitz
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


#ifndef SCHNEK_UTIL_TRAITS_HPP
#define SCHNEK_UTIL_TRAITS_HPP

namespace schnek {

    /**
     * @brief Determines whether Fn can be invoked with the arguments Args....
     */
    template <class Fn, class... Args>
    struct is_invocable
    {
        template <class U>
        static auto test(U* p) -> decltype((*p)(std::declval<Args>()...), void(), std::true_type());
        template <class U>
        static auto test(...) -> decltype(std::false_type());

        static constexpr bool value = decltype(test<Fn>(0))::value;
    };

} // namespace schnek

#endif SCHNEK_UTIL_TRAITS_HPP