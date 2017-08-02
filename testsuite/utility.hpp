/*
 * utility.hpp
 *
 *  Created on: 19 Jul 2017
 *      Author: Holger Schmitz
 */

#ifndef TESTSUITE_UTILITY_HPP_
#define TESTSUITE_UTILITY_HPP_

#include <schnek/grid/array.hpp>
#include <boost/test/unit_test.hpp>


// safe comparison of two floating point numbers
bool is_equal(double a, double b);

/** Class to plug into the Array as CheckingPolicy.
 *  Performs Boost Test argument checking for unit tests.
 */
template<int limit>
class ArrayBoostTestArgCheck
{
  public:
    /** The check method does not do anything */
    void check(int i) const
    {
      BOOST_CHECK_GE(i,0);
      BOOST_CHECK_LT(i,limit);
    }
};

template<int rank>
class GridBoostTestCheck {
  public:
    typedef schnek::Array<int,rank,ArrayBoostTestArgCheck> IndexType;
    static const  IndexType &check(
        const IndexType &pos,
        const IndexType &low,
        const IndexType &high
    )
    {
      for (int i=0; i<rank; ++i)
      {
        BOOST_CHECK_GE(pos[i],low[i]);
        BOOST_CHECK_LE(pos[i],high[i]);
      }
      return pos;
    }
};



#endif /* TESTSUITE_UTILITY_HPP_ */
