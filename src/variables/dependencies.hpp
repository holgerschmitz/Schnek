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
#include "blockparameters.hpp"

#include <boost/variant.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>

#include <map>
#include <set>
#include <list>

namespace schnek {

class DependencyUpdater;

/** DependencyMap contains a directed graph whose edges are oriented from the
 *  independent to the dependent variables.
 *
 */
class DependencyMap
{
  private:
    typedef std::set<long> DependencySet;
    struct VarInfo
    {
      pVariable v;
      DependencySet dependsOn;
      DependencySet modifies;
      int counter;
      VarInfo() {}
      VarInfo(pVariable v_, DependencySet dependsOn_, DependencySet modifies_)
        : v(v_), dependsOn(dependsOn_), modifies(modifies_), counter(0) {
        assert(v);
      }
    };

    typedef std::map<long, VarInfo> DepMap;

    /// This is used internally. The pointers are to VarInfo objects stored in the dependencies map.
    typedef std::map<long, VarInfo*> RefDepMap;
    typedef boost::shared_ptr<RefDepMap> pRefDepMap;

    typedef std::set<pVariable> VariableSet;
    typedef std::list<pVariable> VariableList;

    DepMap dependencies;
    pBlockVariables blockVars;

    pVariable dummyVar;

    friend class DependencyUpdater;

    void constructMapRecursive(const pBlockVariables vars);
    void constructMap(const pBlockVariables vars);
    void resetCounters();

    void makeUpdateList(const VariableSet &independentVars, const VariableSet &dependentVars, VariableList &updateList);
    pRefDepMap makeUpdatePredecessors(const VariableSet &independentVars, const VariableSet &dependentVars);
    pRefDepMap makeUpdateFollowers(const VariableSet &independentVars, pRefDepMap reverseDeps);
    void makeUpdateOrder(pRefDepMap deps, VariableList &updateList);

  public:
    DependencyMap(const pBlockVariables vars);
    void recreate() { constructMap(blockVars); }
    pBlockVariables getBlockVariables();
    void updateAll();

//    bool hasRoots(pVariable v, pParametersGroup roots);
};

typedef boost::shared_ptr<DependencyMap> pDependencyMap;

class DependencyUpdater
{
  private:
    typedef std::set<pParameter> ParameterSet;
    typedef std::set<pVariable> VariableSet;
    typedef std::list<pVariable> VariableList;

    VariableList updateList;
    VariableSet independentVars;
    VariableSet dependentVars;
    ParameterSet dependentParameters;

    pDependencyMap dependencies;
    bool isValid;
  public:
    DependencyUpdater(pDependencyMap dependencies_);
    void addIndependent(pParameter v);
    void addDependent(pParameter v);
    void clearDependent();

    template<int rank, template<int> class CheckingPolicy>
    void addIndependentArray(Array<pParameter, rank, CheckingPolicy> v)
    { for (int i=0; i<rank; ++i) addIndependent(v[i]); }

    template<int rank, template<int> class CheckingPolicy>
    void addDependentArray(Array<pParameter, rank, CheckingPolicy> v)
    { for (int i=0; i<rank; ++i) addDependent(v[i]); }

    /** Updates the dependent variables and all the variables needed to evaluate them.
     *
     *  This method is inline because it is potentially speed critical.
     */
    void update()
    {
      if (!isValid) {
        dependencies->makeUpdateList(independentVars, dependentVars, updateList);
        isValid = true;
      }
      BOOST_FOREACH(pVariable v, updateList) v->evaluateExpression();
      BOOST_FOREACH(pParameter p, dependentParameters) p->update();
    }
};

typedef boost::shared_ptr<DependencyUpdater> pDependencyUpdater;

} // namespace


#endif // SCHNEK_DEPENDENCIES_HPP_
