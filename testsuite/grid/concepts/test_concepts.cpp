/*
 * test_storage_concept.cpp
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

#include <generic/typelist.hpp>
#include <grid/gridstorage.hpp>
#include <grid/gridcheck/gridcheck.hpp>
#include <grid/gridstorage/grid-storage-concept.hpp>

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

BOOST_AUTO_TEST_SUITE( grid )
BOOST_AUTO_TEST_SUITE( concepts )

BOOST_AUTO_TEST_CASE(get_storage_concept) {
    typedef schnek::SingleArrayGridStorage<double, 3> StorageA;
    typedef schnek::SingleArrayGridStorageFortran<int, 2> StorageB;
    typedef schnek::LazyArrayGridStorage<float, 1> StorageC;
    typedef schnek::GridNoArgCheck<int, 3> CheckA;
    typedef schnek::GridAssertCheck<int, 3> CheckB;

    typedef schnek::generic::TypeList<CheckA, StorageA, CheckB> TypeListA;
    typedef schnek::generic::TypeList<CheckA, StorageA, CheckB, StorageB> TypeListB;
    typedef schnek::generic::TypeList<CheckA, CheckB> TypeListC;

    BOOST_CHECK_EQUAL(schnek::concepts::GridStorageConceptCondition<StorageA>::value, true);
    BOOST_CHECK_EQUAL(schnek::concepts::GridStorageConceptCondition<StorageB>::value, true);
    BOOST_CHECK_EQUAL(schnek::concepts::GridStorageConceptCondition<StorageC>::value, true);
    BOOST_CHECK_EQUAL(schnek::concepts::GridStorageConceptCondition<CheckA>::value, false);
    BOOST_CHECK_EQUAL(schnek::concepts::GridStorageConceptCondition<CheckB>::value, false);

    typedef TypeListA::filter<schnek::concepts::GridStorageConceptCondition> StorageListA;
    typedef TypeListB::filter<schnek::concepts::GridStorageConceptCondition> StorageListB;
    typedef TypeListC::filter<schnek::concepts::GridStorageConceptCondition> StorageListC;

    BOOST_CHECK_EQUAL(StorageListA::size, 1);
    BOOST_CHECK_EQUAL(StorageListB::size, 2);
    BOOST_CHECK_EQUAL(StorageListC::size, 0);

    typedef TypeListA::getWithDefault<schnek::concepts::GridStorageConceptCondition, StorageC> StorageSelectA;
    // MANUAL_TEST: This should not compile because StorageC contains more than one GridStorageConcept
    // typedef TypeListB::getWithDefault<schnek::concepts::GridStorageConceptCondition, StorageC> StorageSelectB;
    typedef TypeListC::getWithDefault<schnek::concepts::GridStorageConceptCondition, StorageC> StorageSelectC;

    bool isSameA = std::is_same<StorageSelectA, StorageA>::value;
    bool isSameC = std::is_same<StorageSelectC, StorageC>::value;
    BOOST_CHECK_EQUAL(isSameA, true);
    BOOST_CHECK_EQUAL(isSameC, true);
}



BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()