/*
 * test_kokkos_storage.cpp
 *
 *  Created on: 10 Nov 2022
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

#ifdef SCHNEK_HAVE_KOKKOS

#ifdef KOKKOS_ENABLE_CUDA    
    template <typename T, size_t rank>
    using GridStorage = schnek::KokkosGridStorage<T, rank, Kokkos::CudaHostPinnedSpace>;
#else
    template <typename T, size_t rank>
    using GridStorage = schnek::KokkosGridStorage<T, rank>;
#endif

BOOST_AUTO_TEST_SUITE( grid )

BOOST_AUTO_TEST_SUITE( kokkos_storage )

BOOST_FIXTURE_TEST_CASE( access_1d, GridTest )
{
  typedef schnek::Grid<double, 1, GridBoostTestCheck, GridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(25);
  for (int n=0; n<5; ++n)
  {
    random_extent<1>(lo, hi);
    GridType g(lo, hi);
    test_access_1d(g);
    for (int m=0; m<5; ++m)
    {
      random_extent<1>(lo, hi);
      g.resize(lo, hi);
      test_access_1d(g);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( access_2d, GridTest )
{
  typedef schnek::Grid<double, 2, GridBoostTestCheck, GridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(25);
  for (int n=0; n<5; ++n)
  {
    random_extent<2>(lo, hi);
    GridType g(lo,hi);
    test_access_2d(g);
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
  typedef schnek::Grid<double, 3, GridBoostTestCheck, GridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(25);
  for (int n=0; n<5; ++n)
  {
    random_extent<3>(lo, hi);
    GridType g(lo,hi);
    test_access_3d(g);
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
  typedef schnek::Grid<double, 4, GridBoostTestCheck, GridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(25);
  for (int n=0; n<5; ++n)
  {
    random_extent<4>(lo, hi);
    GridType g(lo,hi);
    test_access_4d(g);
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
  typedef schnek::Grid<double, 5, GridBoostTestCheck, GridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(25);
  for (int n=0; n<5; ++n)
  {
    random_extent<5>(lo, hi);
    GridType g(lo,hi);
    test_access_5d(g);
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
  typedef schnek::Grid<double, 6, GridBoostTestCheck, GridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(25);
  for (int n=0; n<5; ++n)
  {
    random_extent<6>(lo, hi);
    GridType g(lo,hi);
    test_access_6d(g);
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
  typedef schnek::Grid<double, 7, GridBoostTestCheck, GridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(25);
  for (int n=0; n<5; ++n)
  {
    random_extent<7>(lo, hi);
    GridType g(lo,hi);
    test_access_7d(g);
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
  typedef schnek::Grid<double, 8, GridBoostTestCheck, GridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(25);
  for (int n=0; n<5; ++n)
  {
    random_extent<8>(lo, hi);
    GridType g(lo,hi);
    test_access_8d(g);
    for (int m=0; m<5; ++m)
    {
      random_extent<8>(lo, hi);
      g.resize(lo,hi);
      test_access_8d(g);
      ++show_progress;
    }
  }
}

BOOST_FIXTURE_TEST_CASE( range_access_1d, GridTest )
{
  typedef schnek::Grid<double, 1, GridBoostTestCheck, GridStorage> GridType;
  generic_range_access_Nd<1, GridType>();
}

BOOST_FIXTURE_TEST_CASE( range_access_2d, GridTest )
{
  typedef schnek::Grid<double, 2, GridBoostTestCheck, GridStorage> GridType;
  generic_range_access_Nd<2, GridType>();
}

BOOST_FIXTURE_TEST_CASE( range_access_3d, GridTest )
{
  typedef schnek::Grid<double, 3, GridBoostTestCheck, GridStorage> GridType;
  generic_range_access_Nd<3, GridType>();
}

BOOST_FIXTURE_TEST_CASE( range_access_4d, GridTest )
{
  typedef schnek::Grid<double, 4, GridBoostTestCheck, GridStorage> GridType;
  generic_range_access_Nd<4, GridType>();
}

BOOST_FIXTURE_TEST_CASE( range_access_5d, GridTest )
{
  typedef schnek::Grid<double, 5, GridBoostTestCheck, GridStorage> GridType;
  generic_range_access_Nd<5, GridType>();
}

BOOST_FIXTURE_TEST_CASE( range_access_6d, GridTest )
{
  typedef schnek::Grid<double, 6, GridBoostTestCheck, GridStorage> GridType;
  generic_range_access_Nd<6, GridType>();
}

BOOST_FIXTURE_TEST_CASE( range_access_7d, GridTest )
{
  typedef schnek::Grid<double, 7, GridBoostTestCheck, GridStorage> GridType;
  generic_range_access_Nd<7, GridType>();
}

BOOST_FIXTURE_TEST_CASE( range_access_8d, GridTest )
{
  typedef schnek::Grid<double, 8, GridBoostTestCheck, GridStorage> GridType;
  generic_range_access_Nd<8, GridType>();
}

BOOST_FIXTURE_TEST_CASE( stride_1d, GridTest )
{
  typedef schnek::Grid<double, 1, GridBoostTestCheck, GridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent<1>(lo, hi);

    GridType g(lo, hi);
    hi = hi - (ptrdiff_t)1;
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
  typedef schnek::Grid<double, 2, GridBoostTestCheck, GridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent(lo, hi);

    GridType g(lo, hi);
    hi = hi - (ptrdiff_t)1;
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
  typedef schnek::Grid<double, 3, GridBoostTestCheck, GridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent(lo, hi);

    GridType g(lo, hi);
    hi = hi - (ptrdiff_t)1;
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
  typedef schnek::Grid<double, 4, GridBoostTestCheck, GridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent(lo, hi);

    GridType g(lo, hi);
    hi = hi - (ptrdiff_t)1;
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
  typedef schnek::Grid<double, 5, GridBoostTestCheck, GridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent(lo, hi);

    GridType g(lo, hi);
    hi = hi - (ptrdiff_t)1;
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
  typedef schnek::Grid<double, 6, GridBoostTestCheck, GridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent(lo, hi);

    GridType g(lo, hi);
    hi = hi - (ptrdiff_t)1;
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
  typedef schnek::Grid<double, 7, GridBoostTestCheck, GridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent(lo, hi);

    GridType g(lo, hi);
    hi = hi - (ptrdiff_t)1;
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
  typedef schnek::Grid<double, 8, GridBoostTestCheck, GridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::timer::progress_display show_progress(100);
  for (int n=0; n<10; ++n)
  {
    random_extent(lo, hi);

    GridType g(lo, hi);
    hi = hi - (ptrdiff_t)1;
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
  typedef schnek::Grid<double, 3, GridBoostTestCheck, GridStorage> GridType;

  GridType::IndexType lo, hi;
  random_extent<3>(lo, hi);
  GridType g(lo,hi);
  test_copy_constructor(g);
}

BOOST_FIXTURE_TEST_CASE( assignment_operator, GridTest )
{
  typedef schnek::Grid<double, 3, GridBoostTestCheck, GridStorage> GridType;

  GridType::IndexType lo, hi;
  random_extent<3>(lo, hi);
  GridType g(lo,hi);
  test_assignment_operator(g);
}

BOOST_FIXTURE_TEST_CASE( copy_then_resize, GridTest )
{
  typedef schnek::Grid<double, 3, GridBoostTestCheck, GridStorage> GridType;

  GridType::IndexType lo, hi;
  random_extent<3>(lo, hi);
  GridType g(lo,hi);
  test_copy_resize(g);
}

BOOST_AUTO_TEST_CASE( host_accessible_flag )
{
  bool accessible = GridStorage<double, 2>::host_accessible;
  BOOST_CHECK(accessible);
}

BOOST_FIXTURE_TEST_CASE( get_size, GridTest )
{
  typedef schnek::Grid<double, 3, GridBoostTestCheck, GridStorage> GridType;
  GridType::IndexType lo, hi;
  random_extent<3>(lo, hi);
  GridType g(lo, hi);

  size_t expected = 1;
  for (size_t d = 0; d < 3; ++d)
  {
    expected *= (size_t)(hi[d] - lo[d] + 1);
  }
  BOOST_CHECK_EQUAL(g.getSize(), expected);
}

BOOST_FIXTURE_TEST_CASE( kokkos_view_access, GridTest )
{
  typedef schnek::Grid<double, 2, GridBoostTestCheck, GridStorage> GridType;
  GridType::IndexType lo, hi;
  random_extent<2>(lo, hi);
  GridType g(lo, hi);

  auto &view = g.getKokkosView();
  BOOST_CHECK_EQUAL((ptrdiff_t)view.extent(0), hi[0] - lo[0] + 1);
  BOOST_CHECK_EQUAL((ptrdiff_t)view.extent(1), hi[1] - lo[1] + 1);

  // A value written through the grid is visible in the view (offset by lo)
  g(lo[0], lo[1]) = 42.0;
  BOOST_CHECK_EQUAL(view(0, 0), 42.0);
}

BOOST_FIXTURE_TEST_CASE( host_mirror_roundtrip, GridTest )
{
  typedef GridStorage<double, 2> StorageType;
  StorageType::IndexType lo, hi;
  random_extent<2>(lo, hi);
  StorageType storage(lo, hi);

  storage.get(lo) = 2.71;
  auto mirror = storage.createHostMirror();
  storage.deepCopyToHost(mirror);
  BOOST_CHECK_EQUAL(mirror(0, 0), 2.71);

  mirror(0, 0) = 1.41;
  storage.deepCopyFromHost(mirror);
  BOOST_CHECK_EQUAL(storage.get(lo), 1.41);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

#endif