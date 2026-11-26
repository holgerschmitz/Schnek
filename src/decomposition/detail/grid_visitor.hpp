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

#include <type_traits>

#include "../../grid/field.hpp"
#include "../../grid/grid.hpp"
#include "type_indexed_visitor.hpp"

namespace schnek {
  namespace internal {

    class GridVisitorBase {
      public:
        virtual ~GridVisitorBase() = default;
        virtual void visit(const std::type_info &type, void *gridPtr, bool useFieldInfo) = 0;
    };

    template<typename Derived>
    class GridVisitor : public GridVisitorBase, public TypeIndexedVisitor<Derived, bool> {
      public:
        void visit(const std::type_info &type, void *gridPtr, bool useFieldInfo) override {
          this->dispatch(type, gridPtr, "No visitor registered for grid type ", useFieldInfo);
        }
    };

    template<typename Parent, template<typename> class Operation>
    class ForwardingGridVisitor : public GridVisitor<ForwardingGridVisitor<Parent, Operation>> {
      public:
        explicit ForwardingGridVisitor(Parent &parentIn) : parent(parentIn) {}

        template<typename GridType>
        void handle(GridType &grid, bool useFieldInfo) {
          Operation<GridType>::invoke(parent, grid, useFieldInfo);
        }

      private:
        Parent &parent;
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