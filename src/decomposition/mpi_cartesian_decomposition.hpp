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

#include <mpi.h>

#ifdef SCHNEK_HAVE_MPI

namespace schnek {

template<int rank, template<int> class CheckingPolicy = ArrayNoArgCheck>
class MpiCartesianDomainDecomposition : public DomainDecomposition<rank, CheckingPolicy> {
  public:

    /**
     * Initialisation and load balancing based on global weights
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
     */
    int getUniqueId() const;

    /**
     * Check if the current process is the master
     */
    bool master() const;

  private:
    /// The number of processes
    int ComSize;

    /// The rank of the current process
    int ComRank;

    /// The Comm object referring to the Cartesian process grid
    MPI_Comm comm;

    /// Dimensions of the Cartesian topology
    int dims[rank];

    /// The Cartesian coordinates of this process
    int mycoord[rank];

    /**
     * The grid index ranges of each process
     */
    RangeType procRanges[rank];

    /**
     * Determine the new grid layout based on the local or global weights
     */
    void calcGridDistributon(RangeType &ranges[rank]);

    /**
     * Determine the new grid layout based on the local or global weights
     */
    void calcGridDistributonGlobalWeights(RangeType &ranges[rank]);

    /**
     * Determine the new grid layout based on the local or global weights
     */
    void calcGridDistributonLocalWeights(RangeType &ranges[rank]);
};

} // namespace schnek

#include "mpi_cartesian_decomposition.t"

#endif // HAVE_MPI

#endif /* SCHNEK_DECOMPOSITION_MPI_CARTESIAN_DECOMPOSITION_HPP_ */
