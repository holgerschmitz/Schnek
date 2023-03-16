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

#include "range.hpp"
#include "arrayexpression.hpp"

namespace schnek
{

namespace internal {
  //=================================================================
  //============================ GridBase ===========================
  //=================================================================

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  GridBase<T, rank, CheckingPolicy, StoragePolicy>::GridBase() : StoragePolicy()
  {}

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  template<template<size_t> class ArrayCheckingPolicy>
  GridBase<T, rank, CheckingPolicy, StoragePolicy>::GridBase(const Array<int,rank,ArrayCheckingPolicy> &size)
    : StoragePolicy(IndexType::Zero(), size-IndexType::Ones())
  {}

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  template<template<size_t> class ArrayCheckingPolicy>
  GridBase<T, rank, CheckingPolicy, StoragePolicy>::GridBase(const Array<int,rank,ArrayCheckingPolicy> &low, const Array<int,rank,ArrayCheckingPolicy> &high)
    : StoragePolicy(low,high)
  {}

  // -------------------------------------------------------------
  // inline functions

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  template<template<size_t> class ArrayCheckingPolicy>
  inline T& GridBase<T, rank, CheckingPolicy, StoragePolicy>
    ::operator[](const Array<int,rank,ArrayCheckingPolicy>& pos)
  {
    return this->get(this->check(pos,this->getLo(),this->getHi()));
  }

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  template<template<size_t> class ArrayCheckingPolicy>
  inline T GridBase<T, rank, CheckingPolicy, StoragePolicy>
    ::operator[](const Array<int,rank,ArrayCheckingPolicy>& pos) const
  {
    return this->get(this->check(pos,this->getLo(),this->getHi()));
  }

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  template<class Operator, int Length>
  inline T& GridBase<T, rank, CheckingPolicy, StoragePolicy>
    ::operator[](const ArrayExpression<Operator, Length>& pos)
  {
    return this->operator[](IndexType(pos));
  }

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  template<class Operator, int Length>
  inline T GridBase<T, rank, CheckingPolicy, StoragePolicy>
    ::operator[](const ArrayExpression<Operator, Length>& pos) const
  {
    return this->operator[](IndexType(pos));
  }

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  inline  T& GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator [](int i)
  {
    return this->get(this->check(IndexType(i),this->getLo(),this->getHi()));
  }

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  inline  T GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator [](int i) const
  {
    return this->get(this->check(IndexType(i),this->getLo(),this->getHi()));
  }

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  inline  T& GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i)
  {
    return this->get(this->check(IndexType(i),this->getLo(),this->getHi()));
  }

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  inline  T GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i) const
  {
    return this->get(this->check(IndexType(i),this->getLo(),this->getHi()));
  }


  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  inline  T& GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j)
  {
    return this->get(this->check(IndexType(i,j),this->getLo(),this->getHi()));
  }

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  inline  T GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j) const
  {
    return this->get(this->check(IndexType(i,j),this->getLo(),this->getHi()));
  }

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  inline  T& GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k)
  {
    return this->get(this->check(IndexType(i,j,k),this->getLo(),this->getHi()));
  }

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  inline  T GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k) const
  {
    return this->get(this->check(IndexType(i,j,k),this->getLo(),this->getHi()));
  }

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  inline  T& GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k, int l)
  {
    return this->get(this->check(IndexType(i,j,k,l),this->getLo(),this->getHi()));
  }

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  inline  T GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k, int l) const
  {
    return this->get(this->check(IndexType(i,j,k,l),this->getLo(),this->getHi()));
  }

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  inline  T& GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k, int l, int m)
  {
    return this->get(this->check(IndexType(i,j,k,l,m),this->getLo(),this->getHi()));
  }

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  inline  T GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k, int l, int m) const
  {
    return this->get(this->check(IndexType(i,j,k,l,m),this->getLo(),this->getHi()));
  }

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  inline  T& GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k, int l, int m, int o)
  {
    return this->get(this->check(IndexType(i,j,k,l,m,o),this->getLo(),this->getHi()));
  }

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  inline  T GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k, int l, int m, int o) const
  {
    return this->get(this->check(IndexType(i,j,k,l,m,o),this->getLo(),this->getHi()));
  }

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  inline  T& GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k, int l, int m, int o, int p)
  {
    return this->get(this->check(IndexType(i,j,k,l,m,o,p),this->getLo(),this->getHi()));
  }

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  inline  T GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k, int l, int m, int o, int p) const
  {
    return this->get(this->check(IndexType(i,j,k,l,m,o,p),this->getLo(),this->getHi()));
  }

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  inline  T& GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k, int l, int m, int o, int p, int q)
  {
    return this->get(this->check(IndexType(i,j,k,l,m,o,p,q),this->getLo(),this->getHi()));
  }

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  inline  T GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k, int l, int m, int o, int p, int q) const
  {
    return this->get(this->check(IndexType(i,j,k,l,m,o,p,q),this->getLo(),this->getHi()));
  }

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  inline  T& GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k, int l, int m, int o, int p, int q, int r)
  {
    return this->get(this->check(IndexType(i,j,k,l,m,o,p,q,r),this->getLo(),this->getHi()));
  }

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  inline  T GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k, int l, int m, int o, int p, int q, int r) const
  {
    return this->get(this->check(IndexType(i,j,k,l,m,o,p,q,r),this->getLo(),this->getHi()));
  }

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  inline  T& GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k, int l, int m, int o, int p, int q, int r, int s)
  {
    return this->get(this->check(IndexType(i,j,k,l,m,o,p,q,r,s),this->getLo(),this->getHi()));
  }

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  inline  T GridBase<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k, int l, int m, int o, int p, int q, int r, int s) const
  {
    return this->get(this->check(IndexType(i,j,k,l,m,o,p,q,r,s),this->getLo(),this->getHi()));
  }

  template<
    typename T, 
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  GridBase<T, rank, CheckingPolicy, StoragePolicy>&
    GridBase<T, rank, CheckingPolicy, StoragePolicy>
      ::operator=(const T &val)
  {
    typedef typename StoragePolicy::storage_iterator Iterator;
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
    size_t rank,
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
    typedef typename StoragePolicy::storage_iterator Iterator;
    Iterator dest = this->begin();
    Iterator src = grid.begin();
    Iterator end = grid.end();
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
    size_t rank,
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
    Range<int, rank> rec(this->getLo(), this->getHi());
    typename Range<int, rank>::iterator it = rec.begin();
    typename Range<int, rank>::iterator end = rec.end();

    while (it != end)
    {
      this->get(*it) -= grid.get(*it);
      ++it;
    }

    return *this;
  }


  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  template<
    typename T2,
    class CheckingPolicy2
  >
  GridBase<T, rank, CheckingPolicy, StoragePolicy>&
  GridBase<T, rank, CheckingPolicy, StoragePolicy>
      ::operator+=(GridBase<T2, rank, CheckingPolicy2, StoragePolicy>& grid)
  {
    typedef typename StoragePolicy::storage_iterator Iterator;
    Iterator dest = this->begin();
    Iterator src = grid.begin();
    Iterator end = grid.end();
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
    size_t rank,
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
    Range<int, rank> rec(this->getLo(), this->getHi());
    typename Range<int, rank>::iterator it = rec.begin();
    typename Range<int, rank>::iterator end = rec.end();

    while (it != end)
    {
      this->get(*it) += grid.get(*it);
      ++it;
    }

    return *this;
  }


  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  void GridBase<T, rank, CheckingPolicy, StoragePolicy>::resize(const IndexType &d)
  {
    IndexType high;
    for (size_t i=0; i<rank; ++i) high[i] = d[i]-1;
    StoragePolicy::resize(IndexType::Zero(), high);
  }

  template<
    typename T,
    size_t rank,
    class CheckingPolicy,
    class StoragePolicy
  >
  void GridBase<T, rank, CheckingPolicy, StoragePolicy>::resize(const IndexType &low, const IndexType &high)
  {
    StoragePolicy::resize(low, high);
  }

  template<
    typename T,
    size_t rank,
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
    StoragePolicy::resize(grid.getLo(), grid.getHi());
  }

}

//=================================================================
//============================= Grid ==============================
//=================================================================

template<
  typename T,
  size_t rank,
  template<size_t> class CheckingPolicy,
  template<typename, size_t> class StoragePolicy
>
Grid<T, rank, CheckingPolicy, StoragePolicy>::Grid()
  : internal::GridBase<T, rank, CheckingPolicy<rank>,  StoragePolicy<T,Rank> >()
{}

template<
  typename T,
  size_t rank,
  template<size_t> class CheckingPolicy,
  template<typename, size_t> class StoragePolicy
>
Grid<T, rank, CheckingPolicy, StoragePolicy>::Grid(const IndexType &size)
  : internal::GridBase<T, rank, CheckingPolicy<rank>,  StoragePolicy<T,Rank> >(size)
{}

template<
  typename T,
  size_t rank,
  template<size_t> class CheckingPolicy,
  template<typename, size_t> class StoragePolicy
>
Grid<T, rank, CheckingPolicy, StoragePolicy>::Grid(const IndexType &low, const IndexType &high)
  : internal::GridBase<T, rank, CheckingPolicy<rank>,  StoragePolicy<T,Rank> >(low, high)
{}


//template<
//  typename T,
//  size_t rank,
//  template<size_t> class CheckingPolicy,
//  template<typename, size_t> class StoragePolicy
//>
//template<typename Arg0>
//IndexedGrid<
//  Grid<T, rank, CheckingPolicy, StoragePolicy>,
//  TYPELIST_1(Arg0)
//>
//Grid<T, rank, CheckingPolicy, StoragePolicy>::operator()(
//  const Arg0 &i0
//)
//{
//  return IndexedGrid<GridType,TYPELIST_1(Arg0)> (i0);
//}
//
//template<
//  typename T,
//  size_t rank,
//  template<size_t> class CheckingPolicy,
//  template<typename, size_t> class StoragePolicy
//>
//template<typename Arg0, typename Arg1>
//IndexedGrid<
//  Grid<T, rank, CheckingPolicy, StoragePolicy>,
//  TYPELIST_2(Arg0, Arg1)
//>
//Grid<T, rank, CheckingPolicy, StoragePolicy>::operator()(
//  const Arg0 &i0, const Arg1 &i1
//)
//{
//  return IndexedGrid<GridType,TYPELIST_2(Arg0, Arg1)> (i0, i1);
//}


} // namespace
