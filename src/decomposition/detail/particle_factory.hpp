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
#include "../../util/unique.hpp"
#include "grid-communication-traits.hpp"
#include "particle_visitor.hpp"

#include <cstdint>

namespace schnek {

  namespace internal {

#ifdef SCHNEK_HAVE_KOKKOS
    /**
     * Deserialises `count` particles from the device-resident bytes `src` and appends them to `c`.
     *
     * `c.size()`, `c.resize(n)` (keeps existing elements) and `c.data()` address the container's memory space.
     */
    template<class Serializer, class ContainerType>
    void deviceBulkInsert(ContainerType &c, const std::byte *src, std::size_t count) {
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
          Kokkos::RangePolicy<Kokkos::DefaultExecutionSpace>(std::size_t{0}, count),
          KOKKOS_LAMBDA(const std::size_t i) { dst[i] = Serializer::deserialize(src + i * particleBytes); }
      );
      Kokkos::fence();
    }

    /**
     * Returns true when `pred(p)` holds for every particle in the device-resident container `c`.
     */
    template<class ContainerType, class Predicate>
    bool deviceAllOf(const ContainerType &c, Predicate pred) {
      using Particle = typename ContainerType::value_type;
      const Particle *data = c.data();
      bool result = true;
      Kokkos::parallel_reduce(
          "schnek:allOfParticles",
          Kokkos::RangePolicy<Kokkos::DefaultExecutionSpace>(std::size_t{0}, c.size()),
          KOKKOS_LAMBDA(const std::size_t i, bool &ok) { ok = ok && pred(data[i]); },
          Kokkos::LAnd<bool>(result)
      );
      return result;
    }

    /**
     * Counts the particles of the device-resident container `c` that `classify` assigns to each bucket (1-based).
     */
    template<std::size_t NumBuckets, class ContainerType, class Classifier>
    void deviceCountBuckets(const ContainerType &c, const Classifier &classify, std::array<std::size_t, NumBuckets> &counts) {
      using Particle = typename ContainerType::value_type;
      const Particle *data = c.data();
      const std::size_t n = c.size();
      for (std::size_t b = 0; b < NumBuckets; ++b) {
        const int bucket = static_cast<int>(b) + 1;
        std::size_t total = 0;
        Kokkos::parallel_reduce(
            "schnek:countBucketParticles",
            Kokkos::RangePolicy<Kokkos::DefaultExecutionSpace>(std::size_t{0}, n),
            KOKKOS_LAMBDA(const std::size_t i, std::size_t &acc) {
              if (classify(data[i]) == bucket) {
                ++acc;
              }
            },
            total
        );
        counts[b] = total;
      }
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
    template<class Serializer, class ContainerType, class Classifier, std::size_t NumBuckets>
    void devicePartitionAndPack(
        ContainerType &c,
        const Classifier &classify,
        const std::array<std::byte *, NumBuckets> &outPtrs,
        std::byte *scratch
    ) {
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
      for (int target = 0; target <= static_cast<int>(NumBuckets); ++target) {
        std::size_t total = 0;
        Kokkos::parallel_scan(
            "schnek:exclusiveScanBucket",
            Kokkos::RangePolicy<Kokkos::DefaultExecutionSpace>(std::size_t{0}, n),
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
              Kokkos::RangePolicy<Kokkos::DefaultExecutionSpace>(std::size_t{0}, n),
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
              Kokkos::RangePolicy<Kokkos::DefaultExecutionSpace>(std::size_t{0}, n),
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
          Kokkos::RangePolicy<Kokkos::DefaultExecutionSpace>(std::size_t{0}, survivorCount),
          KOKKOS_LAMBDA(const std::size_t i) { data[i] = survivors[i]; }
      );
      Kokkos::fence();
      c.resize(survivorCount);
    }
#else
    template<class Serializer, class ContainerType>
    void deviceBulkInsert(ContainerType &, const std::byte *, std::size_t) {
      static_assert(sizeof(ContainerType) == 0, "device-resident particle containers require Kokkos support");
    }

    template<class ContainerType, class Predicate>
    bool deviceAllOf(const ContainerType &, Predicate) {
      static_assert(sizeof(ContainerType) == 0, "device-resident particle containers require Kokkos support");
      return false;
    }

    template<std::size_t NumBuckets, class ContainerType, class Classifier>
    void deviceCountBuckets(const ContainerType &, const Classifier &, std::array<std::size_t, NumBuckets> &) {
      static_assert(sizeof(ContainerType) == 0, "device-resident particle containers require Kokkos support");
    }

    template<class Serializer, class ContainerType, class Classifier, std::size_t NumBuckets>
    void devicePartitionAndPack(ContainerType &, const Classifier &, const std::array<std::byte *, NumBuckets> &, std::byte *) {
      static_assert(sizeof(ContainerType) == 0, "device-resident particle containers require Kokkos support");
    }
#endif

  }  // namespace internal

  /**
   * @brief Adapts an arbitrary particle container to the small set of
   * operations the domain decomposition requires.
   *
   * The primary template provides a default implementation suitable for any
   * array-of-structs (AoS) container that exposes `value_type`, `size()`,
   * `push_back()`, random-access `operator[]` and `resize()` (e.g.
   * `std::vector`). Users with bespoke storage specialise this trait rather
   * than inheriting from an interface.
   *
   * @tparam ContainerType the user's particle storage type
   */
  template<class ContainerType, class Enable = void>
  struct ParticleContainerTraits {
      using value_type = typename ContainerType::value_type;

      /// Number of particles currently stored.
      static std::size_t size(const ContainerType &c) { return c.size(); }

      /// Append a particle (used when inserting received particles).
      static void insert(ContainerType &c, const value_type &p) { c.push_back(p); }
      static void insert(ContainerType &c, value_type &&p) { c.push_back(std::move(p)); }

      /// Read-only visitation of every particle.
      template<class F>
      static void forEach(const ContainerType &c, F &&f) {
        for (const auto &p : c) {
          f(p);
        }
      }

      /// True when `pred(p)` holds for every particle. Device-resident containers require a device-callable `pred`.
      template<class Predicate>
      static bool allOf(const ContainerType &c, Predicate pred) {
        if constexpr (detail::ParticleCommunicationTraits<ContainerType>::device_resident) {
          return internal::deviceAllOf(c, pred);
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
       * `classify(p)` returns `0` to keep a particle or `1 .. NumBuckets` to
       * remove it into `counts[bucket - 1]`. The caller uses the counts to size
       * the byte buffers passed to `partitionAndPack()`.
       */
      template<std::size_t NumBuckets, class Classifier>
      static void countBuckets(const ContainerType &c, const Classifier &classify, std::array<std::size_t, NumBuckets> &counts) {
        if constexpr (detail::ParticleCommunicationTraits<ContainerType>::device_resident) {
          internal::deviceCountBuckets(c, classify, counts);
        } else {
          counts.fill(0);
          for (const auto &p : c) {
            const int bkt = classify(p);
            if (bkt != 0) {
              ++counts[static_cast<std::size_t>(bkt) - 1];
            }
          }
        }
      }

      /**
       * Number of scratch bytes `partitionAndPack()` needs in the buffer's memory space.
       * Only device-resident containers use scratch; host containers need none.
       */
      static std::size_t partitionScratchBytes([[maybe_unused]] const ContainerType &c) {
        if constexpr (detail::ParticleCommunicationTraits<ContainerType>::device_resident) {
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
       * @tparam NumBuckets  the number of removal buckets (deduced)
       * @tparam Classifier  the device-callable particle classifier (deduced)
       */
      template<class Serializer, std::size_t NumBuckets, class Classifier>
      static void partitionAndPack(
          ContainerType &c,
          const Classifier &classify,
          const std::array<std::byte *, NumBuckets> &outPtrs,
          [[maybe_unused]] std::byte *scratch
      ) {
        if constexpr (detail::ParticleCommunicationTraits<ContainerType>::device_resident) {
          internal::devicePartitionAndPack<Serializer>(c, classify, outPtrs, scratch);
        } else {
          constexpr std::size_t particleBytes = Serializer::size();
          const std::size_t n = c.size();

          std::array<std::size_t, NumBuckets> offset{};
          std::size_t out = 0;
          for (std::size_t i = 0; i < n; ++i) {
            const int bkt = classify(c[i]);
            if (bkt == 0) {
              if (out != i) {
                c[out] = std::move(c[i]);
              }
              ++out;
            } else {
              const std::size_t b = static_cast<std::size_t>(bkt) - 1;
              Serializer::serialize(c[i], outPtrs[b] + offset[b] * particleBytes);
              ++offset[b];
            }
          }
          c.resize(out);
        }
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
        if constexpr (detail::ParticleCommunicationTraits<ContainerType>::device_resident) {
          internal::deviceBulkInsert<Serializer>(c, src, count);
        } else {
          for (std::size_t i = 0; i < count; ++i) {
            c.push_back(Serializer::deserialize(src + i * Serializer::size()));
          }
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
