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
        typedef Array<int, rank> Index;
    private:
        /// The lowest coordinate in the grid (inclusive)
        Index lo;

        /// The highest coordinate in the grid (inclusive)
        Index hi;

        /// The dimensions of the grid `dims = high - low + 1`
        Index dims;

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
         * @param lo the highest coordinate in the grid (inclusive)
         */
        KokkosGridStorage(const Index &lo, const Index &hi);

        /// Destructor frees any allocated memory
        ~KokkosGridStorage();

        /**
         * @brief Get the rvalue at a given grid index
         * 
         * @param index The grid index
         * @return the rvalue at the grid index
         */
        const T &get(const Index &index) const;

        /// Get the lowest coordinate in the grid (inclusive)
        SCHNEK_INLINE const Index &getLo() const { return this->lo; }

        /// Get the highest coordinate in the grid (inclusive)
        SCHNEK_INLINE const Index &getHi() const { return this->hi; }

        /// Get the dimensions of the grid `dims = high - low + 1`
        SCHNEK_INLINE const Index &getDims() const { return this->dims; }

        /// Get k-th component of the lowest coordinate in the grid (inclusive)
        SCHNEK_INLINE int getLo(int k) const { return this->lo[k]; }

        /// Get k-th component of the highest coordinate in the grid (inclusive)
        SCHNEK_INLINE int getHi(int k) const { return this->hi[k]; }

        /// Get k-th component of the dimensions of the grid `dims = high - low + 1`
        SCHNEK_INLINE int getDims(int k) const { return this->dims[k]; }

        /// Get the length of the allocated array
        SCHNEK_INLINE int getSize() const { return this->size; }

        /**
         * @brief resizes to grid with lower indices low[0],...,low[rank-1]
         * and upper indices high[0],...,high[rank-1]
         */
        void resize(const Index &low, const Index &high);

        /**
         * @brief returns the stride of the specified dimension 
         */
        ptrdiff_t stride(size_t dim) const;

        /**
         * @brief Get the lvalue at a given grid index
         * 
         * @param index The grid index
         * @return the lvalue at the grid index
         */
        T &get(const Index &index);

    private:
        template<std::size_t... I>
        auto createKokkosViewImpl(const Index& a, std::index_sequence<I...>)
        {
            Kokkos::View<typename internal::KokkosViewType<T, rank>::type, ViewProperties...> view("schnek", a[I]...);
            return view;
        }
        
        auto createKokkosView(const Index& dims)
        {
            return createKokkosViewImpl(dims, std::make_index_sequence<rank>{});
        }

        template<std::size_t... I>
        SCHNEK_INLINE T& getFromViewImpl(const Index& pos, std::index_sequence<I...>)
        {
            return view(pos[I]...);
        }

        template<std::size_t... I>
        SCHNEK_INLINE const T& getFromViewImpl(const Index& pos, std::index_sequence<I...>) const
        {
            return view(pos[I]...);
        }

        SCHNEK_INLINE T& getFromView(const Index& pos)
        {
            return getFromViewImpl(pos, std::make_index_sequence<rank>{});
        }

        SCHNEK_INLINE const T& getFromView(const Index& pos) const
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
    KokkosGridStorage<T, rank, ViewProperties...>::KokkosGridStorage() : lo(0), hi(0), dims(0)
    {}

    template <typename T, size_t rank, class ...ViewProperties>
    KokkosGridStorage<T, rank, ViewProperties...>::KokkosGridStorage(const Index &lo, const Index &hi) :
        lo(lo), hi(hi)
    {
        for (size_t i = 0; i < rank; ++i)
        {
            dims[i] = hi[i] - lo[i] + 1;
        }
        view = createKokkosView(dims);
    }

    template <typename T, size_t rank, class ...ViewProperties>
    KokkosGridStorage<T, rank, ViewProperties...>::~KokkosGridStorage()
    {}

    template <typename T, size_t rank, class ...ViewProperties>
    SCHNEK_INLINE const T &KokkosGridStorage<T, rank, ViewProperties...>::get(const Index &index) const
    {
        Index pos;
        for (size_t i=0; i<rank; ++i)
        {
            pos[i] = index[i] - lo[i];
        }
        return getFromView(pos);
    }

    template <typename T, size_t rank, class ...ViewProperties>
    SCHNEK_INLINE T &KokkosGridStorage<T, rank, ViewProperties...>::get(const Index &index)
    {       
        Index pos;
        for (size_t i=0; i<rank; ++i)
        {
            pos[i] = index[i] - lo[i];
        }
        return getFromView(pos);
    }
    
    template <typename T, size_t rank, class ...ViewProperties>
    void KokkosGridStorage<T, rank, ViewProperties...>::resize(const Index &lo, const Index &hi)
    {
        this->lo = lo;
        this->hi = hi;
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
