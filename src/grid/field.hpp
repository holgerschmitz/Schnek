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

#include "grid.hpp"
#include "range.hpp"

#include <cstddef>

namespace schnek {

  template<typename T, size_t rank, template<typename, size_t> class ...Policies>
  class Field : public Grid<T, rank, Policies...> {
    public:
      typedef T value_type;
      typedef typename Grid<T, rank, Policies...>::IndexType IndexType;
      typedef typename Grid<T, rank, Policies...>::RangeType RangeType;
      typedef Range<double, rank> DomainType;
      typedef typename Range<double, rank>::LimitType RangeLimit;
      typedef Array<bool, rank> StaggerType;
      typedef Field<T, rank, Policies...> FieldType;
      typedef Grid<T, rank, Policies...> BaseType;

    private:
      DomainType domain;
      StaggerType stagger;
      size_t ghostCells;

    public:
      /** default constructor creates an empty grid */
      Field();

      /** Constructs a grid with a given number of cells in each direction
       *
       */
      template<
          template<size_t>
          class ArrayCheckingPolicy,
          template<size_t>
          class RangeCheckingPolicy,
          template<size_t>
          class StaggerCheckingPolicy>
      Field(
          const Array<size_t, rank, ArrayCheckingPolicy> &size,
          const Range<double, rank, RangeCheckingPolicy> &domain,
          const Array<bool, rank, StaggerCheckingPolicy> &stagger,
          size_t ghostCells
      );

      template<
          template<size_t>
          class ArrayCheckingPolicy,
          template<size_t>
          class RangeCheckingPolicy,
          template<size_t>
          class StaggerCheckingPolicy>
      Field(
          const Array<ptrdiff_t, rank, ArrayCheckingPolicy> &low,
          const Array<ptrdiff_t, rank, ArrayCheckingPolicy> &high,
          const Range<double, rank, RangeCheckingPolicy> &domain,
          const Array<bool, rank, StaggerCheckingPolicy> &stagger,
          size_t ghostCells
      );

      template<
          template<size_t>
          class ArrayCheckingPolicy,
          template<size_t>
          class RangeCheckingPolicy,
          template<size_t>
          class StaggerCheckingPolicy>
      Field(
          const Range<ptrdiff_t, rank, ArrayCheckingPolicy> &range,
          const Range<double, rank, RangeCheckingPolicy> &domain,
          const Array<bool, rank, StaggerCheckingPolicy> &stagger,
          size_t ghostCells
      );

      /** copy constructor */
      Field(const FieldType &);

      /** Get the lo of the inner grid range */
      IndexType getInnerLo() { return this->getLo() + ptrdiff_t(ghostCells); }

      /** Get the hi of the inner grid range */
      IndexType getInnerHi() { return this->getHi() - ptrdiff_t(ghostCells); }

      /** Get the range the inner grid range */
      RangeType getInnerRange() { return RangeType{this->getLo() + ptrdiff_t(ghostCells), this->getHi() - ptrdiff_t(ghostCells)}; }

      /** Calculates index and offset from a position on the field
       *
       * The index returned is the index of the grid point left of the position.
       */
      void positionToIndex(size_t dim, double pos, ptrdiff_t &index, double &offset);

      /** Calculates index from a position on the field
       *
       * The index returned is the index of the grid point left of the position.
       */
      ptrdiff_t positionToIndex(size_t dim, double pos);

      /// Calculates the position of a grid point
      double indexToPosition(size_t dim, ptrdiff_t index);

      /// Get all three components of the grid stagger
      StaggerType &getStagger() { return stagger; }

      /// Get a single component of the grid stagger
      bool getStagger(size_t i) { return stagger[i]; }

      /**
       * @brief Get the physical domain of the field
       */
      const DomainType &getDomain() { return domain; }

      /**
       * @brief Assignment operator
       */
      FieldType &operator=(const FieldType &) = default;

      /** assign a value to the field*/
      FieldType &operator=(const T &val) {
        BaseType::operator=(val);
        return *this;
      }

      /** Constructs a grid with a given number of cells in each direction
       *
       */
      template<
          template<size_t>
          class ArrayCheckingPolicy,
          template<size_t>
          class RangeCheckingPolicy,
          template<size_t>
          class StaggerCheckingPolicy>
      void resize(
          const Array<size_t, rank, ArrayCheckingPolicy> &size,
          const Range<double, rank, RangeCheckingPolicy> &domain,
          const Array<bool, rank, StaggerCheckingPolicy> &stagger,
          size_t ghostCells
      );

      template<
          template<size_t>
          class ArrayCheckingPolicy,
          template<size_t>
          class RangeCheckingPolicy,
          template<size_t>
          class StaggerCheckingPolicy>
      void resize(
          const Array<ptrdiff_t, rank, ArrayCheckingPolicy> &low,
          const Array<ptrdiff_t, rank, ArrayCheckingPolicy> &high,
          const Range<double, rank, RangeCheckingPolicy> &domain,
          const Array<bool, rank, StaggerCheckingPolicy> &stagger,
          size_t ghostCells
      );

      template<
          template<size_t>
          class ArrayCheckingPolicy,
          template<size_t>
          class RangeCheckingPolicy,
          template<size_t>
          class StaggerCheckingPolicy>
      void resize(
          const Range<ptrdiff_t, rank, ArrayCheckingPolicy> &range,
          const Range<double, rank, RangeCheckingPolicy> &domain,
          const Array<bool, rank, StaggerCheckingPolicy> &stagger,
          size_t ghostCells
      );
  };

}  // namespace schnek

#include "field.t"

#endif  // SCHNEK_FIELD_HPP_
