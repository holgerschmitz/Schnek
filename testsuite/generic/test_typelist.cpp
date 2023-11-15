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
#include <memory>
#include <type_traits>
#include <tuple>

#pragma GCC diagnostic pop


template<typename T>
struct ToSharedPtr {
    using type = std::shared_ptr<T>;
};

BOOST_AUTO_TEST_SUITE( generic )

BOOST_AUTO_TEST_CASE(typelist) {
    typedef schnek::generic::TypeList<int, double, std::string> TList;
    
    TList::get<0>::type a = 1;
    TList::get<1>::type b = 0.5;
    TList::get<2>::type c = "1.0";

    
    BOOST_CHECK_EQUAL(TList::size, 3);
    BOOST_CHECK_EQUAL(a, 1);
    BOOST_CHECK_EQUAL(b, 0.5);
    BOOST_CHECK_EQUAL(c, "1.0");
}

BOOST_AUTO_TEST_CASE(typelist_map) {
    typedef schnek::generic::TypeList<int, double, std::string> TList;
    
    typedef typename TList::map<ToSharedPtr> PtrTypeList;

    PtrTypeList::get<0>::type a(new int(1));
    PtrTypeList::get<1>::type b(new double(0.5));
    PtrTypeList::get<2>::type c(new std::string("Hello"));

    BOOST_CHECK_EQUAL(*a, 1);
    BOOST_CHECK_EQUAL(*b, 0.5);
    BOOST_CHECK_EQUAL(*c, "Hello");
}


BOOST_AUTO_TEST_CASE(typelist_filter) {
    typedef schnek::generic::TypeList<int, double, std::string> TList;
    
    typedef typename TList::filter<std::is_floating_point> FilteredTList;

    FilteredTList::get<0>::type a = 0.5;

    BOOST_CHECK_EQUAL(FilteredTList::size, 1);
    BOOST_CHECK_EQUAL(a, 0.5);
}

BOOST_AUTO_TEST_CASE(typelist_apply) {
    typedef schnek::generic::TypeList<int, double, std::string> TList;
    
    typedef typename TList::apply<std::tuple> TupleType;

    TupleType value{1, 0.5, "Hello World!"};

    BOOST_CHECK_EQUAL(std::get<0>(value), 1);
    BOOST_CHECK_EQUAL(std::get<1>(value), 0.5);
    BOOST_CHECK_EQUAL(std::get<2>(value), "Hello World!");
}


BOOST_AUTO_TEST_CASE(tuple_assign) {
    typedef std::tuple<int, double> SourceType;
    typedef std::tuple<int, double, std::string> DestType;
    SourceType source{1, 0.5};
    DestType dest = schnek::generic::tupleAssign<SourceType, DestType>(source);
    
    BOOST_CHECK_EQUAL(std::get<0>(dest), 1);
    BOOST_CHECK_EQUAL(std::get<1>(dest), 0.5);
    BOOST_CHECK_EQUAL(std::get<2>(dest), "");
}


BOOST_AUTO_TEST_SUITE_END()