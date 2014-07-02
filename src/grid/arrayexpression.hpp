/*
 * arrayexpression.hpp
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

#ifndef SCHNEK_ARREXPRESSION_H_
#define SCHNEK_ARREXPRESSION_H_

#include <iostream>

namespace schnek {


/**Expression template for the Array class.
 * This template will be created when adding or subtracting Arrays and
 * will allow performing expressions without temporary variables.
 * @todo Handle type promotion
 */
template<class Operator, int Length>
class ArrayExpression {
  private:
    /**The operator type. The operator will hold the information about
     * the type of operation and refernces to all the arguments
     */
    Operator Op;
  public:
    typedef typename Operator::value_type value_type;

    static const int length = Length;

    /**Construct the expression by passing a reference*/
    ArrayExpression(const Operator Op_) : Op(Op_) {}

    /**Copy constructor*/
    ArrayExpression(const ArrayExpression &Expr) : Op(Expr.Op) {}

    template<template<int> class CheckingPolicy>
    operator Array<value_type, Length, CheckingPolicy>() {
        Array<value_type, Length, CheckingPolicy> value;
        for (int i=0; i<Length; ++i) value[i] = Op[i];
        return value;
    }

    /**Return the i-th element of the expression*/
    value_type operator[](int i) const 
    { 
//      value_type result = Op[i];
//      std::cerr << "ArrayExpression -- " << result << std::endl;
//      return result;
      return Op[i];
    }
};

/**Operator class implementing binary operators for the ArrayExpression.
 * Holds const references to the expressions and type information of the
 * operator
 */
template<class Exp1, class Exp2, class OperatorType>
class ArrayBinaryOp {
  private:
    /// Expression A
    Exp1 A;
    /// Expression B
    Exp2 B;
  public:
    typedef typename OperatorType::value_type value_type;
  
    /**Construct passing the two references to the expressions*/
    ArrayBinaryOp(const Exp1 &A_, const Exp2 &B_) : A(A_), B(B_) {}

    /**Copy constructor*/
    ArrayBinaryOp(const ArrayBinaryOp &Op) : A(Op.A), B(Op.B) {}

    /**Return the i-th element of the operator expression
     * Gets the i-th elements of A and B and asks the static OperatorType::apply 
     * method to perform the calculation
     */
    value_type operator[](int i) const { return OperatorType::apply(A[i], B[i]); }
};

template<typename T>
struct ArrayConstantExp {
  private:
    T val;
  public:
    typedef T value_type;
    ArrayConstantExp(const T& val_) : val(val_) {}
    value_type operator[](int i) const { return val; }
};

/**An operator type implementing addition
 */
template<typename T>
struct ArrayOpPlus {
  typedef T value_type;

  /// Returns the sum of the two elements
  static value_type apply(value_type x, value_type y) { return x+y; }
};

/**An operator type implementing subtraction
 */
template<typename T>
struct ArrayOpMinus {
  typedef T value_type;

  /// Returns the sum of the two elements
  static value_type apply(value_type x, value_type y) { return x-y; }
};

/**An operator type implementing multiplication
 */
template<typename T>
struct ArrayOpMult {
  typedef T value_type;

  /// Returns the sum of the two elements
  static value_type apply(value_type x, value_type y) { return x*y; }
};

/**An operator type implementing division
 */
template<typename T>
struct ArrayOpDiv {
  typedef T value_type;

  /// Returns the sum of the two elements
  static value_type apply(value_type x, value_type y) { return x/y; }
};


//================================================================
//======== Here we define all the operators ======================
//================================================================

// Because I hate writing long lines of templated code, I define a few helper
// macros.

/* Operator for two ArrayExpression objects */
#define EXPR_EXPR(op, symbol)                                                   \
  template<class exp1, class exp2, int length>                                  \
  ArrayExpression<                                                              \
    ArrayBinaryOp<                                                              \
      ArrayExpression<exp1, length>,                                            \
      ArrayExpression<exp2, length>,                                            \
      op<typename exp1::value_type>                                             \
    >,                                                                          \
    length                                                                      \
  >                                                                             \
  operator symbol (                                                             \
    const ArrayExpression<exp1, length> &A,                                     \
    const ArrayExpression<exp2, length> &B)                                     \
  {                                                                             \
    typedef ArrayBinaryOp                                                       \
    <                                                                           \
      ArrayExpression<exp1, length>,                                            \
      ArrayExpression<exp2, length>,                                            \
      op<typename exp1::value_typelength>                                       \
    >  OperatorType;                                                            \
                                                                                \
    return ArrayExpression<OperatorType, length>(OperatorType(A,B));            \
  }


/* Operator for two Arrays */
#define ARR_ARR(op, symbol)                                                     \
template <                                                                      \
  class T,                                                                      \
  int length,                                                                   \
  template<int> class CheckingPolicy1,                                          \
  template<int> class CheckingPolicy2                                           \
>                                                                               \
ArrayExpression<                                                                \
  ArrayBinaryOp<                                                                \
    ArrayExpression< Array<T,length,CheckingPolicy1>, length >,                 \
    ArrayExpression< Array<T,length,CheckingPolicy2>, length >,                 \
    op<T>                                                                       \
  >,                                                                            \
  length                                                                        \
>                                                                               \
operator symbol (                                                               \
  const Array<T,length,CheckingPolicy1> &A,                                     \
  const Array<T,length,CheckingPolicy2> &B                                      \
)                                                                               \
{                                                                               \
  typedef ArrayBinaryOp<                                                        \
    ArrayExpression< Array<T,length,CheckingPolicy1>, length >,                 \
    ArrayExpression< Array<T,length,CheckingPolicy2>, length >,                 \
    op<T>                                                                       \
  > OperatorType;                                                               \
                                                                                \
  return ArrayExpression<OperatorType, length> (OperatorType(A,B));             \
}

/* Operator for a Array and a ArrayExpression object */
#define EXPR_ARR(op, symbol)                                                    \
template<                                                                       \
  class exp,                                                                    \
  class T,                                                                      \
  int length,                                                                   \
  template<int> class CheckingPolicy                                            \
>                                                                               \
ArrayExpression<                                                                \
  ArrayBinaryOp<                                                                \
    ArrayExpression<exp, length>,                                               \
    ArrayExpression< Array<T,length,CheckingPolicy>, length >,                  \
    op<T>                                                                       \
  >,                                                                            \
  length                                                                        \
>                                                                               \
operator symbol (                                                               \
  const ArrayExpression<exp, length> &A,                                        \
  const Array<T,length,CheckingPolicy> &B                                       \
)                                                                               \
{                                                                               \
  typedef ArrayBinaryOp<                                                        \
    ArrayExpression<exp, length>,                                               \
    ArrayExpression< Array<T,length,CheckingPolicy>, length >,                  \
    op<T>                                                                       \
  > OperatorType;                                                               \
                                                                                \
  return ArrayExpression<OperatorType, length> (OperatorType(A,B));             \
}


/* Operator for a Array and a ArrayExpression object */
#define ARR_EXPR(op, symbol)                                                    \
template<                                                                       \
  class exp,                                                                    \
  class T,                                                                      \
  int length,                                                                   \
  template<int> class CheckingPolicy                                            \
>                                                                               \
ArrayExpression<                                                                \
  ArrayBinaryOp<                                                                \
    ArrayExpression< Array<T,length,CheckingPolicy>, length >,                  \
    ArrayExpression<exp, length>,                                               \
    op<T>                                                                       \
  >,                                                                            \
  length                                                                        \
>                                                                               \
operator symbol (                                                               \
  const Array<T,length,CheckingPolicy> &A,                                      \
  const ArrayExpression<exp, length> &B                                         \
)                                                                               \
{                                                                               \
  typedef ArrayBinaryOp<                                                        \
    ArrayExpression< Array<T,length,CheckingPolicy>, length >,                  \
    ArrayExpression<exp, length>,                                               \
    op<T>                                                                       \
  > OperatorType;                                                               \
                                                                                \
  return ArrayExpression<OperatorType, length> (OperatorType(A,B));             \
}

/* Operator for a Scalar and a ArrayExpression object */
#define SCAL_EXPR(op, symbol)                                                   \
template<class T, class exp, int length>                                        \
ArrayExpression<                                                                \
  ArrayBinaryOp<                                                                \
    ArrayConstantExp<T>,                                                        \
    ArrayExpression<exp, length>,                                               \
    op<T>                                                                       \
  >,                                                                            \
  length                                                                        \
>                                                                               \
operator symbol (const T &A, const ArrayExpression<exp, length> &B)             \
{                                                                               \
  typedef ArrayBinaryOp<                                                        \
    ArrayConstantExp<T>,                                                        \
    ArrayExpression<exp, length>,                                               \
    op<T>                                                                       \
  > OperatorType;                                                               \
                                                                                \
  return ArrayExpression<OperatorType, length> (OperatorType(A,B));             \
}                                                                                 
                                                                                  
/* Operator for a ArrayExpression and a Scalar object */
#define EXPR_SCAL(op, symbol)                                                   \
template<class T, class exp, int length>                                        \
ArrayExpression<                                                                \
  ArrayBinaryOp<                                                                \
    ArrayExpression<exp, length>,                                               \
    ArrayConstantExp<T>,                                                        \
    op<T>                                                                       \
  >,                                                                            \
  length                                                                        \
>                                                                               \
operator symbol (const ArrayExpression<exp, length> &A, const T &B)             \
{                                                                               \
  typedef ArrayBinaryOp<                                                        \
    ArrayExpression<exp, length>,                                               \
    ArrayConstantExp<T>,                                                        \
    op<T>                                                                       \
  > OperatorType;                                                               \
                                                                                \
  return ArrayExpression<OperatorType, length> (OperatorType(A,B));             \
}
/* Operator for a Scalar and a Array object */
#define SCAL_ARR(op, symbol)                                                    \
template<class T, int length, template<int> class CheckingPolicy>               \
ArrayExpression<                                                                \
  ArrayBinaryOp<                                                                \
    ArrayConstantExp<T>,                                                        \
    Array<T,length,CheckingPolicy>,                                             \
    op<T>                                                                       \
  >,                                                                            \
  length                                                                        \
>                                                                               \
operator symbol (const T &A, const Array<T,length,CheckingPolicy> &B)           \
{                                                                               \
  typedef ArrayBinaryOp<                                                        \
    ArrayConstantExp<T>,                                                        \
    Array<T,length,CheckingPolicy>,                                             \
    op<T>                                                                       \
  > OperatorType;                                                               \
                                                                                \
  return ArrayExpression<OperatorType, length> (OperatorType(A,B));             \
}

/* Operator for a Array and a Scalar object */
#define ARR_SCAL(op, symbol)                                                    \
template<class T, int length, template<int> class CheckingPolicy>               \
ArrayExpression<                                                                \
  ArrayBinaryOp<                                                                \
    Array<T,length,CheckingPolicy>,                                             \
    ArrayConstantExp<T>,                                                        \
    op<T>                                                                       \
  >,                                                                            \
  length                                                                        \
>                                                                               \
operator symbol (const Array<T,length,CheckingPolicy> &A, const T &B)           \
{                                                                               \
  typedef ArrayBinaryOp<                                                        \
    Array<T,length,CheckingPolicy>,                                             \
    ArrayConstantExp<T>,                                                        \
    op<T>                                                                       \
  > OperatorType;                                                               \
                                                                                \
  return ArrayExpression<OperatorType, length> (OperatorType(A,B));             \
}                                                                                 


//======== Plus ======================

EXPR_EXPR(ArrayOpPlus,+)
ARR_ARR(ArrayOpPlus,+)
EXPR_ARR(ArrayOpPlus,+)
ARR_EXPR(ArrayOpPlus,+)

//======== Minus ======================

EXPR_EXPR(ArrayOpMinus,-)
ARR_ARR(ArrayOpMinus,-)
EXPR_ARR(ArrayOpMinus,-)
ARR_EXPR(ArrayOpMinus,-)


//======== Multiplication ======================

EXPR_SCAL(ArrayOpMult,*)
SCAL_EXPR(ArrayOpMult,*)
ARR_SCAL(ArrayOpMult,*)
SCAL_ARR(ArrayOpMult,*)

//======== Division ======================

EXPR_SCAL(ArrayOpDiv,/)
SCAL_EXPR(ArrayOpDiv,/)
ARR_SCAL(ArrayOpDiv,/)
SCAL_ARR(ArrayOpDiv,/)


#undef EXPR_EXPR
#undef ARR_ARR
#undef ARR_EXPR
#undef EXPR_ARR
#undef EXPR_SCAL
#undef SCAL_EXPR
#undef ARR_SCAL
#undef SCAL_ARR


} // namespace schnek

#endif // SCHNEK_ARRAYEXPRESSION_H_
