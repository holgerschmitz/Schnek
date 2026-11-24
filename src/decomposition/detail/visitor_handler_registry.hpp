/*
 * visitor_handler_registry.hpp
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
 */
#ifndef SCHNEK_DECOMPOSITION_DETAIL_VISITOR_HANDLER_REGISTRY_HPP_
#define SCHNEK_DECOMPOSITION_DETAIL_VISITOR_HANDLER_REGISTRY_HPP_

#include <cstddef>
#include <functional>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

namespace schnek {
  namespace internal {

    template<typename VisitorType>
    class VisitorHandlerRegistry {
      public:
        template<typename HandlerType>
        void addHandler() {
          initializers.emplace(std::type_index(typeid(HandlerType)), [](VisitorType &visitor) {
            visitor.template registerHandler<HandlerType>();
          });
        }

        void initialize(VisitorType &visitor) const {
          for (const auto &entry : initializers) {
            entry.second(visitor);
          }
        }

        bool empty() const { return initializers.empty(); }
        std::size_t size() const { return initializers.size(); }

      private:
        std::unordered_map<std::type_index, std::function<void(VisitorType &)>> initializers;
    };

  }  // namespace internal
}  // namespace schnek

#endif  // SCHNEK_DECOMPOSITION_DETAIL_VISITOR_HANDLER_REGISTRY_HPP_
