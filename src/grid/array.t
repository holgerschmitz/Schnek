/*
 * algo.t
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

#include <boost/static_assert.hpp>
#include <iostream>

namespace schnek
{

template<class T, size_t Length, template <size_t> class CheckingPolicy>
SCHNEK_INLINE Array<T,Length,CheckingPolicy>::Array()
{}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
template<template<size_t> class CheckingPolicy2>
SCHNEK_INLINE Array<T,Length,CheckingPolicy>::Array(const Array<T, Length, CheckingPolicy2> &arr)
{
  for (size_t i=0; i<Length; ++i)
    data[i] = arr[i];
}

// template<class T, size_t Length, template <size_t> class CheckingPolicy>
// inline Array<T,Length,CheckingPolicy>::Array(std::initializer_list<T> l)
// {
//   data = l;
// }

template<class T, size_t Length, template <size_t> class CheckingPolicy>
SCHNEK_INLINE Array<T,Length,CheckingPolicy>::Array(const T& v0)
{
  for (size_t i=0; i<Length; ++i) data[i] = v0;
}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
SCHNEK_INLINE Array<T,Length,CheckingPolicy>::Array(
                  const T& v0, const T& v1
)
{
  BOOST_STATIC_ASSERT(2==Length);
  data[0] = v0;
  data[1] = v1;
}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
SCHNEK_INLINE Array<T,Length,CheckingPolicy>::Array(
                  const T& v0, const T& v1, const T& v2
)
{
  BOOST_STATIC_ASSERT(3==Length);
  data[0] = v0;
  data[1] = v1;
  data[2] = v2;
}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
SCHNEK_INLINE Array<T,Length,CheckingPolicy>::Array(
                  const T& v0, const T& v1, const T& v2, const T& v3
)
{
  BOOST_STATIC_ASSERT(4==Length);
  data[0] = v0;
  data[1] = v1;
  data[2] = v2;
  data[3] = v3;
}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
SCHNEK_INLINE Array<T,Length,CheckingPolicy>::Array(
                  const T& v0, const T& v1, const T& v2, const T& v3, const T& v4
)
{
  BOOST_STATIC_ASSERT(5==Length);
  data[0] = v0;
  data[1] = v1;
  data[2] = v2;
  data[3] = v3;
  data[4] = v4;
}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
SCHNEK_INLINE Array<T,Length,CheckingPolicy>::Array(
                  const T& v0, const T& v1, const T& v2, const T& v3, const T& v4, 
                  const T& v5
)
{
  BOOST_STATIC_ASSERT(6==Length);
  data[0] = v0;
  data[1] = v1;
  data[2] = v2;
  data[3] = v3;
  data[4] = v4;
  data[5] = v5;
}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
SCHNEK_INLINE Array<T,Length,CheckingPolicy>::Array(
                  const T& v0, const T& v1, const T& v2, const T& v3, const T& v4, 
                  const T& v5, const T& v6
)
{
  BOOST_STATIC_ASSERT(7==Length);
  data[0] = v0;
  data[1] = v1;
  data[2] = v2;
  data[3] = v3;
  data[4] = v4;
  data[5] = v5;
  data[6] = v6;
}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
SCHNEK_INLINE Array<T,Length,CheckingPolicy>::Array(
                  const T& v0, const T& v1, const T& v2, const T& v3, const T& v4, 
                  const T& v5, const T& v6, const T& v7
)
{
  BOOST_STATIC_ASSERT(8==Length);
  data[0] = v0;
  data[1] = v1;
  data[2] = v2;
  data[3] = v3;
  data[4] = v4;
  data[5] = v5;
  data[6] = v6;
  data[7] = v7;
}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
SCHNEK_INLINE Array<T,Length,CheckingPolicy>::Array(
                  const T& v0, const T& v1, const T& v2, const T& v3, const T& v4, 
                  const T& v5, const T& v6, const T& v7, const T& v8
)
{
  BOOST_STATIC_ASSERT(9==Length);
  data[0] = v0;
  data[1] = v1;
  data[2] = v2;
  data[3] = v3;
  data[4] = v4;
  data[5] = v5;
  data[6] = v6;
  data[7] = v7;
  data[8] = v8;
}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
SCHNEK_INLINE Array<T,Length,CheckingPolicy>::Array(
                  const T& v0, const T& v1, const T& v2, const T& v3, const T& v4, 
                  const T& v5, const T& v6, const T& v7, const T& v8, const T& v9
)
{
  BOOST_STATIC_ASSERT(10==Length);
  data[0] = v0;
  data[1] = v1;
  data[2] = v2;
  data[3] = v3;
  data[4] = v4;
  data[5] = v5;
  data[6] = v6;
  data[7] = v7;
  data[8] = v8;
  data[9] = v9;
}

/// Accessor operator
template<class T, size_t Length, template <size_t> class CheckingPolicy>
SCHNEK_INLINE T& Array<T,Length,CheckingPolicy>::at(size_t pos)
{
 this->check(pos);
 return data[pos];
}

/// Constant accessor operator
template<class T, size_t Length, template <size_t> class CheckingPolicy>
SCHNEK_INLINE T Array<T,Length,CheckingPolicy>::at(size_t pos) const
{
 this->check(pos);
 return data[pos];
}

/// Accessor operator
template<class T, size_t Length, template <size_t> class CheckingPolicy>
SCHNEK_INLINE T& Array<T,Length,CheckingPolicy>::operator[](size_t pos)
{
  return at(pos);
}

/// Constant accessor operator
template<class T, size_t Length, template <size_t> class CheckingPolicy>
SCHNEK_INLINE T Array<T,Length,CheckingPolicy>::operator[](size_t pos) const
{
  return at(pos);
}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
template<class T2, template <size_t> class CheckingPolicy2>
Array<T,Length,CheckingPolicy> &Array<T,Length,CheckingPolicy>::operator=(const Array<T2,Length,CheckingPolicy2>& val)
{
  for (size_t i=0; i<Length; ++i)
    data[i] = val[i];
  return *this;
}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
template<class T2, template <size_t> class CheckingPolicy2>
Array<T,Length,CheckingPolicy> &Array<T,Length,CheckingPolicy>::operator+=(const Array<T2,Length,CheckingPolicy2>& val)
{
  for (size_t i=0; i<Length; ++i)
    data[i] += val[i];
  return *this;
}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
template<class T2, template <size_t> class CheckingPolicy2>
Array<T,Length,CheckingPolicy> &Array<T,Length,CheckingPolicy>::operator-=(const Array<T2,Length,CheckingPolicy2>& val)
{
  for (size_t i=0; i<Length; ++i)
    data[i] -= val[i];
  return *this;
}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
template<class T2, template <size_t> class CheckingPolicy2>
Array<T,Length,CheckingPolicy> &Array<T,Length,CheckingPolicy>::operator*=(const Array<T2,Length,CheckingPolicy2>& val)
{
  for (size_t i=0; i<Length; ++i)
    data[i] *= val[i];
  return *this;
}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
template<class T2, template <size_t> class CheckingPolicy2>
Array<T,Length,CheckingPolicy> &Array<T,Length,CheckingPolicy>::operator/=(const Array<T2,Length,CheckingPolicy2>& val)
{
  for (size_t i=0; i<Length; ++i)
    data[i] /= val[i];
  return *this;
}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
template<typename T2>
Array<T,Length,CheckingPolicy> &Array<T,Length,CheckingPolicy>::operator+=(const T2 val)
{
  for (size_t i=0; i<Length; ++i)
    data[i] += val;
  return *this;
}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
template<typename T2>
Array<T,Length,CheckingPolicy> &Array<T,Length,CheckingPolicy>::operator-=(const T2 val)
{
  for (size_t i=0; i<Length; ++i)
    data[i] -= val;
  return *this;
}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
template<typename T2>
Array<T,Length,CheckingPolicy> &Array<T,Length,CheckingPolicy>::operator*=(const T2 val)
{
  for (size_t i=0; i<Length; ++i)
    data[i] *= val;
  return *this;
}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
template<typename T2>
Array<T,Length,CheckingPolicy> &Array<T,Length,CheckingPolicy>::operator/=(const T2 val)
{
  for (size_t i=0; i<Length; ++i)
    data[i] /= val;
  return *this;
}



template<class T, size_t Length, template <size_t> class CheckingPolicy>
inline Array<T,Length,CheckingPolicy>& Array<T,Length,CheckingPolicy>::clear()
{
  for (size_t i=0; i<Length; ++i)
    data[i] = 0;
  return *this;
}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
inline Array<T,Length,CheckingPolicy>& Array<T,Length,CheckingPolicy>::fill(const T &value)
{
  for (size_t i=0; i<Length; ++i)
    data[i] = value;
  return *this;
}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
template<size_t destLength>
inline Array<T,destLength,CheckingPolicy> Array<T,Length,CheckingPolicy>::project() const
{
  BOOST_STATIC_ASSERT(destLength<=Length);

  Array<T,destLength,CheckingPolicy> result;
  for (size_t i=0; i<destLength; ++i)
    result[i] = data[i];
  return result;
}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
inline Array<T,Length-1,CheckingPolicy> Array<T,Length,CheckingPolicy>::projectDim(size_t dim) const
{
  Array<T,Length-1,CheckingPolicy> result;
  for (size_t i=0; i<dim; ++i)
  {
    result[i] = data[i];
  }
  for (size_t i=dim+1; i<Length; ++i)
  {
    result[i-1] = data[i];
  }
  return result;
}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
inline Array<T,Length,CheckingPolicy> Array<T,Length,CheckingPolicy>::Zero()
{
  return Array().fill(0);
}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
inline Array<T,Length,CheckingPolicy> Array<T,Length,CheckingPolicy>::Ones()
{
  return Array().fill(1);
}



template<class T, size_t Length, template <size_t> class CheckingPolicy>
inline T Array<T,Length,CheckingPolicy>::product() const
{
    T p(1);
    for (size_t i=0; i<Length; ++i) p *= at(i);
    return p;
}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
inline T Array<T,Length,CheckingPolicy>::sum() const
{
    T s(0);
    for (size_t i=0; i<Length; ++i) s += at(i);
    return s;
}

template<class T, size_t Length, template <size_t> class CheckingPolicy>
inline T Array<T,Length,CheckingPolicy>::sqr() const
{
    T s(0);
    for (size_t i=0; i<Length; ++i) {T v = at(i); s += v*v;}
    return s;
}


} // namespace


template<
  class T1, class T2,
  size_t Length, 
  template<size_t> class CheckingPolicy1, template<size_t> class CheckingPolicy2
>
bool operator==(
  const schnek::Array<T1,Length,CheckingPolicy1>& arr1,
  const schnek::Array<T2,Length,CheckingPolicy2>& arr2
)
{
  for (size_t i=0; i<Length; ++i) if (arr1[i]!=arr2[i]) return false;
  return true;
}

template<
  class T1, class T2,
  size_t Length, 
  template<size_t> class CheckingPolicy1, template<size_t> class CheckingPolicy2
>
bool operator!=(
  const schnek::Array<T1,Length,CheckingPolicy1>& arr1,
  const schnek::Array<T2,Length,CheckingPolicy2>& arr2
)
{
  for (size_t i=0; i<Length; ++i) if (arr1[i]!=arr2[i]) return true;
  return false;
}

template<
  class T1, class T2,
  size_t Length, 
  template<size_t> class CheckingPolicy1, template<size_t> class CheckingPolicy2
>
bool operator<(
  const schnek::Array<T1,Length,CheckingPolicy1>& arr1,
  const schnek::Array<T2,Length,CheckingPolicy2>& arr2
)
{
  for (size_t i=0; i<Length; ++i) if (arr1[i]>=arr2[i]) return false;
  return true;
}

template<
  class T1, class T2,
  size_t Length, 
  template<size_t> class CheckingPolicy1, template<size_t> class CheckingPolicy2
>
bool operator<=(
  const schnek::Array<T1,Length,CheckingPolicy1>& arr1,
  const schnek::Array<T2,Length,CheckingPolicy2>& arr2
)
{
  for (size_t i=0; i<Length; ++i) if (arr1[i]>arr2[i]) return false;
  return true;
}
