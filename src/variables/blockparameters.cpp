/*
 * blockparameters.cpp
 *
 * Created on: 6 Sep 2012
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

#include "blockparameters.hpp"
#include "dependencies.hpp"

using namespace schnek;

void ParametersGroup::add(pParameter p)
{
  parameters.insert(p->getVariable()->getId());
}

void ParametersGroup::add(pVariable v)
{
  parameters.insert(v->getId());
}

bool ParametersGroup::isElement(pParameter p)
{
  return parameters.count(p->getVariable()->getId()) > 0;
}

bool ParametersGroup::isElement(pVariable v)
{
  return parameters.count(v->getId()) > 0;
}

bool ParametersGroup::hasElements(std::set<long> &ids)
{
  ids.erase(parameters.begin(), parameters.end());
  return ids.empty();
}

//bool Parameter::depsAllowed(pDependencyMap deps)
//{
//  return deps->hasRoots(variable, allowedDeps);
//}

