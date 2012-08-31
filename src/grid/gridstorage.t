/*
 * gridstorage.t
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

#include <cstddef>

namespace schnek {

template<typename T, int rank>
SingleArrayGridStorage<T, rank>::SingleArrayGridStorage()
{
  data = NULL;
  data_fast = NULL;
  size = 0;
}


template<typename T, int rank>
SingleArrayGridStorage<T, rank>::SingleArrayGridStorage(
  const IndexType &low_, 
  const IndexType &high_
)
{
  data = NULL;
  size = 0;
  newData(low_, high_);
}


template<class T, int rank> 
SingleArrayGridStorage<T, rank>::~SingleArrayGridStorage()
{
  deleteData();
}


template<typename T, int rank>
inline T& SingleArrayGridStorage<T, rank>::get(const IndexType &index)
{
  int pos = index[rank-1];
  for (int i=rank-2; i>=0; --i)
  {
    pos = index[i] + dims[i]*pos; 
  }
  return data_fast[pos];
}

template<typename T, int rank>
inline const T& SingleArrayGridStorage<T, rank>::get(const IndexType &index) const
{
  int pos = index[rank-1];
  for (int i=rank-2; i>=0; --i)
  {
    pos = index[i] + dims[i]*pos; 
  }
  return data_fast[pos];
}

template<typename T, int rank>
void SingleArrayGridStorage<T, rank>::resize(const IndexType &low_, const IndexType &high_)
{
//  if ( (low != low_) || (high != high_) )
//  {
    deleteData();
    newData(low_,high_);
//  }
}

template<typename T, int rank>
void SingleArrayGridStorage<T, rank>::deleteData()
{
  if (data)
    delete[] data;
  data = NULL;
  size = 0;
}

template<typename T, int rank>
void SingleArrayGridStorage<T, rank>::newData(
  const IndexType &low_, 
  const IndexType &high_
)
{
  size = 1;
  int d;
  
  low = low_;
  high = high_;
  
  for (d = 0; d < rank; d++) {
    dims[d] = high[d] - low[d] + 1;
    size *= dims[d];
  }
  data = new T[size];
  int p = -low[rank-1];

  for (d = rank-2; d >= 0 ; d--) {
    p = p*dims[d] -low[d];
  }
  data_fast = data + p;
}

}
