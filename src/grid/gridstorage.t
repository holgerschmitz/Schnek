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
#include <cmath>

namespace schnek {

//=================================================================
//=============== SingleArrayInstantAllocation ====================
//=================================================================

template<typename T, int rank>
void SingleArrayInstantAllocation<T, rank>::resize(const IndexType &low_, const IndexType &high_)
{
//  if ( (low != low_) || (high != high_) )
//  {
  this->deleteData();
  this->newData(low_,high_);
//  }
}

template<typename T, int rank>
SingleArrayInstantAllocation<T, rank>::~SingleArrayInstantAllocation()
{
  this->deleteData();
}

template<typename T, int rank>
void SingleArrayInstantAllocation<T, rank>::deleteData()
{
  if (data)
    delete[] data;
  data = NULL;
  size = 0;
}

template<typename T, int rank>
void SingleArrayInstantAllocation<T, rank>::newData(
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

//=================================================================
//=============== SingleArrayLazyAllocation ====================
//=================================================================

template<typename T, int rank>
SingleArrayLazyAllocation<T, rank>::SingleArrayLazyAllocation()
  : bufSize(0), avgSize(0.0), avgVar(0.0), r(0.05)
{}

template<typename T, int rank>
SingleArrayLazyAllocation<T, rank>::~SingleArrayLazyAllocation()
{
  this->deleteData();
}

template<typename T, int rank>
void SingleArrayLazyAllocation<T, rank>::resize(const IndexType &low_, const IndexType &high_)
{
  int oldSize = size;
  size = 1;
  int d;

  low = low_;
  high = high_;

  for (d = 0; d < rank; d++) {
    dims[d] = high[d] - low[d] + 1;
    size *= dims[d];
  }

  avgSize = r*size + (1-r)*avgSize;
  int diff = size - avgSize;
  avgVar = r*diff*diff + (1-r)*avgVar;

  if ((size > oldSize) || ((size + 8*sqrt(avgVar)) < bufSize))
  {
    this->deleteData();
    this->newData(size);
  }

  int p = -low[rank-1];

  for (d = rank-2; d >= 0 ; d--) {
    p = p*dims[d] -low[d];
  }
  data_fast = data + p;
}

template<typename T, int rank>
void SingleArrayLazyAllocation<T, rank>::deleteData()
{
  if (data)
    delete[] data;
  data = NULL;
  size = 0;
}

template<typename T, int rank>
void SingleArrayLazyAllocation<T, rank>::newData(
  int size
)
{
  bufSize = size + (int)(4*sqrt(avgVar));
  data = new T[bufSize];
}

//=================================================================
//================== SingleArrayGridStorageBase ===================
//=================================================================

template<typename T, int rank, template<typename, int> class AllocationPolicy>
SingleArrayGridStorageBase<T, rank, AllocationPolicy>::SingleArrayGridStorageBase()
{
  data = NULL;
  data_fast = NULL;
  size = 0;
}


template<typename T, int rank, template<typename, int> class AllocationPolicy>
SingleArrayGridStorageBase<T, rank, AllocationPolicy>::SingleArrayGridStorageBase(
  const IndexType &low_, 
  const IndexType &high_
)
{
  data = NULL;
  size = 0;
  this->resize(low_, high_);
}

template<typename T, int rank, template<typename, int> class AllocationPolicy>
inline T& SingleArrayGridStorageBase<T, rank, AllocationPolicy>::get(const IndexType &index)
{
  int pos = index[rank-1];
  for (int i=rank-2; i>=0; --i)
  {
    pos = index[i] + dims[i]*pos; 
  }
  return data_fast[pos];
}

template<typename T, int rank, template<typename, int> class AllocationPolicy>
inline const T& SingleArrayGridStorageBase<T, rank, AllocationPolicy>::get(const IndexType &index) const
{
  int pos = index[rank-1];
  for (int i=rank-2; i>=0; --i)
  {
    pos = index[i] + dims[i]*pos; 
  }
  return data_fast[pos];
}

}
