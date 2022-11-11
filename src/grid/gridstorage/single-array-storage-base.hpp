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
     * Stores the grid data in a single array
     */
    template <typename T, int rank, template <typename, int> class AllocationPolicy>
    class SingleArrayGridStorageBase : public AllocationPolicy<T, rank>
    {
    public:
        typedef Array<int, rank> IndexType;

    public:

        SingleArrayGridStorageBase();

        SingleArrayGridStorageBase(const IndexType &low_, const IndexType &high_);

        T *getRawData() const { return this->data; }

        /** */
        const IndexType &getLo() const { return this->low; }
        /** */
        const IndexType &getHi() const { return this->high; }
        /** */
        const IndexType &getDims() const { return this->dims; }

        /** */
        int getLo(int k) const { return this->low[k]; }
        /** */
        int getHi(int k) const { return this->high[k]; }
        /** */
        int getDims(int k) const { return this->dims[k]; }

        int getSize() const { return this->size; }
    };

    template <typename T, int rank, template <typename, int> class AllocationPolicy>
    class SingleArrayGridCOrderStorageBase : public SingleArrayGridStorageBase<T, rank, AllocationPolicy>
    {
    public:
        typedef SingleArrayGridStorageBase<T, rank, AllocationPolicy> BaseType;
        typedef typename BaseType::IndexType IndexType;

        SingleArrayGridCOrderStorageBase() : BaseType() {}

        SingleArrayGridCOrderStorageBase(const IndexType &low_, const IndexType &high_)
            : BaseType(low_, high_) {}

        T &get(const IndexType &index);
        const T &get(const IndexType &index) const;
    };

    template <typename T, int rank, template <typename, int> class AllocationPolicy>
    class SingleArrayGridFortranOrderStorageBase : public SingleArrayGridStorageBase<T, rank, AllocationPolicy>
    {
    public:
        typedef SingleArrayGridStorageBase<T, rank, AllocationPolicy> BaseType;
        typedef typename BaseType::IndexType IndexType;

        SingleArrayGridFortranOrderStorageBase() : BaseType() {}

        SingleArrayGridFortranOrderStorageBase(const IndexType &low_, const IndexType &high_)
            : BaseType(low_, high_) {}

        T &get(const IndexType &index);
        const T &get(const IndexType &index) const;
    };

    //=================================================================
    //================== SingleArrayGridStorageBase ===================
    //=================================================================

    template <typename T, int rank, template <typename, int> class AllocationPolicy>
    SingleArrayGridStorageBase<T, rank, AllocationPolicy>::SingleArrayGridStorageBase()
        : AllocationPolicy<T, rank>()
    {
    }

    template <typename T, int rank, template <typename, int> class AllocationPolicy>
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

    template <typename T, int rank, template <typename, int> class AllocationPolicy>
    inline T &SingleArrayGridCOrderStorageBase<T, rank, AllocationPolicy>::get(const IndexType &index)
    {
        int pos = index[0];
        for (int i = 1; i < rank; ++i)
        {
            pos = index[i] + this->dims[i] * pos;
        }
        return this->data_fast[pos];
    }

    template <typename T, int rank, template <typename, int> class AllocationPolicy>
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

    template <typename T, int rank, template <typename, int> class AllocationPolicy>
    inline T &SingleArrayGridFortranOrderStorageBase<T, rank, AllocationPolicy>::get(const IndexType &index)
    {
        int pos = index[rank - 1];
        for (int i = rank - 2; i >= 0; --i)
        {
            pos = index[i] + this->dims[i] * pos;
        }
        return this->data_fast[pos];
    }

    template <typename T, int rank, template <typename, int> class AllocationPolicy>
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
