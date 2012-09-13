/*
 * array.hpp
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

#ifndef SCHNEK_ARRAY_HPP_
#define SCHNEK_ARRAY_HPP_

#include "arraycheck.hpp"

namespace schnek {

template<class Operator>
class FArrExpression;

/**A Fixed size array.
 * The three template parameters are:<br>
 * T: the type of data stored<br>
 * length: the length of the array<br>
 * CheckingPolicy: A policy class defining how to check the index passed to 
 * the accessor method
 */
template<
  class T, 
  int length, 
  template<int> class CheckingPolicy = ArrayNoArgCheck
>
class Array : public CheckingPolicy<length> {
  private:
    /// Holds the data in a simple C-style array
    T data[length];
    /// ThisType defined for convenience
    typedef Array<T,length,CheckingPolicy> ThisType;
  public:
    typedef T value_type;
    enum {Length = length};
    
    /// The default constructor
    Array();
    /// Copy constructor copies the values
    Array(const Array &);
    /// Constructor for length=1 arrays setting the data explicitely
    Array(const T&);
    /// Constructor for length=2 arrays setting the data explicitely
    Array(const T&, const T&);
    /// Constructor for length=3 arrays setting the data explicitely
    Array(const T&, const T&, const T&);
    /// Constructor for length=4 arrays setting the data explicitely
    Array(const T&, const T&, const T&, const T&);
    /// Constructor for length=5 arrays setting the data explicitely
    Array(const T&, const T&, const T&, const T&, const T&);
    /// Constructor for length=6 arrays setting the data explicitely
    Array(const T&, const T&, const T&, const T&, const T&, 
               const T&);
    /// Constructor for length=7 arrays setting the data explicitely
    Array(const T&, const T&, const T&, const T&, const T&, 
               const T&, const T&);
    /// Constructor for length=8 arrays setting the data explicitely
    Array(const T&, const T&, const T&, const T&, const T&, 
               const T&, const T&, const T&);
    /// Constructor for length=9 arrays setting the data explicitely
    Array(const T&, const T&, const T&, const T&, const T&, 
               const T&, const T&, const T&, const T&);
    /// Constructor for length=10 arrays setting the data explicitely
    Array(const T&, const T&, const T&, const T&, const T&, 
               const T&, const T&, const T&, const T&, const T&);

    /** Constructing from an FArrExpression.
     *  The expression is evaluated at construction time so there are no temporary 
     *  variables
     */
    template<class Operator>
    Array(const FArrExpression<Operator>&);
    
    ~Array() {}

    /// Accessor operator
    T& operator[](int);
    /// Constant accessor operator
    const T& operator[](int) const;
    
    /// Accessor operator
    T& at(int);
    /// Constant accessor operator
    const T& at(int) const;
    
    /** Operator for assigning a FArrExpression to the array.
     *  The expression is evaluated at assign time so there are no temporary 
     *  variables
     */
    template<class Operator>
    Array<T,length,CheckingPolicy>& operator=(const FArrExpression<Operator>&);

  public:
    /// Sets all fields to zero
    Array<T,length,CheckingPolicy>& clear();
    /// Fills all fields with a given value
    Array<T,length,CheckingPolicy>& fill(const T&);
    
  public:
  
    /** Returns an array filled with zeros.
     *  Only available if int can be cast to the type T
     */
    static Array<T,length,CheckingPolicy> Zero();
    
    /** Returns an array filled with ones.
     *  Only available if int can be cast to the type T
     */
    static Array<T,length,CheckingPolicy> Unity();
};



} // namespace

template<
  class T1, class T2,
  int length, 
  template<int> class CheckingPolicy1, template<int> class CheckingPolicy2
>
bool operator==(
  const schnek::Array<T1,length,CheckingPolicy1>&,
  const schnek::Array<T2,length,CheckingPolicy2>&
);

template<
  class T1, class T2,
  int length, 
  template<int> class CheckingPolicy1, template<int> class CheckingPolicy2
>
bool operator!=(
  const schnek::Array<T1,length,CheckingPolicy1>&,
  const schnek::Array<T2,length,CheckingPolicy2>&
);

template<
  class T1, class T2,
  int length, 
  template<int> class CheckingPolicy1, template<int> class CheckingPolicy2
>
bool operator<(
  const schnek::Array<T1,length,CheckingPolicy1>&,
  const schnek::Array<T2,length,CheckingPolicy2>&
);

template<
  class T1, class T2,
  int length, 
  template<int> class CheckingPolicy1, template<int> class CheckingPolicy2
>
bool operator<=(
  const schnek::Array<T1,length,CheckingPolicy1>&,
  const schnek::Array<T2,length,CheckingPolicy2>&
);


#include "array.t"

#endif // SCHNEK_ARRAY_HPP_
