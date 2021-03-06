/*
 * field.hpp
 *
 * Created on: 14 May 2012
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

#ifndef SCHNEK_FIELD_HPP_
#define SCHNEK_FIELD_HPP_

#include "range.hpp"
#include "grid.hpp"

namespace schnek {

template<
  typename T,
  int rank,
  template<int> class CheckingPolicy = GridNoArgCheck,
  template<typename, int> class StoragePolicy = SingleArrayGridStorage
>
class Field : public Grid<T, rank, CheckingPolicy, StoragePolicy>
{
  public:
    typedef T value_type;
    typedef Range<double, rank> RangeType;
    typedef typename Range<double, rank>::LimitType RangeLimit;
    typedef Array<bool, rank> Stagger;
    typedef typename Grid<T, rank, CheckingPolicy, StoragePolicy>::IndexType IndexType;
    typedef Field<T, rank, CheckingPolicy, StoragePolicy> FieldType;
    typedef GridBase<T, rank, CheckingPolicy<rank>, StoragePolicy<T,rank> > BaseType;
  private:
    RangeType range;
    Stagger stagger;
    int ghostCells;
  public:
    /** default constructor creates an empty grid */
    Field();

    /** Constructs a grid with a given number of cells in each direction
     *
     */
    template<
      template<int> class ArrayCheckingPolicy,
      template<int> class RangeCheckingPolicy,
      template<int> class StaggerCheckingPolicy>
    Field(const Array<int,rank,ArrayCheckingPolicy> &size,
        const Range<double, rank,RangeCheckingPolicy> &range_,
        const Array<bool, rank, StaggerCheckingPolicy> &stagger_, int ghostCells_);

    template<
      template<int> class ArrayCheckingPolicy,
      template<int> class RangeCheckingPolicy,
      template<int> class StaggerCheckingPolicy>
    Field(const Array<int,rank,ArrayCheckingPolicy> &low_,
        const Array<int,rank,ArrayCheckingPolicy> &high_,
        const Range<double, rank,RangeCheckingPolicy> &range_,
        const Array<bool, rank, StaggerCheckingPolicy> &stagger_, int ghostCells_);

    /** copy constructor */
    Field(const FieldType&);

    /** Get the lo if the inner domain */
    IndexType getInnerLo() {return this->getLo()+ghostCells;}

    /** Get the hi if the inner domain */
    IndexType getInnerHi() {return this->getHi()-ghostCells;}

    /** Calculates index and offset from a position on the field
     *
     * The index returned is the index of the grid point left of the position.
     */
    void positionToIndex(int dim, double pos, int &index, double &offset);

    /** Calculates index from a position on the field
     *
     * The index returned is the index of the grid point left of the position.
     */
    int positionToIndex(int dim, double pos);

    /// Calculates the position of a grid point
    double indexToPosition(int dim, int index);

    /// Get all three components of the grid stagger
    Stagger& getStagger() { return stagger; }

    /// Get a single component of the grid stagger
    bool getStagger(int i) { return stagger[i]; }

    /** assign a value to the field*/
    FieldType& operator=(const T &val)
    {
      BaseType::operator=(val);
      return *this;
    }

    /** assign another grid */
    FieldType& operator=(const FieldType &grid)
    {
      BaseType::operator=(grid);
      range = grid.range;
      stagger = grid.stagger;
      ghostCells = grid.ghostCells;
      return *this;
    }

    /** assign another grid */
    template<
      typename T2,
      template<int> class CheckingPolicy2,
      template<typename, int> class StoragePolicy2
    >
    FieldType& operator=(const Field<T2, rank, CheckingPolicy2, StoragePolicy2> &grid)
    {
      BaseType::operator=(grid);
      range = grid.range;
      stagger = grid.stagger;
      ghostCells = grid.ghostCells;
      return *this;
    }

    /** assign another grid */
    template<
      typename T2,
      class CheckingPolicy2,
      class StoragePolicy2
    >
    FieldType& operator=(const GridBase<T2, rank, CheckingPolicy2, StoragePolicy2> &grid)
    {
      BaseType::operator=(grid);
      return *this;
    }

    /** Constructs a grid with a given number of cells in each direction
     *
     */
    template<
      template<int> class ArrayCheckingPolicy,
      template<int> class RangeCheckingPolicy,
      template<int> class StaggerCheckingPolicy>
    void resize(const Array<int,rank,ArrayCheckingPolicy> &size,
                const Range<double, rank,RangeCheckingPolicy> &range_,
                const Array<bool, rank, StaggerCheckingPolicy> &stagger_, int ghostCells_);

    template<
      template<int> class ArrayCheckingPolicy,
      template<int> class RangeCheckingPolicy,
      template<int> class StaggerCheckingPolicy>
    void resize(const Array<int,rank,ArrayCheckingPolicy> &low_,
                const Array<int,rank,ArrayCheckingPolicy> &high_,
                const Range<double, rank,RangeCheckingPolicy> &range_,
                const Array<bool, rank, StaggerCheckingPolicy> &stagger_, int ghostCells_);

};

} //namespace

#include "field.t"

#endif // SCHNEK_FIELD_HPP_
