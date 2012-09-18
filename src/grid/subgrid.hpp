/*
 * subgrid.hpp
 *
 * Created on: 18 Sep 2012
 * Author: hschmitz
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

#ifndef SCHNEK_SUBGRID_HPP_
#define SCHNEK_SUBGRID_HPP_

#include "grid.hpp"
#include "domain.hpp"

namespace schnek {

template<
  typename T,
  int rank,
  class BaseGrid
>
class SubGridStorage {
  public:
    typedef Array<int,rank> IndexType;
    typedef BaseGridType;
  protected:
    typedef RecDomain<rank, CheckingPolicy> DomainType;
    BaseGridType *baseGrid;
    DomainType domain;
    IndexType dims;

  public:

    class storage_iterator {
      protected:
        DomainType::iterator it;
        BaseGridType *baseGrid;
        T* element;
        storage_iterator(DomainType::iterator it_, BaseGridType *baseGrid_)
          : it(it_), baseGrid(baseGrid_), element(&baseGrid->get(*it)) {}

        friend class SubGridStorage;

      public:
        T& operator*() { return *element;}
        storage_iterator &operator++()
        {
          it++;
          element = &baseGrid->get(*it);
          return *this;
        }
        bool operator==(const storage_iterator &SI)
        { return (it==SI.it) && (baseGrid == SI.baseGrid); }

        bool operator!=(const storage_iterator &SI)
        { return (it!=SI.it) || (baseGrid != SI.baseGrid); }
    };

    class const_storage_iterator {
      protected:
        DomainType::iterator it;
        const BaseGridType *baseGrid;
        const T* element;
        const_storage_iterator(const T* element_) : element(element_) {}

        friend class SubGridStorage;

      public:
        const T& operator*() { return *element;}
        const_storage_iterator &operator++()
        {
          it++;
          element = &baseGrid->get(*it);
          return *this;
        }
        bool operator==(const const_storage_iterator &SI)
        { return (it==SI.it) && (baseGrid == SI.baseGrid); }
        bool operator!=(const const_storage_iterator &SI)
        { return (it!=SI.it) || (baseGrid != SI.baseGrid); }
    };

    SubGridStorage();

    SubGridStorage(const IndexType &low_, const IndexType &high_, BaseGridType &baseGrid_);

    virtual ~SubGridStorage();

    void resize(const IndexType &low_, const IndexType &high_);

    T &get(const IndexType &index)
    {
      CheckingPolicy::check(index, domain.getMin(), domain.getMax());
      return baseGrid->get(index);
    }

    const T &get(const IndexType &index)
    {
      CheckingPolicy::check(index, domain.getMin(), domain.getMax());
      return baseGrid->get(index);
    } const;

    /** */
    const IndexType& getLow() const { return domain.getMin(); }
    /** */
    const IndexType& getHigh() const { return domain.getMax(); }
    /** */
    const IndexType& getDims() const { return dims; }

    /** */
    int getLow(int k) const { return domain.getMin()[k]; }
    /** */
    int getHigh(int k) const { return domain.getMax()[k]; }
    /** */
    int getDims(int k) const { return dims[k]; }

    storage_iterator begin() { return storage_iterator(data); }
    storage_iterator end() { return storage_iterator(data + size); }

    const_storage_iterator cbegin() const { return const_storage_iterator(data); }
    const_storage_iterator cend() const { return const_storage_iterator(data + size); }
};

template<
  class BaseGrid,
  template<int> class CheckingPolicy = GridNoArgCheck
>
class SubGrid
  : public GridBase
    <
      typename BaseGrid::value_type,
      BaseGrid::Rank,
      CheckingPolicy<BaseGrid::Rank>,
      SubGridStorage<
        typename BaseGrid::value_type,
        BaseGrid::Rank,
        BaseGrid
      >
    >
{

  public:
    enum {Rank = BaseGrid::Rank};
    typedef typename BaseGrid::value_type value_type;
    typedef Array<int,Rank> IndexType;

    /** default constructor creates an empty grid */
    SubGrid();

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
    SubGrid(const IndexType &size);

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
    SubGrid(const IndexType &low, const IndexType &high);

    /** copy constructor */
    SubGrid(const Grid<T, rank, CheckingPolicy, StoragePolicy>&);

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

    void resize(const IndexType &size);

    void resize(const IndexType &low, const IndexType &high);

    /** Resize to match the size of another matrix */
    void resize(const Grid<T, rank>& matr);
};


} // namespace schnek

#include "subgrid.t"

#endif // SCHNEK_SUBGRID_HPP_
