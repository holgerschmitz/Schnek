#include "gexpression.h"

//==============================================================
//================== One Argument  =============================
//==============================================================
/*
template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
template<
  int rank,
  template<int> class ACheckingPolicy,
  template<typename, int> class AStoragePolicy
>
const IndexedGrid<T,1,CheckingPolicy,StoragePolicy>& 
  IndexedGrid<T,1,CheckingPolicy,StoragePolicy>::assign(
    const IndexedGrid<T,rank,ACheckingPolicy,AStoragePolicy>& RHS
  ) const
{
  i1.setLow(M.getLow()[0]);
  i1.setHigh(M.getHigh()[0]);
  
  int &c1 = i1();
  
  for (c1 = i1.getLow(); c1<=i1.getHigh(); c1++)
    M(c1) = RHS();
      
  return *this;
}


template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
template<
  int rank,
  template<int> class ACheckingPolicy,
  template<typename, int> class AStoragePolicy
>
const IndexedGrid<T,1,CheckingPolicy,StoragePolicy>& 
  IndexedGrid<T,1,CheckingPolicy,StoragePolicy>::operator=(
    const IndexedGrid<T,rank,ACheckingPolicy,AStoragePolicy>& RHS
  ) const
{
  return assign(RHS);
}

template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
const IndexedGrid<T,1,CheckingPolicy,StoragePolicy>& 
  IndexedGrid<T,1,CheckingPolicy,StoragePolicy>::operator=(
    const IndexedGrid<T,1,CheckingPolicy,StoragePolicy>& RHS
  ) const
{
  return assign(RHS);
}

//==============================================================
//================== Two Arguments =============================
//==============================================================

template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
template<
  int rank,
  template<int> class ACheckingPolicy,
  template<typename, int> class AStoragePolicy
>
const IndexedGrid<T,2,CheckingPolicy,StoragePolicy>& 
  IndexedGrid<T,2,CheckingPolicy,StoragePolicy>::assign(
    const IndexedGrid<T,rank,ACheckingPolicy,AStoragePolicy>& RHS
  ) const
{
  i1.setLow(M.getLow()[0]);
  i1.setHigh(M.getHigh()[0]);
  
  i2.setLow(M.getLow()[1]);
  i2.setHigh(M.getHigh()[1]);
  
  int &c1 = i1();
  int &c2 = i2();
  
  for (c1 = i1.getLow(); c1<=i1.getHigh(); c1++)
    for (c2 = i2.getLow(); c2<=i2.getHigh(); c2++)
      M(c1,c2) = RHS();
      
  return *this;
}


template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
template<
  int rank,
  template<int> class ACheckingPolicy,
  template<typename, int> class AStoragePolicy
>
const IndexedGrid<T,2,CheckingPolicy,StoragePolicy>& 
  IndexedGrid<T,2,CheckingPolicy,StoragePolicy>::operator=(
    const IndexedGrid<T,rank,ACheckingPolicy,AStoragePolicy>& RHS
  ) const
{
  return assign(RHS);
}

template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
const IndexedGrid<T,2,CheckingPolicy,StoragePolicy>& 
  IndexedGrid<T,2,CheckingPolicy,StoragePolicy>::operator=(
    const IndexedGrid<T,2,CheckingPolicy,StoragePolicy>& RHS
  ) const
{
  return assign(RHS);
}
*/
