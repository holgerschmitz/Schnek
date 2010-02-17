//
// C++ Interface: typetools
//
// Description: 
//
//
// Author: Holger Schmitz <hs@aston.ac.uk>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef SCHNEK_TYPETOOLS_H
#define SCHNEK_TYPETOOLS_H

namespace rigger
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

#define TYPELIST_0 rigger::NullType
#define TYPELIST_1(T1) rigger::Typelist<T1, rigger::NullType>
#define TYPELIST_2(T1, T2) rigger::Typelist<T1, TYPELIST_1(T2)>

#endif
