#include <algorithm>
#include <array>
#include <boost/test/unit_test.hpp>
#include <cstddef>
#include <cstdlib>
#include <particle/dense_kokkos_particle_container.hpp>
#include <decomposition/detail/particle_factory.hpp>
#include <decomposition/detail/particle_wrap.hpp>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

  struct DenseTestParticle {
      int id;
      double x;

      bool operator==(const DenseTestParticle &other) const { return id == other.id && x == other.x; }
  };

  struct AddToX {
      KOKKOS_INLINE_FUNCTION void operator()(DenseTestParticle &p) const { p.x += 2.0; }
  };

  struct DensePositionValue {
      double value;
      KOKKOS_INLINE_FUNCTION double operator[](std::size_t) const { return value; }
  };

  struct DensePositionAccessor {
      KOKKOS_INLINE_FUNCTION DensePositionValue operator()(const DenseTestParticle &p) const { return {p.x}; }
      KOKKOS_INLINE_FUNCTION void shift(DenseTestParticle &p, std::size_t, double delta) const { p.x += delta; }
  };

  using HostDenseContainer = schnek::DenseKokkosParticleContainer<DenseTestParticle, Kokkos::Serial, Kokkos::HostSpace>;
  using DefaultDenseContainer = schnek::DenseKokkosParticleContainer<DenseTestParticle>;

  DenseTestParticle makeParticle(int id) {
    return DenseTestParticle{id, 0.25 * id};
  }

  std::vector<DenseTestParticle> sequence(int first, int count) {
    std::vector<DenseTestParticle> values;
    for (int id = first; id < first + count; ++id) {
      values.push_back(makeParticle(id));
    }
    return values;
  }

  // Device-resident checks run only when the GPU test gate is set, like the other CUDA tests.
  bool deviceTestsEnabled() {
    return std::getenv("SCHNEK_RUN_CUDA_DEVICE_TESTS") != nullptr;
  }

  static_assert(HostDenseContainer::host_accessible, "HostSpace storage must be host accessible");
  static_assert(std::is_same_v<HostDenseContainer::ExecutionSpace, Kokkos::Serial>);
  static_assert(std::is_same_v<DefaultDenseContainer::ExecutionSpace, Kokkos::DefaultExecutionSpace>);
  static_assert(
      schnek::ParticleContainerTraits<HostDenseContainer>::kokkos_dispatch,
      "the dense container must select the Kokkos dispatch path"
  );
  static_assert(std::is_same_v<schnek::ParticleContainerTraits<HostDenseContainer>::execution_space, Kokkos::Serial>);

  template<class Container>
  void checkGrowthPreservesParticles() {
    Container c;
    for (const auto &p : sequence(0, 1000)) {
      c.push_back(p);
    }

    BOOST_REQUIRE_EQUAL(c.size(), 1000u);
    BOOST_REQUIRE_GE(c.capacity(), 1000u);
    // Geometric growth from an empty container: 1, 2, 4, ..., 1024.
    BOOST_CHECK_GT(c.reallocationCount(), 0u);
    BOOST_CHECK_LE(c.reallocationCount(), 11u);
    BOOST_CHECK(c.copyToHost() == sequence(0, 1000));
  }

  template<class Container>
  void checkResizeWithinCapacityKeepsStorage() {
    Container c;
    c.reserve(64);
    BOOST_CHECK_EQUAL(c.reallocationCount(), 1u);
    BOOST_CHECK_EQUAL(c.size(), 0u);

    auto *base = c.data();
    for (const auto &p : sequence(0, 40)) {
      c.push_back(p);
    }
    BOOST_CHECK(c.data() == base);
    BOOST_CHECK_EQUAL(c.reallocationCount(), 1u);

    c.resize(10);
    BOOST_CHECK_EQUAL(c.size(), 10u);
    BOOST_CHECK(c.data() == base);
    BOOST_CHECK(c.copyToHost() == sequence(0, 10));

    c.resize(50);
    BOOST_CHECK_EQUAL(c.size(), 50u);
    BOOST_CHECK(c.data() == base);
    BOOST_CHECK_EQUAL(c.reallocationCount(), 1u);
    const auto values = c.copyToHost();
    BOOST_CHECK(std::equal(values.begin(), values.begin() + 10, sequence(0, 10).begin()));

    c.resize(65);
    BOOST_CHECK_GE(c.capacity(), 65u);
    BOOST_CHECK_EQUAL(c.reallocationCount(), 2u);
    BOOST_CHECK_EQUAL(c.size(), 65u);
  }

  // Random pushes and tail truncations, compared with a host reference after every step.
  template<class Container>
  void checkRandomOperationsMatchReference() {
    std::mt19937 rng(20260701u);
    std::uniform_int_distribution<int> choice(0, 9);
    std::uniform_int_distribution<int> burst(1, 40);

    Container c;
    std::vector<DenseTestParticle> reference;
    int nextId = 0;
    for (int step = 0; step < 300; ++step) {
      const std::size_t before = c.reallocationCount();
      if (choice(rng) < 6) {
        const int count = burst(rng);
        for (const auto &p : sequence(nextId, count)) {
          c.push_back(p);
          reference.push_back(p);
        }
        nextId += count;
        BOOST_REQUIRE_GE(c.reallocationCount(), before);
      } else {
        std::uniform_int_distribution<std::size_t> keep(0, reference.size());
        const std::size_t n = keep(rng);
        c.resize(n);
        reference.resize(n);
        // Truncation never reallocates.
        BOOST_REQUIRE_EQUAL(c.reallocationCount(), before);
      }
      BOOST_REQUIRE_EQUAL(c.size(), reference.size());
      BOOST_REQUIRE_LE(c.size(), c.capacity());
      BOOST_REQUIRE(c.copyToHost() == reference);
    }
    BOOST_CHECK_GT(c.reallocationCount(), 1u);
  }

  template<class Container>
  void checkCopyAndMoveAreDeep() {
    Container a;
    for (const auto &p : sequence(0, 10)) {
      a.push_back(p);
    }

    Container copied(a);
    a.resize(0);
    a.push_back(makeParticle(99));
    BOOST_CHECK(copied.copyToHost() == sequence(0, 10));
    BOOST_CHECK(a.copyToHost() == sequence(99, 1));

    Container assigned;
    assigned = copied;
    copied.resize(3);
    BOOST_CHECK(assigned.copyToHost() == sequence(0, 10));

    Container moved(std::move(assigned));
    BOOST_CHECK_EQUAL(assigned.size(), 0u);
    BOOST_CHECK_EQUAL(assigned.capacity(), 0u);
    BOOST_CHECK(moved.copyToHost() == sequence(0, 10));
  }

}  // namespace

BOOST_AUTO_TEST_SUITE(domain_decomposition)
BOOST_AUTO_TEST_SUITE(detail)
BOOST_AUTO_TEST_SUITE(dense_kokkos_particle_container)

BOOST_AUTO_TEST_CASE(host_container_element_access) {
  HostDenseContainer c;
  c.push_back(makeParticle(1));
  c.push_back(makeParticle(2));

  BOOST_CHECK_EQUAL(c[0].id, 1);
  BOOST_CHECK_EQUAL(c[1].id, 2);
  c[1] = makeParticle(7);
  BOOST_CHECK(c.copyToHost() == (std::vector<DenseTestParticle>{makeParticle(1), makeParticle(7)}));

  std::vector<int> ids;
  for (const auto &p : c) {
    ids.push_back(p.id);
  }
  const std::vector<int> expected{1, 7};
  BOOST_CHECK_EQUAL_COLLECTIONS(ids.begin(), ids.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(for_each_particle_updates_host_and_device_containers) {
  using Traits = schnek::ParticleContainerTraits<HostDenseContainer>;
  HostDenseContainer host;
  host.push_back(DenseTestParticle{1, 0.5});
  host.push_back(DenseTestParticle{2, 1.5});
  Traits::forEachParticle(host, AddToX{});
  BOOST_CHECK(host.copyToHost() == (std::vector<DenseTestParticle>{{1, 2.5}, {2, 3.5}}));

  if (deviceTestsEnabled()) {
    DefaultDenseContainer device;
    device.push_back(DenseTestParticle{3, 2.5});
    device.push_back(DenseTestParticle{4, 3.5});
    schnek::ParticleContainerTraits<DefaultDenseContainer>::forEachParticle(device, AddToX{});
    BOOST_CHECK(device.copyToHost() == (std::vector<DenseTestParticle>{{3, 4.5}, {4, 5.5}}));
  }
}

BOOST_AUTO_TEST_CASE(periodic_wrap_functor_runs_on_dense_kokkos_containers) {
  const DensePositionAccessor accessor;
  const schnek::internal::WrapDim<DenseTestParticle, DensePositionAccessor> wrap{accessor, 0, 0.0, 9.0, 10.0};
  HostDenseContainer host;
  host.push_back(DenseTestParticle{1, -0.5});
  host.push_back(DenseTestParticle{2, 10.3});
  schnek::ParticleContainerTraits<HostDenseContainer>::forEachParticle(host, wrap);
  auto hostResult = host.copyToHost();
  BOOST_CHECK_CLOSE(hostResult[0].x, 9.5, 1e-12);
  BOOST_CHECK_CLOSE(hostResult[1].x, 0.3, 1e-12);

  if (deviceTestsEnabled()) {
    DefaultDenseContainer device;
    device.push_back(DenseTestParticle{3, -0.5});
    device.push_back(DenseTestParticle{4, 10.3});
    schnek::ParticleContainerTraits<DefaultDenseContainer>::forEachParticle(device, wrap);
    const auto deviceResult = device.copyToHost();
    BOOST_CHECK_CLOSE(deviceResult[0].x, 9.5, 1e-12);
    BOOST_CHECK_CLOSE(deviceResult[1].x, 0.3, 1e-12);
  }
}

BOOST_AUTO_TEST_CASE(host_growth_preserves_particles) {
  checkGrowthPreservesParticles<HostDenseContainer>();
}

BOOST_AUTO_TEST_CASE(host_resize_within_capacity_keeps_storage) {
  checkResizeWithinCapacityKeepsStorage<HostDenseContainer>();
}

BOOST_AUTO_TEST_CASE(host_random_operations_match_reference) {
  checkRandomOperationsMatchReference<HostDenseContainer>();
}

BOOST_AUTO_TEST_CASE(host_copy_and_move_are_deep) {
  checkCopyAndMoveAreDeep<HostDenseContainer>();
}

BOOST_AUTO_TEST_CASE(default_space_growth_preserves_particles) {
  if (!deviceTestsEnabled()) {
    return;
  }
  checkGrowthPreservesParticles<DefaultDenseContainer>();
}

BOOST_AUTO_TEST_CASE(default_space_resize_within_capacity_keeps_storage) {
  if (!deviceTestsEnabled()) {
    return;
  }
  checkResizeWithinCapacityKeepsStorage<DefaultDenseContainer>();
}

BOOST_AUTO_TEST_CASE(default_space_random_operations_match_reference) {
  if (!deviceTestsEnabled()) {
    return;
  }
  checkRandomOperationsMatchReference<DefaultDenseContainer>();
}

BOOST_AUTO_TEST_CASE(default_space_copy_and_move_are_deep) {
  if (!deviceTestsEnabled()) {
    return;
  }
  checkCopyAndMoveAreDeep<DefaultDenseContainer>();
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
