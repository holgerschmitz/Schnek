#include <iostream>

#ifndef SCHNEK_DATASTREAM_H
#define SCHNEK_DATASTREAM_H

#include "fixedarray.h"
#include "matrix.h"

template<
  class T, 
  int length, 
  template<int> class CheckingPolicy
>
      std::ostream &operator<<(std::ostream&, const schnek::FixedArray<T,length,CheckingPolicy>&);

template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
    std::ostream &operator<<(std::ostream&, const schnek::Matrix<T, 1, CheckingPolicy, StoragePolicy>&);

template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
    std::ostream &operator<<(std::ostream&, const schnek::Matrix<T, 2, CheckingPolicy, StoragePolicy>&);

template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
    std::ostream &operator<<(std::ostream&, const schnek::Matrix<T, 3, CheckingPolicy, StoragePolicy>&);

template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
    std::ostream &operator<<(std::ostream&, const schnek::Matrix<T, 4, CheckingPolicy, StoragePolicy>&);

template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
    std::ostream &operator<<(std::ostream&, const schnek::Matrix<T, 5, CheckingPolicy, StoragePolicy>&);

#include "datastream.t"


#endif 
