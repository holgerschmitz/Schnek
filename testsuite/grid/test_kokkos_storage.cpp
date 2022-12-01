/*
 * test_grid.cpp
 *
 *  Created on: 1 Jan 2014
 *      Author: Holger Schmitz
 */

#include "../utility.hpp"
#include "grid_test_fixture.hpp"

#include <grid/grid.hpp>
#include <grid/gridstorage/kokkos-storage.hpp>

#include <boost/timer/progress_display.hpp>
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <limits>

struct KokkosStorageTest : public GridTest
{
    KokkosStorageTest() : GridTest() {
        Kokkos::InitArguments args;
        args.num_threads = 0;
        args.num_numa = 0;
        Kokkos::initialize(args);
    }

    ~KokkosStorageTest() {
        Kokkos::finalize();
    }
};

BOOST_AUTO_TEST_SUITE( grid )

BOOST_AUTO_TEST_SUITE( kokkos_storage )

BOOST_FIXTURE_TEST_CASE( access_1d, KokkosStorageTest )
{
  typedef schnek::Grid<double, 1, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent<1>(lo, hi);
    GridType g(lo, hi);
    test_access_1d(g);
    for (int m=0; m<10; ++m)
    {
      random_extent<1>(lo, hi);
      g.resize(lo, hi);
      test_access_1d(g);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( access_2d, KokkosStorageTest )
{
  typedef schnek::Grid<double, 2, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent<2>(lo, hi);
    GridType g(lo,hi);
    test_access_2d(g);
    for (int m=0; m<10; ++m)
    {
      random_extent<2>(lo, hi);
      g.resize(lo,hi);
      test_access_2d(g);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( access_3d, KokkosStorageTest )
{
  typedef schnek::Grid<double, 3, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent<3>(lo, hi);
    GridType g(lo,hi);
    test_access_3d(g);
    for (int m=0; m<10; ++m)
    {
      random_extent<3>(lo, hi);
      g.resize(lo,hi);
      test_access_3d(g);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( access_4d, KokkosStorageTest )
{
  typedef schnek::Grid<double, 4, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent<4>(lo, hi);
    GridType g(lo,hi);
    test_access_4d(g);
    for (int m=0; m<10; ++m)
    {
      random_extent<4>(lo, hi);
      g.resize(lo,hi);
      test_access_4d(g);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( access_5d, KokkosStorageTest )
{
  typedef schnek::Grid<double, 5, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent<5>(lo, hi);
    GridType g(lo,hi);
    test_access_5d(g);
    for (int m=0; m<10; ++m)
    {
      random_extent<5>(lo, hi);
      g.resize(lo,hi);
      test_access_5d(g);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( access_6d, KokkosStorageTest )
{
  typedef schnek::Grid<double, 6, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent<6>(lo, hi);
    GridType g(lo,hi);
    test_access_6d(g);
    for (int m=0; m<10; ++m)
    {
      random_extent<6>(lo, hi);
      g.resize(lo,hi);
      test_access_6d(g);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( access_7d, KokkosStorageTest )
{
  typedef schnek::Grid<double, 7, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent<7>(lo, hi);
    GridType g(lo,hi);
    test_access_7d(g);
    for (int m=0; m<10; ++m)
    {
      random_extent<7>(lo, hi);
      g.resize(lo,hi);
      test_access_7d(g);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( access_8d, KokkosStorageTest )
{
  typedef schnek::Grid<double, 8, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent<8>(lo, hi);
    GridType g(lo,hi);
    test_access_8d(g);
    for (int m=0; m<10; ++m)
    {
      random_extent<8>(lo, hi);
      g.resize(lo,hi);
      test_access_8d(g);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( stride_1d, KokkosStorageTest )
{
  typedef schnek::Grid<double, 1, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;
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

BOOST_FIXTURE_TEST_CASE( stride_2d, KokkosStorageTest )
{
  typedef schnek::Grid<double, 2, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;
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

BOOST_FIXTURE_TEST_CASE( stride_3d, KokkosStorageTest )
{
  typedef schnek::Grid<double, 3, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;
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

BOOST_FIXTURE_TEST_CASE( stride_4d, KokkosStorageTest )
{
  typedef schnek::Grid<double, 4, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;
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

BOOST_FIXTURE_TEST_CASE( stride_5d, KokkosStorageTest )
{
  typedef schnek::Grid<double, 5, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;
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

BOOST_FIXTURE_TEST_CASE( stride_6d, KokkosStorageTest )
{
  typedef schnek::Grid<double, 6, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;
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

BOOST_FIXTURE_TEST_CASE( stride_7d, KokkosStorageTest )
{
  typedef schnek::Grid<double, 7, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;
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

BOOST_FIXTURE_TEST_CASE( stride_8d, KokkosStorageTest )
{
  typedef schnek::Grid<double, 8, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;
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

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
