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

namespace schnek
{
    /**
     * @brief The storage base extends from an allocation policy and adds some accessor methods
     * 
     * @tparam T The type of data stored in the grid
     * @tparam rank The rank of the grid
     * @tparam AllocationPolicy The allocation policy
     */
    template <typename T, size_t rank, template <typename, size_t> class AllocationPolicy>
    class SingleArrayGridStorageBase : public AllocationPolicy<T, rank>
    {
    public:
        /// The grid index type
        typedef Array<int, rank> IndexType;

    public:
        /// Default constructor
        SingleArrayGridStorageBase();

        /**
         * @brief Construct with a given size
         * 
         * @param lo the lowest coordinate in the grid (inclusive)
         * @param lo the highest coordinate in the grid (inclusive)
         */
        SingleArrayGridStorageBase(const IndexType &lo, const IndexType &hi);

        /// Access to the underlying raw data
        T *getRawData() const { return this->data; }

        /// Get the lowest coordinate in the grid (inclusive)
        const IndexType &getLo() const { return this->low; }

        /// Get the highest coordinate in the grid (inclusive)
        const IndexType &getHi() const { return this->high; }

        /// Get the dimensions of the grid `dims = high - low + 1`
        const IndexType &getDims() const { return this->dims; }

        /// Get k-th component of the lowest coordinate in the grid (inclusive)
        int getLo(int k) const { return this->low[k]; }

        /// Get k-th component of the highest coordinate in the grid (inclusive)
        int getHi(int k) const { return this->high[k]; }

        /// Get k-th component of the dimensions of the grid `dims = high - low + 1`
        int getDims(int k) const { return this->dims[k]; }

        /// Get the length of the allocated array
        int getSize() const { return this->size; }
    };

    /**
     * @brief Extends from SingleArrayGridStorageBase to provide C-order indexing over the
     * 1-dimensional data array for a multidimensional grid.
     * 
     * @tparam T The type of data stored in the grid
     * @tparam rank The rank of the grid
     * @tparam AllocationPolicy The allocation policy
     */
    template <typename T, size_t rank, template <typename, size_t> class AllocationPolicy>
    class SingleArrayGridCOrderStorageBase : public SingleArrayGridStorageBase<T, rank, AllocationPolicy>
    {
    public:
        /// Base class type
        typedef SingleArrayGridStorageBase<T, rank, AllocationPolicy> BaseType;

        /// The grid index type
        typedef typename BaseType::IndexType IndexType;

        /// Default constructor
        SingleArrayGridCOrderStorageBase() : BaseType() {}

        /**
         * @brief Construct with a given size
         * 
         * @param lo the lowest coordinate in the grid (inclusive)
         * @param lo the highest coordinate in the grid (inclusive)
         */
        SingleArrayGridCOrderStorageBase(const IndexType &low_, const IndexType &high_)
            : BaseType(low_, high_) {}

        /**
         * @brief Get the lvalue at a given grid index
         * 
         * @param index The grid index
         * @return the lvalue at the grid index
         */
        T &get(const IndexType &index);

        /**
         * @brief Get the rvalue at a given grid index
         * 
         * @param index The grid index
         * @return the rvalue at the grid index
         */
        const T &get(const IndexType &index) const;
    };

    /**
     * @brief Extends from SingleArrayGridStorageBase to provide Fortran-order indexing over the
     * 1-dimensional data array for a multidimensional grid.
     * 
     * @tparam T The type of data stored in the grid
     * @tparam rank The rank of the grid
     * @tparam AllocationPolicy The allocation policy
     */
    template <typename T, size_t rank, template <typename, size_t> class AllocationPolicy>
    class SingleArrayGridFortranOrderStorageBase : public SingleArrayGridStorageBase<T, rank, AllocationPolicy>
    {
    public:
        typedef SingleArrayGridStorageBase<T, rank, AllocationPolicy> BaseType;
        typedef typename BaseType::IndexType IndexType;

        /// Default constructor
        SingleArrayGridFortranOrderStorageBase() : BaseType() {}

        /**
         * @brief Construct with a given size
         * 
         * @param lo the lowest coordinate in the grid (inclusive)
         * @param lo the highest coordinate in the grid (inclusive)
         */
        SingleArrayGridFortranOrderStorageBase(const IndexType &low_, const IndexType &high_)
            : BaseType(low_, high_) {}

        /**
         * @brief Get the lvalue at a given grid index
         * 
         * @param index The grid index
         * @return the lvalue at the grid index
         */
        T &get(const IndexType &index);

        /**
         * @brief Get the rvalue at a given grid index
         * 
         * @param index The grid index
         * @return the rvalue at the grid index
         */
        const T &get(const IndexType &index) const;
    };

    //=================================================================
    //================== SingleArrayGridStorageBase ===================
    //=================================================================

    template <typename T, size_t rank, template <typename, size_t> class AllocationPolicy>
    SingleArrayGridStorageBase<T, rank, AllocationPolicy>::SingleArrayGridStorageBase()
        : AllocationPolicy<T, rank>()
    {
    }

    template <typename T, size_t rank, template <typename, size_t> class AllocationPolicy>
    SingleArrayGridStorageBase<T, rank, AllocationPolicy>::SingleArrayGridStorageBase(
        const IndexType &low_,
        const IndexType &high_)
        : AllocationPolicy<T, rank>()
    {
        this->resize(low_, high_);
    }

    //=================================================================
    //=============== SingleArrayGridCOrderStorageBase ================
    //=================================================================

    template <typename T, size_t rank, template <typename, size_t> class AllocationPolicy>
    inline T &SingleArrayGridCOrderStorageBase<T, rank, AllocationPolicy>::get(const IndexType &index)
    {
        int pos = index[0];
        for (int i = 1; i < rank; ++i)
        {
            pos = index[i] + this->dims[i] * pos;
        }
        return this->data_fast[pos];
    }

    template <typename T, size_t rank, template <typename, size_t> class AllocationPolicy>
    inline const T &SingleArrayGridCOrderStorageBase<T, rank, AllocationPolicy>::get(const IndexType &index) const
    {
        int pos = index[0];
        for (int i = 1; i < rank; ++i)
        {
            pos = index[i] + this->dims[i] * pos;
        }
        return this->data_fast[pos];
    }

    //=================================================================
    //============ SingleArrayGridFortranOrderStorageBase =============
    //=================================================================

    template <typename T, size_t rank, template <typename, size_t> class AllocationPolicy>
    inline T &SingleArrayGridFortranOrderStorageBase<T, rank, AllocationPolicy>::get(const IndexType &index)
    {
        int pos = index[rank - 1];
        for (int i = rank - 2; i >= 0; --i)
        {
            pos = index[i] + this->dims[i] * pos;
        }
        return this->data_fast[pos];
    }

    template <typename T, size_t rank, template <typename, size_t> class AllocationPolicy>
    inline const T &SingleArrayGridFortranOrderStorageBase<T, rank, AllocationPolicy>::get(const IndexType &index) const
    {
        int pos = index[rank - 1];
        for (int i = rank - 2; i >= 0; --i)
        {
            pos = index[i] + this->dims[i] * pos;
        }
        return this->data_fast[pos];
    }

}

#endif // SCHNEK_GRID_GRIDSTORAGE_SINGLESTORAGEBASE_HPP_
