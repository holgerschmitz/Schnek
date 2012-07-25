/*
 * dependencies.hpp
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

#ifndef SCHNEK_DEPENDENCIES_HPP_
#define SCHNEK_DEPENDENCIES_HPP_

#include "variables.hpp"

#include <boost/variant.hpp>

#include <map>
#include <set>

namespace schnek {

class DependencyMap
{
  private:
    typedef std::set<long> DependencyList;
    struct VarInfo
    {
      pVariable v;
      DependencyList dep;
      int counter;
      VarInfo(pVariable v_, DependencyList dep_) : v(v_), dep(dep_), visited(false) {}
    };

    typedef std::map<long, VarInfo> DepMap;

    DepMap dependencies;

    void constructMapRecursive(const pBlockVariables vars, DepMap& backDep);
    void constructMap(const pBlockVariables vars);
    void clearCounters();

    struct DependenciesGetter : public boost::static_visitor<DepList>
    {
      template<class ExpressionPointer>
      DepList operator()(ExpressionPointer e) { return e->getDependencies(); }
    };

  public:
    DependencyMap(const pBlockVariables vars);
};

} // namespace


#endif // SCHNEK_DEPENDENCIES_HPP_
