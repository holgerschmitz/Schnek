/*
 * grid.t
 *
 * Created on: 23 Jan 2007
 * Author: Holger Schmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2012 Holger Schmitz
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

// --------------------------------------------------------------
// implementation

#include "arrayexpression.hpp"
#include "range.hpp"
#include "grid.hpp"

namespace schnek {

  namespace internal {
    //=================================================================
    //============================ GridBase ===========================
    //=================================================================

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<template<size_t> class ArrayCheckingPolicy>
    GridBase<T, rank, Policies...>::GridBase(const Array<size_t, rank, ArrayCheckingPolicy>& size)
        : storage(IndexType::Zero(), size - SizeType::Ones()) {}

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<template<size_t> class ArrayCheckingPolicy>
    GridBase<T, rank, Policies...>::GridBase(
        const Array<ptrdiff_t, rank, ArrayCheckingPolicy>& low, const Array<ptrdiff_t, rank, ArrayCheckingPolicy>& high
    )
        : storage(low, high) {}

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<template<size_t> class ArrayCheckingPolicy>
    GridBase<T, rank, Policies...>::GridBase(const Range<ptrdiff_t, rank, ArrayCheckingPolicy>& range)
        : storage(range) {}

    // -------------------------------------------------------------
    // inline functions

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<template<size_t> class ArrayCheckingPolicy>
    inline T& GridBase<T, rank, Policies...>::get(const Array<ptrdiff_t, rank, ArrayCheckingPolicy>& pos) {
      return this->storage.get(CheckingPolicy::check(pos, this->getLo(), this->getHi()));
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<template<size_t> class ArrayCheckingPolicy>
    inline T GridBase<T, rank, Policies...>::get(const Array<ptrdiff_t, rank, ArrayCheckingPolicy>& pos) const {
      return this->storage.get(CheckingPolicy::check(pos, this->getLo(), this->getHi()));
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<template<size_t> class ArrayCheckingPolicy>
    inline T& GridBase<T, rank, Policies...>::operator[](const Array<ptrdiff_t, rank, ArrayCheckingPolicy>& pos
    ) {
      return this->storage.get(CheckingPolicy::check(pos, this->getLo(), this->getHi()));
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<template<size_t> class ArrayCheckingPolicy>
    inline T GridBase<T, rank, Policies...>::operator[](const Array<ptrdiff_t, rank, ArrayCheckingPolicy>& pos
    ) const {
      return this->storage.get(CheckingPolicy::check(pos, this->getLo(), this->getHi()));
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<class Operator, size_t Length>
    inline T& GridBase<T, rank, Policies...>::operator[](const ArrayExpression<Operator, Length>& pos) {
      return this->operator[](IndexType(pos));
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<class Operator, size_t Length>
    inline T GridBase<T, rank, Policies...>::operator[](const ArrayExpression<Operator, Length>& pos) const {
      return this->operator[](IndexType(pos));
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<typename... Indices>
    inline T& GridBase<T, rank, Policies...>::operator()(Indices... indices) {
      static_assert(sizeof...(Indices) == rank, "GridBase::operator() expects exactly rank indices");
      static_assert(
          (std::is_convertible_v<Indices, ptrdiff_t> && ...),
          "GridBase::operator() indices must be convertible to ptrdiff_t"
      );

      IndexType pos{static_cast<ptrdiff_t>(indices)...};
      return this->storage.get(CheckingPolicy::check(pos, this->getLo(), this->getHi()));
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<typename... Indices>
    inline T GridBase<T, rank, Policies...>::operator()(Indices... indices) const {
      static_assert(sizeof...(Indices) == rank, "GridBase::operator() expects exactly rank indices");
      static_assert(
          (std::is_convertible_v<Indices, ptrdiff_t> && ...),
          "GridBase::operator() indices must be convertible to ptrdiff_t"
      );

      IndexType pos{static_cast<ptrdiff_t>(indices)...};
      return this->storage.get(CheckingPolicy::check(pos, this->getLo(), this->getHi()));
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    inline GridBase<T, rank, Policies...>& GridBase<T, rank, Policies...>::operator=(const T& val) {
      auto end = this->storage.end();
      auto dest = this->storage.begin();
      while (dest != end) {
        *dest = val;
        ++dest;
      }

      return *this;
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<typename T2, template<typename, size_t> class... Policies2>
    inline GridBase<T, rank, Policies...>& GridBase<T, rank, Policies...>::operator-=(
        GridBase<T2, rank, Policies2...>& grid
    ) {
      Range<ptrdiff_t, rank> rec(this->getLo(), this->getHi());
      auto it = rec.begin();
      auto end = rec.end();

      while (it != end) {
        this->storage.get(*it) -= grid.get(*it);
        ++it;
      }

      return *this;
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<typename T2, template<typename, size_t> class... Policies2>
    inline GridBase<T, rank, Policies...>& GridBase<T, rank, Policies...>::operator+=(
        GridBase<T2, rank, Policies2...>& grid
    ) {
      Range<ptrdiff_t, rank> rec(this->getLo(), this->getHi());
      auto it = rec.begin();
      auto end = rec.end();

      while (it != end) {
        this->storage.get(*it) += grid.get(*it);
        ++it;
      }

      return *this;
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    inline void GridBase<T, rank, Policies...>::resize(const IndexType& d) {
      IndexType high;
      for (size_t i = 0; i < rank; ++i) high[i] = d[i] - 1;
      storage.resize(IndexType::Zero(), high);
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    inline void GridBase<T, rank, Policies...>::resize(const IndexType& low, const IndexType& high) {
      storage.resize(low, high);
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    inline void GridBase<T, rank, Policies...>::resize(const RangeType& range) {
      storage.resize(range.getLo(), range.getHi());
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<
        typename T2,
        template<typename, size_t>
        class CheckingPolicy2,
        template<typename, size_t>
        class StoragePolicy2>
    inline void GridBase<T, rank, Policies...>::resize(const GridBase<T2, rank, CheckingPolicy2, StoragePolicy2>& grid) {
      storage.resize(grid.getLo(), grid.getHi());
    }


    //=================================================================
    //========================= GridBaseDevice ========================
    //=================================================================

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<template<size_t> class ArrayCheckingPolicy>
    GridBaseDevice<T, rank, Policies...>::GridBaseDevice(const Array<size_t, rank, ArrayCheckingPolicy>& size)
        : storage(IndexType::Zero(), size - SizeType::Ones()) {}

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<template<size_t> class ArrayCheckingPolicy>
    GridBaseDevice<T, rank, Policies...>::GridBaseDevice(
        const Array<ptrdiff_t, rank, ArrayCheckingPolicy>& low, const Array<ptrdiff_t, rank, ArrayCheckingPolicy>& high
    )
        : storage(low, high) {}

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<template<size_t> class ArrayCheckingPolicy>
    GridBaseDevice<T, rank, Policies...>::GridBaseDevice(const Range<ptrdiff_t, rank, ArrayCheckingPolicy>& range)
        : storage(range) {}

    // -------------------------------------------------------------
    // inline functions

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<template<size_t> class ArrayCheckingPolicy>
    SCHNEK_INLINE T& GridBaseDevice<T, rank, Policies...>::get(const Array<ptrdiff_t, rank, ArrayCheckingPolicy>& pos) {
      return this->storage.get(pos);
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<template<size_t> class ArrayCheckingPolicy>
    SCHNEK_INLINE T GridBaseDevice<T, rank, Policies...>::get(const Array<ptrdiff_t, rank, ArrayCheckingPolicy>& pos) const {
      return this->storage.get(pos);
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<template<size_t> class ArrayCheckingPolicy>
    SCHNEK_INLINE T& GridBaseDevice<T, rank, Policies...>::operator[](const Array<ptrdiff_t, rank, ArrayCheckingPolicy>& pos
    ) {
      return this->storage.get(pos);
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<template<size_t> class ArrayCheckingPolicy>
    SCHNEK_INLINE T GridBaseDevice<T, rank, Policies...>::operator[](const Array<ptrdiff_t, rank, ArrayCheckingPolicy>& pos
    ) const {
      return this->storage.get(pos);
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<class Operator, size_t Length>
    SCHNEK_INLINE T& GridBaseDevice<T, rank, Policies...>::operator[](const ArrayExpression<Operator, Length>& pos) {
      return this->operator[](IndexType(pos));
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<class Operator, size_t Length>
    SCHNEK_INLINE T GridBaseDevice<T, rank, Policies...>::operator[](const ArrayExpression<Operator, Length>& pos) const {
      return this->operator[](IndexType(pos));
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<typename... Indices>
    SCHNEK_INLINE T& GridBaseDevice<T, rank, Policies...>::operator()(Indices... indices) {
      static_assert(sizeof...(Indices) == rank, "GridBaseDevice::operator() expects exactly rank indices");
      static_assert(
          (std::is_convertible_v<Indices, ptrdiff_t> && ...),
          "GridBaseDevice::operator() indices must be convertible to ptrdiff_t"
      );

      IndexType pos{static_cast<ptrdiff_t>(indices)...};
      return this->storage.get(pos);
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<typename... Indices>
    SCHNEK_INLINE T GridBaseDevice<T, rank, Policies...>::operator()(Indices... indices) const {
      static_assert(sizeof...(Indices) == rank, "GridBaseDevice::operator() expects exactly rank indices");
      static_assert(
          (std::is_convertible_v<Indices, ptrdiff_t> && ...),
          "GridBaseDevice::operator() indices must be convertible to ptrdiff_t"
      );

      IndexType pos{static_cast<ptrdiff_t>(indices)...};
      return this->storage.get(pos);
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    SCHNEK_INLINE GridBaseDevice<T, rank, Policies...>& GridBaseDevice<T, rank, Policies...>::operator=(const T& val) {
      auto end = this->storage.end();
      auto dest = this->storage.begin();
      while (dest != end) {
        *dest = val;
        ++dest;
      }

      return *this;
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<typename T2, template<typename, size_t> class... Policies2>
    SCHNEK_INLINE GridBaseDevice<T, rank, Policies...>& GridBaseDevice<T, rank, Policies...>::operator-=(
        GridBaseDevice<T2, rank, Policies2...>& grid
    ) {
      Range<ptrdiff_t, rank> rec(this->getLo(), this->getHi());
      auto it = rec.begin();
      auto end = rec.end();

      while (it != end) {
        this->storage.get(*it) -= grid.get(*it);
        ++it;
      }

      return *this;
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<typename T2, template<typename, size_t> class... Policies2>
    SCHNEK_INLINE GridBaseDevice<T, rank, Policies...>& GridBaseDevice<T, rank, Policies...>::operator+=(
        GridBaseDevice<T2, rank, Policies2...>& grid
    ) {
      Range<ptrdiff_t, rank> rec(this->getLo(), this->getHi());
      auto it = rec.begin();
      auto end = rec.end();

      while (it != end) {
        this->storage.get(*it) += grid.get(*it);
        ++it;
      }

      return *this;
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    SCHNEK_INLINE void GridBaseDevice<T, rank, Policies...>::resize(const IndexType& d) {
      IndexType high;
      for (size_t i = 0; i < rank; ++i) high[i] = d[i] - 1;
      storage.resize(IndexType::Zero(), high);
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    SCHNEK_INLINE void GridBaseDevice<T, rank, Policies...>::resize(const IndexType& low, const IndexType& high) {
      storage.resize(low, high);
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    SCHNEK_INLINE void GridBaseDevice<T, rank, Policies...>::resize(const RangeType& range) {
      storage.resize(range.getLo(), range.getHi());
    }

    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    template<
        typename T2,
        template<typename, size_t>
        class CheckingPolicy2,
        template<typename, size_t>
        class StoragePolicy2>
    SCHNEK_INLINE void GridBaseDevice<T, rank, Policies...>::resize(const GridBaseDevice<T2, rank, CheckingPolicy2, StoragePolicy2>& grid) {
      storage.resize(grid.getLo(), grid.getHi());
    }

  }  // namespace internal

  //=================================================================
  //============================= Grid ==============================
  //=================================================================

  template<typename T, size_t rank, template<typename, size_t> class... Policies>
  Grid<T, rank, Policies...>::Grid() : Grid<T, rank, Policies...>::BaseType() {}

  template<typename T, size_t rank, template<typename, size_t> class... Policies>
  Grid<T, rank, Policies...>::Grid(const SizeType& size) : Grid<T, rank, Policies...>::BaseType(size) {}

  template<typename T, size_t rank, template<typename, size_t> class... Policies>
  Grid<T, rank, Policies...>::Grid(const IndexType& low, const IndexType& high)
      : Grid<T, rank, Policies...>::BaseType(low, high) {}

  template<typename T, size_t rank, template<typename, size_t> class... Policies>
  Grid<T, rank, Policies...>::Grid(const RangeType& range) : Grid<T, rank, Policies...>::BaseType(range) {}

  template<typename T, size_t rank, template<typename, size_t> class... Policies>
  template<typename T2, template<size_t> class ArrayCheckingPolicy>
  Grid<T, rank, Policies...>::Grid(
      const Array<T2, rank, ArrayCheckingPolicy>& low, const Array<T2, rank, ArrayCheckingPolicy>& high
  )
      : Grid<T, rank, Policies...>::BaseType(low, high) {}

  template<typename T, size_t rank, template<typename, size_t> class... Policies>
  template<typename T2, template<size_t> class ArrayCheckingPolicy>
  Grid<T, rank, Policies...>::Grid(const Range<T2, rank, ArrayCheckingPolicy>& range)
      : Grid<T, rank, Policies...>::BaseType(range) {}

}  // namespace schnek
