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

#ifndef SCHNEK_GRIDSTORAGE_H_
#define SCHNEK_GRIDSTORAGE_H_



template<typename T, int rank>
class SingleArrayGridStorage
    : public SingleArrayGridCOrderStorageBase<T, rank, SingleArrayInstantAllocation>
{
  public:
    typedef SingleArrayGridCOrderStorageBase<T, rank, SingleArrayInstantAllocation> BaseType;
    typedef typename BaseType::IndexType IndexType;

    SingleArrayGridStorage() : BaseType() {}

    SingleArrayGridStorage(const IndexType &low_, const IndexType &high_)
        : BaseType(low_, high_) {}
};


template<typename T, int rank>
class SingleArrayGridStorageFortran
    : public SingleArrayGridFortranOrderStorageBase<T, rank, SingleArrayInstantFortranAllocation>
{
  public:
    typedef SingleArrayGridFortranOrderStorageBase<T, rank, SingleArrayInstantFortranAllocation> BaseType;
    typedef typename BaseType::IndexType IndexType;

    SingleArrayGridStorageFortran() : BaseType() {}

    SingleArrayGridStorageFortran(const IndexType &low_, const IndexType &high_)
        : BaseType(low_, high_) {}
};

template<typename T, int rank>
class LazyArrayGridStorage
    : public SingleArrayGridCOrderStorageBase<T, rank, SingleArrayLazyAllocation>
{
  public:
    typedef SingleArrayGridCOrderStorageBase<T, rank, SingleArrayLazyAllocation> BaseType;
    typedef typename BaseType::IndexType IndexType;

    LazyArrayGridStorage() : BaseType() {}

    LazyArrayGridStorage(const IndexType &low_, const IndexType &high_)
        : BaseType(low_, high_) {}
};

} // namespace schnek


#include "gridstorage.t"


#endif // SCHNEK_GSTORAGE_H_

