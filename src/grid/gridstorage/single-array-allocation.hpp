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

#include "../array.hpp"

#include <memory>
#include <cmath>

/**
 * @page Grid Allocation Policies
 * 
 * Allocation policies are responsible for managing the underlying data of a 
 * grid.
 * 
 */

namespace schnek
{

    namespace internal {
        template <typename T>
        struct SingleArrayAllocationData
        {
            T *ptr;
            SingleArrayAllocationData(): ptr(NULL) {}
            ~SingleArrayAllocationData() {
                delete[] ptr;
            }
        };
    }

    /**
     * @brief Allocate a single array for multidimensional grids in C ordering.
     *
     * Deallocation and allocation is performed on every resize.
     *
     * @tparam T The type of data stored in the grid
     * @tparam rank The rank of the grid
     */
    template <typename T, size_t rank>
    class SingleArrayInstantAllocation
    {
    public:
        /// The grid index type
        typedef Array<int, rank> IndexType;

    protected:
        /// The pointer to the data
        std::shared_ptr<internal::SingleArrayAllocationData<T> > data;

        /// The length of the allocated array
        size_t size;

        /// The lowest coordinate in the grid (inclusive)
        IndexType low;

        /// The highest coordinate in the grid (inclusive)
        IndexType high;

        /// The dimensions of the grid `dims = high - low + 1`
        IndexType dims;

    public:
        /**
         * @brief Default constructor
         */
        SingleArrayInstantAllocation()
            : data(new internal::SingleArrayAllocationData<T>()), size(0) {}

        /**
         * @brief Copy constructor
         */
        SingleArrayInstantAllocation(const SingleArrayInstantAllocation<T, rank> &) = default;

        /**
         * @brief Assignment operator
         */
        SingleArrayInstantAllocation<T, rank> &operator=(const SingleArrayInstantAllocation<T, rank> &) = default;
    protected:
        /**
         * @brief resizes to grid with lower indices low[0],...,low[rank-1]
         * and upper indices high[0],...,high[rank-1]
         */
        void resizeImpl(const IndexType &low, const IndexType &high);
    private:
        /// Free the allocated memory
        void deleteData();

        /// Allocate a new array
        void newData(const IndexType &low_, const IndexType &high_);
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
     * @tparam rank The rank of the grid
     */
    template <typename T, size_t rank>
    class SingleArrayLazyAllocation
    {
    public:
        /// The grid index type
        typedef Array<int, rank> IndexType;

    protected:
        /// The pointer to the data
        std::shared_ptr<internal::SingleArrayAllocationData<T> > data;

        /// The length of the array
        int size;

        /// The lowest coordinate in the grid (inclusive)
        IndexType low;

        /// The highest coordinate in the grid (inclusive)
        IndexType high;

        /// The dimensions of the grid `dims = high - low + 1`
        IndexType dims;

    private:
        /// The size allocated memory
        int bufSize;

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
        SingleArrayLazyAllocation(const SingleArrayLazyAllocation &) = default;

        /**
         * @brief Assignment operator
         */
        SingleArrayLazyAllocation<T, rank> &operator=(const SingleArrayLazyAllocation<T, rank> &) = default;
    protected:
        /**
         * @brief resizes to grid with lower indices low[0],...,low[rank-1]
         * and upper indices high[0],...,high[rank-1]
         */
        void resizeImpl(const IndexType &low_, const IndexType &high_);

    private:
        /// Free the allocated memory
        void deleteData();

        /// Allocate a new array
        void newData(int size);
    };

    //=================================================================
    //=============== SingleArrayInstantAllocation ====================
    //=================================================================

    template <typename T, size_t rank>
    void SingleArrayInstantAllocation<T, rank>::resizeImpl(const IndexType &low_, const IndexType &high_)
    {
        this->deleteData();
        this->newData(low_, high_);
    }

    template <typename T, size_t rank>
    void SingleArrayInstantAllocation<T, rank>::deleteData()
    {
        if (data->ptr)
        {
            delete[] data->ptr;
        }
        data->ptr = NULL;
        size = 0;
    }

    template <typename T, size_t rank>
    void SingleArrayInstantAllocation<T, rank>::newData(
        const IndexType &low_,
        const IndexType &high_)
    {
        size = 1;

        low = low_;
        high = high_;

        for (size_t d = 0; d < rank; ++d)
        {
            dims[d] = high[d] - low[d] + 1;
            size *= dims[d];
        }
        data->ptr = new T[size];
    }

    //=================================================================
    //================== SingleArrayLazyAllocation ====================
    //=================================================================

    template <typename T, size_t rank>
    SingleArrayLazyAllocation<T, rank>::SingleArrayLazyAllocation()
        : data(new internal::SingleArrayAllocationData<T>()), size(0), bufSize(0), avgSize(0.0), avgVar(0.0), r(0.05)
    {
    }

    template <typename T, size_t rank>
    void SingleArrayLazyAllocation<T, rank>::resizeImpl(const IndexType &low_, const IndexType &high_)
    {
        int newSize = 1;

        low = low_;
        high = high_;

        for (size_t d = 0; d < rank; d++)
        {
            dims[d] = high[d] - low[d] + 1;
            newSize *= dims[d];
        }

        avgSize = r * newSize + (1 - r) * avgSize;
        int diff = newSize - avgSize;
        avgVar = r * diff * diff + (1 - r) * avgVar;

        if ((newSize > bufSize) || (((newSize + 32.0 * sqrt(avgVar)) < bufSize) && (bufSize > 100)))
        {
            this->deleteData();
            this->newData(newSize);
        }
        size = newSize;
    }

    template <typename T, size_t rank>
    void SingleArrayLazyAllocation<T, rank>::deleteData()
    {
        SCHNEK_TRACE_LOG(5, "Deleting pointer (" << (void *)data << "): size=" << size << " avgSize=" << avgSize << " avgVar=" << avgVar << " bufSize=" << bufSize);
        if (data->ptr)
        {
            delete[] data->ptr;
        }
        data->ptr = NULL;
        size = 0;
        bufSize = 0;
    }

    template <typename T, size_t rank>
    void SingleArrayLazyAllocation<T, rank>::newData(
        int newSize)
    {
        bufSize = newSize + (int)(4 * sqrt(avgVar));
        if (bufSize <= 0)
            bufSize = 10;
        // std::cerr << "Allocating pointer: size = " << newSize  << " " << bufSize << std::endl;
        data->ptr = new T[bufSize];
    }
}

#endif // SCHNEK_GRID_GRIDSTORAGE_SINGLEARRAYALLOCATION_HPP_
