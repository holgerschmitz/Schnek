/*
 * expression.hpp
 *
 * Created on: 26 Jan 2011
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

#ifndef SCHNEK_EXPRESSION_HPP_
#define SCHNEK_EXPRESSION_HPP_

#include "variables.hpp"
#include "../util/logger.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy"

#include <boost/lexical_cast.hpp>
#include <boost/function.hpp>

#include <boost/fusion/include/push_back.hpp>
#include <boost/fusion/include/invoke.hpp>
#include <boost/fusion/include/cons.hpp>

#include <boost/mpl/begin.hpp>
#include <boost/mpl/end.hpp>
#include <boost/mpl/next.hpp>
#include <boost/mpl/deref.hpp>

#pragma GCC diagnostic pop
#pragma GCC diagnostic pop

#include <memory>
#include <string>
#include <iostream>
#include <set>

#undef LOGLEVEL
#define LOGLEVEL 0

namespace fusion = boost::fusion;
namespace mpl = boost::mpl;
namespace bft = boost::function_types;

namespace schnek {

class WrongNumberOfArgsException : public EvaluationException
{
  public:
    WrongNumberOfArgsException() : EvaluationException("Wrong number of arguments!") {}
};

class FunctionNotFoundException : public EvaluationException
{
  public:
    FunctionNotFoundException(std::string functionName) : EvaluationException("Function '"+functionName+"' was not defined!") {}
};

class TypeCastException : public EvaluationException
{
  public:
    TypeCastException() : EvaluationException("Could not convert types") {}
};

typedef std::set<long> DependencyList;

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
    virtual ~Expression() {}

    /// The virtual method that causes the expression to evaluate
    virtual vtype eval() = 0;

    /// Returns whether the expression can be evaluated, are all the references also constant
    virtual bool isConstant() = 0;

    /// The () operator allows expressions to be used as function objects
    vtype operator()() { return eval(); }

    virtual DependencyList getDependencies() { return DependencyList(); }

    /// A pointer to an Expression
    typedef std::shared_ptr<Expression> pExpression;
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
    vtype eval()
    {
      SCHNEK_TRACE_LOG(5, "Value<vtype>::eval(" << val << ")")
      return val;
    }
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
    /// Contains the variable
    pVariable var;
  public:
    /// Construct with a value
    ReferencedValue(const pVariable &var_) : var(var_) {}
    /// Return the modified value
    vtype eval() {
//      if (var->isConstant())
//      {
//        //std::cerr << "Attempting to assign variable\n";
//        return boost::get<vtype>(var->getValue());
//      }
//      else
//        return boost::get<vtype>(var->evaluateExpression());
       // it is assumed that the referenced variable has been evaluated
      SCHNEK_TRACE_LOG(5, "ReferencedValue<vtype>::eval(" << var->getValue() << ")")
      return boost::get<vtype>(var->getValue());
    }
    /// Constancy depends on the constancy of the variable
    bool isConstant() { return var->isConstant(); }

    /// returns a list with the variable's id
    DependencyList getDependencies()
    {
      DependencyList dep;
      dep.insert(var->getId());
      return dep;
    }
};

/** A special type of expresion that holds a reference to an external value
 */
template<typename vtype>
class ExternalValue : public Expression<vtype>
{
  private:
    /// Contains the pointer to the external value
    vtype *var;
  public:
    /// Construct with a pointer to the value
    ExternalValue(vtype *var_) : var(var_) {}

    /// Return the value
    vtype eval() {
//      std::cout << "Returning value " << *var << "\n";
      SCHNEK_TRACE_LOG(5, "ExternalValue<vtype>::eval(" << *var << ")")
      return *var;
    }

    /// The value of the external variable can change
    bool isConstant() { return false; }
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

    /// returns the dependencies of the sub expression
    DependencyList getDependencies()
    {
      return expr->getDependencies();
    }
};

template<class vtype>
struct ExpressionInfo {
  bool positive;
  typename Expression<vtype>::pExpression expression;
  ExpressionInfo(bool positive_, typename Expression<vtype>::pExpression expression_)
    : positive(positive_), expression(expression_) {}
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
    typedef BinaryOp<oper, vtype> SelfType;
    typedef typename Expression<vtype>::pExpression pExpression;

    typedef BinaryOp<typename oper::Inverted, vtype> InvType;
    typedef std::shared_ptr<InvType> pInvType;
    friend class BinaryOp<typename oper::Inverted, vtype>;

    /// pointers to the expressions to modify
    std::list<ExpressionInfo<vtype> > expressions;
  public:
    BinaryOp(pExpression expr1_, pExpression expr2_)
    {
      typedef std::shared_ptr<BinaryOp<oper, vtype> > pBinaryOp;

      pBinaryOp binaryExpr1 = std::dynamic_pointer_cast<SelfType>(expr1_);
      pInvType invExpr1 = std::dynamic_pointer_cast<InvType>(expr1_);

      pBinaryOp binaryExpr2 = std::dynamic_pointer_cast<SelfType>(expr2_);
      pInvType invExpr2 = std::dynamic_pointer_cast<InvType>(expr2_);

      if (binaryExpr1)
      {
        expressions.insert(expressions.end(), binaryExpr1->expressions.begin(), binaryExpr1->expressions.end());
      }
      else if (invExpr1)
      {
        expressions.insert(expressions.end(), invExpr1->expressions.begin(), invExpr1->expressions.end());
      }
      else
      {
        expressions.push_back(ExpressionInfo<vtype>(true, expr1_));
      }

      if (binaryExpr2)
      {
        typename std::list<ExpressionInfo<vtype> >::iterator it = binaryExpr2->expressions.begin();
        if (!oper::isPositive)
        {
          expressions.push_back(ExpressionInfo<vtype>(!it->positive, it->expression));
          ++it;
        }
        expressions.insert(expressions.end(), it, binaryExpr2->expressions.end());
      }
      else if (invExpr2)
      {
        typename std::list<ExpressionInfo<vtype> >::iterator it = invExpr2->expressions.begin();
        if (!oper::isPositive)
        {
          expressions.push_back(ExpressionInfo<vtype>(!it->positive, it->expression));
          ++it;
        }
        expressions.insert(expressions.end(), it, invExpr2->expressions.end());
      }
      else
      {
        expressions.push_back(ExpressionInfo<vtype>(oper::isPositive, expr2_));
      }
    }

    /// Return the calculated value
    vtype eval() {
      typedef typename oper::Positive opPositive;
      typedef typename oper::Negative opNegative;
      typename std::list<ExpressionInfo<vtype> >::iterator it = expressions.begin();
//      std::cout << std::endl << "EVAL: " << expressions.size() << std::endl;
      vtype val = it->expression->eval();

      while (++it != expressions.end())
      {
//        std::cout << "    : " << val << " " << it->positive << " " << it->expression->eval() << " ";
        val = it->positive ? opPositive::eval(val, it->expression->eval()) : opNegative::eval(val, it->expression->eval());
//        std::cout << val << std::endl;
      }
      return val;
    }

    /// Constancy depends on the constancy of both expressions
    bool isConstant() {
      for(ExpressionInfo<vtype> exp: expressions)
      {
          if (!exp.expression->isConstant()) return false;
      }
      return true;
    }

    /// returns the joint dependencies of both sub expression
    DependencyList getDependencies()
    {
      DependencyList dependencies;

      for(ExpressionInfo<vtype> exp: expressions)
      {
        DependencyList dep = exp.expression->getDependencies();
        dependencies.insert(dep.begin(), dep.end());
      }
      return dependencies;
    }
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
    vtype eval() {
      try {
        return CastType<vtype>()(expr->eval());
      }
      catch(boost::bad_lexical_cast &e)
      {
        throw TypeCastException();
      }
    }

    /// Constancy depends on the constancy of the expression
    bool isConstant() { return expr->isConstant(); }

    /// returns the dependencies of the sub expression
    DependencyList getDependencies()
    {
      return expr->getDependencies();
    }
};

struct DependenciesGetter : public boost::static_visitor<DependencyList>
{
  template<class ExpressionPointer>
  DependencyList operator()(ExpressionPointer e) { return e->getDependencies(); }
};

#undef LOGLEVEL
#define LOGLEVEL 0

} // namespace schnek

#endif // SCHNEK_EXPRESSION_HPP_
