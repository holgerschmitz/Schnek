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

namespace {

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

  struct FillVisitor {
    int value;
    void operator()(GridType &grid) const {
      for (ptrdiff_t i = grid.getLo(0); i <= grid.getHi(0); ++i) {
        for (ptrdiff_t j = grid.getLo(1); j <= grid.getHi(1); ++j) {
          grid(i, j) = value;
        }
      }
    }
  };

  schnek::internal::visitGridWrapper(wrapper, FillVisitor{7});

  auto typed = std::dynamic_pointer_cast<schnek::internal::GridWrapperImpl<GridType>>(wrapper);
  BOOST_REQUIRE(typed);

  for (ptrdiff_t i = lo[0]; i <= hi[0]; ++i) {
    for (ptrdiff_t j = lo[1]; j <= hi[1]; ++j) {
      BOOST_CHECK_EQUAL(typed->grid(i, j), 7);
    }
  }
}

BOOST_AUTO_TEST_CASE(grid_visitor_reuses_visitable_handle)
{
  using GridType = schnek::Grid<int, 1>;

  schnek::Array<ptrdiff_t, 1> lo(0);
  schnek::Array<ptrdiff_t, 1> hi(3);
  auto range = makeRange(lo, hi);
  schnek::Range<double, 1> domain;

  schnek::GridFactory<GridType> factory;
  auto wrapper = factory.newGrid(range, domain);

  struct FillVisitor {
    int value;
    void operator()(GridType &grid) const {
      for (ptrdiff_t i = grid.getLo(0); i <= grid.getHi(0); ++i) {
        grid(i) = value;
      }
    }
  };

  auto visitable = schnek::internal::makeVisitGridWrapper<FillVisitor>(wrapper);

  visitable->accept(FillVisitor{1});
  visitable->accept(FillVisitor{3});

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

  // Seed grid with deterministic pattern
  auto typed = std::dynamic_pointer_cast<schnek::internal::GridWrapperImpl<GridType>>(wrapper);
  BOOST_REQUIRE(typed);
  for (ptrdiff_t i = lo[0]; i <= hi[0]; ++i) {
    for (ptrdiff_t j = lo[1]; j <= hi[1]; ++j) {
      typed->grid(i, j) = static_cast<int>(i + j);
    }
  }

  auto reader = [](const GridType &grid) {
    int total = 0;
    for (ptrdiff_t i = grid.getLo(0); i <= grid.getHi(0); ++i) {
      for (ptrdiff_t j = grid.getLo(1); j <= grid.getHi(1); ++j) {
        total += grid(i, j);
      }
    }
    BOOST_CHECK_EQUAL(total, 9);  // sum of (i + j) over 3x2 domain
  };

  schnek::internal::visitGridWrapper(wrapper, reader);
}

BOOST_AUTO_TEST_CASE(grid_visitor_throws_on_type_mismatch)
{
  using GridType = schnek::Grid<int, 2>;

  schnek::Array<ptrdiff_t, 2> lo(0, 0);
  schnek::Array<ptrdiff_t, 2> hi(1, 0);
  auto range = makeRange(lo, hi);
  schnek::Range<double, 2> domain;

  schnek::GridFactory<GridType> factory;
  auto wrapper = factory.newGrid(range, domain);

  struct WrongVisitor {
    void operator()(schnek::Grid<double, 2> &) const {}
  };

  BOOST_CHECK_THROW(schnek::internal::visitGridWrapper(wrapper, WrongVisitor{}), schnek::ScheckException);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
