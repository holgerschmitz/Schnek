/*
 * mpi_test_context.cpp
 *
 *  Created on: 25 Sep 2020
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 */

#include "mpi_test_context.hpp"

#include <algorithm>
#include <cstring>

MPI_Comm MpiTestContextImpl::getCommWorld()
{
  return commWorld;
}

int MpiTestContextImpl::MPI_Comm_size(MPI_Comm comm, int *commSize)
{
  size_t argsCount = args_MPI_Comm_size.size();

  args_MPI_Comm_size.push_back(comm);

  auto retVal =  ret_MPI_Comm_size[std::min(argsCount, ret_MPI_Comm_size.size() - 1)];
  *commSize = retVal.get<1>();

  return retVal.get<0>();
}

int MpiTestContextImpl::MPI_Comm_rank(MPI_Comm comm, int* rank)
{
  size_t argsCount = args_MPI_Comm_rank.size();

  args_MPI_Comm_rank.push_back(comm);

  auto retVal =  ret_MPI_Comm_rank[std::min(argsCount, ret_MPI_Comm_rank.size() - 1)];
  *rank = retVal.get<1>();

  return retVal.get<0>();
}

int MpiTestContextImpl::MPI_Cart_create(MPI_Comm comm_old, int ndims, const int dims[], const int periods[],
                                        int reorder, MPI_Comm* comm_cart)
{
  size_t argsCount = args_MPI_Cart_create.size();
  std::vector<int> vdims(dims, dims+ndims);
  std::vector<int> vperiods(periods, periods+ndims);
  args_MPI_Cart_create.push_back(boost::make_tuple(comm_old,
                                                   ndims,
                                                   vdims,
                                                   vperiods,
                                                   reorder));

  auto retVal =  ret_MPI_Cart_create[std::min(argsCount, ret_MPI_Cart_create.size() - 1)];
  *comm_cart = retVal.get<1>();

  return retVal.get<0>();
}

int MpiTestContextImpl::MPI_Cart_coords(MPI_Comm comm, int rank, int maxdims, int coords[])
{
  size_t argsCount = args_MPI_Cart_coords.size();

  args_MPI_Cart_coords.push_back(boost::make_tuple(comm, rank, maxdims));

  auto retVal =  ret_MPI_Cart_coords[std::min(argsCount, ret_MPI_Cart_coords.size() - 1)];
  std::vector<int> retCoords = retVal.get<1>();
  for (int i=0; i<= retCoords.size(); ++i)
  {
    coords[i] = retCoords[i];
  }

  return retVal.get<0>();
}

int MpiTestContextImpl::MPI_Bcast(void* buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
{
  size_t argsCount = args_MPI_Bcast.size();

  args_MPI_Bcast.push_back(boost::make_tuple(count, datatype, root, comm));

  auto retVal =  ret_MPI_Bcast[std::min(argsCount, ret_MPI_Bcast.size() - 1)];
  memcpy(buffer, retVal.get<1>(), retVal.get<2>());

  return retVal.get<0>();
}
