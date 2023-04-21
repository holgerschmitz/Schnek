/*
 * utility.hpp
 *
 *  Created on: 19 Jul 2017
 *      Author: Holger Schmitz
 */

#ifndef TESTSUITE_UTILITY_HPP_
#define TESTSUITE_UTILITY_HPP_

#include <grid/array.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <boost/test/unit_test.hpp>

#pragma GCC diagnostic pop


// safe comparison of two floating point numbers
bool is_equal(double a, double b);



/** 
 *  Class to plug into the Array as CheckingPolicy.
 *  Performs Boost Test argument checking for unit tests.
 */
template<size_t limit>
class ArrayBoostTestArgCheck
{
  private:
    void checkImpl(size_t i) const
    {
      BOOST_CHECK_GE(i,size_t(0));
      BOOST_CHECK_LT(i,limit);
    }

  public:

    #ifdef SCHNEK_WITH_CUDA__
    #pragma hd_warning_disable
    #endif
    SCHNEK_INLINE void check(size_t i) const
    {
      checkImpl(i);
    }
};

template<size_t rank>
class GridBoostTestCheck {
  public:
    typedef schnek::Array<int,rank,ArrayBoostTestArgCheck> IndexType;
  private:
    static const IndexType &checkImpl(
        const IndexType &pos,
        const IndexType &low,
        const IndexType &high
    )
    {
      for (size_t i=0; i<rank; ++i)
      {
        BOOST_CHECK_GE(pos[i],low[i]);
        BOOST_CHECK_LE(pos[i],high[i]);
      }
      return pos;
    }
  public:
    #ifdef SCHNEK_WITH_CUDA__
    #pragma hd_warning_disable
    #endif
    SCHNEK_INLINE static const IndexType &check(
        const IndexType &pos,
        const IndexType &low,
        const IndexType &high
    )
    {
      return checkImpl(pos, low, high);
    }
};



#endif /* TESTSUITE_UTILITY_HPP_ */
