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

#include "array.hpp"

namespace schnek {

template<typename T, int rank>
class SingleArrayInstantAllocation
{
  public:  
    typedef Array<int,rank> IndexType;
    
  protected:
    T* data;
    T* data_fast;
    int size;
    IndexType low;
    IndexType high;
    IndexType dims;

  public:
    ~SingleArrayInstantAllocation();
    /** resizes to grid with lower indices low[0],...,low[rank-1]
     *  and upper indices high[0],...,high[rank-1] */
    void resize(const IndexType &low_, const IndexType &high_);
  private:
    /** */
    void deleteData();
    /** */
    void newData(const IndexType &low_, const IndexType &high_);
};

template<typename T, int rank>
class SingleArrayLazyAllocation
{
  public:
    typedef Array<int,rank> IndexType;

  protected:
    T* data;
    T* data_fast;
    int size;
    IndexType low;
    IndexType high;
    IndexType dims;

  private:
    int bufSize;
    double avgSize;
    double avgVar;
    double r;

  public:
    SingleArrayLazyAllocation();

    ~SingleArrayLazyAllocation();
    /** resizes to grid with lower indices low[0],...,low[rank-1]
     *  and upper indices high[0],...,high[rank-1] */
    void resize(const IndexType &low_, const IndexType &high_);
  private:
    /** */
    void deleteData();
    /** */
    void newData(int size);
};

/** Stores the grid data in a single array
 *
 *  Layout of the data is in FORTRAN ordering.
 */
template<typename T, int rank, template<typename, int> class AllocationPolicy>
class SingleArrayGridStorageBase : public AllocationPolicy<T, rank> {
  public:
    typedef Array<int,rank> IndexType;

  protected:
    T* data;
    T* data_fast;
    int size;
    IndexType low;
    IndexType high;
    IndexType dims;
  public:
    class storage_iterator {
      protected:
        T* element;
        storage_iterator(T* element_) : element(element_) {}

        friend class SingleArrayGridStorageBase;

      public:
        storage_iterator(const storage_iterator &it) : element(it.element) {}
        T& operator*() { return *element;}
        storage_iterator &operator++() {++element; return *this;}
        bool operator==(const storage_iterator &SI) 
          { return element == SI.element; }
        bool operator!=(const storage_iterator &SI) 
          { return element != SI.element; }
    };

    class const_storage_iterator {
      protected:
        const T* element;
        const_storage_iterator(const T* element_) : element(element_) {}

        friend class SingleArrayGridStorageBase;

      public:
        const T& operator*() { return *element;}
        const_storage_iterator &operator++() {++element; return *this;}
        bool operator==(const const_storage_iterator &SI) 
          { return element == SI.element; }
        bool operator!=(const const_storage_iterator &SI) 
          { return element != SI.element; }
    };
    
    SingleArrayGridStorageBase();
    
    SingleArrayGridStorageBase(const IndexType &low_, const IndexType &high_);

    T &get(const IndexType &index);
    const T &get(const IndexType &index) const;
    
    T* getRawData() const { return data; }

    /** */
    const IndexType& getLo() const { return low; }
    /** */
    const IndexType& getHi() const { return high; }
    /** */
    const IndexType& getDims() const { return dims; }

    /** */
    int getLo(int k) const { return low[k]; }
    /** */
    int getHi(int k) const { return high[k]; }
    /** */
    int getDims(int k) const { return dims[k]; }

    int getSize() const { return size; }

    storage_iterator begin() { return storage_iterator(data); }
    storage_iterator end() { return storage_iterator(data + size); }

    const_storage_iterator cbegin() const { return const_storage_iterator(data); }
    const_storage_iterator cend() const { return const_storage_iterator(data + size); }

};

template<typename T, int rank>
class SingleArrayGridStorage
    : public SingleArrayGridStorageBase<T, rank, SingleArrayInstantAllocation>
{
  public:
    typedef SingleArrayGridStorageBase<T, rank, SingleArrayInstantAllocation> BaseType;
    typedef typename BaseType::IndexType IndexType;

    SingleArrayGridStorage() : BaseType() {}

    SingleArrayGridStorage(const IndexType &low_, const IndexType &high_)
        : BaseType(low_, high_) {}
};

template<typename T, int rank>
class LazyArrayGridStorage
    : public SingleArrayGridStorageBase<T, rank, SingleArrayLazyAllocation>
{
  public:
    typedef SingleArrayGridStorageBase<T, rank, SingleArrayLazyAllocation> BaseType;
    typedef typename BaseType::IndexType IndexType;

    LazyArrayGridStorage() : BaseType() {}

    LazyArrayGridStorage(const IndexType &low_, const IndexType &high_)
        : BaseType(low_, high_) {}
};

} // namespace schnek


#include "gridstorage.t"


#endif // SCHNEK_GSTORAGE_H_

