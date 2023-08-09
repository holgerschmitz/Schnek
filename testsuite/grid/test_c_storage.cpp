/*
 * test_c_storage.cpp
 *
 *  Created on: 1 Dec 2022
 *      Author: Holger Schmitz
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

#include "../utility.hpp"
#include "grid_test_fixture.hpp"

#include <grid/grid.hpp>

#include <boost/timer/progress_display.hpp>
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <limits>

BOOST_AUTO_TEST_SUITE( grid )

BOOST_AUTO_TEST_SUITE( c_storage )

BOOST_FIXTURE_TEST_CASE( access_1d, GridTest )
{
  typedef schnek::Grid<double, 1, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(30);
  for (int n=0; n<5; ++n)
  {
    random_extent<1>(lo, hi);
    GridType g(lo,hi);
    test_access_1d(g);
    ++show_progress;
    for (int m=0; m<5; ++m)
    {
      random_extent<1>(lo, hi);
      g.resize(lo,hi);
      test_access_1d(g);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( access_2d, GridTest )
{
  typedef schnek::Grid<double, 2, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(30);
  for (int n=0; n<5; ++n)
  {
    random_extent<2>(lo, hi);
    GridType g(lo,hi);
    test_access_2d(g);
    ++show_progress;
    for (int m=0; m<5; ++m)
    {
      random_extent<2>(lo, hi);
      g.resize(lo,hi);
      test_access_2d(g);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( access_3d, GridTest )
{
  typedef schnek::Grid<double, 3, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(30);
  for (int n=0; n<5; ++n)
  {
    random_extent<3>(lo, hi);
    GridType g(lo,hi);
    test_access_3d(g);
    ++show_progress;
    for (int m=0; m<5; ++m)
    {
      random_extent<3>(lo, hi);
      g.resize(lo,hi);
      test_access_3d(g);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( access_4d, GridTest )
{
  typedef schnek::Grid<double, 4, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(30);
  for (int n=0; n<5; ++n)
  {
    random_extent<4>(lo, hi);
    GridType g(lo,hi);
    test_access_4d(g);
    ++show_progress;
    for (int m=0; m<5; ++m)
    {
      random_extent<4>(lo, hi);
      g.resize(lo,hi);
      test_access_4d(g);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( access_5d, GridTest )
{
  typedef schnek::Grid<double, 5, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(30);
  for (int n=0; n<5; ++n)
  {
    random_extent<5>(lo, hi);
    GridType g(lo,hi);
    test_access_5d(g);
    ++show_progress;
    for (int m=0; m<5; ++m)
    {
      random_extent<5>(lo, hi);
      g.resize(lo,hi);
      test_access_5d(g);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( access_6d, GridTest )
{
  typedef schnek::Grid<double, 6, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(30);
  for (int n=0; n<5; ++n)
  {
    random_extent<6>(lo, hi);
    GridType g(lo,hi);
    test_access_6d(g);
    ++show_progress;
    for (int m=0; m<5; ++m)
    {
      random_extent<6>(lo, hi);
      g.resize(lo,hi);
      test_access_6d(g);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( access_7d, GridTest )
{
  typedef schnek::Grid<double, 7, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(30);
  for (int n=0; n<5; ++n)
  {
    random_extent<7>(lo, hi);
    GridType g(lo,hi);
    test_access_7d(g);
    ++show_progress;
    for (int m=0; m<5; ++m)
    {
      random_extent<7>(lo, hi);
      g.resize(lo,hi);
      test_access_7d(g);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( access_8d, GridTest )
{
  typedef schnek::Grid<double, 8, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(30);
  for (int n=0; n<5; ++n)
  {
    random_extent<8>(lo, hi);
    GridType g(lo,hi);
    test_access_8d(g);
    ++show_progress;
    for (int m=0; m<5; ++m)
    {
      random_extent<8>(lo, hi);
      g.resize(lo,hi);
      test_access_8d(g);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( access_9d, GridTest )
{
  typedef schnek::Grid<double, 9, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(30);
  for (int n=0; n<5; ++n)
  {
    random_extent<9>(lo, hi);
    GridType g(lo,hi);
    test_access_9d(g);
    ++show_progress;
    for (int m=0; m<5; ++m)
    {
      random_extent<9>(lo, hi);
      g.resize(lo,hi);
      test_access_9d(g);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( access_10d, GridTest )
{
  typedef schnek::Grid<double, 10, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(30);
  for (int n=0; n<5; ++n)
  {
    random_extent<10>(lo, hi);
    GridType g(lo,hi);
    test_access_10d(g);
    ++show_progress;
    for (int m=0; m<5; ++m)
    {
      random_extent<10>(lo, hi);
      g.resize(lo,hi);
      test_access_10d(g);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( range_access_1d, GridTest )
{
  typedef schnek::Grid<double, 1, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  generic_range_access_Nd<1, GridType>();
}

BOOST_FIXTURE_TEST_CASE( range_access_2d, GridTest )
{
  typedef schnek::Grid<double, 2, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  generic_range_access_Nd<2, GridType>();
}

BOOST_FIXTURE_TEST_CASE( range_access_3d, GridTest )
{
  typedef schnek::Grid<double, 3, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  generic_range_access_Nd<3, GridType>();
}

BOOST_FIXTURE_TEST_CASE( range_access_4d, GridTest )
{
  typedef schnek::Grid<double, 4, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  generic_range_access_Nd<4, GridType>();
}

BOOST_FIXTURE_TEST_CASE( range_access_5d, GridTest )
{
  typedef schnek::Grid<double, 5, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  generic_range_access_Nd<5, GridType>();
}

BOOST_FIXTURE_TEST_CASE( range_access_6d, GridTest )
{
  typedef schnek::Grid<double, 6, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  generic_range_access_Nd<6, GridType>();
}

BOOST_FIXTURE_TEST_CASE( range_access_7d, GridTest )
{
  typedef schnek::Grid<double, 7, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  generic_range_access_Nd<7, GridType>();
}

BOOST_FIXTURE_TEST_CASE( range_access_8d, GridTest )
{
  typedef schnek::Grid<double, 8, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  generic_range_access_Nd<8, GridType>();
}

BOOST_FIXTURE_TEST_CASE( range_access_9d, GridTest )
{
  typedef schnek::Grid<double, 9, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  generic_range_access_Nd<9, GridType>();
}

BOOST_FIXTURE_TEST_CASE( range_access_10d, GridTest )
{
  typedef schnek::Grid<double, 10, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  generic_range_access_Nd<10, GridType>();
}

BOOST_FIXTURE_TEST_CASE( stride_1d, GridTest )
{
  typedef schnek::Grid<double, 1, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent<1>(lo, hi);

    GridType g(lo, hi);
    hi = hi - 1;
    for (int m=0; m<10; ++m)
    {
      GridType::IndexType index = random_index(lo, hi);
      test_stride(g, index);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( stride_2d, GridTest )
{
  typedef schnek::Grid<double, 2, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent(lo, hi);

    GridType g(lo, hi);
    hi = hi - 1;
    for (int m=0; m<10; ++m)
    {
      GridType::IndexType index = random_index(lo, hi);
      test_stride(g, index);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( stride_3d, GridTest )
{
  typedef schnek::Grid<double, 3, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent(lo, hi);

    GridType g(lo, hi);
    hi = hi - 1;
    for (int m=0; m<10; ++m)
    {
      GridType::IndexType index = random_index(lo, hi);
      test_stride(g, index);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( stride_4d, GridTest )
{
  typedef schnek::Grid<double, 4, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent(lo, hi);

    GridType g(lo, hi);
    hi = hi - 1;
    for (int m=0; m<10; ++m)
    {
      GridType::IndexType index = random_index(lo, hi);
      test_stride(g, index);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( stride_5d, GridTest )
{
  typedef schnek::Grid<double, 5, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent(lo, hi);

    GridType g(lo, hi);
    hi = hi - 1;
    for (int m=0; m<10; ++m)
    {
      GridType::IndexType index = random_index(lo, hi);
      test_stride(g, index);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( stride_6d, GridTest )
{
  typedef schnek::Grid<double, 6, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent(lo, hi);

    GridType g(lo, hi);
    hi = hi - 1;
    for (int m=0; m<10; ++m)
    {
      GridType::IndexType index = random_index(lo, hi);
      test_stride(g, index);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( stride_7d, GridTest )
{
  typedef schnek::Grid<double, 7, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent(lo, hi);

    GridType g(lo, hi);
    hi = hi - 1;
    for (int m=0; m<10; ++m)
    {
      GridType::IndexType index = random_index(lo, hi);
      test_stride(g, index);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( stride_8d, GridTest )
{
  typedef schnek::Grid<double, 8, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent(lo, hi);

    GridType g(lo, hi);
    hi = hi - 1;
    for (int m=0; m<10; ++m)
    {
      GridType::IndexType index = random_index(lo, hi);
      test_stride(g, index);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( stride_9d, GridTest )
{
  typedef schnek::Grid<double, 9, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent(lo, hi);

    GridType g(lo, hi);
    hi = hi - 1;
    for (int m=0; m<10; ++m)
    {
      GridType::IndexType index = random_index(lo, hi);
      test_stride(g, index);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( stride_10d, GridTest )
{
  typedef schnek::Grid<double, 10, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent(lo, hi);

    GridType g(lo, hi);
    hi = hi - 1;
    for (int m=0; m<10; ++m)
    {
      GridType::IndexType index = random_index(lo, hi);
      test_stride(g, index);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( copy_constructor, GridTest )
{
  typedef schnek::Grid<double, 3, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;

  GridType::IndexType lo, hi;
  random_extent<3>(lo, hi);
  GridType g(lo,hi);
  test_copy_constructor(g);
}

BOOST_FIXTURE_TEST_CASE( assignment_operator, GridTest )
{
  typedef schnek::Grid<double, 3, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;

  GridType::IndexType lo, hi;
  random_extent<3>(lo, hi);
  GridType g(lo,hi);
  test_assignment_operator(g);
}

BOOST_FIXTURE_TEST_CASE( copy_then_resize, GridTest )
{
  typedef schnek::Grid<double, 3, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;

  GridType::IndexType lo, hi;
  random_extent<3>(lo, hi);
  GridType g(lo,hi);
  test_copy_resize(g);
}

BOOST_FIXTURE_TEST_CASE( free_shared, GridTest )
{
  typedef schnek::Grid<DeleteCounter, 1, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;
  std::map<int, int> counters;

  DeleteCounter del1(1, counters);
  DeleteCounter del2(2, counters);

  GridType::IndexType lo(0), hi(10);
  GridType *ga = new GridType(lo, hi);
  GridType *gb = new GridType(lo, hi);
  GridType *gc = new GridType(*ga);
  *ga = del1;
  *gb = del2;

  BOOST_CHECK_EQUAL(counters.count(1), 0ul);
  BOOST_CHECK_EQUAL(counters.count(2), 0ul);

  *gb = *ga;

  BOOST_CHECK_EQUAL(counters.count(1), 0ul);
  BOOST_CHECK_EQUAL(counters.count(2), 1ul);
  BOOST_CHECK_EQUAL(counters[2], 11);
  
  delete ga;
  
  BOOST_CHECK_EQUAL(counters.count(1), 0ul);
  BOOST_CHECK_EQUAL(counters.count(2), 1ul);
  BOOST_CHECK_EQUAL(counters[2], 11);
  
  delete gc;
  
  BOOST_CHECK_EQUAL(counters.count(1), 0ul);
  BOOST_CHECK_EQUAL(counters.count(2), 1ul);
  BOOST_CHECK_EQUAL(counters[2], 11);
  
  delete gb;
  
  BOOST_CHECK_EQUAL(counters.count(1), 1ul);
  BOOST_CHECK_EQUAL(counters.count(2), 1ul);
  BOOST_CHECK_EQUAL(counters[1], 11);
  BOOST_CHECK_EQUAL(counters[2], 11);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()