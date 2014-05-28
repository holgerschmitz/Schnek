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
#include <set>

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

    /// returns the joint dependencies of both sub expression
    DependencyList getDependencies()
    {
        DependencyList dep1 = expr1->getDependencies();
        DependencyList dep2 = expr2->getDependencies();
        dep1.insert(dep2.begin(), dep2.end());
        return dep1;
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
    vtype eval() { return CastType<vtype>()(expr->eval()); }

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



} // namespace schnek

#endif // SCHNEK_EXPRESSION_HPP_
