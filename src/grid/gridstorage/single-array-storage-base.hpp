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
  class SingleArrayGridStorageBase  {
    public:
      typedef T value_type;
      static constexpr size_t rank = Rank;

      /// The grid index type
      typedef Array<int, rank> IndexType;

      /// The grid range type
      typedef Range<int, rank> RangeType;

      /// The length of the allocated array
      size_t size;

      /// The lowest and highest coordinates in the grid (inclusive)
      RangeType range;

      /// The dimensions of the grid `dims = high - low + 1`
      IndexType dims;
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
      SingleArrayGridStorageBase();

      /// Copy constructor
      SingleArrayGridStorageBase(const SingleArrayGridStorageBase &);

      SingleArrayGridStorageBase(const IndexType &lo, const IndexType &hi);

      SingleArrayGridStorageBase(const RangeType &range);

      /**
       * @brief Assignment operator
       */
      SingleArrayGridStorageBase<T, Rank, Policies...> &
      operator=(const SingleArrayGridStorageBase<T, Rank, Policies...> &);

      /// Access to the underlying raw data
      T *getRawData() const { return this->allocation.getData(); }

      /// Get the lowest coordinate in the grid (inclusive)
      SCHNEK_INLINE const IndexType &getLo() const { return this->range.getLo(); }

      /// Get the highest coordinate in the grid (inclusive)
      SCHNEK_INLINE const IndexType &getHi() const { return this->range.getHi(); }

      /// Get the lowest coordinate in the grid (inclusive)
      SCHNEK_INLINE const RangeType &getRange() const { return this->range; }

      /// Get the dimensions of the grid `dims = high - low + 1`
      SCHNEK_INLINE const IndexType &getDims() const { return this->dims; }

      /// Get k-th component of the lowest coordinate in the grid (inclusive)
      SCHNEK_INLINE int getLo(int k) const { return this->range.getLo(k); }

      /// Get k-th component of the highest coordinate in the grid (inclusive)
      SCHNEK_INLINE int getHi(int k) const { return this->range.getHi(k); }

      /// Get k-th component of the dimensions of the grid `dims = high - low + 1`
      SCHNEK_INLINE int getDims(int k) const { return this->dims[k]; }

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

  /**
   * @brief Extends from SingleArrayGridStorageBase to provide C-order indexing over the
   * 1-dimensional data array for a multidimensional grid.
   *
   * @tparam T The type of data stored in the grid
   * @tparam rank The rank of the grid
   * @tparam AllocationPolicy The allocation policy
   */
  template<typename T, size_t rank, template<typename, size_t> class AllocationPolicy>
  class SingleArrayGridCOrderStorageBase : public SingleArrayGridStorageBase<T, rank, AllocationPolicy> {
    private:
      /// A pointer offset to the origin for faster access
      T *data_fast;

    public:
      /// Base class type
      typedef SingleArrayGridStorageBase<T, rank, AllocationPolicy> BaseType;

      /// The grid index type
      typedef typename BaseType::IndexType IndexType;

      /// The grid index type
      typedef typename BaseType::RangeType RangeType;

      /// Default constructor
      SingleArrayGridCOrderStorageBase();

      /// Copy constructor
      SingleArrayGridCOrderStorageBase(const SingleArrayGridCOrderStorageBase &);

      /**
       * @brief Construct with a given size
       *
       * @param lo the lowest coordinate in the grid (inclusive)
       * @param hi the highest coordinate in the grid (inclusive)
       */
      SingleArrayGridCOrderStorageBase(const IndexType &lo, const IndexType &hi);

      /**
       * @brief Construct with a given size
       *
       * @param range the lowest and highest coordinates in the grid (inclusive)
       */
      SingleArrayGridCOrderStorageBase(const RangeType &range);

      /**
       * @brief Assignment operator
       */
      SingleArrayGridCOrderStorageBase<T, rank, AllocationPolicy> &
      operator=(const SingleArrayGridCOrderStorageBase<T, rank, AllocationPolicy> &) = default;

      /**
       * @brief Get the lvalue at a given grid index
       *
       * @param index The grid index
       * @return the lvalue at the grid index
       */
      SCHNEK_INLINE T &get(const IndexType &index);

      /**
       * @brief Get the rvalue at a given grid index
       *
       * @param index The grid index
       * @return the rvalue at the grid index
       */
      SCHNEK_INLINE const T &get(const IndexType &index) const;

      /**
       * @brief resizes to grid with lower indices lo[0],...,lo[rank-1]
       * and upper indices hi[0],...,hi[rank-1]
       */
      void resize(const IndexType &low, const IndexType &high);

      /**
       * @brief resizes to grid with the range.
       * The endponts of the range are inclusive
       */
      void resize(const RangeType range);

      /**
       * @brief returns the stride of the specified dimension
       */
      ptrdiff_t stride(size_t dim) const;

    private:
      /**
       * @brief Update the data_fast pointer offset to the origin for faster access
       *
       * This method is called indirectly when a resize is performed on any of the copies of the
       * grid, including the original grid and any copies made by the copy constructor or the
       * assignment operator.
       *
       * This ensures that the data_fast pointer is always up to date between all copies of the
       * grid.
       */
      void updateDataFast(const RangeType &range);
  };

  /**
   * @brief Extends from SingleArrayGridStorageBase to provide Fortran-order indexing over the
   * 1-dimensional data array for a multidimensional grid.
   *
   * @tparam T The type of data stored in the grid
   * @tparam rank The rank of the grid
   * @tparam AllocationPolicy The allocation policy
   */
  template<typename T, size_t rank, template<typename, size_t> class AllocationPolicy>
  class SingleArrayGridFortranOrderStorageBase : public SingleArrayGridStorageBase<T, rank, AllocationPolicy> {
    private:
      /// A pointer offset to the origin for faster access
      T *data_fast;

    public:
      /// Base class type
      typedef SingleArrayGridStorageBase<T, rank, AllocationPolicy> BaseType;

      /// The grid index type
      typedef typename BaseType::IndexType IndexType;

      /// The grid index type
      typedef typename BaseType::RangeType RangeType;

      /// Default constructor
      SingleArrayGridFortranOrderStorageBase();

      /// Copy constructor
      SingleArrayGridFortranOrderStorageBase(const SingleArrayGridFortranOrderStorageBase &);

      /**
       * @brief Construct with a given size
       *
       * @param lo the lowest coordinate in the grid (inclusive)
       * @param lo the highest coordinate in the grid (inclusive)
       */
      SingleArrayGridFortranOrderStorageBase(const IndexType &lo, const IndexType &hi);

      /**
       * @brief Construct with a given size
       *
       * @param range the lowest and highest coordinates in the grid (inclusive)
       */
      SingleArrayGridFortranOrderStorageBase(const RangeType &range);

      /**
       * @brief Assignment operator
       */
      SingleArrayGridFortranOrderStorageBase<T, rank, AllocationPolicy> &
      operator=(const SingleArrayGridFortranOrderStorageBase<T, rank, AllocationPolicy> &) = default;

      /**
       * @brief Get the lvalue at a given grid index
       *
       * @param index The grid index
       * @return the lvalue at the grid index
       */
      SCHNEK_INLINE T &get(const IndexType &index);

      /**
       * @brief Get the rvalue at a given grid index
       *
       * @param index The grid index
       * @return the rvalue at the grid index
       */
      SCHNEK_INLINE const T &get(const IndexType &index) const;

      /**
       * @brief resizes to grid with lower indices low[0],...,low[rank-1]
       * and upper indices high[0],...,high[rank-1]
       */
      void resize(const IndexType &low, const IndexType &high);

      /**
       * @brief resizes to grid with the range.
       * The endponts of the range are inclusive
       */
      void resize(const RangeType &range);

      /**
       * @brief returns the stride of the specified dimension
       */
      ptrdiff_t stride(size_t dim) const;

    private:
      void updateDataFast(const RangeType &range);
  };

  //=================================================================
  //================== SingleArrayGridStorageBase ===================
  //=================================================================

  template<typename T, size_t Rank, template<typename, size_t> class ...Policies>
  SingleArrayGridStorageBase<T, Rank, Policies...>::SingleArrayGridStorageBase()
      : size{0}, range{IndexType{0}, IndexType{0}}, dims{IndexType{0}} {
    this->allocation.onUpdate([this](const RangeType &range) { updateSize(range); });
  }

  template<typename T, size_t Rank, template<typename, size_t> class ...Policies>
  SingleArrayGridStorageBase<T, Rank, Policies...>::SingleArrayGridStorageBase(
      const SingleArrayGridStorageBase &other
  ) : size{other.size}, range{other.range}, dims{other.dims}, allocation{other.allocation}, layout{other.layout} {
    this->allocation.onUpdate([this](const RangeType &range) { updateSize(range); });
  }

  template<typename T, size_t Rank, template<typename, size_t> class ...Policies>
  SingleArrayGridStorageBase<T, Rank, Policies...>::SingleArrayGridStorageBase(
      const IndexType &lo, const IndexType &hi
  ) : size{0}, range{IndexType{0}, IndexType{0}}, dims{IndexType{0}} {
    this->allocation.onUpdate([this](const RangeType &range) { updateSize(range); });
    resize(lo, hi);
  }

  template<typename T, size_t Rank, template<typename, size_t> class ...Policies>
  SingleArrayGridStorageBase<T, Rank, Policies...>::SingleArrayGridStorageBase(const RangeType &range)
      : size{0}, range{IndexType{0}, IndexType{0}}, dims{IndexType{0}} {
    this->allocation.onUpdate([this](const RangeType &range) { updateSize(range); });
    resize(range.getLo(), range.getHi());
  }

  template<typename T, size_t Rank, template<typename, size_t> class ...Policies>
  SingleArrayGridStorageBase<T, Rank, Policies...> &
  SingleArrayGridStorageBase<T, Rank, Policies...>::operator=(const SingleArrayGridStorageBase<T, Rank, Policies...> &other) {
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

  //=================================================================
  //=============== SingleArrayGridCOrderStorageBase ================
  //=================================================================

  template<typename T, size_t rank, template<typename, size_t> class AllocationPolicy>
  SingleArrayGridCOrderStorageBase<T, rank, AllocationPolicy>::SingleArrayGridCOrderStorageBase()
      : BaseType(), data_fast(NULL) {
    this->allocation.onUpdate([this](const RangeType &range) { updateDataFast(range); });
  }

  template<typename T, size_t rank, template<typename, size_t> class AllocationPolicy>
  SingleArrayGridCOrderStorageBase<T, rank, AllocationPolicy>::SingleArrayGridCOrderStorageBase(
      const SingleArrayGridCOrderStorageBase &other
  )
      : BaseType(other), data_fast(other.data_fast) {
    this->allocation.onUpdate([this](const RangeType &range) { updateDataFast(range); });
  }

  template<typename T, size_t rank, template<typename, size_t> class AllocationPolicy>
  SingleArrayGridCOrderStorageBase<T, rank, AllocationPolicy>::SingleArrayGridCOrderStorageBase(
      const IndexType &lo, const IndexType &hi
  )
      : BaseType(), data_fast(NULL) {
    this->allocation.onUpdate([this](const RangeType &range) { updateDataFast(range); });
    resize(lo, hi);
  }

  template<typename T, size_t rank, template<typename, size_t> class AllocationPolicy>
  SingleArrayGridCOrderStorageBase<T, rank, AllocationPolicy>::SingleArrayGridCOrderStorageBase(const RangeType &range)
      : BaseType(), data_fast(NULL) {
    this->allocation.onUpdate([this](const RangeType &range) { updateDataFast(range); });
    resize(range.getLo(), range.getHi());
  }

  template<typename T, size_t rank, template<typename, size_t> class AllocationPolicy>
  SCHNEK_INLINE T &SingleArrayGridCOrderStorageBase<T, rank, AllocationPolicy>::get(const IndexType &index) {
    size_t pos = index[0];
    for (size_t i = 1; i < rank; ++i) {
      pos = index[i] + this->dims[i] * pos;
    }
    return this->data_fast[pos];
  }

  template<typename T, size_t rank, template<typename, size_t> class AllocationPolicy>
  SCHNEK_INLINE const T &SingleArrayGridCOrderStorageBase<T, rank, AllocationPolicy>::get(const IndexType &index
  ) const {
    size_t pos = index[0];
    for (size_t i = 1; i < rank; ++i) {
      pos = index[i] + this->dims[i] * pos;
    }
    return this->data_fast[pos];
  }

  template<typename T, size_t rank, template<typename, size_t> class AllocationPolicy>
  inline ptrdiff_t SingleArrayGridCOrderStorageBase<T, rank, AllocationPolicy>::stride(size_t dim) const {
    size_t stride = 1;
    for (size_t i = rank - 1; i > dim; --i) {
      stride *= this->dims[i];
    }
    return stride;
  }

  template<typename T, size_t rank, template<typename, size_t> class AllocationPolicy>
  void SingleArrayGridCOrderStorageBase<T, rank, AllocationPolicy>::updateDataFast(const RangeType &range) {
    this->range = range;
    this->dims = range.getHi() - range.getLo() + IndexType{1};
    ptrdiff_t p = -range.getLo(0);
    this->size = this->dims[0];

    for (size_t d = 1; d < rank; ++d) {
      p = p * this->dims[d] - range.getLo(d);
      this->size *= this->dims[d];
    }
    data_fast = this->allocation.getData() + p;
  }

  //=================================================================
  //============ SingleArrayGridFortranOrderStorageBase =============
  //=================================================================

  template<typename T, size_t rank, template<typename, size_t> class AllocationPolicy>
  SingleArrayGridFortranOrderStorageBase<T, rank, AllocationPolicy>::SingleArrayGridFortranOrderStorageBase()
      : BaseType(), data_fast(NULL) {
    this->allocation.onUpdate([this](const RangeType &range) { updateDataFast(range); });
  }

  template<typename T, size_t rank, template<typename, size_t> class AllocationPolicy>
  SingleArrayGridFortranOrderStorageBase<T, rank, AllocationPolicy>::SingleArrayGridFortranOrderStorageBase(
      const SingleArrayGridFortranOrderStorageBase &other
  )
      : BaseType(other), data_fast(other.data_fast) {
    this->allocation.onUpdate([this](const RangeType &range) { updateDataFast(range); });
  }

  template<typename T, size_t rank, template<typename, size_t> class AllocationPolicy>
  SingleArrayGridFortranOrderStorageBase<T, rank, AllocationPolicy>::SingleArrayGridFortranOrderStorageBase(
      const IndexType &lo, const IndexType &hi
  )
      : BaseType(), data_fast(NULL) {
    this->allocation.onUpdate([this](const RangeType &range) { updateDataFast(range); });
    resize(lo, hi);
  }

  template<typename T, size_t rank, template<typename, size_t> class AllocationPolicy>
  SingleArrayGridFortranOrderStorageBase<T, rank, AllocationPolicy>::SingleArrayGridFortranOrderStorageBase(
      const RangeType &range
  )
      : BaseType(), data_fast(NULL) {
    this->allocation.onUpdate([this](const RangeType &range) { updateDataFast(range); });
    resize(range.getLo(), range.getHi());
  }

  template<typename T, size_t rank, template<typename, size_t> class AllocationPolicy>
  SCHNEK_INLINE T &SingleArrayGridFortranOrderStorageBase<T, rank, AllocationPolicy>::get(const IndexType &index) {
    size_t pos = index[rank - 1];
    for (ptrdiff_t i = ptrdiff_t(rank) - 2; i >= 0; --i) {
      pos = index[i] + this->dims[i] * pos;
    }
    return this->data_fast[pos];
  }

  template<typename T, size_t rank, template<typename, size_t> class AllocationPolicy>
  SCHNEK_INLINE const T &SingleArrayGridFortranOrderStorageBase<T, rank, AllocationPolicy>::get(const IndexType &index
  ) const {
    size_t pos = index[rank - 1];
    for (ptrdiff_t i = ptrdiff_t(rank) - 2; i >= 0; --i) {
      pos = index[i] + this->dims[i] * pos;
    }
    return this->data_fast[pos];
  }

  template<typename T, size_t rank, template<typename, size_t> class AllocationPolicy>
  inline void SingleArrayGridFortranOrderStorageBase<T, rank, AllocationPolicy>::resize(
      const IndexType &lo, const IndexType &hi
  ) {
    this->allocation.resizeImpl(lo, hi);
  }

  template<typename T, size_t rank, template<typename, size_t> class AllocationPolicy>
  inline void SingleArrayGridFortranOrderStorageBase<T, rank, AllocationPolicy>::resize(const RangeType &range) {
    this->resize(range.getLo(), range.getHi());
  }

  template<typename T, size_t rank, template<typename, size_t> class AllocationPolicy>
  inline ptrdiff_t SingleArrayGridFortranOrderStorageBase<T, rank, AllocationPolicy>::stride(size_t dim) const {
    ptrdiff_t stride = 1;
    for (size_t i = 0; i < dim; ++i) {
      stride *= this->dims[i];
    }
    return stride;
  }

  template<typename T, size_t rank, template<typename, size_t> class AllocationPolicy>
  void SingleArrayGridFortranOrderStorageBase<T, rank, AllocationPolicy>::updateDataFast(const RangeType &range) {
    this->range = range;
    this->dims = range.getHi() - range.getLo() + IndexType{1};
    size_t p = -range.getLo(rank - 1);
    this->size = this->dims[rank - 1];

    for (ptrdiff_t d = ptrdiff_t(rank) - 2; d >= 0; --d) {
      p = p * this->dims[d] - range.getLo(d);
      this->size *= this->dims[d];
    }
    data_fast = this->allocation.getData() + p;
  }

}  // namespace schnek

#endif  // SCHNEK_GRID_GRIDSTORAGE_SINGLESTORAGEBASE_HPP_
