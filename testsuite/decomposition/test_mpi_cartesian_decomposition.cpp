/*
 *  test_mpi_cartesian_decomposition.cpp
 *
 *  Created on: 8 Oct 2020
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 */

#include "mpi_test_context.hpp"
#include "../utility.hpp"
#include <grid/range.hpp>
#include <util/factor.hpp>

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

// ==========================================================================
// 1 dimensional
// ==========================================================================

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

      for (int i=0; i<numProcs; ++i)
      {
        double xmin =  2.0*double(i)/double(numProcs) - 1.0;
        double xmax =  2.0*double(i + 1)/double(numProcs) - 1.0;

        double wxmin = 1.0 - acos(xmin)/M_PI + 1e-10;
        double wxmax = 1.0 - acos(xmax)/M_PI + 1e-10;

        int wlo = (weightMax + 1)*wxmin;
        int whi = (weightMax + 1)*wxmax;

        int lo = globalRangeMin + resolution*wlo;
        int hi = globalRangeMin + resolution*whi - 1;

        // checking multiple possible ranges because we are allowing for rounding errors here
        schnek::Range<int, 1> expectedRangeA = schnek::Range<int, 1>(schnek::Array<int,1>(lo), schnek::Array<int,1>(hi));
        schnek::Range<int, 1> expectedRangeB = schnek::Range<int, 1>(schnek::Array<int,1>(lo + resolution), schnek::Array<int,1>(hi + resolution));
        schnek::Range<int, 1> foundRange = ranges[0](i);
        bool check = ranges[0](i) == expectedRangeA;

        if (lo + resolution < globalRangeMax)
        {
          check = (foundRange.getLo()[0] == expectedRangeA.getLo()[0] || foundRange.getLo()[0] == expectedRangeB.getLo()[0])
                  && (foundRange.getHi()[0] == expectedRangeA.getHi()[0] || foundRange.getHi()[0] == expectedRangeB.getHi()[0]);
        }

        if (!check)
        {
          std::stringstream out;
          out << ranges[0](i) << " not equal to either" << expectedRangeA << " or " << expectedRangeB;
          BOOST_TEST(check, out.str());
        }
      }
    }
  }
}


BOOST_FIXTURE_TEST_CASE( multi_process_1d_global_child, MpiCartesianDomainDecompositionTestFixture )
{
  std::vector<int> numProcsArray;
  numProcsArray += 2, 5, 7, 127, 128, 129, 1023;
  std::vector<int> rankArray;
  rankArray += 1, 2, 4, 120, 126, 500, 1022, 3999, 24999, 127999, 879483;

  int globalRangeMin =  -4000;
  int globalRangeMax =  3999;

  BOOST_FOREACH(int numProcs, numProcsArray)
  {
    BOOST_FOREACH(int rank, rankArray)
    {
      if (rank >= numProcs) continue;
      resetContext();

      int weightMax = (globalRangeMax - globalRangeMin + 1) / 4 - 1;

      MPI_Comm testComm = (MPI_Comm)(void*)123;
      std::vector<int> coords(1, rank);

      int *dims = new int[2*numProcs];
      for (int i=0; i<numProcs; ++i)
      {
        dims[2*i] = 10*i;
        dims[2*i + 1] = 10*(i + 1) - 1;
      }

      context.commWorld = (MPI_Comm)(void*)574;
      context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, numProcs));
      context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, rank));
      context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
      context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));
      context.ret_MPI_Bcast.push_back(boost::tuple<int, void*, size_t>(MPI_SUCCESS, dims, 2*numProcs*sizeof(int)));

      schnek::Range<int, 1> globalRange = schnek::Range<int, 1>(schnek::Array<int,1>(globalRangeMin), schnek::Array<int,1>(globalRangeMax));
      schnek::Range<double, 1> globalDomain = schnek::Range<double, 1>(schnek::Array<double,1>(0), schnek::Array<double,1>(12.5));

      schnek::Grid<double, 1> weights(schnek::Array<int,1>(0), schnek::Array<int,1>(weightMax));
      weights = 1.0;

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
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<1>(), rank);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<2>(), 1);

      BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<0>(), 2*numProcs);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<1>(), MPI_INT);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<2>(), 0);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<3>(), testComm);

      BOOST_CHECK_EQUAL(decomposition.master(), false);
      BOOST_CHECK_EQUAL(decomposition.numProcs(), numProcs);
      BOOST_CHECK_EQUAL(decomposition.getUniqueId(), rank);
      schnek::MpiCartesianDomainDecomposition<1>::ProcRanges ranges = decomposition.getProcRanges();

      BOOST_CHECK_EQUAL(ranges[0].getLo(0), 0);
      BOOST_CHECK_EQUAL(ranges[0].getHi(0), numProcs-1);

      for (int r=0; r<numProcs; ++r)
      {
        BOOST_CHECK_EQUAL(ranges[0](r).getLo()[0], 10*r);
        BOOST_CHECK_EQUAL(ranges[0](r).getHi()[0], 10*(r+1)-1);
      }

      delete[] dims;
    }
  }
}

// ==========================================================================
// 2 dimensional
// ==========================================================================


BOOST_FIXTURE_TEST_CASE( single_process_2d, MpiCartesianDomainDecompositionTestFixture )
{
  MPI_Comm testComm = (MPI_Comm)(void*)123;
  std::vector<int> coords(2, 0);
  context.commWorld = (MPI_Comm)(void*)574;
  context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, 1));
  context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, 0));
  context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
  context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));

  schnek::Range<int, 2> globalRange(schnek::Array<int,2>(0, -10), schnek::Array<int,2>(100, 200));
  schnek::Range<double, 2> globalDomain(schnek::Array<double,2>(0, 0), schnek::Array<double,2>(12.5, 42.2));

  schnek::MpiCartesianDomainDecomposition<2> decomposition(context);
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  decomposition.init();

  // Checking calls
  BOOST_CHECK_EQUAL(context.args_MPI_Comm_size.size(), 1);
  BOOST_CHECK_EQUAL(context.args_MPI_Comm_size[0], context.commWorld);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create.size(), 1);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<0>(), context.commWorld);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<1>(), 2);
  const std::vector<int> cc_dims = context.args_MPI_Cart_create[0].get<2>();
  const std::vector<int> cc_periods = context.args_MPI_Cart_create[0].get<3>();

  BOOST_CHECK_EQUAL(cc_dims.size(), 2);
  BOOST_CHECK_EQUAL(cc_dims[0], 1);
  BOOST_CHECK_EQUAL(cc_dims[1], 1);

  BOOST_CHECK_EQUAL(cc_periods.size(), 2);
  BOOST_CHECK_EQUAL(cc_periods[0], 1);
  BOOST_CHECK_EQUAL(cc_periods[1], 1);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<4>(), 1);

  BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank.size(), 1);
  BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank[0], testComm);

  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords.size(), 1);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<0>(), testComm);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<1>(), 0);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<2>(), 2);

  BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), 0);

  BOOST_CHECK_EQUAL(decomposition.master(), true);
  BOOST_CHECK_EQUAL(decomposition.numProcs(), 1);
  BOOST_CHECK_EQUAL(decomposition.getUniqueId(), 0);
  schnek::MpiCartesianDomainDecomposition<2>::ProcRanges ranges = decomposition.getProcRanges();

  BOOST_CHECK_EQUAL(ranges[0].getLo(0), 0);
  BOOST_CHECK_EQUAL(ranges[0].getHi(0), 0);
  BOOST_CHECK_EQUAL(ranges[1].getLo(0), 0);
  BOOST_CHECK_EQUAL(ranges[1].getHi(0), 0);

  schnek::Range<int, 1> expectedRange0(schnek::Array<int,1>(0), schnek::Array<int,1>(100));
  schnek::Range<int, 1> expectedRange1(schnek::Array<int,1>(-10), schnek::Array<int,1>(200));
  SCHNEK_CHECK_EQUAL(ranges[0](0), expectedRange0);
  SCHNEK_CHECK_EQUAL(ranges[1](0), expectedRange1);
}

BOOST_FIXTURE_TEST_CASE( multi_process_2d, MpiCartesianDomainDecompositionTestFixture )
{
  std::vector<int> numProcsArray;
  numProcsArray += 2, 5, 7, 127, 128, 129, 1023, 4000, 25000, 128000, 879484;
  std::vector<int> rankArray;
  rankArray += 1, 2, 4, 120, 126, 500, 1022, 3999, 24999, 127999, 879483;

  std::vector<int> globalRangeMin(2, -5000);
  std::vector<int> globalRangeMax(2,  5000);

  boost::progress_display show_progress(numProcsArray.size() * rankArray.size());

  BOOST_FOREACH(int numProcs, numProcsArray)
  {
    BOOST_FOREACH(int rank, rankArray)
    {
      ++show_progress;
      if (rank >= numProcs) continue;
      std::vector<int> factors;
      std::vector<int> weights(2,1);
      schnek::equalFactors(numProcs, 2, factors, weights);

      resetContext();

      MPI_Comm testComm = (MPI_Comm)(void*)123;
      std::vector<int> coords(2);
      coords[0] = rank / factors[1];
      coords[1] = rank % factors[1];

      context.commWorld = (MPI_Comm)(void*)574;
      context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, numProcs));
      context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, rank));
      context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
      context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));

      schnek::Range<int, 2> globalRange
        = schnek::Range<int, 2>(
                schnek::Array<int,2>(globalRangeMin[0], globalRangeMin[1]),
                schnek::Array<int,2>(globalRangeMax[0], globalRangeMax[1]));
      schnek::Range<double, 2> globalDomain(schnek::Array<double,2>(0, -1.5), schnek::Array<double,2>(12.5, 42.2));

      schnek::MpiCartesianDomainDecomposition<2> decomposition(context);
      decomposition.setGlobalRange(globalRange);
      decomposition.setGlobalDomain(globalDomain);

      decomposition.init();

      // Checking calls
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size[0], context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<0>(), context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<1>(), 2);
      const std::vector<int> cc_dims = context.args_MPI_Cart_create[0].get<2>();
      const std::vector<int> cc_periods = context.args_MPI_Cart_create[0].get<3>();

      BOOST_CHECK_EQUAL(cc_dims.size(), 2);
      BOOST_CHECK_EQUAL(cc_dims[0], factors[0]);
      BOOST_CHECK_EQUAL(cc_dims[1], factors[1]);

      BOOST_CHECK_EQUAL(cc_periods.size(), 2);
      BOOST_CHECK_EQUAL(cc_periods[0], 1);
      BOOST_CHECK_EQUAL(cc_periods[1], 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<4>(), 1);

      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank[0], testComm);

      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<0>(), testComm);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<1>(), rank);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<2>(), 2);

      BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), 0);


      BOOST_CHECK_EQUAL(decomposition.master(), false);
      BOOST_CHECK_EQUAL(decomposition.numProcs(), numProcs);
      BOOST_CHECK_EQUAL(decomposition.getUniqueId(), rank);
      schnek::MpiCartesianDomainDecomposition<2>::ProcRanges ranges = decomposition.getProcRanges();

      for (int d=0; d<2; ++d)
      {
        BOOST_CHECK_EQUAL(ranges[d].getLo(0), 0);
        BOOST_CHECK_EQUAL(ranges[d].getHi(0), factors[d]-1);

        for (int r=0; r<factors[d]; ++r)
        {
          int lo = globalRangeMin[d] + ((globalRangeMax[d] - globalRangeMin[d] + 1)*long(r))/factors[d];
          int hi = globalRangeMin[d] + ((globalRangeMax[d] - globalRangeMin[d] + 1)*long(r + 1))/factors[d] - 1;

          schnek::Range<int, 1> expectedRange = schnek::Range<int, 1>(schnek::Array<int,1>(lo), schnek::Array<int,1>(hi));
          SCHNEK_CHECK_EQUAL(ranges[d](r), expectedRange);
        }
      }
    }
  }
}

BOOST_FIXTURE_TEST_CASE( multi_process_2d_global_master, MpiCartesianDomainDecompositionTestFixture )
{
  std::vector<int> numProcsArray;
  numProcsArray += 2, 5, 7, 127, 128, 129, 1023, 4000, 25000, 128000, 1093950;

  int globalRangeMin =  -4000;
  int globalRangeMax =  3999;
  boost::progress_display show_progress(numProcsArray.size() * 5);

  BOOST_FOREACH(int numProcs, numProcsArray)
  {
    for (int resolution = 1; resolution <= 16; resolution *= 2)
    {
      ++show_progress;
      std::vector<int> factors;
      std::vector<int> box(2,1);
      schnek::equalFactors(numProcs, 2, factors, box);

      resetContext();

      int weightMax = (globalRangeMax - globalRangeMin + 1) / resolution - 1;

      MPI_Comm testComm = (MPI_Comm)(void*)123;
      std::vector<int> coords(2, 0);
      int dims[4] = {0, 127, 0, 127};
      context.commWorld = (MPI_Comm)(void*)574;
      context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, numProcs));
      context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, 0));
      context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
      context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));
      context.ret_MPI_Bcast.push_back(boost::tuple<int, void*, size_t>(MPI_SUCCESS, dims, 4));

      schnek::Range<int, 2> globalRange
        = schnek::Range<int, 2>(
                schnek::Array<int,2>(globalRangeMin, globalRangeMin),
                schnek::Array<int,2>(globalRangeMax, globalRangeMax));
      schnek::Range<double, 2> globalDomain(schnek::Array<double,2>(0, -1.5), schnek::Array<double,2>(12.5, 42.2));

      schnek::Grid<double, 2> weights(schnek::Array<int,2>(0, 0), schnek::Array<int,2>(weightMax, weightMax));
      for (int i=0; i<=weightMax; ++i)
      {
        double x = M_PI * i / double(weightMax+1);
        for (int j=0; j<=weightMax; ++j)
        {
          double y = M_PI * j / double(weightMax+1);
          weights(i, j) = sin(x)*sin(y);
        }
      }

      schnek::MpiCartesianDomainDecomposition<2> decomposition(context);

      decomposition.setGlobalRange(globalRange);
      decomposition.setGlobalDomain(globalDomain);
      decomposition.setGlobalWeights(weights);

      decomposition.init();

      // Checking calls
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size[0], context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<0>(), context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<1>(), 2);
      const std::vector<int> cc_dims = context.args_MPI_Cart_create[0].get<2>();
      const std::vector<int> cc_periods = context.args_MPI_Cart_create[0].get<3>();

      BOOST_CHECK_EQUAL(cc_dims.size(), 2);
      BOOST_CHECK_EQUAL(cc_dims[0], factors[0]);
      BOOST_CHECK_EQUAL(cc_dims[1], factors[1]);

      BOOST_CHECK_EQUAL(cc_periods.size(), 2);
      BOOST_CHECK_EQUAL(cc_periods[0], 1);
      BOOST_CHECK_EQUAL(cc_periods[1], 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<4>(), 1);

      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank[0], testComm);

      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<0>(), testComm);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<1>(), 0);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<2>(), 2);

      BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), 2);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<0>(), 2*factors[0]);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<1>(), MPI_INT);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<2>(), 0);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<3>(), testComm);

      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[1].get<0>(), 2*factors[1]);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[1].get<1>(), MPI_INT);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[1].get<2>(), 0);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[1].get<3>(), testComm);

      BOOST_CHECK_EQUAL(decomposition.master(), true);
      BOOST_CHECK_EQUAL(decomposition.numProcs(), numProcs);
      BOOST_CHECK_EQUAL(decomposition.getUniqueId(), 0);
      schnek::MpiCartesianDomainDecomposition<2>::ProcRanges ranges = decomposition.getProcRanges();

      BOOST_CHECK_EQUAL(ranges[0].getLo(0), 0);
      BOOST_CHECK_EQUAL(ranges[1].getLo(0), 0);
      BOOST_CHECK_EQUAL(ranges[0].getHi(0), factors[0]-1);
      BOOST_CHECK_EQUAL(ranges[1].getHi(0), factors[1]-1);

      for (int r=0; r<2; ++r)
      {
        for (int i=0; i<factors[r]; ++i)
        {
          double xmin =  2.0*double(i)/double(factors[r]) - 1.0;
          double xmax =  2.0*double(i + 1)/double(factors[r]) - 1.0;

          double wxmin = 1.0 - acos(xmin)/M_PI + 1e-10;
          double wxmax = 1.0 - acos(xmax)/M_PI + 1e-10;

          int wlo = (weightMax + 1)*wxmin;
          int whi = (weightMax + 1)*wxmax;

          int lo = globalRangeMin + resolution*wlo;
          int hi = globalRangeMin + resolution*whi - 1;

          // checking multiple possible ranges because we are allowing for rounding errors here
          schnek::Range<int, 1> expectedRangeA = schnek::Range<int, 1>(schnek::Array<int,1>(lo), schnek::Array<int,1>(hi));
          schnek::Range<int, 1> expectedRangeB = schnek::Range<int, 1>(schnek::Array<int,1>(lo + resolution), schnek::Array<int,1>(hi + resolution));
          schnek::Range<int, 1> foundRange = ranges[r](i);
          bool check = foundRange == expectedRangeA;

          if (lo + resolution < globalRangeMax)
          {
            check = (foundRange.getLo()[0] == expectedRangeA.getLo()[0] || foundRange.getLo()[0] == expectedRangeB.getLo()[0])
                    && (foundRange.getHi()[0] == expectedRangeA.getHi()[0] || foundRange.getHi()[0] == expectedRangeB.getHi()[0]);
          }

          if (!check)
          {
            std::stringstream out;
            out << foundRange << " not equal to either" << expectedRangeA << " or " << expectedRangeB << " for dim = " << r;
            BOOST_TEST(check, out.str());
          }
        }
      }
    }
  }
}

BOOST_FIXTURE_TEST_CASE( multi_process_2d_global_child, MpiCartesianDomainDecompositionTestFixture )
{
  std::vector<int> numProcsArray;
  numProcsArray += 2, 5, 7, 127, 128, 129, 1023, 4000, 25000, 128000, 1093950;
  std::vector<int> rankArray;
  rankArray += 1, 2, 4, 120, 126, 500, 1022, 3999, 24999, 127999, 879483;

  int globalRangeMin =  -4000;
  int globalRangeMax =  3999;
  boost::progress_display show_progress(numProcsArray.size() * rankArray.size());

  BOOST_FOREACH(int numProcs, numProcsArray)
  {
    BOOST_FOREACH(int rank, rankArray)
    {
      ++show_progress;
      if (rank >= numProcs) continue;

      std::vector<int> factors;
      std::vector<int> box(2,1);
      schnek::equalFactors(numProcs, 2, factors, box);

      resetContext();

      int weightMax = (globalRangeMax - globalRangeMin + 1) / 4 - 1;

      MPI_Comm testComm = (MPI_Comm)(void*)123;
      std::vector<int> coords(2);
      coords[0] = rank / factors[1];
      coords[1] = rank % factors[1];

      int *dims0 = new int[2*factors[0]];
      int *dims1 = new int[2*factors[1]];

      for (int i=0; i<factors[0]; ++i)
      {
        dims0[2*i] = 10*i;
        dims0[2*i + 1] = 10*(i + 1) - 1;
      }

      for (int i=0; i<factors[1]; ++i)
      {
        dims1[2*i] = 13*i;
        dims1[2*i + 1] = 13*(i + 1) - 1;
      }

      context.commWorld = (MPI_Comm)(void*)574;
      context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, numProcs));
      context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, rank));
      context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
      context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));
      context.ret_MPI_Bcast.push_back(boost::tuple<int, void*, size_t>(MPI_SUCCESS, dims0, 2*factors[0]*sizeof(int)));
      context.ret_MPI_Bcast.push_back(boost::tuple<int, void*, size_t>(MPI_SUCCESS, dims1, 2*factors[1]*sizeof(int)));

      schnek::Range<int, 2> globalRange
        = schnek::Range<int, 2>(
                schnek::Array<int,2>(globalRangeMin, globalRangeMin),
                schnek::Array<int,2>(globalRangeMax, globalRangeMax));
      schnek::Range<double, 2> globalDomain(schnek::Array<double,2>(0, -1.5), schnek::Array<double,2>(12.5, 42.2));

      schnek::Grid<double, 2> weights(schnek::Array<int,2>(0, 0), schnek::Array<int,2>(weightMax, weightMax));
      weights = 1.0;

      schnek::MpiCartesianDomainDecomposition<2> decomposition(context);

      decomposition.setGlobalRange(globalRange);
      decomposition.setGlobalDomain(globalDomain);
      decomposition.setGlobalWeights(weights);

      decomposition.init();

      // Checking calls
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size[0], context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<0>(), context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<1>(), 2);
      const std::vector<int> cc_dims = context.args_MPI_Cart_create[0].get<2>();
      const std::vector<int> cc_periods = context.args_MPI_Cart_create[0].get<3>();

      BOOST_CHECK_EQUAL(cc_dims.size(), 2);
      BOOST_CHECK_EQUAL(cc_dims[0], factors[0]);
      BOOST_CHECK_EQUAL(cc_dims[1], factors[1]);

      BOOST_CHECK_EQUAL(cc_periods.size(), 2);
      BOOST_CHECK_EQUAL(cc_periods[0], 1);
      BOOST_CHECK_EQUAL(cc_periods[1], 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<4>(), 1);

      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank[0], testComm);

      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<0>(), testComm);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<1>(), rank);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<2>(), 2);

      BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), 2);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<0>(), 2*factors[0]);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<1>(), MPI_INT);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<2>(), 0);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<3>(), testComm);

      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[1].get<0>(), 2*factors[1]);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[1].get<1>(), MPI_INT);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[1].get<2>(), 0);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[1].get<3>(), testComm);

      BOOST_CHECK_EQUAL(decomposition.master(), false);
      BOOST_CHECK_EQUAL(decomposition.numProcs(), numProcs);
      BOOST_CHECK_EQUAL(decomposition.getUniqueId(), rank);
      schnek::MpiCartesianDomainDecomposition<2>::ProcRanges ranges = decomposition.getProcRanges();

      BOOST_CHECK_EQUAL(ranges[0].getLo(0), 0);
      BOOST_CHECK_EQUAL(ranges[0].getHi(0), factors[0]-1);

      for (int r=0; r<factors[0]; ++r)
      {
        BOOST_CHECK_EQUAL(ranges[0](r).getLo()[0], 10*r);
        BOOST_CHECK_EQUAL(ranges[0](r).getHi()[0], 10*(r+1)-1);
      }

      BOOST_CHECK_EQUAL(ranges[1].getLo(0), 0);
      BOOST_CHECK_EQUAL(ranges[1].getHi(0), factors[1]-1);

      for (int r=0; r<factors[1]; ++r)
      {
        BOOST_CHECK_EQUAL(ranges[1](r).getLo()[0], 13*r);
        BOOST_CHECK_EQUAL(ranges[1](r).getHi()[0], 13*(r+1)-1);
      }

      delete[] dims0;
      delete[] dims1;
    }
  }
}

// ==========================================================================
// 3 dimensional
// ==========================================================================


BOOST_FIXTURE_TEST_CASE( single_process_3d, MpiCartesianDomainDecompositionTestFixture )
{
  MPI_Comm testComm = (MPI_Comm)(void*)123;
  std::vector<int> coords(3, 0);
  context.commWorld = (MPI_Comm)(void*)574;
  context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, 1));
  context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, 0));
  context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
  context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));

  schnek::Range<int, 3> globalRange(schnek::Array<int,3>(0, -10, -123), schnek::Array<int,3>(100, 200, -2));
  schnek::Range<double, 3> globalDomain(schnek::Array<double,3>(0, 0, -5.2), schnek::Array<double,3>(12.5, 42.2, 10.1));

  schnek::MpiCartesianDomainDecomposition<3> decomposition(context);
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  decomposition.init();

  // Checking calls
  BOOST_CHECK_EQUAL(context.args_MPI_Comm_size.size(), 1);
  BOOST_CHECK_EQUAL(context.args_MPI_Comm_size[0], context.commWorld);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create.size(), 1);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<0>(), context.commWorld);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<1>(), 3);
  const std::vector<int> cc_dims = context.args_MPI_Cart_create[0].get<2>();
  const std::vector<int> cc_periods = context.args_MPI_Cart_create[0].get<3>();

  BOOST_CHECK_EQUAL(cc_dims.size(), 3);
  BOOST_CHECK_EQUAL(cc_dims[0], 1);
  BOOST_CHECK_EQUAL(cc_dims[1], 1);
  BOOST_CHECK_EQUAL(cc_dims[2], 1);

  BOOST_CHECK_EQUAL(cc_periods.size(), 3);
  BOOST_CHECK_EQUAL(cc_periods[0], 1);
  BOOST_CHECK_EQUAL(cc_periods[1], 1);
  BOOST_CHECK_EQUAL(cc_periods[2], 1);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<4>(), 1);

  BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank.size(), 1);
  BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank[0], testComm);

  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords.size(), 1);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<0>(), testComm);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<1>(), 0);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<2>(), 3);

  BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), 0);

  BOOST_CHECK_EQUAL(decomposition.master(), true);
  BOOST_CHECK_EQUAL(decomposition.numProcs(), 1);
  BOOST_CHECK_EQUAL(decomposition.getUniqueId(), 0);
  schnek::MpiCartesianDomainDecomposition<3>::ProcRanges ranges = decomposition.getProcRanges();

  BOOST_CHECK_EQUAL(ranges[0].getLo(0), 0);
  BOOST_CHECK_EQUAL(ranges[0].getHi(0), 0);
  BOOST_CHECK_EQUAL(ranges[1].getLo(0), 0);
  BOOST_CHECK_EQUAL(ranges[1].getHi(0), 0);
  BOOST_CHECK_EQUAL(ranges[2].getLo(0), 0);
  BOOST_CHECK_EQUAL(ranges[2].getHi(0), 0);

  schnek::Range<int, 1> expectedRange0(schnek::Array<int,1>(0), schnek::Array<int,1>(100));
  schnek::Range<int, 1> expectedRange1(schnek::Array<int,1>(-10), schnek::Array<int,1>(200));
  schnek::Range<int, 1> expectedRange2(schnek::Array<int,1>(-123), schnek::Array<int,1>(-2));
  SCHNEK_CHECK_EQUAL(ranges[0](0), expectedRange0);
  SCHNEK_CHECK_EQUAL(ranges[1](0), expectedRange1);
  SCHNEK_CHECK_EQUAL(ranges[2](0), expectedRange2);
}

BOOST_FIXTURE_TEST_CASE( multi_process_3d, MpiCartesianDomainDecompositionTestFixture )
{
  std::vector<int> numProcsArray;
  numProcsArray += 2, 5, 7, 127, 128, 129, 1023, 4000, 25000, 128000, 879484;
  std::vector<int> rankArray;
  rankArray += 1, 2, 4, 120, 126, 500, 1022, 3999, 24999, 127999, 879483;

  std::vector<int> globalRangeMin(3, -5000);
  std::vector<int> globalRangeMax(3,  5000);

  boost::progress_display show_progress(numProcsArray.size() * rankArray.size());

  BOOST_FOREACH(int numProcs, numProcsArray)
  {
    BOOST_FOREACH(int rank, rankArray)
    {
      ++show_progress;
      if (rank >= numProcs) continue;
      std::vector<int> factors;
      std::vector<int> weights(3,1);
      schnek::equalFactors(numProcs, 3, factors, weights);

      resetContext();

      MPI_Comm testComm = (MPI_Comm)(void*)123;
      std::vector<int> coords(3);
      coords[0] = rank / (factors[1]*factors[2]);
      coords[1] = (rank / factors[2] ) % factors[1];
      coords[2] = rank % factors[2];

      context.commWorld = (MPI_Comm)(void*)574;
      context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, numProcs));
      context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, rank));
      context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
      context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));

      schnek::Range<int, 3> globalRange
        = schnek::Range<int, 3>(
                schnek::Array<int,3>(globalRangeMin[0], globalRangeMin[1], globalRangeMin[2]),
                schnek::Array<int,3>(globalRangeMax[0], globalRangeMax[1], globalRangeMax[2]));
      schnek::Range<double, 3> globalDomain(schnek::Array<double,3>(0, 0, -5.2), schnek::Array<double,3>(12.5, 42.2, 10.1));

      schnek::MpiCartesianDomainDecomposition<3> decomposition(context);
      decomposition.setGlobalRange(globalRange);
      decomposition.setGlobalDomain(globalDomain);

      decomposition.init();

      // Checking calls
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size[0], context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<0>(), context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<1>(), 3);
      const std::vector<int> cc_dims = context.args_MPI_Cart_create[0].get<2>();
      const std::vector<int> cc_periods = context.args_MPI_Cart_create[0].get<3>();

      BOOST_CHECK_EQUAL(cc_dims.size(), 3);
      BOOST_CHECK_EQUAL(cc_dims[0], factors[0]);
      BOOST_CHECK_EQUAL(cc_dims[1], factors[1]);
      BOOST_CHECK_EQUAL(cc_dims[2], factors[2]);

      BOOST_CHECK_EQUAL(cc_periods.size(), 3);
      BOOST_CHECK_EQUAL(cc_periods[0], 1);
      BOOST_CHECK_EQUAL(cc_periods[1], 1);
      BOOST_CHECK_EQUAL(cc_periods[2], 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<4>(), 1);

      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank[0], testComm);

      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<0>(), testComm);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<1>(), rank);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<2>(), 3);

      BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), 0);

      BOOST_CHECK_EQUAL(decomposition.master(), false);
      BOOST_CHECK_EQUAL(decomposition.numProcs(), numProcs);
      BOOST_CHECK_EQUAL(decomposition.getUniqueId(), rank);
      schnek::MpiCartesianDomainDecomposition<3>::ProcRanges ranges = decomposition.getProcRanges();

      for (int d=0; d<3; ++d)
      {
        BOOST_CHECK_EQUAL(ranges[d].getLo(0), 0);
        BOOST_CHECK_EQUAL(ranges[d].getHi(0), factors[d]-1);

        for (int r=0; r<factors[d]; ++r)
        {
          int lo = globalRangeMin[d] + ((globalRangeMax[d] - globalRangeMin[d] + 1)*long(r))/factors[d];
          int hi = globalRangeMin[d] + ((globalRangeMax[d] - globalRangeMin[d] + 1)*long(r + 1))/factors[d] - 1;

          schnek::Range<int, 1> expectedRange = schnek::Range<int, 1>(schnek::Array<int,1>(lo), schnek::Array<int,1>(hi));
          SCHNEK_CHECK_EQUAL(ranges[d](r), expectedRange);
        }
      }
    }
  }
}

BOOST_FIXTURE_TEST_CASE( multi_process_3d_global_master, MpiCartesianDomainDecompositionTestFixture )
{
  std::vector<int> numProcsArray;
  numProcsArray += 2, 5, 7, 128, 1024, 4000, 25000, 128000, 1093950;

  int globalRangeMin =  -3200;
  int globalRangeMax =  3199;
  boost::progress_display show_progress(numProcsArray.size() * 5);

  BOOST_FOREACH(int numProcs, numProcsArray)
  {
    for (int resolution = 16; resolution <= 256; resolution *= 2)
    {
      ++show_progress;
      std::vector<int> factors;
      std::vector<int> box(3, (globalRangeMax - globalRangeMin + 1)/resolution);

      schnek::equalFactors(numProcs, 3, factors, box);

      resetContext();

      int weightMax = (globalRangeMax - globalRangeMin + 1) / resolution - 1;

      MPI_Comm testComm = (MPI_Comm)(void*)123;
      std::vector<int> coords(3, 0);
      int dims[6] = {0, 127, 0, 127, 0, 127};

      context.commWorld = (MPI_Comm)(void*)574;
      context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, numProcs));
      context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, 0));
      context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
      context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));
      context.ret_MPI_Bcast.push_back(boost::tuple<int, void*, size_t>(MPI_SUCCESS, dims, 6));

      schnek::Range<int, 3> globalRange
        = schnek::Range<int, 3>(
                schnek::Array<int,3>(globalRangeMin, globalRangeMin, globalRangeMin),
                schnek::Array<int,3>(globalRangeMax, globalRangeMax, globalRangeMax));
      schnek::Range<double, 3> globalDomain(schnek::Array<double,3>(0, 0, -5.2), schnek::Array<double,3>(12.5, 42.2, 10.1));

      schnek::Grid<double, 3> weights(schnek::Array<int,3>(0, 0, 0), schnek::Array<int,3>(weightMax, weightMax, weightMax));
      for (int i=0; i<=weightMax; ++i)
      {
        double x = M_PI * i / double(weightMax+1);
        for (int j=0; j<=weightMax; ++j)
        {
          double y = M_PI * j / double(weightMax+1);
          for (int k=0; k<=weightMax; ++k)
          {
            double z = M_PI * k / double(weightMax+1);
            weights(i, j, k) = sin(x)*sin(y)*sin(z);
          }
        }
      }

      schnek::MpiCartesianDomainDecomposition<3> decomposition(context);

      decomposition.setGlobalRange(globalRange);
      decomposition.setGlobalDomain(globalDomain);
      decomposition.setGlobalWeights(weights);

      decomposition.init();

      // Checking calls
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size[0], context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<0>(), context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<1>(), 3);
      const std::vector<int> cc_dims = context.args_MPI_Cart_create[0].get<2>();
      const std::vector<int> cc_periods = context.args_MPI_Cart_create[0].get<3>();

      BOOST_CHECK_EQUAL(cc_dims.size(), 3);
      BOOST_CHECK_EQUAL(cc_dims[0], factors[0]);
      BOOST_CHECK_EQUAL(cc_dims[1], factors[1]);
      BOOST_CHECK_EQUAL(cc_dims[2], factors[2]);

      BOOST_CHECK_EQUAL(cc_periods.size(), 3);
      BOOST_CHECK_EQUAL(cc_periods[0], 1);
      BOOST_CHECK_EQUAL(cc_periods[1], 1);
      BOOST_CHECK_EQUAL(cc_periods[2], 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<4>(), 1);

      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank[0], testComm);

      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<0>(), testComm);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<1>(), 0);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<2>(), 3);

      BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), 3);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<0>(), 2*factors[0]);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<1>(), MPI_INT);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<2>(), 0);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<3>(), testComm);

      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[1].get<0>(), 2*factors[1]);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[1].get<1>(), MPI_INT);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[1].get<2>(), 0);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[1].get<3>(), testComm);

      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[2].get<0>(), 2*factors[2]);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[2].get<1>(), MPI_INT);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[2].get<2>(), 0);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[2].get<3>(), testComm);

      BOOST_CHECK_EQUAL(decomposition.master(), true);
      BOOST_CHECK_EQUAL(decomposition.numProcs(), numProcs);
      BOOST_CHECK_EQUAL(decomposition.getUniqueId(), 0);
      schnek::MpiCartesianDomainDecomposition<3>::ProcRanges ranges = decomposition.getProcRanges();

      BOOST_CHECK_EQUAL(ranges[0].getLo(0), 0);
      BOOST_CHECK_EQUAL(ranges[1].getLo(0), 0);
      BOOST_CHECK_EQUAL(ranges[2].getLo(0), 0);
      BOOST_CHECK_EQUAL(ranges[0].getHi(0), factors[0]-1);
      BOOST_CHECK_EQUAL(ranges[1].getHi(0), factors[1]-1);
      BOOST_CHECK_EQUAL(ranges[2].getHi(0), factors[2]-1);

      for (int r=0; r<3; ++r)
      {
        for (int i=0; i<factors[r]; ++i)
        {
          double xmin =  2.0*double(i)/double(factors[r]) - 1.0;
          double xmax =  2.0*double(i + 1)/double(factors[r]) - 1.0;

          double wxmin = 1.0 - acos(xmin)/M_PI + 1e-10;
          double wxmax = 1.0 - acos(xmax)/M_PI + 1e-10;

          int wlo = (weightMax + 1)*wxmin;
          int whi = (weightMax + 1)*wxmax;

          int lo = globalRangeMin + resolution*wlo;
          int hi = globalRangeMin + resolution*whi - 1;

          // checking multiple possible ranges because we are allowing for rounding errors here
          schnek::Range<int, 1> expectedRangeA = schnek::Range<int, 1>(schnek::Array<int,1>(lo), schnek::Array<int,1>(hi));
          schnek::Range<int, 1> expectedRangeB = schnek::Range<int, 1>(schnek::Array<int,1>(lo + resolution), schnek::Array<int,1>(hi + resolution));
          schnek::Range<int, 1> foundRange = ranges[r](i);
          bool check = foundRange == expectedRangeA;

          if (lo + resolution < globalRangeMax)
          {
            check = (foundRange.getLo()[0] == expectedRangeA.getLo()[0] || foundRange.getLo()[0] == expectedRangeB.getLo()[0])
                    && (foundRange.getHi()[0] == expectedRangeA.getHi()[0] || foundRange.getHi()[0] == expectedRangeB.getHi()[0]);
          }

          if (!check)
          {
            std::stringstream out;
            out << foundRange << " not equal to either" << expectedRangeA << " or " << expectedRangeB << " for dim = " << r;
            BOOST_TEST(check, out.str());
          }
        }
      }
    }
  }
}

BOOST_FIXTURE_TEST_CASE( multi_process_3d_global_child, MpiCartesianDomainDecompositionTestFixture )
{
  std::vector<int> numProcsArray;
  numProcsArray += 2, 5, 7, 127, 128, 129, 1023, 4000, 25000, 128000, 1093950;
  std::vector<int> rankArray;
  rankArray += 1, 2, 4, 120, 126, 500, 1022, 3999, 24999, 127999, 879483;

  int globalRangeMin =  -4000;
  int globalRangeMax =  3999;
  boost::progress_display show_progress(numProcsArray.size() * rankArray.size());

  BOOST_FOREACH(int numProcs, numProcsArray)
  {
    BOOST_FOREACH(int rank, rankArray)
    {
      ++show_progress;
      if (rank >= numProcs) continue;

      std::vector<int> factors;
      std::vector<int> box(3, (globalRangeMax - globalRangeMin + 1)/16);
      schnek::equalFactors(numProcs, 3, factors, box);

      resetContext();

      int weightMax = (globalRangeMax - globalRangeMin + 1) / 16 - 1;

      MPI_Comm testComm = (MPI_Comm)(void*)123;
      std::vector<int> coords(3);
      coords[0] = rank / (factors[1]*factors[2]);
      coords[1] = (rank / factors[2] ) % factors[1];
      coords[2] = rank % factors[2];

      int *dims0 = new int[2*factors[0]];
      int *dims1 = new int[2*factors[1]];
      int *dims2 = new int[2*factors[2]];

      for (int i=0; i<factors[0]; ++i)
      {
        dims0[2*i] = 10*i;
        dims0[2*i + 1] = 10*(i + 1) - 1;
      }

      for (int i=0; i<factors[1]; ++i)
      {
        dims1[2*i] = 7*i;
        dims1[2*i + 1] = 7*(i + 1) - 1;
      }

      for (int i=0; i<factors[2]; ++i)
      {
        dims2[2*i] = 13*i;
        dims2[2*i + 1] = 13*(i + 1) - 1;
      }

      context.commWorld = (MPI_Comm)(void*)574;
      context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, numProcs));
      context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, rank));
      context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
      context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));
      context.ret_MPI_Bcast.push_back(boost::tuple<int, void*, size_t>(MPI_SUCCESS, dims0, 2*factors[0]*sizeof(int)));
      context.ret_MPI_Bcast.push_back(boost::tuple<int, void*, size_t>(MPI_SUCCESS, dims1, 2*factors[1]*sizeof(int)));
      context.ret_MPI_Bcast.push_back(boost::tuple<int, void*, size_t>(MPI_SUCCESS, dims2, 2*factors[2]*sizeof(int)));

      schnek::Range<int, 3> globalRange
        = schnek::Range<int, 3>(
                schnek::Array<int,3>(globalRangeMin, globalRangeMin, globalRangeMin),
                schnek::Array<int,3>(globalRangeMax, globalRangeMax, globalRangeMax));
      schnek::Range<double, 3> globalDomain(schnek::Array<double,3>(0, 0, -5.2), schnek::Array<double,3>(12.5, 42.2, 10.1));

      schnek::Grid<double, 3> weights(schnek::Array<int,3>(0, 0, 0), schnek::Array<int,3>(weightMax, weightMax, weightMax));
      weights = 1.0;

      schnek::MpiCartesianDomainDecomposition<3> decomposition(context);

      decomposition.setGlobalRange(globalRange);
      decomposition.setGlobalDomain(globalDomain);
      decomposition.setGlobalWeights(weights);

      decomposition.init();

      // Checking calls
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size[0], context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<0>(), context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<1>(), 3);
      const std::vector<int> cc_dims = context.args_MPI_Cart_create[0].get<2>();
      const std::vector<int> cc_periods = context.args_MPI_Cart_create[0].get<3>();

      BOOST_CHECK_EQUAL(cc_dims.size(), 3);
      BOOST_CHECK_EQUAL(cc_dims[0], factors[0]);
      BOOST_CHECK_EQUAL(cc_dims[1], factors[1]);
      BOOST_CHECK_EQUAL(cc_dims[2], factors[2]);

      BOOST_CHECK_EQUAL(cc_periods.size(), 3);
      BOOST_CHECK_EQUAL(cc_periods[0], 1);
      BOOST_CHECK_EQUAL(cc_periods[1], 1);
      BOOST_CHECK_EQUAL(cc_periods[2], 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<4>(), 1);

      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank[0], testComm);

      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords.size(), 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<0>(), testComm);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<1>(), rank);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<2>(), 3);

      BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), 3);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<0>(), 2*factors[0]);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<1>(), MPI_INT);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<2>(), 0);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[0].get<3>(), testComm);

      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[1].get<0>(), 2*factors[1]);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[1].get<1>(), MPI_INT);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[1].get<2>(), 0);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[1].get<3>(), testComm);

      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[2].get<0>(), 2*factors[2]);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[2].get<1>(), MPI_INT);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[2].get<2>(), 0);
      BOOST_CHECK_EQUAL(context.args_MPI_Bcast[2].get<3>(), testComm);

      BOOST_CHECK_EQUAL(decomposition.master(), false);
      BOOST_CHECK_EQUAL(decomposition.numProcs(), numProcs);
      BOOST_CHECK_EQUAL(decomposition.getUniqueId(), rank);
      schnek::MpiCartesianDomainDecomposition<3>::ProcRanges ranges = decomposition.getProcRanges();

      BOOST_CHECK_EQUAL(ranges[0].getLo(0), 0);
      BOOST_CHECK_EQUAL(ranges[0].getHi(0), factors[0]-1);

      for (int r=0; r<factors[0]; ++r)
      {
        BOOST_CHECK_EQUAL(ranges[0](r).getLo()[0], 10*r);
        BOOST_CHECK_EQUAL(ranges[0](r).getHi()[0], 10*(r+1)-1);
      }

      BOOST_CHECK_EQUAL(ranges[1].getLo(0), 0);
      BOOST_CHECK_EQUAL(ranges[1].getHi(0), factors[1]-1);

      for (int r=0; r<factors[1]; ++r)
      {
        BOOST_CHECK_EQUAL(ranges[1](r).getLo()[0], 7*r);
        BOOST_CHECK_EQUAL(ranges[1](r).getHi()[0], 7*(r+1)-1);
      }

      BOOST_CHECK_EQUAL(ranges[2].getLo(0), 0);
      BOOST_CHECK_EQUAL(ranges[2].getHi(0), factors[2]-1);

      for (int r=0; r<factors[2]; ++r)
      {
        BOOST_CHECK_EQUAL(ranges[2](r).getLo()[0], 13*r);
        BOOST_CHECK_EQUAL(ranges[2](r).getHi()[0], 13*(r+1)-1);
      }

      delete[] dims0;
      delete[] dims1;
      delete[] dims2;
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
