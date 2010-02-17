#ifndef SCHNEK_MSTORAGE_H
#define SCHNEK_MSTORAGE_H

namespace schnek {


template<typename T, int rank>
class SingleArrayMatrixStorage {
  public:  
    typedef FixedArray<int,rank> IndexType;
    
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

        friend class SingleArrayMatrixStorage;

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

        friend class SingleArrayMatrixStorage;

      public:
        const T& operator*() { return *element;}
        const_storage_iterator &operator++() {element++; return *this;}
        bool operator==(const const_storage_iterator &SI) 
          { return element == SI.element; }
        bool operator!=(const const_storage_iterator &SI) 
          { return element != SI.element; }
    };
    
    SingleArrayMatrixStorage();
    
    SingleArrayMatrixStorage(const IndexType &low_, const IndexType &high_);

    virtual ~SingleArrayMatrixStorage();

    /** resizes to matrix with lower indices low[0],...,low[rank-1]
     *  and upper indices high[0],...,high[rank-1] */
    void resize(const IndexType &low_, const IndexType &high_);

    T &get(const IndexType &index);
    const T &get(const IndexType &index) const;
    
    /** */
    const IndexType& getLow() const { return low; }
    /** */
    const IndexType& getHigh() const { return high; }
    /** */
    const IndexType& getDims() const { return dims; }

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


#include "mstorage.t"


#endif

