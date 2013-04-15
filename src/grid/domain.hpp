/*
 * domain.hpp
 *
 * Created on: 26 Apr 2012
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

#ifndef SCHNEK_DOMAIN_H_
#define SCHNEK_DOMAIN_H_

#include "range.hpp"

#include <list>
#include <boost/shared_ptr.hpp>

namespace schnek {

/** RecDomain is a rectangular domain that is not bound to any concrete grid data structure.
 *
 *  The rectangular domain is defined by a minimum and maximum. An iterator is provided that
 *  traverses the domain and returns the positions.
 */
template<
  int rank,
  template<int> class CheckingPolicy = ArrayNoArgCheck
>
class RecDomain : public Range<int, rank, CheckingPolicy>
{
  public:
    typedef typename Range<int, rank, CheckingPolicy>::LimitType LimitType;
    /// Construct with rectangle minimum and maximum
    RecDomain(const LimitType &min, const LimitType &max)
    : Range<int, rank, CheckingPolicy>(min, max) {}

    /// Copy constructor
    RecDomain(const RecDomain &domain) : Range<int, rank, CheckingPolicy>(domain) {}

    /// Assignment operator
    RecDomain &operator=(const RecDomain &domain)
    {
      return static_cast<RecDomain &>( this->operator=(domain) );
    }

    /** Forward iterator over the rectangular domain
     *  Implements operator* and getPos which both return the current iterator position
     */
    class iterator {
      private:
        friend class RecDomain;
        /// Current iterator position
        LimitType pos;
        /// Reference to the rectangular domain
        const RecDomain &domain;
        /// True if the iterator has reached the end
        bool atEnd;

        /// Constructor called by RecDomain to create the iterator
        iterator(const RecDomain &domain_, const LimitType &pos_, bool atEnd_=false)
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

//typedef boost::shared_ptr<RecDomain> pRecDomain;
//
///** MultiRecDomain contains a list of RecDomain rectangular domains.
// *
// *  New domains can be added by supplying the rectangular domain bounds.
// */
//template<int rank>
//class MultiRecDomain {
//  private:
//    /// The list of rectangular domains
//    std::list<pRecDomain> domains;
//
//  public:
//    /// Default constructor
//    MultiRecDomain() {}
//
//    /// Copy constructor
//    MultiRecDomain(const MultiRecDomain &dom)
//    : domains(dom.domains) {}
//
//    /// Add a new rectangular domain by supplying the domain bounds.
//    const Domain &addDomain(const IndexType &min_, const IndexType &max_)
//    {
//      pDomain ndom(new Domain(min_, max_));
//      domains.push_back(ndom);
//      return *ndom;
//    }
//};
//
//typedef boost::shared_ptr<MultiRecDomain> pMultiRecDomain;

} // namespace schnek


#endif // SCHNEK_DOMAIN_H_
