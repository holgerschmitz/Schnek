/*
 * blockparameters.hpp
 *
 * Created on: 1 May 2012
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

#ifndef SCHNEK_BLOCKPARAMETERS_HPP_
#define SCHNEK_BLOCKPARAMETERS_HPP_

#include "types.hpp"
#include "variables.hpp"
#include "expression.hpp"

#include "../grid/array.hpp"
#include "../util/exceptions.hpp"

#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <map>

namespace schnek {

class DependencyMap;
typedef boost::shared_ptr<DependencyMap> pDependencyMap;
class Parameter;
typedef boost::shared_ptr<Parameter> pParameter;

class ParametersGroup
{
  private:
    typedef std::set<long> ParameterSet;
    ParameterSet parameters;
  public:
    void add(pParameter p);
    void add(pVariable v);
    bool isElement(pParameter p);
    bool isElement(pVariable v);

    template<int rank, template<int> class CheckingPolicy>
    void addArray(Array<pParameter, rank, CheckingPolicy> &pa)
    {
        for (int i=0; i<rank; ++i) add(pa[i]);
    }


    // This method has the side effect of modifying ids to the set containing all the values not
    // contained in the group.
    bool hasElements(std::set<long> &ids);
};

typedef boost::shared_ptr<ParametersGroup> pParametersGroup;


class Parameter
{
  protected:
    std::string varName;
    pVariable variable;
    pParametersGroup allowedDeps;
  public:
    Parameter(std::string varName_, pVariable variable_, pParametersGroup allowedDeps_)
      : varName(varName_), variable(variable_), allowedDeps(allowedDeps_)
    {}
    virtual ~Parameter() {}

    bool canEvaluate() { return (variable) && (variable->isInitialised()); }
    pVariable getVariable() { return variable; }
    pParametersGroup getAllowedDeps() { return allowedDeps; }

//    bool depsAllowed(pDependencyMap deps);

    virtual void evaluate() = 0;
    virtual void update() = 0;
};

template<typename T>
class ConcreteParameter : public Parameter
{
  protected:
    T *value;
  public:
    ConcreteParameter(std::string varName_, pVariable variable_, T *value_, pParametersGroup allowedDeps_)
      : Parameter(varName_, variable_, allowedDeps_), value(value_) {}

    void evaluate()
    {
      //std::cout << "Evaluating Parameter " << varName << "\n";
      if (! variable->isInitialised())
        throw VariableNotInitialisedException(varName);

      if (variable->isReadOnly())
      {
//        std::cout << "  read only " << varName << "=" << *value << "\n";
        return;
      }

      if (variable->isConstant())
        *value = boost::get<T>(variable->getValue());
      else
        *value = boost::get<T>(variable->evaluateExpression());
    }

    void update()
    {
      if (variable->isReadOnly()) return;
      *value = boost::get<T>(variable->getValue());
    }
};


template<typename T>
class ConstantParameter : public Parameter
{
  protected:
  public:
    ConstantParameter(std::string varName_, pVariable variable_, const T &value_)
      : Parameter(varName_, variable_, pParametersGroup(new ParametersGroup())) {}

    void evaluate()
    {
      return;
    }

    void update()
    {
      return;
    }
};

class BlockParameters
{
  private:
    pBlockVariables block;
    std::map<std::string, pParameter> parameterMap;
  public:
    typedef enum {readwrite, readonly} Permissions;
    void setContext(pBlockVariables context)
    {
      block = context;
    }

    pBlockVariables getContext()
    {
      return block;
    }


    template<typename T>
    pParameter addParameter(std::string varName,
                            T* var,
                            pParametersGroup allowedDeps,
                            bool hasDefault = false,
                            const T &defaultValue = T(),
                            Permissions perm=readwrite)
    {
      pVariable variable;
      if (perm==readwrite)
        variable = pVariable(new Variable(defaultValue, hasDefault, false));
      else
      {
        typedef boost::shared_ptr<Expression<T> > ParExpression;
        ParExpression pexp(new ExternalValue<T>(var));
        if (hasDefault) *var = defaultValue;
        variable = pVariable(new Variable(pexp, true, true));
      }
      block->addVariable(varName, variable);

      if (varName=="test4") std::cerr << "test4: ID = " << variable->getId();

      pParameter par(new ConcreteParameter<T>(varName, variable, var, allowedDeps));
      parameterMap[varName] = par;
      return par;
    }

    template<typename T>
    pParameter addParameter(std::string varName, T* var, Permissions perm=readwrite)
    {
      pParametersGroup empty(new ParametersGroup());
      return addParameter(varName, var, empty, false, T(), perm);
    }

    template<typename T>
    pParameter addParameter(std::string varName, T* var, const T &defaultValue, Permissions perm=readwrite)
    {
      pParametersGroup empty(new ParametersGroup());
      return addParameter(varName, var, empty, true, defaultValue, perm);
    }

    template<typename T>
    pParameter addConstant(std::string varName, const T &value)
    {
      pVariable variable(new Variable(value, true, true));
      block->addVariable(varName, variable);

      pParameter par(new ConstantParameter<T>(varName, variable, value));
      parameterMap[varName] = par;
      return par;
    }

    template<
      class T,
      int rank,
      template<int> class CheckingPolicy
    >
    Array<pParameter, rank, CheckingPolicy> addArrayParameter(
        std::string varName,
        Array<T, rank, CheckingPolicy> &var,
        Permissions perm=readwrite,
        std::string extension = "xyzuvw")
    {
        SCHNEK_REQUIRE(rank<=extension.length(), "addArrayParameter: extension string not long enough! Rank = "
        		+ boost::lexical_cast<std::string>(rank) + ",  extension.length = "+ boost::lexical_cast<std::string>(extension.length())
				+ " (\"" +extension+"\")");
        Array<pParameter, rank, CheckingPolicy>  result;
        for (int i=0; i<rank; ++i)
          result[i] = addParameter(varName+extension[i], &(var[i]), perm);
        return result;
    }

    template<
      class T,
      int rank,
      template<int> class CheckingPolicy
    >
    Array<pParameter, rank, CheckingPolicy> addArrayParameter(
        std::string varName,
        Array<T, rank, CheckingPolicy> &var,
        Array<T, rank, CheckingPolicy> default_values,
        Permissions perm=readwrite,
        std::string extension = "xyzuvw")
    {
        SCHNEK_REQUIRE(rank<=extension.length(), "addArrayParameter: extension string not long enough! Rank = "
        		+ boost::lexical_cast<std::string>(rank) + ",  extension.length = "+ boost::lexical_cast<std::string>(extension.length())
				+ " (\"" +extension+"\")");
        Array<pParameter, rank, CheckingPolicy>  result;
        for (int i=0; i<rank; ++i)
          result[i] = addParameter(varName+extension[i], &(var[i]), default_values[i], perm);
        return result;
    }

    template<
      class T,
      int rank,
      template<int> class CheckingPolicy
    >
    Array<pParameter, rank, CheckingPolicy> addArrayParameter(
        std::string varName,
        Array<T, rank, CheckingPolicy> &var,
        T default_value,
        Permissions perm=readwrite,
        std::string extension = "xyzuvw")
    {
        SCHNEK_REQUIRE(rank<=extension.length(), "addArrayParameter: extension string not long enough! Rank = "
        		+ boost::lexical_cast<std::string>(rank) + ",  extension.length = "+ boost::lexical_cast<std::string>(extension.length())
				+ " (\"" +extension+"\")");
        Array<pParameter, rank, CheckingPolicy>  result;
        for (int i=0; i<rank; ++i)
          result[i] = addParameter(varName+extension[i], &(var[i]), default_value, perm);
        return result;
    }

    void evaluate()
    {
      typedef std::pair<std::string, pParameter> ParameterPair;
      BOOST_FOREACH(ParameterPair par, parameterMap)
      {
        par.second->evaluate();
      }
    }
};

} //namespace

#endif // SCHNEK_BLOCKPARAMETERS_HPP_
