/*
 * Real MPI integration tests.
 *
 * Build with `cmake --build build --target schnek_mpi_tests`, then run with
 * `mpiexec -n 1 ./build/schnek_mpi_tests`, `mpiexec -n 2 ...`, or
 * `mpiexec -n 4 ...`. CTest registers the two- and four-rank runs.
 */

#include <array>
#include <boost/test/unit_test.hpp>
#include <cmath>
#include <cstddef>
#include <decomposition/mpi_cartesian_decomposition.hpp>
#include <grid/field.hpp>
#include <grid/grid.hpp>
#include <grid/range.hpp>
#include <vector>

namespace {

  using schnek::Array;
  using schnek::Range;

  struct TestParticle {
      double pos[2];
      int id;
  };

  struct PositionAccessor {
      template<typename Particle>
      schnek::Array<double, 2> operator()(const Particle &particle) const {
        return schnek::Array<double, 2>(particle.pos[0], particle.pos[1]);
      }

      template<typename Particle>
      void shift(Particle &particle, std::size_t dim, double delta) const {
        particle.pos[dim] += delta;
      }
  };

  int worldSize() {
    int size = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    return size;
  }

  template<std::size_t Rank>
  void configure(
      schnek::MpiCartesianDomainDecomposition<Rank> &decomposition,
      const Range<ptrdiff_t, Rank> &gridRange,
      const Range<double, Rank> &physicalDomain
  ) {
    decomposition.setGlobalRange(gridRange);
    decomposition.setGlobalDomain(physicalDomain);
    decomposition.init();
  }

  ptrdiff_t periodicIndex(ptrdiff_t index, ptrdiff_t extent) {
    return (index % extent + extent) % extent;
  }

  double fieldValue(ptrdiff_t i, ptrdiff_t j) {
    return static_cast<double>(1000 * i + j);
  }

  double fieldValue(ptrdiff_t i) {
    return static_cast<double>(i * i + 17);
  }

  schnek::Grid<double, 1> localWeights(ptrdiff_t cellCount) {
    schnek::Grid<double, 1> weights(Array<ptrdiff_t, 1>(0), Array<ptrdiff_t, 1>(cellCount - 1));
    for (ptrdiff_t i = 0; i < cellCount; ++i) {
      weights(i) = (i == 0) ? 20.0 : 1.0;
    }
    return weights;
  }

}  // namespace

BOOST_AUTO_TEST_SUITE(real_mpi_domain_decomposition)

BOOST_AUTO_TEST_CASE(periodic_2d_exchange_including_corners) {
  using Field = schnek::Field<double, 2>;
  using RangeType = Range<ptrdiff_t, 2>;
  using DomainType = Range<double, 2>;

  const RangeType globalRange(Array<ptrdiff_t, 2>(0, 0), Array<ptrdiff_t, 2>(7, 7));
  const DomainType globalDomain(Array<double, 2>(0.0, 0.0), Array<double, 2>(8.0, 8.0));
  schnek::MpiContextImpl mpi(MPI_COMM_WORLD);
  schnek::MpiCartesianDomainDecomposition<2> decomposition(mpi);
  configure(decomposition, globalRange, globalDomain);

  if (worldSize() == 4) {
    BOOST_CHECK_EQUAL(decomposition.getProcRanges()[0].getDims()[0], 2);
    BOOST_CHECK_EQUAL(decomposition.getProcRanges()[1].getDims()[0], 2);
  }

  schnek::GridFactory<Field> factory(Array<bool, 2>(false, false), 1);
  const auto registration = decomposition.registerField(factory);
  decomposition.getGridContext({registration}).forEach([&](const RangeType &, Field &field) {
    const auto innerLo = field.getInnerLo();
    const auto innerHi = field.getInnerHi();
    for (ptrdiff_t i = innerLo[0]; i <= innerHi[0]; ++i) {
      for (ptrdiff_t j = innerLo[1]; j <= innerHi[1]; ++j) {
        field(i, j) = fieldValue(i, j);
      }
    }
  });

  decomposition.exchange(registration);
  decomposition.getGridContext({registration}).forEach([&](const RangeType &, Field &field) {
    const auto innerLo = field.getInnerLo();
    const auto innerHi = field.getInnerHi();
    for (ptrdiff_t i = field.getLo()[0]; i <= field.getHi()[0]; ++i) {
      for (ptrdiff_t j = field.getLo()[1]; j <= field.getHi()[1]; ++j) {
        if (i < innerLo[0] || i > innerHi[0] || j < innerLo[1] || j > innerHi[1]) {
          const ptrdiff_t periodicI = periodicIndex(i, 8);
          const ptrdiff_t periodicJ = periodicIndex(j, 8);
          BOOST_CHECK_EQUAL(field(i, j), fieldValue(periodicI, periodicJ));
        }
      }
    }
  });
}

BOOST_AUTO_TEST_CASE(accumulate_applies_neighbour_ghosts_to_interior) {
  using Field = schnek::Field<double, 1>;
  using RangeType = Range<ptrdiff_t, 1>;
  using DomainType = Range<double, 1>;

  const RangeType globalRange(Array<ptrdiff_t, 1>(0), Array<ptrdiff_t, 1>(31));
  const DomainType globalDomain(Array<double, 1>(0.0), Array<double, 1>(32.0));
  schnek::MpiContextImpl mpi(MPI_COMM_WORLD);
  schnek::MpiCartesianDomainDecomposition<1> decomposition(mpi);
  configure(decomposition, globalRange, globalDomain);

  schnek::GridFactory<Field> factory(Array<bool, 1>(false), 1);
  const auto registration = decomposition.registerField(factory);
  const int rank = decomposition.getUniqueId();
  const int size = decomposition.numProcs();
  RangeType localRange;
  decomposition.getGridContext({registration}).forEach([&](const RangeType &range, Field &field) {
    localRange = range;
    const auto innerLo = field.getInnerLo();
    const auto innerHi = field.getInnerHi();
    for (ptrdiff_t i = innerLo[0]; i <= innerHi[0]; ++i) {
      field(i) = fieldValue(i);
    }
    field(innerLo[0] - 1) = 10.0 * (rank + 1);
    field(innerHi[0] + 1) = 100.0 * (rank + 1);
  });

  static_cast<schnek::DomainDecomposition<1> &>(decomposition).accumulate(registration);
  decomposition.getGridContext({registration}).forEach([&](const RangeType &, Field &field) {
    const int previousRank = (rank + size - 1) % size;
    const int nextRank = (rank + 1) % size;
    for (ptrdiff_t i = localRange.getLo()[0]; i <= localRange.getHi()[0]; ++i) {
      double expected = fieldValue(i);
      if (i == localRange.getLo()[0]) {
        expected += 100.0 * (previousRank + 1);
      }
      if (i == localRange.getHi()[0]) {
        expected += 10.0 * (nextRank + 1);
      }
      BOOST_CHECK_EQUAL(field(i), expected);
    }
  });
}

BOOST_AUTO_TEST_CASE(local_weight_balance_preserves_field_values) {
  using Field = schnek::Field<double, 1>;
  using RangeType = Range<ptrdiff_t, 1>;
  using DomainType = Range<double, 1>;

  const RangeType globalRange(Array<ptrdiff_t, 1>(0), Array<ptrdiff_t, 1>(31));
  const DomainType globalDomain(Array<double, 1>(0.0), Array<double, 1>(32.0));
  schnek::MpiContextImpl mpi(MPI_COMM_WORLD);
  schnek::MpiCartesianDomainDecomposition<1> decomposition(mpi);
  configure(decomposition, globalRange, globalDomain);

  schnek::GridFactory<Field> factory(Array<bool, 1>(false), 0);
  const auto registration = decomposition.registerField(factory);
  double localSumBefore = 0.0;
  decomposition.getGridContext({registration}).forEach([&](const RangeType &range, Field &field) {
    for (ptrdiff_t i = range.getLo()[0]; i <= range.getHi()[0]; ++i) {
      field(i) = fieldValue(i);
      localSumBefore += field(i);
    }
  });

  RangeType oldRange;
  decomposition.getGridContext({registration}).forEach([&](const RangeType &range, Field &) { oldRange = range; });
  decomposition.setLocalWeights(localWeights(oldRange.getHi()[0] - oldRange.getLo()[0] + 1));
  const double sumBefore = decomposition.sumReduce(localSumBefore);
  decomposition.balanceLoad();

  double localSumAfter = 0.0;
  decomposition.getGridContext({registration}).forEach([&](const RangeType &range, Field &field) {
    for (ptrdiff_t i = range.getLo()[0]; i <= range.getHi()[0]; ++i) {
      BOOST_CHECK_EQUAL(field(i), fieldValue(i));
      localSumAfter += field(i);
    }
  });
  const double sumAfter = decomposition.sumReduce(localSumAfter);
  BOOST_CHECK_EQUAL(sumAfter, sumBefore);
}

BOOST_AUTO_TEST_CASE(periodic_particle_migration_crosses_corner) {
  using RangeType = Range<ptrdiff_t, 2>;
  using DomainType = Range<double, 2>;
  using Container = std::vector<TestParticle>;

  const RangeType globalRange(Array<ptrdiff_t, 2>(0, 0), Array<ptrdiff_t, 2>(7, 7));
  const DomainType globalDomain(Array<double, 2>(0.0, 0.0), Array<double, 2>(8.0, 8.0));
  schnek::MpiContextImpl mpi(MPI_COMM_WORLD);
  schnek::MpiCartesianDomainDecomposition<2> decomposition(mpi);
  configure(decomposition, globalRange, globalDomain);

  schnek::ParticleContainerFactory<Container> factory;
  const auto registration = decomposition.registerParticleData(factory, PositionAccessor{});
  decomposition.getGridContext({registration}).forEach([&](const RangeType &range, Container &particles) {
    if (range.getHi()[0] == 7 && range.getHi()[1] == 7) {
      particles.push_back(TestParticle{{8.25, 8.75}, 42});
    }
  });

  decomposition.migrate(registration);
  int localParticleCount = 0;
  int localBadParticleCount = 0;
  decomposition.getGridContext({registration}).forEach([&](const RangeType &range, Container &particles) {
    localParticleCount = static_cast<int>(particles.size());
    const bool ownsWrappedCorner = range.getLo()[0] == 0 && range.getLo()[1] == 0;
    BOOST_CHECK_EQUAL(particles.size(), ownsWrappedCorner ? 1u : 0u);
    for (const auto &particle : particles) {
      if (particle.id != 42 || std::abs(particle.pos[0] - 0.25) > 1e-12 || std::abs(particle.pos[1] - 0.75) > 1e-12) {
        ++localBadParticleCount;
      }
    }
  });

  int globalParticleCount = 0;
  int globalBadParticleCount = 0;
  MPI_Allreduce(&localParticleCount, &globalParticleCount, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
  MPI_Allreduce(&localBadParticleCount, &globalBadParticleCount, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
  BOOST_CHECK_EQUAL(globalParticleCount, 1);
  BOOST_CHECK_EQUAL(globalBadParticleCount, 0);
}

BOOST_AUTO_TEST_CASE(subrange_field_keeps_data_after_local_weight_balance) {
  using Field = schnek::Field<double, 1>;
  using RangeType = Range<ptrdiff_t, 1>;
  using DomainType = Range<double, 1>;

  const RangeType globalRange(Array<ptrdiff_t, 1>(0), Array<ptrdiff_t, 1>(31));
  const DomainType globalDomain(Array<double, 1>(0.0), Array<double, 1>(32.0));
  const RangeType subRange(Array<ptrdiff_t, 1>(3), Array<ptrdiff_t, 1>(24));
  schnek::MpiContextImpl mpi(MPI_COMM_WORLD);
  schnek::MpiCartesianDomainDecomposition<1> decomposition(mpi);
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  schnek::GridFactory<Field> factory(Array<bool, 1>(false), 0);
  const auto registration = decomposition.registerField(factory, subRange);
  decomposition.init();

  decomposition.getGridContext({registration}).forEach([&](const RangeType &range, Field &field) {
    for (ptrdiff_t i = range.getLo()[0]; i <= range.getHi()[0]; ++i) {
      field(i) = fieldValue(i);
    }
  });
  const auto localRange = decomposition.getProcRanges()[0](decomposition.getUniqueId());
  decomposition.setLocalWeights(localWeights(localRange.getHi()[0] - localRange.getLo()[0] + 1));
  decomposition.balanceLoad();

  int localCellCount = 0;
  decomposition.getGridContext({registration}).forEach([&](const RangeType &range, Field &field) {
    BOOST_CHECK_GE(range.getLo()[0], subRange.getLo()[0]);
    BOOST_CHECK_LE(range.getHi()[0], subRange.getHi()[0]);
    for (ptrdiff_t i = range.getLo()[0]; i <= range.getHi()[0]; ++i) {
      BOOST_CHECK_EQUAL(field(i), fieldValue(i));
      ++localCellCount;
    }
  });
  int globalCellCount = 0;
  MPI_Allreduce(&localCellCount, &globalCellCount, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
  BOOST_CHECK_EQUAL(globalCellCount, 22);
}

BOOST_AUTO_TEST_CASE(subrange_exchange_and_accumulate_fail_consistently) {
  using Field = schnek::Field<double, 1>;
  using RangeType = Range<ptrdiff_t, 1>;
  using DomainType = Range<double, 1>;

  const RangeType globalRange(Array<ptrdiff_t, 1>(0), Array<ptrdiff_t, 1>(31));
  const DomainType globalDomain(Array<double, 1>(0.0), Array<double, 1>(32.0));
  const RangeType subRange(Array<ptrdiff_t, 1>(0), Array<ptrdiff_t, 1>(7));
  schnek::MpiContextImpl mpi(MPI_COMM_WORLD);
  schnek::MpiCartesianDomainDecomposition<1> decomposition(mpi);
  configure(decomposition, globalRange, globalDomain);

  schnek::GridFactory<Field> factory(Array<bool, 1>(false), 1);
  const auto registration = decomposition.registerField(factory, subRange);

  int localCaught = 0;
  try {
    decomposition.exchange(registration);
  } catch (const schnek::ScheckException &) {
    localCaught = 1;
  }

  int caughtOnEveryRank = 0;
  MPI_Allreduce(&localCaught, &caughtOnEveryRank, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
  BOOST_CHECK_EQUAL(caughtOnEveryRank, 1);

  localCaught = 0;
  try {
    static_cast<schnek::DomainDecomposition<1> &>(decomposition).accumulate(registration);
  } catch (const schnek::ScheckException &) {
    localCaught = 1;
  }

  MPI_Allreduce(&localCaught, &caughtOnEveryRank, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
  BOOST_CHECK_EQUAL(caughtOnEveryRank, 1);
}

BOOST_AUTO_TEST_CASE(too_small_subdomain_throws_on_every_rank) {
  const int size = worldSize();
  if (size < 2) {
    BOOST_CHECK_EQUAL(size, 1);
    return;
  }

  using RangeType = Range<ptrdiff_t, 1>;
  using DomainType = Range<double, 1>;
  const RangeType globalRange(Array<ptrdiff_t, 1>(0), Array<ptrdiff_t, 1>(0));
  const DomainType globalDomain(Array<double, 1>(0.0), Array<double, 1>(1.0));
  schnek::MpiContextImpl mpi(MPI_COMM_WORLD);
  schnek::MpiCartesianDomainDecomposition<1> decomposition(mpi);
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(globalDomain);

  int localCaught = 0;
  try {
    decomposition.init();
  } catch (const schnek::ScheckException &) {
    localCaught = 1;
  }

  int caughtOnEveryRank = 0;
  MPI_Allreduce(&localCaught, &caughtOnEveryRank, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
  BOOST_CHECK_EQUAL(caughtOnEveryRank, 1);
}

BOOST_AUTO_TEST_SUITE_END()
