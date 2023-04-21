/*
 * kokkos-storage.hpp
 *
 * Created on: 17 Nov 2022
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
 */

#ifndef SCHNEK_GRID_GRIDSTORAGE_KOKKOSSTORAGE_HPP_
#define SCHNEK_GRID_GRIDSTORAGE_KOKKOSSTORAGE_HPP_

#include "../../config.hpp"
#include "../../macros.hpp"
#include "../array.hpp"
#include "../range.hpp"

#ifdef SCHNEK_HAVE_KOKKOS

#include <Kokkos_Core.hpp>

namespace schnek {

    namespace internal {
        template<typename T, size_t rank>
        struct KokkosViewType {
            typedef typename KokkosViewType<T, rank-1>::type* type;
        };

        template<typename T>
        struct KokkosViewType<T, 1> {
            typedef T* type;
        };
    }

    /**
     * @brief A grid storage that uses the Kokkos memory model
     * 
     * @tparam T The type of data stored in the grid
     * @tparam rank The rank of the grid
     * @tparam AllocationPolicy The allocation policy
     */
    template <
        typename T, 
        size_t rank, 
        class ...ViewProperties
    >
    class KokkosGridStorage
    {
    public:
        /// The grid index type
        typedef Array<int, rank> IndexType;

        /// The grid range type
        typedef Range<int, rank> RangeType;
    private:
        /// The lowest and highest coordinates in the grid (inclusive)
        RangeType range;

        /// The dimensions of the grid `dims = high - low + 1`
        IndexType dims;

        Kokkos::View<typename internal::KokkosViewType<T, rank>::type, ViewProperties...> view;
    public:
        /// Default constructor
        KokkosGridStorage();

        /**
         * @brief Copy constructor
         */
        KokkosGridStorage(const KokkosGridStorage &) = default;

        /**
         * @brief Construct with a given size
         * 
         * @param lo the lowest coordinate in the grid (inclusive)
         * @param hi the highest coordinate in the grid (inclusive)
         */
        KokkosGridStorage(const IndexType &lo, const IndexType &hi);

        /**
         * @brief Construct with a given size
         * 
         * @param range the lowest and highest coordinates in the grid (inclusive)
         */
        KokkosGridStorage(const RangeType &range);

        /// Destructor frees any allocated memory
        ~KokkosGridStorage();

        /**
         * @brief Get the rvalue at a given grid index
         * 
         * @param index The grid index
         * @return the rvalue at the grid index
         */
        SCHNEK_INLINE const T& get(const IndexType &index) const;

        /**
         * @brief Get the lvalue at a given grid index
         * 
         * @param index The grid index
         * @return the lvalue at the grid index
         */
        SCHNEK_INLINE T& get(const IndexType &index);

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
        SCHNEK_INLINE int getSize() const { return this->size; }

        /**
         * @brief resizes to grid with lower indices low[0],...,low[rank-1]
         * and upper indices high[0],...,high[rank-1]
         */
        void resize(const IndexType &low, const IndexType &high);

        /**
         * @brief returns the stride of the specified dimension 
         */
        SCHNEK_INLINE ptrdiff_t stride(size_t dim) const;

    private:
        template<std::size_t... I>
        auto createKokkosViewImpl(const IndexType& a, std::index_sequence<I...>)
        {
            Kokkos::View<typename internal::KokkosViewType<T, rank>::type, ViewProperties...> view("schnek", a[I]...);
            return view;
        }
        
        auto createKokkosView(const IndexType& dims)
        {
            return createKokkosViewImpl(dims, std::make_index_sequence<rank>{});
        }

        template<std::size_t... I>
        SCHNEK_INLINE T& getFromViewImpl(const IndexType& pos, std::index_sequence<I...>)
        {
            return view(pos[I]...);
        }

        template<std::size_t... I>
        SCHNEK_INLINE const T& getFromViewImpl(const IndexType& pos, std::index_sequence<I...>) const
        {
            return view(pos[I]...);
        }

        SCHNEK_INLINE T& getFromView(const IndexType& pos)
        {
            return getFromViewImpl(pos, std::make_index_sequence<rank>{});
        }

        SCHNEK_INLINE const T& getFromView(const IndexType& pos) const
        {
            return getFromViewImpl(pos, std::make_index_sequence<rank>{});
        }
    };

    template<typename T, size_t rank>
    using KokkosDefaultGridStorage = KokkosGridStorage<T, rank>;

    //=================================================================
    //==================== KokkosGridStorage ==========================
    //=================================================================

    namespace internal {

    }

    template <typename T, size_t rank, class ...ViewProperties>
    KokkosGridStorage<T, rank, ViewProperties...>::KokkosGridStorage() : range{IndexType{0}, IndexType{0}}, dims{0}
    {}

    template <typename T, size_t rank, class ...ViewProperties>
    KokkosGridStorage<T, rank, ViewProperties...>::KokkosGridStorage(const IndexType &lo, const IndexType &hi) :
        range{lo, hi}
    {
        for (size_t i = 0; i < rank; ++i)
        {
            dims[i] = hi[i] - lo[i] + 1;
        }
        view = createKokkosView(dims);
    }

    template <typename T, size_t rank, class ...ViewProperties>
    KokkosGridStorage<T, rank, ViewProperties...>::KokkosGridStorage(const RangeType &range) :
        range{range}
    {
        for (size_t i = 0; i < rank; ++i)
        {
            dims[i] = range.getHi(i) - range.getLo(i) + 1;
        }
        view = createKokkosView(dims);
    }

    template <typename T, size_t rank, class ...ViewProperties>
    KokkosGridStorage<T, rank, ViewProperties...>::~KokkosGridStorage()
    {}

    template <typename T, size_t rank, class ...ViewProperties>
    SCHNEK_INLINE const T &KokkosGridStorage<T, rank, ViewProperties...>::get(const IndexType &index) const
    {
        IndexType pos;
        for (size_t i=0; i<rank; ++i)
        {
            pos[i] = index[i] - range.getLo(i);
        }
        return getFromView(pos);
    }

    template <typename T, size_t rank, class ...ViewProperties>
    SCHNEK_INLINE T &KokkosGridStorage<T, rank, ViewProperties...>::get(const IndexType &index)
    {       
        IndexType pos;
        for (size_t i=0; i<rank; ++i)
        {
            pos[i] = index[i] - range.getLo(i);
        }
        return getFromView(pos);
    }
    
    template <typename T, size_t rank, class ...ViewProperties>
    void KokkosGridStorage<T, rank, ViewProperties...>::resize(const IndexType &lo, const IndexType &hi)
    {
        this->range = RangeType{lo, hi};
        this->dims = hi - lo + 1;
        this->view = createKokkosView(dims);
    }

    template <typename T, size_t rank, class ...ViewProperties>
    SCHNEK_INLINE ptrdiff_t KokkosGridStorage<T, rank, ViewProperties...>::stride(size_t dim) const
    {
        return this->view.stride(dim);
    }

}


#endif // SCHNEK_HAVE_KOKKOS

#endif // SCHNEK_GRID_GRIDSTORAGE_KOKKOSSTORAGE_HPP_
