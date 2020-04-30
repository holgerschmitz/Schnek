/*
 * mpi_cartesian_decomposition.t
 *
 *  Created on: 30 Apr 2020
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 */


#include "../util/factor.hpp"
#include "../util/logger.hpp"
#include "../util/exceptions.hpp"

#include "../diagnostic/diagnostic.hpp"

namespace schnek {

template<int rank, template<int> class CheckingPolicy>
void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::init()
{
  // Arrange the processes in a Cartesian grid topology
  LimitType lo = globalRange.getLo();
  LimitType hi = globalRange.getLo();

  MPI_Comm_size(MPI_COMM_WORLD, &ComSize);

  int periodic[rank];

  std::vector<int> box(rank);

  for (int i=0; i<rank; ++i)
  {
    box[i] = hi[i] - lo[i];
    periodic[i] = true;
  }

  std::vector<int> eqDims;

  equalFactors(ComSize, rank, eqDims, box);

  std::copy(eqDims.begin(), eqDims.end(), dims);

  int errorCode;
  errorCode = MPI_Cart_create(MPI_COMM_WORLD, rank, dims, periodic, true, &comm);
  SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "Could not create MPI Cartesian topology ("+boost::lexical_cast<std::string>(errorCode)+")");

  errorCode = MPI_Comm_rank(comm, &ComRank);
  SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "Could not determine MPI rank ("+boost::lexical_cast<std::string>(errorCode)+")");

  errorCode = MPI_Cart_coords(comm, ComRank, rank, mycoord);
  SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "Could not determine MPI Cartesian coordinates ("+boost::lexical_cast<std::string>(errorCode)+")");

  // Determine the sizes of the local grids
  calcGridDistributon(procRanges);
}

template<int rank, template<int> class CheckingPolicy>
void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::calcGridDistributon(RangeType &ranges[rank])
{
  if (localWeights.getDims().product() == 0)
  {
    calcGridDistributonGlobalWeights(ranges);
  }
  else
  {
    calcGridDistributonLocalWeights(ranges);
  }
}

template<int rank, template<int> class CheckingPolicy>
void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::calcGridDistributonGlobalWeights(RangeType &ranges[rank])
{
  if (master())
  {
    // create a tabulated cumulative weight function for each direction
    // interpolate the inverse of the function to obtain the segments in that direction
    // broadcast solution to all other processes
  }
  else
  {
    // wait for receiving the subdivision from the master
  }
}

} // namespace schnek
