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

#include <cstddef>
#include <cstring>
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include "../../grid/array.hpp"
#include "../../grid/arraycheck.hpp"
#include "../../grid/range.hpp"
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
       * Remove every particle for which `pred(particle)` returns true, calling
       * `sink(particle)` on each removed particle *before* it is erased.
       *
       * This is the single primitive that drives migration: out-of-bounds
       * particles are serialised by `sink` and then dropped. The default
       * implementation performs a stable compaction in a single pass so that
       * surviving particles keep their relative order and no reallocation
       * occurs.
       */
      template<class Pred, class Sink>
      static void extractIf(ContainerType &c, Pred &&pred, Sink &&sink) {
        std::size_t out = 0;
        const std::size_t count = c.size();
        for (std::size_t i = 0; i < count; ++i) {
          if (pred(c[i])) {
            sink(c[i]);
          } else {
            if (out != i) {
              c[out] = std::move(c[i]);
            }
            ++out;
          }
        }
        c.resize(out);
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

  /// Default serializer for trivially-copyable particles (a plain memcpy).
  template<class Particle>
  struct ParticleSerializer<Particle, std::enable_if_t<std::is_trivially_copyable_v<Particle>>> {
      static constexpr std::size_t size() { return sizeof(Particle); }

      static void serialize(const Particle &p, std::byte *dst) { std::memcpy(dst, &p, sizeof(Particle)); }

      static Particle deserialize(const std::byte *src) {
        Particle p;
        std::memcpy(&p, src, sizeof(Particle));
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
