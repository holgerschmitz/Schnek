/*
 *  test_serial_decomposition.cpp
 *
 *  Created on: 8 Oct 2020
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 */

#include "../utility.hpp"
#include <grid/range.hpp>
#include <grid/grid.hpp>
#include <grid/field.hpp>
#include <util/factor.hpp>

#include <decomposition/serial_decomposition.hpp>

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
        result.expectations.push_back({ghostRange, values});
      }

      ptrdiff_t upperHalo = gridHi[dim] - innerHi[dim];
      if (upperHalo > 0) {
        IndexType lo = gridLo;
        IndexType hi = gridHi;
        lo[dim] = gridHi[dim] - upperHalo + 1;
        RangeType ghostRange = makeRange(lo, hi);
        auto values = makeValues(ghostRange);
        result.expectations.push_back({ghostRange, values});
      }
    }

    return result;
  }

  template<typename FieldType>
  struct GhostAccumulateSetupResult {
      FieldType expectedField;
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
        addInto(*loGhostRange, values);
      }

      size_t recvBackLowerCount = hiSourceRange ? rangeVolume(*hiSourceRange) : 0;
      if (recvBackLowerCount > 0) {
        auto values = nextValues(*hiSourceRange);
        assignInto(*hiSourceRange, values);
      }

      // == upper side ==
      size_t recvUpperCount = hiGhostRange ? rangeVolume(*hiGhostRange) : 0;
      if (recvUpperCount > 0) {
        auto values = nextValues(*hiGhostRange);
        addInto(*hiGhostRange, values);
      }

      size_t recvBackUpperCount = loSourceRange ? rangeVolume(*loSourceRange) : 0;
      if (recvBackUpperCount > 0) {
        auto values = nextValues(*loSourceRange);
        assignInto(*loSourceRange, values);
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
      schnek::SerialDomainDecomposition<Rank> &decomposition,
      const schnek::GridRegistration &registration,
      bool useFieldInfo = true
  ) {
    auto &base = static_cast<schnek::DomainDecomposition<Rank> &>(decomposition);
    base.exchange(registration, useFieldInfo);
  }

  template<size_t Rank>
  void accumulateRegistration(
      schnek::SerialDomainDecomposition<Rank> &decomposition,
      const schnek::GridRegistration &registration,
      bool useFieldInfo = true
  ) {
    auto &base = static_cast<schnek::DomainDecomposition<Rank> &>(decomposition);
    base.accumulate(registration, useFieldInfo);
  }

}  // namespace

//
// run only these tests:
// ./schnek_tests --log_level=test_suite --run_test=serial_domain_decomposition

BOOST_AUTO_TEST_SUITE( serial_domain_decomposition )

// ==========================================================================
// 1 dimensional
// ==========================================================================

BOOST_AUTO_TEST_CASE( init_1d )
{
  schnek::Range<ptrdiff_t, 1> globalRange(schnek::Array<ptrdiff_t,1>(0), schnek::Array<ptrdiff_t,1>(100));
  schnek::Range<double, 1> globalDomain(schnek::Array<double,1>(0), schnek::Array<double,1>(12.5));

  schnek::SerialDomainDecomposition<1> decomposition;
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  decomposition.init();

  BOOST_CHECK_EQUAL(decomposition.master(), true);
  BOOST_CHECK_EQUAL(decomposition.numProcs(), 1);
  BOOST_CHECK_EQUAL(decomposition.getUniqueId(), 0);
}

BOOST_AUTO_TEST_CASE( reductions_use_allreduce )
{
  schnek::Range<ptrdiff_t, 1> globalRange(schnek::Array<ptrdiff_t,1>(0), schnek::Array<ptrdiff_t,1>(3));
  schnek::Range<double, 1> globalDomain(schnek::Array<double,1>(0.0), schnek::Array<double,1>(1.0));

  schnek::SerialDomainDecomposition<1> decomposition;
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);
  decomposition.init();

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
  BOOST_CHECK_CLOSE(sumDouble, 2.5, 1e-12);
  BOOST_CHECK_CLOSE(maxDouble, 1.5, 1e-12);
  BOOST_CHECK_CLOSE(minDouble, 3.0, 1e-12);

  BOOST_CHECK_EQUAL(avgInt, 5);
  BOOST_CHECK_EQUAL(sumInt, 5);
  BOOST_CHECK_EQUAL(maxInt, 9);

  BOOST_CHECK_EQUAL(avgLong, 7l);
  BOOST_CHECK_EQUAL(sumLong, 10l);
  BOOST_CHECK_EQUAL(maxLong, 11l);
  BOOST_CHECK_EQUAL(minLong, 6l);
}

BOOST_AUTO_TEST_CASE( foreach_1d )
{
  using GridType = schnek::Grid<double, 1>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;

  RangeType globalRange(schnek::Array<ptrdiff_t,1>(0), schnek::Array<ptrdiff_t,1>(42));
  schnek::Range<double, 1> globalDomain(schnek::Array<double,1>(0), schnek::Array<double,1>(10));

  schnek::SerialDomainDecomposition<1> decomposition;
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

BOOST_AUTO_TEST_CASE( foreach_1d_subrange )
{
  using GridType = schnek::Grid<double, 1>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;

  RangeType globalRange(schnek::Array<ptrdiff_t,1>(0), schnek::Array<ptrdiff_t,1>(42));
  schnek::Range<double, 1> globalDomain(schnek::Array<double,1>(0), schnek::Array<double,1>(10));

  schnek::SerialDomainDecomposition<1> decomposition;
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  decomposition.init();

  RangeType subRange(schnek::Array<ptrdiff_t,1>(10), schnek::Array<ptrdiff_t,1>(20));

  schnek::GridFactory<GridType> factory;
  schnek::GridRegistration registration = decomposition.registerField(factory, subRange);

  auto gridContext = decomposition.getGridContext({registration});

  int callCount = 0;
  gridContext.forEach([&](const RangeType &range, GridType &grid) {
    ++callCount;
    SCHNEK_CHECK_EQUAL(range, subRange);
    SCHNEK_CHECK_EQUAL(grid.getRange(), subRange);
  });

  BOOST_CHECK_EQUAL(callCount, 1);
}

BOOST_AUTO_TEST_CASE( foreach_1d_subrange_no_overlap )
{
  using GridType = schnek::Grid<double, 1>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;

  RangeType globalRange(schnek::Array<ptrdiff_t,1>(0), schnek::Array<ptrdiff_t,1>(42));
  schnek::Range<double, 1> globalDomain(schnek::Array<double,1>(0), schnek::Array<double,1>(10));

  schnek::SerialDomainDecomposition<1> decomposition;
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  decomposition.init();

  RangeType subRange(schnek::Array<ptrdiff_t,1>(100), schnek::Array<ptrdiff_t,1>(120));

  schnek::GridFactory<GridType> factory;
  schnek::GridRegistration registration = decomposition.registerField(factory, subRange);

  auto gridContext = decomposition.getGridContext({registration});

  int callCount = 0;
  gridContext.forEach([&](const RangeType &, GridType &) { ++callCount; });

  BOOST_CHECK_EQUAL(callCount, 0);
}

BOOST_AUTO_TEST_CASE( ghost_exchange_updates_ghost_cells_1d )
{
  using FieldType = schnek::Field<double, 1>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;
  using DomainType = schnek::Range<double, 1>;
  using StaggerType = typename FieldType::StaggerType;

  RangeType globalRange(schnek::Array<ptrdiff_t,1>(0), schnek::Array<ptrdiff_t,1>(3));
  DomainType globalDomain(schnek::Array<double,1>(0.0), schnek::Array<double,1>(1.0));

  schnek::SerialDomainDecomposition<1> decomposition;
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

  exchangeRegistration<1>(decomposition, registration, false);  
  
  typename FieldType::RangeType verificationRange(field->getLo(), field->getHi());
  for (auto it = verificationRange.begin(); it != verificationRange.end(); ++it) {
    BOOST_CHECK_EQUAL((*field)[*it], expectedField[*it]);
  }
}

  BOOST_AUTO_TEST_CASE( ghost_accumulate_updates_ghost_and_inner_cells_1d )
  {
    using FieldType = schnek::Field<double, 1>;
    using RangeType = schnek::Range<ptrdiff_t, 1>;
    using DomainType = schnek::Range<double, 1>;
    using StaggerType = typename FieldType::StaggerType;

    RangeType globalRange(schnek::Array<ptrdiff_t,1>(0), schnek::Array<ptrdiff_t,1>(3));
    DomainType globalDomain(schnek::Array<double,1>(0.0), schnek::Array<double,1>(1.0));

    schnek::SerialDomainDecomposition<1> decomposition;
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

    accumulateRegistration<1>(decomposition, registration, false);

    typename FieldType::RangeType verificationRange(field->getLo(), field->getHi());
    for (auto it = verificationRange.begin(); it != verificationRange.end(); ++it) {
      BOOST_CHECK_EQUAL((*field)[*it], accumulateSetup.expectedField[*it]);
    }
  }

// ==========================================================================
// 2 dimensional
// ==========================================================================


BOOST_AUTO_TEST_CASE( init_2d )
{
  schnek::Range<ptrdiff_t, 2> globalRange(schnek::Array<ptrdiff_t,2>(0, -10), schnek::Array<ptrdiff_t,2>(100, 200));
  schnek::Range<double, 2> globalDomain(schnek::Array<double,2>(0, 0), schnek::Array<double,2>(12.5, 42.2));

  schnek::SerialDomainDecomposition<2> decomposition;
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  decomposition.init();

  BOOST_CHECK_EQUAL(decomposition.master(), true);
  BOOST_CHECK_EQUAL(decomposition.numProcs(), 1);
  BOOST_CHECK_EQUAL(decomposition.getUniqueId(), 0);
}

BOOST_AUTO_TEST_CASE( foreach_2d )
{
  using GridType = schnek::Grid<double, 2>;
  using RangeType = schnek::Range<ptrdiff_t, 2>;

  RangeType globalRange(schnek::Array<ptrdiff_t,2>(1, -3), schnek::Array<ptrdiff_t,2>(8, 4));
  schnek::Range<double, 2> globalDomain(schnek::Array<double,2>(0.0, -1.0), schnek::Array<double,2>(2.0, 3.5));

  schnek::SerialDomainDecomposition<2> decomposition;
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

  BOOST_AUTO_TEST_CASE( ghost_exchange_updates_ghost_cells_2d )
  {
    using FieldType = schnek::Field<double, 2>;
    using RangeType = schnek::Range<ptrdiff_t, 2>;
    using DomainType = schnek::Range<double, 2>;
    using StaggerType = typename FieldType::StaggerType;

    RangeType globalRange(schnek::Array<ptrdiff_t,2>(0, 0), schnek::Array<ptrdiff_t,2>(3, 2));
    DomainType globalDomain(schnek::Array<double,2>(0.0, 0.0), schnek::Array<double,2>(1.0, 1.0));

    schnek::SerialDomainDecomposition<2> decomposition;
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

    exchangeRegistration<2>(decomposition, registration, false);

    typename FieldType::RangeType verificationRange(field->getLo(), field->getHi());
    for (auto it = verificationRange.begin(); it != verificationRange.end(); ++it) {
      BOOST_CHECK_EQUAL((*field)[*it], expectedField[*it]);
    }
  }

  BOOST_AUTO_TEST_CASE( ghost_accumulate_updates_ghost_and_inner_cells_2d )
  {
    using FieldType = schnek::Field<double, 2>;
    using RangeType = schnek::Range<ptrdiff_t, 2>;
    using DomainType = schnek::Range<double, 2>;
    using StaggerType = typename FieldType::StaggerType;

    RangeType globalRange(schnek::Array<ptrdiff_t,2>(0, 0), schnek::Array<ptrdiff_t,2>(3, 2));
    DomainType globalDomain(schnek::Array<double,2>(0.0, 0.0), schnek::Array<double,2>(1.0, 1.0));

    schnek::SerialDomainDecomposition<2> decomposition;
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

    accumulateRegistration<2>(decomposition, registration, false);

    typename FieldType::RangeType verificationRange(field->getLo(), field->getHi());
    for (auto it = verificationRange.begin(); it != verificationRange.end(); ++it) {
      BOOST_CHECK_EQUAL((*field)[*it], accumulateSetup.expectedField[*it]);
    }
  }

// ==========================================================================
// 3 dimensional
// ==========================================================================


BOOST_AUTO_TEST_CASE( init_3d )
{
  schnek::Range<ptrdiff_t, 3> globalRange(schnek::Array<ptrdiff_t,3>(0, -10, -123), schnek::Array<ptrdiff_t,3>(100, 200, -2));
  schnek::Range<double, 3> globalDomain(schnek::Array<double,3>(0, 0, -5.2), schnek::Array<double,3>(12.5, 42.2, 10.1));

  schnek::SerialDomainDecomposition<3> decomposition;
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  decomposition.init();

  BOOST_CHECK_EQUAL(decomposition.master(), true);
  BOOST_CHECK_EQUAL(decomposition.numProcs(), 1);
  BOOST_CHECK_EQUAL(decomposition.getUniqueId(), 0);
}

BOOST_AUTO_TEST_CASE( foreach_3d )
{
  using GridType = schnek::Grid<double, 3>;
  using RangeType = schnek::Range<ptrdiff_t, 3>;

  RangeType globalRange(schnek::Array<ptrdiff_t,3>(-2, 5, 7), schnek::Array<ptrdiff_t,3>(4, 9, 11));
  schnek::Range<double, 3> globalDomain(schnek::Array<double,3>(-1.0, 0.0, 1.0), schnek::Array<double,3>(3.0, 4.0, 5.0));

  schnek::SerialDomainDecomposition<3> decomposition;
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

  BOOST_AUTO_TEST_CASE( ghost_exchange_updates_ghost_cells_3d )
  {
    using FieldType = schnek::Field<double, 3>;
    using RangeType = schnek::Range<ptrdiff_t, 3>;
    using DomainType = schnek::Range<double, 3>;
    using StaggerType = typename FieldType::StaggerType;

    RangeType globalRange(schnek::Array<ptrdiff_t,3>(0, 0, 0), schnek::Array<ptrdiff_t,3>(2, 2, 1));
    DomainType globalDomain(schnek::Array<double,3>(0.0, 0.0, 0.0), schnek::Array<double,3>(1.0, 1.0, 1.0));

    schnek::SerialDomainDecomposition<3> decomposition;
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

    exchangeRegistration<3>(decomposition, registration, false);

    typename FieldType::RangeType verificationRange(field->getLo(), field->getHi());
    for (auto it = verificationRange.begin(); it != verificationRange.end(); ++it) {
      BOOST_CHECK_EQUAL((*field)[*it], expectedField[*it]);
    }
  }

  BOOST_AUTO_TEST_CASE( ghost_accumulate_updates_ghost_and_inner_cells_3d )
  {
    using FieldType = schnek::Field<double, 3>;
    using RangeType = schnek::Range<ptrdiff_t, 3>;
    using DomainType = schnek::Range<double, 3>;
    using StaggerType = typename FieldType::StaggerType;

    RangeType globalRange(schnek::Array<ptrdiff_t,3>(0, 0, 0), schnek::Array<ptrdiff_t,3>(2, 2, 1));
    DomainType globalDomain(schnek::Array<double,3>(0.0, 0.0, 0.0), schnek::Array<double,3>(1.0, 1.0, 1.0));

    schnek::SerialDomainDecomposition<3> decomposition;
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

    accumulateRegistration<3>(decomposition, registration, false);

    typename FieldType::RangeType verificationRange(field->getLo(), field->getHi());
    for (auto it = verificationRange.begin(); it != verificationRange.end(); ++it) {
      BOOST_CHECK_EQUAL((*field)[*it], accumulateSetup.expectedField[*it]);
    }
  }

BOOST_AUTO_TEST_SUITE_END()
