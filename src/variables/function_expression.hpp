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

#include <iterator>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../generic/function_traits.hpp"
#include "expression.hpp"

namespace schnek {

  typedef std::list<ExpressionVariant> ExpressionList;

  template<typename vtype>
  struct ExpressionConverterVisitor : public boost::static_visitor<ExpressionVariant> {
      typedef typename std::shared_ptr<Expression<vtype>> VarExpressionPointer;
      ExpressionVariant operator()(VarExpressionPointer e) { return e; }

      template<class ExpressionPointer>
      ExpressionVariant operator()(ExpressionPointer e) {
        typedef typename ExpressionPointer::element_type::ValueType vtype_orig;
        VarExpressionPointer vEP(new TypecastOp<vtype, vtype_orig, FastCast>(e));
        return vEP;
      }
  };

  template<class vtype, typename func>
  class FunctionExpression : public Expression<vtype> {
    public:
      typedef generic::FunctionReturnTypeT<func> rtype;

    private:
      struct isConstantVisitor : public boost::static_visitor<bool> {
          template<class ExpressionPointer>
          bool operator()(ExpressionPointer e) {
            return e->isConstant();
          }
      };

      ExpressionList args;
      func f;
      bool updateAlways;

    public:
      FunctionExpression(func f_, ExpressionList &args_, bool updateAlways);

      /// Return the modified value
      vtype eval();

      bool isConstant();

      DependencyList getDependencies();
  };

  template<class vtype, typename func>
  struct FunctionExpressionConverter {
      typedef generic::FunctionReturnTypeT<func> rtype;
      typedef generic::FunctionParameterTypesT<func> ArgTuple;
      static constexpr size_t numArgs = std::tuple_size<ArgTuple>::value;

      template<size_t I>
      using ArgType = std::decay_t<std::tuple_element_t<I, ArgTuple>>;

      static void makeList(ExpressionList::iterator var, ExpressionList::iterator end, ExpressionList &args) {
        makeListImpl(var, end, args, std::make_index_sequence<numArgs>{});
      }

      static rtype evaluate(func f, ExpressionList::iterator var) {
        return evaluateImpl(f, var, std::make_index_sequence<numArgs>{});
      }

    private:
      template<typename T>
      static void convertAndAppend(ExpressionVariant &arg, ExpressionList &args) {
        ExpressionConverterVisitor<T> visitor;
        args.push_back(boost::apply_visitor(visitor, arg));
      }

      template<size_t... I>
      static void
      makeListImpl(ExpressionList::iterator var, ExpressionList::iterator end, ExpressionList &args, std::index_sequence<I...>) {
        if (std::distance(var, end) != static_cast<std::ptrdiff_t>(numArgs)) throw WrongNumberOfArgsException();
        (convertAndAppend<ArgType<I>>(*std::next(var, I), args), ...);
      }

      template<size_t... I>
      static rtype evaluateImpl(func f, [[maybe_unused]] ExpressionList::iterator var, std::index_sequence<I...>) {
        // Braced initialisation guarantees left-to-right evaluation of the arguments
        std::tuple<ArgType<I>...> values{
            boost::get<std::shared_ptr<Expression<ArgType<I>>>>(*std::next(var, I))->eval()...
        };
        return std::apply(f, std::move(values));
      }
  };

  template<class vtype, typename func>
  FunctionExpression<vtype, func>::FunctionExpression(func f_, ExpressionList &args_, bool updateAlways_)
      : f(f_), updateAlways(updateAlways_) {
    FunctionExpressionConverter<vtype, func>::makeList(args_.begin(), args_.end(), args);
  }

  template<class vtype, typename func>
  vtype FunctionExpression<vtype, func>::eval() {
    return FunctionExpressionConverter<vtype, func>::evaluate(f, args.begin());
  }

  template<class vtype, typename func>
  bool FunctionExpression<vtype, func>::isConstant() {
    if (updateAlways) return false;
    bool result = true;
    isConstantVisitor visit;
    for (ExpressionVariant ex : args) {
      result = result && boost::apply_visitor(visit, ex);
    }

    return result;
  }

  template<class vtype, typename func>
  DependencyList FunctionExpression<vtype, func>::getDependencies() {
    DependencyList result;

    DependenciesGetter visit;
    for (ExpressionVariant ex : args) {
      DependencyList dep = boost::apply_visitor(visit, ex);
      result.insert(dep.begin(), dep.end());
    }
    if (updateAlways) result.insert(-1);
    return result;
  }

  class FunctionRegistry {
    private:
      class EntryBase {
        public:
          virtual ~EntryBase() {}
          virtual ExpressionVariant getExpression(ExpressionList &) = 0;
      };

      typedef std::shared_ptr<EntryBase> pEntryBase;

      template<typename func>
      class Entry : public EntryBase {
        private:
          typedef generic::FunctionReturnTypeT<func> rtype;
          typedef FunctionExpression<rtype, func> FExprType;

          func f;
          bool updateAlways;

        public:
          Entry(func f_, bool updateAlways_) : f(f_), updateAlways(updateAlways_) {}

          ExpressionVariant getExpression(ExpressionList &args) {
            std::shared_ptr<Expression<rtype>> eP(new FunctionExpression<rtype, func>(f, args, updateAlways));
            return eP;
          }
      };

      typedef std::map<std::string, pEntryBase> FExprMap;
      std::shared_ptr<FExprMap> funcs;

    public:
      FunctionRegistry() : funcs(new FExprMap) {}
      FunctionRegistry(const FunctionRegistry &reg) : funcs(reg.funcs) {}

      template<typename func>
      void registerFunction(std::string fname, func f, bool updateAlways = false) {
        pEntryBase eB(new Entry<func>(f, updateAlways));
        (*funcs)[fname] = eB;
      }

      ExpressionVariant getExpression(std::string fname, ExpressionList &args) {
        if (funcs->count(fname) == 0) throw FunctionNotFoundException(fname);
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

  /** Registers some utility functions
   *
   * Currently the following functions are registered:
   * min, max, minI, maxI
   */
  void registerUtilityFunctions(FunctionRegistry &freg);

  /** Registers some special functions from Boost Math
   *
   * Currently the following functions are registered:
   * gamma, lgamma, digamma, besselj, bessely, normal
   */
  void registerSpecialFunctions(FunctionRegistry &freg);
  void registerAllFunctions(FunctionRegistry &freg);

}  // namespace schnek

#endif  // FUNCTION_EXPRESSION_HPP_
