/*
 *  test_mpi_cartesian_decomposition.cpp
 *
 *  Created on: 8 Oct 2020
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 */

#include "mpi_test_context.hpp"
#include "../utility.hpp"
#include <grid/range.hpp>
#include <grid/grid.hpp>
#include <grid/field.hpp>
#include <util/factor.hpp>

#include <decomposition/mpi_cartesian_decomposition.hpp>

#include <util/array_io.hpp>

#include <boost/timer/progress_display.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/assign/std/vector.hpp>
using namespace boost::assign; // bring 'operator+=()' into scope

#include <sstream>
#include <cstddef>
#include <array>
#include <cstring>
#include <optional>

namespace {

  template<typename RangeType>
  size_t rangeVolume(const RangeType &range) {
    constexpr size_t dims = RangeType::LimitType::length;
    size_t volume = 1;
    for (size_t dim = 0; dim < dims; ++dim) {
      ptrdiff_t extent = range.getHi()[dim] - range.getLo()[dim] + 1;
      if (extent <= 0) {
        return 0;
      }
      volume *= static_cast<size_t>(extent);
    }
    return volume;
  }

  template<typename T>
  std::vector<char> toByteVector(const std::vector<T> &values) {
    std::vector<char> bytes(values.size() * sizeof(T));
    if (!values.empty()) {
      std::memcpy(bytes.data(), values.data(), bytes.size());
    }
    return bytes;
  }

  template<typename T>
  std::vector<char> toByteVectorScalar(const T &value) {
    std::vector<char> bytes(sizeof(T));
    std::memcpy(bytes.data(), &value, sizeof(T));
    return bytes;
  }

  template<typename FieldType>
  struct GhostSliceExpectation {
    using RangeType = typename FieldType::RangeType;
    RangeType range;
    std::vector<typename FieldType::value_type> values;
  };

  template<typename FieldType>
  struct GhostExchangeSetupResult {
      std::vector<boost::tuple<int, std::vector<char>>> responses;
      std::vector<GhostSliceExpectation<FieldType>> expectations;
  };

  template<typename FieldType>
  GhostExchangeSetupResult<FieldType> buildGhostExchangeSetup(FieldType &field) {
    using RangeType = typename FieldType::RangeType;
    using IndexType = typename FieldType::IndexType;

    GhostExchangeSetupResult<FieldType> result;

    IndexType gridLo = field.getLo();
    IndexType gridHi = field.getHi();
  IndexType innerLo = field.getInnerLo();
  IndexType innerHi = field.getInnerHi();

    constexpr size_t dims = RangeType::LimitType::length;

    auto makeRange = [](const IndexType &lo, const IndexType &hi) { return RangeType(lo, hi); };

    double seed = 1.0;
    auto makeValues = [&](const RangeType &range) {
      RangeType rangeCopy(range);
      std::vector<typename FieldType::value_type> values;
      values.reserve(rangeVolume(rangeCopy));
      for (auto it = rangeCopy.begin(); it != rangeCopy.end(); ++it) {
        values.push_back(static_cast<typename FieldType::value_type>(seed));
        seed += 1.0;
      }
      return values;
    };

    for (size_t dim = 0; dim < dims; ++dim) {
      ptrdiff_t lowerHalo = innerLo[dim] - gridLo[dim];
      if (lowerHalo > 0) {
        IndexType lo = gridLo;
        IndexType hi = gridHi;
        hi[dim] = gridLo[dim] + lowerHalo - 1;
        RangeType ghostRange = makeRange(lo, hi);
        auto values = makeValues(ghostRange);
        result.responses.push_back(boost::make_tuple(MPI_SUCCESS, toByteVector(values)));
        result.expectations.push_back({ghostRange, values});
      }

      ptrdiff_t upperHalo = gridHi[dim] - innerHi[dim];
      if (upperHalo > 0) {
        IndexType lo = gridLo;
        IndexType hi = gridHi;
        lo[dim] = gridHi[dim] - upperHalo + 1;
        RangeType ghostRange = makeRange(lo, hi);
        auto values = makeValues(ghostRange);
        result.responses.push_back(boost::make_tuple(MPI_SUCCESS, toByteVector(values)));
        result.expectations.push_back({ghostRange, values});
      }
    }

    return result;
  }

  template<typename FieldType>
  struct GhostAccumulateSetupResult {
      FieldType expectedField;
      std::vector<boost::tuple<int, std::vector<char>>> responses;
      std::vector<int> recvCounts;
  };

  template<typename FieldType>
  GhostAccumulateSetupResult<FieldType> buildGhostAccumulateSetup(FieldType &field) {
    using RangeType = typename FieldType::RangeType;
    using IndexType = typename FieldType::IndexType;

    GhostAccumulateSetupResult<FieldType> result;
    result.expectedField = field;

    IndexType gridLo = field.getLo();
    IndexType gridHi = field.getHi();
    IndexType innerLo = field.getInnerLo();
    IndexType innerHi = field.getInnerHi();

    constexpr size_t dims = RangeType::LimitType::length;

    auto makeRange = [](const IndexType &lo, const IndexType &hi) { return RangeType(lo, hi); };

    double seed = 1.0;
    auto nextValues = [&](const RangeType &range) {
      std::vector<typename FieldType::value_type> values;
      values.reserve(rangeVolume(range));
      for (auto it = range.begin(); it != range.end(); ++it) {
        values.push_back(static_cast<typename FieldType::value_type>(seed));
        seed += 1.0;
      }
      return values;
    };

    auto addInto = [&](const RangeType &range, const std::vector<typename FieldType::value_type> &values) {
      size_t idx = 0;
      for (auto it = range.begin(); it != range.end(); ++it) {
        result.expectedField[*it] += values[idx++];
      }
    };

    auto assignInto = [&](const RangeType &range, const std::vector<typename FieldType::value_type> &values) {
      size_t idx = 0;
      for (auto it = range.begin(); it != range.end(); ++it) {
        result.expectedField[*it] = values[idx++];
      }
    };

    for (size_t dim = 0; dim < dims; ++dim) {
      ptrdiff_t lowerHalo = innerLo[dim] - gridLo[dim];
      ptrdiff_t upperHalo = gridHi[dim] - innerHi[dim];

      std::optional<RangeType> loGhostRange;
      std::optional<RangeType> hiGhostRange;
      std::optional<RangeType> loSourceRange;
      std::optional<RangeType> hiSourceRange;

      if (lowerHalo > 0) {
        IndexType lo = gridLo;
        IndexType hi = gridHi;
        hi[dim] = gridLo[dim] + lowerHalo - 1;
        loGhostRange = makeRange(lo, hi);

        lo = gridLo;
        hi = gridHi;
        lo[dim] = innerLo[dim];
        hi[dim] = innerLo[dim] + lowerHalo - 1;
        loSourceRange = makeRange(lo, hi);
      }

      if (upperHalo > 0) {
        IndexType lo = gridLo;
        IndexType hi = gridHi;
        lo[dim] = gridHi[dim] - upperHalo + 1;
        hi[dim] = gridHi[dim];
        hiGhostRange = makeRange(lo, hi);

        lo = gridLo;
        hi = gridHi;
        lo[dim] = innerHi[dim] - upperHalo + 1;
        hi[dim] = innerHi[dim];
        hiSourceRange = makeRange(lo, hi);
      }

      // == lower side ==
      size_t recvLowerCount = loGhostRange ? rangeVolume(*loGhostRange) : 0;
      if (recvLowerCount > 0) {
        auto values = nextValues(*loGhostRange);
        result.responses.push_back(boost::make_tuple(MPI_SUCCESS, toByteVector(values)));
        result.recvCounts.push_back(static_cast<int>(recvLowerCount));
        addInto(*loGhostRange, values);
      } else {
        result.responses.push_back(boost::make_tuple(MPI_SUCCESS, std::vector<char>{}));
        result.recvCounts.push_back(0);
      }

      size_t recvBackLowerCount = hiSourceRange ? rangeVolume(*hiSourceRange) : 0;
      if (recvBackLowerCount > 0) {
        auto values = nextValues(*hiSourceRange);
        result.responses.push_back(boost::make_tuple(MPI_SUCCESS, toByteVector(values)));
        result.recvCounts.push_back(static_cast<int>(recvBackLowerCount));
        assignInto(*hiSourceRange, values);
      } else {
        result.responses.push_back(boost::make_tuple(MPI_SUCCESS, std::vector<char>{}));
        result.recvCounts.push_back(0);
      }

      // == upper side ==
      size_t recvUpperCount = hiGhostRange ? rangeVolume(*hiGhostRange) : 0;
      if (recvUpperCount > 0) {
        auto values = nextValues(*hiGhostRange);
        result.responses.push_back(boost::make_tuple(MPI_SUCCESS, toByteVector(values)));
        result.recvCounts.push_back(static_cast<int>(recvUpperCount));
        addInto(*hiGhostRange, values);
      } else {
        result.responses.push_back(boost::make_tuple(MPI_SUCCESS, std::vector<char>{}));
        result.recvCounts.push_back(0);
      }

      size_t recvBackUpperCount = loSourceRange ? rangeVolume(*loSourceRange) : 0;
      if (recvBackUpperCount > 0) {
        auto values = nextValues(*loSourceRange);
        result.responses.push_back(boost::make_tuple(MPI_SUCCESS, toByteVector(values)));
        result.recvCounts.push_back(static_cast<int>(recvBackUpperCount));
        assignInto(*loSourceRange, values);
      } else {
        result.responses.push_back(boost::make_tuple(MPI_SUCCESS, std::vector<char>{}));
        result.recvCounts.push_back(0);
      }
    }

    return result;
  }

  template<typename FieldType>
  void fillField(FieldType &field, typename FieldType::value_type value) {
    typename FieldType::RangeType range(field.getLo(), field.getHi());
    for (auto it = range.begin(); it != range.end(); ++it) {
      field[*it] = value;
    }
  }

  template<typename FieldType>
  void applyGhostExpectations(
      FieldType &field,
      const GhostExchangeSetupResult<FieldType> &ghostSetup
  ) {
    for (const auto &expectation : ghostSetup.expectations) {
      size_t idx = 0;
      auto rangeCopy = expectation.range;
      for (auto it = rangeCopy.begin(); it != rangeCopy.end(); ++it) {
        field[*it] = expectation.values[idx++];
      }
    }
  }

  template<size_t Rank>
  void exchangeRegistration(
      schnek::MpiCartesianDomainDecomposition<Rank> &decomposition,
      const schnek::GridRegistration &registration,
      bool useFieldInfo = true
  ) {
    auto &base = static_cast<schnek::DomainDecomposition<Rank> &>(decomposition);
    base.exchange(registration, useFieldInfo);
  }

  template<size_t Rank>
  void accumulateRegistration(
      schnek::MpiCartesianDomainDecomposition<Rank> &decomposition,
      const schnek::GridRegistration &registration,
      bool useFieldInfo = true
  ) {
    auto &base = static_cast<schnek::DomainDecomposition<Rank> &>(decomposition);
    base.accumulate(registration, useFieldInfo);
  }

}  // namespace

//
// run only these tests:
// ./schnek_tests --log_level=test_suite --run_test=mpi_cartesian_domain_decomposition

struct MpiCartesianDomainDecompositionTestFixture
{
    MpiTestContextImpl context;

    void resetContext() {
      context.args_MPI_Comm_size.clear();
      context.args_MPI_Comm_rank.clear();
      context.args_MPI_Cart_create.clear();
      context.args_MPI_Cart_coords.clear();
      context.args_MPI_Bcast.clear();
      context.args_MPI_Cart_shift.clear();
      context.args_MPI_Sendrecv.clear();

      context.ret_MPI_Comm_size.clear();
      context.ret_MPI_Comm_rank.clear();
      context.ret_MPI_Cart_create.clear();
      context.ret_MPI_Cart_coords.clear();
      context.ret_MPI_Bcast.clear();
      context.ret_MPI_Cart_shift.clear();
      context.ret_MPI_Sendrecv.clear();
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

  schnek::Range<ptrdiff_t, 1> globalRange(schnek::Array<ptrdiff_t,1>(0), schnek::Array<ptrdiff_t,1>(100));
  schnek::Range<double, 1> globalDomain(schnek::Array<double,1>(0), schnek::Array<double,1>(12.5));

  schnek::MpiCartesianDomainDecomposition<1> decomposition(context);
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  decomposition.init();

  // Checking calls
  BOOST_CHECK_EQUAL(context.args_MPI_Comm_size.size(), (size_t)1);
  BOOST_CHECK_EQUAL(context.args_MPI_Comm_size[0], context.commWorld);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create.size(), (size_t)1);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<0>(), context.commWorld);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<1>(), 1);
  const std::vector<int> cc_dims = context.args_MPI_Cart_create[0].get<2>();
  const std::vector<int> cc_periods = context.args_MPI_Cart_create[0].get<3>();

  BOOST_CHECK_EQUAL(cc_dims.size(), (size_t)1);
  BOOST_CHECK_EQUAL(cc_dims[0], 1);

  BOOST_CHECK_EQUAL(cc_periods.size(), (size_t)1);
  BOOST_CHECK_EQUAL(cc_periods[0], 1);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<4>(), 1);

  BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank.size(), (size_t)1);
  BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank[0], testComm);

  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords.size(), (size_t)1);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<0>(), testComm);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<1>(), 0);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<2>(), 1);

  BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), (size_t)0);


  BOOST_CHECK_EQUAL(decomposition.master(), true);
  BOOST_CHECK_EQUAL(decomposition.numProcs(), 1);
  BOOST_CHECK_EQUAL(decomposition.getUniqueId(), 0);
  schnek::MpiCartesianDomainDecomposition<1>::ProcRanges ranges = decomposition.getProcRanges();

  BOOST_CHECK_EQUAL(ranges[0].getLo(0), 0);
  BOOST_CHECK_EQUAL(ranges[0].getHi(0), 0);

  schnek::Range<ptrdiff_t, 1> expectedRange(schnek::Array<ptrdiff_t,1>(0), schnek::Array<ptrdiff_t,1>(100));
  SCHNEK_CHECK_EQUAL(ranges[0](0), expectedRange);
}

BOOST_FIXTURE_TEST_CASE( reductions_use_allreduce, MpiCartesianDomainDecompositionTestFixture )
{
  MPI_Comm testComm = (MPI_Comm)(void*)123;
  std::vector<int> coords(1, 0);
  context.commWorld = (MPI_Comm)(void*)574;
  context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, 4));
  context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, 0));
  context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
  context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));

  schnek::Range<ptrdiff_t, 1> globalRange(schnek::Array<ptrdiff_t,1>(0), schnek::Array<ptrdiff_t,1>(3));
  schnek::Range<double, 1> globalDomain(schnek::Array<double,1>(0.0), schnek::Array<double,1>(1.0));

  schnek::MpiCartesianDomainDecomposition<1> decomposition(context);
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);
  decomposition.init();

  context.ret_MPI_Allreduce.push_back(boost::make_tuple(MPI_SUCCESS, toByteVectorScalar(8.0)));
  context.ret_MPI_Allreduce.push_back(boost::make_tuple(MPI_SUCCESS, toByteVectorScalar(10.0)));
  context.ret_MPI_Allreduce.push_back(boost::make_tuple(MPI_SUCCESS, toByteVectorScalar(1.5)));
  context.ret_MPI_Allreduce.push_back(boost::make_tuple(MPI_SUCCESS, toByteVectorScalar(3.0)));
  context.ret_MPI_Allreduce.push_back(boost::make_tuple(MPI_SUCCESS, toByteVectorScalar(20)));
  context.ret_MPI_Allreduce.push_back(boost::make_tuple(MPI_SUCCESS, toByteVectorScalar(25)));
  context.ret_MPI_Allreduce.push_back(boost::make_tuple(MPI_SUCCESS, toByteVectorScalar(9)));
  context.ret_MPI_Allreduce.push_back(boost::make_tuple(MPI_SUCCESS, toByteVectorScalar(28l)));
  context.ret_MPI_Allreduce.push_back(boost::make_tuple(MPI_SUCCESS, toByteVectorScalar(40l)));
  context.ret_MPI_Allreduce.push_back(boost::make_tuple(MPI_SUCCESS, toByteVectorScalar(11l)));
  context.ret_MPI_Allreduce.push_back(boost::make_tuple(MPI_SUCCESS, toByteVectorScalar(6l)));

  double avgDouble = decomposition.avgReduce(2.0);
  double sumDouble = decomposition.sumReduce(2.5);
  double maxDouble = decomposition.maxReduce(1.5);
  double minDouble = decomposition.minReduce(3.0);

  int avgInt = decomposition.avgReduce(5);
  int sumInt = decomposition.sumReduce(5);
  int maxInt = decomposition.maxReduce(9);

  long avgLong = decomposition.avgReduce(7l);
  long sumLong = decomposition.sumReduce(10l);
  long maxLong = decomposition.maxReduce(11l);
  long minLong = decomposition.minReduce(6l);

  BOOST_CHECK_CLOSE(avgDouble, 2.0, 1e-12);
  BOOST_CHECK_CLOSE(sumDouble, 10.0, 1e-12);
  BOOST_CHECK_CLOSE(maxDouble, 1.5, 1e-12);
  BOOST_CHECK_CLOSE(minDouble, 3.0, 1e-12);

  BOOST_CHECK_EQUAL(avgInt, 5);
  BOOST_CHECK_EQUAL(sumInt, 25);
  BOOST_CHECK_EQUAL(maxInt, 9);

  BOOST_CHECK_EQUAL(avgLong, 7l);
  BOOST_CHECK_EQUAL(sumLong, 40l);
  BOOST_CHECK_EQUAL(maxLong, 11l);
  BOOST_CHECK_EQUAL(minLong, 6l);

  BOOST_REQUIRE_EQUAL(context.args_MPI_Allreduce.size(), static_cast<size_t>(11));

  auto checkCall = [&](size_t index, MPI_Datatype expectedType, MPI_Op expectedOp) {
    const auto &call = context.args_MPI_Allreduce[index];
    BOOST_CHECK(call.sendBufferPresent);
    BOOST_CHECK_EQUAL(call.count, 1);
    BOOST_CHECK_EQUAL(call.datatype, expectedType);
    BOOST_CHECK_EQUAL(call.op, expectedOp);
    BOOST_CHECK_EQUAL(call.comm, testComm);
  };

  checkCall(0, MPI_DOUBLE, MPI_SUM);
  checkCall(1, MPI_DOUBLE, MPI_SUM);
  checkCall(2, MPI_DOUBLE, MPI_MAX);
  checkCall(3, MPI_DOUBLE, MPI_MIN);
  checkCall(4, MPI_INT, MPI_SUM);
  checkCall(5, MPI_INT, MPI_SUM);
  checkCall(6, MPI_INT, MPI_MAX);
  checkCall(7, MPI_LONG, MPI_SUM);
  checkCall(8, MPI_LONG, MPI_SUM);
  checkCall(9, MPI_LONG, MPI_MAX);
  checkCall(10, MPI_LONG, MPI_MIN);
}

BOOST_FIXTURE_TEST_CASE( foreach_single_process_1d, MpiCartesianDomainDecompositionTestFixture )
{
  MPI_Comm testComm = (MPI_Comm)(void*)123;
  std::vector<int> coords(1, 0);
  context.commWorld = (MPI_Comm)(void*)574;
  context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, 1));
  context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, 0));
  context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
  context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));

  using GridType = schnek::Grid<double, 1>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;

  RangeType globalRange(schnek::Array<ptrdiff_t,1>(0), schnek::Array<ptrdiff_t,1>(42));
  schnek::Range<double, 1> globalDomain(schnek::Array<double,1>(0), schnek::Array<double,1>(10));

  schnek::MpiCartesianDomainDecomposition<1> decomposition(context);
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  decomposition.init();

  schnek::GridFactory<GridType> factory;
  schnek::GridRegistration registration = decomposition.registerField(factory);

  auto gridContext = decomposition.getGridContext({registration});

  int callCount = 0;
  gridContext.forEach([&](const RangeType &range, GridType &grid) {
    ++callCount;
    SCHNEK_CHECK_EQUAL(range, globalRange);
    SCHNEK_CHECK_EQUAL(grid.getRange(), globalRange);
  });

  BOOST_CHECK_EQUAL(callCount, 1);
}

BOOST_FIXTURE_TEST_CASE( multi_process_1d, MpiCartesianDomainDecompositionTestFixture )
{
  std::vector<int> numProcsArray;
  numProcsArray += 2, 5, 7, 127, 128, 129, 1023, 4000, 25000, 128000, 879484;
  std::vector<int> rankArray;
  rankArray += 1, 2, 4, 120, 126, 500, 1022, 3999, 24999, 127999, 879483;
  int globalRangeMin =  -5000;
  int globalRangeMax =  10000000;
  boost::timer::progress_display show_progress(numProcsArray.size() * rankArray.size());

  for (int numProcs: numProcsArray)
  {
    for (int rank: rankArray)
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

      schnek::Range<ptrdiff_t, 1> globalRange = schnek::Range<ptrdiff_t, 1>(schnek::Array<ptrdiff_t,1>(globalRangeMin), schnek::Array<ptrdiff_t,1>(globalRangeMax));
      schnek::Range<double, 1> globalDomain(schnek::Array<double,1>(0), schnek::Array<double,1>(12.5));

      schnek::MpiCartesianDomainDecomposition<1> decomposition(context);
      decomposition.setGlobalRange(globalRange);
      decomposition.setGlobalDomain(globalDomain);

      decomposition.init();

      // Checking calls
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size[0], context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<0>(), context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<1>(), 1);
      const std::vector<int> cc_dims = context.args_MPI_Cart_create[0].get<2>();
      const std::vector<int> cc_periods = context.args_MPI_Cart_create[0].get<3>();

      BOOST_CHECK_EQUAL(cc_dims.size(), (size_t)1);
      BOOST_CHECK_EQUAL(cc_dims[0], numProcs);

      BOOST_CHECK_EQUAL(cc_periods.size(), (size_t)1);
      BOOST_CHECK_EQUAL(cc_periods[0], 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<4>(), 1);

      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank[0], testComm);

      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<0>(), testComm);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<1>(), rank);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<2>(), 1);

      BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), (size_t)0);


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

        schnek::Range<ptrdiff_t, 1> expectedRange = schnek::Range<ptrdiff_t, 1>(schnek::Array<ptrdiff_t,1>(lo), schnek::Array<ptrdiff_t,1>(hi));
        SCHNEK_CHECK_EQUAL(ranges[0](r), expectedRange);
      }
    }
  }
}

BOOST_FIXTURE_TEST_CASE( foreach_multi_process_1d, MpiCartesianDomainDecompositionTestFixture )
{
  const int numProcs = 4;
  const int rank = 2;

  MPI_Comm testComm = (MPI_Comm)(void*)123;
  std::vector<int> coords(1, rank);
  context.commWorld = (MPI_Comm)(void*)574;
  context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, numProcs));
  context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, rank));
  context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
  context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));

  using GridType = schnek::Grid<double, 1>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;

  RangeType globalRange(schnek::Array<ptrdiff_t,1>(0), schnek::Array<ptrdiff_t,1>(7));
  schnek::Range<double, 1> globalDomain(schnek::Array<double,1>(0.0), schnek::Array<double,1>(1.0));

  schnek::MpiCartesianDomainDecomposition<1> decomposition(context);
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  decomposition.init();

  schnek::GridFactory<GridType> factory;
  schnek::GridRegistration registration = decomposition.registerField(factory);

  auto gridContext = decomposition.getGridContext({registration});

  const ptrdiff_t lo = globalRange.getLo()[0] + ((globalRange.getHi()[0] - globalRange.getLo()[0] + 1) * rank) / numProcs;
  const ptrdiff_t hi = globalRange.getLo()[0] + ((globalRange.getHi()[0] - globalRange.getLo()[0] + 1) * (rank + 1)) / numProcs - 1;
  RangeType expectedRange{schnek::Array<ptrdiff_t,1>{lo}, schnek::Array<ptrdiff_t,1>{hi}};

  int callCount = 0;
  gridContext.forEach([&](const RangeType &range, GridType &grid) {
    ++callCount;
    SCHNEK_CHECK_EQUAL(range, expectedRange);
    SCHNEK_CHECK_EQUAL(grid.getRange(), expectedRange);
  });

  BOOST_CHECK_EQUAL(callCount, 1);
}

  BOOST_FIXTURE_TEST_CASE( ghost_exchange_updates_ghost_cells_1d, MpiCartesianDomainDecompositionTestFixture )
  {
    MPI_Comm testComm = (MPI_Comm)(void*)123;
    std::vector<int> coords(1, 0);
    context.commWorld = (MPI_Comm)(void*)574;
    context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, 1));
    context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, 0));
    context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
    context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));

    using FieldType = schnek::Field<double, 1>;
    using RangeType = schnek::Range<ptrdiff_t, 1>;
    using DomainType = schnek::Range<double, 1>;
    using StaggerType = typename FieldType::StaggerType;

    RangeType globalRange(schnek::Array<ptrdiff_t,1>(0), schnek::Array<ptrdiff_t,1>(3));
    DomainType globalDomain(schnek::Array<double,1>(0.0), schnek::Array<double,1>(1.0));

    schnek::MpiCartesianDomainDecomposition<1> decomposition(context);
    decomposition.setGlobalRange(globalRange);
    decomposition.setGlobalDomain(globalDomain);
    decomposition.init();

    StaggerType noStagger(false);
    constexpr int ghostCells = 1;
    schnek::GridFactory<FieldType> factory(noStagger, ghostCells);
    schnek::GridRegistration registration = decomposition.registerField(factory);

    auto gridContext = decomposition.getGridContext({registration});
    FieldType *field = nullptr;
    gridContext.forEach([&](const RangeType &, FieldType &localField) { field = &localField; });
    BOOST_REQUIRE(field != nullptr);

  fillField(*field, -42.0);

  FieldType expectedField(*field);
  auto ghostSetup = buildGhostExchangeSetup(*field);
  applyGhostExpectations(expectedField, ghostSetup);
  context.ret_MPI_Sendrecv = ghostSetup.responses;

    std::array<int, 1> prevRanks{{7}};
    std::array<int, 1> nextRanks{{8}};
    for (size_t dim = 0; dim < prevRanks.size(); ++dim) {
      context.ret_MPI_Cart_shift.push_back(boost::make_tuple(MPI_SUCCESS, prevRanks[dim], nextRanks[dim]));
    }

    exchangeRegistration<1>(decomposition, registration, false);

    BOOST_CHECK_EQUAL(context.args_MPI_Cart_shift.size(), prevRanks.size());
    BOOST_CHECK_EQUAL(context.args_MPI_Cart_shift[0].get<1>(), 0);
    BOOST_CHECK_EQUAL(context.args_MPI_Cart_shift[0].get<2>(), 1);

    BOOST_REQUIRE_EQUAL(context.args_MPI_Sendrecv.size(), ghostSetup.expectations.size());
    for (size_t i = 0; i < ghostSetup.expectations.size(); ++i) {
      const auto &call = context.args_MPI_Sendrecv[i];
      BOOST_CHECK(call.recvBufferPresent);
      BOOST_CHECK_EQUAL(call.recvCount, static_cast<int>(ghostSetup.expectations[i].values.size()));
    }

    const auto &lowerCall = context.args_MPI_Sendrecv[0];
    BOOST_CHECK_EQUAL(lowerCall.dest, nextRanks[0]);
    BOOST_CHECK_EQUAL(lowerCall.source, prevRanks[0]);
    const auto &upperCall = context.args_MPI_Sendrecv[1];
    BOOST_CHECK_EQUAL(upperCall.dest, prevRanks[0]);
    BOOST_CHECK_EQUAL(upperCall.source, nextRanks[0]);

    typename FieldType::RangeType verificationRange(field->getLo(), field->getHi());
    for (auto it = verificationRange.begin(); it != verificationRange.end(); ++it) {
      BOOST_CHECK_EQUAL((*field)[*it], expectedField[*it]);
    }
  }

  BOOST_FIXTURE_TEST_CASE( ghost_accumulate_updates_ghost_and_inner_cells_1d, MpiCartesianDomainDecompositionTestFixture )
  {
    MPI_Comm testComm = (MPI_Comm)(void*)123;
    std::vector<int> coords(1, 0);
    context.commWorld = (MPI_Comm)(void*)574;
    context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, 1));
    context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, 0));
    context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
    context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));

    using FieldType = schnek::Field<double, 1>;
    using RangeType = schnek::Range<ptrdiff_t, 1>;
    using DomainType = schnek::Range<double, 1>;
    using StaggerType = typename FieldType::StaggerType;

    RangeType globalRange(schnek::Array<ptrdiff_t,1>(0), schnek::Array<ptrdiff_t,1>(3));
    DomainType globalDomain(schnek::Array<double,1>(0.0), schnek::Array<double,1>(1.0));

    schnek::MpiCartesianDomainDecomposition<1> decomposition(context);
    decomposition.setGlobalRange(globalRange);
    decomposition.setGlobalDomain(globalDomain);
    decomposition.init();

    StaggerType noStagger(false);
    constexpr int ghostCells = 1;
    schnek::GridFactory<FieldType> factory(noStagger, ghostCells);
    schnek::GridRegistration registration = decomposition.registerField(factory);

    auto gridContext = decomposition.getGridContext({registration});
    FieldType *field = nullptr;
    gridContext.forEach([&](const RangeType &, FieldType &localField) { field = &localField; });
    BOOST_REQUIRE(field != nullptr);

    fillField(*field, -5.0);

    auto accumulateSetup = buildGhostAccumulateSetup(*field);
    context.ret_MPI_Sendrecv = accumulateSetup.responses;

    std::array<int, 1> prevRanks{{7}};
    std::array<int, 1> nextRanks{{8}};
    for (size_t dim = 0; dim < prevRanks.size(); ++dim) {
      context.ret_MPI_Cart_shift.push_back(boost::make_tuple(MPI_SUCCESS, prevRanks[dim], nextRanks[dim]));
    }

    accumulateRegistration<1>(decomposition, registration, false);

    BOOST_CHECK_EQUAL(context.args_MPI_Cart_shift.size(), prevRanks.size());
    BOOST_CHECK_EQUAL(context.args_MPI_Cart_shift[0].get<1>(), 0);
    BOOST_CHECK_EQUAL(context.args_MPI_Cart_shift[0].get<2>(), 1);

    BOOST_REQUIRE_EQUAL(context.args_MPI_Sendrecv.size(), accumulateSetup.recvCounts.size());
    for (size_t i = 0; i < accumulateSetup.recvCounts.size(); ++i) {
      const auto &call = context.args_MPI_Sendrecv[i];
      BOOST_CHECK(call.recvBufferPresent);
      BOOST_CHECK_EQUAL(call.recvCount, accumulateSetup.recvCounts[i]);
    }

    const auto &lowerInCall = context.args_MPI_Sendrecv[0];
    BOOST_CHECK_EQUAL(lowerInCall.dest, nextRanks[0]);
    BOOST_CHECK_EQUAL(lowerInCall.source, prevRanks[0]);
    const auto &lowerBackCall = context.args_MPI_Sendrecv[1];
    BOOST_CHECK_EQUAL(lowerBackCall.dest, prevRanks[0]);
    BOOST_CHECK_EQUAL(lowerBackCall.source, nextRanks[0]);
    const auto &upperInCall = context.args_MPI_Sendrecv[2];
    BOOST_CHECK_EQUAL(upperInCall.dest, prevRanks[0]);
    BOOST_CHECK_EQUAL(upperInCall.source, nextRanks[0]);
    const auto &upperBackCall = context.args_MPI_Sendrecv[3];
    BOOST_CHECK_EQUAL(upperBackCall.dest, nextRanks[0]);
    BOOST_CHECK_EQUAL(upperBackCall.source, prevRanks[0]);

    typename FieldType::RangeType verificationRange(field->getLo(), field->getHi());
    for (auto it = verificationRange.begin(); it != verificationRange.end(); ++it) {
      BOOST_CHECK_EQUAL((*field)[*it], accumulateSetup.expectedField[*it]);
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

  schnek::Range<ptrdiff_t, 1> globalRange(schnek::Array<ptrdiff_t,1>(0), schnek::Array<ptrdiff_t,1>(99));
  schnek::Range<double, 1> globalDomain(schnek::Array<double,1>(0), schnek::Array<double,1>(12.5));

  schnek::Grid<double, 1> weights(schnek::Array<ptrdiff_t,1>(0), schnek::Array<ptrdiff_t,1>(49));
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
  BOOST_CHECK_EQUAL(context.args_MPI_Comm_size.size(), (size_t)1);
  BOOST_CHECK_EQUAL(context.args_MPI_Comm_size[0], context.commWorld);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create.size(), (size_t)1);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<0>(), context.commWorld);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<1>(), 1);
  const std::vector<int> cc_dims = context.args_MPI_Cart_create[0].get<2>();
  const std::vector<int> cc_periods = context.args_MPI_Cart_create[0].get<3>();

  BOOST_CHECK_EQUAL(cc_dims.size(), (size_t)1);
  BOOST_CHECK_EQUAL(cc_dims[0], 1);

  BOOST_CHECK_EQUAL(cc_periods.size(), (size_t)1);
  BOOST_CHECK_EQUAL(cc_periods[0], 1);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<4>(), 1);

  BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank.size(), (size_t)1);
  BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank[0], testComm);

  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords.size(), (size_t)1);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<0>(), testComm);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<1>(), 0);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<2>(), 1);

  BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), (size_t)1);
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

  schnek::Range<ptrdiff_t, 1> expectedRange(schnek::Array<ptrdiff_t,1>(0), schnek::Array<ptrdiff_t,1>(99));
  SCHNEK_CHECK_EQUAL(ranges[0](0), expectedRange);
}

BOOST_FIXTURE_TEST_CASE( multi_process_1d_global_master, MpiCartesianDomainDecompositionTestFixture )
{
  std::vector<int> numProcsArray;
  numProcsArray += 2, 5, 7, 127, 128, 129, 1023;

  int globalRangeMin =  -4000;
  int globalRangeMax =  3999;


  for (int numProcs: numProcsArray)
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

      schnek::Range<ptrdiff_t, 1> globalRange = schnek::Range<ptrdiff_t, 1>(schnek::Array<ptrdiff_t,1>(globalRangeMin), schnek::Array<ptrdiff_t,1>(globalRangeMax));
      schnek::Range<double, 1> globalDomain = schnek::Range<double, 1>(schnek::Array<double,1>(0), schnek::Array<double,1>(12.5));

      schnek::Grid<double, 1> weights(schnek::Array<ptrdiff_t,1>(0), schnek::Array<ptrdiff_t,1>(weightMax));
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
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size[0], context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<0>(), context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<1>(), 1);
      const std::vector<int> cc_dims = context.args_MPI_Cart_create[0].get<2>();
      const std::vector<int> cc_periods = context.args_MPI_Cart_create[0].get<3>();

      BOOST_CHECK_EQUAL(cc_dims.size(), (size_t)1);
      BOOST_CHECK_EQUAL(cc_dims[0], numProcs);

      BOOST_CHECK_EQUAL(cc_periods.size(), (size_t)1);
      BOOST_CHECK_EQUAL(cc_periods[0], 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<4>(), 1);

      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank[0], testComm);

      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<0>(), testComm);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<1>(), 0);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<2>(), 1);

      BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), (size_t)1);
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
        schnek::Range<ptrdiff_t, 1> expectedRangeA = schnek::Range<ptrdiff_t, 1>(schnek::Array<ptrdiff_t,1>(lo), schnek::Array<ptrdiff_t,1>(hi));
        schnek::Range<ptrdiff_t, 1> expectedRangeB = schnek::Range<ptrdiff_t, 1>(schnek::Array<ptrdiff_t,1>(lo + resolution), schnek::Array<ptrdiff_t,1>(hi + resolution));
        schnek::Range<ptrdiff_t, 1> foundRange = ranges[0](i);
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

  for (int numProcs: numProcsArray)
  {
    for (int rank: rankArray)
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

      schnek::Range<ptrdiff_t, 1> globalRange = schnek::Range<ptrdiff_t, 1>(schnek::Array<ptrdiff_t,1>(globalRangeMin), schnek::Array<ptrdiff_t,1>(globalRangeMax));
      schnek::Range<double, 1> globalDomain = schnek::Range<double, 1>(schnek::Array<double,1>(0), schnek::Array<double,1>(12.5));

      schnek::Grid<double, 1> weights(schnek::Array<ptrdiff_t,1>(0), schnek::Array<ptrdiff_t,1>(weightMax));
      weights = 1.0;

      schnek::MpiCartesianDomainDecomposition<1> decomposition(context);

      decomposition.setGlobalRange(globalRange);
      decomposition.setGlobalDomain(globalDomain);
      decomposition.setGlobalWeights(weights);

      decomposition.init();

      // Checking calls
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size[0], context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<0>(), context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<1>(), 1);
      const std::vector<int> cc_dims = context.args_MPI_Cart_create[0].get<2>();
      const std::vector<int> cc_periods = context.args_MPI_Cart_create[0].get<3>();

      BOOST_CHECK_EQUAL(cc_dims.size(), (size_t)1);
      BOOST_CHECK_EQUAL(cc_dims[0], numProcs);

      BOOST_CHECK_EQUAL(cc_periods.size(), (size_t)1);
      BOOST_CHECK_EQUAL(cc_periods[0], 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<4>(), 1);

      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank[0], testComm);

      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<0>(), testComm);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<1>(), rank);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<2>(), 1);

      BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), (size_t)1);
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

  schnek::Range<ptrdiff_t, 2> globalRange(schnek::Array<ptrdiff_t,2>(0, -10), schnek::Array<ptrdiff_t,2>(100, 200));
  schnek::Range<double, 2> globalDomain(schnek::Array<double,2>(0, 0), schnek::Array<double,2>(12.5, 42.2));

  schnek::MpiCartesianDomainDecomposition<2> decomposition(context);
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  decomposition.init();

  // Checking calls
  BOOST_CHECK_EQUAL(context.args_MPI_Comm_size.size(), (size_t)1);
  BOOST_CHECK_EQUAL(context.args_MPI_Comm_size[0], context.commWorld);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create.size(), (size_t)1);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<0>(), context.commWorld);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<1>(), 2);
  const std::vector<int> cc_dims = context.args_MPI_Cart_create[0].get<2>();
  const std::vector<int> cc_periods = context.args_MPI_Cart_create[0].get<3>();

  BOOST_CHECK_EQUAL(cc_dims.size(), (size_t)2);
  BOOST_CHECK_EQUAL(cc_dims[0], 1);
  BOOST_CHECK_EQUAL(cc_dims[1], 1);

  BOOST_CHECK_EQUAL(cc_periods.size(), (size_t)2);
  BOOST_CHECK_EQUAL(cc_periods[0], 1);
  BOOST_CHECK_EQUAL(cc_periods[1], 1);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<4>(), 1);

  BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank.size(), (size_t)1);
  BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank[0], testComm);

  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords.size(), (size_t)1);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<0>(), testComm);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<1>(), 0);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<2>(), 2);

  BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), (size_t)0);

  BOOST_CHECK_EQUAL(decomposition.master(), true);
  BOOST_CHECK_EQUAL(decomposition.numProcs(), 1);
  BOOST_CHECK_EQUAL(decomposition.getUniqueId(), 0);
  schnek::MpiCartesianDomainDecomposition<2>::ProcRanges ranges = decomposition.getProcRanges();

  BOOST_CHECK_EQUAL(ranges[0].getLo(0), 0);
  BOOST_CHECK_EQUAL(ranges[0].getHi(0), 0);
  BOOST_CHECK_EQUAL(ranges[1].getLo(0), 0);
  BOOST_CHECK_EQUAL(ranges[1].getHi(0), 0);

  schnek::Range<ptrdiff_t, 1> expectedRange0(schnek::Array<ptrdiff_t,1>(0), schnek::Array<ptrdiff_t,1>(100));
  schnek::Range<ptrdiff_t, 1> expectedRange1(schnek::Array<ptrdiff_t,1>(-10), schnek::Array<ptrdiff_t,1>(200));
  SCHNEK_CHECK_EQUAL(ranges[0](0), expectedRange0);
  SCHNEK_CHECK_EQUAL(ranges[1](0), expectedRange1);
}

BOOST_FIXTURE_TEST_CASE( foreach_single_process_2d, MpiCartesianDomainDecompositionTestFixture )
{
  MPI_Comm testComm = (MPI_Comm)(void*)123;
  std::vector<int> coords(2, 0);
  context.commWorld = (MPI_Comm)(void*)574;
  context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, 1));
  context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, 0));
  context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
  context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));

  using GridType = schnek::Grid<double, 2>;
  using RangeType = schnek::Range<ptrdiff_t, 2>;

  RangeType globalRange(schnek::Array<ptrdiff_t,2>(1, -3), schnek::Array<ptrdiff_t,2>(8, 4));
  schnek::Range<double, 2> globalDomain(schnek::Array<double,2>(0.0, -1.0), schnek::Array<double,2>(2.0, 3.5));

  schnek::MpiCartesianDomainDecomposition<2> decomposition(context);
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  decomposition.init();

  schnek::GridFactory<GridType> factory;
  schnek::GridRegistration registration = decomposition.registerField(factory);

  auto gridContext = decomposition.getGridContext({registration});

  int callCount = 0;
  gridContext.forEach([&](const RangeType &range, GridType &grid) {
    ++callCount;
    SCHNEK_CHECK_EQUAL(range, globalRange);
    SCHNEK_CHECK_EQUAL(grid.getRange(), globalRange);
  });

  BOOST_CHECK_EQUAL(callCount, 1);
}

  BOOST_FIXTURE_TEST_CASE( ghost_exchange_updates_ghost_cells_2d, MpiCartesianDomainDecompositionTestFixture )
  {
    MPI_Comm testComm = (MPI_Comm)(void*)123;
    std::vector<int> coords(2, 0);
    context.commWorld = (MPI_Comm)(void*)574;
    context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, 1));
    context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, 0));
    context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
    context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));

    using FieldType = schnek::Field<double, 2>;
    using RangeType = schnek::Range<ptrdiff_t, 2>;
    using DomainType = schnek::Range<double, 2>;
    using StaggerType = typename FieldType::StaggerType;

    RangeType globalRange(schnek::Array<ptrdiff_t,2>(0, 0), schnek::Array<ptrdiff_t,2>(3, 2));
    DomainType globalDomain(schnek::Array<double,2>(0.0, 0.0), schnek::Array<double,2>(1.0, 1.0));

    schnek::MpiCartesianDomainDecomposition<2> decomposition(context);
    decomposition.setGlobalRange(globalRange);
    decomposition.setGlobalDomain(globalDomain);
    decomposition.init();

    StaggerType noStagger(false);
    constexpr int ghostCells = 1;
    schnek::GridFactory<FieldType> factory(noStagger, ghostCells);
    schnek::GridRegistration registration = decomposition.registerField(factory);

    auto gridContext = decomposition.getGridContext({registration});
    FieldType *field = nullptr;
    gridContext.forEach([&](const RangeType &, FieldType &localField) { field = &localField; });
    BOOST_REQUIRE(field != nullptr);

  fillField(*field, -13.0);

  FieldType expectedField(*field);
  auto ghostSetup = buildGhostExchangeSetup(*field);
  applyGhostExpectations(expectedField, ghostSetup);
  context.ret_MPI_Sendrecv = ghostSetup.responses;

    std::array<int, 2> prevRanks{{10, 20}};
    std::array<int, 2> nextRanks{{11, 21}};
    for (size_t dim = 0; dim < prevRanks.size(); ++dim) {
      context.ret_MPI_Cart_shift.push_back(boost::make_tuple(MPI_SUCCESS, prevRanks[dim], nextRanks[dim]));
    }

    exchangeRegistration<2>(decomposition, registration, false);

    BOOST_CHECK_EQUAL(context.args_MPI_Cart_shift.size(), prevRanks.size());
    for (size_t dim = 0; dim < prevRanks.size(); ++dim) {
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_shift[dim].get<1>(), static_cast<int>(dim));
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_shift[dim].get<2>(), 1);
    }

    BOOST_REQUIRE_EQUAL(context.args_MPI_Sendrecv.size(), ghostSetup.expectations.size());
    for (size_t i = 0; i < ghostSetup.expectations.size(); ++i) {
      const auto &call = context.args_MPI_Sendrecv[i];
      BOOST_CHECK(call.recvBufferPresent);
      BOOST_CHECK_EQUAL(call.recvCount, static_cast<int>(ghostSetup.expectations[i].values.size()));
    }

    for (size_t dim = 0; dim < prevRanks.size(); ++dim) {
      size_t baseIdx = 2 * dim;
      const auto &lowerCall = context.args_MPI_Sendrecv[baseIdx];
      BOOST_CHECK_EQUAL(lowerCall.dest, nextRanks[dim]);
      BOOST_CHECK_EQUAL(lowerCall.source, prevRanks[dim]);
      const auto &upperCall = context.args_MPI_Sendrecv[baseIdx + 1];
      BOOST_CHECK_EQUAL(upperCall.dest, prevRanks[dim]);
      BOOST_CHECK_EQUAL(upperCall.source, nextRanks[dim]);
    }

    typename FieldType::RangeType verificationRange(field->getLo(), field->getHi());
    for (auto it = verificationRange.begin(); it != verificationRange.end(); ++it) {
      BOOST_CHECK_EQUAL((*field)[*it], expectedField[*it]);
    }
  }

  BOOST_FIXTURE_TEST_CASE( ghost_accumulate_updates_ghost_and_inner_cells_2d, MpiCartesianDomainDecompositionTestFixture )
  {
    MPI_Comm testComm = (MPI_Comm)(void*)123;
    std::vector<int> coords(2, 0);
    context.commWorld = (MPI_Comm)(void*)574;
    context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, 1));
    context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, 0));
    context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
    context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));

    using FieldType = schnek::Field<double, 2>;
    using RangeType = schnek::Range<ptrdiff_t, 2>;
    using DomainType = schnek::Range<double, 2>;
    using StaggerType = typename FieldType::StaggerType;

    RangeType globalRange(schnek::Array<ptrdiff_t,2>(0, 0), schnek::Array<ptrdiff_t,2>(3, 2));
    DomainType globalDomain(schnek::Array<double,2>(0.0, 0.0), schnek::Array<double,2>(1.0, 1.0));

    schnek::MpiCartesianDomainDecomposition<2> decomposition(context);
    decomposition.setGlobalRange(globalRange);
    decomposition.setGlobalDomain(globalDomain);
    decomposition.init();

    StaggerType noStagger(false);
    constexpr int ghostCells = 1;
    schnek::GridFactory<FieldType> factory(noStagger, ghostCells);
    schnek::GridRegistration registration = decomposition.registerField(factory);

    auto gridContext = decomposition.getGridContext({registration});
    FieldType *field = nullptr;
    gridContext.forEach([&](const RangeType &, FieldType &localField) { field = &localField; });
    BOOST_REQUIRE(field != nullptr);

    fillField(*field, -3.0);

    auto accumulateSetup = buildGhostAccumulateSetup(*field);
    context.ret_MPI_Sendrecv = accumulateSetup.responses;

    std::array<int, 2> prevRanks{{10, 20}};
    std::array<int, 2> nextRanks{{11, 21}};
    for (size_t dim = 0; dim < prevRanks.size(); ++dim) {
      context.ret_MPI_Cart_shift.push_back(boost::make_tuple(MPI_SUCCESS, prevRanks[dim], nextRanks[dim]));
    }

    accumulateRegistration<2>(decomposition, registration, false);

    BOOST_CHECK_EQUAL(context.args_MPI_Cart_shift.size(), prevRanks.size());
    for (size_t dim = 0; dim < prevRanks.size(); ++dim) {
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_shift[dim].get<1>(), static_cast<int>(dim));
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_shift[dim].get<2>(), 1);
    }

    BOOST_REQUIRE_EQUAL(context.args_MPI_Sendrecv.size(), accumulateSetup.recvCounts.size());
    for (size_t i = 0; i < accumulateSetup.recvCounts.size(); ++i) {
      const auto &call = context.args_MPI_Sendrecv[i];
      BOOST_CHECK(call.recvBufferPresent);
      BOOST_CHECK_EQUAL(call.recvCount, accumulateSetup.recvCounts[i]);
    }

    for (size_t dim = 0; dim < prevRanks.size(); ++dim) {
      size_t baseIdx = 4 * dim;
      const auto &lowerInCall = context.args_MPI_Sendrecv[baseIdx];
      BOOST_CHECK_EQUAL(lowerInCall.dest, nextRanks[dim]);
      BOOST_CHECK_EQUAL(lowerInCall.source, prevRanks[dim]);
      const auto &lowerBackCall = context.args_MPI_Sendrecv[baseIdx + 1];
      BOOST_CHECK_EQUAL(lowerBackCall.dest, prevRanks[dim]);
      BOOST_CHECK_EQUAL(lowerBackCall.source, nextRanks[dim]);
      const auto &upperInCall = context.args_MPI_Sendrecv[baseIdx + 2];
      BOOST_CHECK_EQUAL(upperInCall.dest, prevRanks[dim]);
      BOOST_CHECK_EQUAL(upperInCall.source, nextRanks[dim]);
      const auto &upperBackCall = context.args_MPI_Sendrecv[baseIdx + 3];
      BOOST_CHECK_EQUAL(upperBackCall.dest, nextRanks[dim]);
      BOOST_CHECK_EQUAL(upperBackCall.source, prevRanks[dim]);
    }

    typename FieldType::RangeType verificationRange(field->getLo(), field->getHi());
    for (auto it = verificationRange.begin(); it != verificationRange.end(); ++it) {
      BOOST_CHECK_EQUAL((*field)[*it], accumulateSetup.expectedField[*it]);
    }
  }

BOOST_FIXTURE_TEST_CASE( multi_process_2d, MpiCartesianDomainDecompositionTestFixture )
{
  std::vector<int> numProcsArray;
  numProcsArray += 2, 5, 7, 127, 128, 129, 1023, 4000, 25000, 128000, 879484;
  std::vector<int> rankArray;
  rankArray += 1, 2, 4, 120, 126, 500, 1022, 3999, 24999, 127999, 879483;

  std::vector<int> globalRangeMin(2, -5000);
  std::vector<int> globalRangeMax(2,  5000);

  boost::timer::progress_display show_progress(numProcsArray.size() * rankArray.size());

  for (int numProcs: numProcsArray)
  {
    for (int rank: rankArray)
    {
      ++show_progress;
      if (rank >= numProcs) continue;
      std::vector<size_t> factors;
      std::vector<size_t> weights(2, 10001);
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

      schnek::Range<ptrdiff_t, 2> globalRange
        = schnek::Range<ptrdiff_t, 2>(
                schnek::Array<ptrdiff_t,2>(globalRangeMin[0], globalRangeMin[1]),
                schnek::Array<ptrdiff_t,2>(globalRangeMax[0], globalRangeMax[1]));
      schnek::Range<double, 2> globalDomain(schnek::Array<double,2>(0, -1.5), schnek::Array<double,2>(12.5, 42.2));

      schnek::MpiCartesianDomainDecomposition<2> decomposition(context);
      decomposition.setGlobalRange(globalRange);
      decomposition.setGlobalDomain(globalDomain);

      decomposition.init();

      // Checking calls
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size[0], context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<0>(), context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<1>(), 2);
      const std::vector<int> cc_dims = context.args_MPI_Cart_create[0].get<2>();
      const std::vector<int> cc_periods = context.args_MPI_Cart_create[0].get<3>();

      BOOST_CHECK_EQUAL(cc_dims.size(), (size_t)2);
      BOOST_CHECK_EQUAL(cc_dims[0], factors[0]);
      BOOST_CHECK_EQUAL(cc_dims[1], factors[1]);

      BOOST_CHECK_EQUAL(cc_periods.size(), (size_t)2);
      BOOST_CHECK_EQUAL(cc_periods[0], 1);
      BOOST_CHECK_EQUAL(cc_periods[1], 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<4>(), 1);

      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank[0], testComm);

      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<0>(), testComm);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<1>(), rank);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<2>(), 2);

      BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), (size_t)0);


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

          schnek::Range<ptrdiff_t, 1> expectedRange = schnek::Range<ptrdiff_t, 1>(schnek::Array<ptrdiff_t,1>(lo), schnek::Array<ptrdiff_t,1>(hi));
          SCHNEK_CHECK_EQUAL(ranges[d](r), expectedRange);
        }
      }
    }
  }
}

BOOST_FIXTURE_TEST_CASE( foreach_multi_process_2d, MpiCartesianDomainDecompositionTestFixture )
{
  const int numProcs = 6;
  const int rank = 4;

  std::vector<size_t> factors;
  std::vector<size_t> weights;
  weights += 12, 6;
  schnek::equalFactors(numProcs, 2, factors, weights);

  MPI_Comm testComm = (MPI_Comm)(void*)123;
  std::vector<int> coords(2);
  coords[0] = rank / factors[1];
  coords[1] = rank % factors[1];

  context.commWorld = (MPI_Comm)(void*)574;
  context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, numProcs));
  context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, rank));
  context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
  context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));

  using GridType = schnek::Grid<double, 2>;
  using RangeType = schnek::Range<ptrdiff_t, 2>;

  RangeType globalRange(schnek::Array<ptrdiff_t,2>(0, 0), schnek::Array<ptrdiff_t,2>(11, 5));
  schnek::Range<double, 2> globalDomain(schnek::Array<double,2>(0.0, 0.0), schnek::Array<double,2>(1.0, 1.0));

  schnek::MpiCartesianDomainDecomposition<2> decomposition(context);
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  decomposition.init();

  schnek::GridFactory<GridType> factory;
  schnek::GridRegistration registration = decomposition.registerField(factory);

  auto gridContext = decomposition.getGridContext({registration});

  const auto globalLo = globalRange.getLo();
  const auto globalHi = globalRange.getHi();
  const ptrdiff_t lo0 = globalLo[0] + ((globalHi[0] - globalLo[0] + 1) * coords[0]) / factors[0];
  const ptrdiff_t hi0 = globalLo[0] + ((globalHi[0] - globalLo[0] + 1) * (coords[0] + 1)) / factors[0] - 1;
  const ptrdiff_t lo1 = globalLo[1] + ((globalHi[1] - globalLo[1] + 1) * coords[1]) / factors[1];
  const ptrdiff_t hi1 = globalLo[1] + ((globalHi[1] - globalLo[1] + 1) * (coords[1] + 1)) / factors[1] - 1;
  RangeType expectedRange{schnek::Array<ptrdiff_t,2>{lo0, lo1}, schnek::Array<ptrdiff_t,2>{hi0, hi1}};

  int callCount = 0;
  gridContext.forEach([&](const RangeType &range, GridType &grid) {
    ++callCount;
    SCHNEK_CHECK_EQUAL(range, expectedRange);
    SCHNEK_CHECK_EQUAL(grid.getRange(), expectedRange);
  });

  BOOST_CHECK_EQUAL(callCount, 1);
}

BOOST_FIXTURE_TEST_CASE( multi_process_2d_global_master, MpiCartesianDomainDecompositionTestFixture )
{
  std::vector<int> numProcsArray;
  numProcsArray += 2, 5, 7, 127, 128, 129, 1023, 4000, 25000, 128000, 1093950;

  int globalRangeMin =  -4000;
  int globalRangeMax =  3999;
  boost::timer::progress_display show_progress(numProcsArray.size() * 5);

  for (int numProcs: numProcsArray)
  {
    for (int resolution = 1; resolution <= 16; resolution *= 2)
    {
      ++show_progress;
      std::vector<size_t> factors;
      std::vector<size_t> box(2, 8000);
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

      schnek::Range<ptrdiff_t, 2> globalRange
        = schnek::Range<ptrdiff_t, 2>(
                schnek::Array<ptrdiff_t,2>(globalRangeMin, globalRangeMin),
                schnek::Array<ptrdiff_t,2>(globalRangeMax, globalRangeMax));
      schnek::Range<double, 2> globalDomain(schnek::Array<double,2>(0, -1.5), schnek::Array<double,2>(12.5, 42.2));

      schnek::Grid<double, 2> weights(schnek::Array<ptrdiff_t,2>(0, 0), schnek::Array<ptrdiff_t,2>(weightMax, weightMax));
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
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size[0], context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<0>(), context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<1>(), 2);
      const std::vector<int> cc_dims = context.args_MPI_Cart_create[0].get<2>();
      const std::vector<int> cc_periods = context.args_MPI_Cart_create[0].get<3>();

      BOOST_CHECK_EQUAL(cc_dims.size(), (size_t)2);
      BOOST_CHECK_EQUAL(cc_dims[0], factors[0]);
      BOOST_CHECK_EQUAL(cc_dims[1], factors[1]);

      BOOST_CHECK_EQUAL(cc_periods.size(), (size_t)2);
      BOOST_CHECK_EQUAL(cc_periods[0], 1);
      BOOST_CHECK_EQUAL(cc_periods[1], 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<4>(), 1);

      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank[0], testComm);

      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<0>(), testComm);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<1>(), 0);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<2>(), 2);

      BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), (size_t)2);
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
          schnek::Range<ptrdiff_t, 1> expectedRangeA = schnek::Range<ptrdiff_t, 1>(schnek::Array<ptrdiff_t,1>(lo), schnek::Array<ptrdiff_t,1>(hi));
          schnek::Range<ptrdiff_t, 1> expectedRangeB = schnek::Range<ptrdiff_t, 1>(schnek::Array<ptrdiff_t,1>(lo + resolution), schnek::Array<ptrdiff_t,1>(hi + resolution));
          schnek::Range<ptrdiff_t, 1> foundRange = ranges[r](i);
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
  boost::timer::progress_display show_progress(numProcsArray.size() * rankArray.size());

  for (int numProcs: numProcsArray)
  {
    for (int rank: rankArray)
    {
      ++show_progress;
      if (rank >= numProcs) continue;

      std::vector<size_t> factors;
      std::vector<size_t> box(2, 8000);
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

      schnek::Range<ptrdiff_t, 2> globalRange
        = schnek::Range<ptrdiff_t, 2>(
                schnek::Array<ptrdiff_t,2>(globalRangeMin, globalRangeMin),
                schnek::Array<ptrdiff_t,2>(globalRangeMax, globalRangeMax));
      schnek::Range<double, 2> globalDomain(schnek::Array<double,2>(0, -1.5), schnek::Array<double,2>(12.5, 42.2));

      schnek::Grid<double, 2> weights(schnek::Array<ptrdiff_t,2>(0, 0), schnek::Array<ptrdiff_t,2>(weightMax, weightMax));
      weights = 1.0;

      schnek::MpiCartesianDomainDecomposition<2> decomposition(context);

      decomposition.setGlobalRange(globalRange);
      decomposition.setGlobalDomain(globalDomain);
      decomposition.setGlobalWeights(weights);

      decomposition.init();

      // Checking calls
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size[0], context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<0>(), context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<1>(), 2);
      const std::vector<int> cc_dims = context.args_MPI_Cart_create[0].get<2>();
      const std::vector<int> cc_periods = context.args_MPI_Cart_create[0].get<3>();

      BOOST_CHECK_EQUAL(cc_dims.size(), (size_t)2);
      BOOST_CHECK_EQUAL(cc_dims[0], factors[0]);
      BOOST_CHECK_EQUAL(cc_dims[1], factors[1]);

      BOOST_CHECK_EQUAL(cc_periods.size(), (size_t)2);
      BOOST_CHECK_EQUAL(cc_periods[0], 1);
      BOOST_CHECK_EQUAL(cc_periods[1], 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<4>(), 1);

      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank[0], testComm);

      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<0>(), testComm);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<1>(), rank);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<2>(), 2);

      BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), (size_t)2);
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

  schnek::Range<ptrdiff_t, 3> globalRange(schnek::Array<ptrdiff_t,3>(0, -10, -123), schnek::Array<ptrdiff_t,3>(100, 200, -2));
  schnek::Range<double, 3> globalDomain(schnek::Array<double,3>(0, 0, -5.2), schnek::Array<double,3>(12.5, 42.2, 10.1));

  schnek::MpiCartesianDomainDecomposition<3> decomposition(context);
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  decomposition.init();

  // Checking calls
  BOOST_CHECK_EQUAL(context.args_MPI_Comm_size.size(), (size_t)1);
  BOOST_CHECK_EQUAL(context.args_MPI_Comm_size[0], context.commWorld);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create.size(), (size_t)1);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<0>(), context.commWorld);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<1>(), 3);
  const std::vector<int> cc_dims = context.args_MPI_Cart_create[0].get<2>();
  const std::vector<int> cc_periods = context.args_MPI_Cart_create[0].get<3>();

  BOOST_CHECK_EQUAL(cc_dims.size(), (size_t)3);
  BOOST_CHECK_EQUAL(cc_dims[0], 1);
  BOOST_CHECK_EQUAL(cc_dims[1], 1);
  BOOST_CHECK_EQUAL(cc_dims[2], 1);

  BOOST_CHECK_EQUAL(cc_periods.size(), (size_t)3);
  BOOST_CHECK_EQUAL(cc_periods[0], 1);
  BOOST_CHECK_EQUAL(cc_periods[1], 1);
  BOOST_CHECK_EQUAL(cc_periods[2], 1);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<4>(), 1);

  BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank.size(), (size_t)1);
  BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank[0], testComm);

  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords.size(), (size_t)1);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<0>(), testComm);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<1>(), 0);
  BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<2>(), 3);

  BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), (size_t)0);

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

  schnek::Range<ptrdiff_t, 1> expectedRange0(schnek::Array<ptrdiff_t,1>(0), schnek::Array<ptrdiff_t,1>(100));
  schnek::Range<ptrdiff_t, 1> expectedRange1(schnek::Array<ptrdiff_t,1>(-10), schnek::Array<ptrdiff_t,1>(200));
  schnek::Range<ptrdiff_t, 1> expectedRange2(schnek::Array<ptrdiff_t,1>(-123), schnek::Array<ptrdiff_t,1>(-2));
  SCHNEK_CHECK_EQUAL(ranges[0](0), expectedRange0);
  SCHNEK_CHECK_EQUAL(ranges[1](0), expectedRange1);
  SCHNEK_CHECK_EQUAL(ranges[2](0), expectedRange2);
}

BOOST_FIXTURE_TEST_CASE( foreach_single_process_3d, MpiCartesianDomainDecompositionTestFixture )
{
  MPI_Comm testComm = (MPI_Comm)(void*)123;
  std::vector<int> coords(3, 0);
  context.commWorld = (MPI_Comm)(void*)574;
  context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, 1));
  context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, 0));
  context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
  context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));

  using GridType = schnek::Grid<double, 3>;
  using RangeType = schnek::Range<ptrdiff_t, 3>;

  RangeType globalRange(schnek::Array<ptrdiff_t,3>(-2, 5, 7), schnek::Array<ptrdiff_t,3>(4, 9, 11));
  schnek::Range<double, 3> globalDomain(schnek::Array<double,3>(-1.0, 0.0, 1.0), schnek::Array<double,3>(3.0, 4.0, 5.0));

  schnek::MpiCartesianDomainDecomposition<3> decomposition(context);
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  decomposition.init();

  schnek::GridFactory<GridType> factory;
  schnek::GridRegistration registration = decomposition.registerField(factory);

  auto gridContext = decomposition.getGridContext({registration});

  int callCount = 0;
  gridContext.forEach([&](const RangeType &range, GridType &grid) {
    ++callCount;
    SCHNEK_CHECK_EQUAL(range, globalRange);
    SCHNEK_CHECK_EQUAL(grid.getRange(), globalRange);
  });

  BOOST_CHECK_EQUAL(callCount, 1);
}

  BOOST_FIXTURE_TEST_CASE( ghost_exchange_updates_ghost_cells_3d, MpiCartesianDomainDecompositionTestFixture )
  {
    MPI_Comm testComm = (MPI_Comm)(void*)123;
    std::vector<int> coords(3, 0);
    context.commWorld = (MPI_Comm)(void*)574;
    context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, 1));
    context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, 0));
    context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
    context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));

    using FieldType = schnek::Field<double, 3>;
    using RangeType = schnek::Range<ptrdiff_t, 3>;
    using DomainType = schnek::Range<double, 3>;
    using StaggerType = typename FieldType::StaggerType;

    RangeType globalRange(schnek::Array<ptrdiff_t,3>(0, 0, 0), schnek::Array<ptrdiff_t,3>(2, 2, 1));
    DomainType globalDomain(schnek::Array<double,3>(0.0, 0.0, 0.0), schnek::Array<double,3>(1.0, 1.0, 1.0));

    schnek::MpiCartesianDomainDecomposition<3> decomposition(context);
    decomposition.setGlobalRange(globalRange);
    decomposition.setGlobalDomain(globalDomain);
    decomposition.init();

    StaggerType noStagger(false);
    constexpr int ghostCells = 1;
    schnek::GridFactory<FieldType> factory(noStagger, ghostCells);
    schnek::GridRegistration registration = decomposition.registerField(factory);

    auto gridContext = decomposition.getGridContext({registration});
    FieldType *field = nullptr;
    gridContext.forEach([&](const RangeType &, FieldType &localField) { field = &localField; });
    BOOST_REQUIRE(field != nullptr);

  fillField(*field, -7.0);

  FieldType expectedField(*field);
  auto ghostSetup = buildGhostExchangeSetup(*field);
  applyGhostExpectations(expectedField, ghostSetup);
  context.ret_MPI_Sendrecv = ghostSetup.responses;

    std::array<int, 3> prevRanks{{30, 40, 50}};
    std::array<int, 3> nextRanks{{31, 41, 51}};
    for (size_t dim = 0; dim < prevRanks.size(); ++dim) {
      context.ret_MPI_Cart_shift.push_back(boost::make_tuple(MPI_SUCCESS, prevRanks[dim], nextRanks[dim]));
    }

    exchangeRegistration<3>(decomposition, registration, false);

    BOOST_CHECK_EQUAL(context.args_MPI_Cart_shift.size(), prevRanks.size());
    for (size_t dim = 0; dim < prevRanks.size(); ++dim) {
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_shift[dim].get<1>(), static_cast<int>(dim));
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_shift[dim].get<2>(), 1);
    }

    BOOST_REQUIRE_EQUAL(context.args_MPI_Sendrecv.size(), ghostSetup.expectations.size());
    for (size_t i = 0; i < ghostSetup.expectations.size(); ++i) {
      const auto &call = context.args_MPI_Sendrecv[i];
      BOOST_CHECK(call.recvBufferPresent);
      BOOST_CHECK_EQUAL(call.recvCount, static_cast<int>(ghostSetup.expectations[i].values.size()));
    }

    for (size_t dim = 0; dim < prevRanks.size(); ++dim) {
      size_t baseIdx = 2 * dim;
      const auto &lowerCall = context.args_MPI_Sendrecv[baseIdx];
      BOOST_CHECK_EQUAL(lowerCall.dest, nextRanks[dim]);
      BOOST_CHECK_EQUAL(lowerCall.source, prevRanks[dim]);
      const auto &upperCall = context.args_MPI_Sendrecv[baseIdx + 1];
      BOOST_CHECK_EQUAL(upperCall.dest, prevRanks[dim]);
      BOOST_CHECK_EQUAL(upperCall.source, nextRanks[dim]);
    }

    typename FieldType::RangeType verificationRange(field->getLo(), field->getHi());
    for (auto it = verificationRange.begin(); it != verificationRange.end(); ++it) {
      BOOST_CHECK_EQUAL((*field)[*it], expectedField[*it]);
    }
  }

  BOOST_FIXTURE_TEST_CASE( ghost_accumulate_updates_ghost_and_inner_cells_3d, MpiCartesianDomainDecompositionTestFixture )
  {
    MPI_Comm testComm = (MPI_Comm)(void*)123;
    std::vector<int> coords(3, 0);
    context.commWorld = (MPI_Comm)(void*)574;
    context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, 1));
    context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, 0));
    context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
    context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));

    using FieldType = schnek::Field<double, 3>;
    using RangeType = schnek::Range<ptrdiff_t, 3>;
    using DomainType = schnek::Range<double, 3>;
    using StaggerType = typename FieldType::StaggerType;

    RangeType globalRange(schnek::Array<ptrdiff_t,3>(0, 0, 0), schnek::Array<ptrdiff_t,3>(2, 2, 1));
    DomainType globalDomain(schnek::Array<double,3>(0.0, 0.0, 0.0), schnek::Array<double,3>(1.0, 1.0, 1.0));

    schnek::MpiCartesianDomainDecomposition<3> decomposition(context);
    decomposition.setGlobalRange(globalRange);
    decomposition.setGlobalDomain(globalDomain);
    decomposition.init();

    StaggerType noStagger(false);
    constexpr int ghostCells = 1;
    schnek::GridFactory<FieldType> factory(noStagger, ghostCells);
    schnek::GridRegistration registration = decomposition.registerField(factory);

    auto gridContext = decomposition.getGridContext({registration});
    FieldType *field = nullptr;
    gridContext.forEach([&](const RangeType &, FieldType &localField) { field = &localField; });
    BOOST_REQUIRE(field != nullptr);

    fillField(*field, -2.0);

    auto accumulateSetup = buildGhostAccumulateSetup(*field);
    context.ret_MPI_Sendrecv = accumulateSetup.responses;

    std::array<int, 3> prevRanks{{30, 40, 50}};
    std::array<int, 3> nextRanks{{31, 41, 51}};
    for (size_t dim = 0; dim < prevRanks.size(); ++dim) {
      context.ret_MPI_Cart_shift.push_back(boost::make_tuple(MPI_SUCCESS, prevRanks[dim], nextRanks[dim]));
    }

    accumulateRegistration<3>(decomposition, registration, false);

    BOOST_CHECK_EQUAL(context.args_MPI_Cart_shift.size(), prevRanks.size());
    for (size_t dim = 0; dim < prevRanks.size(); ++dim) {
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_shift[dim].get<1>(), static_cast<int>(dim));
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_shift[dim].get<2>(), 1);
    }

    BOOST_REQUIRE_EQUAL(context.args_MPI_Sendrecv.size(), accumulateSetup.recvCounts.size());
    for (size_t i = 0; i < accumulateSetup.recvCounts.size(); ++i) {
      const auto &call = context.args_MPI_Sendrecv[i];
      BOOST_CHECK(call.recvBufferPresent);
      BOOST_CHECK_EQUAL(call.recvCount, accumulateSetup.recvCounts[i]);
    }

    for (size_t dim = 0; dim < prevRanks.size(); ++dim) {
      size_t baseIdx = 4 * dim;
      const auto &lowerInCall = context.args_MPI_Sendrecv[baseIdx];
      BOOST_CHECK_EQUAL(lowerInCall.dest, nextRanks[dim]);
      BOOST_CHECK_EQUAL(lowerInCall.source, prevRanks[dim]);
      const auto &lowerBackCall = context.args_MPI_Sendrecv[baseIdx + 1];
      BOOST_CHECK_EQUAL(lowerBackCall.dest, prevRanks[dim]);
      BOOST_CHECK_EQUAL(lowerBackCall.source, nextRanks[dim]);
      const auto &upperInCall = context.args_MPI_Sendrecv[baseIdx + 2];
      BOOST_CHECK_EQUAL(upperInCall.dest, prevRanks[dim]);
      BOOST_CHECK_EQUAL(upperInCall.source, nextRanks[dim]);
      const auto &upperBackCall = context.args_MPI_Sendrecv[baseIdx + 3];
      BOOST_CHECK_EQUAL(upperBackCall.dest, nextRanks[dim]);
      BOOST_CHECK_EQUAL(upperBackCall.source, prevRanks[dim]);
    }

    typename FieldType::RangeType verificationRange(field->getLo(), field->getHi());
    for (auto it = verificationRange.begin(); it != verificationRange.end(); ++it) {
      BOOST_CHECK_EQUAL((*field)[*it], accumulateSetup.expectedField[*it]);
    }
  }

BOOST_FIXTURE_TEST_CASE( multi_process_3d, MpiCartesianDomainDecompositionTestFixture )
{
  std::vector<int> numProcsArray{2, 5, 7, 127, 128, 129, 1023, 4000, 25000, 128000, 879484};
  std::vector<int> rankArray{1, 2, 4, 120, 126, 500, 1022, 3999, 24999, 127999, 879483};

  std::vector<int> globalRangeMin(3, -5000);
  std::vector<int> globalRangeMax(3,  5000);

  boost::timer::progress_display show_progress(numProcsArray.size() * rankArray.size());

  for (int numProcs: numProcsArray)
  {
    for (int rank: rankArray)
    {
      ++show_progress;
      if (rank >= numProcs) continue;
      std::vector<size_t> factors;
      std::vector<size_t> weights(3, 10001);
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

      schnek::Range<ptrdiff_t, 3> globalRange
        = schnek::Range<ptrdiff_t, 3>(
                schnek::Array<ptrdiff_t,3>(globalRangeMin[0], globalRangeMin[1], globalRangeMin[2]),
                schnek::Array<ptrdiff_t,3>(globalRangeMax[0], globalRangeMax[1], globalRangeMax[2]));
      schnek::Range<double, 3> globalDomain(schnek::Array<double,3>(0, 0, -5.2), schnek::Array<double,3>(12.5, 42.2, 10.1));

      schnek::MpiCartesianDomainDecomposition<3> decomposition(context);
      decomposition.setGlobalRange(globalRange);
      decomposition.setGlobalDomain(globalDomain);

      decomposition.init();

      // Checking calls
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size[0], context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<0>(), context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<1>(), 3);
      const std::vector<int> cc_dims = context.args_MPI_Cart_create[0].get<2>();
      const std::vector<int> cc_periods = context.args_MPI_Cart_create[0].get<3>();

      BOOST_CHECK_EQUAL(cc_dims.size(), (size_t)3);
      BOOST_CHECK_EQUAL(cc_dims[0], factors[0]);
      BOOST_CHECK_EQUAL(cc_dims[1], factors[1]);
      BOOST_CHECK_EQUAL(cc_dims[2], factors[2]);

      BOOST_CHECK_EQUAL(cc_periods.size(), (size_t)3);
      BOOST_CHECK_EQUAL(cc_periods[0], 1);
      BOOST_CHECK_EQUAL(cc_periods[1], 1);
      BOOST_CHECK_EQUAL(cc_periods[2], 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<4>(), 1);

      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank[0], testComm);

      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<0>(), testComm);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<1>(), rank);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<2>(), 3);

      BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), (size_t)0);

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
          ptrdiff_t lo = globalRangeMin[d] + ((globalRangeMax[d] - globalRangeMin[d] + 1)*long(r))/factors[d];
          ptrdiff_t hi = globalRangeMin[d] + ((globalRangeMax[d] - globalRangeMin[d] + 1)*long(r + 1))/factors[d] - 1;

          schnek::Range<ptrdiff_t, 1> expectedRange = schnek::Range<ptrdiff_t, 1>(schnek::Array<ptrdiff_t,1>(lo), schnek::Array<ptrdiff_t,1>(hi));
          SCHNEK_CHECK_EQUAL(ranges[d](r), expectedRange);
        }
      }
    }
  }
}

BOOST_FIXTURE_TEST_CASE( foreach_multi_process_3d, MpiCartesianDomainDecompositionTestFixture )
{
  const int numProcs = 8;
  const int rank = 5;

  std::vector<size_t> factors;
  std::vector<size_t> weights;
  weights += 6, 4, 2;
  schnek::equalFactors(numProcs, 3, factors, weights);

  MPI_Comm testComm = (MPI_Comm)(void*)123;
  std::vector<int> coords(3);
  coords[0] = rank / (factors[1] * factors[2]);
  coords[1] = (rank / factors[2]) % factors[1];
  coords[2] = rank % factors[2];

  context.commWorld = (MPI_Comm)(void*)574;
  context.ret_MPI_Comm_size.push_back(boost::tuple<int, int>(MPI_SUCCESS, numProcs));
  context.ret_MPI_Comm_rank.push_back(boost::tuple<int, int>(MPI_SUCCESS, rank));
  context.ret_MPI_Cart_create.push_back(boost::tuple<int, MPI_Comm>(MPI_SUCCESS, testComm));
  context.ret_MPI_Cart_coords.push_back(boost::tuple<int, std::vector<int>>(MPI_SUCCESS, coords));

  using GridType = schnek::Grid<double, 3>;
  using RangeType = schnek::Range<ptrdiff_t, 3>;

  RangeType globalRange(schnek::Array<ptrdiff_t,3>(0, 0, 0), schnek::Array<ptrdiff_t,3>(5, 3, 1));
  schnek::Range<double, 3> globalDomain(schnek::Array<double,3>(0.0, 0.0, 0.0), schnek::Array<double,3>(1.0, 1.0, 1.0));

  schnek::MpiCartesianDomainDecomposition<3> decomposition(context);
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  decomposition.init();

  schnek::GridFactory<GridType> factory;
  schnek::GridRegistration registration = decomposition.registerField(factory);

  auto gridContext = decomposition.getGridContext({registration});

  const auto globalLo = globalRange.getLo();
  const auto globalHi = globalRange.getHi();
  const ptrdiff_t lo0 = globalLo[0] + ((globalHi[0] - globalLo[0] + 1) * coords[0]) / factors[0];
  const ptrdiff_t hi0 = globalLo[0] + ((globalHi[0] - globalLo[0] + 1) * (coords[0] + 1)) / factors[0] - 1;
  const ptrdiff_t lo1 = globalLo[1] + ((globalHi[1] - globalLo[1] + 1) * coords[1]) / factors[1];
  const ptrdiff_t hi1 = globalLo[1] + ((globalHi[1] - globalLo[1] + 1) * (coords[1] + 1)) / factors[1] - 1;
  const ptrdiff_t lo2 = globalLo[2] + ((globalHi[2] - globalLo[2] + 1) * coords[2]) / factors[2];
  const ptrdiff_t hi2 = globalLo[2] + ((globalHi[2] - globalLo[2] + 1) * (coords[2] + 1)) / factors[2] - 1;
  RangeType expectedRange{
      schnek::Array<ptrdiff_t,3>{lo0, lo1, lo2}, schnek::Array<ptrdiff_t,3>{hi0, hi1, hi2}
  };

  int callCount = 0;
  gridContext.forEach([&](const RangeType &range, GridType &grid) {
    ++callCount;
    SCHNEK_CHECK_EQUAL(range, expectedRange);
    SCHNEK_CHECK_EQUAL(grid.getRange(), expectedRange);
  });

  BOOST_CHECK_EQUAL(callCount, 1);
}

BOOST_FIXTURE_TEST_CASE( multi_process_3d_global_master, MpiCartesianDomainDecompositionTestFixture )
{
  std::vector<int> numProcsArray;
  numProcsArray += 2, 5, 7, 128, 1024, 4000, 25000, 128000, 1093950;

  int globalRangeMin =  -3200;
  int globalRangeMax =  3199;
  boost::timer::progress_display show_progress(numProcsArray.size() * 5);

  for (int numProcs: numProcsArray)
  {
    for (int resolution = 16; resolution <= 256; resolution *= 2)
    {
      ++show_progress;
      std::vector<size_t> factors;
      std::vector<size_t> box(3, (globalRangeMax - globalRangeMin + 1)/resolution);

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

      schnek::Range<ptrdiff_t, 3> globalRange
        = schnek::Range<ptrdiff_t, 3>(
                schnek::Array<ptrdiff_t,3>(globalRangeMin, globalRangeMin, globalRangeMin),
                schnek::Array<ptrdiff_t,3>(globalRangeMax, globalRangeMax, globalRangeMax));
      schnek::Range<double, 3> globalDomain(schnek::Array<double,3>(0, 0, -5.2), schnek::Array<double,3>(12.5, 42.2, 10.1));

      schnek::Grid<double, 3> weights(schnek::Array<ptrdiff_t,3>(0, 0, 0), schnek::Array<ptrdiff_t,3>(weightMax, weightMax, weightMax));
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
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size[0], context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<0>(), context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<1>(), 3);
      const std::vector<int> cc_dims = context.args_MPI_Cart_create[0].get<2>();
      const std::vector<int> cc_periods = context.args_MPI_Cart_create[0].get<3>();

      BOOST_CHECK_EQUAL(cc_dims.size(), (size_t)3);
      BOOST_CHECK_EQUAL(cc_dims[0], factors[0]);
      BOOST_CHECK_EQUAL(cc_dims[1], factors[1]);
      BOOST_CHECK_EQUAL(cc_dims[2], factors[2]);

      BOOST_CHECK_EQUAL(cc_periods.size(), (size_t)3);
      BOOST_CHECK_EQUAL(cc_periods[0], 1);
      BOOST_CHECK_EQUAL(cc_periods[1], 1);
      BOOST_CHECK_EQUAL(cc_periods[2], 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<4>(), 1);

      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank[0], testComm);

      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<0>(), testComm);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<1>(), 0);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<2>(), 3);

      BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), (size_t)3);
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
          schnek::Range<ptrdiff_t, 1> expectedRangeA = schnek::Range<ptrdiff_t, 1>(schnek::Array<ptrdiff_t,1>(lo), schnek::Array<ptrdiff_t,1>(hi));
          schnek::Range<ptrdiff_t, 1> expectedRangeB = schnek::Range<ptrdiff_t, 1>(schnek::Array<ptrdiff_t,1>(lo + resolution), schnek::Array<ptrdiff_t,1>(hi + resolution));
          schnek::Range<ptrdiff_t, 1> foundRange = ranges[r](i);
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
  boost::timer::progress_display show_progress(numProcsArray.size() * rankArray.size());

  for (int numProcs: numProcsArray)
  {
    for (int rank: rankArray)
    {
      ++show_progress;
      if (rank >= numProcs) continue;

      std::vector<size_t> factors;
      std::vector<size_t> box(3, (globalRangeMax - globalRangeMin + 1)/16);
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

      schnek::Range<ptrdiff_t, 3> globalRange
        = schnek::Range<ptrdiff_t, 3>(
                schnek::Array<ptrdiff_t,3>(globalRangeMin, globalRangeMin, globalRangeMin),
                schnek::Array<ptrdiff_t,3>(globalRangeMax, globalRangeMax, globalRangeMax));
      schnek::Range<double, 3> globalDomain(schnek::Array<double,3>(0, 0, -5.2), schnek::Array<double,3>(12.5, 42.2, 10.1));

      schnek::Grid<double, 3> weights(schnek::Array<ptrdiff_t,3>(0, 0, 0), schnek::Array<ptrdiff_t,3>(weightMax, weightMax, weightMax));
      weights = 1.0;

      schnek::MpiCartesianDomainDecomposition<3> decomposition(context);

      decomposition.setGlobalRange(globalRange);
      decomposition.setGlobalDomain(globalDomain);
      decomposition.setGlobalWeights(weights);

      decomposition.init();

      // Checking calls
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_size[0], context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<0>(), context.commWorld);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<1>(), 3);
      const std::vector<int> cc_dims = context.args_MPI_Cart_create[0].get<2>();
      const std::vector<int> cc_periods = context.args_MPI_Cart_create[0].get<3>();

      BOOST_CHECK_EQUAL(cc_dims.size(), (size_t)3);
      BOOST_CHECK_EQUAL(cc_dims[0], factors[0]);
      BOOST_CHECK_EQUAL(cc_dims[1], factors[1]);
      BOOST_CHECK_EQUAL(cc_dims[2], factors[2]);

      BOOST_CHECK_EQUAL(cc_periods.size(), (size_t)3);
      BOOST_CHECK_EQUAL(cc_periods[0], 1);
      BOOST_CHECK_EQUAL(cc_periods[1], 1);
      BOOST_CHECK_EQUAL(cc_periods[2], 1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_create[0].get<4>(), 1);

      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Comm_rank[0], testComm);

      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords.size(), (size_t)1);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<0>(), testComm);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<1>(), rank);
      BOOST_CHECK_EQUAL(context.args_MPI_Cart_coords[0].get<2>(), 3);

      BOOST_CHECK_EQUAL(context.args_MPI_Bcast.size(), (size_t)3);
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
