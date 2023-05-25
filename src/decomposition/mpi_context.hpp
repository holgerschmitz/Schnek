/*
 * mpi_context.hpp
 *
 *  Created on: 25 Sep 2020
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 */

#ifndef SCHNEK_DECOMPOSITION_MPI_CONTEXT_HPP_
#define SCHNEK_DECOMPOSITION_MPI_CONTEXT_HPP_

#include "../config.hpp"

#ifdef SCHNEK_HAVE_MPI

#include <mpi.h>

namespace schnek {

/**
 * Wrapper around MPI functions
 *
 * Use this wrapper to access any MPI functions to allow unit testing
 */
class MpiContext
{
  public:
    virtual ~MpiContext() {}
    virtual MPI_Comm getCommWorld() = 0;
    virtual int MPI_Comm_size( MPI_Comm comm, int *commSize ) = 0;
    virtual int MPI_Comm_rank( MPI_Comm comm, int *rank ) = 0;
    virtual int MPI_Cart_create(MPI_Comm comm_old,
                                int ndims,
                                const int dims[],
                                const int periods[],
                                int reorder,
                                MPI_Comm* comm_cart) = 0;
    virtual int MPI_Cart_coords(MPI_Comm comm, int rank, int maxdims, int coords[]) = 0;
    virtual int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm) = 0;
};

/**
 * Implementation of the MPI context that calls the real MPI functions
 */
class MpiContextImpl : public MpiContext
{
  private:
    MPI_Comm commWorld;
  public:
    MpiContextImpl(MPI_Comm commWorld);
    MPI_Comm getCommWorld();
    int MPI_Comm_size( MPI_Comm comm, int *commSize );
    int MPI_Comm_rank( MPI_Comm comm, int *rank );
    int MPI_Cart_create(MPI_Comm comm_old,
                        int ndims,
                        const int dims[],
                        const int periods[],
                        int reorder,
                        MPI_Comm* comm_cart);
    int MPI_Cart_coords(MPI_Comm comm, int rank, int maxdims, int coords[]);
    int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm);
};

namespace detail {
  extern MpiContextImpl mpiContextImpl;
}

} // namespace schnek

#endif

#endif /* SCHNEK_DECOMPOSITION_MPI_CONTEXT_HPP_ */
