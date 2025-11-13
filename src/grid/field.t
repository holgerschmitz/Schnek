/*
 * field.t
 *
 * Created on: 9 May 2012
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

#include "field.hpp"

namespace schnek {

  template<typename T, size_t rank, template<typename, size_t> class ...Policies>
  Field<T, rank, Policies...>::Field() {}

  template<typename T, size_t rank, template<typename, size_t> class ...Policies>
  template<
      template<size_t>
      class ArrayCheckingPolicy,
      template<size_t>
      class RangeCheckingPolicy,
      template<size_t>
      class StaggerCheckingPolicy>
  Field<T, rank, Policies...>::Field(
      const Array<size_t, rank, ArrayCheckingPolicy> &size,
      const Range<double, rank, RangeCheckingPolicy> &domain,
      const Array<bool, rank, StaggerCheckingPolicy> &stagger,
      size_t ghostCells
  )
      : Grid<T, rank, Policies...>(), domain(domain), stagger(stagger), ghostCells(ghostCells) {
    IndexType low{IndexType::Zero()};
    IndexType high{size};
    for (size_t i = 0; i < rank; ++i) {
      low[i] -= ghostCells;
      high[i] += ghostCells - 1;
    }
    this->Grid<T, rank, Policies...>::resize(low, high);
  }

  template<typename T, size_t rank, template<typename, size_t> class ...Policies>
  template<
      template<size_t>
      class ArrayCheckingPolicy,
      template<size_t>
      class RangeCheckingPolicy,
      template<size_t>
      class StaggerCheckingPolicy>
  Field<T, rank, Policies...>::Field(
      const Array<ptrdiff_t, rank, ArrayCheckingPolicy> &low,
      const Array<ptrdiff_t, rank, ArrayCheckingPolicy> &high,
      const Range<double, rank, RangeCheckingPolicy> &domain,
      const Array<bool, rank, StaggerCheckingPolicy> &stagger,
      size_t ghostCells
  )
      : Grid<T, rank, Policies...>(), domain(domain), stagger(stagger), ghostCells(ghostCells) {
    IndexType lo{low};
    IndexType hi{high};
    for (size_t i = 0; i < rank; ++i) {
      lo[i] -= ghostCells;
      hi[i] += ghostCells;
    }

    this->Grid<T, rank, Policies...>::resize(lo, hi);
  }

  template<typename T, size_t rank, template<typename, size_t> class ...Policies>
  template<
      template<size_t>
      class ArrayCheckingPolicy,
      template<size_t>
      class RangeCheckingPolicy,
      template<size_t>
      class StaggerCheckingPolicy>
  Field<T, rank, Policies...>::Field(
      const Range<ptrdiff_t, rank, ArrayCheckingPolicy> &range,
      const Range<double, rank, RangeCheckingPolicy> &domain,
      const Array<bool, rank, StaggerCheckingPolicy> &stagger,
      size_t ghostCells
  )
      : Grid<T, rank, Policies...>(), domain(domain), stagger(stagger), ghostCells(ghostCells) {
    IndexType lo{range.getLo()};
    IndexType hi{range.getHi()};
    for (size_t i = 0; i < rank; ++i) {
      lo[i] -= ghostCells;
      hi[i] += ghostCells;
    }

    this->Grid<T, rank, Policies...>::resize(lo, hi);
  }

  template<typename T, size_t rank, template<typename, size_t> class ...Policies>
  Field<T, rank, Policies...>::Field(const Field<T, rank, Policies...> &field)
      : Grid<T, rank, Policies...>(field),
        domain(field.domain),
        stagger(field.stagger),
        ghostCells(field.ghostCells) {}

  template<typename T, size_t rank, template<typename, size_t> class ...Policies>
  inline void Field<T, rank, Policies...>::positionToIndex(
      size_t dim, double pos, ptrdiff_t &index, double &offset
  ) {
    ptrdiff_t lo = this->getLo()[dim];
    ptrdiff_t hi = this->getHi()[dim];
    double xnorm =
        (pos - domain.getLo()[dim]) * (hi - lo - 2 * ghostCells + 1) / (domain.getHi()[dim] - domain.getLo()[dim]) -
        0.5 * int(stagger[dim]) + ghostCells + lo;
    index = ptrdiff_t(floor(xnorm));
    offset = xnorm - index;
  }

  template<typename T, size_t rank, template<typename, size_t> class ...Policies>
  template<
      template<size_t>
      class ArrayCheckingPolicy,
      template<size_t>
      class RangeCheckingPolicy,
      template<size_t>
      class StaggerCheckingPolicy>
  void Field<T, rank, Policies...>::resize(
      const Array<size_t, rank, ArrayCheckingPolicy> &size_,
      const Range<double, rank, RangeCheckingPolicy> &domain_,
      const Array<bool, rank, StaggerCheckingPolicy> &stagger_,
      size_t ghostCells_
  ) {
    domain = domain_;
    stagger = stagger_;
    ghostCells = ghostCells_;
    IndexType low(IndexType::Zero());
    IndexType high(size_);
    for (size_t i = 0; i < rank; ++i) {
      low[i] -= ghostCells;
      high[i] += ghostCells - 1;
    }
    this->Grid<T, rank, Policies...>::resize(low, high);
  }

  template<typename T, size_t rank, template<typename, size_t> class ...Policies>
  template<
      template<size_t>
      class ArrayCheckingPolicy,
      template<size_t>
      class RangeCheckingPolicy,
      template<size_t>
      class StaggerCheckingPolicy>
  void Field<T, rank, Policies...>::resize(
      const Array<ptrdiff_t, rank, ArrayCheckingPolicy> &low_,
      const Array<ptrdiff_t, rank, ArrayCheckingPolicy> &high_,
      const Range<double, rank, RangeCheckingPolicy> &domain_,
      const Array<bool, rank, StaggerCheckingPolicy> &stagger_,
      size_t ghostCells_
  ) {
    domain = domain_;
    stagger = stagger_;
    ghostCells = ghostCells_;
    IndexType low(low_);
    IndexType high(high_);
    for (size_t i = 0; i < rank; ++i) {
      low[i] -= ghostCells;
      high[i] += ghostCells;
    }

    this->Grid<T, rank, Policies...>::resize(low, high);
  }

  template<typename T, size_t rank, template<typename, size_t> class ...Policies>
  template<
      template<size_t>
      class ArrayCheckingPolicy,
      template<size_t>
      class RangeCheckingPolicy,
      template<size_t>
      class StaggerCheckingPolicy>
  void Field<T, rank, Policies...>::resize(
      const Range<ptrdiff_t, rank, ArrayCheckingPolicy> &range_,
      const Range<double, rank, RangeCheckingPolicy> &domain_,
      const Array<bool, rank, StaggerCheckingPolicy> &stagger_,
      size_t ghostCells_
  ) {
    domain = domain_;
    stagger = stagger_;
    ghostCells = ghostCells_;
    IndexType low(range_.getLo());
    IndexType high(range_.getHi());
    for (size_t i = 0; i < rank; ++i) {
      low[i] -= ghostCells;
      high[i] += ghostCells;
    }

    this->Grid<T, rank, Policies...>::resize(low, high);
  }

  template<typename T, size_t rank, template<typename, size_t> class ...Policies>
  inline ptrdiff_t Field<T, rank, Policies...>::positionToIndex(size_t dim, double pos) {
    ptrdiff_t lo = this->getLo()[dim];
    ptrdiff_t hi = this->getHi()[dim];
    return ptrdiff_t(floor(
        (pos - domain.getLo()[dim]) * (hi - lo - 2 * ghostCells + 1) / (domain.getHi()[dim] - domain.getLo()[dim]) -
        0.5 * int(stagger[dim]) + ghostCells + lo
    ));
  }

  template<typename T, size_t rank, template<typename, size_t> class ...Policies>
  inline double Field<T, rank, Policies...>::indexToPosition(size_t dim, ptrdiff_t index) {
    ptrdiff_t lo = this->getLo()[dim];
    ptrdiff_t hi = this->getHi()[dim];

    return (domain.getHi()[dim] - domain.getLo()[dim]) * (index - lo + 0.5 * int(stagger[dim]) - ghostCells) /
               (hi - lo - 2 * ghostCells + 1) +
           domain.getLo()[dim];
  }

}  // namespace schnek
