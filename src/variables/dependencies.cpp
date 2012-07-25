/*
 * dependencies.cpp
 *
 * Created on: 10 Jul 2012
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

#include "dependencies.hpp"
#include "../exception.hpp"

#include <boost/variant/apply_visitor.hpp>
#include <boost/foreach.hpp>

using namespace schnek;

DependencyMap::DependencyMap(const pBlockVariables vars)
{
  constructMap(vars);
}

void DependencyMap::constructMapRecursive(const pBlockVariables vars, DepMap &backDep)
{
  BOOST_FOREACH(VariableMap::value_type it, vars->getVariables())
  {
    pVariable v=it.second;
    if (!v->isConstant())
    {
      DependenciesGetter depGet;
      DepList dep = boost::apply_visitor(depGet, v->getExpression());
      long id = v->getId();
      if (backDep.count(id)>0) throw SchnekException();
      backDep[id] = VarInfo(v, dep);
      dependencies[id] = VarInfo(v, DepList(), dep);
    }
  }

  BOOST_FOREACH(pBlockVariables ch, vars->getChildren())
  {
    constructMapRecursive(ch, backDep);
  }
}
void DependencyMap::constructMap(const pBlockVariables vars)
{
	DepMap backDep;

	constructMapRecursive(vars, backDep);

	BOOST_FOREACH(DepMap::value_type entry, backDep)
	{
	  BOOST_FOREACH(long id, entry.second.dep)
	  {
	    dependencies[id].dep.push_back(entry.first);
	  }
	}
}

void DependencyMap::resetCounters()
{
	BOOST_FOREACH(DepMap::value_type entry, dependencies)
	{
	  entry.second.counter = entry.second.dep.size();
	}
}

