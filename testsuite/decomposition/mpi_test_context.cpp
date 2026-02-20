/*
 * mpi_test_context.cpp
 *
 *  Created on: 25 Sep 2020
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 */

#include "mpi_test_context.hpp"

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace {
  size_t datatypeSize(MPI_Datatype type) {
    if (type == MPI_CHAR) {
      return sizeof(char);
    } else if (type == MPI_SHORT) {
      return sizeof(short);
    } else if (type == MPI_INT) {
      return sizeof(int);
    } else if (type == MPI_LONG) {
      return sizeof(long);
    } else if (type == MPI_UNSIGNED_CHAR) {
      return sizeof(unsigned char);
    } else if (type == MPI_UNSIGNED_SHORT) {
      return sizeof(unsigned short);
    } else if (type == MPI_UNSIGNED) {
      return sizeof(unsigned int);
    } else if (type == MPI_UNSIGNED_LONG) {
      return sizeof(unsigned long);
    } else if (type == MPI_FLOAT) {
      return sizeof(float);
    } else if (type == MPI_DOUBLE) {
      return sizeof(double);
    } else if (type == MPI_LONG_DOUBLE) {
      return sizeof(long double);
    }
    throw std::runtime_error("Unsupported MPI datatype in test context");
  }
}

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
  for (size_t i=0; i<retCoords.size(); ++i)
  {
    coords[i] = retCoords[i];
  }

  return retVal.get<0>();
}

int MpiTestContextImpl::MPI_Cart_shift(MPI_Comm comm, int direction, int disp, int* rank_source, int* rank_dest)
{
  size_t argsCount = args_MPI_Cart_shift.size();
  args_MPI_Cart_shift.push_back(boost::make_tuple(comm, direction, disp));

  if (ret_MPI_Cart_shift.empty()) {
    if (rank_source) {
      *rank_source = MPI_PROC_NULL;
    }
    if (rank_dest) {
      *rank_dest = MPI_PROC_NULL;
    }
    return MPI_SUCCESS;
  }

  auto retVal = ret_MPI_Cart_shift[std::min(argsCount, ret_MPI_Cart_shift.size() - 1)];
  if (rank_source) {
    *rank_source = retVal.get<1>();
  }
  if (rank_dest) {
    *rank_dest = retVal.get<2>();
  }
  return retVal.get<0>();
}

int MpiTestContextImpl::MPI_Sendrecv(
    const void* sendbuf,
    int sendcount,
    MPI_Datatype sendtype,
    int dest,
    int sendtag,
    void* recvbuf,
    int recvcount,
    MPI_Datatype recvtype,
    int source,
    int recvtag,
    MPI_Comm comm,
    MPI_Status* status
)
{
  size_t argsCount = args_MPI_Sendrecv.size();
  args_MPI_Sendrecv.push_back({sendbuf != nullptr, sendcount, sendtype, dest, sendtag, recvbuf != nullptr, recvcount, recvtype, source, recvtag, comm});

  if (!ret_MPI_Sendrecv.empty()) {
    auto retVal = ret_MPI_Sendrecv[std::min(argsCount, ret_MPI_Sendrecv.size() - 1)];
    const auto& payload = retVal.get<1>();
    if (recvbuf != nullptr && !payload.empty()) {
      std::memcpy(recvbuf, payload.data(), payload.size());
    }
    if (status != nullptr && status != MPI_STATUS_IGNORE) {
      std::memset(status, 0, sizeof(MPI_Status));
    }
    return retVal.get<0>();
  }

  if (sendbuf != nullptr && recvbuf != nullptr && sendcount > 0 && recvcount > 0 && sendtype == recvtype) {
    size_t sendBytes = datatypeSize(sendtype) * static_cast<size_t>(sendcount);
    size_t recvBytes = datatypeSize(recvtype) * static_cast<size_t>(recvcount);
    std::memcpy(recvbuf, sendbuf, std::min(sendBytes, recvBytes));
  }

  if (status != nullptr && status != MPI_STATUS_IGNORE) {
    std::memset(status, 0, sizeof(MPI_Status));
  }

  return MPI_SUCCESS;
}

int MpiTestContextImpl::MPI_Allreduce(
    const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm
)
{
  size_t argsCount = args_MPI_Allreduce.size();
  args_MPI_Allreduce.push_back({sendbuf != nullptr, count, datatype, op, comm});

  if (!ret_MPI_Allreduce.empty()) {
    auto retVal = ret_MPI_Allreduce[std::min(argsCount, ret_MPI_Allreduce.size() - 1)];
    const auto& payload = retVal.get<1>();
    if (recvbuf != nullptr && !payload.empty()) {
      std::memcpy(recvbuf, payload.data(), std::min(payload.size(), datatypeSize(datatype) * static_cast<size_t>(count)));
    }
    return retVal.get<0>();
  }

  if (sendbuf != nullptr && recvbuf != nullptr && count > 0) {
    std::memcpy(recvbuf, sendbuf, datatypeSize(datatype) * static_cast<size_t>(count));
  }

  return MPI_SUCCESS;
}

int MpiTestContextImpl::MPI_Bcast(void* buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
{
  size_t argsCount = args_MPI_Bcast.size();

  args_MPI_Bcast.push_back(boost::make_tuple(count, datatype, root, comm));

  if (!ret_MPI_Bcast.empty()) {
    auto retVal =  ret_MPI_Bcast[std::min(argsCount, ret_MPI_Bcast.size() - 1)];
    memcpy(buffer, retVal.get<1>(), retVal.get<2>());

    return retVal.get<0>();
  }

  // No return data configured — this is the root (sender) case.
  // Real MPI_Bcast does not modify the root's buffer.
  return MPI_SUCCESS;
}

int MpiTestContextImpl::MPI_Isend(
    const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request* request
) {
  size_t argsCount = args_MPI_Isend.size();
  args_MPI_Isend.push_back({buf != nullptr, count, datatype, dest, tag, comm});

  if (request) {
    *request = MPI_REQUEST_NULL;
  }

  if (!ret_MPI_Isend.empty()) {
    return ret_MPI_Isend[std::min(argsCount, ret_MPI_Isend.size() - 1)];
  }

  return MPI_SUCCESS;
}

int MpiTestContextImpl::MPI_Irecv(
    void* buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request* request
) {
  size_t argsCount = args_MPI_Irecv.size();
  args_MPI_Irecv.push_back({buf != nullptr, count, datatype, source, tag, comm});

  if (request) {
    *request = MPI_REQUEST_NULL;
  }

  // Fill the receive buffer with pre-loaded data if available
  if (!ret_MPI_Irecv.empty()) {
    auto retVal = ret_MPI_Irecv[std::min(argsCount, ret_MPI_Irecv.size() - 1)];
    const auto& payload = retVal.get<1>();
    if (buf != nullptr && !payload.empty()) {
      std::memcpy(buf, payload.data(), payload.size());
    }
    return retVal.get<0>();
  }

  return MPI_SUCCESS;
}

int MpiTestContextImpl::MPI_Waitall(int count, MPI_Request array_of_requests[], MPI_Status array_of_statuses[]) {
  size_t argsCount = args_MPI_Waitall.size();
  args_MPI_Waitall.push_back({count});

  // Set all requests to MPI_REQUEST_NULL (as MPI_Waitall does)
  for (int i = 0; i < count; ++i) {
    array_of_requests[i] = MPI_REQUEST_NULL;
  }

  if (!ret_MPI_Waitall.empty()) {
    return ret_MPI_Waitall[std::min(argsCount, ret_MPI_Waitall.size() - 1)];
  }

  return MPI_SUCCESS;
}

int MpiTestContextImpl::MPI_Cart_rank(MPI_Comm comm, const int coords[], int* rank) {
  size_t argsCount = args_MPI_Cart_rank.size();

  // We need to determine the number of dimensions from the stored topology info
  // For the mock, we use the dims from Cart_create args
  int ndims = 1;
  if (!args_MPI_Cart_create.empty()) {
    ndims = args_MPI_Cart_create.back().get<1>();
  }

  std::vector<int> vcoords(coords, coords + ndims);
  args_MPI_Cart_rank.push_back(boost::make_tuple(comm, vcoords));

  if (!ret_MPI_Cart_rank.empty()) {
    auto retVal = ret_MPI_Cart_rank[std::min(argsCount, ret_MPI_Cart_rank.size() - 1)];
    *rank = retVal.get<1>();
    return retVal.get<0>();
  }

  // Default: compute rank from coordinates using row-major ordering
  // using dims from Cart_create
  if (!args_MPI_Cart_create.empty()) {
    const auto &dimsVec = args_MPI_Cart_create.back().get<2>();
    int result = 0;
    for (int d = 0; d < ndims; ++d) {
      result = result * dimsVec[d] + coords[d];
    }
    *rank = result;
  } else {
    *rank = 0;
  }

  return MPI_SUCCESS;
}
