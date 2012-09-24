/*
 * grid.t
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

// --------------------------------------------------------------
// implementation

#include "domain.hpp"

namespace schnek
{

//================== GridBase =======================

template<
  typename T,
  int rank,
  class CheckingPolicy,
  class StoragePolicy
>
GridBase<T, rank, CheckingPolicy, StoragePolicy>::GridBase() : StoragePolicy()
{}

template<
  typename T,
  int rank,
  class CheckingPolicy,
  class StoragePolicy
>
GridBase<T, rank, CheckingPolicy, StoragePolicy>::GridBase(const IndexType &size)
  : StoragePolicy(IndexType::Zero(), size-IndexType::Unity())
{}

template<
  typename T,
  int rank,
  class CheckingPolicy,
  class StoragePolicy
>
GridBase<T, rank, CheckingPolicy, StoragePolicy>::GridBase(const IndexType &low, const IndexType &high)
  : StoragePolicy(low,high)
{}

// -------------------------------------------------------------
// inline functions

template<
  typename T,
  int rank,
  class CheckingPolicy,
  class StoragePolicy
>
inline T& GridBase<T, rank, CheckingPolicy, StoragePolicy>
  ::operator[](const IndexType& pos)
{
  return get(this->check(pos,this->getLow(),this->getHigh()));
}

template<
  typename T,
  int rank,
  class CheckingPolicy,
  class StoragePolicy
>
inline T GridBase<T, rank, CheckingPolicy, StoragePolicy>
  ::operator[](const IndexType& pos) const
{
  return get(this->check(pos,this->getLow(),this->getHigh()));
}

template<
  typename T,
  int rank,
  class CheckingPolicy,
  class StoragePolicy
>
inline  T& GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i)
{
  return get(this->check(IndexType(i),this->getLow(),this->getHigh()));
}

template<
  typename T,
  int rank,
  class CheckingPolicy,
  class StoragePolicy
>
inline  T GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i) const
{
  return get(this->check(IndexType(i),this->getLow(),this->getHigh()));
}

template<
  typename T,
  int rank,
  class CheckingPolicy,
  class StoragePolicy
>
inline  T& GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j)
{
  return get(this->check(IndexType(i,j),this->getLow(),this->getHigh()));
}

template<
  typename T,
  int rank,
  class CheckingPolicy,
  class StoragePolicy
>
inline  T GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j) const
{
  return get(this->check(IndexType(i,j),this->getLow(),this->getHigh()));
}

template<
  typename T,
  int rank,
  class CheckingPolicy,
  class StoragePolicy
>
inline  T& GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k)
{
  return get(this->check(IndexType(i,j,k),this->getLow(),this->getHigh()));
}

template<
  typename T,
  int rank,
  class CheckingPolicy,
  class StoragePolicy
>
inline  T GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k) const
{
  return get(this->check(IndexType(i,j,k),this->getLow(),this->getHigh()));
}

template<
  typename T,
  int rank,
  class CheckingPolicy,
  class StoragePolicy
>
inline  T& GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k, int l)
{
  return get(this->check(IndexType(i,j,k,l),this->getLow(),this->getHigh()));
}

template<
  typename T,
  int rank,
  class CheckingPolicy,
  class StoragePolicy
>
inline  T GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k, int l) const
{
  return get(this->check(IndexType(i,j,k,l),this->getLow(),this->getHigh()));
}

template<
  typename T,
  int rank,
  class CheckingPolicy,
  class StoragePolicy
>
inline  T& GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k, int l, int m)
{
  return get(this->check(IndexType(i,j,k,l,m),this->getLow(),this->getHigh()));
}

template<
  typename T,
  int rank,
  class CheckingPolicy,
  class StoragePolicy
>
inline  T GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k, int l, int m) const
{
  return get(this->check(IndexType(i,j,k,l,m),this->getLow(),this->getHigh()));
}

template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
template<
  typename T2,
  class CheckingPolicy2,
  class StoragePolicy2
>
GridBase<T, rank, CheckingPolicy, StoragePolicy>&
  GridBase<T, rank, CheckingPolicy, StoragePolicy>
    ::operator=(const GridBase<T2, rank, CheckingPolicy2, StoragePolicy2> &grid)
{
  this->resize(grid);
  this->copyFromGrid(grid);
  return *this;
}

template<
  typename T, 
  int rank,
  class CheckingPolicy,
  class StoragePolicy
>
GridBase<T, rank, CheckingPolicy, StoragePolicy>&
  GridBase<T, rank, CheckingPolicy, StoragePolicy>
    ::operator=(const T &val)
{
  typedef typename StoragePolicy<T,rank>::storage_iterator Iterator;
  Iterator end = this->end();
  Iterator dest = this->begin();
  while (dest != end)
  {
    *dest = val;
    ++dest;
  }
  
  return *this;
}


template<
  typename T,
  int rank,
  class CheckingPolicy,
  class StoragePolicy
>
template<
  typename T2,
  class CheckingPolicy2
>
GridBase<T, rank, CheckingPolicy, StoragePolicy>&
  GridBase<T, rank, CheckingPolicy, StoragePolicy>
    ::operator-=(GridBase<T2, rank, CheckingPolicy2, StoragePolicy>& grid)
{
  typedef typename StoragePolicy<T,rank>::storage_iterator Iterator;
  Iterator dest = this->begin();
  Iterator2 src = grid.begin();
  Iterator2 end = grid.end();
  while (src != end)
  {
    *dest -= *src;
    ++dest;
    ++src;
  }

  return *this;
}

template<
  typename T,
  int rank,
  class CheckingPolicy,
  class StoragePolicy
>
template<
  typename T2,
  class CheckingPolicy2,
  class StoragePolicy2
>
GridBase<T, rank, CheckingPolicy, StoragePolicy>&
  GridBase<T, rank, CheckingPolicy, StoragePolicy>
    ::operator-=(GridBase<T2, rank, CheckingPolicy2, StoragePolicy2>& grid)
{
  RecDomain<rank> rec(this->getMin(), this->getMax());
  RecDomain<rank>::iteratur it = rec.begin();
  RecDomain<rank>::iteratur end = rec.end();

  while (it != end)
  {
    this->get(*it) -= grid.get(*it);
    ++it;
  }

  return *this;
}


template<
  typename T,
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
template<
  typename T2,
  class CheckingPolicy2
>
Grid<T, rank, CheckingPolicy, StoragePolicy>&
  Grid<T, rank, CheckingPolicy, StoragePolicy>
    ::operator+=(Grid<T2, rank, CheckingPolicy2, StoragePolicy>& grid)
{
  typedef typename StoragePolicy<T,rank>::storage_iterator Iterator;
  Iterator dest = this->begin();
  Iterator2 src = grid.begin();
  Iterator2 end = grid.end();
  while (src != end)
  {
    *dest += *src;
    ++dest;
    ++src;
  }

  return *this;
}

template<
  typename T,
  int rank,
  class CheckingPolicy,
  class StoragePolicy
>
template<
  typename T2,
  class CheckingPolicy2,
  class StoragePolicy2
>
GridBase<T, rank, CheckingPolicy, StoragePolicy>&
  GridBase<T, rank, CheckingPolicy, StoragePolicy>
    ::operator+=(GridBase<T2, rank, CheckingPolicy2, StoragePolicy2>& grid)
{
  RecDomain<rank> rec(this->getMin(), this->getMax());
  RecDomain<rank>::iteratur it = rec.begin();
  RecDomain<rank>::iteratur end = rec.end();

  while (it != end)
  {
    this->get(*it) += grid.get(*it);
    ++it;
  }

  return *this;
}


template<
  typename T,
  int rank,
  class CheckingPolicy,
  class StoragePolicy
>
void GridBase<T, rank, CheckingPolicy, StoragePolicy>::resize(const IndexType &d)
{
  IndexType high;
  high = d-IndexType::Unity();
  StoragePolicy::resize(IndexType::Zero(), high);
}

template<
  typename T,
  int rank,
  class CheckingPolicy,
  class StoragePolicy
>
void GridBase<T, rank, CheckingPolicy, StoragePolicy>::resize(const IndexType &low, const IndexType &high)
{
  StoragePolicy::resize(low, high);
}

template<
  typename T,
  int rank,
  class CheckingPolicy,
  class StoragePolicy
>
template<
  typename T2,
  class CheckingPolicy2,
  class StoragePolicy2
>
void GridBase<T, rank, CheckingPolicy, StoragePolicy>::resize(const GridBase<T2, rank, CheckingPolicy2, StoragePolicy2>& grid)
{
  StoragePolicy::resize(grid.getLow(), grid.getHigh());
}


template<
  typename T,
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
template<
  typename T2,
  class CheckingPolicy2
>
void GridBase<T, rank, CheckingPolicy, StoragePolicy>
  ::copyFromGrid(const GridBase<T2, rank, CheckingPolicy2, StoragePolicy>& grid)
{
  typedef typename StoragePolicy<T,rank>::const_storage_iterator CIterator;
  typedef typename StoragePolicy<T,rank>::storage_iterator Iterator;
  CIterator src = grid.cbegin();
  CIterator srcEnd = grid.cend();
  Iterator dest = this->begin();
  while (src != srcEnd)
  {
    *dest = *src;
    ++src;
    ++dest;
  }
}

//================== Grid =======================

template<
  typename T,
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
Grid<T, rank, CheckingPolicy, StoragePolicy>::Grid()
  : GridBase<T, rank, CheckingPolicy<rank>,  StoragePolicy<T,Rank> >()
{}

template<
  typename T,
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
Grid<T, rank, CheckingPolicy, StoragePolicy>::Grid(const IndexType &size)
  : GridBase<T, rank, CheckingPolicy<rank>,  StoragePolicy<T,Rank> >(size)
{}

template<
  typename T,
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
Grid<T, rank, CheckingPolicy, StoragePolicy>::Grid(const IndexType &low, const IndexType &high)
  : GridBase<T, rank, CheckingPolicy<rank>,  StoragePolicy<T,Rank> >(low, high)
{}

template<
  typename T,
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
Grid<T, rank, CheckingPolicy, StoragePolicy>
  ::Grid(const Grid<T, rank, CheckingPolicy, StoragePolicy>& matr)
  : GridBase<T, rank, CheckingPolicy<rank>,  StoragePolicy<T,Rank> >(matr.getLow(), matr.getHigh())
{
  this->copyFromGrid(matr);
}



template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
template<typename Arg0>
IndexedGrid<
  Grid<T, rank, CheckingPolicy, StoragePolicy>,
  TYPELIST_1(Arg0)
>
Grid<T, rank, CheckingPolicy, StoragePolicy>::operator()(
  const Arg0 &i0
)
{
  return IndexedGrid<GridType,TYPELIST_1(Arg0)> (i0);
}

template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
template<typename Arg0, typename Arg1>
IndexedGrid<
  Grid<T, rank, CheckingPolicy, StoragePolicy>,
  TYPELIST_2(Arg0, Arg1)
>
Grid<T, rank, CheckingPolicy, StoragePolicy>::operator()(
  const Arg0 &i0, const Arg1 &i1
)
{
  return IndexedGrid<GridType,TYPELIST_2(Arg0, Arg1)> (i0, i1);
}


} // namespace
