/*
 *  test_mpi_cartesian_decomposition.cpp
 *
 *  Created on: 8 Oct 2020
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 */

#include "mpi_test_context.hpp"
#include "../utility.hpp"
#include <grid/range.hpp>

#include <decomposition/mpi_cartesian_decomposition.hpp>

#include <util/array_io.hpp>

#include <boost/progress.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/assign/std/vector.hpp>
using namespace boost::assign; // bring 'operator+=()' into scope

#include <sstream>

//
// run only these tests:
// ./schnek_test --log_level=test_suite --run_test=mpi_cartesian_domain_decomposition

struct MpiCartesianDomainDecompositionTestFixture
{
    MpiTestContextImpl context;

    void resetContext() {
      context.args_MPI_Comm_size.clear();
      context.args_MPI_Comm_rank.clear();
      context.args_MPI_Cart_create.clear();
      context.args_MPI_Cart_coords.clear();
      context.args_MPI_Bcast.clear();

      context.ret_MPI_Comm_size.clear();
      context.ret_MPI_Comm_rank.clear();
      context.ret_MPI_Cart_create.clear();
      context.ret_MPI_Cart_coords.clear();
      context.ret_MPI_Bcast.clear();
    }
};

BOOST_AUTO_TEST_SUITE( mpi_cartesian_domain_decomposition )

BOOST_FIXTURE_TEST_CASE( single_process_1d, MpiCartesianDomainDecompositionTestFixture )
{
  MPI_Comm testComm = (MPI_Comm)(void*)123;
  std::vector<int> coords(1, 0);
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

  schnek::Range<int, 1> expectedRange(schnek::Array<int,1>(0), schnek::Array<int,1>(100));
  SCHNEK_CHECK_EQUAL(ranges[0](0), expectedRange);
}

BOOST_FIXTURE_TEST_CASE( multi_process_1d, MpiCartesianDomainDecompositionTestFixture )
{
  std::vector<int> numProcsArray;
  numProcsArray += 2, 5, 7, 127, 128, 129, 1023, 4000, 25000, 128000, 879484;
  std::vector<int> rankArray;
  rankArray += 1, 2, 4, 120, 126, 500, 1022, 3999, 24999, 127999, 879483;
  int globalRangeMin =  -5000;
  int globalRangeMax =  10000000;
  boost::progress_display show_progress(numProcsArray.size() * rankArray.size());

  BOOST_FOREACH(int numProcs, numProcsArray)
  {
    BOOST_FOREACH(int rank, rankArray)
    {
      ++show_progress;
      if (rank >= numProcs) continue;

      resetContext();

      MPI_Comm testComm = (MPI_Comm)(void*)123;
      std::vector<int> coords(1, rank);
      context.commWorld = (MPI_Comm)(void*)574;
      context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, numProcs));
      context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, rank));
      context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
      context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));

      schnek::Range<int, 1> globalRange = schnek::Range<int, 1>(schnek::Array<int,1>(globalRangeMin), schnek::Array<int,1>(globalRangeMax));
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
      BOOST_CHECK_EQUAL(cc_dims[0], numProcs);

      BOOST_CHECK_EQUAL(cc_periods.size(), 1);
      BOOST_CHECK_EQUAL(cc_periods[0], 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<4>(), 1);

      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank[0], testComm);

      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<0>(), testComm);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<1>(), rank);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<2>(), 1);

      BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), 0);


      BOOST_CHECK_EQUAL(decomposition.master(), false);
      BOOST_CHECK_EQUAL(decomposition.numProcs(), numProcs);
      BOOST_CHECK_EQUAL(decomposition.getUniqueId(), rank);
      schnek::MpiCartesianDomainDecomposition<1>::ProcRanges ranges = decomposition.getProcRanges();

      BOOST_CHECK_EQUAL(ranges[0].getLo(0), 0);
      BOOST_CHECK_EQUAL(ranges[0].getHi(0), numProcs-1);

      for (int r=0; r<numProcs; ++r)
      {
        int lo = globalRangeMin + ((globalRangeMax - globalRangeMin + 1)*long(r))/numProcs;
        int hi = globalRangeMin + ((globalRangeMax - globalRangeMin + 1)*long(r + 1))/numProcs - 1;

        schnek::Range<int, 1> expectedRange = schnek::Range<int, 1>(schnek::Array<int,1>(lo), schnek::Array<int,1>(hi));
        SCHNEK_CHECK_EQUAL(ranges[0](r), expectedRange);
      }
    }
  }
}

BOOST_FIXTURE_TEST_CASE( single_process_1d_global, MpiCartesianDomainDecompositionTestFixture )
{
  MPI_Comm testComm = (MPI_Comm)(void*)123;
  std::vector<int> coords(1, 0);
  int dims[2] = {0,127};
  context.commWorld = (MPI_Comm)(void*)574;
  context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, 1));
  context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, 0));
  context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
  context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));
  context.ret_MPI_Bcast.push_back(boost::tuple<int, void*, size_t>(MPI_SUCCESS, dims, 2));

  schnek::Range<int, 1> globalRange(schnek::Array<int,1>(0), schnek::Array<int,1>(99));
  schnek::Range<double, 1> globalDomain(schnek::Array<double,1>(0), schnek::Array<double,1>(12.5));

  schnek::Grid<double, 1> weights(schnek::Array<int,1>(0), schnek::Array<int,1>(49));
  for (int i=0; i<=50; ++i)
  {
    double x = M_PI * i / 50.;
    weights(i) = sin(x);
  }

  schnek::MpiCartesianDomainDecomposition<1> decomposition(context);

  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);
  decomposition.setGlobalWeights(weights);

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

  BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), 1);
  BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<0>(), 2);
  BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<1>(), MPI_INT);
  BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<2>(), 0);
  BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<3>(), testComm);

  BOOST_CHECK_EQUAL(decomposition.master(), true);
  BOOST_CHECK_EQUAL(decomposition.numProcs(), 1);
  BOOST_CHECK_EQUAL(decomposition.getUniqueId(), 0);
  schnek::MpiCartesianDomainDecomposition<1>::ProcRanges ranges = decomposition.getProcRanges();

  BOOST_CHECK_EQUAL(ranges[0].getLo(0), 0);
  BOOST_CHECK_EQUAL(ranges[0].getHi(0), 0);

  schnek::Range<int, 1> expectedRange(schnek::Array<int,1>(0), schnek::Array<int,1>(99));
  SCHNEK_CHECK_EQUAL(ranges[0](0), expectedRange);
}

BOOST_FIXTURE_TEST_CASE( multi_process_1d_global_master, MpiCartesianDomainDecompositionTestFixture )
{
  std::vector<int> numProcsArray;
  numProcsArray += 2, 5, 7, 127, 128, 129, 1023;

  int globalRangeMin =  -4000;
  int globalRangeMax =  3999;


  BOOST_FOREACH(int numProcs, numProcsArray)
  {
    for (int resolution = 1; resolution <= 16; resolution *= 2)
    {
      resetContext();

      int weightMax = (globalRangeMax - globalRangeMin + 1) / resolution - 1;

      MPI_Comm testComm = (MPI_Comm)(void*)123;
      std::vector<int> coords(1, 0);
      int dims[2] = {0,127};
      context.commWorld = (MPI_Comm)(void*)574;
      context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, numProcs));
      context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, 0));
      context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
      context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));
      context.ret_MPI_Bcast.push_back(boost::tuple<int, void*, size_t>(MPI_SUCCESS, dims, 2));

      schnek::Range<int, 1> globalRange = schnek::Range<int, 1>(schnek::Array<int,1>(globalRangeMin), schnek::Array<int,1>(globalRangeMax));
      schnek::Range<double, 1> globalDomain = schnek::Range<double, 1>(schnek::Array<double,1>(0), schnek::Array<double,1>(12.5));

      schnek::Grid<double, 1> weights(schnek::Array<int,1>(0), schnek::Array<int,1>(weightMax));
      for (int i=0; i<=weightMax; ++i)
      {
        double x = M_PI * i / double(weightMax+1);
        weights(i) = sin(x);
      }

      schnek::MpiCartesianDomainDecomposition<1> decomposition(context);

      decomposition.setGlobalRange(globalRange);
      decomposition.setGlobalDomain(globalDomain);
      decomposition.setGlobalWeights(weights);

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
      BOOST_CHECK_EQUAL(cc_dims[0], numProcs);

      BOOST_CHECK_EQUAL(cc_periods.size(), 1);
      BOOST_CHECK_EQUAL(cc_periods[0], 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<4>(), 1);

      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank[0], testComm);

      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<0>(), testComm);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<1>(), 0);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<2>(), 1);

      BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<0>(), 2*numProcs);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<1>(), MPI_INT);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<2>(), 0);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<3>(), testComm);

      BOOST_CHECK_EQUAL(decomposition.master(), true);
      BOOST_CHECK_EQUAL(decomposition.numProcs(), numProcs);
      BOOST_CHECK_EQUAL(decomposition.getUniqueId(), 0);
      schnek::MpiCartesianDomainDecomposition<1>::ProcRanges ranges = decomposition.getProcRanges();

      BOOST_CHECK_EQUAL(ranges[0].getLo(0), 0);
      BOOST_CHECK_EQUAL(ranges[0].getHi(0), numProcs-1);

      for (int r=0; r<numProcs; ++r)
      {
        double xmin =  2.0*double(r)/double(numProcs) - 1.0;
        double xmax =  2.0*double(r + 1)/double(numProcs) - 1.0;

        double ymin = 1.0 - acos(xmin)/M_PI + 1e-10;
        double ymax = 1.0 - acos(xmax)/M_PI + 1e-10;

        int wlo = (weightMax + 1)*ymin;
        int whi = (weightMax + 1)*ymax;

        int lo = globalRangeMin + resolution*wlo;
        int hi = globalRangeMin + resolution*whi - 1;

        schnek::Range<int, 1> expectedRangeA = schnek::Range<int, 1>(schnek::Array<int,1>(lo), schnek::Array<int,1>(hi));
        schnek::Range<int, 1> expectedRangeB = schnek::Range<int, 1>(schnek::Array<int,1>(lo + resolution), schnek::Array<int,1>(hi + resolution));
        schnek::Range<int, 1> foundRange = ranges[0](r);
        bool check = ranges[0](r) == expectedRangeA;
        if (lo + resolution < globalRangeMax)
        {
          check = (foundRange.getLo()[0] == expectedRangeA.getLo()[0] || foundRange.getLo()[0] == expectedRangeB.getLo()[0])
                  && (foundRange.getHi()[0] == expectedRangeA.getHi()[0] || foundRange.getHi()[0] == expectedRangeB.getHi()[0]);
        }

        if (!check)
        {
          std::stringstream out;
          out << ranges[0](r) << " not equal to either" << expectedRangeA << " or " << expectedRangeB;
          BOOST_TEST(check, out.str());
        }
      }
    }
  }
}


BOOST_AUTO_TEST_SUITE_END()
