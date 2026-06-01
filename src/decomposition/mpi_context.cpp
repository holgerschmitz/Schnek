/*
 * mpi_context.cpp
 *
 *  Created on: 25 Sep 2020
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 */

#include "mpi_context.hpp"

schnek::MpiContextImpl::MpiContextImpl(MPI_Comm commWorld) : commWorld(commWorld) {}

MPI_Comm schnek::MpiContextImpl::getCommWorld() {
  return commWorld;
}

int schnek::MpiContextImpl::MPI_Comm_size(MPI_Comm comm, int* commSize) {
  return ::MPI_Comm_size(comm, commSize);
}

int schnek::MpiContextImpl::MPI_Comm_rank(MPI_Comm comm, int* rank) {
  return ::MPI_Comm_rank(comm, rank);
}

int schnek::MpiContextImpl::MPI_Cart_create(
    MPI_Comm comm_old, int ndims, const int dims[], const int periods[], int reorder, MPI_Comm* comm_cart
) {
  return ::MPI_Cart_create(comm_old, ndims, dims, periods, reorder, comm_cart);
}

int schnek::MpiContextImpl::MPI_Cart_coords(MPI_Comm comm, int rank, int maxdims, int coords[]) {
  return ::MPI_Cart_coords(comm, rank, maxdims, coords);
}

int schnek::MpiContextImpl::MPI_Cart_shift(MPI_Comm comm, int direction, int disp, int* rank_source, int* rank_dest) {
  return ::MPI_Cart_shift(comm, direction, disp, rank_source, rank_dest);
}

int schnek::MpiContextImpl::MPI_Sendrecv(
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
) {
  return ::MPI_Sendrecv(
      sendbuf, sendcount, sendtype, dest, sendtag, recvbuf, recvcount, recvtype, source, recvtag, comm, status
  );
}

int schnek::MpiContextImpl::MPI_Allreduce(
    const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm
) {
  return ::MPI_Allreduce(sendbuf, recvbuf, count, datatype, op, comm);
}

int schnek::MpiContextImpl::MPI_Bcast(void* buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm) {
  return ::MPI_Bcast(buffer, count, datatype, root, comm);
}

int schnek::MpiContextImpl::MPI_Isend(
    const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request* request
) {
  return ::MPI_Isend(buf, count, datatype, dest, tag, comm, request);
}

int schnek::MpiContextImpl::MPI_Irecv(
    void* buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request* request
) {
  return ::MPI_Irecv(buf, count, datatype, source, tag, comm, request);
}

int schnek::MpiContextImpl::MPI_Waitall(int count, MPI_Request array_of_requests[], MPI_Status array_of_statuses[]) {
  return ::MPI_Waitall(count, array_of_requests, array_of_statuses);
}

int schnek::MpiContextImpl::MPI_Cart_rank(MPI_Comm comm, const int coords[], int* rank) {
  return ::MPI_Cart_rank(comm, coords, rank);
}

int schnek::MpiContextImpl::MPI_Cart_sub(MPI_Comm comm, const int remain_dims[], MPI_Comm* newcomm) {
  return ::MPI_Cart_sub(comm, remain_dims, newcomm);
}

int schnek::MpiContextImpl::MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm* newcomm) {
  return ::MPI_Comm_split(comm, color, key, newcomm);
}

int schnek::MpiContextImpl::MPI_Exscan(
    const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm
) {
  return ::MPI_Exscan(sendbuf, recvbuf, count, datatype, op, comm);
}

int schnek::MpiContextImpl::MPI_Comm_free(MPI_Comm* comm) {
  return ::MPI_Comm_free(comm);
}

namespace schnek::detail {
  MpiContextImpl mpiContextImpl{MPI_COMM_WORLD};
}
