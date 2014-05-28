/*
 * range.hpp
 *
 * Created on: 31 Aug 2012
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

#ifndef SCHNEK_RANGE_HPP_
#define SCHNEK_RANGE_HPP_

#include "array.hpp"

namespace schnek {

/** Range is a rectangular domain defining two corners
 */
template<
  class T,
  int rank,
  template<int> class CheckingPolicy = ArrayNoArgCheck
>
class Range {
  public:
    typedef Array<T,rank,CheckingPolicy> LimitType;
  private:
    /// Minimum and maximum corners of the rectangle
    LimitType lo, hi;
  public:
    /// Default constructor
    Range() : lo(0), hi(0) {};

    /// Construct with rectangle minimum and maximum

    template<template<int> class ArrayCheckingPolicy>
    Range(const Array<T,rank,ArrayCheckingPolicy> &lo_, const Array<T,rank,ArrayCheckingPolicy> &hi_)
    : lo(lo_), hi(hi_) {}

    /// Copy constructor
    template<template<int> class ArrayCheckingPolicy>
    Range(const Range<T,rank,ArrayCheckingPolicy> &domain)
    : lo(domain.getLo()), hi(domain.getHi()) {}

    /// Assignment operator
    Range &operator=(const Range &domain)
    {
      lo = domain.lo;
      hi = domain.hi;
      return *this;
    }

    /// Return rectangle minimum
    const LimitType &getLo() const {
      return lo;
    }
    /// Return rectangle maximum
    const LimitType &getHi() const {
      return hi;
    }

    /// Return rectangle minimum
    LimitType &getLo() {return lo;}
    /// Return rectangle maximum
    LimitType &getHi() {return hi;}

    void grow(const T &s) {
      for (int i=0; i<rank; ++i)
      {
        lo[i] -= s;
        hi[i] += s;
      }
    }
};

} // namespace 

#endif // RANGE_HPP_ 
