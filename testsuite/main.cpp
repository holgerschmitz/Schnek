#define BOOST_TEST_MODULE "Unit Tests for Schnek"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <boost/test/included/unit_test.hpp>

#include "utility.hpp"

#include <grid/iteration/kokkos-iteration.hpp>

#pragma GCC diagnostic pop

#ifdef SCHNEK_HAVE_KOKKOS

class KokkosInitialiser
{
  public:

    KokkosInitialiser() {
        std::cerr << "KOKKOS INIT\n";
        Kokkos::InitializationSettings args;

        args.set_num_threads(0);
        args.set_map_device_id_by("random");    // selects a random device from the available devices
        // args.set_map_device_id_by("mpi_rank");  // selects a device based on assignment of local mpi ranks

        Kokkos::initialize(args);
    }

    ~KokkosInitialiser() {
        Kokkos::finalize();
    }
};

#else

struct KokkosInitialiser {};

#endif

BOOST_GLOBAL_FIXTURE( KokkosInitialiser );

// Run a test
// ./schnek_tests --log_level=test_suite --run_test=some/specific/test

