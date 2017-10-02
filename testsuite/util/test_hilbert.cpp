/*
 * test_hilbert.cpp
 *
 * Created on: 27 Sep 2017
 * Author: Holger Schmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2012 Holger Schmitz
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


#include <grid/array.hpp>
#include <grid/grid.hpp>
#include <grid/range.hpp>
#include <util/hilbert.hpp>
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


struct HilbertTest
{
    typedef unsigned long ulong;
    boost::random::mt19937 rGen;
    boost::random::uniform_int_distribution<ulong> idist;
    boost::random::uniform_int_distribution<ulong> idist_small;

    HilbertTest()
      : idist(std::numeric_limits<ulong>::min(), std::numeric_limits<ulong>::max()),
        idist_small(std::numeric_limits<ulong>::min()/10, std::numeric_limits<ulong>::max()/10)
    {}

    template<int dim, int nBits>
    void testCoordsAgainstKnownValue(ulong *check)
    {
      Array<ulong, 2> coords;
      ulong maxIndex = 1 << (dim*nBits);

      Array<ulong, dim> X;

      X = 0;

      for (ulong i=0; i<maxIndex; ++i)
      {
        schnek::HilbertCurve::indexToTranspose<dim>(X, i, nBits);
        schnek::HilbertCurve::transposeToAxes<dim>(X,nBits);
        for (int d=0; d<dim; ++d)
        {
          BOOST_CHECK_EQUAL(X[d], check[dim*i + d]);
        }
      }
    }

    template<int dim, int nBits>
    void testIndexToTransposeInvertible()
    {
      Array<ulong, 2> coords;
      ulong maxIndex = 1 << (dim*nBits);

      Array<ulong, dim> X;

      for (ulong index=0; index<maxIndex; ++index)
      {
        X = 0;
        schnek::HilbertCurve::indexToTranspose<dim>(X, index, nBits);
        ulong index_out = schnek::HilbertCurve::transposeToIndex<dim>(X,nBits);

        BOOST_CHECK_EQUAL(index, index_out);
      }
    }

    template<int dim, int nBits>
    void testIndexToAxesInvertible()
    {
      Array<ulong, 2> coords;
      ulong maxIndex = 1 << (dim*nBits);

      Array<ulong, dim> X;

      for (ulong index=0; index<maxIndex; ++index)
      {
        X = 0;
        schnek::HilbertCurve::indexToAxes<dim>(X, index, nBits);
        ulong index_out = schnek::HilbertCurve::axesToIndex<dim>(X,nBits);

        BOOST_CHECK_EQUAL(index, index_out);
      }
    }

    template<int dim, int nBits>
    void testFillingHypercube()
    {
      Array<ulong, 2> coords;
      ulong maxIndex = 1 << (dim*nBits);
      ulong gridSize = 1 << nBits;

      Array<ulong, dim> X;
      Array<int, dim> size(gridSize);
      Grid<bool, dim> G(size);
      G = false;

      for (ulong index=0; index<maxIndex; ++index)
      {
        schnek::HilbertCurve::indexToAxes<dim>(X, index, nBits);
        Array<int, dim, ArrayAssertArgCheck> pos;
        for (int i=0; i<dim; ++i) pos[i] = int(X[i]);

        G[pos] = true;
      }

      Range<int, dim> range(G.getLo(), G.getHi());
      for (typename Range<int, dim>::iterator it = range.begin(), end = range.end();
           it!=end;
           ++it)
      {
        BOOST_CHECK(G[*it]);
      }
    }


    template<int dim, int nBits>
    void testCoordsConnected()
    {
      Array<ulong, 2> coords;
      ulong maxIndex = 1 << (dim*nBits);

      Array<ulong, dim> Xprev, X;
      schnek::HilbertCurve::indexToAxes<dim>(Xprev, 0, nBits);

      for (ulong index=1; index<maxIndex; ++index)
      {
        schnek::HilbertCurve::indexToAxes<dim>(X, index, nBits);
        int steps=0;
        for (int i=0; i<dim; ++i) {
          // can't use abs() here because we are dealing with unsigned longs
          if (X[i]>Xprev[i])
            steps += X[i] - Xprev[i];
          else if (X[i]<Xprev[i])
            steps += Xprev[i] - X[i];
        }
        BOOST_CHECK_EQUAL(steps, 1);
        Xprev = X;
      }
    }
};


BOOST_AUTO_TEST_SUITE( hilbert )

// ==========================================================================
// Checking Coordinates against known values
// ==========================================================================

BOOST_FIXTURE_TEST_CASE( coords_against_known_value_2d1b, HilbertTest )
{
  ulong check[] = {0,0,0,1,1,1,1,0};
  testCoordsAgainstKnownValue<2,1>(check);
}

BOOST_FIXTURE_TEST_CASE( coords_against_known_value_2d2b, HilbertTest )
{
  ulong check[] = {0,0,1,0,1,1,0,1,0,2,0,3,1,3,1,2,2,2,2,3,3,3,3,2,3,1,2,1,2,0,3,0};
  testCoordsAgainstKnownValue<2,2>(check);
}

BOOST_FIXTURE_TEST_CASE( coords_against_known_value_2d3b, HilbertTest )
{
  ulong check[] = {0,0,0,1,1,1,1,0,2,0,3,0,3,1,2,1,2,2,3,2,3,3,2,3,1,3,1,2,0,2,0,3,
                   0,4,1,4,1,5,0,5,0,6,0,7,1,7,1,6,2,6,2,7,3,7,3,6,3,5,2,5,2,4,3,4,
                   4,4,5,4,5,5,4,5,4,6,4,7,5,7,5,6,6,6,6,7,7,7,7,6,7,5,6,5,6,4,7,4,
                   7,3,7,2,6,2,6,3,5,3,4,3,4,2,5,2,5,1,4,1,4,0,5,0,6,0,6,1,7,1,7,0};
  testCoordsAgainstKnownValue<2,3>(check);
}

// ==========================================================================
// Checking that indexToTranspose followed by transposeToIndex yields
// the original index
// ==========================================================================

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_1d1b, HilbertTest )
{
  testIndexToTransposeInvertible<1,1>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_1d2b, HilbertTest )
{
  testIndexToTransposeInvertible<1,2>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_1d3b, HilbertTest )
{
  testIndexToTransposeInvertible<1,3>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_1d4b, HilbertTest )
{
  testIndexToTransposeInvertible<1,4>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_1d5b, HilbertTest )
{
  testIndexToTransposeInvertible<1,5>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_1d6b, HilbertTest )
{
  testIndexToTransposeInvertible<1,6>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_1d7b, HilbertTest )
{
  testIndexToTransposeInvertible<1,7>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_1d8b, HilbertTest )
{
  testIndexToTransposeInvertible<1,8>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_1d9b, HilbertTest )
{
  testIndexToTransposeInvertible<1,9>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_1d10b, HilbertTest )
{
  testIndexToTransposeInvertible<1,10>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_1d11b, HilbertTest )
{
  testIndexToTransposeInvertible<1,11>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_1d12b, HilbertTest )
{
  testIndexToTransposeInvertible<1,12>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_1d13b, HilbertTest )
{
  testIndexToTransposeInvertible<1,13>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_1d14b, HilbertTest )
{
  testIndexToTransposeInvertible<1,14>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_1d15b, HilbertTest )
{
  testIndexToTransposeInvertible<1,15>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_1d16b, HilbertTest )
{
  testIndexToTransposeInvertible<1,16>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_2d1b, HilbertTest )
{
  testIndexToTransposeInvertible<2,1>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_2d2b, HilbertTest )
{
  testIndexToTransposeInvertible<2,2>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_2d3b, HilbertTest )
{
  testIndexToTransposeInvertible<2,3>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_2d4b, HilbertTest )
{
  testIndexToTransposeInvertible<2,4>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_2d5b, HilbertTest )
{
  testIndexToTransposeInvertible<2,5>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_2d6b, HilbertTest )
{
  testIndexToTransposeInvertible<2,6>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_2d7b, HilbertTest )
{
  testIndexToTransposeInvertible<2,7>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_2d8b, HilbertTest )
{
  testIndexToTransposeInvertible<2,8>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_3d1b, HilbertTest )
{
  testIndexToTransposeInvertible<3,1>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_3d2b, HilbertTest )
{
  testIndexToTransposeInvertible<3,2>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_3d3b, HilbertTest )
{
  testIndexToTransposeInvertible<3,3>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_3d4b, HilbertTest )
{
  testIndexToTransposeInvertible<3,4>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_3d5b, HilbertTest )
{
  testIndexToTransposeInvertible<3,5>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_4d1b, HilbertTest )
{
  testIndexToTransposeInvertible<4,1>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_4d2b, HilbertTest )
{
  testIndexToTransposeInvertible<4,2>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_4d3b, HilbertTest )
{
  testIndexToTransposeInvertible<4,3>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_4d4b, HilbertTest )
{
  testIndexToTransposeInvertible<4,4>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_5d1b, HilbertTest )
{
  testIndexToTransposeInvertible<5,1>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_5d2b, HilbertTest )
{
  testIndexToTransposeInvertible<5,2>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_5d3b, HilbertTest )
{
  testIndexToTransposeInvertible<5,3>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_6d1b, HilbertTest )
{
  testIndexToTransposeInvertible<6,1>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_6d2b, HilbertTest )
{
  testIndexToTransposeInvertible<6,2>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_7d1b, HilbertTest )
{
  testIndexToTransposeInvertible<7,1>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_7d2b, HilbertTest )
{
  testIndexToTransposeInvertible<7,2>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_8d1b, HilbertTest )
{
  testIndexToTransposeInvertible<8,1>();
}

BOOST_FIXTURE_TEST_CASE( index_to_transpose_invertible_8d2b, HilbertTest )
{
  testIndexToTransposeInvertible<8,2>();
}

// ==========================================================================
// Checking that indexToAxes followed by axesToIndex yields
// the original index
// ==========================================================================

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_1d1b, HilbertTest )
{
  testIndexToAxesInvertible<1,1>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_1d2b, HilbertTest )
{
  testIndexToAxesInvertible<1,2>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_1d3b, HilbertTest )
{
  testIndexToAxesInvertible<1,3>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_1d4b, HilbertTest )
{
  testIndexToAxesInvertible<1,4>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_1d5b, HilbertTest )
{
  testIndexToAxesInvertible<1,5>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_1d6b, HilbertTest )
{
  testIndexToAxesInvertible<1,6>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_1d7b, HilbertTest )
{
  testIndexToAxesInvertible<1,7>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_1d8b, HilbertTest )
{
  testIndexToAxesInvertible<1,8>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_1d9b, HilbertTest )
{
  testIndexToAxesInvertible<1,9>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_1d10b, HilbertTest )
{
  testIndexToAxesInvertible<1,10>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_1d11b, HilbertTest )
{
  testIndexToAxesInvertible<1,11>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_1d12b, HilbertTest )
{
  testIndexToAxesInvertible<1,12>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_1d13b, HilbertTest )
{
  testIndexToAxesInvertible<1,13>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_1d14b, HilbertTest )
{
  testIndexToAxesInvertible<1,14>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_1d15b, HilbertTest )
{
  testIndexToAxesInvertible<1,15>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_1d16b, HilbertTest )
{
  testIndexToAxesInvertible<1,16>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_2d1b, HilbertTest )
{
  testIndexToAxesInvertible<2,1>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_2d2b, HilbertTest )
{
  testIndexToAxesInvertible<2,2>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_2d3b, HilbertTest )
{
  testIndexToAxesInvertible<2,3>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_2d4b, HilbertTest )
{
  testIndexToAxesInvertible<2,4>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_2d5b, HilbertTest )
{
  testIndexToAxesInvertible<2,5>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_2d6b, HilbertTest )
{
  testIndexToAxesInvertible<2,6>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_2d7b, HilbertTest )
{
  testIndexToAxesInvertible<2,7>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_2d8b, HilbertTest )
{
  testIndexToAxesInvertible<2,8>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_3d1b, HilbertTest )
{
  testIndexToAxesInvertible<3,1>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_3d2b, HilbertTest )
{
  testIndexToAxesInvertible<3,2>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_3d3b, HilbertTest )
{
  testIndexToAxesInvertible<3,3>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_3d4b, HilbertTest )
{
  testIndexToAxesInvertible<3,4>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_3d5b, HilbertTest )
{
  testIndexToAxesInvertible<3,5>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_4d1b, HilbertTest )
{
  testIndexToAxesInvertible<4,1>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_4d2b, HilbertTest )
{
  testIndexToAxesInvertible<4,2>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_4d3b, HilbertTest )
{
  testIndexToAxesInvertible<4,3>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_4d4b, HilbertTest )
{
  testIndexToAxesInvertible<4,4>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_5d1b, HilbertTest )
{
  testIndexToAxesInvertible<5,1>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_5d2b, HilbertTest )
{
  testIndexToAxesInvertible<5,2>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_5d3b, HilbertTest )
{
  testIndexToAxesInvertible<5,3>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_6d1b, HilbertTest )
{
  testIndexToAxesInvertible<6,1>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_6d2b, HilbertTest )
{
  testIndexToAxesInvertible<6,2>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_7d1b, HilbertTest )
{
  testIndexToAxesInvertible<7,1>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_7d2b, HilbertTest )
{
  testIndexToAxesInvertible<7,2>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_8d1b, HilbertTest )
{
  testIndexToAxesInvertible<8,1>();
}

BOOST_FIXTURE_TEST_CASE( index_to_axes_invertible_8d2b, HilbertTest )
{
  testIndexToAxesInvertible<8,2>();
}


// ==========================================================================
// Checking that the coordinates from the complete index range pass through
// every point in the hypercube
// ==========================================================================

BOOST_FIXTURE_TEST_CASE( filling_hypercube_1d1b, HilbertTest )
{
  testFillingHypercube<1,1>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_1d2b, HilbertTest )
{
  testFillingHypercube<1,2>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_1d3b, HilbertTest )
{
  testFillingHypercube<1,3>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_1d4b, HilbertTest )
{
  testFillingHypercube<1,4>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_1d5b, HilbertTest )
{
  testFillingHypercube<1,5>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_1d6b, HilbertTest )
{
  testFillingHypercube<1,6>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_1d7b, HilbertTest )
{
  testFillingHypercube<1,7>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_1d8b, HilbertTest )
{
  testFillingHypercube<1,8>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_1d9b, HilbertTest )
{
  testFillingHypercube<1,9>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_1d10b, HilbertTest )
{
  testFillingHypercube<1,10>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_1d11b, HilbertTest )
{
  testFillingHypercube<1,11>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_1d12b, HilbertTest )
{
  testFillingHypercube<1,12>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_1d13b, HilbertTest )
{
  testFillingHypercube<1,13>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_1d14b, HilbertTest )
{
  testFillingHypercube<1,14>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_1d15b, HilbertTest )
{
  testFillingHypercube<1,15>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_1d16b, HilbertTest )
{
  testFillingHypercube<1,16>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_2d1b, HilbertTest )
{
  testFillingHypercube<2,1>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_2d2b, HilbertTest )
{
  testFillingHypercube<2,2>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_2d3b, HilbertTest )
{
  testFillingHypercube<2,3>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_2d4b, HilbertTest )
{
  testFillingHypercube<2,4>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_2d5b, HilbertTest )
{
  testFillingHypercube<2,5>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_2d6b, HilbertTest )
{
  testFillingHypercube<2,6>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_2d7b, HilbertTest )
{
  testFillingHypercube<2,7>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_2d8b, HilbertTest )
{
  testFillingHypercube<2,8>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_3d1b, HilbertTest )
{
  testFillingHypercube<3,1>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_3d2b, HilbertTest )
{
  testFillingHypercube<3,2>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_3d3b, HilbertTest )
{
  testFillingHypercube<3,3>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_3d4b, HilbertTest )
{
  testFillingHypercube<3,4>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_3d5b, HilbertTest )
{
  testFillingHypercube<3,5>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_4d1b, HilbertTest )
{
  testFillingHypercube<4,1>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_4d2b, HilbertTest )
{
  testFillingHypercube<4,2>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_4d3b, HilbertTest )
{
  testFillingHypercube<4,3>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_4d4b, HilbertTest )
{
  testFillingHypercube<4,4>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_5d1b, HilbertTest )
{
  testFillingHypercube<5,1>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_5d2b, HilbertTest )
{
  testFillingHypercube<5,2>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_5d3b, HilbertTest )
{
  testFillingHypercube<5,3>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_6d1b, HilbertTest )
{
  testFillingHypercube<6,1>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_6d2b, HilbertTest )
{
  testFillingHypercube<6,2>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_7d1b, HilbertTest )
{
  testFillingHypercube<7,1>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_7d2b, HilbertTest )
{
  testFillingHypercube<7,2>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_8d1b, HilbertTest )
{
  testFillingHypercube<8,1>();
}

BOOST_FIXTURE_TEST_CASE( filling_hypercube_8d2b, HilbertTest )
{
  testFillingHypercube<8,2>();
}

// ==========================================================================
// Checking that the coordinates from the complete index range pass through
// every point in the hypercube
// ==========================================================================

BOOST_FIXTURE_TEST_CASE( coords_connected_1d1b, HilbertTest )
{
  testCoordsConnected<1,1>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_1d2b, HilbertTest )
{
  testCoordsConnected<1,2>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_1d3b, HilbertTest )
{
  testCoordsConnected<1,3>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_1d4b, HilbertTest )
{
  testCoordsConnected<1,4>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_1d5b, HilbertTest )
{
  testCoordsConnected<1,5>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_1d6b, HilbertTest )
{
  testCoordsConnected<1,6>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_1d7b, HilbertTest )
{
  testCoordsConnected<1,7>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_1d8b, HilbertTest )
{
  testCoordsConnected<1,8>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_1d9b, HilbertTest )
{
  testCoordsConnected<1,9>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_1d10b, HilbertTest )
{
  testCoordsConnected<1,10>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_1d11b, HilbertTest )
{
  testCoordsConnected<1,11>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_1d12b, HilbertTest )
{
  testCoordsConnected<1,12>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_1d13b, HilbertTest )
{
  testCoordsConnected<1,13>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_1d14b, HilbertTest )
{
  testCoordsConnected<1,14>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_1d15b, HilbertTest )
{
  testCoordsConnected<1,15>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_1d16b, HilbertTest )
{
  testCoordsConnected<1,16>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_2d1b, HilbertTest )
{
  testCoordsConnected<2,1>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_2d2b, HilbertTest )
{
  testCoordsConnected<2,2>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_2d3b, HilbertTest )
{
  testCoordsConnected<2,3>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_2d4b, HilbertTest )
{
  testCoordsConnected<2,4>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_2d5b, HilbertTest )
{
  testCoordsConnected<2,5>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_2d6b, HilbertTest )
{
  testCoordsConnected<2,6>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_2d7b, HilbertTest )
{
  testCoordsConnected<2,7>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_2d8b, HilbertTest )
{
  testCoordsConnected<2,8>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_3d1b, HilbertTest )
{
  testCoordsConnected<3,1>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_3d2b, HilbertTest )
{
  testCoordsConnected<3,2>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_3d3b, HilbertTest )
{
  testCoordsConnected<3,3>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_3d4b, HilbertTest )
{
  testCoordsConnected<3,4>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_3d5b, HilbertTest )
{
  testCoordsConnected<3,5>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_4d1b, HilbertTest )
{
  testCoordsConnected<4,1>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_4d2b, HilbertTest )
{
  testCoordsConnected<4,2>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_4d3b, HilbertTest )
{
  testCoordsConnected<4,3>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_4d4b, HilbertTest )
{
  testCoordsConnected<4,4>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_5d1b, HilbertTest )
{
  testCoordsConnected<5,1>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_5d2b, HilbertTest )
{
  testCoordsConnected<5,2>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_5d3b, HilbertTest )
{
  testCoordsConnected<5,3>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_6d1b, HilbertTest )
{
  testCoordsConnected<6,1>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_6d2b, HilbertTest )
{
  testCoordsConnected<6,2>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_7d1b, HilbertTest )
{
  testCoordsConnected<7,1>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_7d2b, HilbertTest )
{
  testCoordsConnected<7,2>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_8d1b, HilbertTest )
{
  testCoordsConnected<8,1>();
}

BOOST_FIXTURE_TEST_CASE( coords_connected_8d2b, HilbertTest )
{
  testCoordsConnected<8,2>();
}


BOOST_AUTO_TEST_SUITE_END()

