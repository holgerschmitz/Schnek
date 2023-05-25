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

#include "../config.hpp"
#include "../macros.hpp"

#include <initializer_list>

namespace schnek {

template<class Operator, size_t Length>
class ArrayExpression;


/**A Fixed size array.
 * The three template parameters are:<br>
 * T: the type of data stored<br>
 * length: the length of the array<br>
 * CheckingPolicy: A policy class defining how to check the index passed to 
 * the accessor method
 */
template<
  class T, 
  size_t Length, 
  template<size_t> class CheckingPolicy = ArrayNoArgCheck
>
class Array :
    public CheckingPolicy<Length>
{
  private:
    /// The data stored in a C array
    T data[Length];

    /// ThisType defined for convenience
    typedef Array<T,Length,CheckingPolicy> ThisType;
  public:
    typedef T value_type;
    static constexpr size_t length = Length;
    
    /// The default constructor
    SCHNEK_INLINE Array();
    /// Copy constructor copies the values
    template<template<size_t> class CheckingPolicy2>
    SCHNEK_INLINE Array(const Array<T, Length, CheckingPolicy2> &);

    /// Construct using an array expression
    template<class Operator>
    SCHNEK_INLINE Array(const ArrayExpression<Operator, Length> &);

    /// Construct using an initializer list
    // Array(std::initializer_list<T> l);

    /// Constructor for length=1 arrays setting the data explicitely
    SCHNEK_INLINE Array(const T&);
    /// Constructor for length=2 arrays setting the data explicitely
    SCHNEK_INLINE Array(const T&, const T&);
    /// Constructor for length=3 arrays setting the data explicitely
    SCHNEK_INLINE Array(const T&, const T&, const T&);
    /// Constructor for length=4 arrays setting the data explicitely
    SCHNEK_INLINE Array(const T&, const T&, const T&, const T&);
    /// Constructor for length=5 arrays setting the data explicitely
    SCHNEK_INLINE Array(const T&, const T&, const T&, const T&, const T&);
    /// Constructor for length=6 arrays setting the data explicitely
    SCHNEK_INLINE Array(const T&, const T&, const T&, const T&, const T&, 
               const T&);
    /// Constructor for length=7 arrays setting the data explicitely
    SCHNEK_INLINE Array(const T&, const T&, const T&, const T&, const T&, 
               const T&, const T&);
    /// Constructor for length=8 arrays setting the data explicitely
    SCHNEK_INLINE Array(const T&, const T&, const T&, const T&, const T&, 
               const T&, const T&, const T&);
    /// Constructor for length=9 arrays setting the data explicitely
    SCHNEK_INLINE Array(const T&, const T&, const T&, const T&, const T&, 
               const T&, const T&, const T&, const T&);
    /// Constructor for length=10 arrays setting the data explicitely
    SCHNEK_INLINE Array(const T&, const T&, const T&, const T&, const T&, 
               const T&, const T&, const T&, const T&, const T&);

    SCHNEK_INLINE ~Array() {}

    /// Accessor operator
    SCHNEK_INLINE T& operator[](size_t);
    /// Constant accessor operator
    SCHNEK_INLINE T operator[](size_t) const;

    /// Accessor operator
    SCHNEK_INLINE T& at(size_t);
    /// Constant accessor operator
    SCHNEK_INLINE T at(size_t) const;

  public:
    /** Assignment operator
     *
     * The argument can have a different value_type and checking policy.
     * The value_type of the RHS must be implicitly castable to the value_type
     * of the LHS.
     */
    template<class T2, template <size_t> class CheckingPolicy2>
    SCHNEK_INLINE Array<T,Length,CheckingPolicy> &operator=(const Array<T2,Length,CheckingPolicy2>&);

    /// Assignment operator using an array expression
    template<class Operator>
    SCHNEK_INLINE Array<T,Length,CheckingPolicy> &operator=(const ArrayExpression<Operator, Length> &);

    /** Addition Assignment operator
     *
     * The argument can have a different value_type and checking policy.
     * The value_type of the RHS must be implicitly castable to the value_type
     * of the LHS.
     */
    template<class T2, template <size_t> class CheckingPolicy2>
    SCHNEK_INLINE Array<T,Length,CheckingPolicy> &operator+=(const Array<T2,Length,CheckingPolicy2>&);

    /** Element-wise multiplication assignment operator
     *
     * The argument can have a different value_type and checking policy.
     * The value_type of the RHS must be implicitly castable to the value_type
     * of the LHS.
     */
    template<class T2, template <size_t> class CheckingPolicy2>
    SCHNEK_INLINE Array<T,Length,CheckingPolicy> &operator*=(const Array<T2,Length,CheckingPolicy2>&);

    /** Element-wise division assignment operator
     *
     * The argument can have a different value_type and checking policy.
     * The value_type of the RHS must be implicitly castable to the value_type
     * of the LHS.
     */
    template<class T2, template <size_t> class CheckingPolicy2>
    SCHNEK_INLINE Array<T,Length,CheckingPolicy> &operator/=(const Array<T2,Length,CheckingPolicy2>&);

    /** Subtraction Assignment operator
     *
     * The argument can have a different value_type and checking policy.
     * The value_type of the RHS must be implicitly castable to the value_type
     * of the LHS.
     */
    template<class T2, template <size_t> class CheckingPolicy2>
    SCHNEK_INLINE Array<T,Length,CheckingPolicy> &operator-=(const Array<T2,Length,CheckingPolicy2>&);

    /** Addition Assignment operator with scalar RHS.
     *
     * The argument can have a different type.
     * The type of the RHS must be implicitly castable to the value_type
     * of the LHS.
     */
    template<typename T2>
    SCHNEK_INLINE Array<T,Length,CheckingPolicy> &operator+=(const T2);

    /** Subtraction Assignment operator with scalar RHS.
     *
     * The argument can have a different type.
     * The type of the RHS must be implicitly castable to the value_type
     * of the LHS.
     */
    template<typename T2>
    SCHNEK_INLINE Array<T,Length,CheckingPolicy> &operator-=(const T2);

    /** Multiplication Assignment operator with scalar RHS.
     *
     * The argument can have a different type.
     * The type of the RHS must be implicitly castable to the value_type
     * of the LHS.
     */
    template<typename T2>
    SCHNEK_INLINE Array<T,Length,CheckingPolicy> &operator*=(const T2);

    /** Division Assignment operator with scalar RHS.
     *
     * The argument can have a different type.
     * The type of the RHS must be implicitly castable to the value_type
     * of the LHS.
     */
    template<typename T2>
    SCHNEK_INLINE Array<T,Length,CheckingPolicy> &operator/=(const T2);


    /// Addition Assignment operator using an array expression
    template<class Operator>
    SCHNEK_INLINE Array<T,Length,CheckingPolicy> &operator+=(const ArrayExpression<Operator, Length> &);

    /// Subtraction Assignment operator using an array expression
    template<class Operator>
    SCHNEK_INLINE Array<T,Length,CheckingPolicy> &operator-=(const ArrayExpression<Operator, Length> &);

    /// Element-wise multiplication assignment operator using an array expression
    template<class Operator>
    SCHNEK_INLINE Array<T,Length,CheckingPolicy> &operator*=(const ArrayExpression<Operator, Length> &);

    /// Element-wise multiplication assignment operator using an array expression
    template<class Operator>
    SCHNEK_INLINE Array<T,Length,CheckingPolicy> &operator/=(const ArrayExpression<Operator, Length> &);

  public:
    /// Sets all fields to zero
    Array<T,Length,CheckingPolicy>& clear();
    /// Fills all fields with a given value
    Array<T,Length,CheckingPolicy>& fill(const T&);
    
    /// projects the Array onto an Array of shorter length
    template<size_t destLength>
    Array<T,destLength,CheckingPolicy> project() const;

    Array<T,Length-1,CheckingPolicy> projectDim(size_t dim) const;


    /** Returns an array filled with zeros.
     *  Only available if int can be cast to the type T
     */
    static Array<T,Length,CheckingPolicy> Zero();
    
    /** Returns an array filled with ones.
     *  Only available if int can be cast to the type T
     */
    static Array<T,Length,CheckingPolicy> Ones();

    /// Returns the product of all elements
    T product() const;

    /// Returns the sum of all elements
    T sum() const;

    /// Returns the sum of squares of all elements
    T sqr() const;
};



} // namespace

template<
  class T1, class T2,
  size_t Length, 
  template<size_t> class CheckingPolicy1, template<size_t> class CheckingPolicy2
>
SCHNEK_INLINE bool operator==(
  const schnek::Array<T1,Length,CheckingPolicy1>&,
  const schnek::Array<T2,Length,CheckingPolicy2>&
);

template<
  class T1, class T2,
  size_t Length, 
  template<size_t> class CheckingPolicy1, template<size_t> class CheckingPolicy2
>
SCHNEK_INLINE bool operator!=(
  const schnek::Array<T1,Length,CheckingPolicy1>&,
  const schnek::Array<T2,Length,CheckingPolicy2>&
);

template<
  class T1, class T2,
  size_t Length, 
  template<size_t> class CheckingPolicy1, template<size_t> class CheckingPolicy2
>
SCHNEK_INLINE bool operator<(
  const schnek::Array<T1,Length,CheckingPolicy1>&,
  const schnek::Array<T2,Length,CheckingPolicy2>&
);

template<
  class T1, class T2,
  size_t Length, 
  template<size_t> class CheckingPolicy1, template<size_t> class CheckingPolicy2
>
SCHNEK_INLINE bool operator<=(
  const schnek::Array<T1,Length,CheckingPolicy1>&,
  const schnek::Array<T2,Length,CheckingPolicy2>&
);


#include "array.t"

#endif // SCHNEK_ARRAY_HPP_
