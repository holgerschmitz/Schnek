/*
 * range_test_fixture.hpp
 *
 *  Created on: 16 Dec 2022
 *      Author: Holger Schmitz
 */

#include <grid/array.hpp>
#include <vector>
#include <cmath>
#include <limits>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy-with-user-provided-copy"

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>

#pragma GCC diagnostic pop

struct RangeIterationTest
{
    boost::random::mt19937 rGen;
    boost::random::uniform_real_distribution<> dist;
    boost::random::uniform_int_distribution<> idist;
    boost::random::uniform_int_distribution<> idist_small;

    RangeIterationTest()
      : dist(-1.0,1.0),
        idist(std::numeric_limits<int>::min(), std::numeric_limits<int>::max()),
        idist_small(std::numeric_limits<int>::min()/10, std::numeric_limits<int>::max()/10)
    {}

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
};
