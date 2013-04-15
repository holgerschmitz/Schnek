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
    /** resizes to grid with lower indices low[0],...,low[rank-1]
     *  and upper indices high[0],...,high[rank-1] */
    void resize(const IndexType &low_, const IndexType &high_);
  private:
    /** */
    void deleteData();
    /** */
    void newData(const IndexType &low_, const IndexType &high_);
};

/** Stores the grid data in a single array
 *
 *  Layout of the data is in FORTRAN ordering.
 */
template<typename T, int rank, template<typename, int> class AllocationPolicy>
class SingleArrayGridStorageBase : public AllocationPolicy<T, rank> {
  public:

    class storage_iterator {
      protected:
        T* element;
        storage_iterator(T* element_) : element(element_) {}

        friend class SingleArrayGridStorage;

      public:
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

        friend class SingleArrayGridStorage;

      public:
        const T& operator*() { return *element;}
        const_storage_iterator &operator++() {++element; return *this;}
        bool operator==(const const_storage_iterator &SI) 
          { return element == SI.element; }
        bool operator!=(const const_storage_iterator &SI) 
          { return element != SI.element; }
    };
    
    SingleArrayGridStorage();
    
    SingleArrayGridStorage(const IndexType &low_, const IndexType &high_);

    ~SingleArrayGridStorage();


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
{};

template<typename T, int rank>
class LazyArrayGridStorage
    : public SingleArrayGridStorageBase<T, rank, SingleArrayLazyAllocation>
{};

/** Provides a
 */
template<typename T, typename InnerGrid>
class AdapterGridStorage {
  public:
    typedef typename InnerGrid::IndexType IndexType;
    typedef boost:shared_ptr<InnerGrid> pInnerGrid;

  protected:
    pInnerGrid grid;
    static const int byteRatio;
    static const int invByteRatio;

  private:
    int convertSize(int size)
    {
      if (byteRatio>0)
        return size/byteRatio;
      else
        return size*invByteRatio;
    }

  public:

    class storage_iterator {
      protected:
        T* element;
        storage_iterator(T* element_) : element(element_) {}

        friend class SingleArrayGridStorage;

      public:
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

        friend class SingleArrayGridStorage;

      public:
        const T& operator*() { return *element;}
        const_storage_iterator &operator++() {++element; return *this;}
        bool operator==(const const_storage_iterator &SI)
          { return element == SI.element; }
        bool operator!=(const const_storage_iterator &SI)
          { return element != SI.element; }
    };

    AdapterGridStorage() : grid(NULL) {}

    AdapterGridStorage(pInnerGrid grid_) : grid(grid_) {}

    /** resizes to grid with lower indices low[0],...,low[rank-1]
     *  and upper indices high[0],...,high[rank-1] */
    void resize(const IndexType &low_, const IndexType &high_);

    T &get(const IndexType &index);
    const T &get(const IndexType &index) const;

    T* getRawData() const { return (T*)(grid->getRawData()); }

    /** */
    IndexType getLo() const { return grid->getLow(); }
    /** */
    IndexType getHi() const
    {
      const IndexType &low = grid->getLow();
      IndexType high = grid->getHigh();
      high[0] = low[0]+convertSize(high[0]-low[0]+1)-1;
      return high;
    }
    /** */
    IndexType getDims() const
    {
      const IndexType dims = grid->getDims();
      dims[0] = convertSize(dims[0]);
      return dims;
    }

    /** */
    int getLo(int k) const { return getLo()[k]; }
    /** */
    int getHi(int k) const { return getHi()[k]; }
    /** */
    int getDims(int k) const { return getDims()[k]; }


    storage_iterator begin() { return storage_iterator(getRawData()); }
    storage_iterator end() { return storage_iterator(getRawData() + convertSize(grid->getSize())); }

    const_storage_iterator cbegin() const { return const_storage_iterator(getRawData()); }
    const_storage_iterator cend() const { return const_storage_iterator(getRawData() + convertSize(grid->getSize())); }

};

} // namespace schnek


#include "gridstorage.t"


#endif // SCHNEK_GSTORAGE_H_

