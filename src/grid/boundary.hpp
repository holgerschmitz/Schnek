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

/** A rectangular region that is aware of ghost cells.
 *
 * Methods are provided that return RecDomain's to the boundary domains
 * on each side of the main domain.
 */
template<
  int rank,
  template<int> class CheckingPolicy = ArrayNoArgCheck
>
class Boundary
{
  public:
    typedef RecDomain<rank, CheckingPolicy> DomainType;
    typedef typename DomainType::LimitType LimitType;

    /** An enum specifying the location of a ghost domain.
     */
    typedef enum {Min, Max} bound;

  private:
    /// The size of the complete domain, including ghost cells
    DomainType size;

    /// The number of ghost cells on each side
    int delta;

  public:
    /** Construct a zero boundary
     */
    Boundary();

    /** Constrauct a boundary supplying lo and hi coordinates and the
     *  number of ghost cells. The domain given should include the ghost cells.
     *
     * @param lo the lower corner of the domain, including the ghost cells
     * @param hi the upper corner of the domain, including the ghost cells
     * @param delta_ the number of ghost cells
     */
    Boundary(const LimitType &lo, const LimitType &hi, int delta_);

    /** Constrauct a boundary supplying a reactangular domain and the
     *  number of ghost cells. The domain given should include the ghost cells.
     *
     * @param size_ the rectangular domain, including the ghost cells
     * @param delta_ the number of ghost cells
     */
    Boundary(DomainType &size_, int delta_);

    /** The number of ghost cells
     *
     * @return the number of ghost cells
     */
    int getDelta() { return delta; }

    /** Returns the original domain, including the ghost cells */
    const DomainType& getDomain() { return size; }

    /** Returns the ghost domain, a rectangular region outside the inner domain.
     * The ghost domain has a thickness given by the number of ghost cells, delta.
     *
     * @param dim the dimanesion index of the side on which the ghost domain lies.
     * @param b the location of the gost domain. Min will return the lower ghost
     *        domain and Max will return the upper ghost domain
     * @return A rectangular domain of ghost cells
     */
    DomainType getGhostDomain(int dim, bound b);

    /** Returns the inner domain corresponding to the ghost domain of the neighbouring
     * process. The domain has a thickness given by the number of ghost cells, delta.
     *
     * @param dim the dimanesion index of the side on which the inner domain lies.
     * @param b the location of the inner domain. Min will return the domain on the lower side
     *        and Max will return the domain on the upper side
     * @return A rectangular domain of source cells
     */
    DomainType getGhostSourceDomain(int dim, bound b);


    DomainType getBoundaryDomain(int dim, bound b, bool stagger);

    /** Returns the inner domain, excluding the ghost cells */
    DomainType getInnerDomain();


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




