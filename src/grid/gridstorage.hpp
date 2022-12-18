/*
 * gridstorage.hpp
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

#ifndef SCHNEK_GRID_GRIDSTORAGE_HPP_
#define SCHNEK_GRID_GRIDSTORAGE_HPP_

#include "gridstorage/single-array-allocation.hpp"
#include "gridstorage/single-array-storage-base.hpp"

namespace schnek {
  template<typename T, size_t rank>
  using SingleArrayGridStorage = SingleArrayGridCOrderStorageBase<T, rank, SingleArrayInstantAllocation>;

  template<typename T, size_t rank>
  using SingleArrayGridStorageFortran = SingleArrayGridFortranOrderStorageBase<T, rank, SingleArrayInstantAllocation>;

  template<typename T, size_t rank>
  using LazyArrayGridStorage = SingleArrayGridCOrderStorageBase<T, rank, SingleArrayLazyAllocation>;

} // namespace schnek


#endif // SCHNEK_GRID_GRIDSTORAGE_HPP_

