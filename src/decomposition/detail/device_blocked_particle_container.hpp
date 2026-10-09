/*
 * device_blocked_particle_container.hpp
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
#ifndef SCHNEK_DECOMPOSITION_DETAIL_DEVICE_BLOCKED_PARTICLE_CONTAINER_HPP_
#define SCHNEK_DECOMPOSITION_DETAIL_DEVICE_BLOCKED_PARTICLE_CONTAINER_HPP_

#include <Kokkos_Core.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "particle_factory.hpp"

namespace schnek {

  namespace internal {

    /**
     * Device-readable description of one block. `data` points at the block's storage, `used` counts its particles and
     * `begin` is the index of its first particle in the container's flat order (the exclusive prefix sum of `used`).
     */
    template<class Particle>
    struct DeviceBlockDescriptor {
        Particle *data;
        std::size_t used;
        std::size_t begin;
    };

    /**
     * Index of the block holding flat particle `index`: the last block whose `begin` is not past `index`. Empty blocks
     * share their `begin` with the following block, so the last match always holds `index`.
     */
    template<class Particle>
    KOKKOS_INLINE_FUNCTION std::size_t findDeviceBlock(
        const DeviceBlockDescriptor<Particle> *blocks, std::size_t blockCount, std::size_t index
    ) {
      std::size_t lo = 0;
      std::size_t hi = blockCount;
      while (lo < hi) {
        const std::size_t mid = lo + (hi - lo) / 2;
        if (blocks[mid].begin <= index) {
          lo = mid + 1;
        } else {
          hi = mid;
        }
      }
      return lo - 1;
    }

    /// Rounds `p` up to a multiple of `alignment`, which must be a power of two.
    inline std::byte *alignDeviceBlockScratch(std::byte *p, std::size_t alignment) {
      const auto addr = reinterpret_cast<std::uintptr_t>(p);
      return reinterpret_cast<std::byte *>((addr + alignment - 1) & ~(alignment - 1));
    }

  }  // namespace internal

  /**
   * @brief Particle container that stores particles in fixed-size blocks in a Kokkos memory space.
   *
   * Each block is a separate allocation of `blockCapacity()` slots, of which the first `blockUsed(b)` hold particles.
   * Blocks are added but never resized, so growing never moves stored particles. The host owns the block list and the
   * per-block counts. Before every device operation the host publishes a descriptor array, indexed by block, that the
   * kernels read.
   *
   * Device operations run as batched kernels over all particles, so the number of launches does not depend on the
   * number of blocks. Partitioning packs survivors front-first, so only the last used block can be partly filled
   * after it. Scratch memory is proportional to the particle count, as in `DenseKokkosParticleContainer`.
   *
   * Only the host knows the block layout. Device kernels never change `blockUsed()`; the host updates it after each
   * operation from the survivor count. Container objects are move-only.
   *
   * @tparam Particle trivially copyable particle type
   * @tparam ExecSpace execution space used by the Kokkos dispatch paths
   * @tparam MemSpace memory space that stores the particles; must be accessible from `ExecSpace`
   */
  template<
      class Particle,
      class ExecSpace = Kokkos::DefaultExecutionSpace,
      class MemSpace = typename ExecSpace::memory_space>
  class DeviceBlockedParticleContainer {
      static_assert(
          std::is_trivially_copyable_v<Particle>, "particles stored in a Kokkos view must be trivially copyable"
      );
      static_assert(
          Kokkos::SpaceAccessibility<ExecSpace, MemSpace>::accessible,
          "execution space cannot access the particle memory space"
      );

      using BlockView = Kokkos::View<Particle *, MemSpace>;
      using BlockDescriptor = internal::DeviceBlockDescriptor<Particle>;
      using DescriptorView = Kokkos::View<BlockDescriptor *, MemSpace>;
      using HostDescriptorView = Kokkos::View<BlockDescriptor *, Kokkos::HostSpace, Kokkos::MemoryUnmanaged>;

    public:
      using value_type = Particle;
      using ExecutionSpace = ExecSpace;
      using MemorySpace = MemSpace;
      static constexpr bool host_accessible = Kokkos::SpaceAccessibility<Kokkos::HostSpace, MemSpace>::accessible;

      /// @throws ScheckException if `blockCapacity` is zero.
      explicit DeviceBlockedParticleContainer(std::size_t blockCapacity) : blockCapacity_(blockCapacity) {
        SCHNEK_ASSERT(blockCapacity > 0, "DeviceBlockedParticleContainer block capacity must be positive");
      }

      DeviceBlockedParticleContainer(const DeviceBlockedParticleContainer &) = delete;
      DeviceBlockedParticleContainer &operator=(const DeviceBlockedParticleContainer &) = delete;

      DeviceBlockedParticleContainer(DeviceBlockedParticleContainer &&other) noexcept
          : blockCapacity_(other.blockCapacity_),
            blocks_(std::exchange(other.blocks_, {})),
            used_(std::exchange(other.used_, {})),
            size_(std::exchange(other.size_, 0)),
            descriptors_(std::exchange(other.descriptors_, DescriptorView{})),
            descriptorsDirty_(std::exchange(other.descriptorsDirty_, true)) {}

      DeviceBlockedParticleContainer &operator=(DeviceBlockedParticleContainer &&other) noexcept {
        DeviceBlockedParticleContainer moved(std::move(other));
        swap(moved);
        return *this;
      }

      ~DeviceBlockedParticleContainer() = default;

      /// Number of stored particles.
      std::size_t size() const noexcept { return size_; }

      bool empty() const noexcept { return size_ == 0; }

      /// Number of particle slots in each block.
      std::size_t blockCapacity() const noexcept { return blockCapacity_; }

      /// Number of blocks allocated, including empty ones.
      std::size_t blockCount() const noexcept { return blocks_.size(); }

      /// Number of particle slots allocated across all blocks.
      std::size_t capacity() const noexcept { return blocks_.size() * blockCapacity_; }

      /// Number of particles stored in block `b`.
      std::size_t blockUsed(std::size_t b) const { return used_.at(b); }

      /// Ensure the blocks hold at least `n` particles in total. Adds whole blocks and never moves stored particles.
      void reserve(std::size_t n) {
        const std::size_t needed = (n + blockCapacity_ - 1) / blockCapacity_;
        if (blocks_.size() < needed) {
          addBlocks(needed - blocks_.size());
        }
      }

      /// Append one particle. Copies from host memory, so it works for every space.
      void push_back(const Particle &value) {
        if (blocks_.empty() || used_.back() == blockCapacity_) {
          addBlocks(1);
        }
        using HostView = Kokkos::View<const Particle *, Kokkos::HostSpace, Kokkos::MemoryUnmanaged>;
        const std::size_t target = blocks_.size() - 1;
        const std::size_t slot = used_[target];
        Kokkos::deep_copy(
            Kokkos::subview(blocks_[target], Kokkos::pair<std::size_t, std::size_t>(slot, slot + 1)),
            HostView(&value, 1)
        );
        ++used_[target];
        ++size_;
        descriptorsDirty_ = true;
      }

      /// Visit every stored particle in block order. Host-only.
      template<class F>
      void forEach(F &&f) const {
        static_assert(host_accessible, "element access requires host-accessible particle memory");
        for (std::size_t b = 0; b < blocks_.size(); ++b) {
          const BlockView &block = blocks_[b];
          for (std::size_t i = 0; i < used_[b]; ++i) {
            f(block(i));
          }
        }
      }

      /// Copy the stored particles into a host vector, in block order.
      std::vector<Particle> copyToHost() const {
        std::vector<Particle> result(size_);
        if (size_ == 0) {
          return result;
        }
        syncDescriptors();
        BlockView flat(Kokkos::view_alloc(Kokkos::WithoutInitializing, "schnek:gather-particles"), size_);
        const BlockDescriptor *blocks = descriptors_.data();
        const std::size_t blockCount = blocks_.size();
        Particle *out = flat.data();
        Kokkos::parallel_for(
            "schnek:gatherDeviceBlocks", Kokkos::RangePolicy<ExecSpace>(std::size_t{0}, size_),
            KOKKOS_LAMBDA(const std::size_t i) {
              const std::size_t b = internal::findDeviceBlock(blocks, blockCount, i);
              out[i] = blocks[b].data[i - blocks[b].begin];
            }
        );
        Kokkos::fence();
        Kokkos::View<Particle *, Kokkos::HostSpace, Kokkos::MemoryUnmanaged> host(result.data(), size_);
        Kokkos::deep_copy(host, flat);
        return result;
      }

      /// True when `pred(p)` holds for every stored particle. `pred` must be device-callable.
      template<class Predicate>
      bool allOf(Predicate pred) const {
        syncDescriptors();
        bool result = true;
        const BlockDescriptor *blocks = descriptors_.data();
        const std::size_t blockCount = blocks_.size();
        Kokkos::parallel_reduce(
            "schnek:allOfDeviceBlocks", Kokkos::RangePolicy<ExecSpace>(std::size_t{0}, size_),
            KOKKOS_LAMBDA(const std::size_t i, bool &ok) {
              const std::size_t b = internal::findDeviceBlock(blocks, blockCount, i);
              ok = ok && pred(blocks[b].data[i - blocks[b].begin]);
            },
            Kokkos::LAnd<bool>(result)
        );
        return result;
      }

      /**
       * Run `f(particle)` on every stored particle in parallel on the device. `f` must be device-callable and may
       * modify the particle in place, but it must not change the number of particles.
       */
      template<class F>
      void parallelForEach(F f) {
        if (size_ == 0) {
          return;
        }
        syncDescriptors();
        const BlockDescriptor *blocks = descriptors_.data();
        const std::size_t blockCount = blocks_.size();
        Kokkos::parallel_for(
            "schnek:forEachDeviceBlocks", Kokkos::RangePolicy<ExecSpace>(std::size_t{0}, size_),
            KOKKOS_LAMBDA(const std::size_t i) {
              const std::size_t b = internal::findDeviceBlock(blocks, blockCount, i);
              f(blocks[b].data[i - blocks[b].begin]);
            }
        );
        Kokkos::fence();
      }

      /// Count the particles each removal bucket takes, without modifying the container. See `ParticleContainerTraits`.
      template<class Classifier>
      void countBuckets(const Classifier &classify, std::size_t *counts, std::size_t numBuckets) const {
        syncDescriptors();
        const BlockDescriptor *blocks = descriptors_.data();
        const std::size_t blockCount = blocks_.size();
        const std::size_t n = size_;
        for (std::size_t b = 0; b < numBuckets; ++b) {
          const int bucket = static_cast<int>(b) + 1;
          std::size_t total = 0;
          Kokkos::parallel_reduce(
              "schnek:countDeviceBlockBucket", Kokkos::RangePolicy<ExecSpace>(std::size_t{0}, n),
              KOKKOS_LAMBDA(const std::size_t i, std::size_t &acc) {
                const std::size_t blk = internal::findDeviceBlock(blocks, blockCount, i);
                if (classify(blocks[blk].data[i - blocks[blk].begin]) == bucket) {
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
            "schnek:checkDeviceBlockBuckets", Kokkos::RangePolicy<ExecSpace>(std::size_t{0}, n),
            KOKKOS_LAMBDA(const std::size_t i, std::size_t &acc) {
              const std::size_t blk = internal::findDeviceBlock(blocks, blockCount, i);
              const int bucket = classify(blocks[blk].data[i - blocks[blk].begin]);
              if (bucket < 0 || bucket > static_cast<int>(numBuckets)) {
                ++acc;
              }
            },
            invalid
        );
        SCHNEK_ASSERT(invalid == 0, "particle classified outside buckets 0.." << numBuckets);
#endif
      }

      /// Convenience forwarder for a compile-time bucket count.
      template<class Classifier, std::size_t NumBuckets>
      void countBuckets(const Classifier &classify, std::array<std::size_t, NumBuckets> &counts) const {
        countBuckets(classify, counts.data(), NumBuckets);
      }

      /// Bytes of scratch `partitionAndPack()` needs for the current size.
      std::size_t partitionScratchBytes() const {
        // One survivor slot and one class byte per particle, plus worst-case padding for the survivor alignment.
        return size_ * (sizeof(Particle) + sizeof(std::uint8_t)) + alignof(Particle);
      }

      /**
       * Remove the particles classified into buckets `1..numBuckets`, serialising each bucket into `outPtrs[b - 1]`,
       * and pack the survivors front-first into the blocks. Particles are classified once into a byte array. Each
       * target then runs one scan that writes its members directly, so the launch count is independent of the number
       * of blocks.
       */
      template<class Serializer, class Classifier>
      void partitionAndPack(
          const Classifier &classify, std::byte *const *outPtrs, std::size_t numBuckets, std::byte *scratch
      ) {
        SCHNEK_ASSERT(numBuckets <= 255, "DeviceBlockedParticleContainer supports at most 255 buckets");
        constexpr std::size_t particleBytes = Serializer::size();
        const std::size_t n = size_;
        if (n == 0) {
          return;
        }
        syncDescriptors();
        const BlockDescriptor *blocks = descriptors_.data();
        const std::size_t blockCount = blocks_.size();
        const std::size_t cap = blockCapacity_;

        Particle *survivors =
            reinterpret_cast<Particle *>(internal::alignDeviceBlockScratch(scratch, alignof(Particle)));
        std::uint8_t *cls = reinterpret_cast<std::uint8_t *>(survivors + n);

        Kokkos::parallel_for(
            "schnek:classifyDeviceBlocks", Kokkos::RangePolicy<ExecSpace>(std::size_t{0}, n),
            KOKKOS_LAMBDA(const std::size_t i) {
              const std::size_t blk = internal::findDeviceBlock(blocks, blockCount, i);
              cls[i] = static_cast<std::uint8_t>(classify(blocks[blk].data[i - blocks[blk].begin]));
            }
        );

        // Target 0 gathers survivors; target b (1-based) packs removal bucket b.
        std::size_t survivorCount = 0;
        for (int target = 0; target <= static_cast<int>(numBuckets); ++target) {
          std::size_t total = 0;
          std::byte *dst = target == 0 ? nullptr : outPtrs[static_cast<std::size_t>(target) - 1];
          Kokkos::parallel_scan(
              "schnek:scanDeviceBlockTarget", Kokkos::RangePolicy<ExecSpace>(std::size_t{0}, n),
              KOKKOS_LAMBDA(const std::size_t i, std::size_t &update, const bool isFinal) {
                const bool member = static_cast<int>(cls[i]) == target;
                if (isFinal && member) {
                  const std::size_t blk = internal::findDeviceBlock(blocks, blockCount, i);
                  const Particle &p = blocks[blk].data[i - blocks[blk].begin];
                  if (target == 0) {
                    survivors[update] = p;
                  } else {
                    Serializer::serialize(p, dst + update * particleBytes);
                  }
                }
                if (member) {
                  ++update;
                }
              },
              total
          );
          if (target == 0) {
            survivorCount = total;
          }
        }

        // Survivor k goes to slot k % cap of block k / cap. Survivors never outnumber the slots, so the block exists.
        Kokkos::parallel_for(
            "schnek:writeBackDeviceSurvivors", Kokkos::RangePolicy<ExecSpace>(std::size_t{0}, survivorCount),
            KOKKOS_LAMBDA(const std::size_t k) { blocks[k / cap].data[k % cap] = survivors[k]; }
        );
        Kokkos::fence();

        std::size_t remaining = survivorCount;
        for (auto &u : used_) {
          u = std::min(remaining, blockCapacity_);
          remaining -= u;
        }
        size_ = survivorCount;
        descriptorsDirty_ = true;
      }

      /// Convenience forwarder for a compile-time bucket count.
      template<class Serializer, class Classifier, std::size_t NumBuckets>
      void partitionAndPack(
          const Classifier &classify, const std::array<std::byte *, NumBuckets> &outPtrs, std::byte *scratch
      ) {
        partitionAndPack<Serializer>(classify, outPtrs.data(), NumBuckets, scratch);
      }

      /**
       * Append `count` particles read from `src`, which must lie in `MemSpace`. The free slots of the last block are
       * filled first and then new blocks are added, so the incoming particles follow the existing ones in buffer order.
       */
      template<class Serializer>
      void bulkInsert(const std::byte *src, std::size_t count) {
        if (count == 0) {
          return;
        }
        const std::size_t first = blocks_.empty() ? 0 : blocks_.size() - 1;
        const std::size_t tailFree = blocks_.empty() ? 0 : blockCapacity_ - used_.back();
        if (count > tailFree) {
          addBlocks((count - tailFree + blockCapacity_ - 1) / blockCapacity_);
        }

        // Each entry is one block that receives particles: `used` is its first free slot and `begin` the index of the
        // first incoming particle it takes.
        std::vector<BlockDescriptor> insert;
        std::size_t placed = 0;
        for (std::size_t b = first; b < blocks_.size() && placed < count; ++b) {
          const std::size_t take = std::min(blockCapacity_ - used_[b], count - placed);
          if (take == 0) {
            continue;
          }
          insert.push_back(BlockDescriptor{blocks_[b].data(), used_[b], placed});
          placed += take;
          used_[b] += take;
        }

        DescriptorView table(
            Kokkos::view_alloc(Kokkos::WithoutInitializing, "schnek:insert-descriptors"), insert.size()
        );
        Kokkos::deep_copy(table, HostDescriptorView(insert.data(), insert.size()));
        const BlockDescriptor *blocks = table.data();
        const std::size_t blockCount = insert.size();
        const std::size_t bytesPerParticle = Serializer::size();
        Kokkos::parallel_for(
            "schnek:bulkInsertDeviceBlocks", Kokkos::RangePolicy<ExecSpace>(std::size_t{0}, count),
            KOKKOS_LAMBDA(const std::size_t k) {
              const std::size_t e = internal::findDeviceBlock(blocks, blockCount, k);
              const BlockDescriptor &target = blocks[e];
              target.data[target.used + (k - target.begin)] = Serializer::deserialize(src + k * bytesPerParticle);
            }
        );
        Kokkos::fence();

        size_ += count;
        descriptorsDirty_ = true;
      }

      void swap(DeviceBlockedParticleContainer &other) noexcept {
        std::swap(blockCapacity_, other.blockCapacity_);
        std::swap(blocks_, other.blocks_);
        std::swap(used_, other.used_);
        std::swap(size_, other.size_);
        std::swap(descriptors_, other.descriptors_);
        std::swap(descriptorsDirty_, other.descriptorsDirty_);
      }

    private:
      void addBlocks(std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
          blocks_.emplace_back(Kokkos::view_alloc(Kokkos::WithoutInitializing, "schnek:device-block"), blockCapacity_);
          used_.push_back(0);
        }
        descriptorsDirty_ = true;
      }

      /// Publish the block table to the device if the host state has changed since the last publication.
      void syncDescriptors() const {
        if (!descriptorsDirty_) {
          return;
        }
        std::vector<BlockDescriptor> host(blocks_.size());
        std::size_t begin = 0;
        for (std::size_t b = 0; b < blocks_.size(); ++b) {
          host[b] = BlockDescriptor{blocks_[b].data(), used_[b], begin};
          begin += used_[b];
        }
        descriptors_ =
            DescriptorView(Kokkos::view_alloc(Kokkos::WithoutInitializing, "schnek:block-descriptors"), host.size());
        if (!host.empty()) {
          Kokkos::deep_copy(descriptors_, HostDescriptorView(host.data(), host.size()));
        }
        descriptorsDirty_ = false;
      }

      std::size_t blockCapacity_;
      std::vector<BlockView> blocks_;
      std::vector<std::size_t> used_;
      std::size_t size_ = 0;
      mutable DescriptorView descriptors_{};
      mutable bool descriptorsDirty_ = true;
  };

  /**
   * Adapts `DeviceBlockedParticleContainer` to the particle operations the domain decomposition needs. Every operation
   * runs on the device, so the particle values are never read on the host.
   */
  template<class Particle, class ExecSpace, class MemSpace>
  struct ParticleContainerTraits<DeviceBlockedParticleContainer<Particle, ExecSpace, MemSpace>> {
      using ContainerType = DeviceBlockedParticleContainer<Particle, ExecSpace, MemSpace>;
      using value_type = Particle;
      using execution_space = ExecSpace;
      static constexpr bool kokkos_dispatch = true;

      static std::size_t size(const ContainerType &c) { return c.size(); }

      static void insert(ContainerType &c, const value_type &p) { c.push_back(p); }

      template<class F>
      static void forEach(const ContainerType &c, F &&f) {
        c.forEach(std::forward<F>(f));
      }

      template<class Predicate>
      static bool allOf(const ContainerType &c, Predicate pred) {
        return c.allOf(pred);
      }

      template<class Classifier>
      static void countBuckets(
          const ContainerType &c, const Classifier &classify, std::size_t *counts, std::size_t numBuckets
      ) {
        c.countBuckets(classify, counts, numBuckets);
      }

      template<class Classifier, std::size_t NumBuckets>
      static void countBuckets(
          const ContainerType &c, const Classifier &classify, std::array<std::size_t, NumBuckets> &counts
      ) {
        c.countBuckets(classify, counts);
      }

      static std::size_t partitionScratchBytes(const ContainerType &c) { return c.partitionScratchBytes(); }

      template<class Serializer, class Classifier>
      static void partitionAndPack(
          ContainerType &c,
          const Classifier &classify,
          std::byte *const *outPtrs,
          std::size_t numBuckets,
          std::byte *scratch
      ) {
        c.template partitionAndPack<Serializer>(classify, outPtrs, numBuckets, scratch);
      }

      template<class Serializer, class Classifier, std::size_t NumBuckets>
      static void partitionAndPack(
          ContainerType &c,
          const Classifier &classify,
          const std::array<std::byte *, NumBuckets> &outPtrs,
          std::byte *scratch
      ) {
        c.template partitionAndPack<Serializer>(classify, outPtrs, scratch);
      }

      template<class Serializer>
      static void bulkInsert(ContainerType &c, const std::byte *src, std::size_t count) {
        c.template bulkInsert<Serializer>(src, count);
      }
  };

  /**
   * @brief Factory for `DeviceBlockedParticleContainer` with a configurable block size.
   *
   * Has no default constructor, so every registration states its block capacity.
   */
  template<class Particle, class ExecSpace, class MemSpace>
  class ParticleContainerFactory<DeviceBlockedParticleContainer<Particle, ExecSpace, MemSpace>> {
    public:
      using Container = DeviceBlockedParticleContainer<Particle, ExecSpace, MemSpace>;

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
        container.reserve(initialParticles_);
        return container;
      }

    private:
      std::size_t blockCapacity_;
      std::size_t initialParticles_;
  };

}  // namespace schnek

#endif /* SCHNEK_DECOMPOSITION_DETAIL_DEVICE_BLOCKED_PARTICLE_CONTAINER_HPP_ */
