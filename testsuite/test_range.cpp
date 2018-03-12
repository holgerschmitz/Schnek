/*
 * test_array.cpp
 *
 *  Created on: 25 Jul 2017
 *      Author: Holger Schmitz
 */

#include "utility.hpp"

#include <grid/array.hpp>
#include <grid/range.hpp>
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

struct RangeTest
{
    boost::random::mt19937 rGen;
    boost::random::uniform_real_distribution<> dist;
    boost::random::uniform_int_distribution<> idist;
    boost::random::uniform_int_distribution<> idist_small;

    RangeTest()
      : dist(-1.0,1.0),
        idist(std::numeric_limits<int>::min(), std::numeric_limits<int>::max()),
        idist_small(std::numeric_limits<int>::min()/10, std::numeric_limits<int>::max()/10)
    {}


};

BOOST_AUTO_TEST_SUITE( range )

BOOST_FIXTURE_TEST_CASE( constructor_1d, RangeTest )
{
  const int N = 100000;
  boost::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int lo1 = idist(rGen);
    int hi1 = idist(rGen);
    Array<int, 1, ArrayBoostTestArgCheck> lo(lo1);
    Array<int, 1, ArrayBoostTestArgCheck> hi(hi1);
    Range<int, 1, ArrayBoostTestArgCheck> test(lo, hi);

    BOOST_CHECK_EQUAL(lo1, test.getLo()[0]);
    BOOST_CHECK_EQUAL(hi1, test.getHi()[0]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( constructor_2d, RangeTest )
{
  const int N = 100000;
  boost::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int lo1  = idist(rGen);
    int lo2  = idist(rGen);

    int hi1  = idist(rGen);
    int hi2  = idist(rGen);

    Array<int, 2, ArrayBoostTestArgCheck> lo(lo1, lo2);
    Array<int, 2, ArrayBoostTestArgCheck> hi(hi1, hi2);
    Range<int, 2, ArrayBoostTestArgCheck> test(lo, hi);

    BOOST_CHECK_EQUAL(lo1 , test.getLo()[0]);
    BOOST_CHECK_EQUAL(lo2 , test.getLo()[1 ]);

    BOOST_CHECK_EQUAL(hi1 , test.getHi()[0]);
    BOOST_CHECK_EQUAL(hi2 , test.getHi()[1 ]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( constructor_3d, RangeTest )
{
  const int N = 100000;
  boost::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int lo1  = idist(rGen);
    int lo2  = idist(rGen);
    int lo3  = idist(rGen);

    int hi1  = idist(rGen);
    int hi2  = idist(rGen);
    int hi3  = idist(rGen);

    Array<int, 3, ArrayBoostTestArgCheck> lo(lo1, lo2, lo3);
    Array<int, 3, ArrayBoostTestArgCheck> hi(hi1, hi2, hi3);
    Range<int, 3, ArrayBoostTestArgCheck> test(lo, hi);

    BOOST_CHECK_EQUAL(lo1 , test.getLo()[0]);
    BOOST_CHECK_EQUAL(lo2 , test.getLo()[1 ]);
    BOOST_CHECK_EQUAL(lo3 , test.getLo()[2 ]);

    BOOST_CHECK_EQUAL(hi1 , test.getHi()[0]);
    BOOST_CHECK_EQUAL(hi2 , test.getHi()[1 ]);
    BOOST_CHECK_EQUAL(hi3 , test.getHi()[2 ]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( constructor_4d, RangeTest )
{
  const int N = 100000;
  boost::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int lo1  = idist(rGen);
    int lo2  = idist(rGen);
    int lo3  = idist(rGen);
    int lo4  = idist(rGen);

    int hi1  = idist(rGen);
    int hi2  = idist(rGen);
    int hi3  = idist(rGen);
    int hi4  = idist(rGen);

    Array<int, 4, ArrayBoostTestArgCheck> lo(lo1, lo2, lo3, lo4);
    Array<int, 4, ArrayBoostTestArgCheck> hi(hi1, hi2, hi3, hi4);
    Range<int, 4, ArrayBoostTestArgCheck> test(lo, hi);

    BOOST_CHECK_EQUAL(lo1 , test.getLo()[0]);
    BOOST_CHECK_EQUAL(lo2 , test.getLo()[1 ]);
    BOOST_CHECK_EQUAL(lo3 , test.getLo()[2 ]);
    BOOST_CHECK_EQUAL(lo4 , test.getLo()[3 ]);

    BOOST_CHECK_EQUAL(hi1 , test.getHi()[0]);
    BOOST_CHECK_EQUAL(hi2 , test.getHi()[1 ]);
    BOOST_CHECK_EQUAL(hi3 , test.getHi()[2 ]);
    BOOST_CHECK_EQUAL(hi4 , test.getHi()[3 ]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( constructor_5d, RangeTest )
{
  const int N = 100000;
  boost::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int lo1  = idist(rGen);
    int lo2  = idist(rGen);
    int lo3  = idist(rGen);
    int lo4  = idist(rGen);
    int lo5  = idist(rGen);

    int hi1  = idist(rGen);
    int hi2  = idist(rGen);
    int hi3  = idist(rGen);
    int hi4  = idist(rGen);
    int hi5  = idist(rGen);

    Array<int, 5, ArrayBoostTestArgCheck> lo(lo1, lo2, lo3, lo4, lo5);
    Array<int, 5, ArrayBoostTestArgCheck> hi(hi1, hi2, hi3, hi4, hi5);
    Range<int, 5, ArrayBoostTestArgCheck> test(lo, hi);

    BOOST_CHECK_EQUAL(lo1 , test.getLo()[0]);
    BOOST_CHECK_EQUAL(lo2 , test.getLo()[1 ]);
    BOOST_CHECK_EQUAL(lo3 , test.getLo()[2 ]);
    BOOST_CHECK_EQUAL(lo4 , test.getLo()[3 ]);
    BOOST_CHECK_EQUAL(lo5 , test.getLo()[4 ]);

    BOOST_CHECK_EQUAL(hi1 , test.getHi()[0]);
    BOOST_CHECK_EQUAL(hi2 , test.getHi()[1 ]);
    BOOST_CHECK_EQUAL(hi3 , test.getHi()[2 ]);
    BOOST_CHECK_EQUAL(hi4 , test.getHi()[3 ]);
    BOOST_CHECK_EQUAL(hi5 , test.getHi()[4 ]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( constructor_6d, RangeTest )
{
  const int N = 100000;
  boost::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int lo1  = idist(rGen);
    int lo2  = idist(rGen);
    int lo3  = idist(rGen);
    int lo4  = idist(rGen);
    int lo5  = idist(rGen);
    int lo6  = idist(rGen);

    int hi1  = idist(rGen);
    int hi2  = idist(rGen);
    int hi3  = idist(rGen);
    int hi4  = idist(rGen);
    int hi5  = idist(rGen);
    int hi6  = idist(rGen);

    Array<int, 6, ArrayBoostTestArgCheck> lo(lo1, lo2, lo3, lo4, lo5, lo6);
    Array<int, 6, ArrayBoostTestArgCheck> hi(hi1, hi2, hi3, hi4, hi5, hi6);
    Range<int, 6, ArrayBoostTestArgCheck> test(lo, hi);

    BOOST_CHECK_EQUAL(lo1 , test.getLo()[0]);
    BOOST_CHECK_EQUAL(lo2 , test.getLo()[1 ]);
    BOOST_CHECK_EQUAL(lo3 , test.getLo()[2 ]);
    BOOST_CHECK_EQUAL(lo4 , test.getLo()[3 ]);
    BOOST_CHECK_EQUAL(lo5 , test.getLo()[4 ]);
    BOOST_CHECK_EQUAL(lo6 , test.getLo()[5 ]);

    BOOST_CHECK_EQUAL(hi1 , test.getHi()[0]);
    BOOST_CHECK_EQUAL(hi2 , test.getHi()[1 ]);
    BOOST_CHECK_EQUAL(hi3 , test.getHi()[2 ]);
    BOOST_CHECK_EQUAL(hi4 , test.getHi()[3 ]);
    BOOST_CHECK_EQUAL(hi5 , test.getHi()[4 ]);
    BOOST_CHECK_EQUAL(hi6 , test.getHi()[5 ]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( constructor_7d, RangeTest )
{
  const int N = 100000;
  boost::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int lo1  = idist(rGen);
    int lo2  = idist(rGen);
    int lo3  = idist(rGen);
    int lo4  = idist(rGen);
    int lo5  = idist(rGen);
    int lo6  = idist(rGen);
    int lo7  = idist(rGen);

    int hi1  = idist(rGen);
    int hi2  = idist(rGen);
    int hi3  = idist(rGen);
    int hi4  = idist(rGen);
    int hi5  = idist(rGen);
    int hi6  = idist(rGen);
    int hi7  = idist(rGen);

    Array<int, 7, ArrayBoostTestArgCheck> lo(lo1, lo2, lo3, lo4, lo5, lo6, lo7);
    Array<int, 7, ArrayBoostTestArgCheck> hi(hi1, hi2, hi3, hi4, hi5, hi6, hi7);
    Range<int, 7, ArrayBoostTestArgCheck> test(lo, hi);

    BOOST_CHECK_EQUAL(lo1 , test.getLo()[0]);
    BOOST_CHECK_EQUAL(lo2 , test.getLo()[1 ]);
    BOOST_CHECK_EQUAL(lo3 , test.getLo()[2 ]);
    BOOST_CHECK_EQUAL(lo4 , test.getLo()[3 ]);
    BOOST_CHECK_EQUAL(lo5 , test.getLo()[4 ]);
    BOOST_CHECK_EQUAL(lo6 , test.getLo()[5 ]);
    BOOST_CHECK_EQUAL(lo7 , test.getLo()[6 ]);

    BOOST_CHECK_EQUAL(hi1 , test.getHi()[0]);
    BOOST_CHECK_EQUAL(hi2 , test.getHi()[1 ]);
    BOOST_CHECK_EQUAL(hi3 , test.getHi()[2 ]);
    BOOST_CHECK_EQUAL(hi4 , test.getHi()[3 ]);
    BOOST_CHECK_EQUAL(hi5 , test.getHi()[4 ]);
    BOOST_CHECK_EQUAL(hi6 , test.getHi()[5 ]);
    BOOST_CHECK_EQUAL(hi7 , test.getHi()[6 ]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( constructor_8d, RangeTest )
{
  const int N = 100000;
  boost::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int lo1  = idist(rGen);
    int lo2  = idist(rGen);
    int lo3  = idist(rGen);
    int lo4  = idist(rGen);
    int lo5  = idist(rGen);
    int lo6  = idist(rGen);
    int lo7  = idist(rGen);
    int lo8  = idist(rGen);

    int hi1  = idist(rGen);
    int hi2  = idist(rGen);
    int hi3  = idist(rGen);
    int hi4  = idist(rGen);
    int hi5  = idist(rGen);
    int hi6  = idist(rGen);
    int hi7  = idist(rGen);
    int hi8  = idist(rGen);

    Array<int, 8, ArrayBoostTestArgCheck> lo(lo1, lo2, lo3, lo4, lo5, lo6, lo7, lo8);
    Array<int, 8, ArrayBoostTestArgCheck> hi(hi1, hi2, hi3, hi4, hi5, hi6, hi7, hi8);
    Range<int, 8, ArrayBoostTestArgCheck> test(lo, hi);

    BOOST_CHECK_EQUAL(lo1 , test.getLo()[0]);
    BOOST_CHECK_EQUAL(lo2 , test.getLo()[1 ]);
    BOOST_CHECK_EQUAL(lo3 , test.getLo()[2 ]);
    BOOST_CHECK_EQUAL(lo4 , test.getLo()[3 ]);
    BOOST_CHECK_EQUAL(lo5 , test.getLo()[4 ]);
    BOOST_CHECK_EQUAL(lo6 , test.getLo()[5 ]);
    BOOST_CHECK_EQUAL(lo7 , test.getLo()[6 ]);
    BOOST_CHECK_EQUAL(lo8 , test.getLo()[7 ]);

    BOOST_CHECK_EQUAL(hi1 , test.getHi()[0]);
    BOOST_CHECK_EQUAL(hi2 , test.getHi()[1 ]);
    BOOST_CHECK_EQUAL(hi3 , test.getHi()[2 ]);
    BOOST_CHECK_EQUAL(hi4 , test.getHi()[3 ]);
    BOOST_CHECK_EQUAL(hi5 , test.getHi()[4 ]);
    BOOST_CHECK_EQUAL(hi6 , test.getHi()[5 ]);
    BOOST_CHECK_EQUAL(hi7 , test.getHi()[6 ]);
    BOOST_CHECK_EQUAL(hi8 , test.getHi()[7 ]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( constructor_9d, RangeTest )
{
  const int N = 100000;
  boost::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int lo1  = idist(rGen);
    int lo2  = idist(rGen);
    int lo3  = idist(rGen);
    int lo4  = idist(rGen);
    int lo5  = idist(rGen);
    int lo6  = idist(rGen);
    int lo7  = idist(rGen);
    int lo8  = idist(rGen);
    int lo9  = idist(rGen);

    int hi1  = idist(rGen);
    int hi2  = idist(rGen);
    int hi3  = idist(rGen);
    int hi4  = idist(rGen);
    int hi5  = idist(rGen);
    int hi6  = idist(rGen);
    int hi7  = idist(rGen);
    int hi8  = idist(rGen);
    int hi9  = idist(rGen);

    Array<int, 9, ArrayBoostTestArgCheck> lo(lo1, lo2, lo3, lo4, lo5, lo6, lo7, lo8, lo9);
    Array<int, 9, ArrayBoostTestArgCheck> hi(hi1, hi2, hi3, hi4, hi5, hi6, hi7, hi8, hi9);
    Range<int, 9, ArrayBoostTestArgCheck> test(lo, hi);

    BOOST_CHECK_EQUAL(lo1 , test.getLo()[0]);
    BOOST_CHECK_EQUAL(lo2 , test.getLo()[1 ]);
    BOOST_CHECK_EQUAL(lo3 , test.getLo()[2 ]);
    BOOST_CHECK_EQUAL(lo4 , test.getLo()[3 ]);
    BOOST_CHECK_EQUAL(lo5 , test.getLo()[4 ]);
    BOOST_CHECK_EQUAL(lo6 , test.getLo()[5 ]);
    BOOST_CHECK_EQUAL(lo7 , test.getLo()[6 ]);
    BOOST_CHECK_EQUAL(lo8 , test.getLo()[7 ]);
    BOOST_CHECK_EQUAL(lo9 , test.getLo()[8 ]);

    BOOST_CHECK_EQUAL(hi1 , test.getHi()[0]);
    BOOST_CHECK_EQUAL(hi2 , test.getHi()[1 ]);
    BOOST_CHECK_EQUAL(hi3 , test.getHi()[2 ]);
    BOOST_CHECK_EQUAL(hi4 , test.getHi()[3 ]);
    BOOST_CHECK_EQUAL(hi5 , test.getHi()[4 ]);
    BOOST_CHECK_EQUAL(hi6 , test.getHi()[5 ]);
    BOOST_CHECK_EQUAL(hi7 , test.getHi()[6 ]);
    BOOST_CHECK_EQUAL(hi8 , test.getHi()[7 ]);
    BOOST_CHECK_EQUAL(hi9 , test.getHi()[8 ]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( constructor_10d, RangeTest )
{
  const int N = 100000;
  boost::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int lo1  = idist(rGen);
    int lo2  = idist(rGen);
    int lo3  = idist(rGen);
    int lo4  = idist(rGen);
    int lo5  = idist(rGen);
    int lo6  = idist(rGen);
    int lo7  = idist(rGen);
    int lo8  = idist(rGen);
    int lo9  = idist(rGen);
    int lo10 = idist(rGen);
    int hi1  = idist(rGen);
    int hi2  = idist(rGen);
    int hi3  = idist(rGen);
    int hi4  = idist(rGen);
    int hi5  = idist(rGen);
    int hi6  = idist(rGen);
    int hi7  = idist(rGen);
    int hi8  = idist(rGen);
    int hi9  = idist(rGen);
    int hi10 = idist(rGen);
    Array<int, 10, ArrayBoostTestArgCheck> lo(lo1, lo2, lo3, lo4, lo5, lo6, lo7, lo8, lo9, lo10);
    Array<int, 10, ArrayBoostTestArgCheck> hi(hi1, hi2, hi3, hi4, hi5, hi6, hi7, hi8, hi9, hi10);
    Range<int, 10, ArrayBoostTestArgCheck> test(lo, hi);

    BOOST_CHECK_EQUAL(lo1 , test.getLo()[0]);
    BOOST_CHECK_EQUAL(lo2 , test.getLo()[1 ]);
    BOOST_CHECK_EQUAL(lo3 , test.getLo()[2 ]);
    BOOST_CHECK_EQUAL(lo4 , test.getLo()[3 ]);
    BOOST_CHECK_EQUAL(lo5 , test.getLo()[4 ]);
    BOOST_CHECK_EQUAL(lo6 , test.getLo()[5 ]);
    BOOST_CHECK_EQUAL(lo7 , test.getLo()[6 ]);
    BOOST_CHECK_EQUAL(lo8 , test.getLo()[7 ]);
    BOOST_CHECK_EQUAL(lo9 , test.getLo()[8 ]);
    BOOST_CHECK_EQUAL(lo10, test.getLo()[9 ]);

    BOOST_CHECK_EQUAL(hi1 , test.getHi()[0]);
    BOOST_CHECK_EQUAL(hi2 , test.getHi()[1 ]);
    BOOST_CHECK_EQUAL(hi3 , test.getHi()[2 ]);
    BOOST_CHECK_EQUAL(hi4 , test.getHi()[3 ]);
    BOOST_CHECK_EQUAL(hi5 , test.getHi()[4 ]);
    BOOST_CHECK_EQUAL(hi6 , test.getHi()[5 ]);
    BOOST_CHECK_EQUAL(hi7 , test.getHi()[6 ]);
    BOOST_CHECK_EQUAL(hi8 , test.getHi()[7 ]);
    BOOST_CHECK_EQUAL(hi9 , test.getHi()[8 ]);
    BOOST_CHECK_EQUAL(hi10, test.getHi()[9 ]);

    ++show_progress;
  }
}



BOOST_FIXTURE_TEST_CASE( project_2d, RangeTest )
{
  const int N = 10000;
  boost::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int lo1 = idist(rGen);
    int lo2 = idist(rGen);

    int hi1 = idist(rGen);
    int hi2 = idist(rGen);
    Array<int, 2, ArrayBoostTestArgCheck> lo(lo1, lo2);
    Array<int, 2, ArrayBoostTestArgCheck> hi(hi1, hi2);

    Range<int, 2, ArrayBoostTestArgCheck> test(lo, hi);

    Range<int, 1, ArrayBoostTestArgCheck> testProject1 = test.project<1>();

    BOOST_CHECK_EQUAL(testProject1.getLo()[0], test.getLo()[0]);
    BOOST_CHECK_EQUAL(testProject1.getHi()[0], test.getHi()[0]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( project_3d, RangeTest )
{
  const int N = 10000;
  boost::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int lo1 = idist(rGen);
    int lo2 = idist(rGen);
    int lo3 = idist(rGen);

    int hi1 = idist(rGen);
    int hi2 = idist(rGen);
    int hi3 = idist(rGen);
    Array<int, 3, ArrayBoostTestArgCheck> lo(lo1, lo2, lo3);
    Array<int, 3, ArrayBoostTestArgCheck> hi(hi1, hi2, hi3);

    Range<int, 3, ArrayBoostTestArgCheck> test(lo, hi);

    Range<int, 1, ArrayBoostTestArgCheck> testProject1 = test.project<1>();
    Range<int, 2, ArrayBoostTestArgCheck> testProject2 = test.project<2>();

    BOOST_CHECK_EQUAL(testProject1.getLo()[0], test.getLo()[0]);
    for (int i=0; i<2; ++i) BOOST_CHECK_EQUAL(testProject2.getLo()[i], test.getLo()[i]);

    BOOST_CHECK_EQUAL(testProject1.getHi()[0], test.getHi()[0]);
    for (int i=0; i<2; ++i) BOOST_CHECK_EQUAL(testProject2.getHi()[i], test.getHi()[i]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( project_4d, RangeTest )
{
  const int N = 10000;
  boost::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int lo1 = idist(rGen);
    int lo2 = idist(rGen);
    int lo3 = idist(rGen);
    int lo4 = idist(rGen);

    int hi1 = idist(rGen);
    int hi2 = idist(rGen);
    int hi3 = idist(rGen);
    int hi4 = idist(rGen);
    Array<int, 4, ArrayBoostTestArgCheck> lo(lo1, lo2, lo3, lo4);
    Array<int, 4, ArrayBoostTestArgCheck> hi(hi1, hi2, hi3, hi4);

    Range<int, 4, ArrayBoostTestArgCheck> test(lo, hi);

    Range<int, 1, ArrayBoostTestArgCheck> testProject1 = test.project<1>();
    Range<int, 2, ArrayBoostTestArgCheck> testProject2 = test.project<2>();
    Range<int, 3, ArrayBoostTestArgCheck> testProject3 = test.project<3>();

    BOOST_CHECK_EQUAL(testProject1.getLo()[0], test.getLo()[0]);
    for (int i=0; i<2; ++i) BOOST_CHECK_EQUAL(testProject2.getLo()[i], test.getLo()[i]);
    for (int i=0; i<3; ++i) BOOST_CHECK_EQUAL(testProject3.getLo()[i], test.getLo()[i]);

    BOOST_CHECK_EQUAL(testProject1.getHi()[0], test.getHi()[0]);
    for (int i=0; i<2; ++i) BOOST_CHECK_EQUAL(testProject2.getHi()[i], test.getHi()[i]);
    for (int i=0; i<3; ++i) BOOST_CHECK_EQUAL(testProject3.getHi()[i], test.getHi()[i]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( project_5d, RangeTest )
{
  const int N = 10000;
  boost::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int lo1 = idist(rGen);
    int lo2 = idist(rGen);
    int lo3 = idist(rGen);
    int lo4 = idist(rGen);
    int lo5 = idist(rGen);

    int hi1 = idist(rGen);
    int hi2 = idist(rGen);
    int hi3 = idist(rGen);
    int hi4 = idist(rGen);
    int hi5 = idist(rGen);
    Array<int, 5, ArrayBoostTestArgCheck> lo(lo1, lo2, lo3, lo4, lo5);
    Array<int, 5, ArrayBoostTestArgCheck> hi(hi1, hi2, hi3, hi4, hi5);

    Range<int, 5, ArrayBoostTestArgCheck> test(lo, hi);

    Range<int, 1, ArrayBoostTestArgCheck> testProject1 = test.project<1>();
    Range<int, 2, ArrayBoostTestArgCheck> testProject2 = test.project<2>();
    Range<int, 3, ArrayBoostTestArgCheck> testProject3 = test.project<3>();
    Range<int, 4, ArrayBoostTestArgCheck> testProject4 = test.project<4>();

    BOOST_CHECK_EQUAL(testProject1.getLo()[0], test.getLo()[0]);
    for (int i=0; i<2; ++i) BOOST_CHECK_EQUAL(testProject2.getLo()[i], test.getLo()[i]);
    for (int i=0; i<3; ++i) BOOST_CHECK_EQUAL(testProject3.getLo()[i], test.getLo()[i]);
    for (int i=0; i<4; ++i) BOOST_CHECK_EQUAL(testProject4.getLo()[i], test.getLo()[i]);
    BOOST_CHECK_EQUAL(testProject1.getHi()[0], test.getHi()[0]);
    for (int i=0; i<2; ++i) BOOST_CHECK_EQUAL(testProject2.getHi()[i], test.getHi()[i]);
    for (int i=0; i<3; ++i) BOOST_CHECK_EQUAL(testProject3.getHi()[i], test.getHi()[i]);
    for (int i=0; i<4; ++i) BOOST_CHECK_EQUAL(testProject4.getHi()[i], test.getHi()[i]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( project_6d, RangeTest )
{
  const int N = 10000;
  boost::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int lo1 = idist(rGen);
    int lo2 = idist(rGen);
    int lo3 = idist(rGen);
    int lo4 = idist(rGen);
    int lo5 = idist(rGen);
    int lo6 = idist(rGen);

    int hi1 = idist(rGen);
    int hi2 = idist(rGen);
    int hi3 = idist(rGen);
    int hi4 = idist(rGen);
    int hi5 = idist(rGen);
    int hi6 = idist(rGen);
    Array<int, 6, ArrayBoostTestArgCheck> lo(lo1, lo2, lo3, lo4, lo5, lo6);
    Array<int, 6, ArrayBoostTestArgCheck> hi(hi1, hi2, hi3, hi4, hi5, hi6);

    Range<int, 6, ArrayBoostTestArgCheck> test(lo, hi);

    Range<int, 1, ArrayBoostTestArgCheck> testProject1 = test.project<1>();
    Range<int, 2, ArrayBoostTestArgCheck> testProject2 = test.project<2>();
    Range<int, 3, ArrayBoostTestArgCheck> testProject3 = test.project<3>();
    Range<int, 4, ArrayBoostTestArgCheck> testProject4 = test.project<4>();
    Range<int, 5, ArrayBoostTestArgCheck> testProject5 = test.project<5>();

    BOOST_CHECK_EQUAL(testProject1.getLo()[0], test.getLo()[0]);
    for (int i=0; i<2; ++i) BOOST_CHECK_EQUAL(testProject2.getLo()[i], test.getLo()[i]);
    for (int i=0; i<3; ++i) BOOST_CHECK_EQUAL(testProject3.getLo()[i], test.getLo()[i]);
    for (int i=0; i<4; ++i) BOOST_CHECK_EQUAL(testProject4.getLo()[i], test.getLo()[i]);
    for (int i=0; i<5; ++i) BOOST_CHECK_EQUAL(testProject5.getLo()[i], test.getLo()[i]);

    BOOST_CHECK_EQUAL(testProject1.getHi()[0], test.getHi()[0]);
    for (int i=0; i<2; ++i) BOOST_CHECK_EQUAL(testProject2.getHi()[i], test.getHi()[i]);
    for (int i=0; i<3; ++i) BOOST_CHECK_EQUAL(testProject3.getHi()[i], test.getHi()[i]);
    for (int i=0; i<4; ++i) BOOST_CHECK_EQUAL(testProject4.getHi()[i], test.getHi()[i]);
    for (int i=0; i<5; ++i) BOOST_CHECK_EQUAL(testProject5.getHi()[i], test.getHi()[i]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( project_7d, RangeTest )
{
  const int N = 10000;
  boost::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int lo1 = idist(rGen);
    int lo2 = idist(rGen);
    int lo3 = idist(rGen);
    int lo4 = idist(rGen);
    int lo5 = idist(rGen);
    int lo6 = idist(rGen);
    int lo7 = idist(rGen);

    int hi1 = idist(rGen);
    int hi2 = idist(rGen);
    int hi3 = idist(rGen);
    int hi4 = idist(rGen);
    int hi5 = idist(rGen);
    int hi6 = idist(rGen);
    int hi7 = idist(rGen);
    Array<int, 7, ArrayBoostTestArgCheck> lo(lo1, lo2, lo3, lo4, lo5, lo6, lo7);
    Array<int, 7, ArrayBoostTestArgCheck> hi(hi1, hi2, hi3, hi4, hi5, hi6, hi7);

    Range<int, 7, ArrayBoostTestArgCheck> test(lo, hi);

    Range<int, 1, ArrayBoostTestArgCheck> testProject1 = test.project<1>();
    Range<int, 2, ArrayBoostTestArgCheck> testProject2 = test.project<2>();
    Range<int, 3, ArrayBoostTestArgCheck> testProject3 = test.project<3>();
    Range<int, 4, ArrayBoostTestArgCheck> testProject4 = test.project<4>();
    Range<int, 5, ArrayBoostTestArgCheck> testProject5 = test.project<5>();
    Range<int, 6, ArrayBoostTestArgCheck> testProject6 = test.project<6>();

    BOOST_CHECK_EQUAL(testProject1.getLo()[0], test.getLo()[0]);
    for (int i=0; i<2; ++i) BOOST_CHECK_EQUAL(testProject2.getLo()[i], test.getLo()[i]);
    for (int i=0; i<3; ++i) BOOST_CHECK_EQUAL(testProject3.getLo()[i], test.getLo()[i]);
    for (int i=0; i<4; ++i) BOOST_CHECK_EQUAL(testProject4.getLo()[i], test.getLo()[i]);
    for (int i=0; i<5; ++i) BOOST_CHECK_EQUAL(testProject5.getLo()[i], test.getLo()[i]);
    for (int i=0; i<6; ++i) BOOST_CHECK_EQUAL(testProject6.getLo()[i], test.getLo()[i]);

    BOOST_CHECK_EQUAL(testProject1.getHi()[0], test.getHi()[0]);
    for (int i=0; i<2; ++i) BOOST_CHECK_EQUAL(testProject2.getHi()[i], test.getHi()[i]);
    for (int i=0; i<3; ++i) BOOST_CHECK_EQUAL(testProject3.getHi()[i], test.getHi()[i]);
    for (int i=0; i<4; ++i) BOOST_CHECK_EQUAL(testProject4.getHi()[i], test.getHi()[i]);
    for (int i=0; i<5; ++i) BOOST_CHECK_EQUAL(testProject5.getHi()[i], test.getHi()[i]);
    for (int i=0; i<6; ++i) BOOST_CHECK_EQUAL(testProject6.getHi()[i], test.getHi()[i]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( project_8d, RangeTest )
{
  const int N = 10000;
  boost::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int lo1 = idist(rGen);
    int lo2 = idist(rGen);
    int lo3 = idist(rGen);
    int lo4 = idist(rGen);
    int lo5 = idist(rGen);
    int lo6 = idist(rGen);
    int lo7 = idist(rGen);
    int lo8 = idist(rGen);

    int hi1 = idist(rGen);
    int hi2 = idist(rGen);
    int hi3 = idist(rGen);
    int hi4 = idist(rGen);
    int hi5 = idist(rGen);
    int hi6 = idist(rGen);
    int hi7 = idist(rGen);
    int hi8 = idist(rGen);
    Array<int, 8, ArrayBoostTestArgCheck> lo(lo1, lo2, lo3, lo4, lo5, lo6, lo7, lo8);
    Array<int, 8, ArrayBoostTestArgCheck> hi(hi1, hi2, hi3, hi4, hi5, hi6, hi7, hi8);

    Range<int, 8, ArrayBoostTestArgCheck> test(lo, hi);

    Range<int, 1, ArrayBoostTestArgCheck> testProject1 = test.project<1>();
    Range<int, 2, ArrayBoostTestArgCheck> testProject2 = test.project<2>();
    Range<int, 3, ArrayBoostTestArgCheck> testProject3 = test.project<3>();
    Range<int, 4, ArrayBoostTestArgCheck> testProject4 = test.project<4>();
    Range<int, 5, ArrayBoostTestArgCheck> testProject5 = test.project<5>();
    Range<int, 6, ArrayBoostTestArgCheck> testProject6 = test.project<6>();
    Range<int, 7, ArrayBoostTestArgCheck> testProject7 = test.project<7>();

    BOOST_CHECK_EQUAL(testProject1.getLo()[0], test.getLo()[0]);
    for (int i=0; i<2; ++i) BOOST_CHECK_EQUAL(testProject2.getLo()[i], test.getLo()[i]);
    for (int i=0; i<3; ++i) BOOST_CHECK_EQUAL(testProject3.getLo()[i], test.getLo()[i]);
    for (int i=0; i<4; ++i) BOOST_CHECK_EQUAL(testProject4.getLo()[i], test.getLo()[i]);
    for (int i=0; i<5; ++i) BOOST_CHECK_EQUAL(testProject5.getLo()[i], test.getLo()[i]);
    for (int i=0; i<6; ++i) BOOST_CHECK_EQUAL(testProject6.getLo()[i], test.getLo()[i]);
    for (int i=0; i<7; ++i) BOOST_CHECK_EQUAL(testProject7.getLo()[i], test.getLo()[i]);

    BOOST_CHECK_EQUAL(testProject1.getHi()[0], test.getHi()[0]);
    for (int i=0; i<2; ++i) BOOST_CHECK_EQUAL(testProject2.getHi()[i], test.getHi()[i]);
    for (int i=0; i<3; ++i) BOOST_CHECK_EQUAL(testProject3.getHi()[i], test.getHi()[i]);
    for (int i=0; i<4; ++i) BOOST_CHECK_EQUAL(testProject4.getHi()[i], test.getHi()[i]);
    for (int i=0; i<5; ++i) BOOST_CHECK_EQUAL(testProject5.getHi()[i], test.getHi()[i]);
    for (int i=0; i<6; ++i) BOOST_CHECK_EQUAL(testProject6.getHi()[i], test.getHi()[i]);
    for (int i=0; i<7; ++i) BOOST_CHECK_EQUAL(testProject7.getHi()[i], test.getHi()[i]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( project_9d, RangeTest )
{
  const int N = 10000;
  boost::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int lo1 = idist(rGen);
    int lo2 = idist(rGen);
    int lo3 = idist(rGen);
    int lo4 = idist(rGen);
    int lo5 = idist(rGen);
    int lo6 = idist(rGen);
    int lo7 = idist(rGen);
    int lo8 = idist(rGen);
    int lo9 = idist(rGen);

    int hi1 = idist(rGen);
    int hi2 = idist(rGen);
    int hi3 = idist(rGen);
    int hi4 = idist(rGen);
    int hi5 = idist(rGen);
    int hi6 = idist(rGen);
    int hi7 = idist(rGen);
    int hi8 = idist(rGen);
    int hi9 = idist(rGen);
    Array<int, 9, ArrayBoostTestArgCheck> lo(lo1, lo2, lo3, lo4, lo5, lo6, lo7, lo8, lo9);
    Array<int, 9, ArrayBoostTestArgCheck> hi(hi1, hi2, hi3, hi4, hi5, hi6, hi7, hi8, hi9);

    Range<int, 9, ArrayBoostTestArgCheck> test(lo, hi);

    Range<int, 1, ArrayBoostTestArgCheck> testProject1 = test.project<1>();
    Range<int, 2, ArrayBoostTestArgCheck> testProject2 = test.project<2>();
    Range<int, 3, ArrayBoostTestArgCheck> testProject3 = test.project<3>();
    Range<int, 4, ArrayBoostTestArgCheck> testProject4 = test.project<4>();
    Range<int, 5, ArrayBoostTestArgCheck> testProject5 = test.project<5>();
    Range<int, 6, ArrayBoostTestArgCheck> testProject6 = test.project<6>();
    Range<int, 7, ArrayBoostTestArgCheck> testProject7 = test.project<7>();
    Range<int, 8, ArrayBoostTestArgCheck> testProject8 = test.project<8>();

    BOOST_CHECK_EQUAL(testProject1.getLo()[0], test.getLo()[0]);
    for (int i=0; i<2; ++i) BOOST_CHECK_EQUAL(testProject2.getLo()[i], test.getLo()[i]);
    for (int i=0; i<3; ++i) BOOST_CHECK_EQUAL(testProject3.getLo()[i], test.getLo()[i]);
    for (int i=0; i<4; ++i) BOOST_CHECK_EQUAL(testProject4.getLo()[i], test.getLo()[i]);
    for (int i=0; i<5; ++i) BOOST_CHECK_EQUAL(testProject5.getLo()[i], test.getLo()[i]);
    for (int i=0; i<6; ++i) BOOST_CHECK_EQUAL(testProject6.getLo()[i], test.getLo()[i]);
    for (int i=0; i<7; ++i) BOOST_CHECK_EQUAL(testProject7.getLo()[i], test.getLo()[i]);
    for (int i=0; i<8; ++i) BOOST_CHECK_EQUAL(testProject8.getLo()[i], test.getLo()[i]);

    BOOST_CHECK_EQUAL(testProject1.getHi()[0], test.getHi()[0]);
    for (int i=0; i<2; ++i) BOOST_CHECK_EQUAL(testProject2.getHi()[i], test.getHi()[i]);
    for (int i=0; i<3; ++i) BOOST_CHECK_EQUAL(testProject3.getHi()[i], test.getHi()[i]);
    for (int i=0; i<4; ++i) BOOST_CHECK_EQUAL(testProject4.getHi()[i], test.getHi()[i]);
    for (int i=0; i<5; ++i) BOOST_CHECK_EQUAL(testProject5.getHi()[i], test.getHi()[i]);
    for (int i=0; i<6; ++i) BOOST_CHECK_EQUAL(testProject6.getHi()[i], test.getHi()[i]);
    for (int i=0; i<7; ++i) BOOST_CHECK_EQUAL(testProject7.getHi()[i], test.getHi()[i]);
    for (int i=0; i<8; ++i) BOOST_CHECK_EQUAL(testProject8.getHi()[i], test.getHi()[i]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( project_10d, RangeTest )
{
  const int N = 10000;
  boost::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int lo1 = idist(rGen);
    int lo2 = idist(rGen);
    int lo3 = idist(rGen);
    int lo4 = idist(rGen);
    int lo5 = idist(rGen);
    int lo6 = idist(rGen);
    int lo7 = idist(rGen);
    int lo8 = idist(rGen);
    int lo9 = idist(rGen);
    int lo10 = idist(rGen);

    int hi1 = idist(rGen);
    int hi2 = idist(rGen);
    int hi3 = idist(rGen);
    int hi4 = idist(rGen);
    int hi5 = idist(rGen);
    int hi6 = idist(rGen);
    int hi7 = idist(rGen);
    int hi8 = idist(rGen);
    int hi9 = idist(rGen);
    int hi10 = idist(rGen);
    Array<int, 10, ArrayBoostTestArgCheck> lo(lo1, lo2, lo3, lo4, lo5, lo6, lo7, lo8, lo9, lo10);
    Array<int, 10, ArrayBoostTestArgCheck> hi(hi1, hi2, hi3, hi4, hi5, hi6, hi7, hi8, hi9, hi10);

    Range<int, 10, ArrayBoostTestArgCheck> test(lo, hi);

    Range<int, 1, ArrayBoostTestArgCheck> testProject1 = test.project<1>();
    Range<int, 2, ArrayBoostTestArgCheck> testProject2 = test.project<2>();
    Range<int, 3, ArrayBoostTestArgCheck> testProject3 = test.project<3>();
    Range<int, 4, ArrayBoostTestArgCheck> testProject4 = test.project<4>();
    Range<int, 5, ArrayBoostTestArgCheck> testProject5 = test.project<5>();
    Range<int, 6, ArrayBoostTestArgCheck> testProject6 = test.project<6>();
    Range<int, 7, ArrayBoostTestArgCheck> testProject7 = test.project<7>();
    Range<int, 8, ArrayBoostTestArgCheck> testProject8 = test.project<8>();
    Range<int, 9, ArrayBoostTestArgCheck> testProject9 = test.project<9>();

    BOOST_CHECK_EQUAL(testProject1.getLo()[0], test.getLo()[0]);
    for (int i=0; i<2; ++i) BOOST_CHECK_EQUAL(testProject2.getLo()[i], test.getLo()[i]);
    for (int i=0; i<3; ++i) BOOST_CHECK_EQUAL(testProject3.getLo()[i], test.getLo()[i]);
    for (int i=0; i<4; ++i) BOOST_CHECK_EQUAL(testProject4.getLo()[i], test.getLo()[i]);
    for (int i=0; i<5; ++i) BOOST_CHECK_EQUAL(testProject5.getLo()[i], test.getLo()[i]);
    for (int i=0; i<6; ++i) BOOST_CHECK_EQUAL(testProject6.getLo()[i], test.getLo()[i]);
    for (int i=0; i<7; ++i) BOOST_CHECK_EQUAL(testProject7.getLo()[i], test.getLo()[i]);
    for (int i=0; i<8; ++i) BOOST_CHECK_EQUAL(testProject8.getLo()[i], test.getLo()[i]);
    for (int i=0; i<9; ++i) BOOST_CHECK_EQUAL(testProject9.getLo()[i], test.getLo()[i]);

    BOOST_CHECK_EQUAL(testProject1.getHi()[0], test.getHi()[0]);
    for (int i=0; i<2; ++i) BOOST_CHECK_EQUAL(testProject2.getHi()[i], test.getHi()[i]);
    for (int i=0; i<3; ++i) BOOST_CHECK_EQUAL(testProject3.getHi()[i], test.getHi()[i]);
    for (int i=0; i<4; ++i) BOOST_CHECK_EQUAL(testProject4.getHi()[i], test.getHi()[i]);
    for (int i=0; i<5; ++i) BOOST_CHECK_EQUAL(testProject5.getHi()[i], test.getHi()[i]);
    for (int i=0; i<6; ++i) BOOST_CHECK_EQUAL(testProject6.getHi()[i], test.getHi()[i]);
    for (int i=0; i<7; ++i) BOOST_CHECK_EQUAL(testProject7.getHi()[i], test.getHi()[i]);
    for (int i=0; i<8; ++i) BOOST_CHECK_EQUAL(testProject8.getHi()[i], test.getHi()[i]);
    for (int i=0; i<9; ++i) BOOST_CHECK_EQUAL(testProject9.getHi()[i], test.getHi()[i]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( projectDim_2d, RangeTest )
{
  const int N = 10000;
  boost::progress_display show_progress(N);
  boost::random::uniform_int_distribution<> index_dist(0, 1);

  for (int i=0; i<N; i++) {
    int lo1 = idist(rGen);
    int lo2 = idist(rGen);

    int hi1 = idist(rGen);
    int hi2 = idist(rGen);

    int dim = index_dist(rGen);

    std::vector<int> indices(2);
    for (int i=0; i<2; ++i) indices[i] = i;
    indices.erase(indices.begin()+dim);

    Array<int, 2, ArrayBoostTestArgCheck> lo(lo1, lo2);
    Array<int, 2, ArrayBoostTestArgCheck> hi(hi1, hi2);
    Range<int, 2, ArrayBoostTestArgCheck> test(lo, hi);

    Range<int, 1, ArrayBoostTestArgCheck> test_projectDim = test.projectDim(dim);

    BOOST_CHECK_EQUAL(test_projectDim.getLo()[0], test.getLo()[indices[0]]);
    BOOST_CHECK_EQUAL(test_projectDim.getHi()[0], test.getHi()[indices[0]]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( projectDim_3d, RangeTest )
{
  const int N = 10000;
  boost::progress_display show_progress(N);
  boost::random::uniform_int_distribution<> index_dist(0, 2);

  for (int i=0; i<N; i++) {
    int lo1 = idist(rGen);
    int lo2 = idist(rGen);
    int lo3 = idist(rGen);

    int hi1 = idist(rGen);
    int hi2 = idist(rGen);
    int hi3 = idist(rGen);

    int dim = index_dist(rGen);

    std::vector<int> indices(3);
    for (int i=0; i<3; ++i) indices[i] = i;
    indices.erase(indices.begin()+dim);

    Array<int, 3, ArrayBoostTestArgCheck> lo(lo1, lo2, lo3);
    Array<int, 3, ArrayBoostTestArgCheck> hi(hi1, hi2, hi3);
    Range<int, 3, ArrayBoostTestArgCheck> test(lo, hi);

    Range<int, 2, ArrayBoostTestArgCheck> test_projectDim = test.projectDim(dim);

    for (int i=0; i<2; ++i)
    {
      BOOST_CHECK_EQUAL(test_projectDim.getLo()[i], test.getLo()[indices[i]]);
      BOOST_CHECK_EQUAL(test_projectDim.getHi()[i], test.getHi()[indices[i]]);
    }

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( projectDim_4d, RangeTest )
{
  const int N = 10000;
  boost::progress_display show_progress(N);
  boost::random::uniform_int_distribution<> index_dist(0, 3);

  for (int i=0; i<N; i++) {
    int lo1 = idist(rGen);
    int lo2 = idist(rGen);
    int lo3 = idist(rGen);
    int lo4 = idist(rGen);

    int hi1 = idist(rGen);
    int hi2 = idist(rGen);
    int hi3 = idist(rGen);
    int hi4 = idist(rGen);

    int dim = index_dist(rGen);

    std::vector<int> indices(4);
    for (int i=0; i<4; ++i) indices[i] = i;
    indices.erase(indices.begin()+dim);

    Array<int, 4, ArrayBoostTestArgCheck> lo(lo1, lo2, lo3, lo4);
    Array<int, 4, ArrayBoostTestArgCheck> hi(hi1, hi2, hi3, hi4);
    Range<int, 4, ArrayBoostTestArgCheck> test(lo, hi);

    Range<int, 3, ArrayBoostTestArgCheck> test_projectDim = test.projectDim(dim);

    for (int i=0; i<3; ++i)
    {
      BOOST_CHECK_EQUAL(test_projectDim.getLo()[i], test.getLo()[indices[i]]);
      BOOST_CHECK_EQUAL(test_projectDim.getHi()[i], test.getHi()[indices[i]]);
    }

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( projectDim_5d, RangeTest )
{
  const int N = 10000;
  boost::progress_display show_progress(N);
  boost::random::uniform_int_distribution<> index_dist(0, 4);

  for (int i=0; i<N; i++) {
    int lo1 = idist(rGen);
    int lo2 = idist(rGen);
    int lo3 = idist(rGen);
    int lo4 = idist(rGen);
    int lo5 = idist(rGen);

    int hi1 = idist(rGen);
    int hi2 = idist(rGen);
    int hi3 = idist(rGen);
    int hi4 = idist(rGen);
    int hi5 = idist(rGen);

    int dim = index_dist(rGen);

    std::vector<int> indices(5);
    for (int i=0; i<5; ++i) indices[i] = i;
    indices.erase(indices.begin()+dim);

    Array<int, 5, ArrayBoostTestArgCheck> lo(lo1, lo2, lo3, lo4, lo5);
    Array<int, 5, ArrayBoostTestArgCheck> hi(hi1, hi2, hi3, hi4, hi5);
    Range<int, 5, ArrayBoostTestArgCheck> test(lo, hi);

    Range<int, 4, ArrayBoostTestArgCheck> test_projectDim = test.projectDim(dim);

    for (int i=0; i<4; ++i)
    {
      BOOST_CHECK_EQUAL(test_projectDim.getLo()[i], test.getLo()[indices[i]]);
      BOOST_CHECK_EQUAL(test_projectDim.getHi()[i], test.getHi()[indices[i]]);
    }

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( projectDim_6d, RangeTest )
{
  const int N = 10000;
  boost::progress_display show_progress(N);
  boost::random::uniform_int_distribution<> index_dist(0, 5);

  for (int i=0; i<N; i++) {
    int lo1 = idist(rGen);
    int lo2 = idist(rGen);
    int lo3 = idist(rGen);
    int lo4 = idist(rGen);
    int lo5 = idist(rGen);
    int lo6 = idist(rGen);

    int hi1 = idist(rGen);
    int hi2 = idist(rGen);
    int hi3 = idist(rGen);
    int hi4 = idist(rGen);
    int hi5 = idist(rGen);
    int hi6 = idist(rGen);

    int dim = index_dist(rGen);

    std::vector<int> indices(6);
    for (int i=0; i<6; ++i) indices[i] = i;
    indices.erase(indices.begin()+dim);

    Array<int, 6, ArrayBoostTestArgCheck> lo(lo1, lo2, lo3, lo4, lo5, lo6);
    Array<int, 6, ArrayBoostTestArgCheck> hi(hi1, hi2, hi3, hi4, hi5, hi6);
    Range<int, 6, ArrayBoostTestArgCheck> test(lo, hi);

    Range<int, 5, ArrayBoostTestArgCheck> test_projectDim = test.projectDim(dim);

    for (int i=0; i<5; ++i)
    {
      BOOST_CHECK_EQUAL(test_projectDim.getLo()[i], test.getLo()[indices[i]]);
      BOOST_CHECK_EQUAL(test_projectDim.getHi()[i], test.getHi()[indices[i]]);
    }

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( projectDim_7d, RangeTest )
{
  const int N = 10000;
  boost::progress_display show_progress(N);
  boost::random::uniform_int_distribution<> index_dist(0, 6);

  for (int i=0; i<N; i++) {
    int lo1 = idist(rGen);
    int lo2 = idist(rGen);
    int lo3 = idist(rGen);
    int lo4 = idist(rGen);
    int lo5 = idist(rGen);
    int lo6 = idist(rGen);
    int lo7 = idist(rGen);

    int hi1 = idist(rGen);
    int hi2 = idist(rGen);
    int hi3 = idist(rGen);
    int hi4 = idist(rGen);
    int hi5 = idist(rGen);
    int hi6 = idist(rGen);
    int hi7 = idist(rGen);

    int dim = index_dist(rGen);

    std::vector<int> indices(7);
    for (int i=0; i<7; ++i) indices[i] = i;
    indices.erase(indices.begin()+dim);

    Array<int, 7, ArrayBoostTestArgCheck> lo(lo1, lo2, lo3, lo4, lo5, lo6, lo7);
    Array<int, 7, ArrayBoostTestArgCheck> hi(hi1, hi2, hi3, hi4, hi5, hi6, hi7);
    Range<int, 7, ArrayBoostTestArgCheck> test(lo, hi);

    Range<int, 6, ArrayBoostTestArgCheck> test_projectDim = test.projectDim(dim);

    for (int i=0; i<6; ++i)
    {
      BOOST_CHECK_EQUAL(test_projectDim.getLo()[i], test.getLo()[indices[i]]);
      BOOST_CHECK_EQUAL(test_projectDim.getHi()[i], test.getHi()[indices[i]]);
    }

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( projectDim_8d, RangeTest )
{
  const int N = 10000;
  boost::progress_display show_progress(N);
  boost::random::uniform_int_distribution<> index_dist(0, 7);

  for (int i=0; i<N; i++) {
    int lo1 = idist(rGen);
    int lo2 = idist(rGen);
    int lo3 = idist(rGen);
    int lo4 = idist(rGen);
    int lo5 = idist(rGen);
    int lo6 = idist(rGen);
    int lo7 = idist(rGen);
    int lo8 = idist(rGen);

    int hi1 = idist(rGen);
    int hi2 = idist(rGen);
    int hi3 = idist(rGen);
    int hi4 = idist(rGen);
    int hi5 = idist(rGen);
    int hi6 = idist(rGen);
    int hi7 = idist(rGen);
    int hi8 = idist(rGen);

    int dim = index_dist(rGen);

    std::vector<int> indices(8);
    for (int i=0; i<8; ++i) indices[i] = i;
    indices.erase(indices.begin()+dim);

    Array<int, 8, ArrayBoostTestArgCheck> lo(lo1, lo2, lo3, lo4, lo5, lo6, lo7, lo8);
    Array<int, 8, ArrayBoostTestArgCheck> hi(hi1, hi2, hi3, hi4, hi5, hi6, hi7, hi8);
    Range<int, 8, ArrayBoostTestArgCheck> test(lo, hi);

    Range<int, 7, ArrayBoostTestArgCheck> test_projectDim = test.projectDim(dim);

    for (int i=0; i<7; ++i)
    {
      BOOST_CHECK_EQUAL(test_projectDim.getLo()[i], test.getLo()[indices[i]]);
      BOOST_CHECK_EQUAL(test_projectDim.getHi()[i], test.getHi()[indices[i]]);
    }

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( projectDim_9d, RangeTest )
{
  const int N = 10000;
  boost::progress_display show_progress(N);
  boost::random::uniform_int_distribution<> index_dist(0, 8);

  for (int i=0; i<N; i++) {
    int lo1 = idist(rGen);
    int lo2 = idist(rGen);
    int lo3 = idist(rGen);
    int lo4 = idist(rGen);
    int lo5 = idist(rGen);
    int lo6 = idist(rGen);
    int lo7 = idist(rGen);
    int lo8 = idist(rGen);
    int lo9 = idist(rGen);

    int hi1 = idist(rGen);
    int hi2 = idist(rGen);
    int hi3 = idist(rGen);
    int hi4 = idist(rGen);
    int hi5 = idist(rGen);
    int hi6 = idist(rGen);
    int hi7 = idist(rGen);
    int hi8 = idist(rGen);
    int hi9 = idist(rGen);

    int dim = index_dist(rGen);

    std::vector<int> indices(9);
    for (int i=0; i<9; ++i) indices[i] = i;
    indices.erase(indices.begin()+dim);

    Array<int, 9, ArrayBoostTestArgCheck> lo(lo1, lo2, lo3, lo4, lo5, lo6, lo7, lo8, lo9);
    Array<int, 9, ArrayBoostTestArgCheck> hi(hi1, hi2, hi3, hi4, hi5, hi6, hi7, hi8, hi9);
    Range<int, 9, ArrayBoostTestArgCheck> test(lo, hi);

    Range<int, 8, ArrayBoostTestArgCheck> test_projectDim = test.projectDim(dim);

    for (int i=0; i<8; ++i)
    {
      BOOST_CHECK_EQUAL(test_projectDim.getLo()[i], test.getLo()[indices[i]]);
      BOOST_CHECK_EQUAL(test_projectDim.getHi()[i], test.getHi()[indices[i]]);
    }

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( projectDim_10d, RangeTest )
{
  const int N = 10000;
  boost::progress_display show_progress(N);
  boost::random::uniform_int_distribution<> index_dist(0, 9);

  for (int i=0; i<N; i++) {
    int lo1 = idist(rGen);
    int lo2 = idist(rGen);
    int lo3 = idist(rGen);
    int lo4 = idist(rGen);
    int lo5 = idist(rGen);
    int lo6 = idist(rGen);
    int lo7 = idist(rGen);
    int lo8 = idist(rGen);
    int lo9 = idist(rGen);
    int lo10 = idist(rGen);

    int hi1 = idist(rGen);
    int hi2 = idist(rGen);
    int hi3 = idist(rGen);
    int hi4 = idist(rGen);
    int hi5 = idist(rGen);
    int hi6 = idist(rGen);
    int hi7 = idist(rGen);
    int hi8 = idist(rGen);
    int hi9 = idist(rGen);
    int hi10 = idist(rGen);

    int dim = index_dist(rGen);

    std::vector<int> indices(10);
    for (int i=0; i<10; ++i) indices[i] = i;
    indices.erase(indices.begin()+dim);

    Array<int, 10, ArrayBoostTestArgCheck> lo(lo1, lo2, lo3, lo4, lo5, lo6, lo7, lo8, lo9, lo10);
    Array<int, 10, ArrayBoostTestArgCheck> hi(hi1, hi2, hi3, hi4, hi5, hi6, hi7, hi8, hi9, hi10);
    Range<int, 10, ArrayBoostTestArgCheck> test(lo, hi);

    Range<int, 9, ArrayBoostTestArgCheck> test_projectDim = test.projectDim(dim);

    for (int i=0; i<9; ++i)
    {
      BOOST_CHECK_EQUAL(test_projectDim.getLo()[i], test.getLo()[indices[i]]);
      BOOST_CHECK_EQUAL(test_projectDim.getHi()[i], test.getHi()[indices[i]]);
    }

    ++show_progress;
  }
}


BOOST_AUTO_TEST_SUITE_END()

