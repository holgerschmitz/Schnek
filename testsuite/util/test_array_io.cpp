/*
 *  test_array_io.cpp
 *
 *  Created on: 12 Oct 2020
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 */

#include <grid/range.hpp>
#include <grid/array.hpp>

#include <util/array_io.hpp>

#include <boost/progress.hpp>
#include <boost/test/unit_test.hpp>

#include <sstream>

BOOST_AUTO_TEST_SUITE( array_io )

BOOST_AUTO_TEST_CASE( array_int )
{
  std::ostringstream t1;
  schnek::Array<int, 1> a1(12);
  t1 << a1;
  BOOST_CHECK_EQUAL(t1.str(), "(12)");
}



BOOST_AUTO_TEST_SUITE_END()
