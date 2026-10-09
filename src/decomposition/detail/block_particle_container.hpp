/*
 * block_particle_container.hpp
 *
 * Created on: 30 Jun 2026
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
#ifndef SCHNEK_DECOMPOSITION_DETAIL_BLOCK_PARTICLE_CONTAINER_HPP_
#define SCHNEK_DECOMPOSITION_DETAIL_BLOCK_PARTICLE_CONTAINER_HPP_

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "../../grid/array.hpp"
#include "../../grid/grid.hpp"
#include "../../util/exceptions.hpp"

namespace schnek {

  /**
   * @brief Particle container that stores particles in fixed-capacity blocks.
   *
   * Each block is a 1-D grid of `blockCapacity()` slots, of which the first `used` slots
   * are active. Blocks are heap-allocated and owned through `std::unique_ptr`, so a block
   * is never relocated once created. Growing the container only appends blocks.
   *
   * Ownership:
   *  - Active blocks form an ordered sequence. Particles are iterated in this order.
   *  - Inactive blocks (after `clear()`) are kept in a free pool and reused before any
   *    new allocation. They are returned to the system by `releaseFreeBlocks()`.
   *  - Appends fill active blocks with free slots before taking blocks from the pool.
   *
   * Binning:
   *  - `rebin()` groups the particles into bins. Each bin owns a contiguous range of active
   *    blocks, so the bins appear in order in the active sequence.
   *  - `push_back()`, `emplace_back()`, `insert()` and `clear()` remove the binning, because
   *    unkeyed particles would break the bin-to-block mapping. `extractIf()` keeps it.
   *
   * Invariants:
   *  - `size()` equals the sum of `blockUsed(i)` over all active blocks.
   *  - Every active block has `1 <= blockUsed(i) <= blockCapacity()`.
   *  - Every block in the pool has `used == 0`.
   *
   * Copying is disabled because copies of a `Grid` share their storage. The container
   * is move-only. A moved-from container is empty and keeps its block capacity.
   *
   * @tparam Particle The particle type. Must be default constructible, because
   *                  `BlockGrid` default-initialises its elements.
   * @tparam BlockGrid A 1-D Schnek grid with `value_type == Particle`, constructible from
   *                   `BlockGrid::SizeType` and indexable by `std::ptrdiff_t`.
   */
  template<typename Particle, typename BlockGrid = Grid<Particle, 1>>
  class BlockParticleContainer {
      static_assert(BlockGrid::Rank == 1, "BlockParticleContainer requires a 1-D block grid");
      static_assert(
          std::is_same<typename BlockGrid::value_type, Particle>::value,
          "BlockGrid::value_type must match the particle type"
      );

    public:
      using value_type = Particle;
      using block_type = BlockGrid;

      /**
       * @brief Create an empty container with the given number of slots per block.
       *
       * @throws ScheckException if `blockCapacity` is zero.
       */
      explicit BlockParticleContainer(std::size_t blockCapacity) : blockCapacity_(blockCapacity) {
        SCHNEK_ASSERT(blockCapacity > 0, "BlockParticleContainer block capacity must be positive");
      }

      BlockParticleContainer(const BlockParticleContainer &) = delete;
      BlockParticleContainer &operator=(const BlockParticleContainer &) = delete;

      /// Move construction transfers ownership of all blocks. The source is left empty.
      BlockParticleContainer(BlockParticleContainer &&other) noexcept
          : blockCapacity_(other.blockCapacity_),
            blocks_(std::move(other.blocks_)),
            partial_(std::move(other.partial_)),
            pool_(std::move(other.pool_)),
            size_(std::exchange(other.size_, std::size_t{0})),
            binBegin_(std::move(other.binBegin_)) {
        other.binBegin_.clear();
      }

      /// Move assignment transfers ownership of all blocks. The source is left empty.
      BlockParticleContainer &operator=(BlockParticleContainer &&other) noexcept {
        if (this != &other) {
          blockCapacity_ = other.blockCapacity_;
          blocks_ = std::move(other.blocks_);
          partial_ = std::move(other.partial_);
          pool_ = std::move(other.pool_);
          size_ = std::exchange(other.size_, std::size_t{0});
          binBegin_ = std::move(other.binBegin_);
          other.blocks_.clear();
          other.partial_.clear();
          other.pool_.clear();
          other.binBegin_.clear();
        }
        return *this;
      }

      ~BlockParticleContainer() = default;

      /// Number of slots in each block.
      std::size_t blockCapacity() const { return blockCapacity_; }

      /// Number of active particles.
      std::size_t size() const { return size_; }

      /// True when there are no active particles.
      bool empty() const { return size_ == 0; }

      /// Total number of owned blocks, active and pooled.
      std::size_t blockCount() const { return blocks_.size() + pool_.size(); }

      /// Number of blocks in the active sequence.
      std::size_t activeBlockCount() const { return blocks_.size(); }

      /// Total number of slots across all owned blocks.
      std::size_t capacity() const { return blockCount() * blockCapacity_; }

      /// Number of active slots in the active block at position `index`.
      std::size_t blockUsed(std::size_t index) const {
        SCHNEK_ASSERT(index < blocks_.size(), "Active block index " << index << " out of range");
        return blocks_[index]->used;
      }

      /// The grid storing the active block at position `index`.
      BlockGrid &block(std::size_t index) {
        SCHNEK_ASSERT(index < blocks_.size(), "Active block index " << index << " out of range");
        return blocks_[index]->data;
      }

      /// The grid storing the active block at position `index`.
      const BlockGrid &block(std::size_t index) const {
        SCHNEK_ASSERT(index < blocks_.size(), "Active block index " << index << " out of range");
        return blocks_[index]->data;
      }

      /// Append a particle. Allocates a block only when no pooled block is available.
      void push_back(const Particle &particle) { append(particle); }

      /// Append a particle. Allocates a block only when no block has a free slot.
      void push_back(Particle &&particle) { append(std::move(particle)); }

      /// Construct a particle at the end of the sequence.
      template<typename... Args>
      void emplace_back(Args &&...args) {
        append(Particle(std::forward<Args>(args)...));
      }

      /**
       * @brief Append the particles in `[first, last)`.
       *
       * For forward iterators, all required blocks are reserved before copying.
       */
      template<typename InputIt>
      void insert(InputIt first, InputIt last) {
        using Category = typename std::iterator_traits<InputIt>::iterator_category;
        if constexpr (std::is_base_of<std::forward_iterator_tag, Category>::value) {
          reserveParticles(size_ + static_cast<std::size_t>(std::distance(first, last)));
        }
        for (; first != last; ++first) {
          append(*first);
        }
      }

      /**
       * @brief Call `visitor(const BlockGrid &, std::size_t used)` for each active block, in order.
       *
       * Only the first `used` slots of each block are active.
       */
      template<typename Visitor>
      void forEachBlock(Visitor &&visitor) const {
        for (const auto &block : blocks_) {
          visitor(static_cast<const BlockGrid &>(block->data), block->used);
        }
      }

      /// Call `visitor(BlockGrid &, std::size_t used)` for each active block, in order.
      template<typename Visitor>
      void forEachBlock(Visitor &&visitor) {
        for (auto &block : blocks_) {
          visitor(block->data, block->used);
        }
      }

      /// Call `visitor(const Particle &)` for each active particle, in block order.
      template<typename Visitor>
      void forEach(Visitor &&visitor) const {
        forEachBlock([&visitor](const BlockGrid &data, std::size_t used) {
          for (std::size_t i = 0; i < used; ++i) {
            visitor(data[static_cast<std::ptrdiff_t>(i)]);
          }
        });
      }

      /// Call `visitor(Particle &)` for each active particle, in block order.
      template<typename Visitor>
      void forEach(Visitor &&visitor) {
        forEachBlock([&visitor](BlockGrid &data, std::size_t used) {
          for (std::size_t i = 0; i < used; ++i) {
            visitor(data[static_cast<std::ptrdiff_t>(i)]);
          }
        });
      }

      /**
       * @brief Remove every particle for which `predicate(const Particle &)` is true.
       *
       * Each removed particle is passed to `sink(Particle &&)`. The sink is called immediately
       * after `predicate` returned true for that same particle, so state set by the predicate
       * is current in the sink. Within a block, the last active slot fills each gap, so
       * survivors are not ordered. Blocks left empty are moved to the pool. Other blocks keep
       * their addresses.
       *
       * @return The number of removed particles.
       */
      template<typename Predicate, typename Sink>
      std::size_t extractIf(Predicate &&predicate, Sink &&sink) {
        std::size_t removed = 0;
        for (auto &block : blocks_) {
          BlockGrid &data = block->data;
          std::size_t i = 0;
          while (i < block->used) {
            const auto slot = static_cast<std::ptrdiff_t>(i);
            if (!predicate(static_cast<const Particle &>(data[slot]))) {
              ++i;
              continue;
            }
            sink(std::move(data[slot]));
            const std::size_t last = block->used - 1;
            if (i != last) {
              data[slot] = std::move(data[static_cast<std::ptrdiff_t>(last)]);
            }
            block->used = last;
            ++removed;
          }
        }
        size_ -= removed;
        retireEmptyBlocks();
        if (isBinned()) {
          rebuildBinTable();
        }
        return removed;
      }

      /// Remove every particle for which `predicate(const Particle &)` is true. Returns the count.
      template<typename Predicate>
      std::size_t eraseIf(Predicate &&predicate) {
        return extractIf(std::forward<Predicate>(predicate), [](Particle &&) {});
      }

      /**
       * @brief Remove all particles. The blocks are kept in the pool for reuse.
       *
       * The number of owned blocks does not change. Existing block addresses stay valid.
       * The container is no longer binned.
       */
      void clear() {
        pool_.reserve(pool_.size() + blocks_.size());
        for (auto &block : blocks_) {
          block->used = 0;
          pool_.push_back(std::move(block));
        }
        blocks_.clear();
        partial_.clear();
        binBegin_.clear();
        size_ = 0;
      }

      /**
       * @brief Ensure at least `count` blocks are owned. New blocks are added to the pool.
       *
       * Does not change `size()` or the active sequence.
       */
      void reserveBlocks(std::size_t count) {
        const std::size_t current = blockCount();
        if (count <= current) {
          return;
        }
        pool_.reserve(pool_.size() + (count - current));
        while (blockCount() < count) {
          pool_.push_back(std::make_unique<Block>(blockCapacity_));
        }
      }

      /**
       * @brief Ensure `capacity()` is at least `count` particles.
       *
       * Rounds up to whole blocks. Does not change `size()` or the active sequence.
       */
      void reserveParticles(std::size_t count) {
        const std::size_t blocksNeeded = count / blockCapacity_ + (count % blockCapacity_ != 0 ? 1 : 0);
        reserveBlocks(blocksNeeded);
      }

      /**
       * @brief Free all pooled blocks. Active blocks and their particles are unchanged.
       */
      void releaseFreeBlocks() {
        pool_.clear();
        pool_.shrink_to_fit();
      }

      /// True when `rebin()` has organised the particles into bins.
      bool isBinned() const { return !binBegin_.empty(); }

      /// Number of bins set by the last `rebin()`, or zero when the container is not binned.
      std::size_t binCount() const { return binBegin_.empty() ? 0 : binBegin_.size() - 1; }

      /**
       * @brief The half-open range `[first, last)` of active block indices holding bin `bin`.
       *
       * Empty bins have `first == last`.
       *
       * @throws ScheckException if the container is not binned or `bin` is out of range.
       */
      std::pair<std::size_t, std::size_t> binBlockRange(std::size_t bin) const {
        SCHNEK_ASSERT(isBinned(), "BlockParticleContainer is not binned");
        SCHNEK_ASSERT(bin < binCount(), "Bin " << bin << " is out of range for " << binCount() << " bins");
        return {binBegin_[bin], binBegin_[bin + 1]};
      }

      /// Number of particles in bin `bin`.
      std::size_t binSize(std::size_t bin) const {
        const auto [first, last] = binBlockRange(bin);
        std::size_t count = 0;
        for (std::size_t b = first; b < last; ++b) {
          count += blocks_[b]->used;
        }
        return count;
      }

      /// Call `visitor(const Particle &)` for each particle in bin `bin`.
      template<typename Visitor>
      void forEachInBin(std::size_t bin, Visitor &&visitor) const {
        const auto [first, last] = binBlockRange(bin);
        for (std::size_t b = first; b < last; ++b) {
          const Block &block = *blocks_[b];
          for (std::size_t i = 0; i < block.used; ++i) {
            visitor(static_cast<const Particle &>(block.data[static_cast<std::ptrdiff_t>(i)]));
          }
        }
      }

      /// Call `visitor(Particle &)` for each particle in bin `bin`.
      template<typename Visitor>
      void forEachInBin(std::size_t bin, Visitor &&visitor) {
        const auto [first, last] = binBlockRange(bin);
        for (std::size_t b = first; b < last; ++b) {
          Block &block = *blocks_[b];
          for (std::size_t i = 0; i < block.used; ++i) {
            visitor(block.data[static_cast<std::ptrdiff_t>(i)]);
          }
        }
      }

      /**
       * @brief Reorder the particles into `numBins` bins. `binOf(const Particle &)` gives each bin.
       *
       * `binOf` must return a value less than `numBins`. Its result is evaluated once per particle.
       * Bin `b` owns the active blocks `[binBlockRange(b).first, binBlockRange(b).second)`. Its
       * particles fill `ceil(count / blockCapacity())` blocks, so only the last block of a bin can
       * be partial.
       *
       * Particles move block by block. A source block goes to the pool as soon as it is empty,
       * so it can serve as a target for later particles. Rebinning to the same layout reuses
       * the existing blocks and does not allocate new ones.
       *
       * `size()` does not change. If `binOf` throws or returns an out-of-range bin, no particle
       * is lost and the container is left unbinned.
       *
       * @throws ScheckException if `numBins` is zero or `binOf` returns a bin out of range.
       */
      template<typename BinOf>
      void rebin(std::size_t numBins, BinOf &&binOf) {
        SCHNEK_ASSERT(numBins > 0, "BlockParticleContainer requires at least one bin");
        std::vector<std::unique_ptr<Block>> source = std::move(blocks_);
        blocks_.clear();
        partial_.clear();
        binBegin_.clear();

        std::vector<std::vector<std::unique_ptr<Block>>> targets(numBins);
        std::size_t next = 0;
        try {
          for (; next < source.size(); ++next) {
            Block &from = *source[next];
            while (from.used > 0) {
              const auto slot = static_cast<std::ptrdiff_t>(from.used - 1);
              const std::size_t bin = binOf(static_cast<const Particle &>(from.data[slot]));
              SCHNEK_ASSERT(bin < numBins, "Bin " << bin << " is out of range for " << numBins << " bins");

              std::vector<std::unique_ptr<Block>> &bucket = targets[bin];
              if (bucket.empty() || bucket.back()->used == blockCapacity_) {
                bucket.push_back(acquireBlock());
                bucket.back()->bin = bin;
              }
              Block &to = *bucket.back();
              to.data[static_cast<std::ptrdiff_t>(to.used)] = std::move(from.data[slot]);
              ++to.used;
              --from.used;
            }
            pool_.push_back(std::move(source[next]));
          }
        } catch (...) {
          for (auto &bucket : targets) {
            for (auto &block : bucket) {
              blocks_.push_back(std::move(block));
            }
          }
          for (std::size_t k = next; k < source.size(); ++k) {
            if (source[k]) {
              blocks_.push_back(std::move(source[k]));
            }
          }
          rebuildPartial();
          throw;
        }

        binBegin_.assign(numBins + 1, std::size_t{0});
        for (std::size_t bin = 0; bin < numBins; ++bin) {
          binBegin_[bin] = blocks_.size();
          for (auto &block : targets[bin]) {
            blocks_.push_back(std::move(block));
          }
        }
        binBegin_[numBins] = blocks_.size();
        rebuildPartial();
      }

    private:
      /// A single fixed-capacity block of particles.
      struct Block {
          explicit Block(std::size_t capacity) : data(typename BlockGrid::SizeType(capacity)) {}

          BlockGrid data;
          std::size_t used = 0;
          /// Bin of the block. Only meaningful while the container is binned.
          std::size_t bin = 0;
      };

      template<typename P>
      void append(P &&particle) {
        binBegin_.clear();
        Block &target = *blocks_[openBlockIndex()];
        target.data[static_cast<std::ptrdiff_t>(target.used)] = std::forward<P>(particle);
        ++target.used;
        ++size_;
        if (target.used == blockCapacity_) {
          partial_.pop_back();
        }
      }

      /// Index of an active block with a free slot. Activates a block if none is partial.
      std::size_t openBlockIndex() {
        if (partial_.empty()) {
          blocks_.push_back(acquireBlock());
          partial_.push_back(blocks_.size() - 1);
        }
        return partial_.back();
      }

      /// Move empty active blocks to the pool and rebuild `partial_`. Keeps active order.
      void retireEmptyBlocks() {
        partial_.clear();
        std::size_t kept = 0;
        for (std::size_t i = 0; i < blocks_.size(); ++i) {
          if (blocks_[i]->used == 0) {
            pool_.push_back(std::move(blocks_[i]));
            continue;
          }
          if (blocks_[i]->used < blockCapacity_) {
            partial_.push_back(kept);
          }
          if (kept != i) {
            blocks_[kept] = std::move(blocks_[i]);
          }
          ++kept;
        }
        blocks_.erase(blocks_.begin() + static_cast<std::ptrdiff_t>(kept), blocks_.end());
      }

      /// Rebuild `partial_` from the active blocks.
      void rebuildPartial() {
        partial_.clear();
        for (std::size_t i = 0; i < blocks_.size(); ++i) {
          if (blocks_[i]->used < blockCapacity_) {
            partial_.push_back(i);
          }
        }
      }

      /// Recompute `binBegin_` from the bin of each active block. Requires blocks grouped by bin.
      void rebuildBinTable() {
        std::fill(binBegin_.begin(), binBegin_.end(), std::size_t{0});
        for (const auto &block : blocks_) {
          ++binBegin_[block->bin + 1];
        }
        for (std::size_t bin = 1; bin < binBegin_.size(); ++bin) {
          binBegin_[bin] += binBegin_[bin - 1];
        }
      }

      /// Take an unused block from the pool, or allocate a new one.
      std::unique_ptr<Block> acquireBlock() {
        if (!pool_.empty()) {
          std::unique_ptr<Block> block = std::move(pool_.back());
          pool_.pop_back();
          return block;
        }
        return std::make_unique<Block>(blockCapacity_);
      }

      std::size_t blockCapacity_;
      /// Active blocks in iteration order.
      std::vector<std::unique_ptr<Block>> blocks_;
      /// Indices into `blocks_` of active blocks with `used < blockCapacity_`.
      std::vector<std::size_t> partial_;
      /// Owned blocks with `used == 0`, available for reuse.
      std::vector<std::unique_ptr<Block>> pool_;
      std::size_t size_ = 0;
      /// First active block index of each bin, plus a final sentinel. Empty when not binned.
      std::vector<std::size_t> binBegin_;
  };

}  // namespace schnek

#endif /* SCHNEK_DECOMPOSITION_DETAIL_BLOCK_PARTICLE_CONTAINER_HPP_ */
