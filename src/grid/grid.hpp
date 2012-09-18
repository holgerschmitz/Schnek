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
#include "gridcheck.hpp"
#include "gridstorage.hpp"
#include "../typetools.hpp"

namespace schnek {

template<class GridType, typename TList>
class IndexedGrid;

template<class IndexType>
struct IndexCast;


/** An elementary grid class */
template<
  typename T, 
  int rank,
  class CheckingPolicy,
  class StoragePolicy
>
class GridBase : public StoragePolicy, public CheckingPolicy
{
  public:
    typedef T value_type;
    typedef Array<int,rank> IndexType;
    typedef GridBase<T,rank,CheckingPolicy,StoragePolicy> GridBaseType;
    enum {Rank = rank};

    GridBase();
    GridBase(const IndexType &size);
    GridBase(const IndexType &low, const IndexType &high);

    /** index operator, writing */
    T& operator[](const IndexType& pos); // write
    /** index operator, reading */
    T  operator[](const IndexType& pos) const; // read
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
};



/** An elementary grid class */
template<
  typename T,
  int rank,
  template<int> class CheckingPolicy = GridNoArgCheck,
  template<typename, int> class StoragePolicy = SingleArrayGridStorage
>
class Grid : public GridBase<T, rank, CheckingPolicy<rank>,  StoragePolicy<T,Rank> >
{

  public:
    typedef T value_type;
    typedef Array<int,rank> IndexType;
    typedef Grid<T,rank,CheckingPolicy,StoragePolicy> GridType;
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

    /** copy constructor */
    Grid(const Grid<T, rank, CheckingPolicy, StoragePolicy>&);


    template<typename Arg0>
    IndexedGrid<GridType, TYPELIST_1(Arg0) > operator()(
      const Arg0 &i0
    );

    template<typename Arg0, typename Arg1>
    IndexedGrid<GridType, TYPELIST_2(Arg0, Arg1) > operator()(
      const Arg0 &i0, const Arg1 &i1
    );
        
    /** assign another grid */
    Grid<T, rank, CheckingPolicy, StoragePolicy>& 
      operator=(const Grid<T, rank, CheckingPolicy, StoragePolicy>&);

    /** assign a value */
    Grid<T, rank, CheckingPolicy, StoragePolicy>& 
      operator=(const T &val);
    
    template<
      template<int> class CheckingPolicy2,
      template<typename, int> class StoragePolicy2
    >
    Grid<T, rank, CheckingPolicy, StoragePolicy>&
      operator-=(Grid<T, rank, CheckingPolicy2, StoragePolicy2>&);

    template<
      template<int> class CheckingPolicy2,
      template<typename, int> class StoragePolicy2
    >
    Grid<T, rank, CheckingPolicy, StoragePolicy>&
      operator+=(Grid<T, rank, CheckingPolicy2, StoragePolicy2>&);

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
    void resize(const Grid<T, rank>& matr);
  private:
    // assumes that the sizes are already set properly
    void copyFromGrid(const Grid<T, rank, CheckingPolicy, StoragePolicy>& matr);
};

} // namespace schnek

#include "grid.t"

#endif // SCHNEK_GRID_H_
