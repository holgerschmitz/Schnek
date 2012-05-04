/*
 * variables.cpp
 *
 *  Created on: 19 Jul 2011
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
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


Variable::Variable(int value, bool initialised_)
  : var(value), type(int_type), fixed(true), initialised(initialised_)
{}

Variable::Variable(double value, bool initialised_)
  : var(value), type(float_type), fixed(true), initialised(initialised_)
{}

Variable::Variable(std::string value, bool initialised_)
  : var(value), type(string_type), fixed(true), initialised(initialised_)
{}

Variable::Variable(pIntExpression expr, bool initialised_)
  : expression(expr), type(int_type), fixed(false), initialised(initialised_)
{}

Variable::Variable(pFloatExpression expr, bool initialised_)
  : expression(expr), type(float_type), fixed(false), initialised(initialised_)
{}

Variable::Variable(pStringExpression expr, bool initialised_)
  : expression(expr), type(string_type), fixed(false), initialised(initialised_)
{}

Variable::Variable(const Variable &var)
  : expression(var.expression), var(var.var), type(var.type), fixed(var.fixed), initialised(var.initialised)
{}

Variable &Variable::operator=(const Variable &rhs)
{
  if (type != rhs.type) throw TypeMismatchException();
  var = rhs.var;
  expression = rhs.expression;
  fixed = rhs.fixed;
  initialised = rhs.initialised;
}


ValueVariant Variable::evaluateExpression()
{
  ExpressionEvaluator<ValueVariant> eval;
  return boost::apply_visitor(eval, expression);
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
  std::string name = path.front();
  if (path.size()>1)
  {
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
    if (vars.count(name)>0) return vars[name];
  }
  if (upward && parent) return parent->getVariable(path, true);

  throw VariableNotFoundException();
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
