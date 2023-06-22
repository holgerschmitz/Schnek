/*
 * test_typelist.hpp
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

#include <generic/typelist.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <boost/test/unit_test.hpp>

#include <string>

#pragma GCC diagnostic pop

BOOST_AUTO_TEST_SUITE( generic )

BOOST_AUTO_TEST_CASE(typelist) {
    typedef schnek::generic::TypeList<int, double, std::string> TList;
    
    TList::get<0>::type a = 1;
    TList::get<1>::type b = 0.5;
    TList::get<2>::type c = "1.0";

    BOOST_CHECK_EQUAL(a, 1);
    BOOST_CHECK_EQUAL(b, 0.5);
    BOOST_CHECK_EQUAL(c, "1.0");
}

BOOST_AUTO_TEST_SUITE_END()