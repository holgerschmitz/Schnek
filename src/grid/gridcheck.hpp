/*
 * gridcheck.hpp
 *
 * Created on: 23 Jan 2007
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
 *
 */

#ifndef SCHNEK_GRIDCHECK_H_
#define SCHNEK_GRIDCHECK_H_

#include "array.hpp"
#include <cassert>

namespace schnek {

template<int rank>
class GridNoArgCheck {
  public:
    typedef Array<int,rank> IndexType;
    static const  IndexType &check(
        const IndexType &pos, 
        const IndexType &low,  
        const IndexType &high
    );
};


template<int rank>
class GridAssertCheck {
  public:
    typedef Array<int,rank> IndexType;
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

#endif // SCHNEK_GRIDCHECK_H_
