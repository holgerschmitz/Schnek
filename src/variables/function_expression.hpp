/*
 * function_expression.hpp
 *
 * Created on: 22 Mar 2012
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

#ifndef FUNCTION_EXPRESSION_HPP_
#define FUNCTION_EXPRESSION_HPP_

#include "expression.hpp"
#include <boost/foreach.hpp>

namespace schnek {

typedef std::list<ExpressionVariant> ExpressionList;

template<typename vtype>
struct ExpressionConverterVisitor : public boost::static_visitor<ExpressionVariant>
{
    typedef typename boost::shared_ptr<Expression<vtype> > VarExpressionPointer;
    ExpressionVariant operator()(VarExpressionPointer e)
    {
      return e;
    }

    template<class ExpressionPointer>
    ExpressionVariant operator()(ExpressionPointer e)
    {
        typedef typename ExpressionPointer::element_type::ValueType vtype_orig;
        VarExpressionPointer vEP(new TypecastOp<vtype, vtype_orig, FastCast>(e));
        return vEP;
    }
};

template<class vtype, typename func>
class FunctionExpression : public Expression<vtype>
{
  public:
    typedef typename bft::result_type<func>::type rtype;

    template<
      typename from = typename mpl::begin< bft::parameter_types<func> >::type,
      typename to = typename mpl::end< bft::parameter_types<func> >::type
    >
    struct converter;

  private:
    struct isConstantVisitor : public boost::static_visitor<bool>
    {
        template<class ExpressionPointer>
        bool operator()(ExpressionPointer e) { return e->isConstant(); }
    };


    ExpressionList args;
    func f;
  public:
    FunctionExpression(func f_, ExpressionList &args_);

    /// Return the modified value
    vtype eval();

    bool isConstant();

    DependencyList getDependencies();
};

template<class vtype, typename func>
template<typename to>
struct FunctionExpression<vtype, func>::converter<to, to>
{
  typedef typename bft::result_type<func>::type rtype;
  static void makeList(ExpressionList::iterator var, ExpressionList::iterator end, ExpressionList &args)
  {
    if (var != end) throw WrongNumberOfArgsException();
  }

  template<typename ArgType>
  static rtype evaluate(func f, ExpressionList::iterator var, ArgType const &sArgs)
  {
    return fusion::invoke(f, sArgs);
  }
};

template<class vtype, typename func>
template<
  typename from,
  typename to
>
struct FunctionExpression<vtype, func>::converter
{
    typedef typename bft::result_type<func>::type rtype;
    typedef typename mpl::deref<from>::type arg_type;
    typedef typename mpl::next<from>::type next_type_iter;

    static void makeList(ExpressionList::iterator var, ExpressionList::iterator end, ExpressionList &args)
    {
      if (var == end) throw WrongNumberOfArgsException();

      ExpressionConverterVisitor<arg_type> visit;
      args.push_back(boost::apply_visitor(visit, (*var)));
      ++var;
      FunctionExpression<vtype, func>::converter<next_type_iter, to>::makeList(var, end, args);
    }

    template<typename ArgType>
    static rtype evaluate(func f, ExpressionList::iterator var, ArgType const &sArgs)
    {
      typedef boost::shared_ptr< Expression<arg_type> > pExprType;
      pExprType expr = boost::get<pExprType>(*var);
      ++var;
      return FunctionExpression<vtype, func>::converter<next_type_iter, to>::evaluate(f, var, fusion::push_back(sArgs, expr->eval()));
    }

};

template<class vtype, typename func>
FunctionExpression<vtype, func>::FunctionExpression(func f_, ExpressionList &args_)
  : f(f_)
{
    converter<>::makeList(args_.begin(), args_.end(), args);
}

template<class vtype, typename func>
vtype FunctionExpression<vtype, func>::eval()
{
    return converter<>::evaluate(f, args.begin(), fusion::nil());
}

template<class vtype, typename func>
bool FunctionExpression<vtype, func>::isConstant()
{
  bool result = true;
  isConstantVisitor visit;
  BOOST_FOREACH(ExpressionVariant ex, args)
  {
    result = result && boost::apply_visitor(visit, ex);
  }

  return result;
}

template<class vtype, typename func>
DependencyList FunctionExpression<vtype, func>::getDependencies()
{
  DependencyList result;

  DependenciesGetter visit;
  BOOST_FOREACH(ExpressionVariant ex, args)
  {
    DependencyList dep = boost::apply_visitor(visit, ex);
    result.insert(dep.begin(), dep.end());
  }
  return result;
}

class FunctionRegistry
{
  private:

    class EntryBase
    {
      public:
        virtual ExpressionVariant getExpression(ExpressionList &) = 0;
    };

    typedef boost::shared_ptr<EntryBase> pEntryBase;

    template<typename func>
    class Entry : public EntryBase
    {
      private:
        typedef typename bft::result_type<func>::type rtype;
        typedef FunctionExpression<rtype, func> FExprType;

        func f;
      public:
        Entry(func f_) : f(f_) {}

        ExpressionVariant getExpression(ExpressionList &args)
        {
          boost::shared_ptr<Expression<rtype> > eP(new FunctionExpression<rtype, func>(f, args));
          return eP;
        }
    };


    typedef std::map<std::string, pEntryBase> FExprMap;
    boost::shared_ptr<FExprMap> funcs;

  public:
    FunctionRegistry() : funcs(new FExprMap) {}
    FunctionRegistry(const FunctionRegistry &reg) : funcs(reg.funcs) {}

    template<typename func>
    void registerFunction(std::string fname, func f)
    {
      pEntryBase eB(new Entry<func>(f));
      (*funcs)[fname] = eB;
    }

    ExpressionVariant getExpression(std::string fname, ExpressionList &args)
    {
      if (funcs->count(fname) == 0) throw FunctionNotFoundException();
      return (*funcs)[fname]->getExpression(args);
    }

};
 /** Registers many functions supplied by cmath with the function registry.
  * This allows the functions to be used in the input deck and for evaluating variables.
  *
  * Currently the following functions are registered:
  * cos, sin, tan,  acos, asin, atan, atan2, cosh, sinh, tanh, exp,
  * ldexp, log, log10, pow, sqrt, ceil, fabs, floor, fmod
  */
void registerCMath(FunctionRegistry &freg);

} // namespace schnek

#endif // FUNCTION_EXPRESSION_HPP_
