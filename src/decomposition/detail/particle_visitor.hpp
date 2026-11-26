/*
 * particle_visitor.hpp
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
#ifndef SCHNEK_DECOMPOSITION_DETAIL_PARTICLE_VISITOR_HPP_
#define SCHNEK_DECOMPOSITION_DETAIL_PARTICLE_VISITOR_HPP_

#include "type_indexed_visitor.hpp"

namespace schnek {
  namespace internal {

    /**
     * Type-erased visitor base for particle containers.
     *
     * Mirrors `GridVisitorBase`: a single virtual `visit` recovers the static
     * type of a `ParticleWrapperImpl` from its `std::type_info` and dispatches
     * to a cached, fully-templated handler. This keeps the per-particle inner
     * loop free of virtual dispatch.
     */
    class ParticleVisitorBase {
      public:
        virtual ~ParticleVisitorBase() = default;
        virtual void visit(const std::type_info &type, void *wrapperPtr) = 0;
    };

    /**
     * CRTP visitor that dispatches on the concrete wrapper type.
     *
     * `Derived` must provide a templated `handle<WrapperImpl>(WrapperImpl &)`
     * member. Inside `handle`, the container and position accessor have their
     * concrete static types, so all operations on individual particles inline.
     *
     * @tparam Derived the concrete visitor type
     */
    template<typename Derived>
    class ParticleVisitor : public ParticleVisitorBase, public TypeIndexedVisitor<Derived> {
      public:
        void visit(const std::type_info &type, void *wrapperPtr) override {
          this->dispatch(type, wrapperPtr, "No particle visitor registered for wrapper type ");
        }
    };

    template<typename Parent, template<typename> class Operation>
    class ForwardingParticleVisitor : public ParticleVisitor<ForwardingParticleVisitor<Parent, Operation>> {
      public:
        explicit ForwardingParticleVisitor(Parent &parentIn) : parent(parentIn) {}

        template<typename WrapperImpl>
        void handle(WrapperImpl &wrapper) {
          Operation<WrapperImpl>::invoke(parent, wrapper);
        }

      private:
        Parent &parent;
    };

  }  // namespace internal
}  // namespace schnek

#endif  // SCHNEK_DECOMPOSITION_DETAIL_PARTICLE_VISITOR_HPP_
