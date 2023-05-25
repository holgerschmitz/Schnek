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
        Kokkos::InitArguments args;
        args.num_threads = 0;
        args.num_numa = 0;
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

