/*
 * domaindecomposition.hpp
 *
 * Created on: 15 Jan 2020
 * Author: Holger Schmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2020 Holger Schmitz
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


/** @file domaindecomposition.hpp
 *  @brief Interface for wrapping and exchanging boundaries
 *
 *  This interface is used to exchange the boundaries of distribution
 *  functions and scalar fields. It can be implemented to define
 *  periodic boundaries or exchange data with other processes.
 */

#ifndef SCHNEK_DOMAINDECOMPOSITION_HPP
#define SCHNEK_DOMAINDECOMPOSITION_HPP

#include "../grid/array.hpp"
#include "../grid/arraycheck.hpp"
#include "../grid/boundary.hpp"
#include "../grid/range.hpp"

#include <boost/shared_ptr.hpp>

#include <iterator>
#include <vector>

namespace schnek {

/**
 * A local domain for the current process
 *
 * Each process can have multiple local domains.
 */
template<int rank, template<int> class CheckingPolicy = ArrayNoArgCheck>
class LocalDomain
{
  public:
    typedef Array<int,rank,ArrayNoArgCheck> IndexType;
    typedef Range<int,rank,ArrayNoArgCheck> RangeType;
  private:
    RangeType range;
    RangeType innerRange;
  public:
    LocalDomain(RangeType range_, RangeType innerRange_) : range(range_), innerRange(innerRange_) {}
    const RangeType &getRange() { return range; }
    const IndexType &getLo() { return range.getLo(); }
    const IndexType &getHi() { return range.getHi(); }

    const RangeType &getInnerRange() { return innerRange; }
    const IndexType &getInnerLo() { return innerRange.getLo(); }
    const IndexType &getInnerHi() { return innerRange.getHi(); }
};

/**
 * A container holding local domains and data for each local domain
 *
 * The data is created by a factory function
 *
 * A simulation block can obtain a local context, register local references to the grids
 */
template<int rank, template<int> class CheckingPolicy = ArrayNoArgCheck>
class LocalDomainContext
{
  public:
    typedef Range<int,rank,ArrayNoArgCheck> RangeType;
    typedef Range<double,rank,ArrayNoArgCheck> DomainType;

    template<class GridType>
    class GridFactory
    {
      public:

        virtual ~GridFactory() {}
        virtual boost::shared_ptr<GridType> newGrid(RangeType range, DomainType domain, int ghostCells) = 0;
    };

    template<class GridType>
    void addGrid(std::string name, GridFactory<GridType> &factory, int ghostCells);

    template<class GridType>
    void addGrid(std::vector<std::string> names, GridFactory<GridType> &factory, int ghostCells);

    template<class GridType>
    void registerLocalReference(std::string name, GridType*& grid);

    void reset();
    bool next();

    const LocalDomain<rank, CheckingPolicy>& domain();
};

/** @brief Interface for wrapping and exchanging boundaries .
 *
 *  This interface is used to exchange the boundaries of grids
 *  between processes. Any implementation should treat the fields as periodic.
 *  The boundary conditions can be applied afterwards.
 */
template<int rank, template<int> class CheckingPolicy = ArrayNoArgCheck>
class DomainDecomposition
{
  public:
    typedef boost::shared_ptr<LocalDomainContext<rank, CheckingPolicy>> pLocalDomainContext;

    typedef Range<int, rank, CheckingPolicy> RangeType;
    typedef Range<double, rank, CheckingPolicy> DomainType;
    typedef Boundary<rank, CheckingPolicy> BoundaryType;
    typedef boost::shared_ptr<BoundaryType> pBoundaryType;

    void setGlobalRange(const RangeType &range);
    void setGlobalDomain(const DomainType &domain);

    template<class GridType>
    void setGlobalWeights(const GridType &weights);

    template<class GridType>
    void setLocalWeights(const GridType &weights);

    /**
     * Initialisation and load balancing based on global weights
     */
    void init();

    /**
     * Perform load balancing based on local weights if present, otherwise based
     * on global weights.
     *
     * Should automatically redistribute all grids added on any local domain
     * context obtained from this decomposition.
     */
    void balanceLoad();

    /**
     * multiple contexts can be created
     */
    pLocalDomainContext getLocalDomainContext();
};

}

#endif //SCHNEK_DOMAINDECOMPOSITION_HPP
