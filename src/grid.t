
#include "farrexpression.h"

// --------------------------------------------------------------
// implementation

namespace schnek
{

template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
Grid<T, rank, CheckingPolicy, StoragePolicy>::Grid() : StoragePolicy<T,rank>()
{}

template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
Grid<T, rank, CheckingPolicy, StoragePolicy>::Grid(const IndexType &size)
  : StoragePolicy<T,rank>(IndexType::Zero(), size-IndexType::Unity())
{}

template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
Grid<T, rank, CheckingPolicy, StoragePolicy>::Grid(const IndexType &low, const IndexType &high)
  : StoragePolicy<T,rank>(low,high)
{}

template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
Grid<T, rank, CheckingPolicy, StoragePolicy>
  ::Grid(const Grid<T, rank, CheckingPolicy, StoragePolicy>& matr) 
  : StoragePolicy<T,rank>(matr.getLow(), matr.getHigh())
{
  this->copyFromGrid(matr);  
}

template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
Grid<T, rank, CheckingPolicy, StoragePolicy>& 
  Grid<T, rank, CheckingPolicy, StoragePolicy>
    ::operator=(const Grid<T, rank, CheckingPolicy, StoragePolicy> &matr)
{
  this->resize(matr);
  this->copyFromGrid(matr);
  return *this;
}

template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
Grid<T, rank, CheckingPolicy, StoragePolicy>& 
  Grid<T, rank, CheckingPolicy, StoragePolicy>
    ::operator=(const T &val)
{
  typedef typename StoragePolicy<T,rank>::storage_iterator Iterator;
  Iterator end = this->end();
  Iterator dest = this->begin();
  while (dest != end)
  {
    *dest = val;
    ++dest;
  }
  
  return *this;
}


template<
  typename T,
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
template<
  template<int> class CheckingPolicy2,
  template<typename, int> class StoragePolicy2
>
Grid<T, rank, CheckingPolicy, StoragePolicy>&
  Grid<T, rank, CheckingPolicy, StoragePolicy>
    ::operator-=(Grid<T, rank, CheckingPolicy2, StoragePolicy2>& grid)
{
  typedef typename StoragePolicy<T,rank>::storage_iterator Iterator;
  typedef typename StoragePolicy2<T,rank>::storage_iterator Iterator2;
  Iterator dest = this->begin();
  Iterator2 src = grid.begin();
  Iterator2 end = grid.end();
  while (src != end)
  {
    *dest -= *src;
    ++dest;
    ++src;
  }

  return *this;
}


template<
  typename T,
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
template<
  template<int> class CheckingPolicy2,
  template<typename, int> class StoragePolicy2
>
Grid<T, rank, CheckingPolicy, StoragePolicy>&
  Grid<T, rank, CheckingPolicy, StoragePolicy>
    ::operator+=(Grid<T, rank, CheckingPolicy2, StoragePolicy2>& grid)
{
  typedef typename StoragePolicy<T,rank>::storage_iterator Iterator;
  typedef typename StoragePolicy2<T,rank>::storage_iterator Iterator2;
  Iterator dest = this->begin();
  Iterator2 src = grid.begin();
  Iterator2 end = grid.end();
  while (src != end)
  {
    *dest += *src;
    ++dest;
    ++src;
  }

  return *this;
}

// -------------------------------------------------------------
// inline functions

template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
inline T& Grid<T, rank, CheckingPolicy, StoragePolicy>
  ::operator[](const IndexType& pos)
{
  return get(this->check(pos,this->getLow(),this->getHigh()));
}

template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
inline T Grid<T, rank, CheckingPolicy, StoragePolicy>
  ::operator[](const IndexType& pos) const
{
  return get(this->check(pos,this->getLow(),this->getHigh()));
}

template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
inline  T& Grid<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i)
{
  return get(this->check(IndexType(i),this->getLow(),this->getHigh()));
}

template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
inline  T Grid<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i) const
{
  return get(this->check(IndexType(i),this->getLow(),this->getHigh()));
}

template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
inline  T& Grid<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j)
{
  return get(this->check(IndexType(i,j),this->getLow(),this->getHigh()));
}

template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
inline  T Grid<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j) const
{
  return get(this->check(IndexType(i,j),this->getLow(),this->getHigh()));
}

template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
inline  T& Grid<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k) 
{
  return get(this->check(IndexType(i,j,k),this->getLow(),this->getHigh()));
}

template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
inline  T Grid<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k) const
{
  return get(this->check(IndexType(i,j,k),this->getLow(),this->getHigh()));
}

template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
inline  T& Grid<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k, int l) 
{
  return get(this->check(IndexType(i,j,k,l),this->getLow(),this->getHigh()));
}

template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
inline  T Grid<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k, int l) const
{
  return get(this->check(IndexType(i,j,k,l),this->getLow(),this->getHigh()));
}

template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
inline  T& Grid<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k, int l, int m) 
{
  return get(this->check(IndexType(i,j,k,l,m),this->getLow(),this->getHigh()));
}

template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
inline  T Grid<T, rank, CheckingPolicy, StoragePolicy>::operator ()(int i, int j, int k, int l, int m) const
{
  return get(this->check(IndexType(i,j,k,l,m),this->getLow(),this->getHigh()));
}


template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
template<typename Arg0>
IndexedGrid<
  Grid<T, rank, CheckingPolicy, StoragePolicy>,
  TYPELIST_1(Arg0)
>
Grid<T, rank, CheckingPolicy, StoragePolicy>::operator()(
  const Arg0 &i0
)
{
  return IndexedGrid<GridType,TYPELIST_1(Arg0)> (i0);
}

template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
template<typename Arg0, typename Arg1>
IndexedGrid<
  Grid<T, rank, CheckingPolicy, StoragePolicy>,
  TYPELIST_2(Arg0, Arg1)
>
Grid<T, rank, CheckingPolicy, StoragePolicy>::operator()(
  const Arg0 &i0, const Arg1 &i1
)
{
  return IndexedGrid<GridType,TYPELIST_2(Arg0, Arg1)> (i0, i1);
}

template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
void Grid<T, rank, CheckingPolicy, StoragePolicy>::resize(const IndexType &d)
{
  IndexType high;
  high = d-IndexType::Unity();
  StoragePolicy<T,rank>::resize(IndexType::Zero(), high);
}

template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
void Grid<T, rank, CheckingPolicy, StoragePolicy>::resize(const IndexType &low, const IndexType &high)
{
  StoragePolicy<T,rank>::resize(low, high);
}

template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
void Grid<T, rank, CheckingPolicy, StoragePolicy>::resize(const Grid<T, rank>& matr)
{
  StoragePolicy<T,rank>::resize(matr.getLow(), matr.getHigh());
}

template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
void Grid<T, rank, CheckingPolicy, StoragePolicy>
  ::copyFromGrid(const Grid<T, rank, CheckingPolicy, StoragePolicy>& matr)
{
  typedef typename StoragePolicy<T,rank>::const_storage_iterator CIterator;
  typedef typename StoragePolicy<T,rank>::storage_iterator Iterator;
  CIterator src = matr.cbegin();
  CIterator srcEnd = matr.cend();
  Iterator dest = this->begin();
  while (src != srcEnd)
  {
    *dest = *src;
    ++src;
    ++dest;
  }
}

}
