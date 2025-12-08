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

#include <boost/function_types/parameter_types.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/size.hpp>

#include <memory>
#include <sstream>
#include <type_traits>
#include <utility>

#include "grid_factory.hpp"
#include "../../util/exceptions.hpp"

namespace schnek {
  namespace internal {

    /**
     * @brief Interface for visiting a stored grid via a visitor functor.
     */
    template<typename VisitorType>
    class GridWrapperVisitable {
      public:
        virtual ~GridWrapperVisitable() = default;
        virtual void accept(VisitorType visitor) = 0;
    };

    template<typename VisitorType>
    using pGridWrapperVisitable = std::shared_ptr<GridWrapperVisitable<VisitorType>>;

    /**
     * @brief Visitable implementation keeping a reference-counted handle to the 
     * actual grid wrapper.
     */
    template<typename VisitorType, typename GridType>
    class GridWrapperVisitableImpl : public GridWrapperVisitable<VisitorType> {
      public:
        explicit GridWrapperVisitableImpl(std::shared_ptr<GridWrapperImpl<GridType>> wrapperIn)
            : wrapper(std::move(wrapperIn)) {}

        void accept(VisitorType visitor) override { visitor(wrapper->grid); }

      private:
        std::shared_ptr<GridWrapperImpl<GridType>> wrapper;
    };

    // Helpers to deduce the grid type from the visitor signature
    template<typename Callable, typename Enable = void>
    struct VisitorParameterTypesImpl;

    template<typename Callable>
    struct VisitorParameterTypesImpl<Callable, typename std::enable_if<!std::is_class<Callable>::value>::type> {
        using type = typename boost::function_types::parameter_types<Callable>::type;
    };

    template<typename Callable>
    struct VisitorParameterTypesImpl<Callable, typename std::enable_if<std::is_class<Callable>::value>::type> {
        using RawTypes = typename VisitorParameterTypesImpl<decltype(&Callable::operator())>::type;
        static_assert(boost::mpl::size<RawTypes>::value > 0, "Visitor must provide operator() parameters");
        using type = typename boost::mpl::pop_front<RawTypes>::type;
    };

    template<typename Callable>
    using VisitorParameterTypesT = typename VisitorParameterTypesImpl<typename std::decay<Callable>::type>::type;

    template<typename VisitorType>
    struct VisitorGridTraits {
        using ParameterSeq = VisitorParameterTypesT<VisitorType>;
        static_assert(boost::mpl::size<ParameterSeq>::value == 1, "Visitor must accept exactly one argument");
        using RawParam = typename boost::mpl::front<ParameterSeq>::type;
        static_assert(std::is_lvalue_reference<RawParam>::value, "Visitor parameter must be an lvalue reference");
        using DecayedParam = typename std::remove_reference<RawParam>::type;
        using GridType = typename std::remove_const<DecayedParam>::type;
    };

    template<typename VisitorType>
    using VisitorGridTypeT = typename VisitorGridTraits<VisitorType>::GridType;

    /**
     * @brief Create a visitable wrapper for the given visitor type from a stored grid wrapper.
     */
    template<typename VisitorType>
    pGridWrapperVisitable<VisitorType> makeVisitGridWrapper(const pGridWrapper &wrapper) {
      using GridType = VisitorGridTypeT<VisitorType>;
      auto typed = std::dynamic_pointer_cast<GridWrapperImpl<GridType>>(wrapper);
      if (!typed) {
        SCHNECK_FAIL("Grid type mismatch for visitor");
      }
      return std::make_shared<GridWrapperVisitableImpl<VisitorType, GridType>>(std::move(typed));
    }

    /**
     * @brief Convenience helper invoking a visitor on a generic grid wrapper.
     */
    template<typename VisitorType>
    void visitGridWrapper(const pGridWrapper &wrapper, VisitorType visitor) {
      auto visitable = makeVisitGridWrapper<VisitorType>(wrapper);
      visitable->accept(std::move(visitor));
    }

    // Backwards-compatible aliases using the simpler naming scheme
    template<typename VisitorType>
    using VisitGridWrapper = GridWrapperVisitable<VisitorType>;
    template<typename VisitorType>
    using VisitGridWrapperImpl = GridWrapperVisitableImpl<VisitorType, VisitorGridTypeT<VisitorType>>;
    template<typename VisitorType>
    using pVisitGridWrapper = pGridWrapperVisitable<VisitorType>;

  }  // namespace internal
}  // namespace schnek

#endif