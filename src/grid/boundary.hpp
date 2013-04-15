/*
 * boundary.hpp
 *
 * Created on: 27 Sep 2012
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

#ifndef SCHNEK_BOUNDARY_HPP_
#define SCHNEK_BOUNDARY_HPP_

#include "subgrid.hpp"
#include "field.hpp"

namespace schnek {

template<
  int rank,
  template<int> class CheckingPolicy = ArrayNoArgCheck
>
class Boundary
{
  public:
    typedef RecDomain<rank, CheckingPolicy> DomainType;
    typedef typename DomainType::LimitType LimitType;
    typedef enum {Min, Max} bound;

  private:
    DomainType size;
    int delta;

  public:
    Boundary();
    Boundary(const LimitType &low, const LimitType &high, int delta_);
    Boundary(DomainType &size_, int delta_);

    int getDelta() { return delta; }

    DomainType getGhostDomain(int dim, bound b);
    DomainType getGhostSourceDomain(int dim, bound b);
    DomainType getBoundaryDomain(int dim, bound b, bool stagger);

    DomainType getDomain() { return size; }

    template<class GridType>
    SubGrid<GridType, CheckingPolicy> getGhostBoundary(int dim, bound b, GridType &grid);

    template<
      typename T,
      template<int> class CheckingPolicy2,
      template<typename, int> class StoragePolicy
    >
    SubGrid<Field<T,rank,CheckingPolicy2,StoragePolicy>, CheckingPolicy>
      getGhostBoundary(int dim, bound b, Field<T,rank,CheckingPolicy2,StoragePolicy> &field);

};

} // namespace schnek

#include "boundary.t"

#endif // SCHNEK_BOUNDARY_HPP_




