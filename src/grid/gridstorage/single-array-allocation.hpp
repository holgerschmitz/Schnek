/*
 * single-array-allocation.hpp
 *
 * Created on: 06 Nov 2022
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

#ifndef SCHNEK_GRID_GRIDSTORAGE_SINGLEARRAYALLOCATION_HPP_
#define SCHNEK_GRID_GRIDSTORAGE_SINGLEARRAYALLOCATION_HPP_

#include <cmath>
#include <functional>
#include <map>
#include <memory>

#include "../../macros.hpp"
#include "../../util/logger.hpp"
#include "../array.hpp"
#include "../range.hpp"

/**
 * @page Grid Allocation Policies
 *
 * Allocation policies are responsible for managing the underlying data of a
 * grid.
 *
 */

namespace schnek {

  namespace internal {
    /**
     * @brief The data for a single array allocation
     *
     * This class is used to store the data for a single array allocation. It
     * stores the pointer to the data and a list of updaters. The updaters should
     * be called when the data is resized.
     */
    template<typename T, typename SizeInfo>
    class SingleArrayAllocationData {
      public:
        typedef std::function<void(const SizeInfo &)> UpdaterType;

      private:
        std::map<void *, UpdaterType> updaters;

      public:
        T *ptr;
        SingleArrayAllocationData() : ptr(NULL) {}
        ~SingleArrayAllocationData() { delete[] ptr; }

        void addUpdater(void *key, const UpdaterType &updater) { updaters[key] = updater; }

        void removeUpdater(void *key) { updaters.erase(key); }

        void update(const SizeInfo &sizeInfo) {
          for (auto &updater : updaters) {
            updater.second(sizeInfo);
          }
        }
    };
  }  // namespace internal

  /**
   * @brief Allocate a single array for multidimensional grids in C ordering.
   *
   * Deallocation and allocation is performed on every resize.
   *
   * @tparam T The type of data stored in the grid
   * @tparam rank The rank of the grid
   */
  template<typename T, size_t Rank>
  class SingleArrayInstantAllocation {
    public:
      typedef T value_type;
      static constexpr size_t rank = Rank;

      /// The grid index type
      typedef Array<int, Rank> IndexType;

      /// The grid range type
      typedef Range<int, Rank> RangeType;
    private:
      struct SizeInfo {
          IndexType lo;
          IndexType hi;
      };

      /// The pointer to the data
      std::shared_ptr<internal::SingleArrayAllocationData<T, SizeInfo> > data;
    public:
      typedef std::function<void(const RangeType &)> UpdaterType;

    public:
      /**
       * @brief Default constructor
       */
      SingleArrayInstantAllocation();

      /**
       * @brief Copy constructor
       */
      SingleArrayInstantAllocation(const SingleArrayInstantAllocation<T, Rank> &);

      /**
       * @brief Assignment operator
       */
      SingleArrayInstantAllocation<T, Rank> &operator=(const SingleArrayInstantAllocation<T, Rank> &);

      /**
       * @brief destructor
       */
      ~SingleArrayInstantAllocation();

    public:
      /**
       * @brief resizes to grid with lower indices lo[0],...,lo[rank-1]
       * and upper indices hi[0],...,hi[rank-1]
       */
      void resizeImpl(const IndexType &lo, const IndexType &hi);

      /**
       * @brief Add an updater to the data
       *
       * The updater is called when the data is resized.
       */
      void onUpdate(const UpdaterType &updater);

      /**
       * @brief Get the pointer to the data
       */
      T* getData();

    private:
      UpdaterType updater;

      /**
       * @brief Update the size information
       */
      void updateSizeInfo(const SizeInfo &sizeInfo);

      /// Free the allocated memory
      void deleteData();

      /// Allocate a new array
      void newData(const IndexType &lo, const IndexType &hi);
  };

  /**
   * @brief Allocate a single array for multidimensional grids in C ordering.
   *
   * Deallocation and allocation is performed only when required. When the memory need
   * grows, the storage will allocate slightly more memory that needed. Shrinking of
   * the allocated space happens lazily.
   *
   * Use this, when a resize is expected regularly.
   *
   * @tparam T The type of data stored in the grid
   * @tparam Rank The rank of the grid
   */
  template<typename T, size_t Rank>
  class SingleArrayLazyAllocation {
    public:
      typedef T value_type;
      static constexpr size_t rank = Rank;

      /// The grid index type
      typedef Array<size_t, Rank> IndexType;

      /// The grid range type
      typedef Range<size_t, Rank> RangeType;

    private:
      struct SizeInfo {
          IndexType lo;
          IndexType hi;
          size_t size;
          size_t bufSize;
          double avgSize;
          double avgVar;
      };

      /// The pointer to the data
      std::shared_ptr<internal::SingleArrayAllocationData<T, SizeInfo> > data;
    public:
      typedef std::function<void(const RangeType &)> UpdaterType;
    private:
      /// The size allocated memory
      size_t bufSize;

      /// The running average of the required array size
      double avgSize;

      /// The running variance of the required array size
      double avgVar;

      /// The factor for calculating the running average
      double r;

    public:
      /// Default constructor
      SingleArrayLazyAllocation();

      /// Copy constructor
      SingleArrayLazyAllocation(const SingleArrayLazyAllocation &);

      /**
       * @brief Assignment operator
       */
      SingleArrayLazyAllocation<T, Rank> &operator=(const SingleArrayLazyAllocation<T, Rank> &);

      /**
       * @brief destructor
       */
      ~SingleArrayLazyAllocation();

    public:
      /**
       * @brief resizes to grid with lower indices lo[0],...,lo[rank-1]
       * and upper indices hi[0],...,hi[rank-1]
       */
      void resizeImpl(const IndexType &lo, const IndexType &hi);

      /**
       * @brief Add an updater to the data
       *
       * The updater is called when the data is resized.
       */
      void onUpdate(const UpdaterType &updater);

      /**
       * @brief Get the pointer to the data
       */
      T* getData();

    private:
      UpdaterType updater;

      /**
       * @brief Update the size information
       *
       * This function must called by the updater from the derived class.
       */
      void updateSizeInfo(const SizeInfo &sizeInfo);

      /// Free the allocated memory
      void deleteData();

      /// Allocate a new array
      void newData(size_t size);
  };

  //=================================================================
  //=============== SingleArrayInstantAllocation ====================
  //=================================================================

  template<typename T, size_t Rank>
  SingleArrayInstantAllocation<T, Rank>::SingleArrayInstantAllocation()
      : data(new internal::SingleArrayAllocationData<T, SizeInfo>()) {
    this->data->addUpdater(this, [this](const SizeInfo &sizeInfo) { this->updateSizeInfo(sizeInfo); });
  }

  template<typename T, size_t Rank>
  SingleArrayInstantAllocation<T, Rank>::SingleArrayInstantAllocation(const SingleArrayInstantAllocation<T, Rank> &other
  )
      : data(other.data) {
    this->data->addUpdater(this, [this](const SizeInfo &sizeInfo) { this->updateSizeInfo(sizeInfo); });
  };

  template<typename T, size_t Rank>
  SingleArrayInstantAllocation<T, Rank> &SingleArrayInstantAllocation<T, Rank>::operator=(
      const SingleArrayInstantAllocation<T, Rank> &other
  ) {
    this->data->removeUpdater(this);
    
    this->data = other.data;
    this->data->addUpdater(this, [this](const SizeInfo &sizeInfo) { this->updateSizeInfo(sizeInfo); });
    return *this;
  };

  template<typename T, size_t Rank>
  SingleArrayInstantAllocation<T, Rank>::~SingleArrayInstantAllocation() {
    this->data->removeUpdater(this);
  }

  template<typename T, size_t Rank>
  SCHNEK_INLINE void SingleArrayInstantAllocation<T, Rank>::onUpdate(const UpdaterType &updater) {
    this->updater = updater;
  }

  template<typename T, size_t Rank>
  T* SingleArrayInstantAllocation<T, Rank>::getData() {
    return this->data->ptr;
  }

  template<typename T, size_t Rank>
  void SingleArrayInstantAllocation<T, Rank>::updateSizeInfo(const SizeInfo &sizeInfo) {
    if (updater) {
      updater(RangeType{sizeInfo.lo, sizeInfo.hi});
    }
  }

  template<typename T, size_t Rank>
  void SingleArrayInstantAllocation<T, Rank>::resizeImpl(const IndexType &lo, const IndexType &hi) {
    this->deleteData();
    this->newData(lo, hi);
    data->update(SizeInfo{lo, hi});
  }

  template<typename T, size_t Rank>
  void SingleArrayInstantAllocation<T, Rank>::deleteData() {
    if (data->ptr) {
      delete[] data->ptr;
    }
    data->ptr = NULL;
  }

  template<typename T, size_t Rank>
  void SingleArrayInstantAllocation<T, Rank>::newData(const IndexType &lo, const IndexType &hi) {
    size_t size = 1;

    for (size_t d = 0; d < Rank; ++d) {
      size *= hi[d] - lo[d] + 1;
    }
    
    data->ptr = new T[size];
  }

  //=================================================================
  //================== SingleArrayLazyAllocation ====================
  //=================================================================

  template<typename T, size_t Rank>
  SingleArrayLazyAllocation<T, Rank>::SingleArrayLazyAllocation()
      : data(new internal::SingleArrayAllocationData<T, SizeInfo>()),
        bufSize(0),
        avgSize(0.0),
        avgVar(0.0),
        r(0.05) {
    this->data->addUpdater(this, [this](const SizeInfo &sizeInfo) { this->updateSizeInfo(sizeInfo); });
  }

  template<typename T, size_t Rank>
  SingleArrayLazyAllocation<T, Rank>::SingleArrayLazyAllocation(const SingleArrayLazyAllocation<T, Rank> &other)
      : data(other.data),
        bufSize(other.bufSize),
        avgSize(other.avgSize),
        avgVar(other.avgVar),
        r(other.r) {
    this->data->addUpdater(this, [this](const SizeInfo &sizeInfo) { this->updateSizeInfo(sizeInfo); });
  };

  template<typename T, size_t Rank>
  SingleArrayLazyAllocation<T, Rank> &SingleArrayLazyAllocation<T, Rank>::operator=(
      const SingleArrayLazyAllocation<T, Rank> &other
  ) {
    this->data = other.data;
    this->data->addUpdater(this, [this](const SizeInfo &sizeInfo) { this->updateSizeInfo(sizeInfo); });
    return *this;
  };

  template<typename T, size_t Rank>
  SingleArrayLazyAllocation<T, Rank>::~SingleArrayLazyAllocation() {
    this->data->removeUpdater(this);
  }

  template<typename T, size_t Rank>
  void SingleArrayLazyAllocation<T, Rank>::onUpdate(const UpdaterType &updater) {
    this->updater = updater;
  }

  template<typename T, size_t Rank>
  SCHNEK_INLINE T* SingleArrayLazyAllocation<T, Rank>::getData() {
    return this->data->ptr;
  }

  template<typename T, size_t Rank>
  void SingleArrayLazyAllocation<T, Rank>::updateSizeInfo(const SizeInfo &sizeInfo) {
    bufSize = sizeInfo.bufSize;
    avgSize = sizeInfo.avgSize;
    avgVar = sizeInfo.avgVar;

    if (updater) {
      updater(RangeType{sizeInfo.lo, sizeInfo.hi});
    }
  }

  template<typename T, size_t Rank>
  void SingleArrayLazyAllocation<T, Rank>::resizeImpl(const IndexType &lo, const IndexType &hi) {
    size_t newSize = 1;

    for (size_t d = 0; d < Rank; d++) {
      newSize *= hi[d] - lo[d] + 1;
    }

    avgSize = r * newSize + (1 - r) * avgSize;
    ptrdiff_t diff = newSize - avgSize;
    avgVar = r * diff * diff + (1 - r) * avgVar;

    if ((newSize > bufSize) || (((newSize + 32.0 * sqrt(avgVar)) < bufSize) && (bufSize > 100))) {
      this->deleteData();
      this->newData(newSize);
    }
    
    this->data->update(SizeInfo{lo, hi, newSize, bufSize, avgSize, avgVar});
  }

  template<typename T, size_t Rank>
  void SingleArrayLazyAllocation<T, Rank>::deleteData() {
    SCHNEK_TRACE_LOG(
        5, "Deleting pointer (" << (void *)data << "): avgSize=" << avgSize << " avgVar=" << avgVar
                                << " bufSize=" << bufSize
    );
    if (data->ptr) {
      delete[] data->ptr;
    }
    data->ptr = NULL;
    bufSize = 0;
  }

  template<typename T, size_t Rank>
  void SingleArrayLazyAllocation<T, Rank>::newData(size_t newSize) {
    bufSize = newSize + (size_t)(4 * sqrt(avgVar));
    if (bufSize <= 0) {
      bufSize = 10;
    }
    data->ptr = new T[bufSize];
  }
}  // namespace schnek

#endif  // SCHNEK_GRID_GRIDSTORAGE_SINGLEARRAYALLOCATION_HPP_
