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

#include "../variables/types.hpp"
#include "../variables/variables.hpp"
#include "../variables/expression.hpp"

#include <boost/foreach.hpp>
#include <map>

namespace schnek {

class Parameter
{
  protected:
    std::string varName;
    pVariable variable;
  public:
    Parameter(std::string varName_, pVariable variable_)
      : varName(varName_), variable(variable_)
    {}

    bool canEvaluate() { return (variable) && (variable->isInitialised()); }
    pVariable getVariable() { return variable; }

    virtual void evaluate() = 0;
    virtual void update() = 0;
};

typedef boost::shared_ptr<Parameter> pParameter;

template<typename T>
class ConcreteParameter : public Parameter
{
  protected:
    T *value;
  public:
    ConcreteParameter(std::string varName_, pVariable variable_, T *value_)
      : Parameter(varName_, variable_), value(value_) {}

    void evaluate()
    {
      std::cout << "Evaluating Parameter" << varName << "\n";
      if (! variable->isInitialised())
        throw VariableNotInitialisedException();

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

    template<typename T>
    pParameter addParameter(std::string varName, T* var, Permissions perm=readwrite)
    {
      T defaultValue = T(); // ensure default values
      pVariable variable;
      if (perm==readwrite)
        variable = pVariable(new Variable(defaultValue, false, false));
      else
      {
        typedef boost::shared_ptr<Expression<T> > ParExpression;
        ParExpression pexp(new ExternalValue<T>(var));
        variable = pVariable(new Variable(pexp, true, true));
      }
      block->addVariable(varName, variable);

      pParameter par(new ConcreteParameter<T>(varName, variable, var));
      parameterMap[varName] = par;
      return par;
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
