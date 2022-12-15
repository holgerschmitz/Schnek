/*
 * grid_test_fixture.hpp
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

#ifndef SCHNEK_TEST_GRID_GRIDTESTFIXTURE
#define SCHNEK_TEST_GRID_GRIDTESTFIXTURE

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>

struct GridTest
{
    boost::random::mt19937 rGen;
    boost::random::uniform_real_distribution<> dist;

    GridTest() : dist(-1.0,1.0) {}

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

    template<class GridType>
    void test_stride(GridType &grid, const typename GridType::IndexType &index)
    {
      for (size_t i=0; i<GridType::Rank; ++i)
      {
        typename GridType::IndexType indexP = index;
        indexP[i] = index[i] + 1;
        ptrdiff_t diff = &(grid.get(indexP)) - &(grid.get(index));
        ptrdiff_t stride = grid.stride(i);
        BOOST_CHECK(is_equal(diff, stride));
      }
    }

    template<size_t rank>
    void random_extent(schnek::Array<int,rank> &lo, schnek::Array<int,rank> &hi)
    {
      const int maxExtent = (int)pow(20000,(1.0/(double)rank));
      boost::random::uniform_int_distribution<> orig(-maxExtent/2, maxExtent/2);
      boost::random::uniform_int_distribution<> extent(1, maxExtent);
      for (size_t i=0; i<rank; ++i)
      {
        int o = orig(rGen);
        int l = extent(rGen);
        lo[i] = o;
        hi[i] = o+l;
      }
    }

    template<size_t rank>
    schnek::Array<int,rank> random_index(const schnek::Array<int,rank> &lo, const schnek::Array<int,rank> &hi)
    {
      schnek::Array<int,rank> index;
      for (size_t i=0; i<rank; ++i)
      {
        boost::random::uniform_int_distribution<> indexDist(lo[i], hi[i]);
        index[i] = indexDist(rGen);
      }
      return index;
    }
};

#endif