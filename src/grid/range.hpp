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
#include "boost/concept/assert.hpp"
#include "boost/concept_check.hpp"

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
    /** Forward iterator over the rectangular domain
     *  Implements operator* and getPos which both return the current iterator position
     */
    class iterator : public std::iterator<std::forward_iterator_tag, LimitType> {
      private:
        BOOST_CONCEPT_ASSERT((boost::Integer<T>));
        friend class Range;
        /// Current iterator position
        LimitType pos;
        /// Reference to the rectangular domain
        const Range &domain;
        /// True if the iterator has reached the end
        bool atEnd;

        /// Constructor called by Range to create the iterator
        iterator(const Range &domain_, const LimitType &pos_, bool atEnd_=false)
        : pos(pos_), domain(domain_), atEnd(atEnd_) {}
        /// Default constructor is declared private for now. (Need to implement assignment first)
        iterator();

        /// Increments the iterator by one position.
        void increment()
        {
          int d = rank;
          while (d>0)
          {
            --d;
            if (++pos[d] > domain.getHi()[d])
            {
              pos[d] = domain.getLo()[d];
            }
            else
              return;
          }
          atEnd = true;
        }
      public:
        /// Copy constructor
        iterator(const iterator &it) : pos(it.pos), domain(it.domain), atEnd(it.atEnd) {}

        /// Prefix increment. Increments the iterator by one position.
        iterator &operator++()
        {
          increment();
          return *this;
        }
        /// Postfix increment. Increments the iterator by one position.
        const iterator operator++(int)
        {
          iterator it(*this);
          increment();
          return it;
        }
        /// Equality test
        bool operator==(const iterator &it)
        {
          return (atEnd==it.atEnd) && (pos==it.pos);
        }

        /// Equality test
        bool operator!=(const iterator &it) { return !(operator==(it)); }

        /// Returns the current iterator position
        const LimitType& operator*() { return pos; }

        /// Returns the current iterator position
        const LimitType& getPos() { return pos; }
    };

    /// Creates an iterator pointing to the beginning of the rectangle
    iterator begin() {
      return iterator(*this, this->getLo());
    }

    /// Creates an iterator pointing to a position after the end of the rectangle
    iterator end() {
      return iterator(*this, this->getLo(), true);
    }
};

} // namespace 

#endif // RANGE_HPP_ 
