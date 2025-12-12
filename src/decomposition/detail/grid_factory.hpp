
/*
 * grid_factory.hpp
 *
 * Created on: 20 Nov 2025
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
#ifndef SCHNEK_DECOMPOSITION_DETAIL_GRID_FACTORY_HPP_
#define SCHNEK_DECOMPOSITION_DETAIL_GRID_FACTORY_HPP_

#include <cstddef>
#include <memory>
#include <typeinfo>
#include <utility>

#include "../../grid/array.hpp"
#include "../../grid/arraycheck.hpp"
#include "../../grid/field.hpp"
#include "../../grid/grid.hpp"
#include "../../grid/range.hpp"
#include "../../util/unique.hpp"
#include "grid_visitor.hpp"

namespace schnek {

  namespace internal {

    struct GridWrapper {
        virtual ~GridWrapper() = default;
        virtual void accept(GridVisitorBase &visitor, bool useFieldInfo) = 0;
    };

    using pGridWrapper = std::shared_ptr<GridWrapper>;

    template<typename GridType>
    struct GridWrapperImpl : GridWrapper {
        explicit GridWrapperImpl(GridType gridIn) : grid(std::move(gridIn)) {}

        void accept(GridVisitorBase &visitor, bool useFieldInfo) override {
          visitor.visit(typeid(GridType), &grid, useFieldInfo);
        }

        GridType grid;
    };

    template<size_t rank, template<size_t> class CheckingPolicy = ArrayNoArgCheck>
    struct GridRegistrationInterface : public Unique<GridRegistrationInterface<rank, CheckingPolicy>> {
        using RangeType = Range<ptrdiff_t, rank, CheckingPolicy>;
        using DomainType = Range<double, rank, CheckingPolicy>;

        virtual ~GridRegistrationInterface() = default;
        virtual pGridWrapper makeGrid(const RangeType &range, const DomainType &domain) = 0;
    };

    template<size_t rank, template<size_t> class CheckingPolicy = ArrayNoArgCheck>
    using pGridRegistrationInterface = std::shared_ptr<GridRegistrationInterface<rank, CheckingPolicy>>;

  }  // namespace internal

  template<class GridType>
  class GridFactory;

  template<typename T, size_t rank, template<typename, size_t> class... Policies>
  class GridFactory<Grid<T, rank, Policies...>> {
    public:
      using GridType = Grid<T, rank, Policies...>;

      GridFactory() = default;

      template<typename RangeType, typename DomainType>
      internal::pGridWrapper newGrid(const RangeType &range, const DomainType &domain) const {
        using TargetRange = typename GridType::RangeType;
        GridType grid(TargetRange{range});
        (void)domain;
        return std::make_shared<internal::GridWrapperImpl<GridType>>(std::move(grid));
      }
  };

  template<typename T, size_t rank, template<typename, size_t> class... Policies>
  class GridFactory<Field<T, rank, Policies...>> {
    public:
      using GridType = Field<T, rank, Policies...>;
      using StaggerType = typename GridType::StaggerType;

      GridFactory() = delete;

      template<template<size_t> class CheckingPolicy>
      GridFactory(const Array<bool, rank, CheckingPolicy> &staggerConfig, int ghostCellCount)
          : stagger(staggerConfig), ghostCells(ghostCellCount) {}

      template<typename RangeType, typename DomainType>
      internal::pGridWrapper newGrid(const RangeType &range, const DomainType &domain) const {
        using TargetRange = typename GridType::RangeType;
        GridType grid(TargetRange{range}, domain, stagger, ghostCells);
        return std::make_shared<internal::GridWrapperImpl<GridType>>(std::move(grid));
      }

    private:
      StaggerType stagger;
      int ghostCells;
  };

  struct GridRegistration {
      long id = -1;
  };

  namespace internal {

    template<size_t rank, template<size_t> class CheckingPolicy, typename GridType>
    struct GridRegistrationImpl : public GridRegistrationInterface<rank, CheckingPolicy> {
        using Base = GridRegistrationInterface<rank, CheckingPolicy>;
        using RangeType = typename Base::RangeType;
        using DomainType = typename Base::DomainType;

        explicit GridRegistrationImpl(GridFactory<GridType> &factoryIn) : factory(factoryIn) {}

        pGridWrapper makeGrid(const RangeType &range, const DomainType &domain) override {
          return factory.newGrid(range, domain);
        }

        GridFactory<GridType> &factory;
    };

  }  // namespace internal

}  // namespace schnek

#endif  // SCHNEK_DECOMPOSITION_DETAIL_GRID_FACTORY_HPP_