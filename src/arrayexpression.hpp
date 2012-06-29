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

#ifndef SCHNEK_FARREXPRESSION_H_
#define SCHNEK_FARREXPRESSION_H_

#include "array.hpp"

#include <iostream>

namespace schnek {


/**Expression template for the Array class.
 * This template will be created when adding or subtracting FixedArrays and 
 * will allow performing expressions without temporary variables.
 * @todo Handle type promotion
 */
template<class Operator>
class FArrExpression {
  private:
    /**The operator type. The operator will hold the information about
     * the type of operation and refernces to all the arguments
     */
    Operator Op;
  public:
    typedef typename Operator::value_type value_type;

    /**Construct the expression by passing a reference*/
    FArrExpression(const Operator Op_) : Op(Op_) {}

    /**Copy constructor*/
    FArrExpression(const FArrExpression &Expr) : Op(Expr.Op) {}

    /**Return the i-th element of the expression*/
    value_type operator[](int i) const 
    { 
      value_type result = Op[i]; 
//      std::cerr << "FArrExpression -- " << result << std::endl;
      return result;
    }
};

/**Operator class implementing binary operators for the FArrExpression.
 * Holds const references to the expressions and type information of the
 * operator
 */
template<class Exp1, class Exp2, class OperatorType>
class FArrBinaryOp {
  private:
    /// Expression A
    Exp1 A;
    /// Expression B
    Exp2 B;
  public:
    typedef typename OperatorType::value_type value_type;
  
    /**Construct passing the two references to the expressions*/
    FArrBinaryOp(const Exp1 &A_, const Exp2 &B_) : A(A_), B(B_) {}

    /**Copy constructor*/
    FArrBinaryOp(const FArrBinaryOp &Op) : A(Op.A), B(Op.B) {}

    /**Return the i-th element of the operator expression
     * Gets the i-th elements of A and B and asks the static OperatorType::apply 
     * method to perform the calculation
     */
    value_type operator[](int i) const { return OperatorType::apply(A[i], B[i]); }
};

template<typename T>
struct FArrScalarHolderExp {
  private:
    T val;
  public:
    typedef T value_type;
    FArrScalarHolderExp(const T& val_) : val(val_) {}
    value_type operator[](int i) const { return val; }
};

/**An operator type implementing the plus sign
 */
template<typename T>
struct FArrOpPlus {
  typedef T value_type;

  /// Returns the sum of the two elements
  static value_type apply(value_type x, value_type y) { return x+y; }
};

/**An operator type implementing the minus sign
 */
template<typename T>
struct FArrOpMinus {
  typedef T value_type;

  /// Returns the sum of the two elements
  static value_type apply(value_type x, value_type y) { return x-y; }
};

/**An operator type implementing the plus sign
 */
template<typename T>
struct FArrOpMult {
  typedef T value_type;

  /// Returns the sum of the two elements
  static value_type apply(value_type x, value_type y) { return x*y; }
};

/**An operator type implementing the plus sign
 */
template<typename T>
struct FArrOpDiv {
  typedef T value_type;

  /// Returns the sum of the two elements
  static value_type apply(value_type x, value_type y) { return x/y; }
};


//================================================================
//======== Here we define all the operators ======================
//================================================================


/* Operator for two FArrExpression objects */
#define EXPR_EXPR(op, symbol)                                                     \
  template<class exp1, class exp2>                                                \
  FArrExpression<                                                                 \
    FArrBinaryOp<                                                                 \
      FArrExpression<exp1>,                                                       \
      FArrExpression<exp2>,                                                       \
      op<typename exp1::value_type>                                               \
    >                                                                             \
  >                                                                               \
  operator symbol (const FArrExpression<exp1> &A, const FArrExpression<exp2> &B)  \
  {                                                                               \
    typedef FArrBinaryOp                                                          \
    <                                                                             \
      FArrExpression<exp1>,                                                       \
      FArrExpression<exp2>,                                                       \
      op<typename exp1::value_type>                                               \
    >  OperatorType;                                                              \
                                                                                  \
    return FArrExpression<OperatorType> (OperatorType(A,B));                      \
  }


/* Operator for two FixedArrays */
#define ARR_ARR(op, symbol)                                                       \
template <                                                                        \
  class T,                                                                        \
  int length,                                                                     \
  template<int> class CheckingPolicy1,                                            \
  template<int> class CheckingPolicy2                                             \
>                                                                                 \
FArrExpression<                                                                   \
  FArrBinaryOp<                                                                   \
    FArrExpression< FixedArray<T,length,CheckingPolicy1> >,                       \
    FArrExpression< FixedArray<T,length,CheckingPolicy2> >,                       \
    op<T> >                                                                       \
  >                                                                               \
operator symbol (                                                                 \
  const FixedArray<T,length,CheckingPolicy1> &A,                                  \
  const FixedArray<T,length,CheckingPolicy2> &B                                   \
)                                                                                 \
{                                                                                 \
  typedef FArrBinaryOp<                                                           \
    FArrExpression< FixedArray<T,length,CheckingPolicy1> >,                       \
    FArrExpression< FixedArray<T,length,CheckingPolicy2> >,                       \
    op<T>                                                                         \
  > OperatorType;                                                                 \
                                                                                  \
  return FArrExpression<OperatorType> (OperatorType(A,B));                        \
}

/* Operator for a Array and a FArrExpression object */
#define EXPR_ARR(op, symbol)                                                      \
template<class exp, class T, int length, template<int> class CheckingPolicy>      \
FArrExpression<                                                                   \
  FArrBinaryOp<                                                                   \
    FArrExpression<exp>,                                                          \
    FArrExpression< FixedArray<T,length,CheckingPolicy> >,                        \
    op<T>                                                                         \
  >                                                                               \
>                                                                                 \
operator symbol (                                                                 \
  const FArrExpression<exp> &A,                                                   \
  const FixedArray<T,length,CheckingPolicy> &B                                    \
)                                                                                 \
{                                                                                 \
  typedef FArrBinaryOp<                                                           \
    FArrExpression<exp>,                                                          \
    FArrExpression< FixedArray<T,length,CheckingPolicy> >,                        \
    op<T>                                                                         \
  > OperatorType;                                                                 \
                                                                                  \
  return FArrExpression<OperatorType> (OperatorType(A,B));                        \
}


/* Operator for a Array and a FArrExpression object */
#define ARR_EXPR(op, symbol)                                                      \
template<                                                                         \
  class exp,                                                                      \
  class T,                                                                        \
  int length,                                                                     \
  template<int> class CheckingPolicy                                              \
>                                                                                 \
FArrExpression<                                                                   \
  FArrBinaryOp<                                                                   \
    FArrExpression< FixedArray<T,length,CheckingPolicy> >,                        \
    FArrExpression<exp>,                                                          \
    op<T>                                                                         \
  >                                                                               \
>                                                                                 \
operator symbol (                                                                 \
  const FixedArray<T,length,CheckingPolicy> &A,                                   \
  const FArrExpression<exp> &B                                                    \
)                                                                                 \
{                                                                                 \
  typedef FArrBinaryOp<                                                         \
    FArrExpression< FixedArray<T,length,CheckingPolicy> >,                        \
    FArrExpression<exp>,                                                          \
    op<T>                                                                         \
  > OperatorType;                                                                 \
                                                                                  \
  return FArrExpression<OperatorType> (OperatorType(A,B));                        \
}

/* Operator for a Scalar and a FArrExpression object */
#define SCAL_EXPR(op, symbol)                                                     \
template<class T, class exp>                 \
FArrExpression<                                                                   \
  FArrBinaryOp<                                                             \
    FArrScalarHolderExp<T>,                                                       \
    FArrExpression<exp>,                                                          \
    op<T>                                                                         \
  >                                                                               \
>                                                                                 \
operator symbol (const T &A, const FArrExpression<exp> &B)        \
{                                                                                 \
  typedef FArrBinaryOp<                                                     \
    FArrScalarHolderExp<T>,                                                       \
    FArrExpression<exp>,                                                          \
    op<T>                                                                         \
  > OperatorType;                                                                 \
                                                                                  \
  return FArrExpression<OperatorType> (OperatorType(A,B));                        \
}                                                                                 
                                                                                  
/* Operator for a FArrExpression and a Scalar object */
#define EXPR_SCAL(op, symbol)                                                     \
template<class T, class exp>                 \
FArrExpression<                                                                   \
  FArrBinaryOp<                                                             \
    FArrExpression<exp>,                                                          \
    FArrScalarHolderExp<T>,                                                       \
    op<T>                                                                         \
  >                                                                               \
>                                                                                 \
operator symbol (const FArrExpression<exp> &A, const T &B)        \
{                                                                                 \
  typedef FArrBinaryOp<                                                     \
    FArrExpression<exp>,                                                          \
    FArrScalarHolderExp<T>,                                                       \
    op<T>                                                                         \
  > OperatorType;                                                                 \
                                                                                  \
  return FArrExpression<OperatorType> (OperatorType(A,B));                        \
}                                                                                 
/* Operator for a Scalar and a Array object */
#define SCAL_ARR(op, symbol)                                                     \
template<class T, int length, template<int> class CheckingPolicy>                 \
FArrExpression<                                                                   \
  FArrBinaryOp<                                                             \
    FArrScalarHolderExp<T>,                                                       \
    FixedArray<T,length,CheckingPolicy>,                                                          \
    op<T>                                                                         \
  >                                                                               \
>                                                                                 \
operator symbol (const T &A, const FixedArray<T,length,CheckingPolicy> &B)        \
{                                                                                 \
  typedef FArrBinaryOp<                                                     \
    FArrScalarHolderExp<T>,                                                       \
    FixedArray<T,length,CheckingPolicy>,                                                          \
    op<T>                                                                         \
  > OperatorType;                                                                 \
                                                                                  \
  return FArrExpression<OperatorType> (OperatorType(A,B));                        \
}                                                                                 
                                                                                  
/* Operator for a Array and a Scalar object */
#define ARR_SCAL(op, symbol)                                                     \
template<class T, int length, template<int> class CheckingPolicy>                 \
FArrExpression<                                                                   \
  FArrBinaryOp<                                                             \
    FixedArray<T,length,CheckingPolicy>,                                                          \
    FArrScalarHolderExp<T>,                                                       \
    op<T>                                                                         \
  >                                                                               \
>                                                                                 \
operator symbol (const FixedArray<T,length,CheckingPolicy> &A, const T &B)        \
{                                                                                 \
  typedef FArrBinaryOp<                                                     \
    FixedArray<T,length,CheckingPolicy>,                                                          \
    FArrScalarHolderExp<T>,                                                       \
    op<T>                                                                         \
  > OperatorType;                                                                 \
                                                                                  \
  return FArrExpression<OperatorType> (OperatorType(A,B));                        \
}                                                                                 


//======== Plus ======================

EXPR_EXPR(FArrOpPlus,+)
ARR_ARR(FArrOpPlus,+)
EXPR_ARR(FArrOpPlus,+)
ARR_EXPR(FArrOpPlus,+)

//======== Minus ======================

EXPR_EXPR(FArrOpMinus,-)
ARR_ARR(FArrOpMinus,-)
EXPR_ARR(FArrOpMinus,-)
ARR_EXPR(FArrOpMinus,-)


//======== Multiplication ======================

EXPR_SCAL(FArrOpMult,*)
SCAL_EXPR(FArrOpMult,*)
ARR_SCAL(FArrOpMult,*)
SCAL_ARR(FArrOpMult,*)

//======== Division ======================

EXPR_SCAL(FArrOpDiv,/)
SCAL_EXPR(FArrOpDiv,/)
ARR_SCAL(FArrOpDiv,/)
SCAL_ARR(FArrOpDiv,/)


#undef EXPR_EXPR
#undef ARR_ARR
#undef ARR_EXPR
#undef EXPR_ARR
#undef EXPR_SCAL
#undef SCAL_EXPR
#undef ARR_SCAL
#undef SCAL_ARR


} // namespace schnek

#endif // SCHNEK_FARREXPRESSION_H_
