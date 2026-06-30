#include "../../utility.hpp"

#include <decomposition/detail/grid_transfer.hpp>
#include <grid/array.hpp>
#include <grid/grid.hpp>
#include <grid/range.hpp>

#include <boost/test/unit_test.hpp>

#include <vector>

namespace {

  template<size_t rank>
  schnek::Range<ptrdiff_t, rank> makeRange(
      const schnek::Array<ptrdiff_t, rank> &lo, const schnek::Array<ptrdiff_t, rank> &hi
  ) {
    return schnek::Range<ptrdiff_t, rank>(lo, hi);
  }

}  // namespace

BOOST_AUTO_TEST_SUITE(domain_decomposition)
BOOST_AUTO_TEST_SUITE(detail)
BOOST_AUTO_TEST_SUITE(grid_transfer)

BOOST_AUTO_TEST_CASE(pack_copies_subrange_into_contiguous_buffer) {
  using GridType = schnek::Grid<int, 2>;
  GridType grid(schnek::Array<ptrdiff_t, 2>(0, 0), schnek::Array<ptrdiff_t, 2>(3, 3));

  for (ptrdiff_t i = 0; i <= 3; ++i) {
    for (ptrdiff_t j = 0; j <= 3; ++j) {
      grid(i, j) = static_cast<int>(10 * i + j);
    }
  }

  auto range = makeRange<2>(schnek::Array<ptrdiff_t, 2>(1, 1), schnek::Array<ptrdiff_t, 2>(2, 2));

  std::vector<int> buffer(4, -1);
  schnek::detail::GridTransfer<GridType>::pack(grid, range, buffer.data());

  std::vector<int> expected = {11, 12, 21, 22};
  BOOST_CHECK_EQUAL_COLLECTIONS(buffer.begin(), buffer.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(unpack_scatters_buffer_into_subrange) {
  using GridType = schnek::Grid<int, 2>;
  GridType grid(schnek::Array<ptrdiff_t, 2>(0, 0), schnek::Array<ptrdiff_t, 2>(3, 3));
  grid = 0;

  auto range = makeRange<2>(schnek::Array<ptrdiff_t, 2>(1, 1), schnek::Array<ptrdiff_t, 2>(2, 2));

  std::vector<int> buffer = {11, 12, 21, 22};
  schnek::detail::GridTransfer<GridType>::unpack(grid, range, buffer.data());

  BOOST_CHECK_EQUAL(grid(1, 1), 11);
  BOOST_CHECK_EQUAL(grid(1, 2), 12);
  BOOST_CHECK_EQUAL(grid(2, 1), 21);
  BOOST_CHECK_EQUAL(grid(2, 2), 22);
  // Cells outside the range must be untouched.
  BOOST_CHECK_EQUAL(grid(0, 0), 0);
  BOOST_CHECK_EQUAL(grid(3, 3), 0);
}

BOOST_AUTO_TEST_CASE(pack_unpack_roundtrip_is_identity) {
  using GridType = schnek::Grid<double, 1>;
  GridType source(schnek::Array<ptrdiff_t, 1>(-2), schnek::Array<ptrdiff_t, 1>(5));
  GridType dest(schnek::Array<ptrdiff_t, 1>(-2), schnek::Array<ptrdiff_t, 1>(5));
  dest = 0.0;

  for (ptrdiff_t i = -2; i <= 5; ++i) {
    source(i) = 0.5 * i;
  }

  auto range = makeRange<1>(schnek::Array<ptrdiff_t, 1>(0), schnek::Array<ptrdiff_t, 1>(4));

  std::vector<double> buffer(5, 0.0);
  schnek::detail::GridTransfer<GridType>::pack(source, range, buffer.data());
  schnek::detail::GridTransfer<GridType>::unpack(dest, range, buffer.data());

  for (ptrdiff_t i = 0; i <= 4; ++i) {
    BOOST_CHECK_CLOSE(dest(i), 0.5 * i, 1e-12);
  }
}

BOOST_AUTO_TEST_CASE(copy_transfers_subrange_between_grids) {
  using GridType = schnek::Grid<int, 2>;
  GridType source(schnek::Array<ptrdiff_t, 2>(0, 0), schnek::Array<ptrdiff_t, 2>(3, 3));
  GridType dest(schnek::Array<ptrdiff_t, 2>(0, 0), schnek::Array<ptrdiff_t, 2>(3, 3));
  dest = 0;

  for (ptrdiff_t i = 0; i <= 3; ++i) {
    for (ptrdiff_t j = 0; j <= 3; ++j) {
      source(i, j) = static_cast<int>(10 * i + j);
    }
  }

  auto range = makeRange<2>(schnek::Array<ptrdiff_t, 2>(1, 1), schnek::Array<ptrdiff_t, 2>(2, 2));
  schnek::detail::GridTransfer<GridType>::copy(source, dest, range);

  BOOST_CHECK_EQUAL(dest(1, 1), 11);
  BOOST_CHECK_EQUAL(dest(1, 2), 12);
  BOOST_CHECK_EQUAL(dest(2, 1), 21);
  BOOST_CHECK_EQUAL(dest(2, 2), 22);
  // Cells outside the range must remain zero.
  BOOST_CHECK_EQUAL(dest(0, 0), 0);
  BOOST_CHECK_EQUAL(dest(3, 3), 0);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
