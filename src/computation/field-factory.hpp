/*
 * field-factory.hpp
 *
 * Created on: 19 Jul 2024
 * Author: Holger Schmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2024 Holger Schmitz
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

#ifndef SCHNEK_COMPUTATION_FIELD_FACTORY_HPP_
#define SCHNEK_COMPUTATION_FIELD_FACTORY_HPP_

#include <stddef.h>

#include "../grid/field.hpp"
#include "../grid/grid.hpp"

namespace schnek::computation {

  /**
   * Used as the FieldType for Fields that can be used on multiple architectures
   */
  template<typename T, size_t rank, template<typename, size_t> class CheckingPolicy = GridNoArgCheck>
  struct FieldTypeWrapper {
      template<template<typename, size_t> typename StorageType>
      using type = Field<T, rank, CheckingPolicy, StorageType>;
  };

  /**
   * Used as the FieldType for Grids that can be used on multiple architectures
   */
  template<typename T, size_t rank, template<typename, size_t> class CheckingPolicy = GridNoArgCheck>
  struct GridTypeWrapper {
      template<template<typename, size_t> typename StorageType>
      using type = Grid<T, rank, CheckingPolicy, StorageType>;
  };

  struct SimpleHostArchitecture {
      template<typename T, size_t rank>
      using GridStorageType = SingleArrayGridStorage<T, rank>;
  };

  /**
   * A factory for fields that can be used on a single architecture
   *
   * @tparam FieldType The type of field that the factory creates.
   */
  template<class FieldType>
  struct MultiArchitectureFieldFactory {
      template<typename Architecture>
      typename FieldType::type<Architecture::template GridStorageType> create(
          const typename FieldType::type<Architecture::template GridStorageType>::RangeType &size,
          const typename FieldType::type<Architecture::template GridStorageType>::DomainType &domain,
          const typename FieldType::type<Architecture::template GridStorageType>::StaggerType &stagger,
          size_t ghostCells
      );
  };

  template<typename T, size_t rank, template<typename, size_t> class CheckingPolicy>
  struct MultiArchitectureFieldFactory<FieldTypeWrapper<T, rank, CheckingPolicy> > {
      template<typename Architecture>
      typename FieldTypeWrapper<T, rank, CheckingPolicy>::type<Architecture::template GridStorageType> create(
          const typename FieldTypeWrapper<T, rank, CheckingPolicy>::type<
              Architecture::template GridStorageType>::RangeType &size,
          const typename FieldTypeWrapper<T, rank, CheckingPolicy>::type<
              Architecture::template GridStorageType>::DomainType &domain,
          const typename FieldTypeWrapper<T, rank, CheckingPolicy>::type<
              Architecture::template GridStorageType>::StaggerType &stagger,
          size_t ghostCells
      ) {
        typedef typename FieldTypeWrapper<T, rank, CheckingPolicy>::type<Architecture::template GridStorageType>
            FieldType;
        return FieldType(size, domain, stagger, ghostCells);
      }
  };

  // template<
  //     typename T,
  //     size_t rank,
  //     template<size_t> class CheckingPolicy
  // >
  // struct MultiArchitectureFieldFactory<GridTypeWrapper<T, rank, CheckingPolicy>::type> {
  //     template<typename Architecture>
  //     static typename FieldType<Architecture>::type create(
  //         const typename FieldType::RangeType &size,
  //         const typename FieldType::DomainType &domain,
  //         const typename FieldType::StaggerType &stagger,
  //         size_t ghostCells
  //     );
  // };

}  // namespace schnek::computation

#endif  // SCHNEK_COMPUTATION_FIELD_FACTORY_HPP_