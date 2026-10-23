#include "../../utility.hpp"

#include <decomposition/detail/grid_transfer.hpp>
#include <decomposition/detail/grid-communication-traits.hpp>
#include <grid/array.hpp>
#include <grid/grid.hpp>
#include <grid/range.hpp>

#include <boost/test/unit_test.hpp>

#include <cstdint>
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

BOOST_AUTO_TEST_CASE(accumulate_adds_to_subrange_and_returns_updated_values) {
  using GridType = schnek::Grid<int, 1>;
  GridType grid(schnek::Array<ptrdiff_t, 1>(0), schnek::Array<ptrdiff_t, 1>(3));
  grid = 10;

  auto range = makeRange<1>(schnek::Array<ptrdiff_t, 1>(1), schnek::Array<ptrdiff_t, 1>(2));
  std::vector<int> incoming = {2, 3};
  std::vector<int> updated(2, 0);
  std::vector<int> expected = {12, 13};

  schnek::detail::GridTransfer<GridType>::accumulate(grid, range, incoming.data(), updated.data());

  BOOST_CHECK_EQUAL_COLLECTIONS(updated.begin(), updated.end(), expected.begin(), expected.end());
  BOOST_CHECK_EQUAL(grid(0), 10);
  BOOST_CHECK_EQUAL(grid(1), 12);
  BOOST_CHECK_EQUAL(grid(2), 13);
  BOOST_CHECK_EQUAL(grid(3), 10);
}

BOOST_AUTO_TEST_CASE(host_communication_buffer_retains_and_grows_capacity) {
  schnek::ScratchBuffer buffer;
  buffer.reserve_bytes(2 * sizeof(int));
  BOOST_CHECK_EQUAL(buffer.capacity_bytes(), 2 * sizeof(int));

  buffer.reserve_bytes(sizeof(char));
  BOOST_CHECK_EQUAL(buffer.capacity_bytes(), 2 * sizeof(int));

  buffer.reserve_bytes(4 * sizeof(int));
  BOOST_CHECK_EQUAL(buffer.capacity_bytes(), 4 * sizeof(int));
}

#ifdef SCHNEK_HAVE_KOKKOS
BOOST_AUTO_TEST_CASE(kokkos_communication_buffer_retains_and_grows_capacity) {
  schnek::detail::KokkosCommunicationBuffer<Kokkos::HostSpace> buffer;
  buffer.reserve_bytes(2 * sizeof(int));
  BOOST_CHECK_EQUAL(buffer.capacity_bytes(), 2 * sizeof(int));

  buffer.reserve_bytes(sizeof(char));
  BOOST_CHECK_EQUAL(buffer.capacity_bytes(), 2 * sizeof(int));

  buffer.reserve_bytes(4 * sizeof(int));
  BOOST_CHECK_EQUAL(buffer.capacity_bytes(), 4 * sizeof(int));
}

// Guards the slab assumption in the device rebalancing plan (section 2.2):
// the send and receive slabs are live at the same time and must come from
// separate arenas, so their storage must never overlap.
BOOST_AUTO_TEST_CASE(kokkos_communication_buffer_slabs_do_not_alias) {
  schnek::detail::BufferRegistry registry;
  auto &buffers = registry.get<Kokkos::HostSpace>();
  auto &sendArena = buffers.send();
  auto &recvArena = buffers.receive();

  constexpr std::size_t sendCount = 4;
  constexpr std::size_t recvCount = 3;
  sendArena.reserve_bytes(sendCount * sizeof(int));
  recvArena.reserve_bytes(recvCount * sizeof(int));

  auto sendSlab = sendArena.make_vector<int>(sendCount);
  auto recvSlab = recvArena.make_vector<int>(recvCount);

  const auto sendBegin = reinterpret_cast<std::uintptr_t>(sendSlab.data());
  const auto sendEnd = sendBegin + sendCount * sizeof(int);
  const auto recvBegin = reinterpret_cast<std::uintptr_t>(recvSlab.data());
  const auto recvEnd = recvBegin + recvCount * sizeof(int);
  BOOST_CHECK(sendEnd <= recvBegin || recvEnd <= sendBegin);

  for (std::size_t i = 0; i < sendCount; ++i) {
    sendSlab(i) = static_cast<int>(100 + i);
  }
  for (std::size_t i = 0; i < recvCount; ++i) {
    recvSlab(i) = static_cast<int>(200 + i);
  }
  for (std::size_t i = 0; i < sendCount; ++i) {
    BOOST_CHECK_EQUAL(sendSlab(i), static_cast<int>(100 + i));
  }
}
#endif

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
