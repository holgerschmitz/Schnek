#ifndef SCHNEK_GRIDCHECK_H
#define SCHNEK_GRIDCHECK_H

#include "fixedarray.h"
#include <cassert>

namespace schnek {

template<int rank>
class GridNoArgCheck {
  public:
    typedef FixedArray<int,rank> IndexType;
    static const  IndexType &check(
        const IndexType &pos, 
        const IndexType &low,  
        const IndexType &high
    );
};


template<int rank>
class GridAssertCheck {
  public:
    typedef FixedArray<int,rank> IndexType;
    static const  IndexType &check(
        const IndexType &pos, 
        const IndexType &low,  
        const IndexType &high
    );
};


template<int rank>
inline const typename GridNoArgCheck<rank>::IndexType &GridNoArgCheck<rank>::check(
        const IndexType &pos, 
        const IndexType &, const IndexType &
    )
{ return pos; }

template<int rank>
inline const typename GridAssertCheck<rank>::IndexType &GridAssertCheck<rank>::check(
        const IndexType &pos, 
        const IndexType &low, 
        const IndexType &high
    )
{
  for (int i=0; i<rank; ++i)
  {
    assert(pos[i]>=low[i]);
    assert(pos[i]<=high[i]);
  }
  return pos; 
}

}

#endif
