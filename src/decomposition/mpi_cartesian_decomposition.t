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

#undef SCHNEK_LOGLEVEL
#define SCHNEK_LOGLEVEL 0

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

  SCHNEK_TRACE_LOG(2,"MpiCartesianDomainDecomposition::getUniqueId() " << id);
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
      int cut = lo[d] + (long(i)*long(dm[d]))/dims[d];
      dimRanges(i-1).getHi()[0] = cut - 1;
      dimRanges(i).getLo()[0] = cut;
    }
  }
}

/**
 * Sum up the global weights over the perpendicular directions and store
 * them in the 1d `weights` grid
 */
template<template<int> class CheckingPolicy>
void sumGlobalWeights(const Grid<double, 1, CheckingPolicy> &globalWeights,
                      typename DomainDecomposition<1, CheckingPolicy>::LimitType &lo,
                      typename DomainDecomposition<1, CheckingPolicy>::LimitType &hi,
                      int d,
                      Grid<double, 1> &weights,
                      double &sumTotal)
{
  SCHNEK_TRACE_ENTER_FUNCTION(2);
  double sum = 0;
  weights(lo[d]-1) = 0.0;
  for (int i=lo[d]; i<=hi[d]; ++i)
  {
    sum += globalWeights(i);

    weights(i) = sum;
  }

  sumTotal = sum;
}

/**
 * Sum up the global weights over the perpendicular directions and store
 * them in the 1d `weights` grid
 */
template<int rank, template<int> class CheckingPolicy>
void sumGlobalWeights(const Grid<double, rank> &globalWeights,
                      typename DomainDecomposition<rank, CheckingPolicy>::LimitType &lo,
                      typename DomainDecomposition<rank, CheckingPolicy>::LimitType &hi,
                      int d,
                      Grid<double, 1> &weights,
                      double &sumTotal)
{
  typedef Range<int, rank-1> Orth;
  Orth orth;
  Array<int, rank-1> orthInd;

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
  sumTotal = 0;

  typename DomainDecomposition<rank, CheckingPolicy>::LimitType pos;
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

      sum += globalWeights[pos];
    }

    weights(i) = sum;
    sumTotal += sum;
  }
}

template<int rank, template<int> class CheckingPolicy>
void MpiCartesianDomainDecomposition<rank, CheckingPolicy>
    ::calcGridDistributonGlobalWeights(ProcRanges &ranges)
{
  typedef Grid<double, 1> Weights;
  typedef Weights::IndexType Index;

  if (master())
  {
    LimitType lo = this->globalWeights.getLo();
    LimitType hi = this->globalWeights.getHi();
    LimitType glo = this->globalRange.getLo();
    LimitType ghi = this->globalRange.getHi();

    for (int d=0; d<rank; ++d)
    {
      int resolution = (ghi[d] - glo[d] + 1) / (hi[d] - lo[d] + 1);
      Weights weights(Index(lo[d]-1), hi[d]);
      double sumTotal;
      sumGlobalWeights(this->globalWeights, lo, hi, d, weights, sumTotal);

      // normalised cumulative sum
      for (int i=lo[d]; i<=hi[d]; ++i)
      {
        weights(i) = weights(i)/sumTotal;
      }

      // finding cut points in the cumulative weights
      Grid<Range<int, 1>, 1> &dimRanges = ranges[d];
      dimRanges.resize(0, dims[d]-1);
      dimRanges(0).getLo()[0] = glo[d];
      dimRanges(dims[d]-1).getHi()[0] = ghi[d];

      double delta = 1.0/double(dims[d]);
      for (int i=1; i<dims[d]; ++i)
      {
        int ins = findInsertIndex(weights, i*delta);
        if ((ins <= lo[d]) || (ins+1 >= hi[d]))
        {
          for (int k=lo[d]-1; k<=hi[d]; ++k)
          {
            std::cout << "w: " << k << " " << weights(k) << std::endl;
          }
          std::cout << "FAIL: " << d << " " << dims[d] << " " << delta << " " << i*delta << std::endl;
          std::cout << "      " << lo[d]-1 << " " << hi[d] << " " << ins << std::endl;

          SCHNECK_FAIL("Invalid range when attempting to balance load: dim=" << i);
        }

        int cut = glo[d] + resolution*(ins - lo[d] + 1);

        dimRanges(i-1).getHi()[0] = cut - 1;
        dimRanges(i).getLo()[0] = cut;
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

#undef SCHNEK_LOGLEVEL
#define SCHNEK_LOGLEVEL 0


} // namespace schnek
