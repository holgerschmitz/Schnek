/*
 * variables.cpp
 *
 * Created on: 19 Jul 2011
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

#include "types.hpp"
#include "variables.hpp"
#include "expression.hpp"
#include "../exception.hpp"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/foreach.hpp>

using namespace schnek;


// -------------------------------------------------------------
// Variable
// -------------------------------------------------------------


Variable::Variable(int value, bool initialised_, bool readonly_)
  : var(value),
    type(int_type),
    fixed(true),
    initialised(initialised_),
    readonly(readonly_),
    uniqueId(new Unique<Variable>())
{}

Variable::Variable(double value, bool initialised_, bool readonly_)
  : var(value),
    type(float_type),
    fixed(true),
    initialised(initialised_),
    readonly(readonly_),
    uniqueId(new Unique<Variable>())
{}

Variable::Variable(std::string value, bool initialised_, bool readonly_)
  : var(value),
    type(string_type),
    fixed(true),
    initialised(initialised_),
    readonly(readonly_),
    uniqueId(new Unique<Variable>())
{}

Variable::Variable(pIntExpression expr, bool initialised_, bool readonly_)
  : expression(expr),
    type(int_type),
    fixed(false),
    initialised(initialised_),
    readonly(readonly_),
    uniqueId(new Unique<Variable>())
{}

Variable::Variable(pFloatExpression expr, bool initialised_, bool readonly_)
  : expression(expr),
    type(float_type),
    fixed(false),
    initialised(initialised_),
    readonly(readonly_),
    uniqueId(new Unique<Variable>())
{}

Variable::Variable(pStringExpression expr, bool initialised_, bool readonly_)
  : expression(expr),
    type(string_type),
    fixed(false),
    initialised(initialised_),
    readonly(readonly_),
    uniqueId(new Unique<Variable>())
{}

Variable::Variable(const Variable &var)
  : expression(var.expression),
    var(var.var),
    type(var.type),
    fixed(var.fixed),
    initialised(var.initialised),
    readonly(var.readonly),
    uniqueId(var.uniqueId)
{}

Variable &Variable::operator=(const Variable &rhs)
{
  if (type != rhs.type) throw TypeMismatchException();
  if (readonly) throw ReadOnlyAssignmentException();
  var = rhs.var;
  expression = rhs.expression;
  fixed = rhs.fixed;
  initialised = rhs.initialised;
  uniqueId = rhs.uniqueId;
  return *this;
}


const ValueVariant &Variable::evaluateExpression()
{
  ExpressionEvaluator<ValueVariant> eval;
  var = boost::apply_visitor(eval, expression);
  return var;
}

// -------------------------------------------------------------
// BlockVariables
// -------------------------------------------------------------


bool BlockVariables::exists(std::list<std::string> path, bool upward)
{
  std::string name = path.front();
  if (path.size()>1)
  {
    if (childrenByName.count(name)>0)
    {
      path.pop_front();
      // if the child exists, we are forced to go down into the tree
      // the upward=false ensures that the search will not come back
      // up the hierarchy
      return childrenByName[name]->exists(path, false);
    }
  }
  else
  {
    if (vars.count(name)>0) return true;
  }
  if (upward && (parent.get()==0)) return parent->exists(path, true);

  return false;
}

bool BlockVariables::exists(std::string name)
{
  std::list<std::string> path;
  boost::split(path, name, boost::is_any_of("."));
  return exists(path, true);
}

pVariable BlockVariables::getVariable(std::list<std::string> path, bool upward)
{
  //std::cout << "BlockVariables " << getBlockName() << "(" << getClassName() << ")" << std::endl;
  std::string name = path.front();
  if (path.size()>1)
  {
    //std::cout << "  -- Going into path " << name << std::endl;
    if (childrenByName.count(name)>0)
    {
      path.pop_front();
      // if the child exists, we are forced to go down into the tree
      // the upward=false ensures that the search will not come back
      // up the hierarchy
      return childrenByName[name]->getVariable(path, false);
    }
  }
  else
  {
    //std::cout << "  -- Checking Variable " << name << std::endl;
    if (vars.count(name)>0) return vars[name];
  }

  if (upward && parent) return parent->getVariable(path, true);

  throw VariableNotFoundException("Could not find Block variable "+path.back());
}

pVariable BlockVariables::getVariable(std::string name)
{
  std::list<std::string> path;
  boost::split(path, name, boost::is_any_of(":"));
  return getVariable(path, true);
}

bool BlockVariables::addVariable(std::string name, pVariable variable)
{
  if (vars.count(name)>0) return false;
  vars[name] = variable;
  return true;
}

bool BlockVariables::addChild(pBlockVariables child)
{
  if (childrenByName.count(child->getBlockName())>0) return false;
  childrenByName[child->getBlockName()] = child;
  childrenByClassName[child->getClassName()].push_back(child);
  children.push_back(child);
  return true;
}


// -------------------------------------------------------------
// VariableStorage
// -------------------------------------------------------------


VariableStorage::VariableStorage(std::string name, std::string classname)
{
  pBlockVariables bV(new BlockVariables(name, classname, pBlockVariables()));
  root = bV;
  cursor = root;
}

void VariableStorage::resetCursor()
{
  cursor = root;
}

void VariableStorage::cursorUp()
{
  cursor = cursor->getParent();
  if (cursor == 0) cursor = root;
}

void VariableStorage::addVariable(std::string name, pVariable variable)
{
  if (!cursor->addVariable(name, variable))
    throw DuplicateVariableException();
}

pBlockVariables VariableStorage::createBlock(std::string name, std::string classname)
{
  pBlockVariables new_block(new BlockVariables(name,classname,cursor));
  if (!cursor->addChild(new_block))
    throw DuplicateBlockException();
  cursor = new_block;
  return new_block;
}
