/*
 * test_array.cpp
 *
 *  Created on: 2 Aug 2017
 *      Author: Holger Schmitz
 */


#include "utility.hpp"

#include <grid/array.hpp>
#include <grid/grid.hpp>
#include <grid/arrayexpression.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <limits>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/progress.hpp>

#include <boost/test/unit_test.hpp>

using namespace schnek;

template<int size>
    struct SizeDummy
{};

struct ArrayExpressionTest
{
    boost::random::mt19937 rGen;
    boost::random::uniform_real_distribution<> dist;
    boost::random::uniform_int_distribution<> idist;
    boost::random::uniform_int_distribution<> idist_half;
    boost::random::uniform_int_distribution<> idist_small;

    ArrayExpressionTest()
      : dist(-1.0,1.0),
        idist(1,100),
        idist_half(1,50),
        idist_small(1, 10)
    {}
};


BOOST_AUTO_TEST_SUITE( arrayexpression )

BOOST_FIXTURE_TEST_CASE( grid_constructor, ArrayExpressionTest )
{
  const int N = 10000;
  boost::progress_display show_progress(4*N);

  for (int i=0; i<N; i++) {
    Array<int,2> s1(idist_small(rGen), idist_small(rGen));
    Array<int,2> s2(idist_small(rGen), idist_small(rGen));
    Grid<int,2> grid(s1+s2);

    BOOST_CHECK_EQUAL(grid.getLo()[0], 0);
    BOOST_CHECK_EQUAL(grid.getLo()[1], 0);
    BOOST_CHECK_EQUAL(grid.getHi()[0]+1, s1[0]+s2[0]);
    BOOST_CHECK_EQUAL(grid.getHi()[1]+1, s1[1]+s2[1]);

    ++show_progress;
  }

  for (int i=0; i<N; i++) {
    Array<int,2, ArrayBoostTestArgCheck> s1(idist_small(rGen), idist_small(rGen));
    Array<int,2, ArrayBoostTestArgCheck> s2(idist_small(rGen), idist_small(rGen));
    Grid<int,2> grid(s1+s2);

    BOOST_CHECK_EQUAL(grid.getLo()[0], 0);
    BOOST_CHECK_EQUAL(grid.getLo()[1], 0);
    BOOST_CHECK_EQUAL(grid.getHi()[0]+1, s1[0]+s2[0]);
    BOOST_CHECK_EQUAL(grid.getHi()[1]+1, s1[1]+s2[1]);

    ++show_progress;
  }

  for (int i=0; i<N; i++) {
    Array<int,2> s1(idist_small(rGen), idist_small(rGen));
    Array<int,2> s2(idist_small(rGen), idist_small(rGen));
    Grid<int,2> grid(-s1-s2, s1+s2);

    BOOST_CHECK_EQUAL(grid.getLo()[0], -s1[0]-s2[0]);
    BOOST_CHECK_EQUAL(grid.getLo()[1], -s1[1]-s2[1]);
    BOOST_CHECK_EQUAL(grid.getHi()[0],  s1[0]+s2[0]);
    BOOST_CHECK_EQUAL(grid.getHi()[1],  s1[1]+s2[1]);

    ++show_progress;
  }

  for (int i=0; i<N; i++) {
    Array<int,2, ArrayBoostTestArgCheck> s1(idist_small(rGen), idist_small(rGen));
    Array<int,2, ArrayBoostTestArgCheck> s2(idist_small(rGen), idist_small(rGen));
    Grid<int,2> grid(-s1-s2, s1+s2);

    BOOST_CHECK_EQUAL(grid.getLo()[0], -s1[0]-s2[0]);
    BOOST_CHECK_EQUAL(grid.getLo()[1], -s1[1]-s2[1]);
    BOOST_CHECK_EQUAL(grid.getHi()[0],  s1[0]+s2[0]);
    BOOST_CHECK_EQUAL(grid.getHi()[1],  s1[1]+s2[1]);

    ++show_progress;
  }
}

BOOST_AUTO_TEST_SUITE_END()
