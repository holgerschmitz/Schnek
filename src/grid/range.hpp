/*
 * range.hpp
 *
 * Created on: 31 Aug 2012
 * Author: hschmitz
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

/** RecDomain is a rectangular domain that is not bound to any concrete grid data structure.
 *
 *  The rectangular domain is defined by a minimum and maximum. An iterator is provided that
 *  traverses the domain and returns the positions.
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
    LimitType min, max;
  public:
    /// Construct with rectangle minimum and maximum
    Range(const LimitType &min_, const LimitType &max_)
    : min(min_), max(max_) {}

    /// Copy constructor
    Range(const Range &domain)
    : min(domain.min), max(domain.max) {}

    /// Assignment operator
    Range &operator=(const Range &domain)
    {
      min = domain.min;
      max = domain.max;
      return *this;
    }

    /// Return rectangle minimum
    const LimitType &getMin() const {return min;}
    /// Return rectangle maximum
    const LimitType &getMax() const {return max;}
};

} // namespace 
#endif // RANGE_HPP_ 
