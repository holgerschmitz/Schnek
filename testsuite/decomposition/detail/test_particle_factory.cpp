#include "../../utility.hpp"

#include <decomposition/detail/particle_factory.hpp>
#include <util/exceptions.hpp>

#include <boost/test/unit_test.hpp>

#ifdef KOKKOS_ENABLE_CUDA
#include <cuda_runtime_api.h>
#endif

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdlib>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

struct TestParticle {
  int id;
  double x;
};

using VectorTraits = schnek::ParticleContainerTraits<std::vector<TestParticle>>;
using WireFormat = schnek::ParticleSerializer<TestParticle>;

// Bucket 0 keeps a particle. Buckets 1 and 2 remove it.
const auto classifyModulo = [](const TestParticle &p) { return p.id % 3; };

std::vector<TestParticle> makeParticles(int count) {
  std::vector<TestParticle> particles;
  for (int id = 0; id < count; ++id) {
    particles.push_back(TestParticle{id, 0.5 * id});
  }
  return particles;
}

std::vector<int> idsOf(const std::vector<TestParticle> &particles) {
  std::vector<int> ids;
  for (const auto &p : particles) {
    ids.push_back(p.id);
  }
  return ids;
}

std::vector<int> decodeIds(const std::byte *src, std::size_t count) {
  std::vector<int> ids;
  for (std::size_t i = 0; i < count; ++i) {
    ids.push_back(WireFormat::deserialize(src + i * WireFormat::size()).id);
  }
  return ids;
}

// A container with no contiguous storage. It specialises the traits fully and has no data() member.
struct SegmentedParticles {
  std::vector<std::vector<TestParticle>> blocks;
};

}  // namespace

namespace schnek {

template<>
struct ParticleContainerTraits<::SegmentedParticles> {
  using ContainerType = ::SegmentedParticles;
  using value_type = TestParticle;

  static std::size_t size(const ContainerType &c) {
    std::size_t total = 0;
    for (const auto &block : c.blocks) {
      total += block.size();
    }
    return total;
  }

  static void insert(ContainerType &c, const value_type &p) {
    if (c.blocks.empty()) {
      c.blocks.emplace_back();
    }
    c.blocks.back().push_back(p);
  }

  template<class F>
  static void forEach(const ContainerType &c, F &&f) {
    for (const auto &block : c.blocks) {
      for (const auto &p : block) {
        f(p);
      }
    }
  }

  template<class Predicate>
  static bool allOf(const ContainerType &c, Predicate pred) {
    for (const auto &block : c.blocks) {
      for (const auto &p : block) {
        if (!pred(p)) {
          return false;
        }
      }
    }
    return true;
  }

  template<std::size_t NumBuckets, class Classifier>
  static void countBuckets(const ContainerType &c, const Classifier &classify, std::array<std::size_t, NumBuckets> &counts) {
    counts.fill(0);
    forEach(c, [&](const value_type &p) {
      const int bkt = classify(p);
      if (bkt != 0) {
        ++counts[static_cast<std::size_t>(bkt) - 1];
      }
    });
  }

  static std::size_t partitionScratchBytes(const ContainerType &) { return 0; }

  template<class Serializer, std::size_t NumBuckets, class Classifier>
  static void partitionAndPack(
      ContainerType &c,
      const Classifier &classify,
      const std::array<std::byte *, NumBuckets> &outPtrs,
      std::byte *
  ) {
    std::array<std::size_t, NumBuckets> offset{};
    for (auto &block : c.blocks) {
      std::vector<value_type> survivors;
      for (const auto &p : block) {
        const int bkt = classify(p);
        if (bkt == 0) {
          survivors.push_back(p);
        } else {
          const std::size_t b = static_cast<std::size_t>(bkt) - 1;
          Serializer::serialize(p, outPtrs[b] + offset[b] * Serializer::size());
          ++offset[b];
        }
      }
      block = std::move(survivors);
    }
  }

  template<class Serializer>
  static void bulkInsert(ContainerType &c, const std::byte *src, std::size_t count) {
    for (std::size_t i = 0; i < count; ++i) {
      insert(c, Serializer::deserialize(src + i * Serializer::size()));
    }
  }
};

}  // namespace schnek

BOOST_AUTO_TEST_SUITE(domain_decomposition)
BOOST_AUTO_TEST_SUITE(detail)
BOOST_AUTO_TEST_SUITE(particle_factory)

BOOST_AUTO_TEST_CASE(count_buckets_does_not_modify_container) {
  auto particles = makeParticles(6);
  const std::vector<int> before = idsOf(particles);

  std::array<std::size_t, 2> counts{};
  VectorTraits::countBuckets(particles, classifyModulo, counts);

  BOOST_CHECK_EQUAL(counts[0], 2u);
  BOOST_CHECK_EQUAL(counts[1], 2u);

  const std::vector<int> after = idsOf(particles);
  BOOST_CHECK_EQUAL_COLLECTIONS(after.begin(), after.end(), before.begin(), before.end());
}

BOOST_AUTO_TEST_CASE(count_buckets_rejects_out_of_range_bucket) {
  const auto particles = makeParticles(1);
  std::array<std::size_t, 2> counts{};

  const auto tooLarge = [](const TestParticle &) { return 3; };
  const auto negative = [](const TestParticle &) { return -1; };

  BOOST_CHECK_THROW(VectorTraits::countBuckets(particles, tooLarge, counts), schnek::ScheckException);
  BOOST_CHECK_THROW(VectorTraits::countBuckets(particles, negative, counts), schnek::ScheckException);
}

BOOST_AUTO_TEST_CASE(partition_and_pack_keeps_survivor_order_and_packs_by_bucket) {
  auto particles = makeParticles(6);

  std::array<std::size_t, 2> counts{};
  VectorTraits::countBuckets(particles, classifyModulo, counts);

  std::vector<std::byte> down(counts[0] * WireFormat::size());
  std::vector<std::byte> up(counts[1] * WireFormat::size());
  const std::array<std::byte *, 2> outPtrs{down.data(), up.data()};
  VectorTraits::partitionAndPack<WireFormat>(particles, classifyModulo, outPtrs, nullptr);

  const std::vector<int> survivors = idsOf(particles);
  const std::vector<int> expectedSurvivors{0, 3};
  BOOST_CHECK_EQUAL_COLLECTIONS(survivors.begin(), survivors.end(), expectedSurvivors.begin(), expectedSurvivors.end());

  const std::vector<int> downIds = decodeIds(down.data(), counts[0]);
  const std::vector<int> expectedDown{1, 4};
  BOOST_CHECK_EQUAL_COLLECTIONS(downIds.begin(), downIds.end(), expectedDown.begin(), expectedDown.end());

  const std::vector<int> upIds = decodeIds(up.data(), counts[1]);
  const std::vector<int> expectedUp{2, 5};
  BOOST_CHECK_EQUAL_COLLECTIONS(upIds.begin(), upIds.end(), expectedUp.begin(), expectedUp.end());
}

BOOST_AUTO_TEST_CASE(partition_and_pack_without_departures_needs_no_buffers) {
  auto particles = makeParticles(4);
  const auto keepAll = [](const TestParticle &) { return 0; };
  const std::array<std::byte *, 2> outPtrs{nullptr, nullptr};

  VectorTraits::partitionAndPack<WireFormat>(particles, keepAll, outPtrs, nullptr);

  const std::vector<int> survivors = idsOf(particles);
  const std::vector<int> expected{0, 1, 2, 3};
  BOOST_CHECK_EQUAL_COLLECTIONS(survivors.begin(), survivors.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(bulk_insert_appends_in_buffer_order) {
  auto particles = makeParticles(1);

  std::vector<std::byte> buffer(2 * WireFormat::size());
  WireFormat::serialize(TestParticle{20, 1.0}, buffer.data());
  WireFormat::serialize(TestParticle{21, 2.0}, buffer.data() + WireFormat::size());

  VectorTraits::bulkInsert<WireFormat>(particles, buffer.data(), 2);

  const std::vector<int> ids = idsOf(particles);
  const std::vector<int> expected{0, 20, 21};
  BOOST_CHECK_EQUAL_COLLECTIONS(ids.begin(), ids.end(), expected.begin(), expected.end());

  VectorTraits::bulkInsert<WireFormat>(particles, nullptr, 0);
  BOOST_CHECK_EQUAL(particles.size(), 3u);
}

BOOST_AUTO_TEST_CASE(segmented_specialisation_needs_no_data_pointer) {
  static_assert(!schnek::internal::has_data<SegmentedParticles>::value, "segmented test container must not expose data()");
  static_assert(schnek::internal::has_data<std::vector<TestParticle>>::value, "std::vector must expose data()");

  SegmentedParticles container;
  container.blocks.push_back({TestParticle{0, 0.0}, TestParticle{1, 0.0}, TestParticle{2, 0.0}});
  container.blocks.push_back({TestParticle{3, 0.0}, TestParticle{4, 0.0}, TestParticle{5, 0.0}});

  using SegTraits = schnek::ParticleContainerTraits<SegmentedParticles>;
  BOOST_CHECK_EQUAL(SegTraits::size(container), 6u);

  std::array<std::size_t, 2> counts{};
  SegTraits::countBuckets(container, classifyModulo, counts);
  BOOST_CHECK_EQUAL(counts[0], 2u);
  BOOST_CHECK_EQUAL(counts[1], 2u);

  std::vector<std::byte> down(counts[0] * WireFormat::size());
  std::vector<std::byte> up(counts[1] * WireFormat::size());
  const std::array<std::byte *, 2> outPtrs{down.data(), up.data()};
  SegTraits::partitionAndPack<WireFormat>(container, classifyModulo, outPtrs, nullptr);

  std::vector<int> survivors;
  SegTraits::forEach(container, [&](const TestParticle &p) { survivors.push_back(p.id); });
  const std::vector<int> expectedSurvivors{0, 3};
  BOOST_CHECK_EQUAL_COLLECTIONS(survivors.begin(), survivors.end(), expectedSurvivors.begin(), expectedSurvivors.end());

  const std::vector<int> upIds = decodeIds(up.data(), counts[1]);
  const std::vector<int> expectedUp{2, 5};
  BOOST_CHECK_EQUAL_COLLECTIONS(upIds.begin(), upIds.end(), expectedUp.begin(), expectedUp.end());

  SegTraits::bulkInsert<WireFormat>(container, up.data(), counts[1]);
  BOOST_CHECK_EQUAL(SegTraits::size(container), 4u);
}

BOOST_AUTO_TEST_CASE(host_container_has_no_kokkos_dispatch) {
  using Comm = schnek::detail::ParticleCommunicationTraits<std::vector<TestParticle>>;
  BOOST_CHECK(!VectorTraits::kokkos_dispatch);
  BOOST_CHECK((std::is_void<VectorTraits::execution_space>::value));
  BOOST_CHECK(Comm::mpi_access == schnek::detail::DeviceMpiAccess::Direct);
}

#ifdef SCHNEK_HAVE_KOKKOS
namespace {

// Kokkos host-memory container. No ExecutionSpace is declared, so the execution space of the memory space is used.
struct HostMemoryParticles {
  using value_type = TestParticle;
  using MemorySpace = Kokkos::HostSpace;
  static constexpr bool host_accessible = true;
};

// Host-memory container with an explicit execution space.
struct ExplicitExecutionParticles {
  using value_type = TestParticle;
  using MemorySpace = Kokkos::HostSpace;
  using ExecutionSpace = Kokkos::DefaultHostExecutionSpace;
  static constexpr bool host_accessible = true;
};

#ifdef KOKKOS_ENABLE_CUDA
// Device-memory container. Its execution space is the one of CudaSpace.
struct CudaMemoryParticles {
  using value_type = TestParticle;
  using MemorySpace = Kokkos::CudaSpace;
  static constexpr bool host_accessible = false;
};

// Device-memory container that asks for host-staged MPI.
struct HostStagedCudaParticles {
  using value_type = TestParticle;
  using MemorySpace = Kokkos::CudaSpace;
  static constexpr bool host_accessible = false;
  static constexpr schnek::detail::DeviceMpiAccess mpi_access = schnek::detail::DeviceMpiAccess::HostStaged;
};

template<typename T>
class CudaParticles {
  public:
    using value_type = T;
    using MemorySpace = Kokkos::CudaSpace;
    static constexpr bool host_accessible = false;

    CudaParticles() = default;

    explicit CudaParticles(const std::vector<T> &values) {
      resize(values.size());
      auto mirror = Kokkos::create_mirror_view(view_);
      for (std::size_t i = 0; i < values.size(); ++i) {
        mirror(i) = values[i];
      }
      Kokkos::deep_copy(view_, mirror);
    }

    std::size_t size() const { return size_; }

    void resize(std::size_t newSize) {
      view_type next(Kokkos::view_alloc(Kokkos::WithoutInitializing, "schnek:test-particles"), newSize);
      const std::size_t kept = std::min(size_, newSize);
      if (kept > 0) {
        Kokkos::deep_copy(
            Kokkos::subview(next, Kokkos::pair<std::size_t, std::size_t>(0, kept)),
            Kokkos::subview(view_, Kokkos::pair<std::size_t, std::size_t>(0, kept))
        );
      }
      view_ = next;
      size_ = newSize;
    }

    T *data() { return view_.data(); }
    const T *data() const { return view_.data(); }

    std::vector<T> hostValues() const {
      auto mirror = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(), view_);
      return std::vector<T>(mirror.data(), mirror.data() + size_);
    }

  private:
    using view_type = Kokkos::View<T *, Kokkos::CudaSpace>;
    view_type view_;
    std::size_t size_ = 0;
};
#endif

}  // namespace

BOOST_AUTO_TEST_CASE(memory_space_container_uses_its_execution_space) {
  using Traits = schnek::ParticleContainerTraits<HostMemoryParticles>;
  using Comm = schnek::detail::ParticleCommunicationTraits<HostMemoryParticles>;
  BOOST_CHECK(Traits::kokkos_dispatch);
  BOOST_CHECK((std::is_same<Traits::execution_space, Kokkos::DefaultHostExecutionSpace>::value));
  BOOST_CHECK((std::is_same<Comm::memory_space, Kokkos::HostSpace>::value));
  BOOST_CHECK(Comm::host_accessible);
  BOOST_CHECK(Comm::mpi_access == schnek::detail::DeviceMpiAccess::Direct);
}

BOOST_AUTO_TEST_CASE(explicit_execution_space_is_selected) {
  using Traits = schnek::ParticleContainerTraits<ExplicitExecutionParticles>;
  BOOST_CHECK(Traits::kokkos_dispatch);
  BOOST_CHECK((std::is_same<Traits::execution_space, Kokkos::DefaultHostExecutionSpace>::value));
  BOOST_CHECK(schnek::detail::ParticleCommunicationTraits<ExplicitExecutionParticles>::host_accessible);
}

#ifdef KOKKOS_ENABLE_CUDA
BOOST_AUTO_TEST_CASE(device_memory_container_uses_device_execution_space) {
  using Traits = schnek::ParticleContainerTraits<CudaMemoryParticles>;
  using Comm = schnek::detail::ParticleCommunicationTraits<CudaMemoryParticles>;
  BOOST_CHECK(Traits::kokkos_dispatch);
  BOOST_CHECK((std::is_same<Traits::execution_space, Kokkos::Cuda>::value));
  BOOST_CHECK((std::is_same<Comm::memory_space, Kokkos::CudaSpace>::value));
  BOOST_CHECK(!Comm::host_accessible);
  BOOST_CHECK(Comm::mpi_access == schnek::detail::DeviceMpiAccess::Direct);
}

BOOST_AUTO_TEST_CASE(host_staged_mpi_is_selected_by_container) {
  using Comm = schnek::detail::ParticleCommunicationTraits<HostStagedCudaParticles>;
  BOOST_CHECK(Comm::mpi_access == schnek::detail::DeviceMpiAccess::HostStaged);
  BOOST_CHECK(!Comm::host_accessible);
}

BOOST_AUTO_TEST_CASE(device_memory_partition_compact_and_bulk_insert) {
  if (std::getenv("SCHNEK_RUN_CUDA_DEVICE_TESTS") == nullptr) {
    return;
  }

  int deviceCount = 0;
  if (cudaGetDeviceCount(&deviceCount) != cudaSuccess || deviceCount == 0) {
    return;
  }

  using Container = CudaParticles<TestParticle>;
  using Traits = schnek::ParticleContainerTraits<Container>;
  using Serializer = schnek::ParticleSerializer<TestParticle>;
  using Buffer = schnek::detail::KokkosCommunicationBuffer<Kokkos::CudaSpace>;

  Container particles({TestParticle{0, 0.0}, TestParticle{1, 1.0}, TestParticle{2, 2.0},
                       TestParticle{3, 3.0}, TestParticle{4, 4.0}, TestParticle{5, 5.0}});
  particles.resize(8);
  particles.resize(6);

  const auto classify = KOKKOS_LAMBDA(const TestParticle &p) { return p.id % 3; };
  std::array<std::size_t, 2> counts{};
  Traits::countBuckets(particles, classify, counts);
  BOOST_CHECK_EQUAL(counts[0], 2u);
  BOOST_CHECK_EQUAL(counts[1], 2u);

  Buffer buffer;
  auto payload = buffer.make_vector<std::byte>(4 * Serializer::size());
  std::array<std::byte *, 2> outPtrs{payload.data(), payload.data() + 2 * Serializer::size()};
  auto scratch = buffer.make_vector<std::byte>(Traits::partitionScratchBytes(particles));
  Traits::template partitionAndPack<Serializer>(particles, classify, outPtrs, scratch.data());

  auto survivors = particles.hostValues();
  BOOST_REQUIRE_EQUAL(survivors.size(), 2u);
  BOOST_CHECK_EQUAL(survivors[0].id, 0);
  BOOST_CHECK_EQUAL(survivors[1].id, 3);

  Traits::template bulkInsert<Serializer>(particles, payload.data() + 2 * Serializer::size(), 2);
  auto result = particles.hostValues();
  std::sort(result.begin(), result.end(), [](const auto &left, const auto &right) { return left.id < right.id; });
  const std::vector<int> expected{0, 2, 3, 5};
  for (std::size_t i = 0; i < expected.size(); ++i) {
    BOOST_CHECK_EQUAL(result[i].id, expected[i]);
    BOOST_CHECK_EQUAL(result[i].x, static_cast<double>(expected[i]));
  }
}
#endif
#endif

namespace {

using BlockContainer = schnek::BlockParticleContainer<TestParticle>;
using BlockTraits = schnek::ParticleContainerTraits<BlockContainer>;

BlockContainer makeBlockContainer(int count, std::size_t blockCapacity) {
  BlockContainer container(blockCapacity);
  for (int id = 0; id < count; ++id) {
    container.push_back(TestParticle{id, 0.5 * id});
  }
  return container;
}

std::vector<int> sortedBlockIds(const BlockContainer &container) {
  std::vector<int> ids;
  BlockTraits::forEach(container, [&ids](const TestParticle &p) { ids.push_back(p.id); });
  std::sort(ids.begin(), ids.end());
  return ids;
}

}  // namespace

BOOST_AUTO_TEST_CASE(block_traits_size_and_scratch_need_no_storage) {
  const auto container = makeBlockContainer(7, 3);
  BOOST_CHECK_EQUAL(BlockTraits::size(container), 7u);
  BOOST_CHECK_EQUAL(BlockTraits::partitionScratchBytes(container), 0u);
  BOOST_CHECK(!BlockTraits::kokkos_dispatch);
}

BOOST_AUTO_TEST_CASE(block_traits_count_buckets_matches_vector_and_keeps_container) {
  const auto container = makeBlockContainer(11, 3);  // blocks of 3, 3, 3, 2 particles
  const auto particles = makeParticles(11);

  std::array<std::size_t, 2> blockCounts{};
  BlockTraits::countBuckets(container, classifyModulo, blockCounts);

  std::array<std::size_t, 2> vectorCounts{};
  VectorTraits::countBuckets(particles, classifyModulo, vectorCounts);

  BOOST_CHECK_EQUAL(blockCounts[0], vectorCounts[0]);
  BOOST_CHECK_EQUAL(blockCounts[1], vectorCounts[1]);
  BOOST_CHECK_EQUAL(BlockTraits::size(container), 11u);
}

BOOST_AUTO_TEST_CASE(block_traits_partition_and_pack_preserves_each_particle_once) {
  auto container = makeBlockContainer(11, 3);

  std::array<std::size_t, 2> counts{};
  BlockTraits::countBuckets(container, classifyModulo, counts);

  std::vector<std::byte> down(counts[0] * WireFormat::size());
  std::vector<std::byte> up(counts[1] * WireFormat::size());
  const std::array<std::byte *, 2> outPtrs{down.data(), up.data()};
  BlockTraits::partitionAndPack<WireFormat>(container, classifyModulo, outPtrs, nullptr);

  // Survivors are exactly the bucket-0 particles, each once. Their order is not part of the contract.
  const std::vector<int> expectedSurvivors{0, 3, 6, 9};
  const std::vector<int> survivorIds = sortedBlockIds(container);
  BOOST_CHECK_EQUAL_COLLECTIONS(
      survivorIds.begin(), survivorIds.end(), expectedSurvivors.begin(), expectedSurvivors.end()
  );
  BOOST_CHECK_EQUAL(BlockTraits::size(container), expectedSurvivors.size());

  std::vector<int> downIds = decodeIds(down.data(), counts[0]);
  std::sort(downIds.begin(), downIds.end());
  const std::vector<int> expectedDown{1, 4, 7, 10};
  BOOST_CHECK_EQUAL_COLLECTIONS(downIds.begin(), downIds.end(), expectedDown.begin(), expectedDown.end());

  std::vector<int> upIds = decodeIds(up.data(), counts[1]);
  std::sort(upIds.begin(), upIds.end());
  const std::vector<int> expectedUp{2, 5, 8};
  BOOST_CHECK_EQUAL_COLLECTIONS(upIds.begin(), upIds.end(), expectedUp.begin(), expectedUp.end());
}

BOOST_AUTO_TEST_CASE(block_traits_partition_retires_emptied_blocks) {
  // Eight particles in blocks of two. Bucket 1 takes ids 0..3, which empties the first two blocks.
  auto container = makeBlockContainer(8, 2);
  BOOST_REQUIRE_EQUAL(container.activeBlockCount(), 4u);

  const auto takeFirstHalf = [](const TestParticle &p) { return p.id < 4 ? 1 : 0; };
  std::array<std::size_t, 1> counts{};
  BlockTraits::countBuckets(container, takeFirstHalf, counts);
  BOOST_REQUIRE_EQUAL(counts[0], 4u);

  std::vector<std::byte> down(counts[0] * WireFormat::size());
  const std::array<std::byte *, 1> outPtrs{down.data()};
  BlockTraits::partitionAndPack<WireFormat>(container, takeFirstHalf, outPtrs, nullptr);

  const std::vector<int> expected{4, 5, 6, 7};
  const std::vector<int> ids = sortedBlockIds(container);
  BOOST_CHECK_EQUAL_COLLECTIONS(ids.begin(), ids.end(), expected.begin(), expected.end());
  BOOST_CHECK_EQUAL(container.activeBlockCount(), 2u);
  BOOST_CHECK_EQUAL(container.blockCount(), 4u);  // emptied blocks are pooled, not freed
}

BOOST_AUTO_TEST_CASE(block_traits_without_departures_keeps_all_particles) {
  auto container = makeBlockContainer(5, 2);
  const auto keepAll = [](const TestParticle &) { return 0; };
  const std::array<std::byte *, 2> outPtrs{nullptr, nullptr};

  BlockTraits::partitionAndPack<WireFormat>(container, keepAll, outPtrs, nullptr);

  const std::vector<int> expected{0, 1, 2, 3, 4};
  const std::vector<int> ids = sortedBlockIds(container);
  BOOST_CHECK_EQUAL_COLLECTIONS(ids.begin(), ids.end(), expected.begin(), expected.end());
  BOOST_CHECK_EQUAL(BlockTraits::size(container), 5u);
}

BOOST_AUTO_TEST_CASE(block_traits_bulk_insert_appends_across_blocks) {
  auto container = makeBlockContainer(5, 2);  // blocks 2, 2, 1: the last block is partial

  std::vector<std::byte> buffer(4 * WireFormat::size());
  for (int i = 0; i < 4; ++i) {
    WireFormat::serialize(TestParticle{20 + i, 1.0}, buffer.data() + static_cast<std::size_t>(i) * WireFormat::size());
  }

  BlockTraits::bulkInsert<WireFormat>(container, buffer.data(), 4);

  BOOST_CHECK_EQUAL(BlockTraits::size(container), 9u);
  BOOST_CHECK_GE(container.capacity(), 9u);
  const std::vector<int> expected{0, 1, 2, 3, 4, 20, 21, 22, 23};
  const std::vector<int> ids = sortedBlockIds(container);
  BOOST_CHECK_EQUAL_COLLECTIONS(ids.begin(), ids.end(), expected.begin(), expected.end());

  BlockTraits::bulkInsert<WireFormat>(container, nullptr, 0);
  BOOST_CHECK_EQUAL(BlockTraits::size(container), 9u);
}

BOOST_AUTO_TEST_CASE(block_traits_all_of_ignores_unused_slots) {
  auto container = makeBlockContainer(4, 4);
  // Removing the last particle leaves its value in the now unused slot. allOf must not see it.
  container.eraseIf([](const TestParticle &p) { return p.id == 3; });
  BOOST_REQUIRE_EQUAL(container.size(), 3u);

  BOOST_CHECK(BlockTraits::allOf(container, [](const TestParticle &p) { return p.id != 3; }));
  BOOST_CHECK(!BlockTraits::allOf(container, [](const TestParticle &p) { return p.id != 2; }));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()