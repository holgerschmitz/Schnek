/*
 * mpi_cartesian_decomposition.t
 *
 *  Created on: 30 Apr 2020
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 */


#include "../util/factor.hpp"
#include "../util/logger.hpp"
#include "../util/exceptions.hpp"
#include "../util/interpolate1d.hpp"

#include "../diagnostic/diagnostic.hpp"


namespace schnek {

template<int rank, template<int> class CheckingPolicy>
MpiCartesianDomainDecomposition<rank, CheckingPolicy>::MpiCartesianDomainDecomposition(MpiContext &mpi) : mpi(mpi)
{}

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
const Array<Grid<Range<int, 1>, 1>, rank>& MpiCartesianDomainDecomposition<rank, CheckingPolicy>::getProcRanges()
{
  return procRanges;
}

template<int rank, template<int> class CheckingPolicy>
void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::balanceLoad()
{
  SCHNECK_FAIL("MpiCartesianDomainDecomposition::balanceLoad() not implemented");
}


template<int rank, template<int> class CheckingPolicy>
int MpiCartesianDomainDecomposition<rank, CheckingPolicy>::getUniqueId() const
{
  int id = myCoord[0];
  for (int i=1; i<rank; ++i) id = dims[i]*id + myCoord[i];

  SCHNEK_TRACE_LOG(2, "MpiCartesianDomainDecomposition::getUniqueId() " << id)
  return id;
}

template<int rank, template<int> class CheckingPolicy>
void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::init()
{
  int errorCode;
  // Arrange the processes in a Cartesian grid topology
  LimitType lo = this->globalRange.getLo();
  LimitType hi = this->globalRange.getLo();

  // if global weights are specified, use the resolution of those for the processor layout
  if (this->globalWeights.getDims().product() != 0)
  {
    lo = this->globalWeights.getLo();
    hi = this->globalWeights.getHi();
  }

  errorCode = mpi.MPI_Comm_size(mpi.getCommWorld(), &ComSize);
  SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "Could not obtain MPI Comm size ("+boost::lexical_cast<std::string>(errorCode)+")");

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

  errorCode = this->mpi.MPI_Cart_create(this->mpi.getCommWorld(), rank, dimsRaw, periodic, true, &comm);
  SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "Could not create MPI Cartesian topology ("+boost::lexical_cast<std::string>(errorCode)+")");

  errorCode = this->mpi.MPI_Comm_rank(comm, &ComRank);
  SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "Could not determine MPI rank ("+boost::lexical_cast<std::string>(errorCode)+")");

  errorCode = this->mpi.MPI_Cart_coords(comm, ComRank, rank, myCoordRaw);
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
void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::calcGridDistributon(ProcRanges &ranges)
{
  if (this->localWeights.getDims().product() > 0)
  {
    calcGridDistributonLocalWeights(ranges);
  }
  else if (this->globalWeights.getDims().product() > 0)
  {
    calcGridDistributonGlobalWeights(ranges);
  }
  else
  {
    calcGridDistributonUniform(ranges);
  }
}


template<int rank, template<int> class CheckingPolicy>
void MpiCartesianDomainDecomposition<rank, CheckingPolicy>
    ::calcGridDistributonUniform(ProcRanges &ranges)
{
  typedef Grid<double, 1> Weights;
  typedef Weights::IndexType Index;
  typedef Range<int, rank-1> Orth;

  const LimitType lo = this->globalRange.getLo();
  const LimitType hi = this->globalRange.getHi();
  const LimitType dm = hi - lo + 1;

  for (int d=0; d<rank; ++d)
  {
    // finding cut points in the cumulative weights
    Grid<Range<int, 1>, 1> &dimRanges = ranges[d];

    dimRanges.resize(0, dims[d]-1);
    dimRanges(0).getLo()[0] = lo[d];
    dimRanges(dims[d]-1).getHi()[0] = hi[d];

    for (int i=1; i<dims[d]; ++i)
    {
      int cut = lo[d] + (i*dm[d])/dims[d];
      dimRanges(i-1).getHi()[0] = cut - 1;
      dimRanges(i).getLo()[0] = cut;
    }
  }
}

template<int rank, template<int> class CheckingPolicy>
void MpiCartesianDomainDecomposition<rank, CheckingPolicy>
    ::calcGridDistributonGlobalWeights(ProcRanges &ranges)
{
  typedef Grid<double, 1> Weights;
  typedef Weights::IndexType Index;
  typedef Range<int, rank-1> Orth;

  if (master())
  {
    LimitType lo = this->globalWeights.getLo();
    LimitType hi = this->globalWeights.getHi();

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
      pos[d] = oi;
      weights(lo[d]-1) = 0.0;
      for (int i=lo[d]; i<=hi[d]; ++i)
      {
        double sum = 0;
        typename Orth::iterator e = orth.end();
        for (typename Orth::iterator pi=orth.begin(); pi!=e; ++pi)
        {
          const Array<int, rank-1> &p = *pi;
          for (int oi=0; oi<rank-1; ++oi)
          {
            pos[orthInd[oi]] = p[oi];
          }

          sum += this->globalWeights[pos];
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
      dimRanges(0).getLo()[0] = this->globalRange.getLo()[d];
      dimRanges(dims[d]-1).getHi()[0] = this->globalRange.getHi()[d];

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
      mpi.MPI_Bcast(transfer.getRawData(), 2*dims[d], MPI_INT, 0, comm);
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
      mpi.MPI_Bcast(transfer.getRawData(), 2*dims[d], MPI_INT, 0, comm);
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
    ::calcGridDistributonLocalWeights(ProcRanges &ranges)
{
}

} // namespace schnek
