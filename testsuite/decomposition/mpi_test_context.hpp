/*
 * mpi_test_context.hpp
 *
 *  Created on: 25 Sep 2020
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 */


#include <decomposition/mpi_context.hpp>

#include <vector>
#include <boost/tuple/tuple.hpp>

/**
 * Implementation of the MPI context that calls the real MPI functions
 */
class MpiTestContextImpl : public schnek::MpiContext
{
  public:
    MPI_Comm commWorld;

    std::vector<MPI_Comm> args_MPI_Comm_size;
    std::vector<MPI_Comm> args_MPI_Comm_rank;
    std::vector<boost::tuple<MPI_Comm, int, std::vector<int>, std::vector<int>, int>> args_MPI_Cart_create;
    std::vector<boost::tuple<MPI_Comm, int, int>> args_MPI_Cart_coords;
    std::vector<boost::tuple<int, MPI_Datatype, int, MPI_Comm>> args_MPI_Bcast;
    std::vector<boost::tuple<MPI_Comm, int, int>> args_MPI_Cart_shift;
    struct SendrecvCallInfo {
      bool sendBufferPresent;
      int sendCount;
      MPI_Datatype sendType;
      int dest;
      int sendTag;
      bool recvBufferPresent;
      int recvCount;
      MPI_Datatype recvType;
      int source;
      int recvTag;
      MPI_Comm comm;
    };
    std::vector<SendrecvCallInfo> args_MPI_Sendrecv;

    std::vector<boost::tuple<int, int>> ret_MPI_Comm_size;
    std::vector<boost::tuple<int, int>> ret_MPI_Comm_rank;
    std::vector<boost::tuple<int, MPI_Comm>> ret_MPI_Cart_create;
    std::vector<boost::tuple<int, std::vector<int>>> ret_MPI_Cart_coords;
    std::vector<boost::tuple<int, void*, size_t>> ret_MPI_Bcast;
    std::vector<boost::tuple<int, int, int>> ret_MPI_Cart_shift;
    std::vector<boost::tuple<int, std::vector<char>>> ret_MPI_Sendrecv;

    MPI_Comm getCommWorld();
    int MPI_Comm_size(MPI_Comm comm, int *commSize );
    int MPI_Comm_rank(MPI_Comm comm, int *rank );
    int MPI_Cart_create(MPI_Comm comm_old,
                        int ndims,
                        const int dims[],
                        const int periods[],
                        int reorder,
                        MPI_Comm* comm_cart);
    int MPI_Cart_coords(MPI_Comm comm, int rank, int maxdims, int coords[]);
    int MPI_Cart_shift(MPI_Comm comm, int direction, int disp, int *rank_source, int *rank_dest);
    int MPI_Sendrecv(const void *sendbuf,
                     int sendcount,
                     MPI_Datatype sendtype,
                     int dest,
                     int sendtag,
                     void *recvbuf,
                     int recvcount,
                     MPI_Datatype recvtype,
                     int source,
                     int recvtag,
                     MPI_Comm comm,
                     MPI_Status *status);
    int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm);
};
