/*
 * mpisubdivision.t
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

#ifdef SCHNEK_HAVE_MPI

#include "../util/factor.hpp"
#include "../util/logger.hpp"

#undef LOGLEVEL
#define LOGLEVEL 0

#include <iostream>
#include <vector>

namespace schnek {

/* **************************************************************
 *                 MPICartSubdivision                    *
 ****************************************************************/

template<class GridType>
MPICartSubdivision<GridType>::MPICartSubdivision() : comm(0), prevcoord(0), nextcoord(0)
{}

template<class GridType>
void MPICartSubdivision<GridType>::init(const LimitType &lo, const LimitType &hi, int delta)
{
  LimitType Low(lo);
  LimitType High(hi);

  MPI_Comm_size(MPI_COMM_WORLD, &ComSize);

  int periodic[Rank];

  std::vector<int> box(Rank);

  for (int i=0; i<Rank; ++i)
  {
    box[i] = High[i]-Low[i];
    periodic[Rank] = true;
  }

  std::vector<int> eqDims;

  equalFactors(ComSize, Rank, eqDims, box);

  std::copy(eqDims.begin(), eqDims.end(), dims);

  MPI_Cart_create(MPI_COMM_WORLD,Rank,dims,periodic,true,&comm);
  MPI_Comm_rank(comm,&ComRank);

  MPI_Cart_coords(comm,ComRank,Rank,mycoord);

  double width[Rank];
  int exchangeSizeProduct = delta;

  //std::cout << "Calculating exchange size product: " << exchangeSizeProduct << std::endl;

  for (int i=0; i<Rank; ++i)
  {
    MPI_Cart_shift(comm,i,1,&prevcoord[i],&nextcoord[i]);

    width[i] = (High[i]-1.)/double(dims[i]);

    if (mycoord[i]>0)
      Low[i] = int(width[i]*mycoord[i])-delta+1;
    else
      Low[i] = -delta;

    if (mycoord[i]<(dims[i]-1))
      High[i] = int(width[i]*(mycoord[i]+1))+delta;
    else
      High[i] = High[i]+delta;

    exchangeSizeProduct *= (High[i]-Low[i]+1);
  }

  for (int i=0; i<Rank; ++i)
  {
    exchSize[i] = exchangeSizeProduct/(High[i]-Low[i]+1);
    //std::cout << "Calculating exchange size "<<i<<": " << exchSize[i] << std::endl;
    sendarr[i] = new value_type[exchSize[i]];
    recvarr[i] = new value_type[exchSize[i]];
    for (int k=0; k<exchSize[i]; ++k)
    {
      sendarr[i][k] = value_type();
      recvarr[i][k] = value_type();
    }
  }

  this->bounds = typename DomainSubdivision<GridType>::pBoundaryType(new BoundaryType(Low, High, delta));
}

template<class GridType>
MPICartSubdivision<GridType>::~MPICartSubdivision()
{
  for (int i=0; i<Rank; ++i)
  {
    delete[] sendarr[i];
    delete[] recvarr[i];
  }
  MPI_Comm_free(&comm);
}

template<class GridType>
void MPICartSubdivision<GridType>::exchange(GridType &grid, int dim)
{
  // nothing to be done
  //if (dims[dim]==1) return;

  DomainType loGhost = this->bounds->getGhostDomain(dim, BoundaryType::Min);
  DomainType hiGhost = this->bounds->getGhostDomain(dim, BoundaryType::Max);
  DomainType loSource = this->bounds->getGhostSourceDomain(dim, BoundaryType::Min);
  DomainType hiSource = this->bounds->getGhostSourceDomain(dim, BoundaryType::Max);

  MPI_Status stat;

  value_type *send = sendarr[dim];
  value_type *recv = recvarr[dim];

  MPI_Datatype mpiType = MpiValueType<value_type>::value;

  // fill the lower ghost cells with the vales from higher source cells
  // in the neighbouring process
  {
    int arr_ind = 0;
    typename DomainType::iterator domIt  = hiSource.begin();
    typename DomainType::iterator domEnd = hiSource.end();

    while (domIt != domEnd)
    {
      send[arr_ind++] = grid[*domIt];
      ++domIt;
    }
    if (arr_ind!=exchSize[dim]) {
      std::cerr << "Error "<< dim << "-min: "<< arr_ind << " vs " << exchSize[dim] << std::endl;
    }
  }

  MPI_Sendrecv(send, exchSize[dim], mpiType, nextcoord[dim], 0,
               recv, exchSize[dim], mpiType, prevcoord[dim], 0,
               comm, &stat);
  {
    int arr_ind = 0;
    typename DomainType::iterator domIt  = loGhost.begin();
    typename DomainType::iterator domEnd = loGhost.end();

    while (domIt != domEnd)
    {
      grid[*domIt] = recv[arr_ind++];
      ++domIt;
    }
  }

  // fill the upper ghost cells with the values from lower source cells
  // in the neighbouring process
  {
    int arr_ind = 0;
    typename DomainType::iterator domIt  = loSource.begin();
    typename DomainType::iterator domEnd = loSource.end();

    while (domIt != domEnd)
    {
      send[arr_ind++] = grid[*domIt];
      ++domIt;
    }
    if (arr_ind!=exchSize[dim]) {
      std::cerr << "Error "<< dim << "-max: "<< arr_ind << " vs " << exchSize[dim] << std::endl;
    }
  }

  MPI_Sendrecv(send, exchSize[dim], mpiType, prevcoord[dim], 0,
               recv, exchSize[dim], mpiType, nextcoord[dim], 0,
               comm, &stat);
  {
    int arr_ind = 0;
    typename DomainType::iterator domIt  = hiGhost.begin();
    typename DomainType::iterator domEnd = hiGhost.end();

    while (domIt != domEnd)
    {
      grid[*domIt] = recv[arr_ind++];
      ++domIt;
    }
  }
}


template<class GridType>
void MPICartSubdivision<GridType>::exchangeData(
        int dim,
        int orientation,
        BufferType &in,
        BufferType &out)
{
  typedef typename BufferType::IndexType Index;
  int sendSize = in.getDims(0);
  int recvSize = 0;

  int sendCoord = (orientation>0)?nextcoord[dim]:prevcoord[dim];
  int recvCoord = (orientation>0)?prevcoord[dim]:nextcoord[dim];

  MPI_Status stat;

  MPI_Sendrecv(
      &sendSize, 1, MPI_INT, sendCoord, 0,
      &recvSize, 1, MPI_INT, recvCoord, 0,
      comm, &stat);


  out.resize(Index(recvSize));

  // memcpy(out.getRawData(), in.getRawData(), sendSize*sizeof(value_type));

  MPI_Sendrecv(
      in.getRawData() , sendSize, MPI_UNSIGNED_CHAR, sendCoord, 0,
      out.getRawData(), recvSize, MPI_UNSIGNED_CHAR, recvCoord, 0,
      comm, &stat);
}

template<class GridType>
double MPICartSubdivision<GridType>::avgReduce(double val) const {
  double result;
  MPI_Allreduce(&val, &result, 1, MPI_DOUBLE, MPI_SUM, comm);
  return result/double(ComSize);
}

template<class GridType>
double MPICartSubdivision<GridType>::maxReduce(double val) const {
  double result;
  MPI_Allreduce(&val, &result, 1, MPI_DOUBLE, MPI_MAX, comm);
  return result;
}

template<class GridType>
double MPICartSubdivision<GridType>::sumReduce(double val) const
{
  double result;
  MPI_Allreduce(&val, &result, 1, MPI_DOUBLE, MPI_SUM, comm);
  return result;
}

///returns an ID, which consists of the Dimensions and coordinates
template<class GridType>
int MPICartSubdivision<GridType>::getUniqueId() const
{
  int id = mycoord[0];
  for (int i=1; i<Rank; ++i) id = dims[i]*id + mycoord[i];

  SCHNEK_TRACE_LOG(2, "MPICartSubdivision::getUniqueId() " << id)
  return id;
}


}

#undef LOGLEVEL
#define LOGLEVEL 0

#endif // HAVE_MPI

