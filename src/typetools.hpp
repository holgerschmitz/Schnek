/*
 * typetools.hpp
 *
 * Created on: 19 Feb 2007
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

#ifndef SCHNEK_TYPETOOLS_H_
#define SCHNEK_TYPETOOLS_H_

namespace schnek
{

struct NullType {};
struct EmptyType {};

/** Type to type mapper class.
 *
 *  A method to pass type information in an argument without creating a large
 *  object.
 */
template<class Type>
struct Type2Type
{
    /** the original type */
    typedef Type OriginalType;
};


/** Type to type mapper class.
 *
 *  A method to pass type information in an argument without creating a large
 *  object.
 */
template<template<class> class Type>
struct Type2Type1
{
    /** the original type */
    typedef Type OriginalType;
};

/** A templated typelist struct
 *
 *  Typelists are used for creating generalized functors, generating
 *  class hierarchies and more
 */
  template<class H, class T>
  struct Typelist
  {
    /** The head is an actual type */
    typedef H Head;
    /** The tail is either a typelist itself or NullType*/
    typedef T Tail;
  };

  template<class TList, unsigned int index, class DefaultType> 
  struct TypeAtNonStrict;

  template<class Head, class Tail, class DefaultType>
  struct TypeAtNonStrict<Typelist<Head,Tail>, 0, DefaultType>
  {
    typedef Head Result;
  };

  template<class DefaultType, unsigned int index>
  struct TypeAtNonStrict<NullType, index, DefaultType>
  {
    typedef DefaultType Result;
  };

  template<class Head, class Tail, unsigned int index, class DefaultType>
  struct TypeAtNonStrict<Typelist<Head,Tail>, index, DefaultType>
  {
    typedef typename TypeAtNonStrict<Tail,index-1,DefaultType>::Result Result;
  };

}

#define TYPELIST_0 NullType
#define TYPELIST_1(T1) schnek::Typelist<T1, schnek::NullType>
#define TYPELIST_2(T1, T2) schnek::Typelist<T1, TYPELIST_1(T2)>

#endif // SCHNEK_TYPETOOLS_H_
