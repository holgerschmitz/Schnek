/*
 * mpisubdivision.hpp
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

#ifndef SCHNEK_MPISUBDIVISION_HPP
#define SCHNEK_MPISUBDIVISION_HPP

#include "domainsubdivision.hpp"
#include "../schnek_config.hpp"

#ifdef SCHNEK_HAVE_MPI

#include <mpi.h>

namespace schnek {

/** @brief a boundary class for multiple processor runs
 *
 * Is designed to be exchanged via the MPI protocol.
 * Here splitting is performed in both spatial directions.
 */
template<class GridType>
class MPICartSubdivision : public DomainSubdivision<GridType>
{
  public:
    typedef typename DomainSubdivision<GridType>::LimitType LimitType;
    typedef typename GridType::value_type value_type;
    typedef typename DomainSubdivision<GridType>::DomainType DomainType;
    typedef typename DomainSubdivision<GridType>::BoundaryType BoundaryType;
    typedef typename DomainSubdivision<GridType>::BufferType BufferType;

    enum {Rank = GridType::Rank};
  protected:
    /// The number of processes
    int ComSize;

    /// The rank of the current process
    int ComRank;

    /// The Comm object referring to the cartesian process grid
    MPI_Comm comm;

    LimitType prevcoord; ///< The ranks of the neighbour processes towards the lower boundary
    LimitType nextcoord; ///< The ranks of the neighbour processes towards the higher boundary

    ///dimensions
    int dims[Rank];
    /// The cartesian coordinates of this process
    int mycoord[Rank];

    /** @brief The size of the array that needs to be exchanged,
     *  when the exchange method is called
     */
    int exchSize[Rank];

    value_type *sendarr[Rank]; ///< send buffers for exchanging data
    value_type *recvarr[Rank]; ///< receive buffers for exchanging data

    /// The size of the scalar fields when reducing
    int scalarSize;

  public:
    using DomainSubdivision<GridType>::init;
    using DomainSubdivision<GridType>::exchange;
    ///default constructor
    MPICartSubdivision();

    /// Virtual destructor deleting all the allocated arrays
    ~MPICartSubdivision();

    ///initialize
    void init(const LimitType &low, const LimitType &high, int delta);

    /** @brief Exchanges the boundaries in direction specified by dim.
     *
     *  The outermost simulated cells are sent and the surrounding
     *  ghost cells are filled with values
     */
    void exchange(GridType &field, int dim);

    /** @brief Exchange the boundaries of a field function
     *  summing the data from ghost cells and inner cells
     */
    void accumulate(GridType &grid, int dim);

    /**
     * @param dim
     * @param orientation
     * @param in
     * @param out
     */
    void exchangeData(int dim, int orientation, BufferType &in, BufferType &out);

    /// Use MPIALLReduce to calculate the sum and then divide by the number of processes.
    double avgReduce(double val) const;

    /// Use MPIALLReduce to calculate the maximum
    double maxReduce(double val) const;

    /// Use MPIALLReduce to calculate the maximum
    double sumReduce(double val) const;

    /// The process with the rank zero is designated master process
    bool master() const { return ComRank==0; }

    /// Returns the comm rank as given by mpi
    int procnum() const { return ComRank; }

    /// Return the total number of processes
    int procCount() const { return ComSize; }

    ///returns an ID, which consists of the Dimensions and coordinates
    int getUniqueId() const;

    /** Returns true if this process is on the lower bound of the
     * global domain
     *
     *  The serial domain subdivision does not subdivide. It is always on the bound
     *  of the domain.
     *
     * @param dim The dimension in which to inspect
     * @return A boolean indicating if this process is on the lower boud of the
     * global domain
     */
    bool isBoundLo(int dim) { return mycoord[dim]==0; }

    /** Returns true if this process is on the upper bound of the
     * global domain.
     *
     *  The serial domain subdivision does not subdivide. It is always on the bound
     *  of the domain.
     *
     * @param dim The dimension in which to inspect
     * @return A boolean indicating if this process is on the upper boud of the
     * global domain
     */
    bool isBoundHi(int dim) { return mycoord[dim]==dims[dim]-1; }
};

template<typename value_type>
struct MpiValueType
{
    static const MPI_Datatype value;
};

} // namespace schnek

#endif // HAVE_MPI

#include "mpisubdivision.t"


#endif // SCHNEK_MPISUBDIVISION_HPP


