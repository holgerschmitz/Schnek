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
};


BOOST_AUTO_TEST_SUITE( hilbert )

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

BOOST_AUTO_TEST_SUITE_END()

