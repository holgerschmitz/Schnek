#ifndef SCHNEK_GSTORAGE_H
#define SCHNEK_GSTORAGE_H

namespace schnek {


/** Stores the grid data in a single array
 *
 *  Layout of the data is in FORTRAN ordering.
 */
template<typename T, int rank>
class SingleArrayGridStorage {
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

        friend class SingleArrayGridStorage;

      public:
        T& operator*() { return *element;}
        storage_iterator &operator++() {element++; return *this;}
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
        const_storage_iterator &operator++() {element++; return *this;}
        bool operator==(const const_storage_iterator &SI) 
          { return element == SI.element; }
        bool operator!=(const const_storage_iterator &SI) 
          { return element != SI.element; }
    };
    
    SingleArrayGridStorage();
    
    SingleArrayGridStorage(const IndexType &low_, const IndexType &high_);

    virtual ~SingleArrayGridStorage();

    /** resizes to grid with lower indices low[0],...,low[rank-1]
     *  and upper indices high[0],...,high[rank-1] */
    void resize(const IndexType &low_, const IndexType &high_);

    T &get(const IndexType &index);
    const T &get(const IndexType &index) const;
    
    T* getRawData() const { return data; }

    /** */
    const IndexType& getLow() const { return low; }
    /** */
    const IndexType& getHigh() const { return high; }
    /** */
    const IndexType& getDims() const { return dims; }

    /** */
    int getLow(int k) const { return low[k]; }
    /** */
    int getHigh(int k) const { return high[k]; }
    /** */
    int getDims(int k) const { return dims[k]; }

    storage_iterator begin() { return storage_iterator(data); }
    storage_iterator end() { return storage_iterator(data + size); }

    const_storage_iterator cbegin() const { return const_storage_iterator(data); }
    const_storage_iterator cend() const { return const_storage_iterator(data + size); }

  private:
    /** */
    virtual void deleteData();
    /** */
    virtual void newData(const IndexType &low_, const IndexType &high_);

};

} // namespace schnek


#include "gstorage.t"


#endif

