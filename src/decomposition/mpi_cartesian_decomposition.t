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

template<size_t rank, template<size_t> class CheckingPolicy>
MpiCartesianDomainDecomposition<rank, CheckingPolicy>::MpiCartesianDomainDecomposition(MpiContext &mpi) : mpi(mpi)
{}

template<size_t rank, template<size_t> class CheckingPolicy>
bool MpiCartesianDomainDecomposition<rank, CheckingPolicy>::master() const
{
  return ComRank == 0;
}

template<size_t rank, template<size_t> class CheckingPolicy>
int MpiCartesianDomainDecomposition<rank, CheckingPolicy>::numProcs() const
{
  return ComSize;
}

template<size_t rank, template<size_t> class CheckingPolicy>
const Array<Grid<Range<ptrdiff_t, 1>, 1>, rank>& MpiCartesianDomainDecomposition<rank, CheckingPolicy>::getProcRanges()
{
  return procRanges;
}

template<size_t rank, template<size_t> class CheckingPolicy>
void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::balanceLoad()
{
  SCHNECK_FAIL("MpiCartesianDomainDecomposition::balanceLoad() not implemented");
}


template<size_t rank, template<size_t> class CheckingPolicy>
int MpiCartesianDomainDecomposition<rank, CheckingPolicy>::getUniqueId() const
{
  int id = myCoord[0];
  for (size_t i=1; i<rank; ++i) id = dims[i]*id + myCoord[i];

  SCHNEK_TRACE_LOG(2,"MpiCartesianDomainDecomposition::getUniqueId() " << id);
  return id;
}

template<size_t rank, template<size_t> class CheckingPolicy>
void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::init()
{
  int errorCode;
  // Arrange the processes in a Cartesian grid topology
  LimitType lo = this->globalRange.getLo();
  LimitType hi = this->globalRange.getHi();

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

  for (size_t i=0; i<rank; ++i)
  {
    box[i] = hi[i] - lo[i] + 1;
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

  for (size_t i=0; i<rank; ++i)
  {
    dims[i] = dimsRaw[i];
    myCoord[i] = myCoordRaw[i];
  }

  // Determine the sizes of the local grids
  calcGridDistributon(procRanges);
}

template<size_t rank, template<size_t> class CheckingPolicy>
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


template<size_t rank, template<size_t> class CheckingPolicy>
void MpiCartesianDomainDecomposition<rank, CheckingPolicy>
    ::calcGridDistributonUniform(ProcRanges &ranges)
{
  typedef Grid<double, 1> Weights;
  typedef Weights::IndexType Index;
  typedef Range<size_t, rank-1> Orth;

  const LimitType lo = this->globalRange.getLo();
  const LimitType hi = this->globalRange.getHi();
  const LimitType dm = hi - lo + 1;

  for (size_t d=0; d<rank; ++d)
  {
    // finding cut points in the cumulative weights
    Grid<Range<ptrdiff_t, 1>, 1> &dimRanges = ranges[d];

    dimRanges.resize(0, dims[d]-1);
    dimRanges(0).getLo()[0] = lo[d];
    dimRanges(dims[d]-1).getHi()[0] = hi[d];

    for (size_t i=1; i<dims[d]; ++i)
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
template<template<size_t> class CheckingPolicy>
void sumGlobalWeights(const Grid<double, 1, CheckingPolicy> &globalWeights,
                      typename DomainDecomposition<1, CheckingPolicy>::LimitType &lo,
                      typename DomainDecomposition<1, CheckingPolicy>::LimitType &hi,
                      size_t d,
                      Grid<double, 1> &weights,
                      double &sumTotal)
{
  SCHNEK_TRACE_ENTER_FUNCTION(2);
  double sum = 0;
  weights(lo[d]-1) = 0.0;
  for (size_t i=lo[d]; i<=hi[d]; ++i)
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
template<size_t rank, template<size_t> class CheckingPolicy>
void sumGlobalWeights(const Grid<double, rank, CheckingPolicy> &globalWeights,
                      typename DomainDecomposition<rank, CheckingPolicy>::LimitType &lo,
                      typename DomainDecomposition<rank, CheckingPolicy>::LimitType &hi,
                      size_t d,
                      Grid<double, 1> &weights,
                      double &sumTotal)
{
  typedef Range<size_t, rank-1> Orth;
  Orth orth;
  Array<size_t, rank-1> orthInd;

  // calculating orthogonal directions
  size_t oi = 0;
  for (size_t o=0; o<rank; ++o)
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
  weights(lo[d]-1) = 0.0;
  for (size_t i=lo[d]; i<=hi[d]; ++i)
  {
    pos[d] = i;
    typename Orth::iterator e = orth.end();
    for (typename Orth::iterator pi=orth.begin(); pi!=e; ++pi)
    {
      const Array<size_t, rank-1> &p = *pi;
      for (size_t oi=0; oi<rank-1; ++oi)
      {
        pos[orthInd[oi]] = p[oi];
      }

//      std::cout << "Sum: " << pos[0] << " " << pos[1] << " " << globalWeights[pos] << std::endl;

      sumTotal += globalWeights[pos];
    }

    weights(i) = sumTotal;
  }
}

template<size_t rank, template<size_t> class CheckingPolicy>
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

    for (size_t d=0; d<rank; ++d)
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
      Grid<Range<ptrdiff_t, 1>, 1> &dimRanges = ranges[d];
      dimRanges.resize(0, dims[d]-1);
      dimRanges(0).getLo()[0] = glo[d];
      dimRanges(dims[d]-1).getHi()[0] = ghi[d];

      double delta = 1.0/double(dims[d]);
      for (size_t i=1; i<dims[d]; ++i)
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
      for (size_t i=0; i<dims[d]; ++i)
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
    for (size_t d=0; d<rank; ++d)
    {
      Grid<Range<ptrdiff_t, 1>, 1> &dimRanges = ranges[d];
      dimRanges.resize(0, dims[d]-1);

      Grid<int, 1> transfer(2*dims[d]);
      mpi.MPI_Bcast(transfer.getRawData(), 2*dims[d], MPI_INT, 0, comm);
      for (size_t i=0; i<dims[d]; ++i)
      {
        dimRanges(i).getLo()[0] = transfer(2*i);
        dimRanges(i).getHi()[0] = transfer(2*i + 1);
      }
    }
  }
}

template<size_t rank, template<size_t> class CheckingPolicy>
void MpiCartesianDomainDecomposition<rank, CheckingPolicy>
    ::calcGridDistributonLocalWeights(ProcRanges &ranges)
{
}

#undef SCHNEK_LOGLEVEL
#define SCHNEK_LOGLEVEL 0


} // namespace schnek
