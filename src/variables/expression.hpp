/*
 * expression.hpp
 *
 *  Created on: 26 Jan 2011
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
 */

#ifndef SCHNEK_EXPRESSION_HPP_
#define SCHNEK_EXPRESSION_HPP_

#include "variables.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/function.hpp>

#include <boost/fusion/include/push_back.hpp>
#include <boost/fusion/include/invoke.hpp>
#include <boost/fusion/include/cons.hpp>

#include <boost/mpl/begin.hpp>
#include <boost/mpl/end.hpp>
#include <boost/mpl/next.hpp>
#include <boost/mpl/deref.hpp>

#include <string>
#include <iostream>

namespace fusion = boost::fusion;
namespace mpl = boost::mpl;
namespace bft = boost::function_types;

namespace schnek {

class WrongNumberOfArgsException : public SchnekException
{
  public:
    WrongNumberOfArgsException() : SchnekException() {}
};

class FunctionNotFoundException : public SchnekException
{
  public:
    FunctionNotFoundException() : SchnekException() {}
};

/** A base type for real time evaluation of mathematical expressions.
 *
 * Expressions are most commonly constructed from parsing input decks, but might
 * be used elsewhere as function objects.
 *
 * The expression is templated with the return type. To allow expressions with
 * variables of different type, type promotion operators must be inserted.
 */
template<typename vtype>
class Expression
{
  public:
    /// The virtual method that causes the expression to evaluate
    virtual vtype eval() = 0;

    /// Returns whether the expression can be evaluated, are all the references also constant
    virtual bool isConstant() = 0;

    /// The () operator allows expressions to be used as function objects
    vtype operator()() { return eval(); }

    /// A pointer to an Expression
    typedef boost::shared_ptr<Expression> pExpression;
    typedef vtype ValueType;
};

template<class ResultVariant>
struct ExpressionEvaluator : public boost::static_visitor<ResultVariant>
{
  template<class ExpressionPointer>
  ResultVariant operator()(ExpressionPointer e) { return e->eval(); }
};

/** This Expression stores values directly. Generated from literals
 */
template<typename vtype>
class Value : public Expression<vtype>
{
  private:
    /// Contains the value
    vtype val;
  public:
    typedef vtype ValueType;
    /// Construct with a value
    Value(vtype val_) : val(val_) {}
    /// Return the stored value
    vtype eval() { return val; }
    /// A literal is a constant
    bool isConstant() { return true; }
    /// Return a reference to the value
    vtype &getReference() { return val; }
};

/** A special type of expresion that holds a reference to a variable.
 *
 * This is used by the parser when parsing variables
 */
template<typename vtype>
class ReferencedValue : public Expression<vtype>
{
  private:
    /// Contains the name of the variable
    Variable var;
  public:
    /// Construct with a value
    ReferencedValue(const Variable &var_) : var(var_) {}
    /// Return the modified value
    vtype eval() {
      if (var.isConstant())
      {
        //std::cerr << "Attempting to assign variable\n";
        return boost::get<vtype>(var.getValue());
      }
      else
        return boost::get<vtype>(var.evaluateExpression());
    }
    /// Constancy depends on the constancy of the variable
    bool isConstant() { return var.isConstant(); }
};

/** Unary operator expression
 *
 * This expression modifies the value according to a unary operator.
 * For generality the operator type is templated.
 */
template<class oper, class vtype>
class UnaryOp : public Expression<vtype>
{
  private:
    typedef typename Expression<vtype>::pExpression pExpression;
    /// pointer to the expression to modify
    pExpression expr;
  public:
    UnaryOp(pExpression expr_) : expr(expr_) {}

    /// Return the modified value
    vtype eval() { return oper::eval(expr->eval()); }

    /// Constancy depends on the constancy of the expression
    bool isConstant() { return expr->isConstant(); }
};

/** Binary operator expression
 *
 * This expression calculates a value from two arguments according to a
 * binary operator. For generality the operator type is templated.
 */
template<class oper, class vtype>
class BinaryOp : public Expression<vtype>
{
  private:
    typedef typename Expression<vtype>::pExpression pExpression;
    /// pointers to the expressions to modify
    typename Expression<vtype>::pExpression expr1, expr2;
  public:
    BinaryOp(pExpression expr1_, pExpression expr2_)
      : expr1(expr1_), expr2(expr2_) {}

    /// Return the calculated value
    vtype eval() { return oper::eval(expr1->eval(), expr2->eval()); }

    /// Constancy depends on the constancy of both expressions
    bool isConstant() { return expr1->isConstant() && expr2->isConstant(); }
};

template<class vtype>
struct FastCast
{
    vtype operator()(int a);
    vtype operator()(double a);
    vtype operator()(std::string a);
};

template<>
struct FastCast<int>
{
    int operator()(int a) { return a; }
    int operator()(double a) { return static_cast<int>(a); }
    int operator()(std::string a) { return boost::lexical_cast<int>(a); }
};

template<>
struct FastCast<double>
{
    double operator()(int a) { return static_cast<double>(a); }
    double operator()(double a) { return a; }
    double operator()(std::string a) { return boost::lexical_cast<double>(a); }
};

template<>
struct FastCast<std::string>
{
    std::string operator()(int a) { return boost::lexical_cast<std::string>(a); }
    std::string operator()(double a) { return boost::lexical_cast<std::string>(a); }
    std::string operator()(std::string a) { return a; }
};



template<class vtype>
struct StaticCast
{
  public:
    template<class atype>
    vtype operator()(const atype& a) { return static_cast<vtype>(a); }
};

template<class vtype>
struct LexicalCast
{
  public:
    template<class atype>
    vtype operator()(const atype& a) { return boost::lexical_cast<vtype>(a); }
};

template<class vtype, class vtype_orig, template<class> class CastType = StaticCast>
class TypecastOp : public Expression<vtype>
{
  public:
    /// A pointer to an original Expression
    typedef typename Expression<vtype_orig>::pExpression pExpressionOrig;
  private:
    /// pointers to the expressions to cast
    pExpressionOrig expr;
  public:
    TypecastOp(pExpressionOrig expr_) : expr(expr_) {}

    /// Return the modified value
    vtype eval() { return CastType<vtype>()(expr->eval()); }

    /// Constancy depends on the constancy of the expression
    bool isConstant() { return expr->isConstant(); }
};



} // namespace schnek

#endif /* SCHNEK_EXPRESSION_HPP_ */
