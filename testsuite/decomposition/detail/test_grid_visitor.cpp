#include "../../utility.hpp"

#include <decomposition/detail/grid_factory.hpp>
#include <decomposition/detail/grid_visitor.hpp>
#include <grid/array.hpp>
#include <grid/grid.hpp>
#include <grid/range.hpp>
#include <util/exceptions.hpp>

#include <boost/test/unit_test.hpp>

#include <cstddef>
#include <memory>
#include <type_traits>

namespace {

struct FillGrid2DVisitor : schnek::internal::GridVisitor<FillGrid2DVisitor> {
    using GridType = schnek::Grid<int, 2>;

    explicit FillGrid2DVisitor(int valueIn) : value(valueIn) { this->template registerHandler<GridType>(); }

    template<typename Grid>
    void handle(Grid &grid, bool /*useFieldInfo*/) {
      static_assert(std::is_same<Grid, GridType>::value, "Unexpected grid type");
      for (ptrdiff_t i = grid.getLo(0); i <= grid.getHi(0); ++i) {
        for (ptrdiff_t j = grid.getLo(1); j <= grid.getHi(1); ++j) {
          grid(i, j) = value;
        }
      }
    }

    int value;
};

struct FillGrid1DVisitor : schnek::internal::GridVisitor<FillGrid1DVisitor> {
    using GridType = schnek::Grid<int, 1>;

    FillGrid1DVisitor() { this->template registerHandler<GridType>(); }

    void setValue(int v) { value = v; }

    template<typename Grid>
    void handle(Grid &grid, bool /*useFieldInfo*/) {
      static_assert(std::is_same<Grid, GridType>::value, "Unexpected grid type");
      for (ptrdiff_t i = grid.getLo(0); i <= grid.getHi(0); ++i) {
        grid(i) = value;
      }
    }

    int value = 0;
};

struct ReaderGrid2DVisitor : schnek::internal::GridVisitor<ReaderGrid2DVisitor> {
    using GridType = schnek::Grid<int, 2>;

    ReaderGrid2DVisitor() { this->template registerHandler<GridType>(); }

    template<typename Grid>
    void handle(Grid &grid, bool /*useFieldInfo*/) {
      static_assert(std::is_same<Grid, GridType>::value, "Unexpected grid type");
      int total = 0;
      for (ptrdiff_t i = grid.getLo(0); i <= grid.getHi(0); ++i) {
        for (ptrdiff_t j = grid.getLo(1); j <= grid.getHi(1); ++j) {
          total += grid(i, j);
        }
      }
      BOOST_CHECK_EQUAL(total, 9);
    }
};

struct EmptyVisitor : schnek::internal::GridVisitor<EmptyVisitor> {
    template<typename Grid>
    void handle(Grid &, bool) {
      BOOST_FAIL("EmptyVisitor should not be invoked");
    }
};

template<size_t rank>
schnek::Range<ptrdiff_t, rank> makeRange(
    const schnek::Array<ptrdiff_t, rank> &lo,
    const schnek::Array<ptrdiff_t, rank> &hi)
{
  return schnek::Range<ptrdiff_t, rank>(lo, hi);
}

}  // namespace

BOOST_AUTO_TEST_SUITE(domain_decomposition)
BOOST_AUTO_TEST_SUITE(detail)

BOOST_AUTO_TEST_CASE(grid_visitor_invokes_callable_on_grid)
{
  using GridType = schnek::Grid<int, 2>;

  schnek::Array<ptrdiff_t, 2> lo(0, 0);
  schnek::Array<ptrdiff_t, 2> hi(1, 1);
  auto range = makeRange(lo, hi);
  schnek::Range<double, 2> domain;

  schnek::GridFactory<GridType> factory;
  auto wrapper = factory.newGrid(range, domain);

  FillGrid2DVisitor visitor{7};

  wrapper->accept(visitor, true);

  auto typed = std::dynamic_pointer_cast<schnek::internal::GridWrapperImpl<GridType>>(wrapper);
  BOOST_REQUIRE(typed);

  for (ptrdiff_t i = lo[0]; i <= hi[0]; ++i) {
    for (ptrdiff_t j = lo[1]; j <= hi[1]; ++j) {
      BOOST_CHECK_EQUAL(typed->grid(i, j), 7);
    }
  }
}

BOOST_AUTO_TEST_CASE(grid_visitor_reuses_handle)
{
  using GridType = schnek::Grid<int, 1>;

  schnek::Array<ptrdiff_t, 1> lo(0);
  schnek::Array<ptrdiff_t, 1> hi(3);
  auto range = makeRange(lo, hi);
  schnek::Range<double, 1> domain;

  schnek::GridFactory<GridType> factory;
  auto wrapper = factory.newGrid(range, domain);

  FillGrid1DVisitor visitor;

  visitor.setValue(1);
  wrapper->accept(visitor, true);
  visitor.setValue(3);
  wrapper->accept(visitor, true);

  auto typed = std::dynamic_pointer_cast<schnek::internal::GridWrapperImpl<GridType>>(wrapper);
  BOOST_REQUIRE(typed);

  for (ptrdiff_t i = lo[0]; i <= hi[0]; ++i) {
    BOOST_CHECK_EQUAL(typed->grid(i), 3);
  }
}

BOOST_AUTO_TEST_CASE(grid_visitor_allows_const_readers)
{
  using GridType = schnek::Grid<int, 2>;

  schnek::Array<ptrdiff_t, 2> lo(0, 0);
  schnek::Array<ptrdiff_t, 2> hi(2, 1);
  auto range = makeRange(lo, hi);
  schnek::Range<double, 2> domain;

  schnek::GridFactory<GridType> factory;
  auto wrapper = factory.newGrid(range, domain);

  auto typed = std::dynamic_pointer_cast<schnek::internal::GridWrapperImpl<GridType>>(wrapper);
  BOOST_REQUIRE(typed);
  for (ptrdiff_t i = lo[0]; i <= hi[0]; ++i) {
    for (ptrdiff_t j = lo[1]; j <= hi[1]; ++j) {
      typed->grid(i, j) = static_cast<int>(i + j);
    }
  }

  ReaderGrid2DVisitor visitor;

  wrapper->accept(visitor, false);
}

BOOST_AUTO_TEST_CASE(grid_visitor_throws_on_unregistered_type)
{
  using GridType = schnek::Grid<int, 2>;

  schnek::Array<ptrdiff_t, 2> lo(0, 0);
  schnek::Array<ptrdiff_t, 2> hi(1, 0);
  auto range = makeRange(lo, hi);
  schnek::Range<double, 2> domain;

  schnek::GridFactory<GridType> factory;
  auto wrapper = factory.newGrid(range, domain);

  EmptyVisitor visitor;

  BOOST_CHECK_THROW(wrapper->accept(visitor, true), schnek::ScheckException);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
