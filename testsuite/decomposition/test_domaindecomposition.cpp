/*
 *  test_domaindecomposition.cpp
 *
 *  Created on: 3 Dec 2024
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 */

#include "../utility.hpp"

#include <decomposition/domaindecomposition.hpp>
#include <grid/grid.hpp>
#include <grid/field.hpp>

#include <boost/test/unit_test.hpp>

//
// run only these tests:
// ./schnek_tests --log_level=test_suite --run_test=domain_decomposition

/**
 * @brief Mock implementation of DomainDecomposition for testing
 * 
 * This mock exposes the protected addLocalRange method so that tests
 * can add local ranges and verify the grid registration and forEach functionality.
 */
template<size_t rank, template<size_t> class CheckingPolicy = schnek::ArrayNoArgCheck>
class MockDomainDecomposition : public schnek::DomainDecomposition<rank, CheckingPolicy>
{
  public:
    using Base = schnek::DomainDecomposition<rank, CheckingPolicy>;
    using RangeType = typename Base::RangeType;
    using DomainType = typename Base::DomainType;
    using RegistrationVariant = typename Base::RegistrationVariant;

    MockDomainDecomposition() : Base() {}

    // Implement abstract methods with default/empty implementations
    void init() override {}
    void balanceLoad() override {}
    int getUniqueId() const override { return 0; }
    bool master() const override { return true; }
    int numProcs() const override { return 1; }
    void exchange(const schnek::internal::pGridWrapper &wrapper, bool useFieldInfo) override {
      (void)wrapper;
      (void)useFieldInfo;
    }

    void accumulate(const schnek::internal::pGridWrapper &wrapper, bool useFieldInfo) override {
      (void)wrapper;
      (void)useFieldInfo;
    }

    double avgReduce(double value) const override { return value; }
    int avgReduce(int value) const override { return value; }
    long avgReduce(long value) const override { return value; }

    double sumReduce(double value) const override { return value; }
    int sumReduce(int value) const override { return value; }
    long sumReduce(long value) const override { return value; }

    double maxReduce(double value) const override { return value; }
    int maxReduce(int value) const override { return value; }
    long maxReduce(long value) const override { return value; }

    double minReduce(double value) const override { return value; }
    int minReduce(int value) const override { return value; }
    long minReduce(long value) const override { return value; }

    // Expose the protected addLocalRange method for testing
    void testAddLocalRange(RangeType range, DomainType domain)
    {
      this->addLocalRange(range, domain);
    }

    // Expose the protected addLocalIterationRange method for testing
    void testAddLocalIterationRange(RangeType range)
    {
      this->addLocalIterationRange(range);
    }

    template<class GridType>
    schnek::GridRegistration registerField(schnek::GridFactory<GridType> &factory) {
      return this->registerFieldImpl(factory);
    }

    template<class GridType>
    schnek::GridRegistration registerField(schnek::GridFactory<GridType> &factory, const RangeType &subRange) {
      return this->registerFieldImpl(factory, subRange);
    }

    template<class GridType>
    typename Base::template ProjectedRegistration<GridType::Rank> registerProjection(
        schnek::GridFactory<GridType> &factory,
        const std::array<size_t, GridType::Rank> &axes
    ) {
      return this->registerFieldProjectionImpl(factory, axes);
    }
};

BOOST_AUTO_TEST_SUITE( domain_decomposition )

// ==========================================================================
// Basic grid registration tests
// ==========================================================================

BOOST_AUTO_TEST_CASE( register_single_grid_1d )
{
  using GridType = schnek::Grid<double, 1>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;
  using DomainType = schnek::Range<double, 1>;
  using IndexType = schnek::Array<ptrdiff_t, 1>;
  using DomainLimitType = schnek::Array<double, 1>;

  MockDomainDecomposition<1> decomposition;

  // Set up global range and domain
  RangeType globalRange(IndexType(0), IndexType(100));
  DomainType globalDomain(DomainLimitType(0.0), DomainLimitType(10.0));
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  // Create a grid factory and register it
  schnek::GridFactory<GridType> factory;
  schnek::GridRegistration reg = decomposition.registerField(factory);

  // Check that registration has a valid id
  BOOST_CHECK_GE(reg.id, 0);
}

BOOST_AUTO_TEST_CASE( register_multiple_grids_2d )
{
  using GridType = schnek::Grid<double, 2>;
  using RangeType = schnek::Range<ptrdiff_t, 2>;
  using DomainType = schnek::Range<double, 2>;
  using IndexType = schnek::Array<ptrdiff_t, 2>;
  using DomainLimitType = schnek::Array<double, 2>;

  MockDomainDecomposition<2> decomposition;

  // Set up global range and domain
  RangeType globalRange(IndexType(0, 0), IndexType(100, 100));
  DomainType globalDomain(DomainLimitType(0.0, 0.0), DomainLimitType(10.0, 10.0));
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  // Create grid factories and register them
  schnek::GridFactory<GridType> factory1;
  schnek::GridFactory<GridType> factory2;
  schnek::GridRegistration reg1 = decomposition.registerField(factory1);
  schnek::GridRegistration reg2 = decomposition.registerField(factory2);

  // Check that registrations have valid and different ids
  BOOST_CHECK_GE(reg1.id, 0);
  BOOST_CHECK_GE(reg2.id, 0);
  BOOST_CHECK_NE(reg1.id, reg2.id);
}

BOOST_AUTO_TEST_CASE( register_subrange_intersections_1d )
{
  using FieldType = schnek::Field<double, 1>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;
  using DomainType = schnek::Range<double, 1>;
  using IndexType = schnek::Array<ptrdiff_t, 1>;
  using DomainLimitType = schnek::Array<double, 1>;
  using StaggerType = typename FieldType::StaggerType;

  MockDomainDecomposition<1> decomposition;

  RangeType globalRange(IndexType(0), IndexType(14));
  DomainType globalDomain(DomainLimitType(0.0), DomainLimitType(15.0));
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  RangeType localRange1(IndexType(0), IndexType(4));
  DomainType localDomain1(DomainLimitType(0.0), DomainLimitType(5.0));
  decomposition.testAddLocalRange(localRange1, localDomain1);

  RangeType localRange2(IndexType(5), IndexType(9));
  DomainType localDomain2(DomainLimitType(5.0), DomainLimitType(10.0));
  decomposition.testAddLocalRange(localRange2, localDomain2);

  RangeType localRange3(IndexType(10), IndexType(14));
  DomainType localDomain3(DomainLimitType(10.0), DomainLimitType(15.0));
  decomposition.testAddLocalRange(localRange3, localDomain3);

  RangeType subRange(IndexType(2), IndexType(7));

  StaggerType noStagger(false);
  schnek::GridFactory<FieldType> factory(noStagger, 0);
  schnek::GridRegistration reg = decomposition.registerField(factory, subRange);

  auto context = decomposition.getGridContext({reg});

  int callCount = 0;
  std::vector<RangeType> expectedRanges = {
      RangeType(IndexType(2), IndexType(4)),
      RangeType(IndexType(5), IndexType(7))
  };
  std::vector<DomainType> expectedDomains = {
      DomainType(DomainLimitType(2.0), DomainLimitType(5.0)),
      DomainType(DomainLimitType(5.0), DomainLimitType(8.0))
  };

  context.forEach([&](const RangeType &range, FieldType &field) {
    BOOST_REQUIRE_LT(callCount, static_cast<int>(expectedRanges.size()));

    const RangeType &expectedRange = expectedRanges[callCount];
    const DomainType &expectedDomain = expectedDomains[callCount];

    BOOST_CHECK_EQUAL(range.getLo()[0], expectedRange.getLo()[0]);
    BOOST_CHECK_EQUAL(range.getHi()[0], expectedRange.getHi()[0]);
    BOOST_CHECK_EQUAL(field.getLo()[0], expectedRange.getLo()[0]);
    BOOST_CHECK_EQUAL(field.getHi()[0], expectedRange.getHi()[0]);

    const DomainType &fieldDomain = field.getDomain();
    BOOST_CHECK_CLOSE(fieldDomain.getLo()[0], expectedDomain.getLo()[0], 1e-12);
    BOOST_CHECK_CLOSE(fieldDomain.getHi()[0], expectedDomain.getHi()[0], 1e-12);

    ++callCount;
  });

  BOOST_CHECK_EQUAL(callCount, 2);
}

BOOST_AUTO_TEST_CASE( register_subrange_no_overlap_skips_ranges )
{
  using GridType = schnek::Grid<double, 1>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;
  using DomainType = schnek::Range<double, 1>;
  using IndexType = schnek::Array<ptrdiff_t, 1>;
  using DomainLimitType = schnek::Array<double, 1>;

  MockDomainDecomposition<1> decomposition;

  RangeType globalRange(IndexType(0), IndexType(9));
  DomainType globalDomain(DomainLimitType(0.0), DomainLimitType(10.0));
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  RangeType localRange1(IndexType(0), IndexType(4));
  DomainType localDomain1(DomainLimitType(0.0), DomainLimitType(5.0));
  decomposition.testAddLocalRange(localRange1, localDomain1);

  RangeType localRange2(IndexType(5), IndexType(9));
  DomainType localDomain2(DomainLimitType(5.0), DomainLimitType(10.0));
  decomposition.testAddLocalRange(localRange2, localDomain2);

  RangeType subRange(IndexType(20), IndexType(25));

  schnek::GridFactory<GridType> factory;
  schnek::GridRegistration reg = decomposition.registerField(factory, subRange);

  auto context = decomposition.getGridContext({reg});

  int callCount = 0;
  context.forEach([&](const RangeType &, GridType &) { ++callCount; });

  BOOST_CHECK_EQUAL(callCount, 0);
}

BOOST_AUTO_TEST_CASE( register_projection_1d_from_2d )
{
  using FullGridType = schnek::Grid<double, 2>;
  using ProjectedGridType = schnek::Grid<double, 1>;
  using RangeType = schnek::Range<ptrdiff_t, 2>;
  using DomainType = schnek::Range<double, 2>;
  using IndexType = schnek::Array<ptrdiff_t, 2>;
  using DomainLimitType = schnek::Array<double, 2>;

  MockDomainDecomposition<2> decomposition;

  RangeType globalRange(IndexType(0, 0), IndexType(9, 9));
  DomainType globalDomain(DomainLimitType(0.0, 0.0), DomainLimitType(10.0, 10.0));
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  RangeType localRange1(IndexType(0, 0), IndexType(4, 4));
  DomainType localDomain1(DomainLimitType(0.0, 0.0), DomainLimitType(5.0, 5.0));
  decomposition.testAddLocalRange(localRange1, localDomain1);

  RangeType localRange2(IndexType(0, 5), IndexType(4, 9));
  DomainType localDomain2(DomainLimitType(0.0, 5.0), DomainLimitType(5.0, 10.0));
  decomposition.testAddLocalRange(localRange2, localDomain2);

  schnek::GridFactory<FullGridType> fullFactory;
  auto fullReg = decomposition.registerField(fullFactory);

  schnek::GridFactory<ProjectedGridType> projectedFactory;
  auto projectedReg = decomposition.registerProjection<ProjectedGridType>(projectedFactory, {0});

  auto context = decomposition.getGridContext({
      MockDomainDecomposition<2>::RegistrationVariant{fullReg},
      MockDomainDecomposition<2>::RegistrationVariant{projectedReg}
  });

  int callCount = 0;
  context.forEach([&](const RangeType &range, FullGridType &fullGrid, ProjectedGridType &projectedGrid) {
    ++callCount;

    BOOST_CHECK_EQUAL(fullGrid.getLo()[0], range.getLo()[0]);
    BOOST_CHECK_EQUAL(fullGrid.getLo()[1], range.getLo()[1]);
    BOOST_CHECK_EQUAL(fullGrid.getHi()[0], range.getHi()[0]);
    BOOST_CHECK_EQUAL(fullGrid.getHi()[1], range.getHi()[1]);

    BOOST_CHECK_EQUAL(projectedGrid.getLo()[0], range.getLo()[0]);
    BOOST_CHECK_EQUAL(projectedGrid.getHi()[0], range.getHi()[0]);
  });

  BOOST_CHECK_EQUAL(callCount, 2);
}

BOOST_AUTO_TEST_CASE( register_projection_invalid_axes )
{
  using ProjectedGridType1 = schnek::Grid<double, 1>;
  using ProjectedGridType2 = schnek::Grid<double, 2>;
  using RangeType = schnek::Range<ptrdiff_t, 3>;
  using DomainType = schnek::Range<double, 3>;
  using IndexType = schnek::Array<ptrdiff_t, 3>;
  using DomainLimitType = schnek::Array<double, 3>;

  MockDomainDecomposition<3> decomposition;

  RangeType globalRange(IndexType(0, 0, 0), IndexType(9, 9, 9));
  DomainType globalDomain(DomainLimitType(0.0, 0.0, 0.0), DomainLimitType(10.0, 10.0, 10.0));
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  schnek::GridFactory<ProjectedGridType1> projectedFactory1;
  schnek::GridFactory<ProjectedGridType2> projectedFactory2;

  BOOST_CHECK_THROW(
      decomposition.registerProjection<ProjectedGridType1>(projectedFactory1, {3}),
      schnek::ScheckException
  );
  BOOST_CHECK_THROW(
      decomposition.registerProjection<ProjectedGridType2>(projectedFactory2, {0, 0}),
      schnek::ScheckException
  );
}

// ==========================================================================
// Grid context and forEach tests
// ==========================================================================

BOOST_AUTO_TEST_CASE( foreach_single_range_single_grid_1d )
{
  using GridType = schnek::Grid<double, 1>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;
  using DomainType = schnek::Range<double, 1>;
  using IndexType = schnek::Array<ptrdiff_t, 1>;
  using DomainLimitType = schnek::Array<double, 1>;

  MockDomainDecomposition<1> decomposition;

  // Set up global range and domain
  RangeType globalRange(IndexType(0), IndexType(100));
  DomainType globalDomain(DomainLimitType(0.0), DomainLimitType(10.0));
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  // Add a local range
  RangeType localRange(IndexType(0), IndexType(50));
  DomainType localDomain(DomainLimitType(0.0), DomainLimitType(5.0));
  decomposition.testAddLocalRange(localRange, localDomain);

  // Create a grid factory and register it
  schnek::GridFactory<GridType> factory;
  schnek::GridRegistration reg = decomposition.registerField(factory);

  // Get the grid context
  auto context = decomposition.getGridContext({reg});

  // Call forEach and verify it works
  int callCount = 0;
  context.forEach([&callCount, &localRange](const RangeType &range, GridType &grid) {
    callCount++;
    BOOST_CHECK_EQUAL(range.getLo()[0], localRange.getLo()[0]);
    BOOST_CHECK_EQUAL(range.getHi()[0], localRange.getHi()[0]);
    
    // Check grid dimensions match the range
    BOOST_CHECK_EQUAL(grid.getLo()[0], localRange.getLo()[0]);
    BOOST_CHECK_EQUAL(grid.getHi()[0], localRange.getHi()[0]);
  });

  BOOST_CHECK_EQUAL(callCount, 1);
}

BOOST_AUTO_TEST_CASE( foreach_multiple_ranges_single_grid_2d )
{
  using GridType = schnek::Grid<double, 2>;
  using RangeType = schnek::Range<ptrdiff_t, 2>;
  using DomainType = schnek::Range<double, 2>;
  using IndexType = schnek::Array<ptrdiff_t, 2>;
  using DomainLimitType = schnek::Array<double, 2>;

  MockDomainDecomposition<2> decomposition;

  // Set up global range and domain
  RangeType globalRange(IndexType(0, 0), IndexType(100, 100));
  DomainType globalDomain(DomainLimitType(0.0, 0.0), DomainLimitType(10.0, 10.0));
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  // Add multiple local ranges
  RangeType localRange1(IndexType(0, 0), IndexType(50, 50));
  DomainType localDomain1(DomainLimitType(0.0, 0.0), DomainLimitType(5.0, 5.0));
  decomposition.testAddLocalRange(localRange1, localDomain1);

  RangeType localRange2(IndexType(51, 0), IndexType(100, 50));
  DomainType localDomain2(DomainLimitType(5.1, 0.0), DomainLimitType(10.0, 5.0));
  decomposition.testAddLocalRange(localRange2, localDomain2);

  // Create a grid factory and register it
  schnek::GridFactory<GridType> factory;
  schnek::GridRegistration reg = decomposition.registerField(factory);

  // Get the grid context
  auto context = decomposition.getGridContext({reg});

  // Call forEach and verify it iterates over all ranges
  int callCount = 0;
  std::vector<RangeType> expectedRanges = {localRange1, localRange2};
  
  context.forEach([&callCount, &expectedRanges](const RangeType &range, GridType &grid) {
    BOOST_REQUIRE_LT(callCount, static_cast<int>(expectedRanges.size()));
    
    const RangeType &expected = expectedRanges[callCount];
    BOOST_CHECK_EQUAL(range.getLo()[0], expected.getLo()[0]);
    BOOST_CHECK_EQUAL(range.getLo()[1], expected.getLo()[1]);
    BOOST_CHECK_EQUAL(range.getHi()[0], expected.getHi()[0]);
    BOOST_CHECK_EQUAL(range.getHi()[1], expected.getHi()[1]);
    
    // Check grid dimensions match the range
    BOOST_CHECK_EQUAL(grid.getLo()[0], expected.getLo()[0]);
    BOOST_CHECK_EQUAL(grid.getLo()[1], expected.getLo()[1]);
    BOOST_CHECK_EQUAL(grid.getHi()[0], expected.getHi()[0]);
    BOOST_CHECK_EQUAL(grid.getHi()[1], expected.getHi()[1]);
    
    callCount++;
  });

  BOOST_CHECK_EQUAL(callCount, 2);
}

BOOST_AUTO_TEST_CASE( foreach_multiple_grids_2d )
{
  using GridType = schnek::Grid<double, 2>;
  using RangeType = schnek::Range<ptrdiff_t, 2>;
  using DomainType = schnek::Range<double, 2>;
  using IndexType = schnek::Array<ptrdiff_t, 2>;
  using DomainLimitType = schnek::Array<double, 2>;

  MockDomainDecomposition<2> decomposition;

  // Set up global range and domain
  RangeType globalRange(IndexType(0, 0), IndexType(100, 100));
  DomainType globalDomain(DomainLimitType(0.0, 0.0), DomainLimitType(10.0, 10.0));
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  // Add a local range first
  RangeType localRange(IndexType(0, 0), IndexType(50, 50));
  DomainType localDomain(DomainLimitType(0.0, 0.0), DomainLimitType(5.0, 5.0));
  decomposition.testAddLocalRange(localRange, localDomain);

  // Create grid factories and register them
  schnek::GridFactory<GridType> factory1;
  schnek::GridFactory<GridType> factory2;
  schnek::GridRegistration reg1 = decomposition.registerField(factory1);
  schnek::GridRegistration reg2 = decomposition.registerField(factory2);

  // Get the grid context with multiple registrations
  auto context = decomposition.getGridContext({reg1, reg2});

  // Call forEach with multiple grids
  int callCount = 0;
  context.forEach([&callCount, &localRange](const RangeType &range, GridType &grid1, GridType &grid2) {
    callCount++;
    
    // Verify range
    BOOST_CHECK_EQUAL(range.getLo()[0], localRange.getLo()[0]);
    BOOST_CHECK_EQUAL(range.getLo()[1], localRange.getLo()[1]);
    BOOST_CHECK_EQUAL(range.getHi()[0], localRange.getHi()[0]);
    BOOST_CHECK_EQUAL(range.getHi()[1], localRange.getHi()[1]);
    
    // Verify grid1 dimensions
    BOOST_CHECK_EQUAL(grid1.getLo()[0], localRange.getLo()[0]);
    BOOST_CHECK_EQUAL(grid1.getLo()[1], localRange.getLo()[1]);
    BOOST_CHECK_EQUAL(grid1.getHi()[0], localRange.getHi()[0]);
    BOOST_CHECK_EQUAL(grid1.getHi()[1], localRange.getHi()[1]);
    
    // Verify grid2 dimensions
    BOOST_CHECK_EQUAL(grid2.getLo()[0], localRange.getLo()[0]);
    BOOST_CHECK_EQUAL(grid2.getLo()[1], localRange.getLo()[1]);
    BOOST_CHECK_EQUAL(grid2.getHi()[0], localRange.getHi()[0]);
    BOOST_CHECK_EQUAL(grid2.getHi()[1], localRange.getHi()[1]);
    
    // Verify that the two grids are different objects (different addresses)
    BOOST_CHECK_NE(static_cast<void*>(&grid1), static_cast<void*>(&grid2));
  });

  BOOST_CHECK_EQUAL(callCount, 1);
}

BOOST_AUTO_TEST_CASE( foreach_grid_modification_2d )
{
  using GridType = schnek::Grid<double, 2>;
  using RangeType = schnek::Range<ptrdiff_t, 2>;
  using DomainType = schnek::Range<double, 2>;
  using IndexType = schnek::Array<ptrdiff_t, 2>;
  using DomainLimitType = schnek::Array<double, 2>;

  MockDomainDecomposition<2> decomposition;

  // Set up global range and domain
  RangeType globalRange(IndexType(0, 0), IndexType(10, 10));
  DomainType globalDomain(DomainLimitType(0.0, 0.0), DomainLimitType(1.0, 1.0));
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  // Add a local range
  RangeType localRange(IndexType(0, 0), IndexType(5, 5));
  DomainType localDomain(DomainLimitType(0.0, 0.0), DomainLimitType(0.5, 0.5));
  decomposition.testAddLocalRange(localRange, localDomain);

  // Create a grid factory and register it
  schnek::GridFactory<GridType> factory;
  schnek::GridRegistration reg = decomposition.registerField(factory);

  // Get the grid context
  auto context = decomposition.getGridContext({reg});

  // Modify the grid through forEach
  double testValue = 42.0;
  context.forEach([testValue](const RangeType &range, GridType &grid) {
    for (ptrdiff_t i = range.getLo()[0]; i <= range.getHi()[0]; ++i) {
      for (ptrdiff_t j = range.getLo()[1]; j <= range.getHi()[1]; ++j) {
        grid(i, j) = testValue;
      }
    }
  });

  // Verify the modification persisted
  context.forEach([testValue](const RangeType &range, GridType &grid) {
    for (ptrdiff_t i = range.getLo()[0]; i <= range.getHi()[0]; ++i) {
      for (ptrdiff_t j = range.getLo()[1]; j <= range.getHi()[1]; ++j) {
        BOOST_CHECK_EQUAL(grid(i, j), testValue);
      }
    }
  });
}

BOOST_AUTO_TEST_CASE( foreach_register_after_add_range )
{
  using GridType = schnek::Grid<double, 2>;
  using RangeType = schnek::Range<ptrdiff_t, 2>;
  using DomainType = schnek::Range<double, 2>;
  using IndexType = schnek::Array<ptrdiff_t, 2>;
  using DomainLimitType = schnek::Array<double, 2>;

  MockDomainDecomposition<2> decomposition;

  // Set up global range and domain
  RangeType globalRange(IndexType(0, 0), IndexType(100, 100));
  DomainType globalDomain(DomainLimitType(0.0, 0.0), DomainLimitType(10.0, 10.0));
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  // Add local ranges BEFORE registering fields
  RangeType localRange1(IndexType(0, 0), IndexType(50, 50));
  DomainType localDomain1(DomainLimitType(0.0, 0.0), DomainLimitType(5.0, 5.0));
  decomposition.testAddLocalRange(localRange1, localDomain1);

  RangeType localRange2(IndexType(51, 51), IndexType(100, 100));
  DomainType localDomain2(DomainLimitType(5.1, 5.1), DomainLimitType(10.0, 10.0));
  decomposition.testAddLocalRange(localRange2, localDomain2);

  // Now register fields
  schnek::GridFactory<GridType> factory;
  schnek::GridRegistration reg = decomposition.registerField(factory);

  // Get the grid context
  auto context = decomposition.getGridContext({reg});

  // Call forEach and verify it iterates over all pre-existing ranges
  int callCount = 0;
  std::vector<RangeType> expectedRanges = {localRange1, localRange2};
  
  context.forEach([&callCount, &expectedRanges](const RangeType &range, GridType &grid) {
    BOOST_REQUIRE_LT(callCount, static_cast<int>(expectedRanges.size()));
    
    const RangeType &expected = expectedRanges[callCount];
    BOOST_CHECK_EQUAL(range.getLo()[0], expected.getLo()[0]);
    BOOST_CHECK_EQUAL(range.getLo()[1], expected.getLo()[1]);
    BOOST_CHECK_EQUAL(range.getHi()[0], expected.getHi()[0]);
    BOOST_CHECK_EQUAL(range.getHi()[1], expected.getHi()[1]);
    
    callCount++;
  });

  BOOST_CHECK_EQUAL(callCount, 2);
}

BOOST_AUTO_TEST_CASE( foreach_add_range_after_register )
{
  using GridType = schnek::Grid<double, 2>;
  using RangeType = schnek::Range<ptrdiff_t, 2>;
  using DomainType = schnek::Range<double, 2>;
  using IndexType = schnek::Array<ptrdiff_t, 2>;
  using DomainLimitType = schnek::Array<double, 2>;

  MockDomainDecomposition<2> decomposition;

  // Set up global range and domain
  RangeType globalRange(IndexType(0, 0), IndexType(100, 100));
  DomainType globalDomain(DomainLimitType(0.0, 0.0), DomainLimitType(10.0, 10.0));
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  // Register fields BEFORE adding ranges
  schnek::GridFactory<GridType> factory;
  schnek::GridRegistration reg = decomposition.registerField(factory);

  // Add local ranges AFTER registering fields
  RangeType localRange1(IndexType(0, 0), IndexType(50, 50));
  DomainType localDomain1(DomainLimitType(0.0, 0.0), DomainLimitType(5.0, 5.0));
  decomposition.testAddLocalRange(localRange1, localDomain1);

  RangeType localRange2(IndexType(51, 51), IndexType(100, 100));
  DomainType localDomain2(DomainLimitType(5.1, 5.1), DomainLimitType(10.0, 10.0));
  decomposition.testAddLocalRange(localRange2, localDomain2);

  // Get the grid context
  auto context = decomposition.getGridContext({reg});

  // Call forEach and verify it iterates over all ranges added after registration
  int callCount = 0;
  std::vector<RangeType> expectedRanges = {localRange1, localRange2};
  
  context.forEach([&callCount, &expectedRanges](const RangeType &range, GridType &grid) {
    BOOST_REQUIRE_LT(callCount, static_cast<int>(expectedRanges.size()));
    
    const RangeType &expected = expectedRanges[callCount];
    BOOST_CHECK_EQUAL(range.getLo()[0], expected.getLo()[0]);
    BOOST_CHECK_EQUAL(range.getLo()[1], expected.getLo()[1]);
    BOOST_CHECK_EQUAL(range.getHi()[0], expected.getHi()[0]);
    BOOST_CHECK_EQUAL(range.getHi()[1], expected.getHi()[1]);
    
    callCount++;
  });

  BOOST_CHECK_EQUAL(callCount, 2);
}

BOOST_AUTO_TEST_CASE( foreach_mixed_registration_and_ranges_with_convolution )
{
  using DoubleGridType = schnek::Grid<double, 2>;
  using IntGridType = schnek::Grid<int, 2>;
  using FieldType = schnek::Field<double, 2>;
  using RangeType = schnek::Range<ptrdiff_t, 2>;
  using DomainType = schnek::Range<double, 2>;
  using IndexType = schnek::Array<ptrdiff_t, 2>;
  using DomainLimitType = schnek::Array<double, 2>;
  using StaggerType = schnek::Array<bool, 2>;

  MockDomainDecomposition<2> decomposition;

  // Set up global range and domain
  RangeType globalRange(IndexType(0, 0), IndexType(100, 100));
  DomainType globalDomain(DomainLimitType(0.0, 0.0), DomainLimitType(10.0, 10.0));
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  // Step 1: Add first local range
  RangeType localRange1(IndexType(10, 10), IndexType(30, 30));
  DomainType localDomain1(DomainLimitType(1.0, 1.0), DomainLimitType(3.0, 3.0));
  decomposition.testAddLocalRange(localRange1, localDomain1);

  // Step 2: Register Grid<double, 2> - this will create a grid for localRange1
  schnek::GridFactory<DoubleGridType> doubleGridFactory;
  schnek::GridRegistration doubleGridReg = decomposition.registerField(doubleGridFactory);

  // Step 3: Add second local range
  RangeType localRange2(IndexType(40, 40), IndexType(60, 60));
  DomainType localDomain2(DomainLimitType(4.0, 4.0), DomainLimitType(6.0, 6.0));
  decomposition.testAddLocalRange(localRange2, localDomain2);

  // Step 4: Register Grid<int, 2> - this will create grids for localRange1 and localRange2
  schnek::GridFactory<IntGridType> intGridFactory;
  schnek::GridRegistration intGridReg = decomposition.registerField(intGridFactory);

  // Step 5: Register Field<double, 2> with no stagger and 2 ghost cells
  StaggerType noStagger(false);
  int ghostCells = 2;
  schnek::GridFactory<FieldType> fieldFactory(noStagger, ghostCells);
  schnek::GridRegistration fieldReg = decomposition.registerField(fieldFactory);

  // Step 6: Add third local range
  RangeType localRange3(IndexType(70, 70), IndexType(90, 90));
  DomainType localDomain3(DomainLimitType(7.0, 7.0), DomainLimitType(9.0, 9.0));
  decomposition.testAddLocalRange(localRange3, localDomain3);

  // Get the grid context with all three registrations
  auto context = decomposition.getGridContext({doubleGridReg, intGridReg, fieldReg});

  // Initialize the int grid and field with test values before convolution
  context.forEach([](const RangeType &range, DoubleGridType &doubleGrid, IntGridType &intGrid, FieldType &field) {
    // Initialize intGrid with a simple pattern (used as multiplier)
    for (ptrdiff_t i = range.getLo()[0]; i <= range.getHi()[0]; ++i) {
      for (ptrdiff_t j = range.getLo()[1]; j <= range.getHi()[1]; ++j) {
        intGrid(i, j) = 2;  // Multiplier of 2
      }
    }

    // Initialize field with values including ghost cells
    // The field should have ghost cells padding: [lo-2, hi+2] in each dimension
    ptrdiff_t fieldLoI = field.getLo()[0];
    ptrdiff_t fieldHiI = field.getHi()[0];
    ptrdiff_t fieldLoJ = field.getLo()[1];
    ptrdiff_t fieldHiJ = field.getHi()[1];
    
    // Verify the field has the expected ghost cell padding
    BOOST_CHECK_EQUAL(fieldLoI, range.getLo()[0] - 2);
    BOOST_CHECK_EQUAL(fieldHiI, range.getHi()[0] + 2);
    BOOST_CHECK_EQUAL(fieldLoJ, range.getLo()[1] - 2);
    BOOST_CHECK_EQUAL(fieldHiJ, range.getHi()[1] + 2);

    // Fill the entire field (including ghost cells) with a simple pattern
    for (ptrdiff_t i = fieldLoI; i <= fieldHiI; ++i) {
      for (ptrdiff_t j = fieldLoJ; j <= fieldHiJ; ++j) {
        field(i, j) = 1.0;  // Uniform value for simple convolution test
      }
    }

    // Initialize doubleGrid to zero
    for (ptrdiff_t i = range.getLo()[0]; i <= range.getHi()[0]; ++i) {
      for (ptrdiff_t j = range.getLo()[1]; j <= range.getHi()[1]; ++j) {
        doubleGrid(i, j) = 0.0;
      }
    }
  });

  // Now perform the convolution calculation
  // The convolution uses a 5x5 kernel (using 2 ghost cells in each direction)
  context.forEach([](const RangeType &range, DoubleGridType &doubleGrid, IntGridType &intGrid, FieldType &field) {
    // Perform a local convolution of the field using the ghost cells
    // The kernel is a simple averaging over a 5x5 neighborhood (radius 2)
    for (ptrdiff_t i = range.getLo()[0]; i <= range.getHi()[0]; ++i) {
      for (ptrdiff_t j = range.getLo()[1]; j <= range.getHi()[1]; ++j) {
        double sum = 0.0;
        int count = 0;
        
        // Sum over 5x5 neighborhood centered at (i, j)
        for (ptrdiff_t di = -2; di <= 2; ++di) {
          for (ptrdiff_t dj = -2; dj <= 2; ++dj) {
            sum += field(i + di, j + dj);
            ++count;
          }
        }
        
        // Average and multiply by the int grid value
        double convolutionResult = (sum / count) * intGrid(i, j);
        doubleGrid(i, j) = convolutionResult;
      }
    }
  });

  // Verify the results
  // Since field was filled with 1.0 uniformly, the convolution average is 1.0
  // Multiplied by intGrid value of 2, the result should be 2.0
  int rangeCount = 0;
  std::vector<RangeType> expectedRanges = {localRange1, localRange2, localRange3};
  
  context.forEach([&rangeCount, &expectedRanges](const RangeType &range, DoubleGridType &doubleGrid, IntGridType &intGrid, FieldType &field) {
    (void)intGrid;  // Suppress unused warning
    (void)field;    // Suppress unused warning
    
    BOOST_REQUIRE_LT(rangeCount, static_cast<int>(expectedRanges.size()));
    
    // Verify range matches expected
    const RangeType &expected = expectedRanges[rangeCount];
    BOOST_CHECK_EQUAL(range.getLo()[0], expected.getLo()[0]);
    BOOST_CHECK_EQUAL(range.getLo()[1], expected.getLo()[1]);
    BOOST_CHECK_EQUAL(range.getHi()[0], expected.getHi()[0]);
    BOOST_CHECK_EQUAL(range.getHi()[1], expected.getHi()[1]);
    
    // Verify convolution result: average of 1.0 * multiplier of 2 = 2.0
    for (ptrdiff_t i = range.getLo()[0]; i <= range.getHi()[0]; ++i) {
      for (ptrdiff_t j = range.getLo()[1]; j <= range.getHi()[1]; ++j) {
        BOOST_CHECK_CLOSE(doubleGrid(i, j), 2.0, 1e-10);
      }
    }
    
    ++rangeCount;
  });

  BOOST_CHECK_EQUAL(rangeCount, 3);
}

BOOST_AUTO_TEST_SUITE_END()
