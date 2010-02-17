
#include "fixedarray.h"
#include <cassert>

namespace schnek {


template<int rank>
class MatrixNoArgCheck {
  public:
    typedef FixedArray<int,rank> IndexType;
    static const  IndexType &check(
        const IndexType &pos, 
        const IndexType &low,  
        const IndexType &high
    );
};


template<int rank>
class MatrixAssertCheck {
  public:
    typedef FixedArray<int,rank> IndexType;
    static const  IndexType &check(
        const IndexType &pos, 
        const IndexType &low,  
        const IndexType &high
    );
};


template<int rank>
inline const typename MatrixNoArgCheck<rank>::IndexType &MatrixNoArgCheck<rank>::check(
        const IndexType &pos, 
        const IndexType &, const IndexType &
    )
{ return pos; }

template<int rank>
inline const typename MatrixAssertCheck<rank>::IndexType &MatrixAssertCheck<rank>::check(
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
