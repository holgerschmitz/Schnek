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

#ifdef SCHNEK_HAVE_KOKKOS

#include <Kokkos_Core.hpp>
#include <functional>
#include <map>
#include <memory>

#include "../../macros.hpp"
#include "../array.hpp"
#include "../range.hpp"
#include "../gridcheck.hpp"

#include <schnek/grid/iteration/kokkos-iteration.hpp>

namespace schnek {

  namespace internal {
    template<typename T, size_t rank_t>
    struct KokkosViewType {
        typedef typename KokkosViewType<T, rank_t - 1>::type *type;
    };

    template<typename T>
    struct KokkosViewType<T, 1> {
        typedef T *type;
    };
  }  // namespace internal

  /**
   * @brief A grid storage that uses the Kokkos memory model
   *
   * @tparam T The type of data stored in the grid
   * @tparam rank The rank of the grid
   * @tparam AllocationPolicy The allocation policy
   */
  template<typename T, size_t rank_t, class... ViewProperties>
  class KokkosGridStorage {
    public:
      /// The value type
      typedef T value_type;

      /// The rank of the grid
      static constexpr size_t rank = rank_t;

      /// The grid index type
      typedef Array<int, rank_t> IndexType;

      /// The grid range type
      typedef Range<int, rank_t> RangeType;

    private:
      typedef std::function<void(const RangeType &)> UpdaterType;
      typedef std::map<void *, UpdaterType> UpdaterMapType;

      size_t size;

      /// The lowest and highest coordinates in the grid (inclusive)
      RangeType range;

      /// The dimensions of the grid `dims = high - low + 1`
      IndexType dims;

      Kokkos::View<typename internal::KokkosViewType<T, rank_t>::type, ViewProperties...> view;

      /// A map of updaters that are called when the grid is resized
      std::shared_ptr<UpdaterMapType> updaters;

    public:
      /// Default constructor
      KokkosGridStorage();

      /**
       * @brief Copy constructor
       */
      KokkosGridStorage(const KokkosGridStorage &);

      /**
       * @brief Explicitly defaulting the copy assignment operator
       */
      KokkosGridStorage& operator=(const KokkosGridStorage&) = default;
      
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
      SCHNEK_INLINE const T &get(const IndexType &index) const;

      /**
       * @brief Get the lvalue at a given grid index
       *
       * @param index The grid index
       * @return the lvalue at the grid index
       */
      SCHNEK_INLINE T &get(const IndexType &index);

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

      SCHNEK_INLINE T* getRawData() const {
          return view.data();
      }

      // T* getRawData() const {
      //   // copy from device to host (check in case I am using GPU)
      //   auto host_view = Kokkos::create_mirror_view(view);
      //   Kokkos::deep_copy(host_view, view);
        
      //   return host_view.data();
      // }

      /**
       * @brief resizes to grid with lower indices low[0],...,low[rank-1]
       * and upper indices high[0],...,high[rank-1]
       */
      SCHNEK_INLINE void resize(const IndexType &low, const IndexType &high);

      KokkosGridStorage<T, rank_t, ViewProperties...> &operator=(const T &val);
      
      void fill(const T &val);

      SCHNEK_INLINE void set(const IndexType &index, const T &value) const;

      template<typename FunctionType>
      void parallel_func(const IndexType &low, const IndexType &high, FunctionType func) const;

      /**
       * @brief returns the stride of the specified dimension
       */
      SCHNEK_INLINE ptrdiff_t stride(size_t dim) const;

    private:
      template<std::size_t... I>
      auto createKokkosViewImpl(const IndexType &a, std::index_sequence<I...>) {
        Kokkos::View<typename internal::KokkosViewType<T, rank_t>::type, ViewProperties...> view("schnek", a[I]...);
        return view;
      }

      auto createKokkosView(const IndexType &dims) {
        return createKokkosViewImpl(dims, std::make_index_sequence<rank_t>{});
      }

      template<std::size_t... I>
      SCHNEK_INLINE T &getFromViewImpl(const IndexType &pos, std::index_sequence<I...>) {
        return view(pos[I]...);
      }

      template<std::size_t... I>
      SCHNEK_INLINE const T &getFromViewImpl(const IndexType &pos, std::index_sequence<I...>) const {
        return view(pos[I]...);
      }

      SCHNEK_INLINE T &getFromView(const IndexType &pos) {
        return getFromViewImpl(pos, std::make_index_sequence<rank_t>{});
      }

      SCHNEK_INLINE const T &getFromView(const IndexType &pos) const {
        return getFromViewImpl(pos, std::make_index_sequence<rank_t>{});
      }

      void update(const RangeType &range) {
        for (auto &updater : *updaters) {
          updater.second(range);
        }
      }

      void updateSizeInfo(const RangeType &range) {
        this->range = range;
        for (size_t i = 0; i < rank_t; ++i) {
          dims[i] = range.getHi(i) - range.getLo(i) + 1;
        }
        size = 1;
        for (size_t i = 0; i < rank_t; ++i) {
          size *= dims[i];
        }
      }
  };

  template<typename T, size_t rank_t>
  using KokkosDefaultGridStorage = KokkosGridStorage<T, rank_t>;

  //=================================================================
  //==================== KokkosGridStorage ==========================
  //=================================================================
  
  template<typename T, size_t rank_t, class... ViewProperties>
  SCHNEK_INLINE void KokkosGridStorage<T, rank_t, ViewProperties...>::set(
      const IndexType &index, const T &value) const {
      IndexType pos;
      for (size_t i = 0; i < rank_t; ++i) {
          pos[i] = index[i] - range.getLo(i);
      }
      // getFromView(pos) = value;
      const_cast<T&>(getFromView(pos)) = value;
  }

  template<typename T, size_t rank_t, class... ViewProperties>
  KokkosGridStorage<T, rank_t, ViewProperties...>::KokkosGridStorage()
      : range{IndexType{0}, IndexType{0}}, dims{0}, size(0), updaters{new UpdaterMapType} {
    (*updaters)[this] = [this](const RangeType &range) { this->updateSizeInfo(range); };
  }

  template<typename T, size_t rank_t, class... ViewProperties>
  KokkosGridStorage<T, rank_t, ViewProperties...>::KokkosGridStorage(const KokkosGridStorage &other)
      : range{other.range}, dims{other.dims}, size(other.size), view{other.view}, updaters{other.updaters} {
    (*updaters)[this] = [this](const RangeType &range) { this->updateSizeInfo(range); };
  }

  template<typename T, size_t rank_t, class... ViewProperties>
  KokkosGridStorage<T, rank_t, ViewProperties...>::KokkosGridStorage(const IndexType &lo, const IndexType &hi)
      : range{lo, hi}, updaters{new UpdaterMapType} {
    dims = hi - lo + 1;
    size = 1;
    for (size_t i = 0; i < rank_t; ++i) {
      size *= dims[i];
    }
    view = createKokkosView(dims);
    (*updaters)[this] = [this](const RangeType &range) { this->updateSizeInfo(range); };
  }

  template<typename T, size_t rank_t, class... ViewProperties>
  KokkosGridStorage<T, rank_t, ViewProperties...>::KokkosGridStorage(const RangeType &range)
    : range{range}, updaters{new UpdaterMapType} {
  dims = range.getHi() - range.getLo() + 1;
  size = 1;
  for (size_t i = 0; i < rank_t; ++i) {
    size *= dims[i];
  }
  view = createKokkosView(dims);
  (*updaters)[this] = [this](const RangeType &range) { this->updateSizeInfo(range); };
  }
  
  template<typename T, size_t rank_t, class... ViewProperties>
  KokkosGridStorage<T, rank_t, ViewProperties...>::~KokkosGridStorage() {
    updaters->erase(this);
  }

  template<typename T, size_t rank_t, class... ViewProperties>
  SCHNEK_INLINE const T &KokkosGridStorage<T, rank_t, ViewProperties...>::get(const IndexType &index) const {
    IndexType pos;
    for (size_t i = 0; i < rank_t; ++i) {
      pos[i] = index[i] - range.getLo(i);
    }
    return getFromView(pos);
  }

  template<typename T, size_t rank_t, class... ViewProperties>
  SCHNEK_INLINE T &KokkosGridStorage<T, rank_t, ViewProperties...>::get(const IndexType &index) {
    IndexType pos;
    for (size_t i = 0; i < rank_t; ++i) {
      pos[i] = index[i] - range.getLo(i);
    }
    return getFromView(pos);
  }

  template<typename T, size_t rank_t, class... ViewProperties>
  void KokkosGridStorage<T, rank_t, ViewProperties...>::resize(const IndexType &lo, const IndexType &hi) {
    dims = hi - lo + 1;
    size = 1;
    for (size_t i = 0; i < rank_t; ++i) {
      size *= dims[i];
    }
    this->view = createKokkosView(dims);
    update(RangeType{lo, hi});
  }

  template<typename T, size_t rank_t, class... ViewProperties>
  SCHNEK_INLINE ptrdiff_t KokkosGridStorage<T, rank_t, ViewProperties...>::stride(size_t dim) const {
    return this->view.stride(dim);
  }
  
  template<typename T, size_t rank_t, class... ViewProperties>
  void KokkosGridStorage<T, rank_t, ViewProperties...>::fill(const T& val) {
      // For 1D case
      if constexpr (rank_t == 1) {
          Kokkos::parallel_for("fill_grid_1d", 
              Kokkos::RangePolicy<>(0, dims[0]),
              SCHNEK_DEVICE_LAMBDA (const int i) {
                  IndexType pos;
                  pos[0] = i;
                  view(i) = val;
              }
          );
      }
      // For 2D case
      else if constexpr (rank_t == 2) {
          Kokkos::parallel_for("fill_grid_2d", 
              Kokkos::MDRangePolicy<Kokkos::Rank<2>>({0,0}, {dims[0], dims[1]}),
              SCHNEK_DEVICE_LAMBDA (const int i, const int j) {
                  view(i, j) = val;
              }
          );
      }
      Kokkos::fence(); // Ensure operations complete
  }

  template<typename T, size_t rank_t, class... ViewProperties>
  template<typename FunctionType>
  void KokkosGridStorage<T, rank_t, ViewProperties...>::parallel_func(
      const IndexType& low, 
      const IndexType& high, 
      FunctionType func) const {
      
      if constexpr (rank_t == 2) {
        // std::cout << "parallel_func is being called" << std::endl;
          Kokkos::parallel_for("kokkos_parallel_2d",
              Kokkos::MDRangePolicy<Kokkos::Rank<2>>(
                  {low[0], low[1]}, 
                  {high[0], high[1]}
              ),
              SCHNEK_DEVICE_LAMBDA (const int i, const int j) {
              // better for device execution
              // [=] KOKKOS_LAMBDA (const int i, const int j) {
              // reference call mgiht cause issues for host memory
              // [&](int i, int j) {
                  IndexType pos;
                  pos[0] = i;
                  pos[1] = j;
                  func(pos);
              }
          );
      }
      Kokkos::fence();
  }

  namespace kokkos_utils {
    template<typename GridType, typename FunctionType>
    void parallel_kokkos_parallel_for(
        const typename GridType::IndexType& low,
        const typename GridType::IndexType& high,
        FunctionType func) {
        
        if constexpr (GridType::Rank == 2) {
          // std::cout << "parallel_kokkos_parallel_for is being called" << std::endl;
            Kokkos::parallel_for("parallel_operation",
                Kokkos::MDRangePolicy<Kokkos::Rank<2>>(
                    {low[0], low[1]}, 
                    {high[0], high[1]}
                ),
                SCHNEK_DEVICE_LAMBDA (const int i, const int j) {
                    typename GridType::IndexType pos;
                    pos[0] = i;
                    pos[1] = j;
                    func(pos);
                }
            );
            Kokkos::fence();
        }
    }

    template<typename GridType, typename FunctionType>
    void parallel_kokkos_iteration(
        const typename GridType::IndexType& low,
        const typename GridType::IndexType& high,
        FunctionType func) {
        
        using RangeType = schnek::Range<int, GridType::Rank, schnek::ArrayNoArgCheck>;
        RangeType range(low, high);
        
        // std::cout << "parallel_kokkos_iteration is being called" << std::endl;
        schnek::RangeKokkosIterationPolicy<GridType::Rank>::forEach(range, func);

        // if constexpr (GridType::Rank == 1) {
        //  schnek::RangeKokkosIterationPolicy<1>::forEach(range, func);
        // }
        // else if constexpr (GridType::Rank == 2) {
        //   schnek::RangeKokkosIterationPolicy<2>::forEach(range, func);
        // }

        Kokkos::fence();   
    }

    template<typename GridType, typename FunctionType>
    void parallel_kokkos_parallel_for_v1(
        const typename GridType::IndexType& low,
        const typename GridType::IndexType& high,
        FunctionType func) {
        
        if constexpr (GridType::Rank == 2) {
            int ni = high[0] - low[0] + 1;
            int nj = high[1] - low[1] + 1;
            
            // int team_size = 32; vector_length = 32;
            
            // Kokkos::TeamPolicy<> policy(ni, team_size, vector_length);

            Kokkos::TeamPolicy<> policy(ni, Kokkos::AUTO); // calc team size automaticlaly
            
            Kokkos::parallel_for("parallel_hierarchical", policy, 
                SCHNEK_DEVICE_LAMBDA (const Kokkos::TeamPolicy<>::member_type& team_member) {
                    const int i = team_member.league_rank() + low[0];
                    
                    Kokkos::parallel_for(Kokkos::TeamThreadRange(team_member, nj), 
                        SCHNEK_DEVICE_LAMBDA (const int j_local) {
                            const int j = j_local + low[1];
                            
                            typename GridType::IndexType pos;
                            pos[0] = i;
                            pos[1] = j;
                            func(pos);
                        }
                    );
                }
            );
            Kokkos::fence();
        }
    }
  } // namespace kokkos_utils

}  // namespace schnek

#endif  // SCHNEK_HAVE_KOKKOS

#endif  // SCHNEK_GRID_GRIDSTORAGE_KOKKOSSTORAGE_HPP_
