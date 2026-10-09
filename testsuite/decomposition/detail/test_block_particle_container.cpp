#include "../../utility.hpp"

#include <decomposition/detail/particle_factory.hpp>
#include <decomposition/detail/block_particle_container.hpp>
#include <grid/range.hpp>
#include <util/exceptions.hpp>

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <random>
#include <set>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

struct TestParticle {
  int id;
  double x;
};

// Counts live instances, so that the lifetime of block storage can be checked.
struct TrackedParticle {
  static inline int alive = 0;

  int id;
  double x;

  TrackedParticle() : id(-1), x(0.0) { ++alive; }
  TrackedParticle(int id_, double x_) : id(id_), x(x_) { ++alive; }
  TrackedParticle(const TrackedParticle &other) : id(other.id), x(other.x) { ++alive; }
  TrackedParticle(TrackedParticle &&other) noexcept : id(other.id), x(other.x) { ++alive; }
  TrackedParticle &operator=(const TrackedParticle &) = default;
  TrackedParticle &operator=(TrackedParticle &&) = default;
  ~TrackedParticle() { --alive; }
};

using Container = schnek::BlockParticleContainer<TestParticle>;
using TrackedContainer = schnek::BlockParticleContainer<TrackedParticle>;

TestParticle makeParticle(int id) { return TestParticle{id, 0.5 * id}; }

std::vector<int> idsOf(const Container &container) {
  std::vector<int> ids;
  container.forEach([&ids](const TestParticle &p) { ids.push_back(p.id); });
  return ids;
}

std::vector<int> range(int count) {
  std::vector<int> ids;
  for (int id = 0; id < count; ++id) {
    ids.push_back(id);
  }
  return ids;
}

// Checks the size, used-slot and capacity invariants of the container.
void checkInvariants(const Container &container) {
  std::size_t total = 0;
  for (std::size_t i = 0; i < container.activeBlockCount(); ++i) {
    const std::size_t used = container.blockUsed(i);
    BOOST_REQUIRE_GE(used, 1u);
    BOOST_REQUIRE_LE(used, container.blockCapacity());
    total += used;
  }
  BOOST_REQUIRE_EQUAL(container.size(), total);
  BOOST_REQUIRE_LE(container.size(), container.capacity());
  BOOST_REQUIRE_LE(container.activeBlockCount(), container.blockCount());
  BOOST_REQUIRE_EQUAL(container.capacity(), container.blockCount() * container.blockCapacity());
}
struct TestAccessor {};

schnek::Range<std::ptrdiff_t, 1> localRange() {
  return schnek::Range<std::ptrdiff_t, 1>(schnek::Array<std::ptrdiff_t, 1>(0), schnek::Array<std::ptrdiff_t, 1>(9));
}

schnek::Range<double, 1> localDomain() {
  return schnek::Range<double, 1>(schnek::Array<double, 1>(0.0), schnek::Array<double, 1>(1.0));
}
}  // namespace

BOOST_AUTO_TEST_SUITE(domain_decomposition)
BOOST_AUTO_TEST_SUITE(detail)
BOOST_AUTO_TEST_SUITE(block_particle_container)

BOOST_AUTO_TEST_CASE(empty_container_has_no_blocks) {
  Container container(4);

  BOOST_CHECK_EQUAL(container.blockCapacity(), 4u);
  BOOST_CHECK_EQUAL(container.size(), 0u);
  BOOST_CHECK(container.empty());
  BOOST_CHECK_EQUAL(container.blockCount(), 0u);
  BOOST_CHECK_EQUAL(container.activeBlockCount(), 0u);
  BOOST_CHECK_EQUAL(container.capacity(), 0u);
}

BOOST_AUTO_TEST_CASE(zero_block_capacity_is_rejected) {
  BOOST_CHECK_THROW(Container(0), schnek::ScheckException);
}

BOOST_AUTO_TEST_CASE(push_back_fills_block_then_allocates_next) {
  Container container(4);
  for (int id = 0; id < 9; ++id) {
    container.push_back(makeParticle(id));
  }

  BOOST_CHECK_EQUAL(container.size(), 9u);
  BOOST_CHECK_EQUAL(container.blockCount(), 3u);
  BOOST_CHECK_EQUAL(container.activeBlockCount(), 3u);
  BOOST_CHECK_EQUAL(container.blockUsed(0), 4u);
  BOOST_CHECK_EQUAL(container.blockUsed(1), 4u);
  BOOST_CHECK_EQUAL(container.blockUsed(2), 1u);
  BOOST_CHECK_EQUAL(container.capacity(), 12u);
  checkInvariants(container);
}

BOOST_AUTO_TEST_CASE(growth_does_not_relocate_existing_blocks) {
  const std::size_t blockCapacity = 8;
  Container container(blockCapacity);
  container.push_back(makeParticle(0));

  TestParticle *firstBlockAddress = &container.block(0)[std::ptrdiff_t{0}];
  const void *firstBlockObject = &container.block(0);

  // Grow across many block boundaries.
  const int total = static_cast<int>(20 * blockCapacity);
  for (int id = 1; id < total; ++id) {
    container.push_back(makeParticle(id));
  }

  BOOST_CHECK_EQUAL(container.activeBlockCount(), 20u);
  BOOST_CHECK_EQUAL(static_cast<const void *>(&container.block(0)), firstBlockObject);
  BOOST_CHECK_EQUAL(&container.block(0)[std::ptrdiff_t{0}], firstBlockAddress);
  BOOST_CHECK_EQUAL(firstBlockAddress->id, 0);
  checkInvariants(container);
}

BOOST_AUTO_TEST_CASE(iteration_visits_particles_in_insertion_order) {
  Container container(4);
  for (int id = 0; id < 25; ++id) {
    container.push_back(makeParticle(id));
  }

  BOOST_CHECK(idsOf(container) == range(25));
}

BOOST_AUTO_TEST_CASE(invariants_hold_while_growing) {
  Container container(3);
  for (int id = 0; id < 3 * 5 + 2; ++id) {
    container.push_back(makeParticle(id));
    checkInvariants(container);
  }
}

BOOST_AUTO_TEST_CASE(clear_keeps_blocks_for_reuse) {
  Container container(4);
  for (int id = 0; id < 10; ++id) {
    container.push_back(makeParticle(id));
  }
  const std::size_t blocksBefore = container.blockCount();

  container.clear();
  BOOST_CHECK_EQUAL(container.size(), 0u);
  BOOST_CHECK(container.empty());
  BOOST_CHECK_EQUAL(container.activeBlockCount(), 0u);
  BOOST_CHECK_EQUAL(container.blockCount(), blocksBefore);
  BOOST_CHECK_EQUAL(container.capacity(), blocksBefore * 4);
  BOOST_CHECK(idsOf(container).empty());
  checkInvariants(container);

  // Refilling within the existing capacity must not allocate new blocks.
  for (int id = 0; id < 12; ++id) {
    container.push_back(makeParticle(id));
  }
  BOOST_CHECK_EQUAL(container.blockCount(), blocksBefore);
  BOOST_CHECK(idsOf(container) == range(12));
  checkInvariants(container);
}

BOOST_AUTO_TEST_CASE(reserve_blocks_adds_pooled_blocks) {
  Container container(4);
  container.reserveBlocks(5);

  BOOST_CHECK_EQUAL(container.blockCount(), 5u);
  BOOST_CHECK_EQUAL(container.activeBlockCount(), 0u);
  BOOST_CHECK_EQUAL(container.size(), 0u);

  // Reserving fewer blocks than already owned changes nothing.
  container.reserveBlocks(2);
  BOOST_CHECK_EQUAL(container.blockCount(), 5u);

  // Pushing within reserved capacity must not allocate further blocks.
  for (int id = 0; id < 20; ++id) {
    container.push_back(makeParticle(id));
  }
  BOOST_CHECK_EQUAL(container.blockCount(), 5u);
  BOOST_CHECK_EQUAL(container.activeBlockCount(), 5u);
  checkInvariants(container);
}

BOOST_AUTO_TEST_CASE(reserve_particles_rounds_up_to_whole_blocks) {
  Container container(4);
  container.reserveParticles(9);

  BOOST_CHECK_EQUAL(container.blockCount(), 3u);
  BOOST_CHECK_EQUAL(container.capacity(), 12u);
  BOOST_CHECK_EQUAL(container.size(), 0u);

  container.reserveParticles(12);
  BOOST_CHECK_EQUAL(container.blockCount(), 3u);
  BOOST_CHECK_EQUAL(container.capacity(), 12u);
}

BOOST_AUTO_TEST_CASE(release_free_blocks_keeps_active_particles) {
  Container container(4);
  for (int id = 0; id < 10; ++id) {
    container.push_back(makeParticle(id));
  }
  container.reserveBlocks(6);
  BOOST_CHECK_EQUAL(container.blockCount(), 6u);

  container.releaseFreeBlocks();
  BOOST_CHECK_EQUAL(container.blockCount(), 3u);
  BOOST_CHECK_EQUAL(container.activeBlockCount(), 3u);
  BOOST_CHECK_EQUAL(container.size(), 10u);
  BOOST_CHECK(idsOf(container) == range(10));
  checkInvariants(container);
}

BOOST_AUTO_TEST_CASE(clear_then_release_empties_container) {
  Container container(4);
  for (int id = 0; id < 10; ++id) {
    container.push_back(makeParticle(id));
  }
  container.clear();
  container.releaseFreeBlocks();

  BOOST_CHECK_EQUAL(container.size(), 0u);
  BOOST_CHECK_EQUAL(container.blockCount(), 0u);
  BOOST_CHECK_EQUAL(container.capacity(), 0u);
  BOOST_CHECK_EQUAL(container.blockCapacity(), 4u);
}

BOOST_AUTO_TEST_CASE(move_construction_transfers_blocks) {
  Container source(4);
  for (int id = 0; id < 10; ++id) {
    source.push_back(makeParticle(id));
  }
  const TestParticle *address = &source.block(0)[std::ptrdiff_t{0}];

  Container target(std::move(source));

  BOOST_CHECK_EQUAL(target.size(), 10u);
  BOOST_CHECK_EQUAL(target.blockCount(), 3u);
  BOOST_CHECK_EQUAL(&target.block(0)[std::ptrdiff_t{0}], address);
  BOOST_CHECK(idsOf(target) == range(10));

  // The moved-from container is empty and still usable.
  BOOST_CHECK_EQUAL(source.size(), 0u);
  BOOST_CHECK_EQUAL(source.blockCount(), 0u);
  BOOST_CHECK_EQUAL(source.blockCapacity(), 4u);
  source.push_back(makeParticle(42));
  BOOST_CHECK_EQUAL(source.size(), 1u);
  checkInvariants(source);
}

BOOST_AUTO_TEST_CASE(move_assignment_replaces_blocks) {
  Container source(4);
  for (int id = 0; id < 6; ++id) {
    source.push_back(makeParticle(id));
  }

  Container target(2);
  for (int id = 100; id < 103; ++id) {
    target.push_back(makeParticle(id));
  }

  target = std::move(source);

  BOOST_CHECK_EQUAL(target.blockCapacity(), 4u);
  BOOST_CHECK(idsOf(target) == range(6));
  BOOST_CHECK_EQUAL(source.size(), 0u);
  BOOST_CHECK_EQUAL(source.blockCount(), 0u);
  checkInvariants(target);
}

BOOST_AUTO_TEST_CASE(copying_is_disabled) {
  static_assert(!std::is_copy_constructible<Container>::value, "copy construction must be disabled");
  static_assert(!std::is_copy_assignable<Container>::value, "copy assignment must be disabled");
  static_assert(std::is_nothrow_move_constructible<Container>::value, "move construction must not throw");
  static_assert(std::is_nothrow_move_assignable<Container>::value, "move assignment must not throw");
  BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(block_storage_is_released_with_container) {
  const int baseline = TrackedParticle::alive;
  {
    TrackedContainer container(3);
    for (int id = 0; id < 10; ++id) {
      container.push_back(TrackedParticle(id, 0.0));
    }
    // Every slot of every owned block is a live particle.
    BOOST_CHECK_EQUAL(TrackedParticle::alive, baseline + static_cast<int>(container.capacity()));

    container.clear();
    BOOST_CHECK_EQUAL(TrackedParticle::alive, baseline + static_cast<int>(container.capacity()));

    container.releaseFreeBlocks();
    BOOST_CHECK_EQUAL(TrackedParticle::alive, baseline);

    for (int id = 0; id < 7; ++id) {
      container.push_back(TrackedParticle(id, 0.0));
    }
    BOOST_CHECK_EQUAL(TrackedParticle::alive, baseline + static_cast<int>(container.capacity()));
  }
  BOOST_CHECK_EQUAL(TrackedParticle::alive, baseline);
}

BOOST_AUTO_TEST_CASE(random_operations_match_reference) {
  const std::size_t blockCapacity = 5;
  Container container(blockCapacity);
  std::vector<int> reference;
  std::mt19937 rng(12345);
  int nextId = 0;

  for (int step = 0; step < 2000; ++step) {
    const int op = static_cast<int>(rng() % 100);
    if (op < 60) {
      // Growth must not move existing blocks.
      std::vector<const void *> before;
      for (std::size_t i = 0; i < container.activeBlockCount(); ++i) {
        before.push_back(static_cast<const void *>(&container.block(i)));
      }

      container.push_back(makeParticle(nextId));
      reference.push_back(nextId);
      ++nextId;

      for (std::size_t i = 0; i < before.size(); ++i) {
        BOOST_REQUIRE_EQUAL(static_cast<const void *>(&container.block(i)), before[i]);
      }
    } else if (op < 70) {
      container.clear();
      reference.clear();
    } else if (op < 85) {
      container.reserveParticles(rng() % 40);
    } else if (op < 95) {
      container.releaseFreeBlocks();
    } else {
      container.reserveBlocks(rng() % 10);
    }

    checkInvariants(container);
    BOOST_REQUIRE(idsOf(container) == reference);
  }
}

BOOST_AUTO_TEST_CASE(emplace_back_constructs_in_place) {
  const int baseline = TrackedParticle::alive;
  TrackedContainer container(2);
  for (int id = 0; id < 5; ++id) {
    container.emplace_back(id, 0.25 * id);
  }

  BOOST_CHECK_EQUAL(container.size(), 5u);
  // Temporaries are destroyed, so only the block slots remain live.
  BOOST_CHECK_EQUAL(TrackedParticle::alive, baseline + static_cast<int>(container.capacity()));
  int expectedId = 0;
  container.forEach([&expectedId](const TrackedParticle &p) {
    BOOST_CHECK_EQUAL(p.id, expectedId);
    ++expectedId;
  });
  BOOST_CHECK_EQUAL(expectedId, 5);
}

BOOST_AUTO_TEST_CASE(insert_appends_range_with_single_reservation) {
  Container container(4);
  container.push_back(makeParticle(0));

  std::vector<TestParticle> batch;
  for (int id = 1; id < 11; ++id) {
    batch.push_back(makeParticle(id));
  }
  container.insert(batch.begin(), batch.end());

  BOOST_CHECK_EQUAL(container.size(), 11u);
  BOOST_CHECK_EQUAL(container.blockCount(), 3u);
  BOOST_CHECK(idsOf(container) == range(11));
  checkInvariants(container);
}

BOOST_AUTO_TEST_CASE(mutable_for_each_updates_particles) {
  Container container(3);
  for (int id = 0; id < 7; ++id) {
    container.push_back(makeParticle(id));
  }
  container.forEach([](TestParticle &p) { p.x += 1.0; });

  container.forEach([](const TestParticle &p) {
    BOOST_CHECK_CLOSE(p.x, 0.5 * p.id + 1.0, 1e-9);
  });
}

BOOST_AUTO_TEST_CASE(for_each_block_reports_used_slots) {
  Container container(4);
  for (int id = 0; id < 10; ++id) {
    container.push_back(makeParticle(id));
  }

  std::size_t total = 0;
  std::size_t blocks = 0;
  container.forEachBlock([&total, &blocks](const auto &, std::size_t used) {
    total += used;
    ++blocks;
  });
  BOOST_CHECK_EQUAL(total, 10u);
  BOOST_CHECK_EQUAL(blocks, 3u);
}

BOOST_AUTO_TEST_CASE(erase_if_removes_matching_particles) {
  Container container(4);
  for (int id = 0; id < 10; ++id) {
    container.push_back(makeParticle(id));
  }

  const std::size_t removed = container.eraseIf([](const TestParticle &p) { return p.id % 3 == 0; });

  BOOST_CHECK_EQUAL(removed, 4u);
  BOOST_CHECK_EQUAL(container.size(), 6u);
  std::vector<int> ids = idsOf(container);
  std::sort(ids.begin(), ids.end());
  BOOST_CHECK(ids == (std::vector<int>{1, 2, 4, 5, 7, 8}));
  checkInvariants(container);
}

BOOST_AUTO_TEST_CASE(extract_if_moves_removed_particles_to_sink) {
  Container container(4);
  for (int id = 0; id < 9; ++id) {
    container.push_back(makeParticle(id));
  }

  std::vector<int> extracted;
  const std::size_t removed = container.extractIf(
      [](const TestParticle &p) { return p.id >= 5; },
      [&extracted](TestParticle &&p) { extracted.push_back(p.id); });

  BOOST_CHECK_EQUAL(removed, 4u);
  std::sort(extracted.begin(), extracted.end());
  BOOST_CHECK(extracted == (std::vector<int>{5, 6, 7, 8}));
  BOOST_CHECK_EQUAL(container.size(), 5u);
  checkInvariants(container);
}

BOOST_AUTO_TEST_CASE(erase_retires_empty_blocks_without_relocating_others) {
  Container container(4);
  for (int id = 0; id < 12; ++id) {
    container.push_back(makeParticle(id));
  }
  const void *lastBlock = &container.block(2);

  // Empties the middle block.
  container.eraseIf([](const TestParticle &p) { return p.id >= 4 && p.id < 8; });

  BOOST_CHECK_EQUAL(container.size(), 8u);
  BOOST_CHECK_EQUAL(container.activeBlockCount(), 2u);
  BOOST_CHECK_EQUAL(container.blockCount(), 3u);
  BOOST_CHECK_EQUAL(static_cast<const void *>(&container.block(1)), lastBlock);
  checkInvariants(container);

  // The retired block is reused rather than allocated.
  for (int id = 100; id < 104; ++id) {
    container.push_back(makeParticle(id));
  }
  BOOST_CHECK_EQUAL(container.blockCount(), 3u);
  checkInvariants(container);
}

BOOST_AUTO_TEST_CASE(append_fills_free_slots_before_allocating) {
  Container container(4);
  for (int id = 0; id < 8; ++id) {
    container.push_back(makeParticle(id));
  }
  // Leaves two partial blocks with one free slot each.
  container.eraseIf([](const TestParticle &p) { return p.id == 1 || p.id == 5; });
  BOOST_CHECK_EQUAL(container.blockCount(), 2u);

  container.push_back(makeParticle(20));
  container.push_back(makeParticle(21));
  BOOST_CHECK_EQUAL(container.blockCount(), 2u);
  checkInvariants(container);

  container.push_back(makeParticle(22));
  BOOST_CHECK_EQUAL(container.blockCount(), 3u);
  BOOST_CHECK_EQUAL(container.size(), 9u);
  checkInvariants(container);
}

BOOST_AUTO_TEST_CASE(random_append_remove_matches_reference_multiset) {
  Container container(5);
  std::vector<int> reference;
  std::mt19937 rng(2024);
  int nextId = 0;

  for (int step = 0; step < 3000; ++step) {
    const int op = static_cast<int>(rng() % 100);
    if (op < 45) {
      container.push_back(makeParticle(nextId));
      reference.push_back(nextId);
      ++nextId;
    } else if (op < 55) {
      std::vector<TestParticle> batch;
      const int count = static_cast<int>(rng() % 8);
      for (int k = 0; k < count; ++k) {
        batch.push_back(makeParticle(nextId));
        reference.push_back(nextId);
        ++nextId;
      }
      container.insert(batch.begin(), batch.end());
    } else if (op < 85) {
      // Removing a residue class leaves partial and empty blocks behind.
      const int modulus = 2 + static_cast<int>(rng() % 4);
      const int residue = static_cast<int>(rng() % modulus);
      const auto matches = [modulus, residue](const TestParticle &p) { return p.id % modulus == residue; };
      const std::size_t removed = container.eraseIf(matches);
      const auto newEnd = std::remove_if(reference.begin(), reference.end(), [modulus, residue](int id) {
        return id % modulus == residue;
      });
      BOOST_REQUIRE_EQUAL(removed, static_cast<std::size_t>(reference.end() - newEnd));
      reference.erase(newEnd, reference.end());
    } else if (op < 92) {
      container.clear();
      reference.clear();
    } else if (op < 97) {
      container.releaseFreeBlocks();
    } else {
      container.reserveParticles(rng() % 50);
    }

    checkInvariants(container);
    std::vector<int> actual = idsOf(container);
    std::sort(actual.begin(), actual.end());
    std::sort(reference.begin(), reference.end());
    BOOST_REQUIRE(actual == reference);
  }
}

BOOST_AUTO_TEST_CASE(factory_applies_block_capacity) {
  schnek::ParticleContainerFactory<Container> factory(4);
  Container container = factory.makeContainer(localRange(), localDomain());

  BOOST_CHECK_EQUAL(container.blockCapacity(), 4u);
  BOOST_CHECK(container.empty());
  BOOST_CHECK_EQUAL(container.blockCount(), 0u);

  for (int id = 0; id < 5; ++id) {
    container.push_back(makeParticle(id));
  }
  BOOST_CHECK_EQUAL(container.blockUsed(0), 4u);
  BOOST_CHECK_EQUAL(container.blockUsed(1), 1u);
  checkInvariants(container);
}

BOOST_AUTO_TEST_CASE(factory_reserves_initial_particles) {
  schnek::ParticleContainerFactory<Container> factory(4, 10);
  Container container = factory.makeContainer(localRange(), localDomain());

  BOOST_CHECK_EQUAL(container.size(), 0u);
  BOOST_CHECK_EQUAL(container.activeBlockCount(), 0u);
  BOOST_CHECK_EQUAL(container.blockCount(), 3u);
  BOOST_CHECK_EQUAL(container.capacity(), 12u);
  checkInvariants(container);
}

BOOST_AUTO_TEST_CASE(registrations_keep_their_own_block_capacity) {
  using Registration = schnek::internal::ParticleRegistrationImpl<1, schnek::ArrayNoArgCheck, Container, TestAccessor>;
  using Wrapper = schnek::internal::ParticleWrapperImpl<Container, TestAccessor>;

  Registration small(schnek::ParticleContainerFactory<Container>(2), TestAccessor{});
  Registration large(schnek::ParticleContainerFactory<Container>(16, 32), TestAccessor{});

  auto smallWrapper = std::static_pointer_cast<Wrapper>(small.makeContainer(localRange(), localDomain()));
  auto largeWrapper = std::static_pointer_cast<Wrapper>(large.makeContainer(localRange(), localDomain()));

  BOOST_CHECK_EQUAL(smallWrapper->container.blockCapacity(), 2u);
  BOOST_CHECK_EQUAL(largeWrapper->container.blockCapacity(), 16u);
  BOOST_CHECK_EQUAL(largeWrapper->container.capacity(), 32u);

  // Each call creates a fresh container, as addLocalRange() does after a rebalance.
  auto freshWrapper = std::static_pointer_cast<Wrapper>(large.makeContainer(localRange(), localDomain()));
  BOOST_CHECK(freshWrapper->container.empty());
  BOOST_CHECK_EQUAL(freshWrapper->container.blockCapacity(), 16u);
  BOOST_CHECK_EQUAL(freshWrapper->container.capacity(), 32u);
}

// Counts construction, assignment, copy and move. Growth must assign only the
// newly appended particle and never copy or move particles already stored.
struct CountedParticle {
  static inline int defaultConstructed = 0;
  static inline int constructed = 0;
  static inline int copied = 0;
  static inline int moved = 0;
  static inline int assigned = 0;

  static void resetCounters() {
    defaultConstructed = 0;
    constructed = 0;
    copied = 0;
    moved = 0;
    assigned = 0;
  }

  int id;
  double x;

  CountedParticle() : id(-1), x(0.0) { ++defaultConstructed; }
  CountedParticle(int id_, double x_) : id(id_), x(x_) { ++constructed; }
  CountedParticle(const CountedParticle &other) : id(other.id), x(other.x) { ++copied; }
  CountedParticle(CountedParticle &&other) noexcept : id(other.id), x(other.x) { ++moved; }
  CountedParticle &operator=(const CountedParticle &other) {
    ++assigned;
    id = other.id;
    x = other.x;
    return *this;
  }
  CountedParticle &operator=(CountedParticle &&other) noexcept {
    ++assigned;
    id = other.id;
    x = other.x;
    return *this;
  }
};

struct LargePayloadParticle {
  int id;
  std::array<double, 128> payload;
};

LargePayloadParticle makeLargeParticle(int id) {
  LargePayloadParticle p{};
  p.id = id;
  for (std::size_t k = 0; k < p.payload.size(); ++k) {
    p.payload[k] = id + 0.25 * static_cast<double>(k);
  }
  return p;
}

// Collects the address of every active particle slot in a container.
// Uses non-const access so that the element addresses are real storage, not temporaries.
std::set<const TestParticle *> elementAddresses(Container &container) {
  std::set<const TestParticle *> addresses;
  for (std::size_t i = 0; i < container.activeBlockCount(); ++i) {
    auto &data = container.block(i);
    for (std::size_t j = 0; j < container.blockUsed(i); ++j) {
      addresses.insert(&data[static_cast<std::ptrdiff_t>(j)]);
    }
  }
  return addresses;
}

// Collects the storage address of every block, active or pooled.
std::set<const TestParticle *> blockStorage(Container &container) {
  std::set<const TestParticle *> addresses;
  container.forEachBlock([&addresses](auto &data, std::size_t) {
    addresses.insert(&data[std::ptrdiff_t{0}]);
  });
  return addresses;
}

BOOST_AUTO_TEST_CASE(block_boundaries_keep_invariants_for_each_size) {
  for (std::size_t blockCapacity : {std::size_t{1}, std::size_t{2}, std::size_t{3}, std::size_t{8}}) {
    for (std::size_t blocks = 0; blocks <= 4; ++blocks) {
      for (std::ptrdiff_t delta = -1; delta <= 1; ++delta) {
        const std::ptrdiff_t signedCount = static_cast<std::ptrdiff_t>(blocks * blockCapacity) + delta;
        if (signedCount < 0) {
          continue;
        }
        const std::size_t count = static_cast<std::size_t>(signedCount);

        Container container(blockCapacity);
        for (int id = 0; id < static_cast<int>(count); ++id) {
          container.push_back(makeParticle(id));
        }

        BOOST_REQUIRE_EQUAL(container.size(), count);
        BOOST_REQUIRE_EQUAL(container.activeBlockCount(), (count + blockCapacity - 1) / blockCapacity);
        BOOST_CHECK(idsOf(container) == range(static_cast<int>(count)));
        checkInvariants(container);
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(full_and_partial_final_blocks_are_reported_exactly) {
  Container container(4);
  for (int id = 0; id < 8; ++id) {
    container.push_back(makeParticle(id));
  }
  BOOST_CHECK_EQUAL(container.activeBlockCount(), 2u);
  BOOST_CHECK_EQUAL(container.blockUsed(0), 4u);
  BOOST_CHECK_EQUAL(container.blockUsed(1), 4u);
  checkInvariants(container);

  container.push_back(makeParticle(8));
  BOOST_CHECK_EQUAL(container.activeBlockCount(), 3u);
  BOOST_CHECK_EQUAL(container.blockUsed(2), 1u);
  checkInvariants(container);
}

BOOST_AUTO_TEST_CASE(sparse_container_reuses_partial_blocks_before_allocating) {
  Container container(4);
  for (int id = 0; id < 64; ++id) {
    container.push_back(makeParticle(id));
  }
  BOOST_REQUIRE_EQUAL(container.blockCount(), 16u);

  // Keep the first particle of every block, so each block holds exactly one.
  container.eraseIf([](const TestParticle &p) { return p.id % 4 != 0; });
  BOOST_CHECK_EQUAL(container.size(), 16u);
  BOOST_CHECK_EQUAL(container.activeBlockCount(), 16u);
  for (std::size_t i = 0; i < container.activeBlockCount(); ++i) {
    BOOST_CHECK_EQUAL(container.blockUsed(i), 1u);
  }
  checkInvariants(container);

  // Appending into the three free slots of each block allocates nothing new.
  for (int id = 64; id < 80; ++id) {
    container.push_back(makeParticle(id));
  }
  BOOST_CHECK_EQUAL(container.blockCount(), 16u);
  BOOST_CHECK_EQUAL(container.size(), 32u);
  checkInvariants(container);

  container.eraseIf([](const TestParticle &) { return true; });
  BOOST_CHECK(container.empty());
  BOOST_CHECK_EQUAL(container.activeBlockCount(), 0u);
  BOOST_CHECK_EQUAL(container.blockCount(), 16u);
  container.releaseFreeBlocks();
  BOOST_CHECK_EQUAL(container.blockCount(), 0u);
}

BOOST_AUTO_TEST_CASE(alternating_growth_and_shrink_reuses_the_same_storage) {
  const std::size_t blockCapacity = 5;
  const int total = 100;
  Container container(blockCapacity);

  std::set<const TestParticle *> initialStorage;
  for (int cycle = 0; cycle < 10; ++cycle) {
    for (int id = 0; id < total; ++id) {
      container.push_back(makeParticle(id));
    }
    BOOST_REQUIRE_EQUAL(container.size(), static_cast<std::size_t>(total));
    BOOST_REQUIRE_EQUAL(container.blockCount(), 20u);
    checkInvariants(container);

    if (cycle == 0) {
      initialStorage = blockStorage(container);
      BOOST_REQUIRE_EQUAL(initialStorage.size(), 20u);
    } else {
      // Refilling after a full shrink reuses the pooled blocks.
      BOOST_CHECK(blockStorage(container) == initialStorage);
    }

    container.eraseIf([](const TestParticle &) { return true; });
    BOOST_REQUIRE(container.empty());
    BOOST_CHECK_EQUAL(container.activeBlockCount(), 0u);
    BOOST_CHECK_EQUAL(container.blockCount(), 20u);
  }
}

BOOST_AUTO_TEST_CASE(large_payloads_survive_growth_and_compaction) {
  using LargeContainer = schnek::BlockParticleContainer<LargePayloadParticle>;
  LargeContainer container(3);
  for (int id = 0; id < 50; ++id) {
    container.push_back(makeLargeParticle(id));
  }
  container.eraseIf([](const LargePayloadParticle &p) { return p.id % 3 == 1; });
  BOOST_REQUIRE_EQUAL(container.size(), 33u);

  container.forEach([](const LargePayloadParticle &p) {
    BOOST_REQUIRE_EQUAL(p.id % 3 == 1, false);
    for (std::size_t k = 0; k < p.payload.size(); ++k) {
      BOOST_REQUIRE_EQUAL(p.payload[k], p.id + 0.25 * static_cast<double>(k));
    }
  });
}

BOOST_AUTO_TEST_CASE(containers_of_different_particle_types_grow_independently) {
  using LargeContainer = schnek::BlockParticleContainer<LargePayloadParticle>;
  Container small(2);
  LargeContainer large(5);
  for (int id = 0; id < 30; ++id) {
    small.push_back(makeParticle(id));
    large.push_back(makeLargeParticle(id));
  }

  BOOST_CHECK_EQUAL(small.activeBlockCount(), 15u);
  BOOST_CHECK_EQUAL(large.activeBlockCount(), 6u);
  BOOST_CHECK(idsOf(small) == range(30));
  checkInvariants(small);

  std::vector<int> largeIds;
  large.forEach([&largeIds](const LargePayloadParticle &p) { largeIds.push_back(p.id); });
  BOOST_CHECK(largeIds == range(30));
}

BOOST_AUTO_TEST_CASE(growth_allocates_blocks_without_relocating_particles) {
  using CountedContainer = schnek::BlockParticleContainer<CountedParticle>;
  const std::size_t blockCapacity = 8;
  const int total = 25 * static_cast<int>(blockCapacity);

  CountedParticle::resetCounters();
  CountedContainer container(blockCapacity);
  for (int id = 0; id < total; ++id) {
    container.emplace_back(id, 0.5 * id);
  }

  BOOST_CHECK_EQUAL(container.activeBlockCount(), 25u);
  BOOST_CHECK_EQUAL(container.blockCount(), 25u);
  // Only the new block slots are default-constructed, one per slot of each allocated block.
  BOOST_CHECK_EQUAL(CountedParticle::defaultConstructed, static_cast<int>(container.capacity()));
  // Each particle is built once and assigned once. Stored particles are never copied or moved.
  BOOST_CHECK_EQUAL(CountedParticle::constructed, total);
  BOOST_CHECK_EQUAL(CountedParticle::assigned, total);
  BOOST_CHECK_EQUAL(CountedParticle::copied, 0);
  BOOST_CHECK_EQUAL(CountedParticle::moved, 0);

  int expectedId = 0;
  container.forEach([&expectedId](const CountedParticle &p) { BOOST_CHECK_EQUAL(p.id, expectedId++); });
  BOOST_CHECK_EQUAL(expectedId, total);
}

BOOST_AUTO_TEST_CASE(block_storage_is_not_aliased_and_moves_transfer_ownership) {
  Container source(3);
  for (int id = 0; id < 30; ++id) {
    source.push_back(makeParticle(id));
  }

  const std::set<const TestParticle *> before = elementAddresses(source);
  BOOST_CHECK_EQUAL(before.size(), 30u);

  Container other(3);
  for (int id = 100; id < 130; ++id) {
    other.push_back(makeParticle(id));
  }
  const std::set<const TestParticle *> otherAddresses = elementAddresses(other);
  std::vector<const TestParticle *> common;
  std::set_intersection(before.begin(), before.end(), otherAddresses.begin(), otherAddresses.end(),
                        std::back_inserter(common));
  BOOST_CHECK(common.empty());

  Container target(std::move(source));
  BOOST_CHECK(source.empty());
  BOOST_CHECK_EQUAL(source.blockCount(), 0u);
  BOOST_CHECK(elementAddresses(target) == before);
  BOOST_CHECK(idsOf(target) == range(30));

  Container assigned(2);
  assigned.push_back(makeParticle(99));
  assigned = std::move(target);
  BOOST_CHECK(elementAddresses(assigned) == before);
  BOOST_CHECK_EQUAL(assigned.blockCapacity(), 3u);
  checkInvariants(assigned);
}

// Checks that the bins cover every particle, that each particle sits in the bin given by `keys[id]`,
// and that the container invariants hold.
void checkBins(const Container &container, const std::vector<std::size_t> &keys) {
  BOOST_REQUIRE(container.isBinned());
  std::size_t total = 0;
  for (std::size_t bin = 0; bin < container.binCount(); ++bin) {
    std::size_t count = 0;
    container.forEachInBin(bin, [&](const TestParticle &p) {
      BOOST_CHECK_EQUAL(keys.at(static_cast<std::size_t>(p.id)), bin);
      ++count;
    });
    BOOST_CHECK_EQUAL(count, container.binSize(bin));
    total += count;
  }
  BOOST_CHECK_EQUAL(total, container.size());
  checkInvariants(container);
}

std::vector<int> sortedIdsOf(const Container &container) {
  std::vector<int> ids = idsOf(container);
  std::sort(ids.begin(), ids.end());
  return ids;
}

BOOST_AUTO_TEST_CASE(rebin_groups_particles_by_bin) {
  constexpr std::size_t bins = 4;
  Container container(3);
  for (int id = 0; id < 30; ++id) {
    container.push_back(makeParticle(id));
  }
  std::vector<std::size_t> keys(30);
  for (int id = 0; id < 30; ++id) {
    keys[static_cast<std::size_t>(id)] = static_cast<std::size_t>(id) % bins;
  }

  container.rebin(bins, [&keys](const TestParticle &p) { return keys[static_cast<std::size_t>(p.id)]; });

  BOOST_CHECK(container.isBinned());
  BOOST_CHECK_EQUAL(container.binCount(), bins);
  BOOST_CHECK_EQUAL(container.size(), 30u);
  checkBins(container, keys);
  BOOST_CHECK(sortedIdsOf(container) == range(30));

  const std::size_t expectedSizes[] = {8, 8, 7, 7};
  for (std::size_t bin = 0; bin < bins; ++bin) {
    BOOST_CHECK_EQUAL(container.binSize(bin), expectedSizes[bin]);
    const auto [first, last] = container.binBlockRange(bin);
    // Each bin holds ceil(count / capacity) blocks, and only its last block may be partial.
    BOOST_CHECK_EQUAL(last - first, (expectedSizes[bin] + 2) / 3);
    for (std::size_t i = first; i + 1 < last; ++i) {
      BOOST_CHECK_EQUAL(container.blockUsed(i), 3u);
    }
  }
  BOOST_CHECK_EQUAL(container.binBlockRange(0).first, 0u);
  BOOST_CHECK_EQUAL(container.binBlockRange(bins - 1).second, container.activeBlockCount());
}

BOOST_AUTO_TEST_CASE(rebin_handles_empty_bins_and_empty_container) {
  Container container(4);
  container.rebin(5, [](const TestParticle &) { return std::size_t{2}; });
  BOOST_CHECK(container.isBinned());
  BOOST_CHECK_EQUAL(container.binCount(), 5u);
  BOOST_CHECK_EQUAL(container.activeBlockCount(), 0u);
  for (std::size_t bin = 0; bin < 5; ++bin) {
    BOOST_CHECK_EQUAL(container.binSize(bin), 0u);
    const auto [first, last] = container.binBlockRange(bin);
    BOOST_CHECK_EQUAL(first, last);
  }

  for (int id = 0; id < 10; ++id) {
    container.push_back(makeParticle(id));
  }
  std::vector<std::size_t> keys(10);
  for (int id = 0; id < 10; ++id) {
    keys[static_cast<std::size_t>(id)] = id < 5 ? 0 : 2;
  }
  container.rebin(5, [&keys](const TestParticle &p) { return keys[static_cast<std::size_t>(p.id)]; });
  checkBins(container, keys);
  for (std::size_t bin : {1u, 3u, 4u}) {
    const auto [first, last] = container.binBlockRange(bin);
    BOOST_CHECK_EQUAL(first, last);
  }
  BOOST_CHECK_EQUAL(container.binSize(0), 5u);
  BOOST_CHECK_EQUAL(container.binSize(2), 5u);
}

BOOST_AUTO_TEST_CASE(heavy_bin_spans_several_blocks) {
  Container container(10);
  for (int id = 0; id < 103; ++id) {
    container.push_back(makeParticle(id));
  }
  container.rebin(2, [](const TestParticle &p) { return p.id < 100 ? std::size_t{0} : std::size_t{1}; });

  const auto heavy = container.binBlockRange(0);
  const auto light = container.binBlockRange(1);
  BOOST_CHECK_EQUAL(heavy.second - heavy.first, 10u);
  BOOST_CHECK_EQUAL(light.second - light.first, 1u);
  BOOST_CHECK_EQUAL(container.blockUsed(light.first), 3u);
  BOOST_CHECK_EQUAL(container.binSize(0), 100u);
}

BOOST_AUTO_TEST_CASE(rebin_reuses_blocks_for_the_same_layout) {
  constexpr std::size_t capacity = 4;
  Container container(capacity);
  for (int id = 0; id < 50; ++id) {
    container.push_back(makeParticle(id));
  }
  std::vector<std::size_t> keys(50);
  for (int id = 0; id < 50; ++id) {
    keys[static_cast<std::size_t>(id)] = static_cast<std::size_t>(id) % 5;
  }
  const auto key = [&keys](const TestParticle &p) { return keys[static_cast<std::size_t>(p.id)]; };

  container.rebin(5, key);
  const std::size_t blocks = container.blockCount();

  // Particles move between blocks, so only the set of storage slots is fixed. Every
  // address seen over repeated rebins must be one of the blocks' `blocks * 4` slots.
  std::set<const TestParticle *> slots = elementAddresses(container);
  for (int round = 0; round < 5; ++round) {
    container.rebin(5, key);
    BOOST_CHECK_EQUAL(container.blockCount(), blocks);
    const std::set<const TestParticle *> current = elementAddresses(container);
    slots.insert(current.begin(), current.end());
    checkBins(container, keys);
  }
  BOOST_CHECK_LE(slots.size(), blocks * capacity);
}

BOOST_AUTO_TEST_CASE(rebin_rejects_out_of_range_keys_and_keeps_particles) {
  Container container(3);
  for (int id = 0; id < 10; ++id) {
    container.push_back(makeParticle(id));
  }
  BOOST_CHECK_THROW(container.rebin(2, [](const TestParticle &p) { return static_cast<std::size_t>(p.id); }),
                    schnek::ScheckException);
  BOOST_CHECK(!container.isBinned());
  BOOST_CHECK_EQUAL(container.binCount(), 0u);
  BOOST_CHECK_EQUAL(container.size(), 10u);
  BOOST_CHECK(sortedIdsOf(container) == range(10));
  checkInvariants(container);
}

BOOST_AUTO_TEST_CASE(rebin_failure_from_key_function_keeps_particles) {
  Container container(3);
  for (int id = 0; id < 10; ++id) {
    container.push_back(makeParticle(id));
  }
  BOOST_CHECK_THROW(container.rebin(3,
                                    [](const TestParticle &p) -> std::size_t {
                                      if (p.id == 5) {
                                        throw std::runtime_error("key failure");
                                      }
                                      return static_cast<std::size_t>(p.id) % 3;
                                    }),
                    std::runtime_error);
  BOOST_CHECK(!container.isBinned());
  BOOST_CHECK_EQUAL(container.size(), 10u);
  BOOST_CHECK(sortedIdsOf(container) == range(10));
  checkInvariants(container);
}

BOOST_AUTO_TEST_CASE(unkeyed_appends_and_clear_remove_binning) {
  Container container(3);
  for (int id = 0; id < 9; ++id) {
    container.push_back(makeParticle(id));
  }
  const auto rebinMod3 = [&container]() {
    container.rebin(3, [](const TestParticle &p) { return static_cast<std::size_t>(p.id) % 3; });
  };

  rebinMod3();
  BOOST_CHECK(container.isBinned());
  container.push_back(makeParticle(9));
  BOOST_CHECK(!container.isBinned());
  BOOST_CHECK_EQUAL(container.binCount(), 0u);
  BOOST_CHECK_EQUAL(container.size(), 10u);

  rebinMod3();
  container.emplace_back(TestParticle{10, 5.0});
  BOOST_CHECK(!container.isBinned());

  rebinMod3();
  const std::vector<TestParticle> extra{makeParticle(11)};
  container.insert(extra.begin(), extra.end());
  BOOST_CHECK(!container.isBinned());
  BOOST_CHECK(sortedIdsOf(container) == range(12));

  rebinMod3();
  const std::size_t blocks = container.blockCount();
  container.clear();
  BOOST_CHECK(!container.isBinned());
  BOOST_CHECK_EQUAL(container.binCount(), 0u);
  BOOST_CHECK(container.empty());
  BOOST_CHECK_EQUAL(container.blockCount(), blocks);
}

BOOST_AUTO_TEST_CASE(extract_keeps_binning_and_drops_empty_bin_blocks) {
  Container container(3);
  for (int id = 0; id < 30; ++id) {
    container.push_back(makeParticle(id));
  }
  std::vector<std::size_t> keys(30);
  for (int id = 0; id < 30; ++id) {
    keys[static_cast<std::size_t>(id)] = static_cast<std::size_t>(id) % 4;
  }
  container.rebin(4, [&keys](const TestParticle &p) { return keys[static_cast<std::size_t>(p.id)]; });

  const std::size_t removed = container.eraseIf([](const TestParticle &p) { return p.id % 2 == 0; });
  BOOST_CHECK_EQUAL(removed, 15u);
  BOOST_CHECK(container.isBinned());
  checkBins(container, keys);

  container.eraseIf([](const TestParticle &p) { return p.id % 4 == 1; });
  BOOST_CHECK(container.isBinned());
  BOOST_CHECK_EQUAL(container.binSize(1), 0u);
  const auto [first, last] = container.binBlockRange(1);
  BOOST_CHECK_EQUAL(first, last);
  checkBins(container, keys);
}

BOOST_AUTO_TEST_CASE(forEachInBin_non_const_updates_particles) {
  Container container(4);
  for (int id = 0; id < 12; ++id) {
    container.push_back(makeParticle(id));
  }
  container.rebin(3, [](const TestParticle &p) { return static_cast<std::size_t>(p.id) / 4; });
  container.forEachInBin(1, [](TestParticle &p) { p.x = -1.0; });
  container.forEach([](const TestParticle &p) {
    const bool inBin1 = p.id >= 4 && p.id < 8;
    BOOST_CHECK_EQUAL(p.x, inBin1 ? -1.0 : 0.5 * p.id);
  });
}

BOOST_AUTO_TEST_CASE(move_transfers_binning) {
  Container source(3);
  for (int id = 0; id < 12; ++id) {
    source.push_back(makeParticle(id));
  }
  std::vector<std::size_t> keys(12);
  for (int id = 0; id < 12; ++id) {
    keys[static_cast<std::size_t>(id)] = static_cast<std::size_t>(id) % 3;
  }
  source.rebin(3, [&keys](const TestParticle &p) { return keys[static_cast<std::size_t>(p.id)]; });

  Container target(std::move(source));
  BOOST_CHECK(target.isBinned());
  BOOST_CHECK_EQUAL(target.binCount(), 3u);
  BOOST_CHECK(!source.isBinned());
  BOOST_CHECK_EQUAL(source.binCount(), 0u);
  checkBins(target, keys);

  Container assigned(2);
  assigned.push_back(makeParticle(99));
  assigned = std::move(target);
  BOOST_CHECK(assigned.isBinned());
  BOOST_CHECK(!target.isBinned());
  checkBins(assigned, keys);
}

BOOST_AUTO_TEST_CASE(randomised_appends_rebins_and_extractions_keep_bins_consistent) {
  constexpr std::size_t bins = 7;
  std::mt19937 gen(20260701);
  std::uniform_int_distribution<std::size_t> keyDist(0, bins - 1);
  std::uniform_int_distribution<int> dropDist(0, 3);
  std::uniform_int_distribution<int> addDist(0, 40);

  Container container(5);
  std::vector<std::size_t> keys;
  std::vector<bool> present;
  int nextId = 0;

  for (int round = 0; round < 30; ++round) {
    const int added = addDist(gen);
    for (int k = 0; k < added; ++k) {
      container.push_back(makeParticle(nextId));
      keys.push_back(0);
      present.push_back(true);
      ++nextId;
    }

    for (auto &key : keys) {
      key = keyDist(gen);
    }
    container.rebin(bins, [&keys](const TestParticle &p) { return keys[static_cast<std::size_t>(p.id)]; });
    checkBins(container, keys);

    const std::size_t before = container.size();
    const std::size_t removed = container.eraseIf([&present, &gen, &dropDist](const TestParticle &p) {
      const bool drop = dropDist(gen) == 0;
      if (drop) {
        present[static_cast<std::size_t>(p.id)] = false;
      }
      return drop;
    });
    const std::size_t expected = static_cast<std::size_t>(std::count(present.begin(), present.end(), true));
    BOOST_CHECK_EQUAL(container.size(), expected);
    BOOST_CHECK_EQUAL(before - removed, expected);
    checkBins(container, keys);

    std::vector<int> survivors;
    for (std::size_t id = 0; id < present.size(); ++id) {
      if (present[id]) {
        survivors.push_back(static_cast<int>(id));
      }
    }
    BOOST_CHECK(sortedIdsOf(container) == survivors);
  }
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

