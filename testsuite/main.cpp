#define BOOST_TEST_MODULE "Unit Tests for Schnek"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <boost/test/included/unit_test.hpp>

#include "utility.hpp"

#include <grid/iteration/kokkos-iteration.hpp>

#ifdef SCHNEK_HAVE_MPI
#include <mpi.h>
#endif

#pragma GCC diagnostic pop

#ifdef SCHNEK_HAVE_KOKKOS

class KokkosInitialiser
{
  public:

    KokkosInitialiser() {
        std::cerr << "KOKKOS INIT\n";
        Kokkos::initialize(Kokkos::InitializationSettings()
            .set_num_threads(1));
    }

    ~KokkosInitialiser() {
        Kokkos::finalize();
    }
};

#else

struct KokkosInitialiser {};

#endif

BOOST_GLOBAL_FIXTURE( KokkosInitialiser );

#ifdef SCHNEK_HAVE_MPI

struct MpiInitialiser {
        MpiInitialiser() {
            int initialized = 0;
            MPI_Initialized(&initialized);
            if (!initialized) {
                int argc = 0;
                char **argv = nullptr;
                MPI_Init(&argc, &argv);
            }
        }

        ~MpiInitialiser() {
            int finalized = 0;
            MPI_Finalized(&finalized);
            if (!finalized) {
                MPI_Finalize();
            }
        }
};

BOOST_GLOBAL_FIXTURE( MpiInitialiser );

#endif

// Run a test
// ./schnek_tests --log_level=test_suite --run_test=some/specific/test

