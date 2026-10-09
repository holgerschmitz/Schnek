#include <algorithm>
#include <array>
#include <boost/test/unit_test.hpp>
#include <cstddef>
#include <cstdlib>
#include <decomposition/detail/device_blocked_particle_container.hpp>
#include <decomposition/detail/particle_factory.hpp>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

  struct BlockTestParticle {
      int id;
      double x;
  };

  using HostBlocked = schnek::DeviceBlockedParticleContainer<BlockTestParticle, Kokkos::Serial, Kokkos::HostSpace>;
  using DefaultBlocked = schnek::DeviceBlockedParticleContainer<BlockTestParticle>;
  using Serializer = schnek::ParticleSerializer<BlockTestParticle>;

  bool deviceTestsEnabled() {
    return std::getenv("SCHNEK_RUN_CUDA_DEVICE_TESTS") != nullptr;
  }

  // Bucket 0 keeps the particle, buckets 1 and 2 remove it.
  struct ModThreeClassifier {
      KOKKOS_INLINE_FUNCTION int operator()(const BlockTestParticle &p) const { return p.id % 3; }
  };

  struct KeepFirstTwoClassifier {
      KOKKOS_INLINE_FUNCTION int operator()(const BlockTestParticle &p) const { return p.id < 2 ? 0 : 1; }
  };

  struct KeepAllClassifier {
      KOKKOS_INLINE_FUNCTION int operator()(const BlockTestParticle &) const { return 0; }
  };

  struct RemoveAllClassifier {
      KOKKOS_INLINE_FUNCTION int operator()(const BlockTestParticle &) const { return 1; }
  };

  struct NonNegativeIdPredicate {
      KOKKOS_INLINE_FUNCTION bool operator()(const BlockTestParticle &p) const { return p.id >= 0; }
  };

  static_assert(HostBlocked::host_accessible, "HostSpace storage must be host accessible");
  static_assert(std::is_same_v<HostBlocked::ExecutionSpace, Kokkos::Serial>);
  static_assert(std::is_same_v<HostBlocked::MemorySpace, Kokkos::HostSpace>);
  static_assert(std::is_same_v<DefaultBlocked::ExecutionSpace, Kokkos::DefaultExecutionSpace>);
  static_assert(schnek::ParticleContainerTraits<HostBlocked>::kokkos_dispatch);
  static_assert(std::is_same_v<schnek::ParticleContainerTraits<HostBlocked>::execution_space, Kokkos::Serial>);
  static_assert(std::is_same_v<schnek::ParticleContainerTraits<HostBlocked>::value_type, BlockTestParticle>);
  static_assert(std::is_same_v<schnek::ParticleContainerFactory<HostBlocked>::Container, HostBlocked>);
  static_assert(!std::is_copy_constructible_v<HostBlocked>, "containers own device memory and must not be copied");
  static_assert(std::is_nothrow_move_constructible_v<HostBlocked>);

  template<class MemSpace>
  Kokkos::View<std::byte *, MemSpace> makeBytes(const std::string &label, std::size_t bytes) {
    return Kokkos::View<std::byte *, MemSpace>(Kokkos::view_alloc(Kokkos::WithoutInitializing, label), bytes);
  }

  template<class MemSpace>
  Kokkos::View<std::byte *, MemSpace> uploadParticles(const std::vector<BlockTestParticle> &host) {
    std::vector<std::byte> staging(host.size() * Serializer::size());
    for (std::size_t i = 0; i < host.size(); ++i) {
      Serializer::serialize(host[i], staging.data() + i * Serializer::size());
    }
    auto buffer = makeBytes<MemSpace>("test:upload", staging.size());
    Kokkos::View<std::byte *, Kokkos::HostSpace, Kokkos::MemoryUnmanaged> hostView(staging.data(), staging.size());
    Kokkos::deep_copy(buffer, hostView);
    return buffer;
  }

  // `buffer` must hold exactly `count` serialised particles.
  template<class MemSpace>
  std::vector<BlockTestParticle> downloadParticles(
      const Kokkos::View<std::byte *, MemSpace> &buffer, std::size_t count
  ) {
    std::vector<BlockTestParticle> result;
    if (count == 0) {
      return result;
    }
    std::vector<std::byte> staging(count * Serializer::size());
    Kokkos::View<std::byte *, Kokkos::HostSpace, Kokkos::MemoryUnmanaged> hostView(staging.data(), staging.size());
    Kokkos::deep_copy(hostView, buffer);
    for (std::size_t i = 0; i < count; ++i) {
      result.push_back(Serializer::deserialize(staging.data() + i * Serializer::size()));
    }
    return result;
  }

  std::vector<int> idsOf(const std::vector<BlockTestParticle> &particles) {
    std::vector<int> ids;
    for (const auto &p : particles) {
      ids.push_back(p.id);
    }
    return ids;
  }

  template<class Container>
  std::vector<int> idsOfContainer(const Container &c) {
    return idsOf(c.copyToHost());
  }

  template<class Container>
  void growthPreservesParticles() {
    Container c(16);
    std::vector<int> expected;
    for (int i = 0; i < 1000; ++i) {
      c.push_back(BlockTestParticle{i, 0.5 * i});
      expected.push_back(i);
    }
    BOOST_CHECK_EQUAL(c.size(), 1000u);
    BOOST_CHECK_EQUAL(c.blockCount(), 63u);
    BOOST_CHECK_EQUAL(c.capacity(), 63u * 16u);
    BOOST_CHECK_EQUAL(c.blockUsed(62), 8u);
    BOOST_CHECK(idsOfContainer(c) == expected);
  }

  template<class Container>
  void reserveAddsWholeBlocks() {
    Container c(8);
    c.reserve(20);
    BOOST_CHECK_EQUAL(c.blockCount(), 3u);
    BOOST_CHECK_EQUAL(c.size(), 0u);
    BOOST_CHECK(c.empty());
    c.push_back(BlockTestParticle{7, 0.0});
    BOOST_CHECK_EQUAL(c.blockCount(), 3u);
    BOOST_CHECK_EQUAL(c.blockUsed(0), 0u);
    BOOST_CHECK_EQUAL(c.blockUsed(2), 1u);
    BOOST_CHECK(idsOfContainer(c) == std::vector<int>{7});
  }

  struct IncrementIdHalveX {
      KOKKOS_INLINE_FUNCTION void operator()(BlockTestParticle &p) const {
        p.id += 1;
        p.x *= 0.5;
      }
  };

  template<class Container>
  void parallelForEachUpdatesParticles() {
    Container empty(4);
    empty.parallelForEach(IncrementIdHalveX{});
    BOOST_CHECK(empty.empty());

    Container c(16);
    std::vector<int> expectedIds;
    for (int i = 0; i < 100; ++i) {
      c.push_back(BlockTestParticle{i, 2.0 * i});
      expectedIds.push_back(i + 1);
    }
    c.parallelForEach(IncrementIdHalveX{});
    BOOST_CHECK_EQUAL(c.size(), 100u);
    BOOST_CHECK(idsOfContainer(c) == expectedIds);
    const auto particles = c.copyToHost();
    for (std::size_t i = 0; i < particles.size(); ++i) {
      BOOST_CHECK_CLOSE(particles[i].x, static_cast<double>(i), 1e-12);
    }
  }

  template<class Container>
  void allOfDetectsViolation() {
    using Traits = schnek::ParticleContainerTraits<Container>;
    Container c(4);
    for (int i = 0; i < 10; ++i) {
      c.push_back(BlockTestParticle{i, 0.0});
    }
    BOOST_CHECK(c.allOf(NonNegativeIdPredicate{}));
    BOOST_CHECK(Traits::allOf(c, NonNegativeIdPredicate{}));
    c.push_back(BlockTestParticle{-1, 0.0});
    BOOST_CHECK(!c.allOf(NonNegativeIdPredicate{}));
    BOOST_CHECK(!Traits::allOf(c, NonNegativeIdPredicate{}));
  }

  template<class Container>
  void countBucketsOnEmptyAndPartialBlocks() {
    using Traits = schnek::ParticleContainerTraits<Container>;
    Container c(4);
    std::array<std::size_t, 2> counts{99, 99};
    Traits::countBuckets(c, ModThreeClassifier{}, counts);
    BOOST_CHECK_EQUAL(counts[0], 0u);
    BOOST_CHECK_EQUAL(counts[1], 0u);

    for (int i = 0; i < 10; ++i) {
      c.push_back(BlockTestParticle{i, 0.0});
    }
    Traits::countBuckets(c, ModThreeClassifier{}, counts);
    BOOST_CHECK_EQUAL(counts[0], 3u);  // ids 1, 4, 7
    BOOST_CHECK_EQUAL(counts[1], 3u);  // ids 2, 5, 8
    BOOST_CHECK_EQUAL(c.size(), 10u);
    BOOST_CHECK(idsOfContainer(c) == (std::vector<int>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
  }

  template<class Container>
  void partitionRoundTrip() {
    using Traits = schnek::ParticleContainerTraits<Container>;
    using Space = typename Container::MemorySpace;
    Container c(4);
    for (int i = 0; i < 10; ++i) {
      c.push_back(BlockTestParticle{i, 0.25 * i});
    }

    std::array<std::size_t, 2> counts{};
    Traits::countBuckets(c, ModThreeClassifier{}, counts);
    auto out1 = makeBytes<Space>("test:out1", counts[0] * Serializer::size());
    auto out2 = makeBytes<Space>("test:out2", counts[1] * Serializer::size());
    auto scratch = makeBytes<Space>("test:scratch", Traits::partitionScratchBytes(c));
    const std::array<std::byte *, 2> outPtrs{out1.data(), out2.data()};
    Traits::template partitionAndPack<Serializer>(c, ModThreeClassifier{}, outPtrs, scratch.data());

    BOOST_CHECK(idsOfContainer(c) == (std::vector<int>{0, 3, 6, 9}));
    BOOST_CHECK(idsOf(downloadParticles(out1, counts[0])) == (std::vector<int>{1, 4, 7}));
    BOOST_CHECK(idsOf(downloadParticles(out2, counts[1])) == (std::vector<int>{2, 5, 8}));
    BOOST_CHECK_EQUAL(c.size(), 4u);
    BOOST_CHECK_EQUAL(c.blockUsed(0), 4u);
    BOOST_CHECK_EQUAL(c.blockUsed(1), 0u);

    // Re-inserting the outgoing bucket appends it after the survivors.
    Traits::template bulkInsert<Serializer>(c, out1.data(), counts[0]);
    BOOST_CHECK(idsOfContainer(c) == (std::vector<int>{0, 3, 6, 9, 1, 4, 7}));
    BOOST_CHECK_EQUAL(c.size(), 7u);
  }

  template<class Container>
  void emptyBlocksAreSkipped() {
    using Traits = schnek::ParticleContainerTraits<Container>;
    using Space = typename Container::MemorySpace;
    Container c(4);
    for (int i = 0; i < 10; ++i) {
      c.push_back(BlockTestParticle{i, 0.0});
    }

    // Packing leaves blocks 1 and 2 empty.
    std::array<std::size_t, 1> counts{};
    Traits::countBuckets(c, KeepFirstTwoClassifier{}, counts);
    auto out = makeBytes<Space>("test:out", counts[0] * Serializer::size());
    auto scratch = makeBytes<Space>("test:scratch", Traits::partitionScratchBytes(c));
    const std::array<std::byte *, 1> outPtrs{out.data()};
    Traits::template partitionAndPack<Serializer>(c, KeepFirstTwoClassifier{}, outPtrs, scratch.data());
    BOOST_CHECK_EQUAL(c.blockCount(), 3u);
    BOOST_CHECK_EQUAL(c.blockUsed(0), 2u);
    BOOST_CHECK_EQUAL(c.blockUsed(1), 0u);
    BOOST_CHECK_EQUAL(c.blockUsed(2), 0u);
    BOOST_CHECK(idsOfContainer(c) == (std::vector<int>{0, 1}));

    // The next push fills the empty tail block, which leaves an empty block between the two.
    c.push_back(BlockTestParticle{100, 0.0});
    BOOST_CHECK_EQUAL(c.blockUsed(1), 0u);
    BOOST_CHECK_EQUAL(c.blockUsed(2), 1u);
    BOOST_CHECK(idsOfContainer(c) == (std::vector<int>{0, 1, 100}));

    std::vector<BlockTestParticle> incoming;
    for (int i = 200; i < 205; ++i) {
      incoming.push_back(BlockTestParticle{i, 0.0});
    }
    auto buffer = uploadParticles<Space>(incoming);
    Traits::template bulkInsert<Serializer>(c, buffer.data(), incoming.size());
    BOOST_CHECK_EQUAL(c.blockCount(), 4u);
    BOOST_CHECK_EQUAL(c.blockUsed(2), 4u);
    BOOST_CHECK_EQUAL(c.blockUsed(3), 2u);
    BOOST_CHECK(idsOfContainer(c) == (std::vector<int>{0, 1, 100, 200, 201, 202, 203, 204}));
  }

  template<class Container>
  void partitionEdgeCases() {
    using Traits = schnek::ParticleContainerTraits<Container>;
    using Space = typename Container::MemorySpace;
    Container c(3);
    for (int i = 0; i < 7; ++i) {
      c.push_back(BlockTestParticle{i, 0.0});
    }

    // No outbound buckets: every particle survives and the order is unchanged.
    std::array<std::byte *, 0> noOut{};
    auto scratch = makeBytes<Space>("test:scratch", Traits::partitionScratchBytes(c));
    Traits::template partitionAndPack<Serializer>(c, KeepAllClassifier{}, noOut, scratch.data());
    BOOST_CHECK_EQUAL(c.size(), 7u);
    BOOST_CHECK(idsOfContainer(c) == (std::vector<int>{0, 1, 2, 3, 4, 5, 6}));
    BOOST_CHECK_EQUAL(c.blockUsed(0), 3u);
    BOOST_CHECK_EQUAL(c.blockUsed(1), 3u);
    BOOST_CHECK_EQUAL(c.blockUsed(2), 1u);

    // Every particle is removed into one bucket, which leaves the container empty.
    std::array<std::size_t, 1> counts{};
    Traits::countBuckets(c, RemoveAllClassifier{}, counts);
    BOOST_CHECK_EQUAL(counts[0], 7u);
    auto out = makeBytes<Space>("test:out", counts[0] * Serializer::size());
    const std::array<std::byte *, 1> outPtrs{out.data()};
    Traits::template partitionAndPack<Serializer>(c, RemoveAllClassifier{}, outPtrs, scratch.data());
    BOOST_CHECK_EQUAL(c.size(), 0u);
    BOOST_CHECK(c.empty());
    BOOST_CHECK_EQUAL(c.blockUsed(0), 0u);
    BOOST_CHECK_EQUAL(c.blockUsed(1), 0u);
    BOOST_CHECK_EQUAL(c.blockUsed(2), 0u);
    BOOST_CHECK(idsOf(downloadParticles(out, counts[0])) == (std::vector<int>{0, 1, 2, 3, 4, 5, 6}));
  }

  template<class Container>
  void randomOperationsMatchReference(std::size_t blockCapacity, unsigned seed) {
    using Traits = schnek::ParticleContainerTraits<Container>;
    using Space = typename Container::MemorySpace;

    Container c(blockCapacity);
    std::vector<BlockTestParticle> reference;
    std::mt19937 rng(seed);
    int nextId = 0;
    for (int step = 0; step < 200; ++step) {
      const int op = std::uniform_int_distribution<int>(0, 2)(rng);
      if (op == 0) {
        const BlockTestParticle p{nextId++, 0.5 * nextId};
        c.push_back(p);
        reference.push_back(p);
      } else if (op == 1) {
        const std::size_t count = std::uniform_int_distribution<std::size_t>(0, 3 * blockCapacity + 3)(rng);
        std::vector<BlockTestParticle> incoming;
        for (std::size_t i = 0; i < count; ++i) {
          incoming.push_back(BlockTestParticle{nextId++, 0.5 * nextId});
        }
        auto buffer = uploadParticles<Space>(incoming);
        Traits::template bulkInsert<Serializer>(c, buffer.data(), count);
        reference.insert(reference.end(), incoming.begin(), incoming.end());
      } else {
        std::vector<BlockTestParticle> survivors;
        std::vector<BlockTestParticle> out1;
        std::vector<BlockTestParticle> out2;
        for (const auto &p : reference) {
          const int bucket = p.id % 3;
          if (bucket == 0) {
            survivors.push_back(p);
          } else if (bucket == 1) {
            out1.push_back(p);
          } else {
            out2.push_back(p);
          }
        }

        std::array<std::size_t, 2> counts{};
        Traits::countBuckets(c, ModThreeClassifier{}, counts);
        BOOST_CHECK_EQUAL(counts[0], out1.size());
        BOOST_CHECK_EQUAL(counts[1], out2.size());

        auto b1 = makeBytes<Space>("test:b1", counts[0] * Serializer::size());
        auto b2 = makeBytes<Space>("test:b2", counts[1] * Serializer::size());
        auto scratch = makeBytes<Space>("test:scratch", Traits::partitionScratchBytes(c));
        const std::array<std::byte *, 2> outPtrs{b1.data(), b2.data()};
        Traits::template partitionAndPack<Serializer>(c, ModThreeClassifier{}, outPtrs, scratch.data());

        BOOST_CHECK(idsOf(downloadParticles(b1, counts[0])) == idsOf(out1));
        BOOST_CHECK(idsOf(downloadParticles(b2, counts[1])) == idsOf(out2));
        reference = survivors;
      }
      BOOST_CHECK_EQUAL(c.size(), reference.size());
      BOOST_CHECK(idsOfContainer(c) == idsOf(reference));
    }
  }

  template<class Container>
  void factoryConfiguresBlockCapacity() {
    schnek::ParticleContainerFactory<Container> factory(8, 20);
    Container c = factory.makeContainer(0, 0);
    BOOST_CHECK_EQUAL(c.blockCapacity(), 8u);
    BOOST_CHECK_EQUAL(c.blockCount(), 3u);
    BOOST_CHECK_EQUAL(c.size(), 0u);
  }

  template<class Container>
  void moveLeavesSourceEmpty() {
    Container a(4);
    for (int i = 0; i < 3; ++i) {
      a.push_back(BlockTestParticle{i, 0.0});
    }
    Container b(std::move(a));
    BOOST_CHECK_EQUAL(a.size(), 0u);
    BOOST_CHECK_EQUAL(b.size(), 3u);
    BOOST_CHECK(idsOfContainer(b) == (std::vector<int>{0, 1, 2}));

    Container d(2);
    d.push_back(BlockTestParticle{42, 0.0});
    d = std::move(b);
    BOOST_CHECK_EQUAL(d.size(), 3u);
    BOOST_CHECK_EQUAL(b.size(), 0u);
    BOOST_CHECK(idsOfContainer(d) == (std::vector<int>{0, 1, 2}));

    b.push_back(BlockTestParticle{9, 0.0});
    BOOST_CHECK(idsOfContainer(b) == (std::vector<int>{9}));
  }

}  // namespace

BOOST_AUTO_TEST_SUITE(domain_decomposition)
BOOST_AUTO_TEST_SUITE(detail)
BOOST_AUTO_TEST_SUITE(device_blocked_particle_container)

BOOST_AUTO_TEST_CASE(host_growth_preserves_particles) {
  growthPreservesParticles<HostBlocked>();
}

BOOST_AUTO_TEST_CASE(host_reserve_adds_whole_blocks) {
  reserveAddsWholeBlocks<HostBlocked>();
}

BOOST_AUTO_TEST_CASE(host_all_of_detects_violation) {
  allOfDetectsViolation<HostBlocked>();
}

BOOST_AUTO_TEST_CASE(host_count_buckets_on_empty_and_partial_blocks) {
  countBucketsOnEmptyAndPartialBlocks<HostBlocked>();
}

BOOST_AUTO_TEST_CASE(host_partition_round_trip) {
  partitionRoundTrip<HostBlocked>();
}

BOOST_AUTO_TEST_CASE(host_empty_blocks_are_skipped) {
  emptyBlocksAreSkipped<HostBlocked>();
}

BOOST_AUTO_TEST_CASE(host_partition_edge_cases) {
  partitionEdgeCases<HostBlocked>();
}

BOOST_AUTO_TEST_CASE(host_random_operations_match_reference) {
  randomOperationsMatchReference<HostBlocked>(5, 1234u);
}

BOOST_AUTO_TEST_CASE(host_factory_configures_block_capacity) {
  factoryConfiguresBlockCapacity<HostBlocked>();
}

BOOST_AUTO_TEST_CASE(host_move_leaves_source_empty) {
  moveLeavesSourceEmpty<HostBlocked>();
}

BOOST_AUTO_TEST_CASE(host_parallel_for_each_updates_particles) {
  parallelForEachUpdatesParticles<HostBlocked>();
}

BOOST_AUTO_TEST_CASE(default_space_parallel_for_each_updates_particles) {
  if (!deviceTestsEnabled()) {
    return;
  }
  parallelForEachUpdatesParticles<DefaultBlocked>();
}

BOOST_AUTO_TEST_CASE(default_space_growth_preserves_particles) {
  if (!deviceTestsEnabled()) {
    return;
  }
  growthPreservesParticles<DefaultBlocked>();
}

BOOST_AUTO_TEST_CASE(default_space_all_of_detects_violation) {
  if (!deviceTestsEnabled()) {
    return;
  }
  allOfDetectsViolation<DefaultBlocked>();
}

BOOST_AUTO_TEST_CASE(default_space_count_buckets_on_empty_and_partial_blocks) {
  if (!deviceTestsEnabled()) {
    return;
  }
  countBucketsOnEmptyAndPartialBlocks<DefaultBlocked>();
}

BOOST_AUTO_TEST_CASE(default_space_partition_round_trip) {
  if (!deviceTestsEnabled()) {
    return;
  }
  partitionRoundTrip<DefaultBlocked>();
}

BOOST_AUTO_TEST_CASE(default_space_empty_blocks_are_skipped) {
  if (!deviceTestsEnabled()) {
    return;
  }
  emptyBlocksAreSkipped<DefaultBlocked>();
}

BOOST_AUTO_TEST_CASE(default_space_partition_edge_cases) {
  if (!deviceTestsEnabled()) {
    return;
  }
  partitionEdgeCases<DefaultBlocked>();
}

BOOST_AUTO_TEST_CASE(default_space_random_operations_match_reference) {
  if (!deviceTestsEnabled()) {
    return;
  }
  randomOperationsMatchReference<DefaultBlocked>(5, 4321u);
}

BOOST_AUTO_TEST_CASE(default_space_move_leaves_source_empty) {
  if (!deviceTestsEnabled()) {
    return;
  }
  moveLeavesSourceEmpty<DefaultBlocked>();
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
