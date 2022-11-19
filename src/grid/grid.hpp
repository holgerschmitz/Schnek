/*
 * grid.h
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

#ifndef SCHNEK_GRID_H_
#define SCHNEK_GRID_H_

#include "array.hpp"
#include "range.hpp"
#include "gridcheck.hpp"
#include "gridstorage.hpp"
#include "../typetools.hpp"

namespace schnek {

template<class GridType, typename TList>
class IndexedGrid;

template<class IndexType>
struct IndexCast;

template<class Operator, int Length>
class ArrayExpression;

/** An elementary grid class */
template<
  typename T, 
  size_t rank,
  class CheckingPolicy,
  class StoragePolicy
>
class GridBase : public StoragePolicy, public CheckingPolicy
{
  public:
    typedef T value_type;
    typedef CheckingPolicy CheckingPolicyType;
    typedef StoragePolicy StoragePolicyType;
    typedef typename CheckingPolicy::IndexType IndexType;
    typedef GridBase<T,rank,CheckingPolicy,StoragePolicy> GridBaseType;
    enum {Rank = rank};

    GridBase();

    template<template<int> class ArrayCheckingPolicy>
    GridBase(const Array<int,rank,ArrayCheckingPolicy> &size);

    template<template<int> class ArrayCheckingPolicy>
    GridBase(const Array<int,rank,ArrayCheckingPolicy> &low, const Array<int,rank,ArrayCheckingPolicy> &high);

    /** index operator, writing */
    template<template<int> class ArrayCheckingPolicy>
    T& operator[](const Array<int,rank,ArrayCheckingPolicy>& pos); // write
    /** index operator, reading */
    template<template<int> class ArrayCheckingPolicy>
    T  operator[](const Array<int,rank,ArrayCheckingPolicy>& pos) const; // read

    /** index operator, writing */
    template<class Operator, int Length>
    T& operator[](const ArrayExpression<Operator, Length>& pos); // write
    /** index operator, reading */
    template<class Operator, int Length>
    T  operator[](const ArrayExpression<Operator, Length>& pos) const; // read

    /** index operator, for 1D grids, writing */
    T& operator[](int i);
    /** index operator, for 1D grids, reading */
    T  operator[](int i) const;

    /** index operator, writing */
    T& operator()(int i);
    /** index operator, reading */
    T  operator()(int i) const;
    /** index operator, writing */
    T& operator()(int i, int j);
    /** index operator, reading */
    T  operator()(int i, int j) const;
    /** index operator, writing */
    T& operator()(int i, int j, int k);
    /** index operator, reading */
    T  operator()(int i, int j, int k) const;
    /** index operator, writing */
    T& operator()(int i, int j, int k, int l);
    /** index operator, reading */
    T  operator()(int i, int j, int k, int l) const;
    /** index operator, writing */
    T& operator()(int i, int j, int k, int l, int m);
    /** index operator, reading */
    T  operator()(int i, int j, int k, int l, int m) const;
    /** index operator, writing */
    T& operator()(int i, int j, int k, int l, int m, int o);
    /** index operator, reading */
    T  operator()(int i, int j, int k, int l, int m, int o) const;
    /** index operator, writing */
    T& operator()(int i, int j, int k, int l, int m, int o, int p);
    /** index operator, reading */
    T  operator()(int i, int j, int k, int l, int m, int o, int p) const;
    /** index operator, writing */
    T& operator()(int i, int j, int k, int l, int m, int o, int p, int q);
    /** index operator, reading */
    T  operator()(int i, int j, int k, int l, int m, int o, int p, int q) const;
    /** index operator, writing */
    T& operator()(int i, int j, int k, int l, int m, int o, int p, int q, int r);
    /** index operator, reading */
    T  operator()(int i, int j, int k, int l, int m, int o, int p, int q, int r) const;
    /** index operator, writing */
    T& operator()(int i, int j, int k, int l, int m, int o, int p, int q, int r, int s);
    /** index operator, reading */
    T  operator()(int i, int j, int k, int l, int m, int o, int p, int q, int r, int s) const;

    /** assign another grid */
    GridBase<T, rank, CheckingPolicy, StoragePolicy>&
      operator=(const GridBase<T, rank, CheckingPolicy, StoragePolicy>&);

    /** assign another grid */
    template<
      typename T2,
      class CheckingPolicy2,
      class StoragePolicy2
    >
    GridBase<T, rank, CheckingPolicy, StoragePolicy>&
      operator=(const GridBase<T2, rank, CheckingPolicy2, StoragePolicy2>&);

    /** assign a value */
    GridBase<T, rank, CheckingPolicy, StoragePolicy>&
      operator=(const T &val);

    template<
      typename T2,
      class CheckingPolicy2
    >
    GridBase<T, rank, CheckingPolicy, StoragePolicy>&
      operator-=(GridBase<T2, rank, CheckingPolicy2, StoragePolicy>&);

    template<
      typename T2,
      class CheckingPolicy2,
      class StoragePolicy2
    >
    GridBase<T, rank, CheckingPolicy, StoragePolicy>&
      operator-=(GridBase<T2, rank, CheckingPolicy2, StoragePolicy2>&);

    template<
      typename T2,
      class CheckingPolicy2
    >
    GridBase<T, rank, CheckingPolicy, StoragePolicy>&
      operator+=(GridBase<T2, rank, CheckingPolicy2, StoragePolicy>&);


    template<
      typename T2,
      class CheckingPolicy2,
      class StoragePolicy2
    >
    GridBase<T, rank, CheckingPolicy, StoragePolicy>&
      operator+=(GridBase<T2, rank, CheckingPolicy2, StoragePolicy2>&);

    /** Resize to size[0] x ... x size[rank-1]
     *
     *  Example:
     *  \begin{verbatim}
     *  Grid<double,2>::IndexType size=(512,512);
     *  Grid<double,2> m;
     *  m.resize(size);
     *  \end{verbatim}
     *
     *  The ranges then extend from 0 to size[i]-1
     */
    void resize(const IndexType &size);

    /** Resize to lower indices low[0],...,low[rank-1]
     *  and upper indices high[0],...,high[rank-1]
     *
     *  Example:
     *  \begin{verbatim}
     *  Grid<double,2>::IndexType low(-5,-10);
     *  Grid<double,2>::IndexType high(15,36);
     *  Grid<double,2> m;
     *  m.resize(l,h);
     *  \end{verbatim}
     *
     *  The ranges then extend from low[i] to high[i]
     */
    void resize(const IndexType &low, const IndexType &high);

    /** Resize to match the size of another matrix */
    template<
      typename T2,
      class CheckingPolicy2,
      class StoragePolicy2
    >
    void resize(const GridBase<T2, rank, CheckingPolicy2, StoragePolicy2>& grid);

  protected:
    // assumes that the sizes are already set properly
    template<typename T2, class CheckingPolicy2>
    void copyFromGrid(const GridBase<T2, rank, CheckingPolicy2, StoragePolicy>& grid);

};



/** An elementary grid class
 *
 *  The reason we inherit from GridBase, is for ease of specifying template arguments.
 */
template<
  typename T,
  size_t rank,
  template<int> class CheckingPolicy = GridNoArgCheck,
  template<typename, size_t> class StoragePolicy = SingleArrayGridStorage
>
class Grid : public GridBase<T, rank, CheckingPolicy<rank>,  StoragePolicy<T,rank> >
{

  public:
    typedef T value_type;
    typedef Array<int,rank> IndexType;
    typedef Range<int,rank> RangeType;
    typedef Grid<T,rank,CheckingPolicy,StoragePolicy> GridType;
    typedef GridBase<T, rank, CheckingPolicy<rank>, StoragePolicy<T,rank> > BaseType;
    enum {Rank = rank};
  
    /** default constructor creates an empty grid */
    Grid();

    /** constructor, which builds Grid of size size[0] x ... x size[rank-1]
     * 
     *  Example: 
     *  \begin{verbatim}
     *  Grid<double,2>::IndexType size=(512,512);
     *  Grid<double,2> m(size);
     *  \end{verbatim}
     * 
     *  The ranges then extend from 0 to size[i]-1
     */
    Grid(const IndexType &size);

    /** constructor, which builds Grid with lower indices low[0],...,low[rank-1]
     *  and upper indices high[0],...,high[rank-1]
     * 
     *  Example: 
     *  \begin{verbatim}
     *  Grid<double,2>::IndexType low(-5,-10);
     *  Grid<double,2>::IndexType high(15,36);
     *  Grid<double,2> m(l,h);
     *  \end{verbatim}
     * 
     *  The ranges then extend from low[i] to high[i]
     */
    Grid(const IndexType &low, const IndexType &high);

    /** constructor, which builds Grid with range given by range
     *
     *  Example:
     *  \begin{verbatim}
     *  Grid<double,2>::IndexType low(-5,-10);
     *  Grid<double,2>::IndexType high(15,36);
     *  Grid<double,2>::RangeType range(low, high);
     *  Grid<double,2> m(range);
     *  \end{verbatim}
     *
     *  The ranges then extend from low[i] to high[i]
     */
    Grid(const RangeType &range);

    /** copy constructor */
    Grid(const Grid<T, rank, CheckingPolicy, StoragePolicy>&);

//
//    template<typename Arg0>
//    IndexedGrid<GridType, TYPELIST_1(Arg0) > operator()(
//      const Arg0 &i0
//    );
//
//    template<typename Arg0, typename Arg1>
//    IndexedGrid<GridType, TYPELIST_2(Arg0, Arg1) > operator()(
//      const Arg0 &i0, const Arg1 &i1
//    );


    /** assign another grid */
    GridType& operator=(const T &val)
    {
      BaseType::operator=(val);
      return *this;
    }

    /** assign another grid */
    GridType& operator=(const GridType &grid)
    {
      BaseType::operator=(grid);
      return *this;
    }

    /** assign another grid */
    template<
      typename T2,
      class CheckingPolicy2,
      class StoragePolicy2
    >
    GridType& operator=(const GridBase<T2, rank, CheckingPolicy2, StoragePolicy2> &grid)
    {
      BaseType::operator=(grid);
      return *this;
    }
};

} // namespace schnek

#include "grid.t"

#endif // SCHNEK_GRID_H_
