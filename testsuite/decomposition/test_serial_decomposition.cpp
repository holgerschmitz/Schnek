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

  struct SerialWrapParticle {
      std::array<double, 1> position;
      int id;
  };

  struct SerialNoShiftParticle {
      double position[1];
      int id;
  };

  struct SerialNoShiftAccessor {
      SCHNEK_FUNCTION schnek::Array<double, 1> operator()(const SerialNoShiftParticle &particle) const {
        return schnek::Array<double, 1>(particle.position[0]);
      }
  };

  using SerialWrapAccessor = schnek::MemberPositionAccessor<&SerialWrapParticle::position>;

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
  void fillField(FieldType &field, typename FieldType::value_type value) {
    typename FieldType::RangeType range(field.getLo(), field.getHi());
    for (auto it = range.begin(); it != range.end(); ++it) {
      field[*it] = value;
    }
  }

  template<typename FieldType>
  FieldType deepCopyField(FieldType &field) {
    auto innerLo = field.getInnerLo();
    auto innerHi = field.getInnerHi();
    const size_t ghostCells = static_cast<size_t>(innerLo[0] - field.getLo()[0]);
    FieldType copy(innerLo, innerHi, field.getDomain(), field.getStagger(), ghostCells);

    typename FieldType::RangeType range(field.getLo(), field.getHi());
    for (auto it = range.begin(); it != range.end(); ++it) {
      copy[*it] = field[*it];
    }
    return copy;
  }

  template<typename FieldType>
  struct GhostAccumulateSetupResult {
      FieldType expectedField;
  };

  template<typename FieldType>
  GhostAccumulateSetupResult<FieldType> buildGhostAccumulateSetup(FieldType &field) {
    using RangeType = typename FieldType::RangeType;
    using IndexType = typename FieldType::IndexType;

    GhostAccumulateSetupResult<FieldType> result{deepCopyField(field)};
    IndexType gridLo = field.getLo();
    IndexType gridHi = field.getHi();
    IndexType innerLo = field.getInnerLo();
    IndexType innerHi = field.getInnerHi();

    constexpr size_t dims = RangeType::LimitType::length;
    auto makeRange = [](const IndexType &lo, const IndexType &hi) { return RangeType(lo, hi); };

    for (size_t dim = 0; dim < dims; ++dim) {
      ptrdiff_t lowerHalo = innerLo[dim] - gridLo[dim];
      ptrdiff_t upperHalo = gridHi[dim] - innerHi[dim];

      std::optional<RangeType> loGhostRange;
      std::optional<RangeType> hiGhostRange;
      std::optional<RangeType> hiSourceRange;
      std::optional<RangeType> loSourceRange;

      if (lowerHalo > 0) {
        IndexType lo = gridLo;
        IndexType hi = gridHi;
        hi[dim] = gridLo[dim] + lowerHalo - 1;
        loGhostRange = makeRange(lo, hi);

        lo[dim] = innerHi[dim] - lowerHalo + 1;
        hi[dim] = innerHi[dim];
        hiSourceRange = makeRange(lo, hi);
      }

      if (upperHalo > 0) {
        IndexType lo = gridLo;
        IndexType hi = gridHi;
        lo[dim] = gridHi[dim] - upperHalo + 1;
        hiGhostRange = makeRange(lo, hi);

        lo = gridLo;
        hi = gridHi;
        lo[dim] = innerLo[dim];
        hi[dim] = innerLo[dim] + upperHalo - 1;
        loSourceRange = makeRange(lo, hi);
      }

      if (loGhostRange && hiSourceRange) {
        auto itGhost = loGhostRange->begin();
        auto itSource = hiSourceRange->begin();
        for (; itGhost != loGhostRange->end(); ++itGhost, ++itSource) {
          result.expectedField[*itGhost] += result.expectedField[*itSource];
          result.expectedField[*itSource] = result.expectedField[*itGhost];
        }
      }

      if (hiGhostRange && loSourceRange) {
        auto itGhost = hiGhostRange->begin();
        auto itSource = loSourceRange->begin();
        for (; itGhost != hiGhostRange->end(); ++itGhost, ++itSource) {
          result.expectedField[*itGhost] += result.expectedField[*itSource];
          result.expectedField[*itSource] = result.expectedField[*itGhost];
        }
      }
    }

    return result;
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

BOOST_AUTO_TEST_CASE( migrate_wraps_periodic_particle_positions )
{
  using Container = std::vector<SerialWrapParticle>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;

  const RangeType globalRange(schnek::Array<ptrdiff_t, 1>(0), schnek::Array<ptrdiff_t, 1>(9));
  const schnek::Range<double, 1> globalDomain(
      schnek::Array<double, 1>(0.0), schnek::Array<double, 1>(1.0)
  );
  schnek::SerialDomainDecomposition<1> decomposition;
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);
  decomposition.init();

  schnek::ParticleContainerFactory<Container> factory;
  const SerialWrapAccessor accessor;
  decomposition.registerParticleData(factory, accessor);

  Container particles{
      SerialWrapParticle{{-0.5}, 1},
      SerialWrapParticle{{-0.01}, 2},
      SerialWrapParticle{{9.99}, 3},
      SerialWrapParticle{{10.3}, 4},
      SerialWrapParticle{{34.0}, 5}
  };
  auto wrapper = std::make_shared<schnek::internal::ParticleWrapperImpl<Container, SerialWrapAccessor>>(
      std::move(particles), accessor
  );
  decomposition.migrateParticles(wrapper);

  BOOST_REQUIRE_EQUAL(wrapper->container.size(), 5u);
  BOOST_CHECK_CLOSE(wrapper->container[0].position[0], 9.5, 1e-12);
  BOOST_CHECK_CLOSE(wrapper->container[1].position[0], 9.99, 1e-12);
  BOOST_CHECK_CLOSE(wrapper->container[2].position[0], 9.99, 1e-12);
  BOOST_CHECK_CLOSE(wrapper->container[3].position[0], 0.3, 1e-12);
  BOOST_CHECK_CLOSE(wrapper->container[4].position[0], 4.0, 1e-12);
}

BOOST_AUTO_TEST_CASE( migrate_rejects_out_of_domain_particle_without_shift )
{
  using Container = std::vector<SerialNoShiftParticle>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;

  const RangeType globalRange(schnek::Array<ptrdiff_t, 1>(0), schnek::Array<ptrdiff_t, 1>(9));
  const schnek::Range<double, 1> globalDomain(
      schnek::Array<double, 1>(0.0), schnek::Array<double, 1>(1.0)
  );
  schnek::SerialDomainDecomposition<1> decomposition;
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);
  decomposition.init();

  schnek::ParticleContainerFactory<Container> factory;
  const SerialNoShiftAccessor accessor;
  decomposition.registerParticleData(factory, accessor);
  auto wrapper = std::make_shared<schnek::internal::ParticleWrapperImpl<Container, SerialNoShiftAccessor>>(
      Container{SerialNoShiftParticle{{10.5}, 1}}, accessor
  );

  BOOST_CHECK_EXCEPTION(
      decomposition.migrateParticles(wrapper),
      schnek::ScheckException,
      [](const schnek::ScheckException &error) {
        return std::string(error.what()).find(
                   "Particle left the global domain and the position accessor has no shift()"
               ) != std::string::npos;
      }
  );
}

BOOST_AUTO_TEST_CASE( migrate_rejects_excessive_periodic_wrap_count )
{
  using Container = std::vector<SerialWrapParticle>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;

  const RangeType globalRange(schnek::Array<ptrdiff_t, 1>(0), schnek::Array<ptrdiff_t, 1>(9));
  const schnek::Range<double, 1> globalDomain(
      schnek::Array<double, 1>(0.0), schnek::Array<double, 1>(1.0)
  );
  schnek::SerialDomainDecomposition<1> decomposition;
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);
  decomposition.init();

  schnek::ParticleContainerFactory<Container> factory;
  const SerialWrapAccessor accessor;
  decomposition.registerParticleData(factory, accessor);
  auto wrapper = std::make_shared<schnek::internal::ParticleWrapperImpl<Container, SerialWrapAccessor>>(
      Container{SerialWrapParticle{{10737418240.0}, 1}}, accessor
  );

  BOOST_CHECK_EXCEPTION(
      decomposition.migrateParticles(wrapper),
      schnek::ScheckException,
      [](const schnek::ScheckException &error) {
        return std::string(error.what()).find("Particle periodic wrap count exceeds the supported magnitude") !=
               std::string::npos;
      }
  );
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

  exchangeRegistration<1>(decomposition, registration, false);  
  
  typename FieldType::RangeType verificationRange(field->getLo(), field->getHi());
  for (auto it = verificationRange.begin(); it != verificationRange.end(); ++it) {
    BOOST_CHECK_EQUAL((*field)[*it], -42.0);
  }
}

BOOST_AUTO_TEST_CASE( ghost_exchange_uses_opposite_interior_edge_1d )
{
  using FieldType = schnek::Field<double, 1>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;
  using DomainType = schnek::Range<double, 1>;
  using StaggerType = typename FieldType::StaggerType;

  RangeType globalRange(schnek::Array<ptrdiff_t, 1>(0), schnek::Array<ptrdiff_t, 1>(7));
  DomainType globalDomain(schnek::Array<double, 1>(0.0), schnek::Array<double, 1>(1.0));

  schnek::SerialDomainDecomposition<1> decomposition;
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);
  decomposition.init();

  StaggerType noStagger(false);
  schnek::GridFactory<FieldType> factory(noStagger, 2);
  schnek::GridRegistration registration = decomposition.registerField(factory);
  auto gridContext = decomposition.getGridContext({registration});
  FieldType *field = nullptr;
  gridContext.forEach([&](const RangeType &, FieldType &localField) { field = &localField; });
  BOOST_REQUIRE(field != nullptr);

  fillField(*field, -1.0);
  for (ptrdiff_t i = 0; i <= 7; ++i) {
    (*field)[i] = 100.0 + i;
  }

  exchangeRegistration<1>(decomposition, registration, false);

  BOOST_CHECK_EQUAL((*field)[-2], 106.0);
  BOOST_CHECK_EQUAL((*field)[-1], 107.0);
  BOOST_CHECK_EQUAL((*field)[8], 100.0);
  BOOST_CHECK_EQUAL((*field)[9], 101.0);
}

BOOST_AUTO_TEST_CASE( ghost_exchange_fills_periodic_2d_corners )
{
  using FieldType = schnek::Field<double, 2>;
  using RangeType = schnek::Range<ptrdiff_t, 2>;
  using DomainType = schnek::Range<double, 2>;
  using StaggerType = typename FieldType::StaggerType;

  RangeType globalRange(schnek::Array<ptrdiff_t, 2>(0, 0), schnek::Array<ptrdiff_t, 2>(3, 2));
  DomainType globalDomain(schnek::Array<double, 2>(0.0, 0.0), schnek::Array<double, 2>(1.0, 1.0));

  schnek::SerialDomainDecomposition<2> decomposition;
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);
  decomposition.init();

  StaggerType noStagger(false);
  schnek::GridFactory<FieldType> factory(noStagger, 1);
  schnek::GridRegistration registration = decomposition.registerField(factory);
  auto gridContext = decomposition.getGridContext({registration});
  FieldType *field = nullptr;
  gridContext.forEach([&](const RangeType &, FieldType &localField) { field = &localField; });
  BOOST_REQUIRE(field != nullptr);

  fillField(*field, -1.0);
  for (ptrdiff_t i = 0; i <= 3; ++i) {
    for (ptrdiff_t j = 0; j <= 2; ++j) {
      (*field)[schnek::Array<ptrdiff_t, 2>(i, j)] = 1000.0 * i + j;
    }
  }

  exchangeRegistration<2>(decomposition, registration, false);

  BOOST_CHECK_EQUAL((*field)(-1, -1), 3002.0);
  BOOST_CHECK_EQUAL((*field)(-1, 3), 3000.0);
  BOOST_CHECK_EQUAL((*field)(4, -1), 2.0);
  BOOST_CHECK_EQUAL((*field)(4, 3), 0.0);
  BOOST_CHECK_EQUAL((*field)(-1, 0), 3000.0);
  BOOST_CHECK_EQUAL((*field)(4, 1), 1.0);
}

BOOST_AUTO_TEST_CASE( ghost_accumulate_combines_periodic_images_1d )
{
  using FieldType = schnek::Field<double, 1>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;
  using DomainType = schnek::Range<double, 1>;
  using StaggerType = typename FieldType::StaggerType;

  RangeType globalRange(schnek::Array<ptrdiff_t, 1>(0), schnek::Array<ptrdiff_t, 1>(7));
  DomainType globalDomain(schnek::Array<double, 1>(0.0), schnek::Array<double, 1>(1.0));

  schnek::SerialDomainDecomposition<1> decomposition;
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);
  decomposition.init();

  StaggerType noStagger(false);
  schnek::GridFactory<FieldType> factory(noStagger, 2);
  schnek::GridRegistration registration = decomposition.registerField(factory);
  auto gridContext = decomposition.getGridContext({registration});
  FieldType *field = nullptr;
  gridContext.forEach([&](const RangeType &, FieldType &localField) { field = &localField; });
  BOOST_REQUIRE(field != nullptr);

  fillField(*field, -1.0);
  for (ptrdiff_t i = 0; i <= 7; ++i) {
    (*field)[i] = i + 1.0;
  }
  (*field)[-2] = 10.0;
  (*field)[-1] = 20.0;
  (*field)[8] = 80.0;
  (*field)[9] = 90.0;

  accumulateRegistration<1>(decomposition, registration, false);

  BOOST_CHECK_EQUAL((*field)[-2], 17.0);
  BOOST_CHECK_EQUAL((*field)[-1], 28.0);
  BOOST_CHECK_EQUAL((*field)[0], 81.0);
  BOOST_CHECK_EQUAL((*field)[1], 92.0);
  BOOST_CHECK_EQUAL((*field)[6], 17.0);
  BOOST_CHECK_EQUAL((*field)[7], 28.0);
  BOOST_CHECK_EQUAL((*field)[8], 81.0);
  BOOST_CHECK_EQUAL((*field)[9], 92.0);
}

BOOST_AUTO_TEST_CASE( asymmetric_grid_halos_remain_paired_1d )
{
  using GridType = schnek::Grid<double, 1>;
  using RangeType = schnek::Range<ptrdiff_t, 1>;
  using DomainType = schnek::Range<double, 1>;

  RangeType globalRange(schnek::Array<ptrdiff_t, 1>(0), schnek::Array<ptrdiff_t, 1>(7));
  DomainType globalDomain(schnek::Array<double, 1>(0.0), schnek::Array<double, 1>(1.0));

  schnek::SerialDomainDecomposition<1> decomposition;
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);
  decomposition.init();

  schnek::GridFactory<GridType> factory;
  schnek::GridRegistration registration = decomposition.registerField(factory);
  auto gridContext = decomposition.getGridContext({registration});
  GridType *grid = nullptr;
  gridContext.forEach([&](const RangeType &, GridType &localGrid) { grid = &localGrid; });
  BOOST_REQUIRE(grid != nullptr);
  grid->resize(schnek::Array<ptrdiff_t, 1>(-1), schnek::Array<ptrdiff_t, 1>(9));

  fillField(*grid, -1.0);
  for (ptrdiff_t i = 0; i <= 7; ++i) {
    (*grid)[i] = 100.0 + i;
  }
  exchangeRegistration<1>(decomposition, registration, false);
  BOOST_CHECK_EQUAL((*grid)[-1], 107.0);
  BOOST_CHECK_EQUAL((*grid)[8], 100.0);
  BOOST_CHECK_EQUAL((*grid)[9], 101.0);

  fillField(*grid, -1.0);
  for (ptrdiff_t i = 0; i <= 7; ++i) {
    (*grid)[i] = i + 1.0;
  }
  (*grid)[-1] = 10.0;
  (*grid)[8] = 80.0;
  (*grid)[9] = 90.0;
  accumulateRegistration<1>(decomposition, registration, false);
  BOOST_CHECK_EQUAL((*grid)[-1], 18.0);
  BOOST_CHECK_EQUAL((*grid)[7], 18.0);
  BOOST_CHECK_EQUAL((*grid)[8], 81.0);
  BOOST_CHECK_EQUAL((*grid)[9], 92.0);
  BOOST_CHECK_EQUAL((*grid)[0], 81.0);
  BOOST_CHECK_EQUAL((*grid)[1], 92.0);
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

    exchangeRegistration<2>(decomposition, registration, false);

    typename FieldType::RangeType verificationRange(field->getLo(), field->getHi());
    for (auto it = verificationRange.begin(); it != verificationRange.end(); ++it) {
      BOOST_CHECK_EQUAL((*field)[*it], -13.0);
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

    exchangeRegistration<3>(decomposition, registration, false);

    typename FieldType::RangeType verificationRange(field->getLo(), field->getHi());
    for (auto it = verificationRange.begin(); it != verificationRange.end(); ++it) {
      BOOST_CHECK_EQUAL((*field)[*it], -7.0);
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
