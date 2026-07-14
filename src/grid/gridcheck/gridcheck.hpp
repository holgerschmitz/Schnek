/*
 * gridcheck.hpp
 *
 * Created on: 23 Jan 2007
 * Author: Holger Schmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2012 Holger Schmitz
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

#ifndef SCHNEK_GRIDCHECK_GRIDCHECK_H_
#define SCHNEK_GRIDCHECK_GRIDCHECK_H_

#include <cassert>

#include "../../macros.hpp"
#include "../../util/logger.hpp"
#include "../array.hpp"

namespace schnek {

  template<typename T, size_t rank>
  class GridNoArgCheck {
    public:
      typedef Array<ptrdiff_t, rank, ArrayNoArgCheck> IndexType;
      static const IndexType &check(const IndexType &pos, const IndexType &low, const IndexType &high);
  };

  template<typename T, size_t rank>
  class GridAssertCheck {
    public:
      typedef Array<ptrdiff_t, rank, ArrayAssertArgCheck> IndexType;
      static const IndexType &check(const IndexType &pos, const IndexType &low, const IndexType &high);
  };

  template<typename T, size_t rank>
  class GridDebugCheck {
    public:
      typedef Array<ptrdiff_t, rank, ArrayAssertArgCheck> IndexType;

    private:
      static bool errorFlag;
      static int errorInfo;
      static IndexType offending;

    public:
      static const IndexType check(const IndexType &pos, const IndexType &low, const IndexType &high);

      static bool getErrorFlag() { return errorFlag; }

      static int getErrorInfo() { return errorInfo; }

      static IndexType getOffending() { return offending; }
  };

  template<typename T, size_t rank>
  const typename GridNoArgCheck<T, rank>::IndexType &
  GridNoArgCheck<T, rank>::check(const IndexType &pos, const IndexType &, const IndexType &) {
    return pos;
  }

  template<typename T, size_t rank>
  const typename GridAssertCheck<T, rank>::IndexType &GridAssertCheck<T, rank>::check(
      const IndexType &pos, const IndexType &low, const IndexType &high
  ) {
    for (size_t i = 0; i < rank; ++i) {
      assert(pos[i] >= low[i]);
      assert(pos[i] <= high[i]);
    }
    return pos;
  }

  template<typename T, size_t rank>
  bool GridDebugCheck<T, rank>::errorFlag = false;

  template<typename T, size_t rank>
  int GridDebugCheck<T, rank>::errorInfo = 0;

  template<typename T, size_t rank>
  typename GridDebugCheck<T, rank>::IndexType GridDebugCheck<T, rank>::offending;

  template<typename T, size_t rank>
  inline const typename GridDebugCheck<T, rank>::IndexType GridDebugCheck<T, rank>::check(
      const IndexType &pos, const IndexType &low, const IndexType &high
  ) {
    IndexType pos_copy(pos);
    for (size_t i = 0; i < rank; ++i) {
      if (pos_copy[i] < low[i]) {
        SCHNEK_TRACE_ERR(
            1, "schnek::GridDebugCheck index out of range (dim=" << i << "): index=" << pos_copy[i] << "  lo=" << low[i]
        )
        pos_copy[i] = low[i];
        GridDebugCheck<T, rank>::errorFlag = true;
        GridDebugCheck<T, rank>::errorInfo = -i;
        GridDebugCheck<T, rank>::offending = pos;
      }
      if (pos_copy[i] > high[i]) {
        SCHNEK_TRACE_ERR(
            1,
            "schnek::GridDebugCheck index out of range (dim=" << i << "): index=" << pos_copy[i] << "  hi=" << high[i]
        )
        pos_copy[i] = high[i];
        GridDebugCheck<T, rank>::errorFlag = true;
        GridDebugCheck<T, rank>::errorInfo = i;
        GridDebugCheck<T, rank>::offending = pos;
      }
    }
    return pos_copy;
  }

}  // namespace schnek

#endif  // SCHNEK_GRIDCHECK_GRIDCHECK_H_
