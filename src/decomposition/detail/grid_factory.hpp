
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

#include "../../grid/array.hpp"
#include "../../grid/arraycheck.hpp"
#include "../../grid/grid.hpp"
#include "../../grid/field.hpp"

#include <memory>

namespace schnek {
    namespace internal {
        struct GridWrapper {
          virtual ~GridWrapper() {}
        };

        typedef std::shared_ptr<GridWrapper> pGridWrapper;

        template<typename GridType>
        struct GridWrapperImpl {
          GridType grid;
        };

        template<size_t rank, template<size_t> class CheckingPolicy = ArrayNoArgCheck>
        struct GridRegistrationInterface: public Unique<GridRegistrationInterface> {
            typedef Range<ptrdiff_t, rank, CheckingPolicy> RangeType;
            typedef Range<double, rank, CheckingPolicy> DomainType;

            virtual ~GridRegistrationInterface() {}
            virtual pGridWrapper makeGrid(const RangeType& range,  DomainType domain) = 0;
        };

        std::shared_ptr<GridRegistrationInterface> pGridRegistrationInterface;
    }

    template<class GridType>
    struct GridFactory {};

    template<typename T, size_t rank, template<typename, size_t> class ...Policies>
    struct GridFactory<Grid<T, rank, Policies...> > {
        typedef Grid<T, rank, Policies...> GridType;
        pGridWrapper newGrid(const RangeType& range,  DomainType domain) {
            GridType grid(range);
            return std::make_shared<GridWrapperImpl<GridType>>(grid);
        }
    };

    template<typename T, size_t rank, template<typename, size_t> class ...Policies>
    struct GridFactory<Field<T, rank, Policies...> > {
        typedef Field<T, rank, Policies...> GridType;
        
        Array<bool::rank> stagger;
        size_t ghostCells;

        pGridWrapper newGrid(const RangeType& range,  DomainType domain) {
            GridType grid(range, domain, stagger, ghostCells);
            return std::make_shared<GridWrapperImpl<GridType>>(grid);
        }
    };
  

    struct GridRegistration {
      long id;
    };

    namespace internal {

        template<size_t rank, template<size_t> class CheckingPolicy = ArrayNoArgCheck, typename GridType>
        struct GridRegistrationImpl : public GridRegistrationInterface<rank, CheckingPolicy> {
            GridFactory<GridType> &factory;
            pGridWrapper makeGrid(const RangeType& range,  DomainType domain) override {
              return std::make_shared<GridWrapper>(factory.newGrid(range, domain));
            }
        };
    } // namespace internal



} // namespace schnek