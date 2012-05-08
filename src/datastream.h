#include <iostream>

#ifndef SCHNEK_DATASTREAM_H
#define SCHNEK_DATASTREAM_H

#include "fixedarray.h"
#include "grid.h"

template<
  class T, 
  int length, 
  template<int> class CheckingPolicy
>
      std::ostream &operator<<(std::ostream&, const schnek::Array<T,length,CheckingPolicy>&);

template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
    std::ostream &operator<<(std::ostream&, const schnek::Grid<T, 1, CheckingPolicy, StoragePolicy>&);

template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
    std::ostream &operator<<(std::ostream&, const schnek::Grid<T, 2, CheckingPolicy, StoragePolicy>&);

template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
    std::ostream &operator<<(std::ostream&, const schnek::Grid<T, 3, CheckingPolicy, StoragePolicy>&);

template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
    std::ostream &operator<<(std::ostream&, const schnek::Grid<T, 4, CheckingPolicy, StoragePolicy>&);

template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
    std::ostream &operator<<(std::ostream&, const schnek::Grid<T, 5, CheckingPolicy, StoragePolicy>&);

#include "datastream.t"


#endif 
