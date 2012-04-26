/*
 * parsertoken.t
 *
 *  Created on: 12 Mar 2012
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
 */


/* ========================
 * TypePromoter
 */



template<class ExpressionPointer>
inline void TypePromoter::operator()(ExpressionPointer e1, ExpressionPointer e2)
{
  result1 = e1;
  result2 = e2;
}

template<>
inline void TypePromoter::operator()(pIntExpression e1, pFloatExpression e2)
{
  pFloatExpression pF(new TypecastOp<double,int>(e1));
  result1 = pF;
  result2 = e2;
}

template<>
inline void TypePromoter::operator()(pFloatExpression e1, pIntExpression e2)
{
  pFloatExpression pF(new TypecastOp<double,int>(e2));
  result1 = e1;
  result2 = pF;
}

template<>
inline void TypePromoter::operator()(pIntExpression e1, pStringExpression e2)
{
  pStringExpression pS(new TypecastOp<std::string,int,LexicalCast>(e1));
  result1 = pS;
  result2 = e2;
}

template<>
inline void TypePromoter::operator()(pStringExpression e1, pIntExpression e2)
{
  pStringExpression pS(new TypecastOp<std::string,int,LexicalCast>(e2));
  result1 = e1;
  result2 = pS;
}

template<>
inline void TypePromoter::operator()(pFloatExpression e1, pStringExpression e2)
{
  pStringExpression pS(new TypecastOp<std::string,double,LexicalCast>(e1));
  result1 = pS;
  result2 = e2;
}

template<>
inline void TypePromoter::operator()(pStringExpression e1, pFloatExpression e2)
{
  pStringExpression pS(new TypecastOp<std::string,double, LexicalCast>(e2));
  result1 = e1;
  result2 = pS;
}
/* ========================
 * ParserToken operator assignment
 */

template<template<class> class op>
struct UnaryOperatorVisitor : public boost::static_visitor<ExpressionVariant>
{
    template<class ExpressionPointer>
    ExpressionVariant operator()(ExpressionPointer a1)
    {
        typedef typename ExpressionPointer::element_type etype;
        typedef typename etype::ValueType vtype;
        typedef op<vtype> OpType;

        ExpressionPointer eP(new UnaryOp<OpType, vtype>(a1));
        return eP;
    }
};

template<template<class> class op>
struct BinaryOperatorVisitor : public boost::static_visitor<ExpressionVariant>
{
    ExpressionVariant arg2;

    BinaryOperatorVisitor(ExpressionVariant arg2_) :arg2(arg2_) {}

    template<class ExpressionPointer>
    ExpressionVariant operator()(ExpressionPointer a1)
    {
        typedef typename ExpressionPointer::element_type etype;
        typedef typename etype::ValueType vtype;
        typedef op<vtype> OpType;

        ExpressionPointer a2 = boost::get<ExpressionPointer>(arg2);

        ExpressionPointer eP(new BinaryOp<OpType, vtype>(a1,a2));
        return eP;
    }
};

template<template<class> class OpType>
void ParserToken::assignUnaryOperator(ParserToken &parTok)
{
  context = parTok.context;
  atomTok = parTok.atomTok;
  type = expression;

  UnaryOperatorVisitor<OpType> visit;
  data = boost::apply_visitor(visit, parTok.data);
}

template<template<class> class OpType>
void ParserToken::assignBinaryOperator(ParserToken &parTok1, ParserToken &parTok2)
{
  context = parTok1.context;
  atomTok = parTok1.atomTok;
  type = expression;
  TypePromoter tprom;
  boost::apply_visitor(tprom, parTok1.data, parTok2.data);

  BinaryOperatorVisitor<OpType> visit(tprom.getResultB());
  data = boost::apply_visitor(visit, tprom.getResultA());
}
