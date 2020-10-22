/*
 *  array_io.hpp
 *
 *  Created on: 12 Oct 2020
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 */

#ifndef SCHNEK_UTIL_ARRAY_IO_HPP_
#define SCHNEK_UTIL_ARRAY_IO_HPP_

#include "../grid/array.hpp"
#include "../grid/range.hpp"

#include <ostream>

template<
  class T,
  int rank,
  template<int> class CheckingPolicy
>
inline std::ostream &operator<<(std::ostream &out, const schnek::Array<T, rank, CheckingPolicy> a)
{
  out << "(";
  std::string sep = "";
  for (int i=0; i<rank; ++i)
  {
    out << sep << a[i];
    sep = ", ";
  }
  out << ")";
  return out;
}

template<
  class T,
  int rank,
  template<int> class CheckingPolicy
>
inline std::ostream &operator<<(std::ostream &out, const schnek::Range<T, rank, CheckingPolicy> r)
{
  return out << r.getLo() << "--" << r.getHi();
}


#endif /* SCHNEK_UTIL_ARRAY_IO_HPP_ */
