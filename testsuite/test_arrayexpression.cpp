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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy"

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/timer/progress_display.hpp>

#include <boost/test/unit_test.hpp>

#pragma GCC diagnostic pop
#pragma GCC diagnostic pop

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


BOOST_FIXTURE_TEST_CASE( array_expression, ArrayExpressionTest )
{
  const int N = 10000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    Array<int,3> s1(idist(rGen), idist(rGen), idist(rGen));
    Array<int,3> s2(idist(rGen), idist(rGen), idist(rGen));

    {
      auto exp = s1+s2;
      std::cerr << "Constructed\n";
      Array<int,3> s3 = exp; 
      Array<int,3> s4;
      BOOST_CHECK_EQUAL(s3[0], s1[0]+s2[0]);
      BOOST_CHECK_EQUAL(s3[1], s1[1]+s2[1]);
      BOOST_CHECK_EQUAL(s3[2], s1[2]+s2[2]);

      s4 = s1+s2;
      BOOST_CHECK_EQUAL(s4[0], s1[0]+s2[0]);
      BOOST_CHECK_EQUAL(s4[1], s1[1]+s2[1]);
      BOOST_CHECK_EQUAL(s4[2], s1[2]+s2[2]);
    }

    {
      Array<int,3> s3 = s1-s2;
      Array<int,3> s4;
      BOOST_CHECK_EQUAL(s3[0], s1[0]-s2[0]);
      BOOST_CHECK_EQUAL(s3[1], s1[1]-s2[1]);
      BOOST_CHECK_EQUAL(s3[2], s1[2]-s2[2]);

      s4 = s1-s2;
      BOOST_CHECK_EQUAL(s4[0], s1[0]-s2[0]);
      BOOST_CHECK_EQUAL(s4[1], s1[1]-s2[1]);
      BOOST_CHECK_EQUAL(s4[2], s1[2]-s2[2]);
    }

    {
      Array<int,3> s3 = s1*s2;
      Array<int,3> s4;
      BOOST_CHECK_EQUAL(s3[0], s1[0]*s2[0]);
      BOOST_CHECK_EQUAL(s3[1], s1[1]*s2[1]);
      BOOST_CHECK_EQUAL(s3[2], s1[2]*s2[2]);

      s4 = s1*s2;
      BOOST_CHECK_EQUAL(s4[0], s1[0]*s2[0]);
      BOOST_CHECK_EQUAL(s4[1], s1[1]*s2[1]);
      BOOST_CHECK_EQUAL(s4[2], s1[2]*s2[2]);
    }

    {
      Array<int,3> s3 = s1/s2;
      Array<int,3> s4;
      BOOST_CHECK_EQUAL(s3[0], s1[0]/s2[0]);
      BOOST_CHECK_EQUAL(s3[1], s1[1]/s2[1]);
      BOOST_CHECK_EQUAL(s3[2], s1[2]/s2[2]);

      s4 = s1/s2;
      BOOST_CHECK_EQUAL(s4[0], s1[0]/s2[0]);
      BOOST_CHECK_EQUAL(s4[1], s1[1]/s2[1]);
      BOOST_CHECK_EQUAL(s4[2], s1[2]/s2[2]);
    }


    {
      Array<int,3> s3(idist(rGen), idist(rGen), idist(rGen));
      Array<int,3> s4(idist(rGen), idist(rGen), idist(rGen));
      Array<int,3> s5 = ((s1+s2)*s3 - s4)/(s2+s3);
      Array<int,3> s6;
      BOOST_CHECK_EQUAL(s5[0], ((s1[0]+s2[0])*s3[0] - s4[0])/(s2[0]+s3[0]));
      BOOST_CHECK_EQUAL(s5[1], ((s1[1]+s2[1])*s3[1] - s4[1])/(s2[1]+s3[1]));
      BOOST_CHECK_EQUAL(s5[2], ((s1[2]+s2[2])*s3[2] - s4[2])/(s2[2]+s3[2]));

      s6 = ((s1+s2)*s3 - s4)/(s2+s3);
      BOOST_CHECK_EQUAL(s6[0], ((s1[0]+s2[0])*s3[0] - s4[0])/(s2[0]+s3[0]));
      BOOST_CHECK_EQUAL(s6[1], ((s1[1]+s2[1])*s3[1] - s4[1])/(s2[1]+s3[1]));
      BOOST_CHECK_EQUAL(s6[2], ((s1[2]+s2[2])*s3[2] - s4[2])/(s2[2]+s3[2]));
    }

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( compound_assignment, ArrayExpressionTest )
{
  const int N = 10000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    Array<int,3> s1(idist(rGen), idist(rGen), idist(rGen));
    Array<int,3> s2(idist(rGen), idist(rGen), idist(rGen));
    Array<int,3> s3(idist(rGen), idist(rGen), idist(rGen));


    {
      Array<int,3> s4 = s3;
      s4 += s1;

      BOOST_CHECK_EQUAL(s4[0], s3[0]+s1[0]);
      BOOST_CHECK_EQUAL(s4[1], s3[1]+s1[1]);
      BOOST_CHECK_EQUAL(s4[2], s3[2]+s1[2]);
    }

    {
      Array<int,3> s4 = s3;
      s4 -= s1;

      BOOST_CHECK_EQUAL(s4[0], s3[0]-s1[0]);
      BOOST_CHECK_EQUAL(s4[1], s3[1]-s1[1]);
      BOOST_CHECK_EQUAL(s4[2], s3[2]-s1[2]);
    }

    {
      Array<int,3> s4 = s3;
      s4 *= s1;

      BOOST_CHECK_EQUAL(s4[0], s3[0]*s1[0]);
      BOOST_CHECK_EQUAL(s4[1], s3[1]*s1[1]);
      BOOST_CHECK_EQUAL(s4[2], s3[2]*s1[2]);
    }

    {
      Array<int,3> s4 = s3;
      s4 /= s1;

      BOOST_CHECK_EQUAL(s4[0], s3[0]/s1[0]);
      BOOST_CHECK_EQUAL(s4[1], s3[1]/s1[1]);
      BOOST_CHECK_EQUAL(s4[2], s3[2]/s1[2]);
    }

    {
      Array<int,3> s4 = s3;
      s4 += (s1 + s2);

      BOOST_CHECK_EQUAL(s4[0], s3[0]+s1[0]+s2[0]);
      BOOST_CHECK_EQUAL(s4[1], s3[1]+s1[1]+s2[1]);
      BOOST_CHECK_EQUAL(s4[2], s3[2]+s1[2]+s2[2]);
    }

    {
      Array<int,3> s4 = s3;
      s4 -= (s1 + s2);

      BOOST_CHECK_EQUAL(s4[0], s3[0]-s1[0]-s2[0]);
      BOOST_CHECK_EQUAL(s4[1], s3[1]-s1[1]-s2[1]);
      BOOST_CHECK_EQUAL(s4[2], s3[2]-s1[2]-s2[2]);
    }

    {
      Array<int,3> s4 = s3;
      s4 *= (s1 + s2);

      BOOST_CHECK_EQUAL(s4[0], s3[0]*(s1[0]+s2[0]));
      BOOST_CHECK_EQUAL(s4[1], s3[1]*(s1[1]+s2[1]));
      BOOST_CHECK_EQUAL(s4[2], s3[2]*(s1[2]+s2[2]));
    }

    {
      Array<int,3> s4 = s3;
      s4 /= (s1 + s2);

      BOOST_CHECK_EQUAL(s4[0], s3[0]/(s1[0]+s2[0]));
      BOOST_CHECK_EQUAL(s4[1], s3[1]/(s1[1]+s2[1]));
      BOOST_CHECK_EQUAL(s4[2], s3[2]/(s1[2]+s2[2]));
    }

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( grid_constructor, ArrayExpressionTest )
{
  const int N = 10000;
  boost::timer::progress_display show_progress(4*N);

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
