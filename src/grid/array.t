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

template<class T, int length, template <int> class CheckingPolicy>
inline Array<T,length,CheckingPolicy>::Array()
{}

template<class T, int length, template <int> class CheckingPolicy>
template<template<int> class CheckingPolicy2>
inline Array<T,length,CheckingPolicy>::Array(const Array<T, length, CheckingPolicy2> &arr)
{
  for (int i=0; i<length; ++i)
    data[i] = arr[i];
}

template<class T, int length, template <int> class CheckingPolicy>
inline Array<T,length,CheckingPolicy>::Array(const T& v0)
{
  for (int i=0; i<length; ++i) data[i] = v0;
}

template<class T, int length, template <int> class CheckingPolicy>
inline Array<T,length,CheckingPolicy>::Array(
                  const T& v0, const T& v1
)
{
  BOOST_STATIC_ASSERT(2==length);
  data[0] = v0;
  data[1] = v1;
}

template<class T, int length, template <int> class CheckingPolicy>
inline Array<T,length,CheckingPolicy>::Array(
                  const T& v0, const T& v1, const T& v2
)
{
  BOOST_STATIC_ASSERT(3==length);
  data[0] = v0;
  data[1] = v1;
  data[2] = v2;
}

template<class T, int length, template <int> class CheckingPolicy>
inline Array<T,length,CheckingPolicy>::Array(
                  const T& v0, const T& v1, const T& v2, const T& v3
)
{
  BOOST_STATIC_ASSERT(4==length);
  data[0] = v0;
  data[1] = v1;
  data[2] = v2;
  data[3] = v3;
}

template<class T, int length, template <int> class CheckingPolicy>
inline Array<T,length,CheckingPolicy>::Array(
                  const T& v0, const T& v1, const T& v2, const T& v3, const T& v4
)
{
  BOOST_STATIC_ASSERT(5==length);
  data[0] = v0;
  data[1] = v1;
  data[2] = v2;
  data[3] = v3;
  data[4] = v4;
}

template<class T, int length, template <int> class CheckingPolicy>
inline Array<T,length,CheckingPolicy>::Array(
                  const T& v0, const T& v1, const T& v2, const T& v3, const T& v4, 
                  const T& v5
)
{
  BOOST_STATIC_ASSERT(6==length);
  data[0] = v0;
  data[1] = v1;
  data[2] = v2;
  data[3] = v3;
  data[4] = v4;
  data[5] = v5;
}

template<class T, int length, template <int> class CheckingPolicy>
inline Array<T,length,CheckingPolicy>::Array(
                  const T& v0, const T& v1, const T& v2, const T& v3, const T& v4, 
                  const T& v5, const T& v6
)
{
  BOOST_STATIC_ASSERT(7==length);
  data[0] = v0;
  data[1] = v1;
  data[2] = v2;
  data[3] = v3;
  data[4] = v4;
  data[5] = v5;
  data[6] = v6;
}

template<class T, int length, template <int> class CheckingPolicy>
inline Array<T,length,CheckingPolicy>::Array(
                  const T& v0, const T& v1, const T& v2, const T& v3, const T& v4, 
                  const T& v5, const T& v6, const T& v7
)
{
  BOOST_STATIC_ASSERT(8==length);
  data[0] = v0;
  data[1] = v1;
  data[2] = v2;
  data[3] = v3;
  data[4] = v4;
  data[5] = v5;
  data[6] = v6;
  data[7] = v7;
}

template<class T, int length, template <int> class CheckingPolicy>
inline Array<T,length,CheckingPolicy>::Array(
                  const T& v0, const T& v1, const T& v2, const T& v3, const T& v4, 
                  const T& v5, const T& v6, const T& v7, const T& v8
)
{
  BOOST_STATIC_ASSERT(9==length);
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

template<class T, int length, template <int> class CheckingPolicy>
inline Array<T,length,CheckingPolicy>::Array(
                  const T& v0, const T& v1, const T& v2, const T& v3, const T& v4, 
                  const T& v5, const T& v6, const T& v7, const T& v8, const T& v9
)
{
  BOOST_STATIC_ASSERT(10==length);
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
template<class T, int length, template <int> class CheckingPolicy>
inline T& Array<T,length,CheckingPolicy>::at(int pos)
{
 this->check(pos);
 return data[pos];
}

/// Constant accessor operator
template<class T, int length, template <int> class CheckingPolicy>
inline T Array<T,length,CheckingPolicy>::at(int pos) const
{
 this->check(pos);
 return data[pos];
}

/// Accessor operator
template<class T, int length, template <int> class CheckingPolicy>
inline T& Array<T,length,CheckingPolicy>::operator[](int pos)
{
  return at(pos);
}

/// Constant accessor operator
template<class T, int length, template <int> class CheckingPolicy>
inline T Array<T,length,CheckingPolicy>::operator[](int pos) const
{
  return at(pos);
}

template<class T, int length, template <int> class CheckingPolicy>
template<class T2, template <int> class CheckingPolicy2>
Array<T,length,CheckingPolicy> &Array<T,length,CheckingPolicy>::operator=(const Array<T2,length,CheckingPolicy2>& val)
{
  for (int i=0; i<length; ++i)
    data[i] = val[i];
  return *this;
}

template<class T, int length, template <int> class CheckingPolicy>
template<class T2, template <int> class CheckingPolicy2>
Array<T,length,CheckingPolicy> &Array<T,length,CheckingPolicy>::operator+=(const Array<T2,length,CheckingPolicy2>& val)
{
  for (int i=0; i<length; ++i)
    data[i] += val[i];
  return *this;
}

template<class T, int length, template <int> class CheckingPolicy>
template<class T2, template <int> class CheckingPolicy2>
Array<T,length,CheckingPolicy> &Array<T,length,CheckingPolicy>::operator-=(const Array<T2,length,CheckingPolicy2>& val)
{
  for (int i=0; i<length; ++i)
    data[i] -= val[i];
  return *this;
}

template<class T, int length, template <int> class CheckingPolicy>
template<typename T2>
Array<T,length,CheckingPolicy> &Array<T,length,CheckingPolicy>::operator+=(const T2 val)
{
  for (int i=0; i<length; ++i)
    data[i] += val;
  return *this;
}

template<class T, int length, template <int> class CheckingPolicy>
template<typename T2>
Array<T,length,CheckingPolicy> &Array<T,length,CheckingPolicy>::operator-=(const T2 val)
{
  for (int i=0; i<length; ++i)
    data[i] -= val;
  return *this;
}

template<class T, int length, template <int> class CheckingPolicy>
template<typename T2>
Array<T,length,CheckingPolicy> &Array<T,length,CheckingPolicy>::operator*=(const T2 val)
{
  for (int i=0; i<length; ++i)
    data[i] *= val;
  return *this;
}

template<class T, int length, template <int> class CheckingPolicy>
template<typename T2>
Array<T,length,CheckingPolicy> &Array<T,length,CheckingPolicy>::operator/=(const T2 val)
{
  for (int i=0; i<length; ++i)
    data[i] /= val;
  return *this;
}



template<class T, int length, template <int> class CheckingPolicy>
inline Array<T,length,CheckingPolicy>& Array<T,length,CheckingPolicy>::clear()
{
  for (int i=0; i<length; ++i)
    data[i] = 0;
  return *this;
}

template<class T, int length, template <int> class CheckingPolicy>
inline Array<T,length,CheckingPolicy>& Array<T,length,CheckingPolicy>::fill(const T &value)
{
  for (int i=0; i<length; ++i)
    data[i] = value;
  return *this;
}

template<class T, int length, template <int> class CheckingPolicy>
template<int destLength>
inline Array<T,destLength,CheckingPolicy> Array<T,length,CheckingPolicy>::project()
{
  BOOST_STATIC_ASSERT(destLength<=length);

  Array<T,destLength,CheckingPolicy> result;
  for (int i=0; i<destLength; ++i)
    result[i] = data[i];
  return result;
}

template<class T, int length, template <int> class CheckingPolicy>
inline Array<T,length,CheckingPolicy> Array<T,length,CheckingPolicy>::Zero()
{
  return Array().fill(0);
}

template<class T, int length, template <int> class CheckingPolicy>
inline Array<T,length,CheckingPolicy> Array<T,length,CheckingPolicy>::Unity()
{
  return Array().fill(1);
}



template<class T, int length, template <int> class CheckingPolicy>
inline T Array<T,length,CheckingPolicy>::product() const
{
    T p(1);
    for (int i=0; i<Length; ++i) p *= at(i);
    return p;
}

template<class T, int length, template <int> class CheckingPolicy>
inline T Array<T,length,CheckingPolicy>::sum() const
{
    T s(0);
    for (int i=0; i<Length; ++i) s += at(i);
    return s;
}

template<class T, int length, template <int> class CheckingPolicy>
inline T Array<T,length,CheckingPolicy>::sqr() const
{
    T s(0);
    for (int i=0; i<Length; ++i) {T v = at(i); s += v*v;}
    return s;
}


} // namespace


template<
  class T1, class T2,
  int length, 
  template<int> class CheckingPolicy1, template<int> class CheckingPolicy2
>
bool operator==(
  const schnek::Array<T1,length,CheckingPolicy1>& arr1,
  const schnek::Array<T2,length,CheckingPolicy2>& arr2
)
{
  for (int i=0; i<length; ++i) if (arr1[i]!=arr2[i]) return false;
  return true;
}

template<
  class T1, class T2,
  int length, 
  template<int> class CheckingPolicy1, template<int> class CheckingPolicy2
>
bool operator!=(
  const schnek::Array<T1,length,CheckingPolicy1>& arr1,
  const schnek::Array<T2,length,CheckingPolicy2>& arr2
)
{
  for (int i=0; i<length; ++i) if (arr1[i]!=arr2[i]) return true;
  return false;
}

template<
  class T1, class T2,
  int length, 
  template<int> class CheckingPolicy1, template<int> class CheckingPolicy2
>
bool operator<(
  const schnek::Array<T1,length,CheckingPolicy1>& arr1,
  const schnek::Array<T2,length,CheckingPolicy2>& arr2
)
{
  for (int i=0; i<length; ++i) if (arr1[i]>=arr2[i]) return false;
  return true;
}

template<
  class T1, class T2,
  int length, 
  template<int> class CheckingPolicy1, template<int> class CheckingPolicy2
>
bool operator<=(
  const schnek::Array<T1,length,CheckingPolicy1>& arr1,
  const schnek::Array<T2,length,CheckingPolicy2>& arr2
)
{
  for (int i=0; i<length; ++i) if (arr1[i]>arr2[i]) return false;
  return true;
}
