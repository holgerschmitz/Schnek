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

#include "config.hpp"

#ifdef SCHNEK_HAVE_KOKKOS

#include <Kokkos_Core.hpp>
#include <functional>
#include <map>
#include <memory>

#include "../../macros.hpp"
#include "../array.hpp"
#include "../range.hpp"

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
      typedef Array<ptrdiff_t, rank_t> IndexType;

      /// The grid size type
      typedef Array<size_t, rank> SizeType;

      /// The grid range type
      typedef Range<ptrdiff_t, rank_t> RangeType;

      /// The underlying Kokkos view type
      typedef Kokkos::View<typename internal::KokkosViewType<T, rank_t>::type, ViewProperties...> ViewType;

      /// The memory space of the underlying Kokkos view
      typedef typename ViewType::memory_space MemorySpace;

      /// True if the underlying memory is accessible from host code
      static constexpr bool host_accessible =
          Kokkos::SpaceAccessibility<Kokkos::HostSpace, MemorySpace>::accessible;

      /// Set the flag to true, so that the encapsulating Grid class compiles with device annotations.
      static constexpr bool use_device_annotations = true;
          
    private:
      typedef std::function<void(const RangeType &)> UpdaterType;
      typedef std::map<void *, UpdaterType> UpdaterMapType;

      /// The lowest and highest coordinates in the grid (inclusive)
      RangeType range;

      /// The dimensions of the grid `dims = high - low + 1`
      IndexType dims;

      ViewType view;

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
      SCHNEK_FUNCTION const T &get(const IndexType &index) const;

      /**
       * @brief Get the lvalue at a given grid index
       *
       * @param index The grid index
       * @return the lvalue at the grid index
       */
      SCHNEK_FUNCTION T &get(const IndexType &index);

      /// Get the lowest coordinate in the grid (inclusive)
      SCHNEK_FUNCTION const IndexType &getLo() const { return this->range.getLo(); }

      /// Get the highest coordinate in the grid (inclusive)
      SCHNEK_FUNCTION const IndexType &getHi() const { return this->range.getHi(); }

      /// Get the lowest coordinate in the grid (inclusive)
      SCHNEK_FUNCTION const RangeType &getRange() const { return this->range; }

      /// Get the dimensions of the grid `dims = high - low + 1`
      SCHNEK_FUNCTION const SizeType &getDims() const { return this->dims; }

      /// Get k-th component of the lowest coordinate in the grid (inclusive)
      SCHNEK_FUNCTION ptrdiff_t getLo(size_t k) const { return this->range.getLo(k); }

      /// Get k-th component of the highest coordinate in the grid (inclusive)
      SCHNEK_FUNCTION ptrdiff_t getHi(size_t k) const { return this->range.getHi(k); }

      /// Get k-th component of the dimensions of the grid `dims = high - low + 1`
      SCHNEK_FUNCTION size_t getDims(size_t k) const { return this->dims[k]; }

      /// Get the length of the allocated array
      SCHNEK_FUNCTION size_t getSize() const { return this->view.size(); }

      /// Get a reference to the underlying Kokkos view
      SCHNEK_FUNCTION ViewType &getKokkosView() { return this->view; }

      /// Get a const reference to the underlying Kokkos view
      const ViewType &getKokkosView() const { return this->view; }

      /**
       * @brief Get a raw pointer to the underlying data
       *
       * Only available when the memory space is accessible from host code.
       */
      template<typename V = ViewType>
      auto getRawData() -> std::enable_if_t<
          Kokkos::SpaceAccessibility<Kokkos::HostSpace, typename V::memory_space>::accessible, 
          T *>{
        return this->view.data();
      }

      /// Create a host-accessible mirror of the underlying view
      auto createHostMirror() const { return Kokkos::create_mirror_view(this->view); }

      /// Copy the device data into a previously created host mirror
      template<typename HostView>
      void deepCopyToHost(HostView &hostView) const {
        Kokkos::deep_copy(hostView, this->view);
      }

      /// Copy data from a host mirror into the device view
      template<typename HostView>
      void deepCopyFromHost(const HostView &hostView) {
        Kokkos::deep_copy(this->view, hostView);
      }

      /**
       * @brief resizes to grid with lower indices low[0],...,low[rank-1]
       * and upper indices high[0],...,high[rank-1]
       */
      SCHNEK_FUNCTION void resize(const IndexType &low, const IndexType &high);

      /**
       * @brief resizes the grid to the given range
       */
      SCHNEK_FUNCTION void resize(const RangeType &range) { this->resize(range.getLo(), range.getHi()); }

      /**
       * @brief returns the stride of the specified dimension
       */
      ptrdiff_t stride(size_t dim) const;

    private:
      template<std::size_t... I>
      ViewType createKokkosViewImpl(const IndexType &a, std::index_sequence<I...>) {
        ViewType view("schnek", a[I]...);
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
      }
  };

  template<typename T, size_t rank_t>
  using KokkosDefaultGridStorage = KokkosGridStorage<T, rank_t>;

  //=================================================================
  //==================== KokkosGridStorage ==========================
  //=================================================================

  template<typename T, size_t rank_t, class... ViewProperties>
  KokkosGridStorage<T, rank_t, ViewProperties...>::KokkosGridStorage()
      : range{IndexType{0}, IndexType{0}}, dims{0}, updaters{new UpdaterMapType} {
    (*updaters)[this] = [this](const RangeType &range) { this->updateSizeInfo(range); };
  }

  template<typename T, size_t rank_t, class... ViewProperties>
  KokkosGridStorage<T, rank_t, ViewProperties...>::KokkosGridStorage(const KokkosGridStorage &other)
      : range{other.range}, dims{other.dims}, view{other.view}, updaters{other.updaters} {
    (*updaters)[this] = [this](const RangeType &range) { this->updateSizeInfo(range); };
  }

  template<typename T, size_t rank_t, class... ViewProperties>
  KokkosGridStorage<T, rank_t, ViewProperties...>::KokkosGridStorage(const IndexType &lo, const IndexType &hi)
      : range{lo, hi}, updaters{new UpdaterMapType} {
    dims = hi - lo + 1;
    view = createKokkosView(dims);
    (*updaters)[this] = [this](const RangeType &range) { this->updateSizeInfo(range); };
  }

  template<typename T, size_t rank_t, class... ViewProperties>
  KokkosGridStorage<T, rank_t, ViewProperties...>::KokkosGridStorage(const RangeType &range)
      : range{range}, updaters{new UpdaterMapType} {
    dims = range.getHi() - range.getLo() + 1;
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
  SCHNEK_INLINE void KokkosGridStorage<T, rank_t, ViewProperties...>::resize(const IndexType &lo, const IndexType &hi) {
    IndexType dims = hi - lo + 1;
    this->view = createKokkosView(dims);
    update(RangeType{lo, hi});
  }

  template<typename T, size_t rank_t, class... ViewProperties>
  ptrdiff_t KokkosGridStorage<T, rank_t, ViewProperties...>::stride(size_t dim) const {
    return this->view.stride(dim);
  }

}  // namespace schnek

#endif  // SCHNEK_HAVE_KOKKOS

#endif  // SCHNEK_GRID_GRIDSTORAGE_KOKKOSSTORAGE_HPP_
