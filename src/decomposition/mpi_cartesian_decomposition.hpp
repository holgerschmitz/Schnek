/*
 * mpi_cartesian_decomposition.hpp
 *
 *  Created on: 10 Apr 2020
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 */

#ifndef SCHNEK_DECOMPOSITION_MPI_CARTESIAN_DECOMPOSITION_HPP_
#define SCHNEK_DECOMPOSITION_MPI_CARTESIAN_DECOMPOSITION_HPP_

#include "../schnek_config.hpp"
#include "domaindecomposition.hpp"
#include "mpi_context.hpp"

#ifdef SCHNEK_HAVE_MPI

#include <mpi.h>

namespace schnek {

template<int rank, template<int> class CheckingPolicy = ArrayNoArgCheck>
class MpiCartesianDomainDecomposition : public DomainDecomposition<rank, CheckingPolicy>
{
  public:
    typedef Array<Grid<Range<int, 1>, 1>, rank> ProcRanges;
    /**
     * Constructor creating the domain decomposition object
     *
     * @param mpi  the MPI context to use for MPI calls
     */
    MpiCartesianDomainDecomposition(MpiContext &mpi = schnek::detail::mpiContextImpl);

    /**
     * Initialisation and load balancing based on global weights
     *
     * The processor layout is determined by the global grid size or, if specified, by the
     * global weights.
     */
    void init();

    /**
     * Perform load balancing based on local weights if present, otherwise based
     * on global weights.
     *
     * Should automatically redistribute all grids added on any local domain
     * context obtained from this decomposition.
     */
    void balanceLoad();

    /**
     * Return a unique reproducible ID of the current process
     *
     * @return the unique ID
     */
    int getUniqueId() const;

    /**
     * Check if the current process is the master
     *
     * @return `true` if this is the master process
     */
    bool master() const;

    /**
     * Get the number of processes
     *
     * @return the number of processes in the MPI communicator
     */
    int numProcs() const;

    /**
     * Return the grid index ranges of each process coordinates in each direction
     *
     * @return An array with an entry for each dimension. For each dimension multiple ranges
     *         are stored inside a 1d Grid
     */
    const ProcRanges& getProcRanges();
  private:
    typedef typename DomainDecomposition<rank, CheckingPolicy>::LimitType LimitType;
    typedef typename DomainDecomposition<rank, CheckingPolicy>::RangeType RangeType;

    /// The MPI context
    MpiContext &mpi;

    /// The number of processes
    int ComSize;

    /// The rank of the current process
    int ComRank;

    /// The Comm object referring to the Cartesian process grid
    MPI_Comm comm;

    /// Dimensions of the Cartesian topology
    LimitType dims;

    /// The Cartesian coordinates of this process
    LimitType myCoord;

    /**
     * The grid index ranges of each process coordinates in each direction
     */
    ProcRanges procRanges;

    /**
     * Determine the new grid layout based on the local or global weights
     */
    void calcGridDistributon(ProcRanges &ranges);

    /**
     * Determine the new grid layout when no weights are given
     */
    void calcGridDistributonUniform(ProcRanges &ranges);

    /**
     * Determine the new grid layout based on the global weights
     */
    void calcGridDistributonGlobalWeights(ProcRanges &ranges);

    /**
     * Determine the new grid layout based on the local weights
     */
    void calcGridDistributonLocalWeights(ProcRanges &ranges);
};

} // namespace schnek

#include "mpi_cartesian_decomposition.t"

#endif // HAVE_MPI

#endif /* SCHNEK_DECOMPOSITION_MPI_CARTESIAN_DECOMPOSITION_HPP_ */
