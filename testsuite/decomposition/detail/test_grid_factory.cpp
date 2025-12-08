#include "../../utility.hpp"

#include <decomposition/detail/grid_factory.hpp>
#include <grid/array.hpp>
#include <grid/field.hpp>
#include <grid/grid.hpp>
#include <grid/range.hpp>

#include <boost/test/unit_test.hpp>

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

BOOST_AUTO_TEST_CASE(grid_factory_creates_grid_with_converted_range)
{
  schnek::Array<ptrdiff_t, 2> lo(-2, 1);
  schnek::Array<ptrdiff_t, 2> hi(3, 4);
  auto range = makeRange(lo, hi);
  schnek::Range<double, 2> domain;

  schnek::GridFactory<schnek::Grid<int, 2>> factory;
  auto wrapper = factory.newGrid(range, domain);
  auto typed = std::dynamic_pointer_cast<schnek::internal::GridWrapperImpl<schnek::Grid<int, 2>>>(wrapper);
  BOOST_REQUIRE(typed);
  BOOST_CHECK_EQUAL(typed->grid.getLo(0), -2);
  BOOST_CHECK_EQUAL(typed->grid.getLo(1), 1);
  BOOST_CHECK_EQUAL(typed->grid.getHi(0), 3);
  BOOST_CHECK_EQUAL(typed->grid.getHi(1), 4);
}

BOOST_AUTO_TEST_CASE(field_factory_respects_domain_stagger_and_ghost_cells)
{
  schnek::Array<ptrdiff_t, 2> lo(0, 0);
  schnek::Array<ptrdiff_t, 2> hi(5, 7);
  auto range = makeRange(lo, hi);
  schnek::Range<double, 2> domain(schnek::Array<double, 2>(0.0, 0.5), schnek::Array<double, 2>(1.0, 2.5));

  schnek::Array<bool, 2> stagger(false);
  stagger[0] = true;
  stagger[1] = false;

  schnek::GridFactory<schnek::Field<double, 2>> factory{stagger, 2};

  auto wrapper = factory.newGrid(range, domain);
  auto typed = std::dynamic_pointer_cast<schnek::internal::GridWrapperImpl<schnek::Field<double, 2>>>(wrapper);
  BOOST_REQUIRE(typed);

  auto &field = typed->grid;
  BOOST_CHECK_EQUAL(field.getStagger(0), true);
  BOOST_CHECK_EQUAL(field.getStagger(1), false);

  auto innerLo = field.getInnerLo();
  auto innerHi = field.getInnerHi();
  BOOST_CHECK_EQUAL(innerLo[0], lo[0]);
  BOOST_CHECK_EQUAL(innerHi[0], hi[0]);
  BOOST_CHECK_EQUAL(innerLo[1], lo[1]);
  BOOST_CHECK_EQUAL(innerHi[1], hi[1]);

  auto gridLo = field.getLo();
  auto gridHi = field.getHi();
  BOOST_CHECK_EQUAL(gridLo[0], lo[0] - 2);
  BOOST_CHECK_EQUAL(gridHi[0], hi[0] + 2);
  BOOST_CHECK_EQUAL(gridLo[1], lo[1] - 2);
  BOOST_CHECK_EQUAL(gridHi[1], hi[1] + 2);

  const auto &resultDomain = field.getDomain();
  BOOST_CHECK_CLOSE(resultDomain.getLo(0), domain.getLo(0), 1e-12);
  BOOST_CHECK_CLOSE(resultDomain.getHi(1), domain.getHi(1), 1e-12);
}

BOOST_AUTO_TEST_CASE(grid_registration_impl_creates_wrappers_via_factory)
{
  schnek::GridFactory<schnek::Grid<float, 1>> factory;
  schnek::internal::GridRegistrationImpl<1, schnek::ArrayNoArgCheck, schnek::Grid<float, 1>> registration(factory);

  schnek::Array<ptrdiff_t, 1> lo(0);
  schnek::Array<ptrdiff_t, 1> hi(4);
  auto range = makeRange(lo, hi);
  schnek::Range<double, 1> domain(schnek::Array<double, 1>(0.0), schnek::Array<double, 1>(1.0));

  auto wrapper = registration.makeGrid(range, domain);
  auto typed = std::dynamic_pointer_cast<schnek::internal::GridWrapperImpl<schnek::Grid<float, 1>>>(wrapper);
  BOOST_REQUIRE(typed);
  BOOST_CHECK_EQUAL(typed->grid.getDims(0), 5);
  BOOST_CHECK(registration.getId() >= 0);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
