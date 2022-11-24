/*
 * test_grid.cpp
 *
 *  Created on: 1 Jan 2014
 *      Author: Holger Schmitz
 */

#include <grid/grid.hpp>
#include <grid/gridstorage/kokkos-storage.hpp>

#include "../utility.hpp"

#include <iostream>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/progress.hpp>

#include <limits>

#include <boost/test/unit_test.hpp>


struct KokkosStorageTest
{
    boost::random::mt19937 rGen;
    boost::random::uniform_real_distribution<> dist;

    KokkosStorageTest() : dist(-1.0,1.0) {
        Kokkos::InitArguments args;
        args.num_threads = 0;
        args.num_numa = 0;
        Kokkos::initialize(args);
    }

    ~KokkosStorageTest() {
        Kokkos::finalize();
    }

    template<class GridType>
    void test_access_1d(GridType &grid)
    {
      double sum_direct = 0.0;

      typename GridType::IndexType lo = grid.getLo();
      typename GridType::IndexType hi = grid.getHi();

      // write random numbers
      for (int i=lo[0]; i<=hi[0]; ++i)
      {
        double val = dist(rGen);
        grid(i) = val;
        sum_direct += val;
      }

      double sum_grid = 0.0;

      // read back random numbers
      for (int i=lo[0]; i<=hi[0]; ++i)
      {
        sum_grid += grid(i);
      }

      BOOST_CHECK(is_equal(sum_direct, sum_grid));
    }

    template<class GridType>
    void test_access_2d(GridType &grid)
    {
      double sum_direct = 0.0;

      typename GridType::IndexType lo = grid.getLo();
      typename GridType::IndexType hi = grid.getHi();

      // write random numbers
      for (int i=lo[0]; i<=hi[0]; ++i)
        for (int j=lo[1]; j<=hi[1]; ++j)
        {
          double val = dist(rGen);
          grid(i,j) = val;
          sum_direct += val;
        }

      double sum_grid = 0.0;

      // read back random numbers
      for (int i=lo[0]; i<=hi[0]; ++i)
        for (int j=lo[1]; j<=hi[1]; ++j)
        {
          sum_grid += grid(i,j);
        }

      BOOST_CHECK(is_equal(sum_direct, sum_grid));
    }

    template<class GridType>
    void test_access_3d(GridType &grid)
    {
      double sum_direct = 0.0;

      typename GridType::IndexType lo = grid.getLo();
      typename GridType::IndexType hi = grid.getHi();

      // write random numbers
      for (int i=lo[0]; i<=hi[0]; ++i)
        for (int j=lo[1]; j<=hi[1]; ++j)
          for (int k=lo[2]; k<=hi[2]; ++k)
          {
            double val = dist(rGen);
            grid(i,j,k) = val;
            sum_direct += val;
          }

      double sum_grid = 0.0;

      // read back random numbers
      for (int i=lo[0]; i<=hi[0]; ++i)
        for (int j=lo[1]; j<=hi[1]; ++j)
          for (int k=lo[2]; k<=hi[2]; ++k)
          {
            sum_grid += grid(i,j,k);
          }

      BOOST_CHECK(is_equal(sum_direct, sum_grid));
    }

    template<class GridType>
    void test_access_4d(GridType &grid)
    {
      double sum_direct = 0.0;

      typename GridType::IndexType lo = grid.getLo();
      typename GridType::IndexType hi = grid.getHi();

      // write random numbers
      for (int i=lo[0]; i<=hi[0]; ++i)
        for (int j=lo[1]; j<=hi[1]; ++j)
          for (int k=lo[2]; k<=hi[2]; ++k)
            for (int l=lo[3]; l<=hi[3]; ++l)
            {
              double val = dist(rGen);
              grid(i,j,k,l) = val;
              sum_direct += val;
            }

      double sum_grid = 0.0;

      // read back random numbers
      for (int i=lo[0]; i<=hi[0]; ++i)
        for (int j=lo[1]; j<=hi[1]; ++j)
          for (int k=lo[2]; k<=hi[2]; ++k)
          for (int l=lo[3]; l<=hi[3]; ++l)
            {
              sum_grid += grid(i,j,k,l);
            }

      BOOST_CHECK(is_equal(sum_direct, sum_grid));
    }

    template<class GridType>
    void test_access_5d(GridType &grid)
    {
      double sum_direct = 0.0;

      typename GridType::IndexType lo = grid.getLo();
      typename GridType::IndexType hi = grid.getHi();

      // write random numbers
      for (int i=lo[0]; i<=hi[0]; ++i)
        for (int j=lo[1]; j<=hi[1]; ++j)
          for (int k=lo[2]; k<=hi[2]; ++k)
            for (int l=lo[3]; l<=hi[3]; ++l)
              for (int m=lo[4]; m<=hi[4]; ++m)
              {
                double val = dist(rGen);
                grid(i,j,k,l,m) = val;
                sum_direct += val;
              }

      double sum_grid = 0.0;

      // read back random numbers
      for (int i=lo[0]; i<=hi[0]; ++i)
        for (int j=lo[1]; j<=hi[1]; ++j)
          for (int k=lo[2]; k<=hi[2]; ++k)
            for (int l=lo[3]; l<=hi[3]; ++l)
              for (int m=lo[4]; m<=hi[4]; ++m)
              {
                sum_grid += grid(i,j,k,l,m);
              }

      BOOST_CHECK(is_equal(sum_direct, sum_grid));
    }

    template<class GridType>
    void test_access_6d(GridType &grid)
    {
      double sum_direct = 0.0;

      typename GridType::IndexType lo = grid.getLo();
      typename GridType::IndexType hi = grid.getHi();

      // write random numbers
      for (int i=lo[0]; i<=hi[0]; ++i)
        for (int j=lo[1]; j<=hi[1]; ++j)
          for (int k=lo[2]; k<=hi[2]; ++k)
            for (int l=lo[3]; l<=hi[3]; ++l)
              for (int m=lo[4]; m<=hi[4]; ++m)
                for (int n=lo[5]; n<=hi[5]; ++n)
                {
                  double val = dist(rGen);
                  grid(i,j,k,l,m,n) = val;
                  sum_direct += val;
                }

      double sum_grid = 0.0;

      // read back random numbers
      for (int i=lo[0]; i<=hi[0]; ++i)
        for (int j=lo[1]; j<=hi[1]; ++j)
          for (int k=lo[2]; k<=hi[2]; ++k)
            for (int l=lo[3]; l<=hi[3]; ++l)
              for (int m=lo[4]; m<=hi[4]; ++m)
                for (int n=lo[5]; n<=hi[5]; ++n)
                {
                  sum_grid += grid(i,j,k,l,m,n);
                }

      BOOST_CHECK(is_equal(sum_direct, sum_grid));
    }

    template<class GridType>
    void test_access_7d(GridType &grid)
    {
      double sum_direct = 0.0;

      typename GridType::IndexType lo = grid.getLo();
      typename GridType::IndexType hi = grid.getHi();

      // write random numbers
      for (int i=lo[0]; i<=hi[0]; ++i)
        for (int j=lo[1]; j<=hi[1]; ++j)
          for (int k=lo[2]; k<=hi[2]; ++k)
            for (int l=lo[3]; l<=hi[3]; ++l)
              for (int m=lo[4]; m<=hi[4]; ++m)
                for (int n=lo[5]; n<=hi[5]; ++n)
                  for (int o=lo[6]; o<=hi[6]; ++o)
                  {
                    double val = dist(rGen);
                    grid(i,j,k,l,m,n,o) = val;
                    sum_direct += val;
                  }

      double sum_grid = 0.0;

      // read back random numbers
      for (int i=lo[0]; i<=hi[0]; ++i)
        for (int j=lo[1]; j<=hi[1]; ++j)
          for (int k=lo[2]; k<=hi[2]; ++k)
            for (int l=lo[3]; l<=hi[3]; ++l)
              for (int m=lo[4]; m<=hi[4]; ++m)
                for (int n=lo[5]; n<=hi[5]; ++n)
                  for (int o=lo[6]; o<=hi[6]; ++o)
                  {
                    sum_grid += grid(i,j,k,l,m,n,o);
                  }

      BOOST_CHECK(is_equal(sum_direct, sum_grid));
    }

    template<class GridType>
    void test_access_8d(GridType &grid)
    {
      double sum_direct = 0.0;

      typename GridType::IndexType lo = grid.getLo();
      typename GridType::IndexType hi = grid.getHi();

      // write random numbers
      for (int i=lo[0]; i<=hi[0]; ++i)
        for (int j=lo[1]; j<=hi[1]; ++j)
          for (int k=lo[2]; k<=hi[2]; ++k)
            for (int l=lo[3]; l<=hi[3]; ++l)
              for (int m=lo[4]; m<=hi[4]; ++m)
                for (int n=lo[5]; n<=hi[5]; ++n)
                  for (int o=lo[6]; o<=hi[6]; ++o)
                    for (int p=lo[7]; p<=hi[7]; ++p)
                    {
                      double val = dist(rGen);
                      grid(i,j,k,l,m,n,o,p) = val;
                      sum_direct += val;
                    }

      double sum_grid = 0.0;

      // read back random numbers
      for (int i=lo[0]; i<=hi[0]; ++i)
        for (int j=lo[1]; j<=hi[1]; ++j)
          for (int k=lo[2]; k<=hi[2]; ++k)
            for (int l=lo[3]; l<=hi[3]; ++l)
              for (int m=lo[4]; m<=hi[4]; ++m)
                for (int n=lo[5]; n<=hi[5]; ++n)
                  for (int o=lo[6]; o<=hi[6]; ++o)
                    for (int p=lo[7]; p<=hi[7]; ++p)
                    {
                      sum_grid += grid(i,j,k,l,m,n,o,p);
                    }

      BOOST_CHECK(is_equal(sum_direct, sum_grid));
    }

    template<class GridType>
    void test_access_9d(GridType &grid)
    {
      double sum_direct = 0.0;

      typename GridType::IndexType lo = grid.getLo();
      typename GridType::IndexType hi = grid.getHi();

      // write random numbers
      for (int i=lo[0]; i<=hi[0]; ++i)
        for (int j=lo[1]; j<=hi[1]; ++j)
          for (int k=lo[2]; k<=hi[2]; ++k)
            for (int l=lo[3]; l<=hi[3]; ++l)
              for (int m=lo[4]; m<=hi[4]; ++m)
                for (int n=lo[5]; n<=hi[5]; ++n)
                  for (int o=lo[6]; o<=hi[6]; ++o)
                    for (int p=lo[7]; p<=hi[7]; ++p)
                      for (int q=lo[8]; q<=hi[8]; ++q)
                      {
                        double val = dist(rGen);
                        grid(i,j,k,l,m,n,o,p,q) = val;
                        sum_direct += val;
                      }

      double sum_grid = 0.0;

      // read back random numbers
      for (int i=lo[0]; i<=hi[0]; ++i)
        for (int j=lo[1]; j<=hi[1]; ++j)
          for (int k=lo[2]; k<=hi[2]; ++k)
            for (int l=lo[3]; l<=hi[3]; ++l)
              for (int m=lo[4]; m<=hi[4]; ++m)
                for (int n=lo[5]; n<=hi[5]; ++n)
                  for (int o=lo[6]; o<=hi[6]; ++o)
                    for (int p=lo[7]; p<=hi[7]; ++p)
                      for (int q=lo[8]; q<=hi[8]; ++q)
                      {
                        sum_grid += grid(i,j,k,l,m,n,o,p,q);
                      }

      BOOST_CHECK(is_equal(sum_direct, sum_grid));
    }

    template<class GridType>
    void test_access_10d(GridType &grid)
    {
      double sum_direct = 0.0;

      typename GridType::IndexType lo = grid.getLo();
      typename GridType::IndexType hi = grid.getHi();

      // write random numbers
      for (int i=lo[0]; i<=hi[0]; ++i)
        for (int j=lo[1]; j<=hi[1]; ++j)
          for (int k=lo[2]; k<=hi[2]; ++k)
            for (int l=lo[3]; l<=hi[3]; ++l)
              for (int m=lo[4]; m<=hi[4]; ++m)
                for (int n=lo[5]; n<=hi[5]; ++n)
                  for (int o=lo[6]; o<=hi[6]; ++o)
                    for (int p=lo[7]; p<=hi[7]; ++p)
                      for (int q=lo[8]; q<=hi[8]; ++q)
                        for (int r=lo[9]; r<=hi[9]; ++r)
                        {
                          double val = dist(rGen);
                          grid(i,j,k,l,m,n,o,p,q,r) = val;
                          sum_direct += val;
                        }

      double sum_grid = 0.0;

      // read back random numbers
      for (int i=lo[0]; i<=hi[0]; ++i)
        for (int j=lo[1]; j<=hi[1]; ++j)
          for (int k=lo[2]; k<=hi[2]; ++k)
            for (int l=lo[3]; l<=hi[3]; ++l)
              for (int m=lo[4]; m<=hi[4]; ++m)
                for (int n=lo[5]; n<=hi[5]; ++n)
                  for (int o=lo[6]; o<=hi[6]; ++o)
                    for (int p=lo[7]; p<=hi[7]; ++p)
                      for (int q=lo[8]; q<=hi[8]; ++q)
                        for (int r=lo[9]; r<=hi[9]; ++r)
                        {
                          sum_grid += grid(i,j,k,l,m,n,o,p,q,r);
                        }

      BOOST_CHECK(is_equal(sum_direct, sum_grid));
    }

    template<int rank>
    void random_extent(schnek::Array<int,rank> &lo, schnek::Array<int,rank> &hi)
    {
      const int maxExtent = (int)pow(10000,(1.0/(double)rank));
      boost::random::uniform_int_distribution<> orig(-maxExtent/2, maxExtent/2);
      boost::random::uniform_int_distribution<> extent(1, maxExtent);
      for (int i=0; i<rank; ++i)
      {
        int o = orig(rGen);
        int l = extent(rGen);
        lo[i] = o;
        hi[i] = o+l;
      }
    }
};

BOOST_AUTO_TEST_SUITE( grid )

BOOST_FIXTURE_TEST_CASE( grid_1d_Kokkos_model, KokkosStorageTest )
{
  typedef schnek::Grid<double, 1, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::progress_display show_progress(100);
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

BOOST_FIXTURE_TEST_CASE( grid_2d_Kokkos_model, KokkosStorageTest )
{
  typedef schnek::Grid<double, 2, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::progress_display show_progress(100);
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

BOOST_FIXTURE_TEST_CASE( grid_3d_Kokkos_model, KokkosStorageTest )
{
  typedef schnek::Grid<double, 3, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::progress_display show_progress(100);
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

BOOST_FIXTURE_TEST_CASE( grid_4d_Kokkos_model, KokkosStorageTest )
{
  typedef schnek::Grid<double, 4, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::progress_display show_progress(100);
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

BOOST_FIXTURE_TEST_CASE( grid_5d_Kokkos_model, KokkosStorageTest )
{
  typedef schnek::Grid<double, 5, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::progress_display show_progress(100);
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

BOOST_FIXTURE_TEST_CASE( grid_6d_Kokkos_model, KokkosStorageTest )
{
  typedef schnek::Grid<double, 6, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::progress_display show_progress(100);
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

BOOST_FIXTURE_TEST_CASE( grid_7d_Kokkos_model, KokkosStorageTest )
{
  typedef schnek::Grid<double, 7, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::progress_display show_progress(100);
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

BOOST_FIXTURE_TEST_CASE( grid_8d_Kokkos_model, KokkosStorageTest )
{
  typedef schnek::Grid<double, 8, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;
  GridType::IndexType lo, hi;
  boost::progress_display show_progress(100);
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

BOOST_AUTO_TEST_SUITE_END()
