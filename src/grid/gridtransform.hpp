/*
 * gridtransform.hpp
 *
 * Created on: 24 Sep 2012
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

#ifndef SCHNEK_GRIDTRANSFORM_HPP_
#define SCHNEK_GRIDTRANSFORM_HPP_

#include "grid.hpp"
#include "domain.hpp"

namespace schnek {

template<
  typename T,
  int rank,
  class BaseGrid,
  typename Transformation
>
class GridTransformStorage {
  public:
    typedef Array<int,rank> IndexType;
    typedef BaseGrid BaseGridType;
    typedef Transformation TransformationType;
  protected:
    BaseGridType *baseGrid;
  public:

    class storage_iterator {
      protected:
        typedef BaseGridType::storage_iterator BaseIter;
        BaseIter baseIter;
        storage_iterator(BaseIter baseIter_)
          : baseIter(baseIter) {}

        friend class GridTransformStorage;

      public:
        T& operator*()
        { return Transformation(*baseIter);}

        storage_iterator &operator++()
        {
          ++baseIter;
          return *this;
        }

        bool operator==(const storage_iterator &SI)
        { return baseIter==SI.baseIter; }

        bool operator!=(const storage_iterator &SI)
        { return baseIter!=SI.baseIter; }
    };

    class const_storage_iterator {
      protected:
        typedef BaseGridType::const_storage_iterator BaseIter;
        BaseIter baseIter;
        storage_iterator(BaseIter baseIter_)
          : baseIter(baseIter) {}

        friend class GridTransformStorage;

      public:
        const T& operator*()
        { return Transformation(*baseIter);}

        storage_iterator &operator++()
        {
          ++baseIter;
          return *this;
        }

        bool operator==(const storage_iterator &SI)
        { return baseIter==SI.baseIter; }

        bool operator!=(const storage_iterator &SI)
        { return baseIter!=SI.baseIter; }
    };

    GridTransformStorage();

    T &get(const IndexType &index)
    {
      CheckingPolicy::check(index, domain.getLo(), domain.getHi());
      return Transformation(baseGrid->get(index));
    }

    const T &get(const IndexType &index) const
    {
      CheckingPolicy::check(index, domain.getLo(), domain.getHi());
      return Transformation(baseGrid->get(index));
    }

    /** */
    const IndexType& getLo() const { return baseGrid->getLo(); }
    /** */
    const IndexType& getHi() const { return baseGrid->getHi(); }
    /** */
    const IndexType& getDims() const { return baseGrid->getDims(); }

    /** */
    int getLo(int k) const { return baseGrid->getLo(k); }
    /** */
    int getHi(int k) const { return baseGrid->getHi(k); }
    /** */
    int getDims(int k) const { return baseGrid->getDims(k); }

    storage_iterator begin() { return storage_iterator(baseGrid->begin()); }
    storage_iterator end() { return storage_iterator(baseGrid->end()); }

    const_storage_iterator cbegin() const { return const_storage_iterator(baseGrid->cbegin()); }
    const_storage_iterator cend() const { return const_storage_iterator(baseGrid->cend()); }

    void setBaseGrid(BaseGridType &baseGrid_) { baseGrid = &baseGrid_; }

};

template<
  class BaseGrid,
  typename Transformation,
  template<int> class CheckingPolicy = GridNoArgCheck
>
class GridTransform
  : public GridBase
    <
      typename BaseGrid::value_type,
      BaseGrid::Rank,
      CheckingPolicy<BaseGrid::Rank>,
      GridTransformStorage<
        typename BaseGrid::value_type,
        BaseGrid::Rank,
        BaseGrid,
        Transformation
      >
    >
{
  private:
    typedef GridBase
        <
          typename BaseGrid::value_type,
          BaseGrid::Rank,
          CheckingPolicy<BaseGrid::Rank>,
          SubGridStorage<
            typename BaseGrid::value_type,
            BaseGrid::Rank,
            BaseGrid,
            Transformation
          >
        > ParentType;

  public:
    enum {Rank = BaseGrid::Rank};
    typedef typename BaseGrid::value_type value_type;
    typedef typename BaseGrid::IndexType IndexType;
    typedef BaseGrid BaseGridType;
    /** default constructor creates an empty grid */
    GridTransform();

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
    GridTransform(BaseGridType &baseGrid_);

};


} // namespace schnek

#include "subgrid.t"

#endif // SCHNEK_SUBGRID_HPP_
