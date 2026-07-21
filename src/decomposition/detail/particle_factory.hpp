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
#include "particle_visitor.hpp"

namespace schnek {

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

      /**
       * Partition the container in place, routing departing particles into
       * per-bucket, host-accessible byte buffers using a count -> scan -> pack
       * scheme.
       *
       * For every particle, `classify(p)` returns a bucket index:
       *   - `0` keeps the particle (survivors are compacted in place, preserving
       *     their relative order),
       *   - `1 .. NumBuckets` removes the particle and serialises it (via
       *     `Serializer`) into `outBuffers[bucket - 1]`.
       *
       * On return `outBuffers[b]` holds exactly `counts[b] * Serializer::size()`
       * bytes, tightly packed with no worst-case over-allocation, and is safe to
       * hand straight to MPI.
       *
       * This is the single primitive that drives migration. The default
       * (host) implementation performs the three passes serially. A
       * device-resident container specialises this trait and implements each
       * pass with a parallel classify / exclusive-scan / scatter so that no
       * particle data ever has to leave the device before packing; only the
       * final tightly-sized byte buffers are staged to the host. Because the
       * output buffers are ordinary `std::vector<std::byte>`, the surrounding
       * MPI code is identical for host and device containers.
       *
       * @tparam Serializer  the particle serialiser (device-callable)
       * @tparam NumBuckets  the number of removal buckets (deduced)
       * @tparam Classifier  the device-callable particle classifier (deduced)
       */
      template<class Serializer, std::size_t NumBuckets, class Classifier>
      static void partitionAndPack(
          ContainerType &c,
          const Classifier &classify,
          std::array<std::vector<std::byte>, NumBuckets> &outBuffers,
          std::array<std::size_t, NumBuckets> &counts
      ) {
        constexpr std::size_t particleBytes = Serializer::size();
        const std::size_t n = c.size();

        // Pass 1: classify every particle and count per bucket.
        std::vector<int> bucket(n);
        for (std::size_t b = 0; b < NumBuckets; ++b) {
          counts[b] = 0;
        }
        for (std::size_t i = 0; i < n; ++i) {
          const int bkt = classify(c[i]);
          bucket[i] = bkt;
          if (bkt != 0) {
            ++counts[static_cast<std::size_t>(bkt) - 1];
          }
        }

        // Pass 2: size the output buffers from the per-bucket totals (the
        // host-serial equivalent of an exclusive scan over bucket sizes).
        for (std::size_t b = 0; b < NumBuckets; ++b) {
          outBuffers[b].assign(counts[b] * particleBytes, std::byte{0});
        }

        // Pass 3: scatter. Running per-bucket offsets act as the exclusive scan;
        // survivors are compacted in place.
        std::array<std::size_t, NumBuckets> offset{};
        std::size_t out = 0;
        for (std::size_t i = 0; i < n; ++i) {
          const int bkt = bucket[i];
          if (bkt == 0) {
            if (out != i) {
              c[out] = std::move(c[i]);
            }
            ++out;
          } else {
            const std::size_t b = static_cast<std::size_t>(bkt) - 1;
            Serializer::serialize(c[i], outBuffers[b].data() + offset[b] * particleBytes);
            ++offset[b];
          }
        }
        c.resize(out);
      }

      /**
       * Deserialise `count` particles from the tightly-packed, host-accessible
       * byte buffer `src` (`Serializer::size()` bytes each) and append them to
       * `c`.
       *
       * The default implementation deserialises serially on the host. A
       * device-resident container specialises this trait to stage the bytes onto
       * the device and deserialise them in parallel into the appended region.
       */
      template<class Serializer>
      static void bulkInsert(ContainerType &c, const std::byte *src, std::size_t count) {
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
      auto operator()(const Particle &p) const -> decltype(p.*MemberPtr) {
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
