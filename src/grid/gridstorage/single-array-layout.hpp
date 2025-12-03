/*
 * single-array-layout.hpp
 *
 * Created on: 29 Oct 2025
 * Author: Holger Schmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2025 Holger Schmitz
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

#ifndef SCHNEK_GRID_GRIDSTORAGE_SINGLEARRAYLAYOUT_HPP_
#define SCHNEK_GRID_GRIDSTORAGE_SINGLEARRAYLAYOUT_HPP_

#include "../array.hpp"
#include "../range.hpp"

namespace schnek {

  /**
   * @brief A layout policy for SingleArrayStorage to provide C-order indexing over the
   * 1-dimensional data array for a multidimensional grid.
   *
   * @tparam T The type of data stored in the grid
   * @tparam Rank The rank of the grid
   */
  template<typename T, size_t Rank>
  class SingleArrayGridCOrderLayout {
    private:
      /// A pointer offset to the origin for faster access
      T *data_fast;

    public:
      /// The grid index type
      typedef Array<ptrdiff_t, Rank> IndexType;

      /// The grid size type
      typedef Array<size_t, Rank> SizeType;

      /// The grid range type
      typedef Range<ptrdiff_t, Rank> RangeType;

      /// Default constructor
      SingleArrayGridCOrderLayout() = default;

      /// Copy constructor
      SingleArrayGridCOrderLayout(const SingleArrayGridCOrderLayout &) = default;

      /// Assignment operator
      SingleArrayGridCOrderLayout<T, Rank> &operator=(const SingleArrayGridCOrderLayout<T, Rank> &) = default;

      /**
       * @brief Get the lvalue at a given grid index
       *
       * @param index The grid index
       * @return the lvalue at the grid index
       */
      SCHNEK_INLINE T &get(const IndexType &index, const SizeType &dims);

      /**
       * @brief Get the rvalue at a given grid index
       *
       * @param index The grid index
       * @return the rvalue at the grid index
       */
      SCHNEK_INLINE const T &get(const IndexType &index, const SizeType &dims) const;

      /**
       * @brief returns the stride of the specified dimension
       */
      SCHNEK_INLINE ptrdiff_t stride(size_t dim, const SizeType &dims) const;

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
      void updateSize(const RangeType &range, T *data);
  };

  /**
   * @brief A layout policy for SingleArrayStorage to provide Fortran-order indexing over the
   * 1-dimensional data array for a multidimensional grid.
   *
   * @tparam T The type of data stored in the grid
   * @tparam Rank The rank of the grid
   */
  template<typename T, size_t Rank>
  class SingleArrayGridFortranOrderLayout {
    private:
      /// A pointer offset to the origin for faster access
      T *data_fast;

    public:
      /// The grid index type
      typedef Array<ptrdiff_t, Rank> IndexType;

      /// The grid size type
      typedef Array<size_t, Rank> SizeType;

      /// The grid range type
      typedef Range<ptrdiff_t, Rank> RangeType;

      /// Default constructor
      SingleArrayGridFortranOrderLayout() = default;

      /// Copy constructor
      SingleArrayGridFortranOrderLayout(const SingleArrayGridFortranOrderLayout &) = default;

      /// Assignment operator
      SingleArrayGridFortranOrderLayout<T, Rank> &operator=(const SingleArrayGridFortranOrderLayout<T, Rank> &) =
          default;

      /**
       * @brief Get the lvalue at a given grid index
       *
       * @param index The grid index
       * @return the lvalue at the grid index
       */
      SCHNEK_INLINE T &get(const IndexType &index, const SizeType &dims);

      /**
       * @brief Get the rvalue at a given grid index
       *
       * @param index The grid index
       * @return the rvalue at the grid index
       */
      SCHNEK_INLINE const T &get(const IndexType &index, const SizeType &dims) const;

      /**
       * @brief returns the stride of the specified dimension
       */
      SCHNEK_INLINE ptrdiff_t stride(size_t dim, const SizeType &dims) const;

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
      void updateSize(const RangeType &range, T *data);
  };

  //=================================================================
  //================== SingleArrayGridCOrderLayout ==================
  //=================================================================

  template<typename T, size_t Rank>
  SCHNEK_INLINE T &SingleArrayGridCOrderLayout<T, Rank>::get(const IndexType &index, const SizeType &dims) {
    size_t pos = index[0];
    for (size_t i = 1; i < Rank; ++i) {
      pos = index[i] + dims[i] * pos;
    }
    return this->data_fast[pos];
  }

  template<typename T, size_t Rank>
  SCHNEK_INLINE const T &SingleArrayGridCOrderLayout<T, Rank>::get(const IndexType &index, const SizeType &dims) const {
    size_t pos = index[0];
    for (size_t i = 1; i < Rank; ++i) {
      pos = index[i] + dims[i] * pos;
    }
    return this->data_fast[pos];
  }

  template<typename T, size_t Rank>
  SCHNEK_INLINE ptrdiff_t SingleArrayGridCOrderLayout<T, Rank>::stride(size_t dim, const SizeType &dims) const {
    size_t stride = 1;
    for (size_t i = Rank - 1; i > dim; --i) {
      stride *= dims[i];
    }
    return stride;
  }

  template<typename T, size_t Rank>
  void SingleArrayGridCOrderLayout<T, Rank>::updateSize(const RangeType &range, T *data) {
    ptrdiff_t p = -range.getLo(0);

    for (size_t d = 1; d < Rank; ++d) {
      p = p * (range.getHi(d) - range.getLo(d) + 1) - range.getLo(d);
    }
    data_fast = data + p;
  }

  //=================================================================
  //=============== SingleArrayGridFortranOrderLayout ===============
  //=================================================================

  template<typename T, size_t Rank>
  SCHNEK_INLINE T &SingleArrayGridFortranOrderLayout<T, Rank>::get(const IndexType &index, const SizeType &dims) {
    size_t pos = index[Rank - 1];
    for (ptrdiff_t i = ptrdiff_t(Rank) - 2; i >= 0; --i) {
      pos = index[i] + dims[i] * pos;
    }
    return this->data_fast[pos];
  }

  template<typename T, size_t Rank>
  SCHNEK_INLINE const T &SingleArrayGridFortranOrderLayout<T, Rank>::get(const IndexType &index, const SizeType &dims) const {
    size_t pos = index[Rank - 1];
    for (ptrdiff_t i = ptrdiff_t(Rank) - 2; i >= 0; --i) {
      pos = index[i] + dims[i] * pos;
    }
    return this->data_fast[pos];
  }

  template<typename T, size_t Rank>
  SCHNEK_INLINE ptrdiff_t SingleArrayGridFortranOrderLayout<T, Rank>::stride(size_t dim, const SizeType &dims) const {
    ptrdiff_t stride = 1;
    for (size_t i = 0; i < dim; ++i) {
      stride *= dims[i];
    }
    return stride;
  }

  template<typename T, size_t Rank>
  void SingleArrayGridFortranOrderLayout<T, Rank>::updateSize(const RangeType &range, T *data) {
    size_t p = -range.getLo(Rank - 1);

    for (ptrdiff_t d = ptrdiff_t(Rank) - 2; d >= 0; --d) {
      p = p * (range.getHi(d) - range.getLo(d) + 1) - range.getLo(d);
    }
    data_fast = data + p;
  }
}  // namespace schnek

#endif  // SCHNEK_GRID_GRIDSTORAGE_SINGLEARRAYLAYOUT_HPP_