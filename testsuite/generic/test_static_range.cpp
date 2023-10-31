/*
 * test_static_range.hpp
 *
 * Created on: 31 Oct 2023
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

#include <generic/static-range.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <boost/test/unit_test.hpp>

#include <string>

#pragma GCC diagnostic pop

BOOST_AUTO_TEST_SUITE( generic )

BOOST_AUTO_TEST_CASE(static_range) {
    typedef schnek::generic::StaticRange<4, 6> SRange;

    BOOST_CHECK_EQUAL(SRange::lo, 4);
    BOOST_CHECK_EQUAL(SRange::hi, 6);

    typedef schnek::generic::StaticGhostCells<> SGC0;
    typedef SGC0::put<4, 6> SGC1;
    typedef SGC0::put<1, 2>::put<3, 4>::put<5, 6> SGC2;

    BOOST_CHECK_EQUAL(SGC1::rank, 1);
    BOOST_CHECK_EQUAL(SGC1::get<0>::lo, 4);
    BOOST_CHECK_EQUAL(SGC1::get<0>::hi, 6);
    BOOST_CHECK_EQUAL(SGC2::rank, 3);
    BOOST_CHECK_EQUAL(SGC2::get<0>::lo, 1);
    BOOST_CHECK_EQUAL(SGC2::get<0>::hi, 2);
    BOOST_CHECK_EQUAL(SGC2::get<1>::lo, 3);
    BOOST_CHECK_EQUAL(SGC2::get<1>::hi, 4);
    BOOST_CHECK_EQUAL(SGC2::get<2>::lo, 5);
    BOOST_CHECK_EQUAL(SGC2::get<2>::hi, 6);

    typedef SGC0::repeat<3, 4, 6>::type SGC3;

    BOOST_CHECK_EQUAL(SGC3::rank, 3);
    BOOST_CHECK_EQUAL(SGC3::get<0>::lo, 4);
    BOOST_CHECK_EQUAL(SGC3::get<0>::hi, 6);
    BOOST_CHECK_EQUAL(SGC3::get<1>::lo, 4);
    BOOST_CHECK_EQUAL(SGC3::get<1>::hi, 6);
    BOOST_CHECK_EQUAL(SGC3::get<2>::lo, 4);
    BOOST_CHECK_EQUAL(SGC3::get<2>::hi, 6);
}

BOOST_AUTO_TEST_SUITE_END()