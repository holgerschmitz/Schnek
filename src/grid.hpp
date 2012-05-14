/*
 * grid.h
 *
 *  Created on: 26 Apr 2010
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
 */

#ifndef SCHNEK_GRID_H
#define SCHNEK_GRID_H

#include "array.hpp"
#include "gridcheck.hpp"
#include "gstorage.hpp"

namespace schnek {

template<class GridType, typename TList>
class IndexedGrid;

template<class IndexType>
struct IndexCast;


/** An elementary grid class */
template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy = GridNoArgCheck,
  template<typename, int> class StoragePolicy = SingleArrayGridStorage
>
class Grid : public StoragePolicy<T,rank>, public CheckingPolicy<rank> {

  public:
    typedef T value_type;
    typedef FixedArray<int,rank> IndexType;
    typedef Grid<T,rank,CheckingPolicy,StoragePolicy> GridType;
    enum {Rank = rank};
  
    /** default constructor creates an empty grid */
    Grid();

    /** constructor, which builds Grid of size size[0] x ... x size[rank-1]
     * 
     *  Example: 
     *  \begin{verbatim}
     *  Grid<double,2>::IndexType size=(512,512);
     *  Grid<double,2> m(size);
     *  \end{verbatim}
     * 
     *  The ranges then extend from 0 to size[i]-1
     */
    Grid(const IndexType &size);

    /** constructor, which builds Grid with lower indices low[0],...,low[rank-1]
     *  and upper indices high[0],...,high[rank-1]
     * 
     *  Example: 
     *  \begin{verbatim}
     *  Grid<double,2>::IndexType low(-5,-10);
     *  Grid<double,2>::IndexType high(15,36);
     *  Grid<double,2> m(l,h);
     *  \end{verbatim}
     * 
     *  The ranges then extend from low[i] to high[i]
     */
    Grid(const IndexType &low, const IndexType &high);

    /** copy constructor */
    Grid(const Grid<T, rank, CheckingPolicy, StoragePolicy>&);
    
    /** index operator, writing */
    T& operator[](const IndexType& pos); // write
    /** index operator, reading */
    T  operator[](const IndexType& pos) const; // read
    /** index operator, writing */
    T& operator()(int i);
    /** index operator, reading */
    T  operator()(int i) const;
    /** index operator, writing */
    T& operator()(int i, int j);
    /** index operator, reading */
    T  operator()(int i, int j) const;
    /** index operator, writing */
    T& operator()(int i, int j, int k);
    /** index operator, reading */
    T  operator()(int i, int j, int k) const;
    /** index operator, writing */
    T& operator()(int i, int j, int k, int l);
    /** index operator, reading */
    T  operator()(int i, int j, int k, int l) const;
    /** index operator, writing */
    T& operator()(int i, int j, int k, int l, int m);
    /** index operator, reading */
    T  operator()(int i, int j, int k, int l, int m) const;

    template<typename Arg0>
    IndexedGrid<GridType, TYPELIST_1(Arg0) > operator()(
      const Arg0 &i0
    );

    template<typename Arg0, typename Arg1>
    IndexedGrid<GridType, TYPELIST_2(Arg0, Arg1) > operator()(
      const Arg0 &i0, const Arg1 &i1
    );
        
    /** assign another grid */
    Grid<T, rank, CheckingPolicy, StoragePolicy>& 
      operator=(const Grid<T, rank, CheckingPolicy, StoragePolicy>&);

    /** assign a value */
    Grid<T, rank, CheckingPolicy, StoragePolicy>& 
      operator=(const T &val);
    
    template<
      template<int> class CheckingPolicy2,
      template<typename, int> class StoragePolicy2
    >
    Grid<T, rank, CheckingPolicy, StoragePolicy>&
      operator-=(Grid<T, rank, CheckingPolicy2, StoragePolicy2>&);

    template<
      template<int> class CheckingPolicy2,
      template<typename, int> class StoragePolicy2
    >
    Grid<T, rank, CheckingPolicy, StoragePolicy>&
      operator+=(Grid<T, rank, CheckingPolicy2, StoragePolicy2>&);

    /** Resize to size[0] x ... x size[rank-1]
     * 
     *  Example: 
     *  \begin{verbatim}
     *  Grid<double,2>::IndexType size=(512,512);
     *  Grid<double,2> m;
     *  m.resize(size);
     *  \end{verbatim}
     * 
     *  The ranges then extend from 0 to size[i]-1
     */        
    void resize(const IndexType &size);

    /** Resize to lower indices low[0],...,low[rank-1]
     *  and upper indices high[0],...,high[rank-1]
     * 
     *  Example: 
     *  \begin{verbatim}
     *  Grid<double,2>::IndexType low(-5,-10);
     *  Grid<double,2>::IndexType high(15,36);
     *  Grid<double,2> m;
     *  m.resize(l,h);
     *  \end{verbatim}
     * 
     *  The ranges then extend from low[i] to high[i]
     */
    void resize(const IndexType &low, const IndexType &high);
   
    /** Resize to match the size of another matrix */
    void resize(const Grid<T, rank>& matr);
  private:
    // assumes that the sizes are already set properly
    void copyFromGrid(const Grid<T, rank, CheckingPolicy, StoragePolicy>& matr);
};

} // namespace schnek

#include "grid.t"

#endif
