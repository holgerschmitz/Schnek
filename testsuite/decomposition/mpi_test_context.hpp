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
    struct AllreduceCallInfo {
      bool sendBufferPresent;
      int count;
      MPI_Datatype datatype;
      MPI_Op op;
      MPI_Comm comm;
    };
    std::vector<AllreduceCallInfo> args_MPI_Allreduce;
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
    std::vector<boost::tuple<int, std::vector<char>>> ret_MPI_Allreduce;
    std::vector<boost::tuple<int, std::vector<char>>> ret_MPI_Sendrecv;

    struct IsendCallInfo {
      bool sendBufferPresent;
      int count;
      MPI_Datatype datatype;
      int dest;
      int tag;
      MPI_Comm comm;
    };
    std::vector<IsendCallInfo> args_MPI_Isend;

    struct IrecvCallInfo {
      bool recvBufferPresent;
      int count;
      MPI_Datatype datatype;
      int source;
      int tag;
      MPI_Comm comm;
    };
    std::vector<IrecvCallInfo> args_MPI_Irecv;

    struct WaitallCallInfo {
      int count;
    };
    std::vector<WaitallCallInfo> args_MPI_Waitall;

    std::vector<boost::tuple<MPI_Comm, std::vector<int>>> args_MPI_Cart_rank;

    std::vector<boost::tuple<MPI_Comm, std::vector<int>>> args_MPI_Cart_sub;
    std::vector<MPI_Comm> args_MPI_Comm_free;

    std::vector<boost::tuple<MPI_Comm, int, int>> args_MPI_Comm_split;
    std::vector<boost::tuple<MPI_Comm, bool, bool, int, MPI_Datatype, MPI_Op>> args_MPI_Exscan;

    // Isend return values: error code
    std::vector<int> ret_MPI_Isend;
    // Irecv return values: error code plus the data to fill into the recv buffer
    std::vector<boost::tuple<int, std::vector<char>>> ret_MPI_Irecv;
    // Waitall return values: error code
    std::vector<int> ret_MPI_Waitall;
    // Cart_rank return values: error code plus resulting rank
    std::vector<boost::tuple<int, int>> ret_MPI_Cart_rank;

    // Cart_sub return values: error code plus sub-communicator handle
    std::vector<boost::tuple<int, MPI_Comm>> ret_MPI_Cart_sub;

    // Comm_split return values: error code plus new communicator handle
    std::vector<boost::tuple<int, MPI_Comm>> ret_MPI_Comm_split;

    // Exscan return values: error code plus the data to fill into the recv buffer
    std::vector<boost::tuple<int, std::vector<char>>> ret_MPI_Exscan;

    // Comm_free return value: error code
    std::vector<int> ret_MPI_Comm_free;

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
            int MPI_Allreduce(const void *sendbuf,
                      void *recvbuf,
                      int count,
                      MPI_Datatype datatype,
                      MPI_Op op,
                      MPI_Comm comm);
    int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm);
    int MPI_Isend(
        const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request
    );
    int MPI_Irecv(
        void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request
    );
    int MPI_Waitall(int count, MPI_Request array_of_requests[], MPI_Status array_of_statuses[]);
    int MPI_Cart_rank(MPI_Comm comm, const int coords[], int *rank);
    int MPI_Cart_sub(MPI_Comm comm, const int remain_dims[], MPI_Comm *newcomm);
    int MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm);
    int MPI_Exscan(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
    int MPI_Comm_free(MPI_Comm *comm);
};
