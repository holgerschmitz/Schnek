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
 *
 */

#ifndef SCHNEK_GRID_GRIDSTORAGE_KOKKOSSTORAGE_HPP_
#define SCHNEK_GRID_GRIDSTORAGE_KOKKOSSTORAGE_HPP_

#include "../../config.hpp"
#include "../array.hpp"

#include <Kokkos_Core.hpp>

#ifdef SCHNEK_HAVE_KOKKOS

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

    //     template <typename T, size_t rank, class ...ViewProperties>
    //     class KokkosViewContainer {

    //     public:
    //         KokkosViewContainer() = delete;
    //         KokkosViewContainer(const KokkosViewContainer&) = delete;
    //     };

    //     template <typename T, class ...ViewProperties>
    //     class KokkosViewContainer<T, 1, ViewProperties...> {
    //     protected:
    //         /// The kokkos view 
    //         Kokkos::View<typename internal::KokkosViewType<T, 1>::type, ViewProperties...> view;
    //     public:
    //         KokkosViewContainer(): view() {}
    //         KokkosViewContainer(const Array<int, 1> &dims) : view(dims[0]) {}
    //     };

    }

    /**
     * @brief A grid storage that uses the Kokkos memory model
     * 
     * @tparam T The type of data stored in the grid
     * @tparam rank The rank of the grid
     * @tparam AllocationPolicy The allocation policy
     */
    template <typename T, size_t rank, class ...ViewProperties>
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

        Kokkos::View<typename internal::KokkosViewType<T, 1>::type, ViewProperties...> view;
    public:
        /// Default constructor
        KokkosGridStorage();

        /**
         * @brief Construct with a given size
         * 
         * @param lo the lowest coordinate in the grid (inclusive)
         * @param lo the highest coordinate in the grid (inclusive)
         */
        KokkosGridStorage(const Index &low_, const Index &high_);

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
        const Index &getLo() const { return this->lo; }

        /// Get the highest coordinate in the grid (inclusive)
        const Index &getHi() const { return this->hi; }

        /// Get the dimensions of the grid `dims = high - low + 1`
        const Index &getDims() const { return this->dims; }

        /// Get k-th component of the lowest coordinate in the grid (inclusive)
        int getLo(int k) const { return this->lo[k]; }

        /// Get k-th component of the highest coordinate in the grid (inclusive)
        int getHi(int k) const { return this->hi[k]; }

        /// Get k-th component of the dimensions of the grid `dims = high - low + 1`
        int getDims(int k) const { return this->dims[k]; }

        /// Get the length of the allocated array
        int getSize() const { return this->size; }

        /**
         * @brief resizes to grid with lower indices low[0],...,low[rank-1]
         * and upper indices high[0],...,high[rank-1]
         */
        void resize(const Index &low, const Index &high);

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
    };

    template<typename T, size_t rank>
    using KokkosDefaultGridStorage = KokkosGridStorage<T, rank>;

    //=================================================================
    //==================== KokkosGridStorage ==========================
    //=================================================================

// #include <iostream>
// #include <tuple>
// #include <utility>

// int foo(int a) {return a;};
// double foo(int a, double b) {return a+b;};
// double foo(int a, double b, int c) {return a+b+c;};

// template <typename... T, std::size_t... Indices>
// auto call_foo_helper(const std::tuple<T...>& v, std::index_sequence<Indices...>) {
//   return foo(std::get<Indices>(v)...);
// }

// template <typename... T>
// auto call_foo(const std::tuple<T...>& v) {
//   return call_foo_helper(v, std::make_index_sequence<sizeof...(T)>());
// }

// int main(int argc, char** argv) {
//     auto v = std::make_tuple(1, 2.0);
//     auto x = call_foo(v);
//     return 0;
// }

    namespace internal {

    }

    template <typename T, size_t rank, class ...ViewProperties>
    KokkosGridStorage<T, rank, ViewProperties...>::KokkosGridStorage() : lo(0), hi(0), dims(0)
    {}

    template <typename T, size_t rank, class ...ViewProperties>
    KokkosGridStorage<T, rank, ViewProperties...>::KokkosGridStorage(const Index &lo, const Index &hi) :
        lo(lo), hi(hi), dims(hi - lo + 1)
    {
        view = createKokkosView(dims);

    }

    template <typename T, size_t rank, class ...ViewProperties>
    KokkosGridStorage<T, rank, ViewProperties...>::~KokkosGridStorage()
    {}

    template <typename T, size_t rank, class ...ViewProperties>
    const T &KokkosGridStorage<T, rank, ViewProperties...>::get(const Index &index) const
    {

    }

    template <typename T, size_t rank, class ...ViewProperties>
    T &KokkosGridStorage<T, rank, ViewProperties...>::get(const Index &index)
    {

    }
    
    template <typename T, size_t rank, class ...ViewProperties>
    void KokkosGridStorage<T, rank, ViewProperties...>::resize(const Index &lo, const Index &hi)
    {

    }

}


#endif // SCHNEK_HAVE_KOKKOS

#endif // SCHNEK_GRID_GRIDSTORAGE_KOKKOSSTORAGE_HPP_