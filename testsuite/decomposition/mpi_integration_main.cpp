#define BOOST_TEST_MODULE "Schnek MPI Integration Tests"
#define BOOST_TEST_NO_MAIN

#include <mpi.h>

#include <boost/test/included/unit_test.hpp>

int main(int argc, char *argv[]) {
  MPI_Init(&argc, &argv);
  const int result = boost::unit_test::unit_test_main(&init_unit_test_suite, argc, argv);
  MPI_Finalize();
  return result;
}
