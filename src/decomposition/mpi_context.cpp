/*
 * mpi_context.cpp
 *
 *  Created on: 25 Sep 2020
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 */

#include "mpi_context.hpp"


schnek::MpiContextImpl::MpiContextImpl(MPI_Comm commWorld) : commWorld(commWorld)
{
}

MPI_Comm schnek::MpiContextImpl::getCommWorld()
{
  return commWorld;
}

int schnek::MpiContextImpl::MPI_Comm_size(MPI_Comm comm, int* commSize)
{
  return MPI_Comm_size(comm, commSize);
}

int schnek::MpiContextImpl::MPI_Comm_rank(MPI_Comm comm, int* rank)
{
  return MPI_Comm_rank(comm, rank);
}

int schnek::MpiContextImpl::MPI_Cart_create(MPI_Comm comm_old, int ndims, const int dims[], const int periods[],
                                            int reorder, MPI_Comm* comm_cart)
{
  return MPI_Cart_create(comm_old, ndims, dims, periods, reorder, comm_cart);
}

int schnek::MpiContextImpl::MPI_Cart_coords(MPI_Comm comm, int rank, int maxdims, int coords[])
{
  return MPI_Cart_coords(comm, rank, maxdims, coords);
}

int schnek::MpiContextImpl::MPI_Bcast(void* buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
{
  return MPI_Bcast(buffer, count, datatype, root, comm);
}

