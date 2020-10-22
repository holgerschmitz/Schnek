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

#include "../util/exceptions.hpp"

#include <boost/shared_ptr.hpp>

#include <iostream>
#include <string>
#include <iterator>
#include <vector>


namespace schnek {

/**
 * A local domain for the current process
 *
 * The LocalDomain class contains information about the extent of a local grid domain.
 *
 * Each process can have multiple local domains.
 */
template<int rank, template<int> class CheckingPolicy = ArrayNoArgCheck>
class LocalDomain
{
  public:
    /**
     * The index type
     */
    typedef Array<int,rank,ArrayNoArgCheck> IndexType;

    /**
     * The range type
     */
    typedef Range<int,rank,ArrayNoArgCheck> RangeType;
  private:
    /**
     * The range of the local domain including ghost cells
     */
    RangeType range;

    /**
     * The inner range of the local domain, excluding ghost cells
     */
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
 * Iterate over all local domains
 *
 * Pointers to grids can be registered with the iterator. The iterator will then automatically
 * create grids for each local domain using the GridFactory that was passed to
 * LocalDomainContext::getGridIterator() when the iterator was created.
 *
 * Calling #next() will update all registered grid pointers to the current local grids.
 *
 * An application can contain multiple instances of LocalDomainIterator iterating over different
 * grid types and also grids that are logically different.
 */
template<int rank, class GridType, template<int> class CheckingPolicy = ArrayNoArgCheck>
class LocalDomainIterator
{
  public:

    /**
     * Reset the iterator to the first local domain.
     */
    void reset();

    /**
     * Move to the next local domain.
     *
     * This has to be called before any grids can be accessed
     *
     * Modifies all grid pointers that have been registered with the iterator
     *
     * @example
     * ```
     * iterator.reset();
     *
     * while(iterator.next()) {
     *   doSomething();
     * }
     * ```
     *
     * @return true if the iterator points to a valid local domain
     */
    bool next();

    /**
     * Perform a boundary exchange on the registered grids
     */
    void exchange();

    /**
     * Get the current local domain
     */
    const LocalDomain<rank, CheckingPolicy> &getDomain();

    /**
     * Register a pointer to a grid to be managed by the domain iterator
     */
    void registerGrid(GridType*& grid);

    /**
     * Register multiple pointers to a grid to be managed by the domain iterator
     */
    void registerGrid(std::vector<GridType*>& grids);
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
    LocalDomainIterator<rank, GridType, CheckingPolicy> getGridIterator(GridFactory<GridType> &factory, int ghostCells);
};

/** @brief Interface for wrapping and exchanging boundaries .
 *
 *  This interface is used to exchange the boundaries of grids
 *  between processes. Any implementation should treat the fields as periodic.
 *  The boundary conditions can be applied afterwards.
 *
 *  The `CheckingPolicy` template argument defines the checking policy of the indices, ranges
 *  and boundaries used by the domain composition. The checking policies for the grids and fields
 *  managed by the domain decomposition can be chosen independently.
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
    typedef Array<int, rank> LimitType;

    DomainDecomposition();

    virtual ~DomainDecomposition() {}

    /**
     * Set the global grid size of the simulation that should be decomposed
     *
     * This needs to be called before the domain decomposition can be initialised by calling
     * init().
     */
    void setGlobalRange(const RangeType &range);

    /**
     * Set the global physical size of the simulation domain
     *
     * This needs to be called before the domain decomposition can be initialised by calling
     * init().
     */
    void setGlobalDomain(const DomainType &domain);

    /**
     * Set the global simulation weights used for load balancing
     *
     * This may be called before calling init() or balanceLoad() to inform the load balancer on the
     * relative work required for each region in the simulation domain.
     *
     * The grid type of the weights may be a Grid or Field with dimensions that are scaled down from
     * the global grid dimensions by an integer factor. This coarsening factor may be different in the
     * different directions.
     *
     * If the global size is specified, it must be identical on each process
     *
     * The weights may be integers or floating point numbers.
     */
    template<class GridType>
    void setGlobalWeights(const GridType &weights);

    /**
     * Set the local simulation weights used for load balancing
     *
     * This may be called after calling init() before calling balanceLoad() to inform the load balancer on the
     * relative work required for each region in the simulation domain.
     *
     * The grid type of the weights may be a Grid or Field with dimensions that are scaled down from
     * the global grid dimensions by an integer factor. This coarsening factor may be different in the
     * different directions.
     *
     * The weights may be integers or floating point numbers.
     *
     * Setting local weights will override any global weights that might have been set.
     */
    template<class GridType>
    void setLocalWeights(const GridType &weights);

    /**
     * Initialisation and load balancing based on global weights
     *
     * This function is allowed to create multiple threads and return
     */
    virtual void init() = 0;

    /**
     * Perform load balancing based on local weights if present, otherwise based
     * on global weights.
     *
     * Should automatically redistribute all grids added on any local domain
     * context obtained from this decomposition.
     */
    virtual void balanceLoad() = 0;

    /**
     * Return a unique reproducible ID of the current process
     */
    virtual int getUniqueId() const = 0;

    /**
     * Check if the current process is the master
     */
    virtual bool master() const = 0;

    /**
     * Get the number of processes
     */
    virtual int numProcs() const = 0;

    /**
     * Get a local domain context
     *
     * multiple contexts can be created
     */
    pLocalDomainContext getLocalDomainContext();
  protected:
    typedef Grid<double, rank> InternalGridType;
    /// The global grid size
    RangeType globalRange;

    /// The global domain size
    DomainType globalDomain;

    /// The global weights
    InternalGridType globalWeights;

    /// The local weights
    InternalGridType localWeights;
    /**
     * This allows implementations to add a local range
     */
    void addLocalRange(RangeType range);
  private:
    void checkGlobalWeights();
    void checkLocalWeights();
};

template<int rank, template<int> class CheckingPolicy>
DomainDecomposition<rank, CheckingPolicy>::DomainDecomposition() :
  globalRange(LimitType(-1), LimitType(0))
{
}


template<int rank, template<int> class CheckingPolicy>
inline void DomainDecomposition<rank, CheckingPolicy>::setGlobalRange(const RangeType& range)
{
  globalRange = range;
  checkGlobalWeights();
}

template<int rank, template<int> class CheckingPolicy>
inline void schnek::DomainDecomposition<rank, CheckingPolicy>::setGlobalDomain(const DomainType& domain)
{
  globalDomain = domain;
}

template<int rank, template<int> class CheckingPolicy>
template<class GridType>
inline void schnek::DomainDecomposition<rank, CheckingPolicy>::setGlobalWeights(const GridType& weights)
{
  globalWeights = weights;
  checkGlobalWeights();
}

template<int rank, template<int> class CheckingPolicy>
template<class GridType>
inline void schnek::DomainDecomposition<rank, CheckingPolicy>::setLocalWeights(const GridType& weights)
{
  localWeights = weights;
//  checkLocalWeights();
}

template<int rank, template<int> class CheckingPolicy>
inline void DomainDecomposition<rank, CheckingPolicy>::checkGlobalWeights()
{
  LimitType globalSize = globalRange.getHi() - globalRange.getLo() + 1;
  LimitType weightsSize = globalWeights.getDims();

  if (globalSize.product() == 0 || weightsSize.product() == 0)
  {
    return;
  }

  for (int d=0; d<rank; ++d)
  {
    if (globalSize[d] % weightsSize[d] != 0)
    {
      SCHNECK_FAIL("Global weights must evenly divide the global grid size: dim = " << d);
    }
  }
}

}
#endif //SCHNEK_DOMAINDECOMPOSITION_HPP
