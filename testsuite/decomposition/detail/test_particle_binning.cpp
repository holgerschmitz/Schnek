#include <decomposition/detail/particle_binning.hpp>
#include <grid/array.hpp>
#include <grid/range.hpp>
#include <util/exceptions.hpp>

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <vector>

namespace {

using schnek::Array;
using schnek::CellBinning;
using schnek::Range;

using Binning1 = CellBinning<1>;
using Binning2 = CellBinning<2>;
using Region1 = Range<std::ptrdiff_t, 1>;
using Region2 = Range<std::ptrdiff_t, 2>;
using Index1 = Array<std::ptrdiff_t, 1>;
using Index2 = Array<std::ptrdiff_t, 2>;
using Position1 = Array<double, 1>;
using Position2 = Array<double, 2>;

}  // namespace

BOOST_AUTO_TEST_SUITE(domain_decomposition)
BOOST_AUTO_TEST_SUITE(detail)
BOOST_AUTO_TEST_SUITE(particle_binning)

BOOST_AUTO_TEST_CASE(negative_origin_and_partial_last_tile) {
  // Cells -3..4 with tiles of three cells: [-3,-1], [0,2], [3,4]
  const Binning1 binning(Region1(Index1(-3), Index1(4)), Index1(3));

  BOOST_CHECK_EQUAL(binning.binCount(), 3u);
  BOOST_CHECK_EQUAL(binning.tilesPerDimension(0), 3);

  BOOST_CHECK_EQUAL(binning.bin(Position1(-3.0)), 0u);
  BOOST_CHECK_EQUAL(binning.bin(Position1(-2.5)), 0u);
  BOOST_CHECK_EQUAL(binning.bin(Position1(-0.5)), 0u);
  BOOST_CHECK_EQUAL(binning.bin(Position1(0.0)), 1u);
  BOOST_CHECK_EQUAL(binning.bin(Position1(2.99)), 1u);
  BOOST_CHECK_EQUAL(binning.bin(Position1(3.0)), 2u);
  BOOST_CHECK_EQUAL(binning.bin(Position1(4.99)), 2u);
}

BOOST_AUTO_TEST_CASE(positions_outside_region_are_clamped_to_boundary_bins) {
  const Binning1 binning(Region1(Index1(-3), Index1(4)), Index1(3));

  BOOST_CHECK_EQUAL(binning.bin(Position1(-3.5)), 0u);
  BOOST_CHECK_EQUAL(binning.bin(Position1(-100.0)), 0u);
  BOOST_CHECK_EQUAL(binning.bin(Position1(-std::numeric_limits<double>::infinity())), 0u);
  BOOST_CHECK_EQUAL(binning.bin(Position1(5.0)), 2u);
  BOOST_CHECK_EQUAL(binning.bin(Position1(1e300)), 2u);
  BOOST_CHECK_EQUAL(binning.bin(Position1(std::numeric_limits<double>::infinity())), 2u);
  BOOST_CHECK_EQUAL(binning.bin(Position1(std::numeric_limits<double>::quiet_NaN())), 0u);
}

BOOST_AUTO_TEST_CASE(two_dimensions_use_row_major_numbering) {
  // Cells 0..3 x 0..5 with tiles of 2x2 cells: 2 x 3 bins
  const Binning2 binning(Region2(Index2(0, 0), Index2(3, 5)), Index2(2, 2));

  BOOST_CHECK_EQUAL(binning.binCount(), 6u);
  BOOST_CHECK_EQUAL(binning.tilesPerDimension(0), 2);
  BOOST_CHECK_EQUAL(binning.tilesPerDimension(1), 3);

  BOOST_CHECK_EQUAL(binning.bin(Position2(0.0, 0.0)), 0u);
  BOOST_CHECK_EQUAL(binning.bin(Position2(1.0, 2.0)), 1u);
  BOOST_CHECK_EQUAL(binning.bin(Position2(2.0, 0.0)), 3u);
  BOOST_CHECK_EQUAL(binning.bin(Position2(3.0, 5.0)), 5u);
}

BOOST_AUTO_TEST_CASE(tile_larger_than_region_gives_single_bin) {
  const Binning1 binning(Region1(Index1(10), Index1(12)), Index1(100));

  BOOST_CHECK_EQUAL(binning.binCount(), 1u);
  BOOST_CHECK_EQUAL(binning.bin(Position1(10.5)), 0u);
  BOOST_CHECK_EQUAL(binning.bin(Position1(12.9)), 0u);
  BOOST_CHECK_EQUAL(binning.bin(Position1(0.0)), 0u);
  BOOST_CHECK((binning.neighbourhood(0, 5) == std::vector<std::size_t>{0}));
}

BOOST_AUTO_TEST_CASE(one_dimensional_neighbourhood_is_clipped_to_region) {
  // Cells 0..9 with tiles of two cells: 5 bins
  const Binning1 binning(Region1(Index1(0), Index1(9)), Index1(2));
  using Keys = std::vector<std::size_t>;

  BOOST_CHECK_EQUAL(binning.binCount(), 5u);
  BOOST_CHECK((binning.neighbourhood(0, 1) == Keys{0, 1}));
  BOOST_CHECK((binning.neighbourhood(4, 1) == Keys{3, 4}));
  BOOST_CHECK((binning.neighbourhood(2, 0) == Keys{2}));
  BOOST_CHECK((binning.neighbourhood(2, 1) == Keys{1, 2, 3}));
  BOOST_CHECK((binning.neighbourhood(2, 2) == Keys{1, 2, 3}));
  BOOST_CHECK((binning.neighbourhood(2, 3) == Keys{0, 1, 2, 3, 4}));
}

BOOST_AUTO_TEST_CASE(neighbourhood_covers_every_stencil_reach) {
  const std::ptrdiff_t halo = 2;
  const std::ptrdiff_t loX = -2;
  const std::ptrdiff_t hiX = 4;
  const std::ptrdiff_t loY = -3;
  const std::ptrdiff_t hiY = 6;
  const Binning2 binning(Region2(Index2(loX, loY), Index2(hiX, hiY)), Index2(3, 2));

  // Tile counts are 3 x 5, and the halo spans one tile in each direction
  BOOST_CHECK_EQUAL(binning.binCount(), 15u);
  const std::size_t maxSize = 9;

  for (std::ptrdiff_t cx = loX; cx <= hiX; ++cx) {
    for (std::ptrdiff_t cy = loY; cy <= hiY; ++cy) {
      const std::size_t bin = binning.bin(Position2(static_cast<double>(cx), static_cast<double>(cy)));
      const std::vector<std::size_t> hood = binning.neighbourhood(bin, halo);

      std::vector<std::size_t> unique = hood;
      std::sort(unique.begin(), unique.end());
      const bool isUnique = std::adjacent_find(unique.begin(), unique.end()) == unique.end();
      BOOST_CHECK(isUnique);
      BOOST_CHECK_LE(hood.size(), maxSize);
      BOOST_CHECK(std::find(hood.begin(), hood.end(), bin) != hood.end());

      bool covered = true;
      for (std::ptrdiff_t nx = std::max(loX, cx - halo); nx <= std::min(hiX, cx + halo); ++nx) {
        for (std::ptrdiff_t ny = std::max(loY, cy - halo); ny <= std::min(hiY, cy + halo); ++ny) {
          const std::size_t other = binning.bin(Position2(static_cast<double>(nx), static_cast<double>(ny)));
          if (std::find(hood.begin(), hood.end(), other) == hood.end()) {
            covered = false;
          }
        }
      }
      BOOST_CHECK(covered);
    }
  }
}

BOOST_AUTO_TEST_CASE(invalid_geometry_is_rejected) {
  BOOST_CHECK_THROW((void)Binning1(Region1(Index1(3), Index1(2)), Index1(2)), schnek::ScheckException);
  BOOST_CHECK_THROW((void)Binning1(Region1(Index1(0), Index1(4)), Index1(0)), schnek::ScheckException);
  BOOST_CHECK_THROW((void)Binning1(Region1(Index1(0), Index1(4)), Index1(-1)), schnek::ScheckException);
}

BOOST_AUTO_TEST_CASE(invalid_queries_are_rejected) {
  const Binning1 binning(Region1(Index1(0), Index1(9)), Index1(2));

  BOOST_CHECK_THROW((void)binning.neighbourhood(5, 1), schnek::ScheckException);
  BOOST_CHECK_THROW((void)binning.neighbourhood(0, -1), schnek::ScheckException);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
