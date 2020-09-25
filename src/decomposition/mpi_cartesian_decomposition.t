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
bool MpiCartesianDomainDecomposition<rank, CheckingPolicy>::master() const
{
  return ComRank == 0;
}

template<int rank, template<int> class CheckingPolicy>
int MpiCartesianDomainDecomposition<rank, CheckingPolicy>::numProcs() const
{
  return ComSize;
}

template<int rank, template<int> class CheckingPolicy>
void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::init()
{
  // Arrange the processes in a Cartesian grid topology
  LimitType lo = globalRange.getLo();
  LimitType hi = globalRange.getLo();

  // if global weights are specified, use the resolution of those for the processor layout
  if (globalWeights.getDims().product() != 0)
  {
    lo = globalWeights.getLo();
    hi = globalWeights.getHi();
  }

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

  int dimsRaw[rank];
  int myCoordRaw[rank];
  std::copy(eqDims.begin(), eqDims.end(), dimsRaw);

  int errorCode;
  errorCode = MPI_Cart_create(MPI_COMM_WORLD, rank, dimsRaw, periodic, true, &comm);
  SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "Could not create MPI Cartesian topology ("+boost::lexical_cast<std::string>(errorCode)+")");

  errorCode = MPI_Comm_rank(comm, &ComRank);
  SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "Could not determine MPI rank ("+boost::lexical_cast<std::string>(errorCode)+")");

  errorCode = MPI_Cart_coords(comm, ComRank, rank, myCoordRaw);
  SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "Could not determine MPI Cartesian coordinates ("+boost::lexical_cast<std::string>(errorCode)+")");

  for (int i=0; i<rank; ++i)
  {
    dims[i] = dimsRaw[i];
    myCoord[i] = myCoordRaw[i];
  }

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
void MpiCartesianDomainDecomposition<rank, CheckingPolicy>
    ::calcGridDistributonGlobalWeights(Array<Grid<Range<int, 1>, 1>, rank> &ranges)
{
  typedef Grid<double, 1> Weights;
  typedef Weights::IndexType Index;
  typedef Range<int, rank-1> Orth;

  if (master())
  {
    LimitType lo = globalWeights.getLo();
    LimitType hi = globalWeights.getHi();

    Orth orth;
    Array<int, rank-1> orthInd;

    for (int d=0; d<rank; ++d)
    {
      // calculating orthogonal directions
      int oi = 0;
      for (int o=0; o<rank; ++o)
      {
        if (o != d)
        {
          orth.getLo()[oi] = lo[o];
          orth.getHi()[oi] = hi[o];
          orthInd[oi] = o;
          oi++;
        }
      }

      // summing global weights
      double sumTotal = 0;
      Weights weights(Index(lo[d]-1), hi[d]);
      LimitType pos;
      pos[d] = i;
      weights(lo[d]-1) = 0.0;
      for (int i=lo[d]; i<=hi[d]; ++i)
      {
        double sum = 0;
        typename Orth::iterator e = orth.end();
        for (typename Orth::iterator pi=orth.begin(); pi!=e; ++pi)
        {
          Array<int, rank-1> &p = *pi;
          for (int oi=0; oi<rank-1; ++oi)
          {
            pos[orthInd[oi]] = p[oi];
          }

          sum += globalWeights[pos];
        }

        weights(i) = sum;
        sumTotal += sum;
      }

      // normalised cumulative sum
      for (int i=lo[d]; i<=hi[d]; ++i)
      {
        weights(i) = weights(i)/sumTotal + weights(i-1);
      }

      // finding cut points in the cumulative weights
      Grid<Range<int, 1>, 1> &dimRanges = ranges[d];

      dimRanges.resize(0, dims[d]-1);
      dimRanges(0).getLo()[0] = globalRange.getLo()[d];
      dimRanges(dims[d]-1).getHi()[0] = globalRange.getHi()[d];

      double delta = 1.0/double(dims[d]);
      for (int i=1; i<dims[d]; ++i)
      {
        int cut = findInsertIndex(weights, i*delta);
        if ((cut <= dimRanges(i-1).getLo()[0]) || (cut+1 >= dimRanges(i).getHi()[0]))
        {
          SCHNECK_FAIL("Invalid range when attempting to balance load: dim=" << i);
        }
        dimRanges(i-1).getHi()[0] = cut;
        dimRanges(i).getLo()[0] = cut+1;
      }

      // broadcasting the layout in dimRanges to other processes
      Grid<int, 1> transfer(2*dims[d]);
      for (int i=0; i<dims[d]; ++i)
      {
        transfer(2*i) = dimRanges(i).getLo()[0];
        transfer(2*i + 1) = dimRanges(i).getHi()[0];
      }
      MPI_BCast(transfer.getRawData(), 2*dims[d], MPI_INT, 0, comm);
    }
  }
  else
  {
    // receiving dim ranges for each dimension from the master process
    for (int d=0; d<rank; ++d)
    {
      Grid<Range<int, 1>, 1> &dimRanges = ranges[d];
      dimRanges.resize(0, dims[d]-1);

      Grid<int, 1> transfer(2*dims[d]);
      MPI_BCast(transfer.getRawData(), 2*dims[d], MPI_INT, 0, comm);
      for (int i=0; i<dims[d]; ++i)
      {
        dimRanges(i).getLo()[0] = transfer(2*i);
        dimRanges(i).getHi()[0] = transfer(2*i + 1);
      }
    }
  }
}

template<int rank, template<int> class CheckingPolicy>
void MpiCartesianDomainDecomposition<rank, CheckingPolicy>
    ::calcGridDistributonGlobalWeights(Array<Grid<Range<int, 1>, 1>, rank> &ranges)
{
}

} // namespace schnek
