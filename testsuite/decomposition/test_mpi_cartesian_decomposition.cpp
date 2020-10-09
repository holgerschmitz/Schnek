/*
 *  test_mpi_cartesian_decomposition.cpp
 *
 *  Created on: 8 Oct 2020
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 */

#include "mpi_test_context.hpp"
#include <grid/range.hpp>

#include <decomposition/mpi_cartesian_decomposition.hpp>

#include <boost/progress.hpp>
#include <boost/test/unit_test.hpp>

struct MpiCartesianDomainDecompositionTestFixture
{
    MpiTestContextImpl context;
};

BOOST_AUTO_TEST_SUITE( mpi_cartesian_domain_decomposition )

BOOST_FIXTURE_TEST_CASE( sigle_process_1d, MpiCartesianDomainDecompositionTestFixture )
{
  MPI_Comm testComm = (MPI_Comm)(void*)123;
  std::vector<int> coords = { 0 };
  context.commWorld = (MPI_Comm)(void*)574;
  context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, 1));
  context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, 0));
  context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
  context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));

  schnek::Range<int, 1> globalRange(schnek::Array<int,1>(0), schnek::Array<int,1>(100));
  schnek::Range<double, 1> globalDomain(schnek::Array<double,1>(0), schnek::Array<double,1>(12.5));

  schnek::MpiCartesianDomainDecomposition<1> decomposition(context);
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  decomposition.init();

  // Checking calls
  BOOST_CHECK_EQUAL(context.args_MPI_Comm_size.size(), 1);
  BOOST_CHECK_EQUAL(context.args_MPI_Comm_size[0], context.commWorld);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create.size(), 1);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<0>(), context.commWorld);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<1>(), 1);
  const std::vector<int> cc_dims = context.args_MPI_Cart_create[0].get<2>();
  const std::vector<int> cc_periods = context.args_MPI_Cart_create[0].get<3>();

  BOOST_CHECK_EQUAL(cc_dims.size(), 1);
  BOOST_CHECK_EQUAL(cc_dims[0], 1);

  BOOST_CHECK_EQUAL(cc_periods.size(), 1);
  BOOST_CHECK_EQUAL(cc_periods[0], 1);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<4>(), 1);

  BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank.size(), 1);
  BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank[0], testComm);

  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords.size(), 1);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<0>(), testComm);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<1>(), 0);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<2>(), 1);

  BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), 0);


  BOOST_CHECK_EQUAL(decomposition.master(), true);
  BOOST_CHECK_EQUAL(decomposition.numProcs(), 1);
  BOOST_CHECK_EQUAL(decomposition.getUniqueId(), 0);
  schnek::MpiCartesianDomainDecomposition<1>::ProcRanges ranges = decomposition.getProcRanges();

  BOOST_CHECK_EQUAL(ranges[0].getLo(0), 0);
  BOOST_CHECK_EQUAL(ranges[0].getHi(0), 0);

  schnek::Range<int, 1> expectedRange(schnek::Array<int,1>(0), schnek::Array<int,1>(0));
  BOOST_CHECK_EQUAL(ranges[0](0), expectedRange);
}



BOOST_AUTO_TEST_SUITE_END()
