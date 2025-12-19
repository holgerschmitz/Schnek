/*
 * grid.hpp
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

#ifndef SCHNEK_GRID_H_
#define SCHNEK_GRID_H_

#include <cstddef>
#include <type_traits>

#include "../macros.hpp"
#include "../typetools.hpp"
#include "../generic/is-detected.hpp"
#include "array.hpp"
#include "gridcheck/grid-check-concept.hpp"
#include "gridcheck/gridcheck.hpp"
#include "gridstorage.hpp"
#include "range.hpp"

namespace schnek {

  template<class GridType, typename TList>
  class IndexedGrid;

  template<class IndexType>
  struct IndexCast;

  template<class Operator, size_t Length>
  class ArrayExpression;

  namespace internal {
    /**
     * @brief The generic base class for the Grid class template
     *
     * GridBase implements all the overloaded index
     */
    template<typename T, size_t rank, template<typename, size_t> class... Policies>
    class GridBase {
      private:
        using PolicyList = generic::TypeList<Policies<T, rank>...>;
        using StoragePolicy = typename PolicyList::template getWithDefault<
            schnek::concepts::GridStorageConceptCondition,
            schnek::SingleArrayGridStorage<T, rank>>;
        using CheckingPolicy = typename PolicyList::
            template getWithDefault<schnek::concepts::GridCheckConceptCondition, schnek::GridNoArgCheck<T, rank>>;
        concepts::GridCheckConcept<CheckingPolicy> concept_check;
        concepts::GridStorageConcept<StoragePolicy> concept_storage;
        StoragePolicy storage;

      public:
        typedef T value_type;
        typedef typename CheckingPolicy::IndexType IndexType;
        typedef typename StoragePolicy::SizeType SizeType;
        typedef typename StoragePolicy::RangeType RangeType;
        enum { Rank = rank };

      public:
        /**
         * @brief Default constructor
         */
        GridBase() = default;

        /**
         * @brief Copy constructor
         */
        GridBase(const GridBase&) = default;

        template<template<size_t> class ArrayCheckingPolicy>
        GridBase(const Array<size_t, rank, ArrayCheckingPolicy>& size);

        template<template<size_t> class ArrayCheckingPolicy>
        GridBase(
            const Array<ptrdiff_t, rank, ArrayCheckingPolicy>& low,
            const Array<ptrdiff_t, rank, ArrayCheckingPolicy>& high
        );

        template<template<size_t> class ArrayCheckingPolicy>
        GridBase(const Range<ptrdiff_t, rank, ArrayCheckingPolicy>& range);

        /// Get the lowest coordinate in the grid (inclusive)
        SCHNEK_INLINE const IndexType getLo() const { return this->storage.getLo(); }

        /// Get the highest coordinate in the grid (inclusive)
        SCHNEK_INLINE const IndexType getHi() const { return this->storage.getHi(); }

        /// Get the lowest coordinate in the grid (inclusive)
        SCHNEK_INLINE const RangeType getRange() const { return this->storage.getRange(); }

        /// Get the dimensions of the grid `dims = high - low + 1`
        SCHNEK_INLINE const SizeType getDims() const { return this->storage.getDims(); }

        /// Get k-th component of the lowest coordinate in the grid (inclusive)
        SCHNEK_INLINE ptrdiff_t getLo(size_t k) const { return this->storage.getLo(k); }

        /// Get k-th component of the highest coordinate in the grid (inclusive)
        SCHNEK_INLINE ptrdiff_t getHi(size_t k) const { return this->storage.getHi(k); }

        /// Get k-th component of the dimensions of the grid `dims = high - low + 1`
        SCHNEK_INLINE ptrdiff_t getDims(size_t k) const { return this->storage.getDims(k); }

        /// Get the stride of the specified dimension
        SCHNEK_INLINE ptrdiff_t stride(size_t dim) const { return this->storage.stride(dim); }

        /// Get a raw pointer to the underlying data if provided by the storage policy
        template<typename S = StoragePolicy>
        SCHNEK_INLINE auto getRawData() -> std::enable_if_t<
            concepts::GridStorageConceptCondition<S>::has_get_raw_data_method,
            decltype(this->storage.getRawData())> {
          return this->storage.getRawData();
        }

        /// Fallback overload that triggers a compilation error when getRawData is unavailable
        template<typename S = StoragePolicy>
        SCHNEK_INLINE auto getRawData() -> std::enable_if_t<
            !concepts::GridStorageConceptCondition<S>::has_get_raw_data_method,
            T *> {
          static_assert(
              concepts::GridStorageConceptCondition<S>::has_get_raw_data_method,
              "GridBase::getRawData() requires the storage policy to implement getRawData() returning a raw pointer"
          );
          return nullptr;
        }

        /// Get the size of the underlying data if provided by the storage policy
        template<typename S = StoragePolicy>
        SCHNEK_INLINE auto getSize() const -> std::enable_if_t<
            concepts::GridStorageConceptCondition<S>::has_get_size_method,
            decltype(this->storage.getSize())> {
          return this->storage.getSize();
        }

        /// Fallback overload that triggers a compilation error when getSize is unavailable
        template<typename S = StoragePolicy>
        SCHNEK_INLINE auto getSize() const -> std::enable_if_t<
            !concepts::GridStorageConceptCondition<S>::has_get_size_method,
            T *> {
          static_assert(
              concepts::GridStorageConceptCondition<S>::has_get_size_method,
              "GridBase::getSize() requires the storage policy to implement getSize() returning a raw pointer"
          );
          return nullptr;
        }

        /** get access, writing */
        template<template<size_t> class ArrayCheckingPolicy>
        SCHNEK_INLINE T& get(const Array<ptrdiff_t, rank, ArrayCheckingPolicy>& pos);  // write
        /** get access, reading */
        template<template<size_t> class ArrayCheckingPolicy>
        SCHNEK_INLINE T get(const Array<ptrdiff_t, rank, ArrayCheckingPolicy>& pos) const;  // read

        /** index operator, writing */
        template<template<size_t> class ArrayCheckingPolicy>
        SCHNEK_INLINE T& operator[](const Array<ptrdiff_t, rank, ArrayCheckingPolicy>& pos);  // write
        /** index operator, reading */
        template<template<size_t> class ArrayCheckingPolicy>
        SCHNEK_INLINE T operator[](const Array<ptrdiff_t, rank, ArrayCheckingPolicy>& pos) const;  // read

        /** index operator, writing */
        template<class Operator, size_t Length>
        SCHNEK_INLINE T& operator[](const ArrayExpression<Operator, Length>& pos);  // write
        /** index operator, reading */
        template<class Operator, size_t Length>
        SCHNEK_INLINE T operator[](const ArrayExpression<Operator, Length>& pos) const;  // read

        /** index operator forwarding to the checking policy, writing */
        template<typename... Indices>
        SCHNEK_INLINE T& operator()(Indices... indices);
        /** index operator forwarding to the checking policy, reading */
        template<typename... Indices>
        SCHNEK_INLINE T operator()(Indices... indices) const;

        /** index operator, for 1D grids, writing */
        SCHNEK_INLINE T& operator[](ptrdiff_t i) { return this->operator()(i); }
        /** index operator, for 1D grids, reading */
        SCHNEK_INLINE T operator[](ptrdiff_t i) const { return this->operator()(i); }

        /** assign a value */
        SCHNEK_INLINE GridBase<T, rank, Policies...>& operator=(const T& val);

        /** copy constructor */
        SCHNEK_INLINE GridBase<T, rank, Policies...>& operator=(const GridBase<T, rank, Policies...>& val) = default;

        template<typename T2, template<typename, size_t> class... Policies2>
        SCHNEK_INLINE GridBase<T, rank, Policies...>& operator-=(GridBase<T2, rank, Policies2...>&);

        template<typename T2, template<typename, size_t> class... Policies2>
        SCHNEK_INLINE GridBase<T, rank, Policies...>& operator+=(GridBase<T2, rank, Policies2...>&);

        /**
         * @brief Resize to size[0] x ... x size[rank-1]
         *
         * Example:
         * \begin{verbatim}
         * Grid<double,2>::IndexType size=(512,512);
         * Grid<double,2> m;
         * m.resize(size);
         * \end{verbatim}
         *
         * The ranges then extend from 0 to size[i]-1
         */
        void resize(const IndexType& size);

        /**
         * @brief Resize to lower indices low[0],...,low[rank-1]
         * and upper indices high[0],...,high[rank-1]
         *
         * Example:
         * \begin{verbatim}
         * Grid<double,2>::IndexType lo{-5,-10};
         * Grid<double,2>::IndexType hi{15,36};
         * Grid<double,2> m;
         * m.resize(l,h);
         * \end{verbatim}
         *
         * The ranges then extend from low[i] to high[i]
         */
        void resize(const IndexType& low, const IndexType& high);

        /**
         * @brief Resize to lower indices range.getLo(0),...,range.getLo(rank-1)
         * and upper indices range.getHi(0),...,range.getHi(rank-1)
         *
         * Example:
         * \begin{verbatim}
         * Grid<double,2>::IndexType lo{-5, -10};
         * Grid<double,2>::IndexType hi{15, 36};
         * Grid<double,2>::RangeType range{lo, hi};
         * Grid<double,2> m;
         * m.resize(range);
         * \end{verbatim}
         */
        void resize(const RangeType& range);

        /** Resize to match the size of another matrix */
        template<
            typename T2,
            template<typename, size_t>
            class CheckingPolicy2,
            template<typename, size_t>
            class StoragePolicy2>
        void resize(const GridBase<T2, rank, CheckingPolicy2, StoragePolicy2>& grid);
    };
  }  // namespace internal

  /**
   * @brief A multidimensional grid that stores simple data
   *
   * @tparam T the data type of the grid
   * @tparam rank the dimensionality of the grid
   * @tparam CheckingPolicy a policy for checking index access operations
   * @tparam StoragePolicy a policy that defines how the data is stored in memory
   */
  template<typename T, size_t rank, template<typename, size_t> class... Policies>
  class Grid : public internal::GridBase<T, rank, Policies...> {
    public:
      typedef T value_type;
      typedef Array<ptrdiff_t, rank> IndexType;
      typedef Array<size_t, rank> SizeType;
      typedef Range<ptrdiff_t, rank> RangeType;
      typedef Grid<T, rank, Policies...> GridType;
      typedef internal::GridBase<T, rank, Policies...> BaseType;
      enum { Rank = rank };

      /**
       * @brief default constructor creates an empty grid
       */
      Grid();

      /**
       * @brief copy constructor
       */
      Grid(const Grid&) = default;

      /**
       * @brief constructor, which builds Grid of size size[0] x ... x size[rank-1]
       *
       *  Example:
       *  \begin{verbatim}
       *  Grid<double,2>::IndexType size=(512,512);
       *  Grid<double,2> m(size);
       *  \end{verbatim}
       *
       *  The ranges then extend from 0 to size[i]-1
       */
      Grid(const SizeType& size);

      /** constructor, which builds Grid with lower indices low[0],...,low[rank-1]
       *  and upper indices high[0],...,high[rank-1]
       *
       *  Example:
       *  \begin{verbatim}
       *  Grid<double,2>::IndexType low(-5,-10);
       *  Grid<double,2>::IndexType high(15,36);
       *  Grid<double,2> m(l,h);
       *  \end{verbatim}
       *
       *  The ranges then extend from low[i] to high[i]
       */
      Grid(const IndexType& low, const IndexType& high);

      template<typename T2, template<size_t> class ArrayCheckingPolicy>
      Grid(const Array<T2, rank, ArrayCheckingPolicy>& low, const Array<T2, rank, ArrayCheckingPolicy>& high);

      /** constructor, which builds Grid with range given by range
       *
       *  Example:
       *  \begin{verbatim}
       *  Grid<double,2>::IndexType low(-5,-10);
       *  Grid<double,2>::IndexType high(15,36);
       *  Grid<double,2>::RangeType range(low, high);
       *  Grid<double,2> m(range);
       *  \end{verbatim}
       *
       *  The ranges then extend from low[i] to high[i]
       */
      Grid(const RangeType& range);

      template<typename T2, template<size_t> class ArrayCheckingPolicy>
      Grid(const Range<T2, rank, ArrayCheckingPolicy>& range);

      /** assign another grid */
      GridType& operator=(const T& val) {
        BaseType::operator=(val);
        return *this;
      }

      /** assign another grid */
      GridType& operator=(const GridType& grid) {
        BaseType::operator=(grid);
        return *this;
      }

      /** assign another grid */
      template<
          typename T2,
          template<typename, size_t>
          class CheckingPolicy2,
          template<typename, size_t>
          class StoragePolicy2>
      GridType& operator=(const internal::GridBase<T2, rank, CheckingPolicy2, StoragePolicy2>& grid) {
        BaseType::operator=(grid);
        return *this;
      }
  };

}  // namespace schnek

#include "grid.t"

#endif  // SCHNEK_GRID_H_
