#define EXPR_BINARY_OP(atype, btype, optype)      \
  ArrayExpression<                                \
    ArrayBinaryOp<                                \
      ArrayExpression<atype, length>,             \
      ArrayExpression<btype, length>,             \
      op<optype>                                  \
    >,                                            \
    length                                        \
  >  
  
#define OPDEF_BINARY_OP(atype, btype, optype)      \
    typedef ArrayBinaryOp                          \
    <                                              \
      ArrayExpression<atype, length>,              \
      ArrayExpression<btype, length>,              \
      op<typename exp1::value_type>                \
    >  OperatorType;                               \
    return ArrayExpression<OperatorType, length>(OperatorType(A,B));
    

  
/* Operator for two ArrayExpression objects */
#define EXPR_EXPR(op, symbol)                                  \
  template<class exp1, class exp2, int length>                 \
  EXPR_BINARY_OP(exp1,exp2,typename exp1::value_type)          \
  operator symbol (                                            \
    const ArrayExpression<exp1, length> &A,                    \
    const ArrayExpression<exp2, length> &B)                    \
  {                                                            \
    OPDEF_BINARY_OP(exp1,exp2,typename exp1::value_type)       \
  }


/* Operator for two Arrays */
#define ARR_ARR(op, symbol)                                                       \
template <                                                                        \
  class T,                                                                        \
  int length,                                                                     \
  template<int> class CheckingPolicy1,                                            \
  template<int> class CheckingPolicy2                                             \
>                                                                                 \
ArrayExpression<                                                                  \
  ArrayBinaryOp<                                                                  \
    ArrayExpression< Array<T,length,CheckingPolicy1>, length >,                           \
    ArrayExpression< Array<T,length,CheckingPolicy2>, length >,                           \
    op<T, length> >                                                               \
  >                                                                               \
operator symbol (                                                                 \
  const Array<T,length,CheckingPolicy1> &A,                                       \
  const Array<T,length,CheckingPolicy2> &B                                        \
)                                                                                 \
{                                                                                 \
  typedef ArrayBinaryOp<                                                          \
    ArrayExpression< Array<T,length,CheckingPolicy1>, length >,                           \
    ArrayExpression< Array<T,length,CheckingPolicy2>, length >,                           \
    op<T, length>                                                                 \
  > OperatorType;                                                                 \
                                                                                  \
  return ArrayExpression<OperatorType> (OperatorType(A,B));                       \
}

/* Operator for a Array and a ArrayExpression object */
#define EXPR_ARR(op, symbol)                                                      \
template<                                                                         \
  class exp,                                                                      \
  class T,                                                                        \
  int length,                                                                     \
  template<int> class CheckingPolicy                                              \
>                                                                                 \
ArrayExpression<                                                                  \
  ArrayBinaryOp<                                                                  \
    ArrayExpression<exp>,                                                         \
    ArrayExpression< Array<T,length,CheckingPolicy> >,                            \
    op<T>                                                                         \
  >,
  length                                                                               \
>                                                                                 \
operator symbol (                                                                 \
  const ArrayExpression<exp, length> &A,                                                  \
  const Array<T,length,CheckingPolicy> &B                                         \
)                                                                                 \
{                                                                                 \
  typedef ArrayBinaryOp<                                                          \
    ArrayExpression<exp, length>,                                                         \
    ArrayExpression< Array<T,length,CheckingPolicy>, length >,                            \
    op<T, length>                                                                 \
  > OperatorType;                                                                 \
                                                                                  \
  return ArrayExpression<OperatorType> (OperatorType(A,B));                       \
}


/* Operator for a Array and a ArrayExpression object */
#define ARR_EXPR(op, symbol)                                                      \
template<                                                                         \
  class exp,                                                                      \
  class T,                                                                        \
  int length,                                                                     \
  template<int> class CheckingPolicy                                              \
>                                                                                 \
ArrayExpression<                                                                  \
  ArrayBinaryOp<                                                                  \
    ArrayExpression< Array<T,length,CheckingPolicy> >,                            \
    ArrayExpression<exp>,                                                         \
    op<T, length>                                                                         \
  >                                                                               \
>                                                                                 \
operator symbol (                                                                 \
  const Array<T,length,CheckingPolicy> &A,                                        \
  const ArrayExpression<exp> &B                                                   \
)                                                                                 \
{                                                                                 \
  typedef ArrayBinaryOp<                                                          \
    ArrayExpression< Array<T,length,CheckingPolicy> >,                            \
    ArrayExpression<exp>,                                                         \
    op<T, length>                                                                         \
  > OperatorType;                                                                 \
                                                                                  \
  return ArrayExpression<OperatorType> (OperatorType(A,B));                       \
}

/* Operator for a Scalar and a ArrayExpression object */
#define SCAL_EXPR(op, symbol)                                                     \
template<class T, class exp>                                                      \
ArrayExpression<                                                                  \
  ArrayBinaryOp<                                                                  \
    ArrayConstantExp<T>,                                                          \
    ArrayExpression<exp>,                                                         \
    op<T>                                                                         \
  >                                                                               \
>                                                                                 \
operator symbol (const T &A, const ArrayExpression<exp> &B)                       \
{                                                                                 \
  typedef ArrayBinaryOp<                                                          \
    ArrayConstantExp<T>,                                                          \
    ArrayExpression<exp>,                                                         \
    op<T>                                                                         \
  > OperatorType;                                                                 \
                                                                                  \
  return ArrayExpression<OperatorType> (OperatorType(A,B));                       \
}                                                                                 
                                                                                  
/* Operator for a ArrayExpression and a Scalar object */
#define EXPR_SCAL(op, symbol)                                                     \
template<class T, class exp>                                                      \
ArrayExpression<                                                                  \
  ArrayBinaryOp<                                                                  \
    ArrayExpression<exp>,                                                         \
    ArrayConstantExp<T>,                                                          \
    op<T>                                                                         \
  >                                                                               \
>                                                                                 \
operator symbol (const ArrayExpression<exp> &A, const T &B)                       \
{                                                                                 \
  typedef ArrayBinaryOp<                                                          \
    ArrayExpression<exp>,                                                         \
    ArrayConstantExp<T>,                                                          \
    op<T>                                                                         \
  > OperatorType;                                                                 \
                                                                                  \
  return ArrayExpression<OperatorType> (OperatorType(A,B));                       \
}                                                                                 
/* Operator for a Scalar and a Array object */
#define SCAL_ARR(op, symbol)                                                      \
template<class T, int length, template<int> class CheckingPolicy>                 \
ArrayExpression<                                                                  \
  ArrayBinaryOp<                                                                  \
    ArrayConstantExp<T>,                                                          \
    Array<T,length,CheckingPolicy>,                                               \
    op<T>                                                                         \
  >                                                                               \
>                                                                                 \
operator symbol (const T &A, const Array<T,length,CheckingPolicy> &B)             \
{                                                                                 \
  typedef ArrayBinaryOp<                                                          \
    ArrayConstantExp<T>,                                                          \
    Array<T,length,CheckingPolicy>,                                               \
    op<T>                                                                         \
  > OperatorType;                                                                 \
                                                                                  \
  return ArrayExpression<OperatorType> (OperatorType(A,B));                       \
}                                                                                 
                                                                                  
/* Operator for a Array and a Scalar object */
#define ARR_SCAL(op, symbol)                                                      \
template<class T, int length, template<int> class CheckingPolicy>                 \
ArrayExpression<                                                                  \
  ArrayBinaryOp<                                                                  \
    Array<T,length,CheckingPolicy>,                                               \
    ArrayConstantExp<T>,                                                          \
    op<T>                                                                         \
  >                                                                               \
>                                                                                 \
operator symbol (const Array<T,length,CheckingPolicy> &A, const T &B)             \
{                                                                                 \
  typedef ArrayBinaryOp<                                                          \
    Array<T,length,CheckingPolicy>,                                               \
    ArrayConstantExp<T>,                                                          \
    op<T>                                                                         \
  > OperatorType;                                                                 \
                                                                                  \
  return ArrayExpression<OperatorType> (OperatorType(A,B));                       \
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

