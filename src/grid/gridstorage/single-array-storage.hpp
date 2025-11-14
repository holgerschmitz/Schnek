/*
 * single-array-storage-base.hpp
 *
 * Created on: 07 Nov 2022
 * Author: Holger Schmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2012-2022 Holger Schmitz
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

#ifndef SCHNEK_GRID_GRIDSTORAGE_SINGLESTORAGEBASE_HPP_
#define SCHNEK_GRID_GRIDSTORAGE_SINGLESTORAGEBASE_HPP_

#include "../array.hpp"
#include "../../generic/typelist.hpp"
#include "grid-allocation-concept.hpp"
#include "grid-layout-concept.hpp"

namespace schnek {
  /**
   * @brief The storage base extends from an allocation policy and adds some accessor methods
   *
   * @tparam T The type of data stored in the grid
   * @tparam rank The rank of the grid
   * @tparam AllocationPolicy The allocation policy
   */
  template<typename T, size_t Rank, template<typename, size_t> class ...Policies>
  class SingleArrayGridStorage  {
    public:
      typedef T value_type;
      static constexpr size_t rank = Rank;

      /// The grid index type
      typedef Array<ptrdiff_t, rank> IndexType;

      /// The grid size type
      typedef Array<size_t, rank> SizeType;

      /// The grid range type
      typedef Range<ptrdiff_t, rank> RangeType;

      /// The length of the allocated array
      size_t size;

      /// The lowest and highest coordinates in the grid (inclusive)
      RangeType range;

      /// The dimensions of the grid `dims = high - low + 1`
      Array<size_t, rank> dims;
    private:
      using PolicyList = generic::TypeList<Policies<T, rank>...>;
      using AllocationPolicy = typename PolicyList::template getWithDefault<
        schnek::concepts::GridAllocationConceptCondition, 
        schnek::SingleArrayInstantAllocation<T, rank>>;
      using LayoutPolicy = typename PolicyList::template getWithDefault<
        schnek::concepts::GridLayoutConceptCondition, 
        schnek::SingleArrayGridCOrderLayout<T, rank>>;

      concepts::GridAllocationConcept<AllocationPolicy> concept_check;
    protected:
      AllocationPolicy allocation;
      LayoutPolicy layout;

    public:
      /// Default constructor
      SingleArrayGridStorage();

      /// Copy constructor
      SingleArrayGridStorage(const SingleArrayGridStorage &);

      SingleArrayGridStorage(const IndexType &lo, const IndexType &hi);

      SingleArrayGridStorage(const RangeType &range);

      /**
       * @brief Assignment operator
       */
      SingleArrayGridStorage<T, Rank, Policies...> &
      operator=(const SingleArrayGridStorage<T, Rank, Policies...> &);

      /// Access to the underlying raw data
      T *getRawData() const { return this->allocation.getData(); }

      /// Get the lowest coordinate in the grid (inclusive)
      SCHNEK_INLINE const IndexType &getLo() const { return this->range.getLo(); }

      /// Get the highest coordinate in the grid (inclusive)
      SCHNEK_INLINE const IndexType &getHi() const { return this->range.getHi(); }

      /// Get the lowest coordinate in the grid (inclusive)
      SCHNEK_INLINE const RangeType &getRange() const { return this->range; }

      /// Get the dimensions of the grid `dims = high - low + 1`
      SCHNEK_INLINE const SizeType &getDims() const { return this->dims; }

      /// Get k-th component of the lowest coordinate in the grid (inclusive)
      SCHNEK_INLINE ptrdiff_t getLo(size_t k) const { return this->range.getLo(k); }

      /// Get k-th component of the highest coordinate in the grid (inclusive)
      SCHNEK_INLINE ptrdiff_t getHi(size_t k) const { return this->range.getHi(k); }

      /// Get k-th component of the dimensions of the grid `dims = high - low + 1`
      SCHNEK_INLINE size_t getDims(size_t k) const { return this->dims[k]; }

      /// Get the length of the allocated array
      SCHNEK_INLINE size_t getSize() const { return this->size; }
      
      /**
       * @brief Get the lvalue at a given grid index
       *
       * @param index The grid index
       * @return the lvalue at the grid index
       */
      SCHNEK_INLINE T &get(const IndexType &index) { return this->layout.get(index, this->dims); }

      /**
       * @brief Get the rvalue at a given grid index
       *
       * @param index The grid index
       * @return the rvalue at the grid index
       */
      SCHNEK_INLINE const T &get(const IndexType &index) const { return this->layout.get(index, this->dims); }

      /**
       * @brief resizes to grid with lower indices lo[0],...,lo[rank-1]
       * and upper indices hi[0],...,hi[rank-1]
       */
      void resize(const IndexType &low, const IndexType &high) {
        this->allocation.resizeImpl(low, high);
      }

      /**
       * @brief resizes to grid with the range.
       * The endponts of the range are inclusive
       */
      void resize(const RangeType range) {
        this->resize(range.getLo(), range.getHi());
      }

      /**
       * @brief returns the stride of the specified dimension
       */
      SCHNEK_INLINE ptrdiff_t stride(size_t dim) const {
        return this->layout.stride(dim, this->dims);
      }

      typedef T *storage_iterator;
      typedef const T *const_storage_iterator;

      SCHNEK_INLINE storage_iterator begin() { return this->allocation.getData(); }
      SCHNEK_INLINE storage_iterator end() { return this->allocation.getData() + this->size; }

      SCHNEK_INLINE const_storage_iterator cbegin() const { return this->allocation.getData(); }
      SCHNEK_INLINE const_storage_iterator cend() const { return this->allocation.getData() + this->size; }

    private:
      void updateSize(const RangeType &range) {
        this->range = range;
        this->size = 1;
        for (size_t d = 0; d < rank; ++d) {
          this->dims[d] = this->range.getHi(d) - this->range.getLo(d) + 1;
          this->size *= this->dims[d];
        }
        this->layout.updateSize(range, this->allocation.getData());
      }
  };

  //=================================================================
  //==================== SingleArrayGridStorage =====================
  //=================================================================

  template<typename T, size_t Rank, template<typename, size_t> class ...Policies>
  SingleArrayGridStorage<T, Rank, Policies...>::SingleArrayGridStorage()
      : size{0}, range{IndexType{0}, IndexType{0}}, dims{SizeType{0}} {
    this->allocation.onUpdate([this](const RangeType &range) { updateSize(range); });
  }

  template<typename T, size_t Rank, template<typename, size_t> class ...Policies>
  SingleArrayGridStorage<T, Rank, Policies...>::SingleArrayGridStorage(
      const SingleArrayGridStorage &other
  ) : size{other.size}, range{other.range}, dims{other.dims}, allocation{other.allocation}, layout{other.layout} {
    this->allocation.onUpdate([this](const RangeType &range) { updateSize(range); });
  }

  template<typename T, size_t Rank, template<typename, size_t> class ...Policies>
  SingleArrayGridStorage<T, Rank, Policies...>::SingleArrayGridStorage(
      const IndexType &lo, const IndexType &hi
  ) : size{0}, range{IndexType{0}, IndexType{0}}, dims{SizeType{0}} {
    this->allocation.onUpdate([this](const RangeType &range) { updateSize(range); });
    resize(lo, hi);
  }

  template<typename T, size_t Rank, template<typename, size_t> class ...Policies>
  SingleArrayGridStorage<T, Rank, Policies...>::SingleArrayGridStorage(const RangeType &range)
      : size{0}, range{IndexType{0}, IndexType{0}}, dims{SizeType{0}} {
    this->allocation.onUpdate([this](const RangeType &range) { updateSize(range); });
    resize(range.getLo(), range.getHi());
  }

  template<typename T, size_t Rank, template<typename, size_t> class ...Policies>
  SingleArrayGridStorage<T, Rank, Policies...> &
  SingleArrayGridStorage<T, Rank, Policies...>::operator=(const SingleArrayGridStorage<T, Rank, Policies...> &other) {
    if (this != &other) {
      this->size = other.size;
      this->range = other.range;
      this->dims = other.dims;
      this->allocation = other.allocation;
      this->layout = other.layout;
      this->allocation.onUpdate([this](const RangeType &range) { updateSize(range); });
    }
    return *this;
  }
}  // namespace schnek

#endif  // SCHNEK_GRID_GRIDSTORAGE_SINGLESTORAGEBASE_HPP_
