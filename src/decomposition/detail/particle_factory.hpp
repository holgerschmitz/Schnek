/*
 * particle_factory.hpp
 *
 * Created on: 29 Jun 2026
 * Author: Holger Schmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2026 Holger Schmitz
 *
 * This file is part of Schnek.
 *
 * Schnek is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Schnek is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Schnek.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef SCHNEK_DECOMPOSITION_DETAIL_PARTICLE_FACTORY_HPP_
#define SCHNEK_DECOMPOSITION_DETAIL_PARTICLE_FACTORY_HPP_

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

#include "../../grid/array.hpp"
#include "../../grid/arraycheck.hpp"
#include "../../grid/range.hpp"
#include "../../macros.hpp"
#include "../../util/exceptions.hpp"
#include "../../util/unique.hpp"
#include "block_particle_container.hpp"
#include "grid-communication-traits.hpp"
#include "particle_visitor.hpp"

#include <cstdint>

namespace schnek {

  namespace internal {

    /// True when `c.data()` is callable. The default device-resident traits require it.
    template<class ContainerType, class = void>
    struct has_data : std::false_type {};

    template<class ContainerType>
    struct has_data<ContainerType, std::void_t<decltype(std::declval<ContainerType &>().data())>> : std::true_type {};

    /// Execution space declared by the memory space, or `void` when it declares none.
    template<class MemorySpace, class = void>
    struct MemorySpaceExecutionSpace {
      using type = void;
    };

    template<class MemorySpace>
    struct MemorySpaceExecutionSpace<MemorySpace, std::void_t<typename MemorySpace::execution_space>> {
      using type = typename MemorySpace::execution_space;
    };

    /// `ContainerType::ExecutionSpace` when declared, otherwise the execution space of its memory space.
    template<class ContainerType, class = void>
    struct ContainerExecutionSpace {
      using type = typename MemorySpaceExecutionSpace<typename ContainerType::MemorySpace>::type;
    };

    template<class ContainerType>
    struct ContainerExecutionSpace<ContainerType, std::void_t<typename ContainerType::ExecutionSpace>> {
      using type = typename ContainerType::ExecutionSpace;
    };

    /**
     * Selects how the operations of a particle container run. A container that declares `MemorySpace` dispatches
     * through Kokkos on `type`, which must be able to access that memory space. Other containers loop on the host.
     */
    template<class ContainerType, class = void>
    struct ParticleExecutionSpace {
      using type = void;
      static constexpr bool kokkos_dispatch = false;
    };

    template<class ContainerType>
    struct ParticleExecutionSpace<ContainerType, std::void_t<typename ContainerType::MemorySpace>> {
      using memory_space = typename ContainerType::MemorySpace;
      using type = typename ContainerExecutionSpace<ContainerType>::type;
      static constexpr bool kokkos_dispatch = true;

      static_assert(!std::is_void_v<type>, "particle container declares a MemorySpace without an execution space; declare ExecutionSpace");
#ifdef SCHNEK_HAVE_KOKKOS
      static_assert(
          Kokkos::SpaceAccessibility<type, memory_space>::accessible,
          "particle container execution space cannot access its memory space"
      );
#endif
    };

#ifdef SCHNEK_HAVE_KOKKOS
    /**
     * Deserialises `count` particles from the device-resident bytes `src` and appends them to `c`.
     *
     * `c.size()`, `c.resize(n)` (keeps existing elements) and `c.data()` address the container's memory space.
     */
    template<class ExecSpace, class Serializer, class ContainerType>
    void deviceBulkInsert(ContainerType &c, const std::byte *src, std::size_t count) {
      static_assert(has_data<ContainerType>::value, "device-resident containers must provide data() or specialise ParticleContainerTraits");
      if (count == 0) {
        return;
      }

      using Particle = typename ContainerType::value_type;
      const std::size_t offset = c.size();
      c.resize(offset + count);

      Particle *dst = c.data() + offset;
      const std::size_t particleBytes = Serializer::size();
      Kokkos::parallel_for(
          "schnek:bulkInsertParticles",
          Kokkos::RangePolicy<ExecSpace>(std::size_t{0}, count),
          KOKKOS_LAMBDA(const std::size_t i) { dst[i] = Serializer::deserialize(src + i * particleBytes); }
      );
      Kokkos::fence();
    }

    /**
     * Returns true when `pred(p)` holds for every particle in the device-resident container `c`.
     */
    template<class ExecSpace, class ContainerType, class Predicate>
    bool deviceAllOf(const ContainerType &c, Predicate pred) {
      static_assert(has_data<ContainerType>::value, "device-resident containers must provide data() or specialise ParticleContainerTraits");
      using Particle = typename ContainerType::value_type;
      const Particle *data = c.data();
      bool result = true;
      Kokkos::parallel_reduce(
          "schnek:allOfParticles",
          Kokkos::RangePolicy<ExecSpace>(std::size_t{0}, c.size()),
          KOKKOS_LAMBDA(const std::size_t i, bool &ok) { ok = ok && pred(data[i]); },
          Kokkos::LAnd<bool>(result)
      );
      return result;
    }

    /**
     * Counts the particles of the device-resident container `c` that `classify` assigns to each bucket (1-based).
     */
    template<class ExecSpace, class ContainerType, class Classifier>
    void deviceCountBuckets(
        const ContainerType &c, const Classifier &classify, std::size_t *counts, std::size_t numBuckets
    ) {
      static_assert(
          has_data<ContainerType>::value,
          "device-resident containers must provide data() or specialise ParticleContainerTraits"
      );
      using Particle = typename ContainerType::value_type;
      const Particle *data = c.data();
      const std::size_t n = c.size();
      for (std::size_t b = 0; b < numBuckets; ++b) {
        const int bucket = static_cast<int>(b) + 1;
        std::size_t total = 0;
        Kokkos::parallel_reduce(
            "schnek:countBucketParticles",
            Kokkos::RangePolicy<ExecSpace>(std::size_t{0}, n),
            KOKKOS_LAMBDA(const std::size_t i, std::size_t &acc) {
              if (classify(data[i]) == bucket) {
                ++acc;
              }
            },
            total
        );
        counts[b] = total;
      }

#ifndef NDEBUG
      // Bucket validation costs an extra pass, so it runs in debug builds only.
      std::size_t invalid = 0;
      Kokkos::parallel_reduce(
          "schnek:checkBucketValues",
          Kokkos::RangePolicy<ExecSpace>(std::size_t{0}, n),
          KOKKOS_LAMBDA(const std::size_t i, std::size_t &acc) {
            const int bucket = classify(data[i]);
            if (bucket < 0 || bucket > static_cast<int>(numBuckets)) {
              ++acc;
            }
          },
          invalid
      );
      SCHNEK_ASSERT(invalid == 0, "particle classified outside buckets 0.." << numBuckets);
#endif
    }

    /// Rounds `p` up to a multiple of `alignment`, which must be a power of two.
    inline std::byte *alignDeviceScratch(std::byte *p, std::size_t alignment) {
      const auto addr = reinterpret_cast<std::uintptr_t>(p);
      return reinterpret_cast<std::byte *>((addr + alignment - 1) & ~(alignment - 1));
    }

    /**
     * Partitions the device-resident container `c` in place. Departing particles are serialised into `outPtrs`,
     * survivors are compacted in order. `scratch` holds the survivor and scan storage (see partitionScratchBytes).
     * Only the survivor count is copied back to the host.
     */
    template<class ExecSpace, class Serializer, class ContainerType, class Classifier>
    void devicePartitionAndPack(
        ContainerType &c,
        const Classifier &classify,
        std::byte *const *outPtrs,
        std::size_t numBuckets,
        std::byte *scratch
    ) {
      static_assert(has_data<ContainerType>::value, "device-resident containers must provide data() or specialise ParticleContainerTraits");
      using Particle = typename ContainerType::value_type;
      const std::size_t particleBytes = Serializer::size();
      const std::size_t n = c.size();
      if (n == 0) {
        return;
      }

      Particle *survivors = reinterpret_cast<Particle *>(alignDeviceScratch(scratch, alignof(Particle)));
      std::size_t *position = reinterpret_cast<std::size_t *>(
          alignDeviceScratch(reinterpret_cast<std::byte *>(survivors + n), alignof(std::size_t))
      );
      Particle *data = c.data();

      // Target 0 gathers survivors; target b (1-based) packs removal bucket b.
      std::size_t survivorCount = 0;
      for (int target = 0; target <= static_cast<int>(numBuckets); ++target) {
        std::size_t total = 0;
        Kokkos::parallel_scan(
            "schnek:exclusiveScanBucket",
            Kokkos::RangePolicy<ExecSpace>(std::size_t{0}, n),
            KOKKOS_LAMBDA(const std::size_t i, std::size_t &update, const bool isFinal) {
              if (isFinal) {
                position[i] = update;
              }
              if (classify(data[i]) == target) {
                ++update;
              }
            },
            total
        );

        if (target == 0) {
          Kokkos::parallel_for(
              "schnek:gatherSurvivors",
              Kokkos::RangePolicy<ExecSpace>(std::size_t{0}, n),
              KOKKOS_LAMBDA(const std::size_t i) {
                if (classify(data[i]) == 0) {
                  survivors[position[i]] = data[i];
                }
              }
          );
          survivorCount = total;
        } else {
          std::byte *dst = outPtrs[static_cast<std::size_t>(target) - 1];
          Kokkos::parallel_for(
              "schnek:packBucket",
              Kokkos::RangePolicy<ExecSpace>(std::size_t{0}, n),
              KOKKOS_LAMBDA(const std::size_t i) {
                if (classify(data[i]) == target) {
                  Serializer::serialize(data[i], dst + position[i] * particleBytes);
                }
              }
          );
        }
      }

      Kokkos::parallel_for(
          "schnek:compactSurvivors",
          Kokkos::RangePolicy<ExecSpace>(std::size_t{0}, survivorCount),
          KOKKOS_LAMBDA(const std::size_t i) { data[i] = survivors[i]; }
      );
      Kokkos::fence();
      c.resize(survivorCount);
    }
#else
    template<class ExecSpace, class Serializer, class ContainerType>
    void deviceBulkInsert(ContainerType &, const std::byte *, std::size_t) {
      static_assert(sizeof(ContainerType) == 0, "device-resident particle containers require Kokkos support");
    }

    template<class ExecSpace, class ContainerType, class Predicate>
    bool deviceAllOf(const ContainerType &, Predicate) {
      static_assert(sizeof(ContainerType) == 0, "device-resident particle containers require Kokkos support");
      return false;
    }

    template<class ExecSpace, class ContainerType, class Classifier>
    void deviceCountBuckets(const ContainerType &, const Classifier &, std::size_t *, std::size_t) {
      static_assert(sizeof(ContainerType) == 0, "device-resident particle containers require Kokkos support");
    }

    template<class ExecSpace, class Serializer, class ContainerType, class Classifier>
    void devicePartitionAndPack(ContainerType &, const Classifier &, std::byte *const *, std::size_t, std::byte *) {
      static_assert(sizeof(ContainerType) == 0, "device-resident particle containers require Kokkos support");
    }
#endif

  }  // namespace internal

  /**
   * @brief Adapts an arbitrary particle container to the small set of
   * operations the domain decomposition requires.
   *
   * Contract for every specialisation:
   * - `size()` counts stored particles, not capacity.
   * - `insert()` appends one particle. `forEach()` is host-only.
   * - `countBuckets()` does not modify the container. Bucket 0 keeps a particle and buckets `1..numBuckets` remove it.
   *   Any other value throws `ScheckException` on the host. Device containers check in debug builds only.
   * - `partitionAndPack()` runs on the container unchanged since `countBuckets()`. It writes `countBuckets()[b] *
   *   Serializer::size()` bytes per bucket into caller-owned buffers in `ParticleBufferMemorySpace`, which the
   *   container must not retain. An exception leaves the container unspecified.
   * - Both primitives take the bucket count at run time, so a caller can choose the number of destinations. The
   *   `std::array` overloads are convenience forwarders for a compile-time count.
   * - `bulkInsert()` appends `count` particles read from a buffer in `ParticleBufferMemorySpace`, in buffer order.
   * - Survivor order is not part of the contract. Stable `Particle&` references and a single `data()` pointer are not required.
   *
   * The primary template is a contiguous AoS adapter. It needs `value_type`, `size()`, `push_back()`, `operator[]` and
   * `resize()`. Containers that declare `MemorySpace` also need `data()`. Specialise this trait fully for segmented or non-AoS storage.
   * Such containers dispatch on `execution_space`: `ExecutionSpace` if declared, otherwise the execution space of
   * `MemorySpace`. The execution space must be able to access the memory space, which is checked at compile time.
   *
   * @tparam ContainerType the user's particle storage type
   */
  template<class ContainerType, class Enable = void>
  struct ParticleContainerTraits {
      using value_type = typename ContainerType::value_type;

      /// Execution space of the Kokkos dispatch paths; `void` for host containers.
      using execution_space = typename internal::ParticleExecutionSpace<ContainerType>::type;

      /// True when operations dispatch through Kokkos on `execution_space`.
      static constexpr bool kokkos_dispatch = internal::ParticleExecutionSpace<ContainerType>::kokkos_dispatch;

      /// Number of particles currently stored.
      static std::size_t size(const ContainerType &c) { return c.size(); }

      /// Append a particle (used when inserting received particles). Host-only.
      static void insert(ContainerType &c, const value_type &p) { c.push_back(p); }
      static void insert(ContainerType &c, value_type &&p) { c.push_back(std::move(p)); }

      /// Read-only visitation of every particle. Host-only.
      template<class F>
      static void forEach(const ContainerType &c, F &&f) {
        for (const auto &p : c) {
          f(p);
        }
      }

      /// True when `pred(p)` holds for every particle. Device-resident containers require a device-callable `pred`.
      template<class Predicate>
      static bool allOf(const ContainerType &c, Predicate pred) {
        if constexpr (kokkos_dispatch) {
          return internal::deviceAllOf<execution_space>(c, pred);
        } else {
          for (const auto &p : c) {
            if (!pred(p)) {
              return false;
            }
          }
          return true;
        }
      }

      /**
       * Count the particles each removal bucket would take, without modifying
       * the container.
       *
       * `classify(p)` returns `0` to keep a particle or `1 .. numBuckets` to
       * remove it into `counts[bucket - 1]`. The caller uses the counts to size
       * the byte buffers passed to `partitionAndPack()`.
       */
      template<class Classifier>
      static void countBuckets(
          const ContainerType &c, const Classifier &classify, std::size_t *counts, std::size_t numBuckets
      ) {
        if constexpr (kokkos_dispatch) {
          internal::deviceCountBuckets<execution_space>(c, classify, counts, numBuckets);
        } else {
          std::fill(counts, counts + numBuckets, std::size_t{0});
          for (const auto &p : c) {
            const int bkt = classify(p);
            if (bkt != 0) {
              SCHNEK_ASSERT(
                  bkt >= 1 && static_cast<std::size_t>(bkt) <= numBuckets,
                  "particle bucket " << bkt << " outside 0.." << numBuckets
              );
              ++counts[static_cast<std::size_t>(bkt) - 1];
            }
          }
        }
      }

      /// Convenience forwarder for a compile-time bucket count.
      template<class Classifier, std::size_t NumBuckets>
      static void countBuckets(
          const ContainerType &c, const Classifier &classify, std::array<std::size_t, NumBuckets> &counts
      ) {
        countBuckets(c, classify, counts.data(), NumBuckets);
      }

      /**
       * Number of scratch bytes `partitionAndPack()` needs in the buffer's memory space.
       * Only device-resident containers use scratch; host containers need none.
       */
      static std::size_t partitionScratchBytes([[maybe_unused]] const ContainerType &c) {
        if constexpr (kokkos_dispatch) {
          // Worst-case padding for the two alignment rounds in devicePartitionAndPack().
          return c.size() * (sizeof(value_type) + sizeof(std::size_t)) + alignof(value_type) + alignof(std::size_t);
        } else {
          return 0;
        }
      }

      /**
       * Partition the container in place, serialising departing particles into
       * caller-provided byte buffers, one per removal bucket.
       *
       * `outPtrs[b]` must hold `countBuckets()[b] * Serializer::size()` bytes in
       * the buffer's memory space. Survivors are compacted in place, preserving
       * their relative order. The container is unchanged since `countBuckets()`,
       * so classification is repeated here and yields the same buckets.
       *
       * A device-resident container runs the classify, exclusive-scan and scatter
       * passes in parallel on the device. `scratch` must hold `partitionScratchBytes(c)`
       * bytes in the buffer's memory space; host containers ignore it.
       *
       * @tparam Serializer  the particle serialiser (device-callable)
       * @tparam Classifier  the device-callable particle classifier (deduced)
       */
      template<class Serializer, class Classifier>
      static void partitionAndPack(
          ContainerType &c,
          const Classifier &classify,
          std::byte *const *outPtrs,
          std::size_t numBuckets,
          [[maybe_unused]] std::byte *scratch
      ) {
        if constexpr (kokkos_dispatch) {
          internal::devicePartitionAndPack<execution_space, Serializer>(c, classify, outPtrs, numBuckets, scratch);
        } else {
          constexpr std::size_t particleBytes = Serializer::size();
          const std::size_t n = c.size();

          std::vector<std::size_t> offset(numBuckets, 0);
          std::size_t out = 0;
          for (std::size_t i = 0; i < n; ++i) {
            const int bkt = classify(c[i]);
            if (bkt == 0) {
              if (out != i) {
                c[out] = std::move(c[i]);
              }
              ++out;
            } else {
              SCHNEK_ASSERT(
                  bkt >= 1 && static_cast<std::size_t>(bkt) <= numBuckets,
                  "particle bucket " << bkt << " outside 0.." << numBuckets
              );
              const std::size_t b = static_cast<std::size_t>(bkt) - 1;
              Serializer::serialize(c[i], outPtrs[b] + offset[b] * particleBytes);
              ++offset[b];
            }
          }
          c.resize(out);
        }
      }

      /// Convenience forwarder for a compile-time bucket count.
      template<class Serializer, class Classifier, std::size_t NumBuckets>
      static void partitionAndPack(
          ContainerType &c,
          const Classifier &classify,
          const std::array<std::byte *, NumBuckets> &outPtrs,
          std::byte *scratch
      ) {
        partitionAndPack<Serializer>(c, classify, outPtrs.data(), NumBuckets, scratch);
      }

      /**
       * Deserialise `count` particles from the byte buffer `src` (`Serializer::size()` bytes each) and append them to
       * `c`. `src` must lie in `ParticleBufferMemorySpace<ContainerType>`.
       *
       * Host containers deserialise serially. Device-resident containers deserialise in parallel on the device, so
       * `src` is never read on the host.
       */
      template<class Serializer>
      static void bulkInsert(ContainerType &c, const std::byte *src, std::size_t count) {
        if constexpr (kokkos_dispatch) {
          internal::deviceBulkInsert<execution_space, Serializer>(c, src, count);
        } else {
          for (std::size_t i = 0; i < count; ++i) {
            c.push_back(Serializer::deserialize(src + i * Serializer::size()));
          }
        }
      }
  };

  /**
   * Host specialisation for `BlockParticleContainer`. Only occupied slots are visited. Departing particles are
   * serialised as they are removed by `extractIf()`, which also returns emptied blocks to the pool. Survivors are not
   * kept in their original order, which the contract permits. No block-sized or container-sized temporary is needed.
   */
  template<class Particle, class BlockGrid>
  struct ParticleContainerTraits<BlockParticleContainer<Particle, BlockGrid>> {
      using ContainerType = BlockParticleContainer<Particle, BlockGrid>;
      using value_type = Particle;

      /// Blocks are host-resident, so operations loop on the host.
      using execution_space = void;
      static constexpr bool kokkos_dispatch = false;

      static std::size_t size(const ContainerType &c) { return c.size(); }

      static void insert(ContainerType &c, const value_type &p) { c.push_back(p); }
      static void insert(ContainerType &c, value_type &&p) { c.push_back(std::move(p)); }

      template<class F>
      static void forEach(const ContainerType &c, F &&f) {
        c.forEach(std::forward<F>(f));
      }

      template<class Predicate>
      static bool allOf(const ContainerType &c, Predicate pred) {
        for (std::size_t b = 0; b < c.activeBlockCount(); ++b) {
          const BlockGrid &data = c.block(b);
          const std::size_t used = c.blockUsed(b);
          for (std::size_t i = 0; i < used; ++i) {
            if (!pred(data[static_cast<std::ptrdiff_t>(i)])) {
              return false;
            }
          }
        }
        return true;
      }

      template<class Classifier>
      static void countBuckets(
          const ContainerType &c, const Classifier &classify, std::size_t *counts, std::size_t numBuckets
      ) {
        std::fill(counts, counts + numBuckets, std::size_t{0});
        c.forEachBlock([&](const BlockGrid &data, std::size_t used) {
          for (std::size_t i = 0; i < used; ++i) {
            const int bkt = classify(data[static_cast<std::ptrdiff_t>(i)]);
            if (bkt != 0) {
              SCHNEK_ASSERT(
                  bkt >= 1 && static_cast<std::size_t>(bkt) <= numBuckets,
                  "particle bucket " << bkt << " outside 0.." << numBuckets
              );
              ++counts[static_cast<std::size_t>(bkt) - 1];
            }
          }
        });
      }

      template<class Classifier, std::size_t NumBuckets>
      static void countBuckets(
          const ContainerType &c, const Classifier &classify, std::array<std::size_t, NumBuckets> &counts
      ) {
        countBuckets(c, classify, counts.data(), NumBuckets);
      }

      /// Blocks pack directly into the caller's buffers, so no scratch is needed.
      static std::size_t partitionScratchBytes([[maybe_unused]] const ContainerType &c) { return 0; }

      template<class Serializer, class Classifier>
      static void partitionAndPack(
          ContainerType &c,
          const Classifier &classify,
          std::byte *const *outPtrs,
          std::size_t numBuckets,
          [[maybe_unused]] std::byte *scratch
      ) {
        constexpr std::size_t particleBytes = Serializer::size();
        std::vector<std::size_t> offset(numBuckets, 0);
        int bkt = 0;
        c.extractIf(
            [&](const value_type &p) {
              bkt = classify(p);
              SCHNEK_ASSERT(
                  bkt == 0 || (bkt >= 1 && static_cast<std::size_t>(bkt) <= numBuckets),
                  "particle bucket " << bkt << " outside 0.." << numBuckets
              );
              return bkt != 0;
            },
            [&](value_type &&p) {
              // extractIf calls the sink directly after the predicate accepted this same particle, so `bkt` is current.
              const std::size_t b = static_cast<std::size_t>(bkt) - 1;
              Serializer::serialize(p, outPtrs[b] + offset[b] * particleBytes);
              ++offset[b];
            }
        );
      }

      template<class Serializer, class Classifier, std::size_t NumBuckets>
      static void partitionAndPack(
          ContainerType &c,
          const Classifier &classify,
          const std::array<std::byte *, NumBuckets> &outPtrs,
          std::byte *scratch
      ) {
        partitionAndPack<Serializer>(c, classify, outPtrs.data(), NumBuckets, scratch);
      }

      template<class Serializer>
      static void bulkInsert(ContainerType &c, const std::byte *src, std::size_t count) {
        // One capacity decision for the whole batch: free slots are counted, and only the missing blocks are added.
        c.reserveParticles(c.size() + count);
        for (std::size_t i = 0; i < count; ++i) {
          c.push_back(Serializer::deserialize(src + i * Serializer::size()));
        }
      }
  };

  /**
   * @brief Fixed-size byte serialisation of a particle for MPI transport.
   *
   * The primary template is intentionally left undefined: a particle is only
   * serialisable if it is either trivially copyable (handled by the partial
   * specialisation below) or the user provides a full specialisation of this
   * trait.
   *
   * A serializer must provide:
   * - `static constexpr std::size_t size();` — the fixed wire size in bytes,
   * - `static void serialize(const Particle &, std::byte *dst);`
   * - `static Particle deserialize(const std::byte *src);`
   *
   * @tparam Particle the particle value type
   */
  template<class Particle, class Enable = void>
  struct ParticleSerializer;

  /// Default serializer for trivially-copyable particles (a plain byte copy).
  template<class Particle>
  struct ParticleSerializer<Particle, std::enable_if_t<std::is_trivially_copyable_v<Particle>>> {
      static constexpr std::size_t size() { return sizeof(Particle); }

      SCHNEK_FUNCTION static void serialize(const Particle &p, std::byte *dst) {
        const std::byte *src = reinterpret_cast<const std::byte *>(&p);
        for (std::size_t i = 0; i < sizeof(Particle); ++i) {
          dst[i] = src[i];
        }
      }

      SCHNEK_FUNCTION static Particle deserialize(const std::byte *src) {
        Particle p;
        std::byte *dst = reinterpret_cast<std::byte *>(&p);
        for (std::size_t i = 0; i < sizeof(Particle); ++i) {
          dst[i] = src[i];
        }
        return p;
      }
  };

  /**
   * @brief Compile-time predicate: does a usable `ParticleSerializer<Particle>`
   * exist?
   *
   * True when the particle is trivially copyable or a user specialisation is in
   * scope. Used to produce a one-line diagnostic at registration time rather
   * than a deep template backtrace.
   */
  template<class Particle, class = void>
  struct has_particle_serializer : std::false_type {};

  template<class Particle>
  struct has_particle_serializer<Particle, std::void_t<decltype(ParticleSerializer<Particle>::size())>>
      : std::true_type {};

  /**
   * @brief Convenience position accessor reading a particle member.
   *
   * For the common case where the particle already stores its grid position as
   * a member (e.g. `Array<double, rank> position`), use
   * `MemberPositionAccessor<&Particle::position>{}`.
   *
   * @tparam MemberPtr pointer to the position member
   */
  template<auto MemberPtr>
  struct MemberPositionAccessor {
      template<class Particle>
      SCHNEK_FUNCTION auto operator()(const Particle &p) const -> decltype(p.*MemberPtr) {
        return p.*MemberPtr;
      }
  };

  namespace internal {

    /**
     * @brief Type-erased handle to a particle container plus its position
     * accessor.
     *
     * Mirrors `GridWrapper`. The single virtual `accept` defers to a cached
     * dispatcher in the visitor; everything below it is statically typed.
     */
    struct ParticleWrapper {
        virtual ~ParticleWrapper() = default;
        virtual void accept(ParticleVisitorBase &visitor) = 0;
    };

    using pParticleWrapper = std::shared_ptr<ParticleWrapper>;

    /**
     * @brief Concrete particle wrapper.
     *
     * Both `ContainerType` and `PositionAccessor` are template parameters, so
     * the bounds test and position read inline with no indirection. The
     * `std::type_index` of this type uniquely identifies the
     * `(container, accessor)` pair for the visitor dispatch.
     */
    template<class ContainerType, class PositionAccessor>
    struct ParticleWrapperImpl : ParticleWrapper {
        ParticleWrapperImpl(ContainerType containerIn, PositionAccessor accessorIn)
            : container(std::move(containerIn)), accessor(std::move(accessorIn)) {}

        void accept(ParticleVisitorBase &visitor) override { visitor.visit(typeid(ParticleWrapperImpl), this); }

        ContainerType container;
        PositionAccessor accessor;
    };

    /**
     * @brief Polymorphic factory interface used by the decomposition to create
     * one container per local range.
     *
     * Mirrors `GridRegistrationInterface`.
     */
    template<size_t rank, template<size_t> class CheckingPolicy = ArrayNoArgCheck>
    struct ParticleRegistrationInterface : public Unique<ParticleRegistrationInterface<rank, CheckingPolicy>> {
        using RangeType = Range<ptrdiff_t, rank, CheckingPolicy>;
        using DomainType = Range<double, rank, CheckingPolicy>;

        virtual ~ParticleRegistrationInterface() = default;
        virtual pParticleWrapper makeContainer(const RangeType &range, const DomainType &domain) = 0;
    };

    template<size_t rank, template<size_t> class CheckingPolicy = ArrayNoArgCheck>
    using pParticleRegistrationInterface = std::shared_ptr<ParticleRegistrationInterface<rank, CheckingPolicy>>;

  }  // namespace internal

  /**
   * @brief Factory teaching the decomposition how to *create* a particle
   * container for a local range, without knowing what a particle is.
   *
   * Analogous to `GridFactory`. The default factory value-initialises the
   * container and ignores the range/domain; users needing a capacity hint or a
   * custom allocator supply their own factory.
   *
   * @tparam ContainerType the user's particle storage type
   */
  template<class ContainerType>
  class ParticleContainerFactory {
    public:
      using Container = ContainerType;
      using Particle = typename ParticleContainerTraits<ContainerType>::value_type;

      ParticleContainerFactory() = default;

      template<typename RangeType, typename DomainType>
      ContainerType makeContainer(const RangeType &range, const DomainType &domain) const {
        (void)range;
        (void)domain;
        return ContainerType{};
      }
  };

  /**
   * @brief Factory for `BlockParticleContainer` with a configurable block size.
   *
   * Has no default constructor, so every registration states its block capacity. Each registration keeps its own copy,
   * so different registrations may use different block sizes.
   */
  template<class Particle, class BlockGrid>
  class ParticleContainerFactory<BlockParticleContainer<Particle, BlockGrid>> {
    public:
      using Container = BlockParticleContainer<Particle, BlockGrid>;

      /**
       * @param blockCapacity number of particle slots per block, must be positive
       * @param initialParticles number of particle slots to reserve in each new container
       */
      explicit ParticleContainerFactory(std::size_t blockCapacity, std::size_t initialParticles = 0)
          : blockCapacity_(blockCapacity), initialParticles_(initialParticles) {}

      template<typename RangeType, typename DomainType>
      Container makeContainer(const RangeType &range, const DomainType &domain) const {
        (void)range;
        (void)domain;
        Container container(blockCapacity_);
        container.reserveParticles(initialParticles_);
        return container;
      }

    private:
      std::size_t blockCapacity_;
      std::size_t initialParticles_;
  };

  /**
   * @brief Opaque handle returned by `registerParticleData`.
   *
   * Carries only an id, consistent with `GridRegistration`.
   */
  struct ParticleRegistration {
      long id = -1;
  };

  namespace internal {

    /**
     * @brief Concrete registration that creates `ParticleWrapperImpl`s for the
     * registered container/accessor pair.
     *
     * Mirrors `GridRegistrationImpl`. Holds the factory and the position
     * accessor; `makeContainer` constructs a fresh container via the factory
     * and pairs it with a copy of the accessor in the wrapper.
     */
    template<size_t rank, template<size_t> class CheckingPolicy, class ContainerType, class PositionAccessor>
    struct ParticleRegistrationImpl : public ParticleRegistrationInterface<rank, CheckingPolicy> {
        using Base = ParticleRegistrationInterface<rank, CheckingPolicy>;
        using RangeType = typename Base::RangeType;
        using DomainType = typename Base::DomainType;
        using WrapperImpl = ParticleWrapperImpl<ContainerType, PositionAccessor>;

        ParticleRegistrationImpl(const ParticleContainerFactory<ContainerType> &factoryIn, PositionAccessor accessorIn)
            : factory(factoryIn), accessor(std::move(accessorIn)) {}

        pParticleWrapper makeContainer(const RangeType &range, const DomainType &domain) override {
          return std::make_shared<WrapperImpl>(factory.makeContainer(range, domain), accessor);
        }

        ParticleContainerFactory<ContainerType> factory;
        PositionAccessor accessor;
    };

  }  // namespace internal

}  // namespace schnek

#endif  // SCHNEK_DECOMPOSITION_DETAIL_PARTICLE_FACTORY_HPP_
