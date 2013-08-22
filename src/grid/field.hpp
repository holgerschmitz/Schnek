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
#include "domain.hpp"
#include "grid.hpp"

namespace schnek {

template<
  typename T,
  int rank,
  template<int> class CheckingPolicy = GridAssertCheck,
  template<typename, int> class StoragePolicy = SingleArrayGridStorage
>
class Field : public Grid<T, rank, CheckingPolicy, StoragePolicy>
{
  public:
    typedef Range<double, rank> FieldRange;
    typedef Array<bool, rank> Stagger;
    typedef typename Grid<T, rank, CheckingPolicy, StoragePolicy>::IndexType IndexType;
    typedef Field<T, rank, CheckingPolicy, StoragePolicy> FieldType;
    typedef GridBase<T, rank, CheckingPolicy<rank>, StoragePolicy<T,rank> > BaseType;
  private:
    FieldRange range;
    Stagger stagger;
    /// Stores the size of the simulation domain
    IndexType size;
    int ghostCells;
  public:
    /** default constructor creates an empty grid */
    Field();

    /** Constructs a grid with a given number of cells in each direction
     *
     */
    Field(const IndexType &size, const FieldRange &range_, const Stagger &stagger_, int ghostCells_);

    Field(const IndexType &low_, const IndexType &high_, const FieldRange &range_, const Stagger &stagger_, int ghostCells_);

    /** copy constructor */
    Field(const Field<T, rank, CheckingPolicy, StoragePolicy>&);

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

    Stagger& getStagger() { return stagger; }
    bool getStagger(int i) { return stagger[i]; }

    /** assign a value to the field*/
    FieldType& operator=(const T &val)
    {
      BaseType::operator=(val);
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
      size = grid.size;
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
};

} //namespace

#include "field.t"

#endif // SCHNEK_FIELD_HPP_
