/*
 * grid_visitor.hpp
 *
 * Created on: 8 Dec 2025
 * Author: Holger Schmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2025 Holger Schmitz
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
#ifndef SCHNEK_DECOMPOSITION_DETAIL_GRID_VISITOR_HPP_
#define SCHNEK_DECOMPOSITION_DETAIL_GRID_VISITOR_HPP_

#include <functional>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include "../../grid/field.hpp"
#include "../../grid/grid.hpp"
#include "../../util/exceptions.hpp"

namespace schnek {
  namespace internal {

    class GridVisitorBase {
      public:
        virtual ~GridVisitorBase() = default;
        virtual void visit(const std::type_info &type, void *gridPtr, bool useFieldInfo) = 0;
    };

    template<typename Derived>
    class GridVisitor : public GridVisitorBase {
      public:
        void visit(const std::type_info &type, void *gridPtr, bool useFieldInfo) override {
          auto it = dispatchers.find(std::type_index(type));
          if (it == dispatchers.end()) {
            SCHNECK_FAIL("No visitor registered for grid type " << type.name());
          }
          it->second(static_cast<Derived &>(*this), gridPtr, useFieldInfo);
        }

        template<typename GridType>
        void registerHandler() {
          auto key = std::type_index(typeid(GridType));
          dispatchers.emplace(key, [](Derived &visitor, void *gridPtr, bool useFieldInfo) {
            visitor.template handle<GridType>(*static_cast<GridType *>(gridPtr), useFieldInfo);
          });
        }

        bool empty() const {
          return dispatchers.empty();
        }

      private:
        using Dispatcher = std::function<void(Derived &, void *, bool)>;
        std::unordered_map<std::type_index, Dispatcher> dispatchers;
    };

    template<typename T>
    struct is_field : std::false_type {};

    template<typename ValueType, size_t rank, template<typename, size_t> class... Policies>
    struct is_field<Field<ValueType, rank, Policies...>> : std::true_type {};

    template<typename T>
    inline constexpr bool is_field_v = is_field<T>::value;

    template<typename T>
    struct is_grid : std::false_type {};

    template<typename ValueType, size_t rank, template<typename, size_t> class... Policies>
    struct is_grid<Grid<ValueType, rank, Policies...>> : std::true_type {};

    template<typename T>
    inline constexpr bool is_grid_v = is_grid<T>::value;

  }  // namespace internal
}  // namespace schnek

#endif