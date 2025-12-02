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


/**
 * @file domaindecomposition.hpp
 * @brief Interface for wrapping and exchanging boundaries
 *
 * This interface is used to exchange the boundaries of distribution
 * functions and scalar fields. It can be implemented to define
 * periodic boundaries or exchange data with other processes.
 */

#ifndef SCHNEK_DOMAINDECOMPOSITION_HPP
#define SCHNEK_DOMAINDECOMPOSITION_HPP

#include "../grid/array.hpp"
#include "../grid/arraycheck.hpp"
#include "../grid/boundary.hpp"
#include "../grid/range.hpp"

#include "../util/exceptions.hpp"

#include "detail/grid_factory.hpp"

#include <iostream>
#include <string>
#include <iterator>
#include <vector>
#include <memory>
#include <list>
#include <map>
#include <tuple>
#include <type_traits>
#include <utility>
#include <algorithm>

#include <boost/function_types/parameter_types.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/pop_front.hpp>

namespace schnek {

/**
 * A local domain for the current process
 *
 * The LocalDomain class contains information about the extent of a local grid domain.
 *
 * Each process can have multiple local domains.
 */
template<size_t rank, template<size_t> class CheckingPolicy = ArrayNoArgCheck>
class LocalDomain
{
  public:
    /**
     * The index type
     */
    typedef Array<ptrdiff_t,rank,ArrayNoArgCheck> IndexType;

    /**
     * The range type
     */
    typedef Range<ptrdiff_t,rank,ArrayNoArgCheck> RangeType;
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


namespace internal {
    template<typename Callable, typename Enable = void>
    struct FunctionParameterTypesImpl;

    template<typename Callable>
    struct FunctionParameterTypesImpl<Callable, typename std::enable_if<!std::is_class<Callable>::value>::type>
    {
    using type = typename boost::function_types::parameter_types<Callable>::type;
    };

    template<typename Callable>
    struct FunctionParameterTypesImpl<Callable, typename std::enable_if<std::is_class<Callable>::value>::type>
    {
        using RawTypes = typename FunctionParameterTypesImpl<decltype(&Callable::operator())>::type;
        static_assert(boost::mpl::size<RawTypes>::value > 0, "Callable must have operator() parameters");
        using type = typename boost::mpl::pop_front<RawTypes>::type;
    };

    template<typename Callable>
    struct FunctionParameterTypes
    {
    using type = typename FunctionParameterTypesImpl<typename std::decay<Callable>::type>::type;
    };

    template<typename Callable>
    using FunctionParameterTypesT = typename FunctionParameterTypes<Callable>::type;

    template<typename Param>
    struct GridReferenceExtractor
    {
        static_assert(std::is_lvalue_reference<Param>::value, "Function parameters must be lvalue references");
        using Reference = Param;
        using ValueType = typename std::remove_reference<Reference>::type;
        using GridType = typename std::remove_const<ValueType>::type;

        static Reference extract(const pGridWrapper &wrapper)
        {
            auto typedWrapper = std::dynamic_pointer_cast<GridWrapperImpl<GridType>>(wrapper);
            if (!typedWrapper)
            {
                SCHNECK_FAIL("Grid type mismatch for registered field");
            }
            return typedWrapper->grid;
        }
    };

    template<typename ParameterSeq>
    struct GridArgumentBuilder
    {
    template<typename IteratorVec, std::size_t... Is>
    static auto buildImpl(const IteratorVec &iterators, std::index_sequence<Is...>)
    {
        return std::tuple<typename boost::mpl::at_c<ParameterSeq, Is>::type...>(
        GridReferenceExtractor<typename boost::mpl::at_c<ParameterSeq, Is>::type>::extract(*iterators[Is])...
        );
    }

    template<typename IteratorVec>
    static auto build(const IteratorVec &iterators)
    {
        return buildImpl(
        iterators,
        std::make_index_sequence<boost::mpl::size<ParameterSeq>::value>{}
        );
    }
    };
} // namespace internal

/** 
 * @brief Interface for wrapping and exchanging boundaries .
 *
 * This interface is used to exchange the boundaries of grids
 * between processes. Any implementation should treat the fields as periodic.
 * The boundary conditions can be applied afterwards.
 *
 * The `CheckingPolicy` template argument defines the checking policy of the indices, ranges
 * and boundaries used by the domain composition. The checking policies for the grids and fields
 * managed by the domain decomposition can be chosen independently.
 */
template<size_t rank, template<size_t> class CheckingPolicy = ArrayNoArgCheck>
class DomainDecomposition
{
  public:
    typedef std::shared_ptr<LocalDomainContext<rank, CheckingPolicy>> pLocalDomainContext;

    typedef Range<ptrdiff_t, rank, CheckingPolicy> RangeType;
    typedef Range<double, rank, CheckingPolicy> DomainType;
    typedef Boundary<rank, CheckingPolicy> BoundaryType;
    typedef boost::shared_ptr<BoundaryType> pBoundaryType;
    typedef Array<ptrdiff_t, rank> LimitType;

    class GridContext {
      private:
        friend class DomainDecomposition;
        std::vector<long> ids;
        const std::map<long, std::list<internal::pGridWrapper>> &grids;
        std::vector<RangeType> ranges;
        GridContext(
          std::vector<long> ids,
          const std::map<long, std::list<internal::pGridWrapper>> &grids,
          std::vector<RangeType> ranges
        ): ids(std::move(ids)), grids(grids), ranges(std::move(ranges)) {}
      public:
        GridContext() = delete;
        GridContext(const GridContext &) = default;


        /**
         * @brief Calls the function for each local domain. 
         * 
         * The arguments to the function are the local grids corresponding to the registrations.
         * 
         * @tparam Func the function type
         * @param func a function taking the grids corrsponding to the registrations
         */
        template<typename Func>
        void forEach(Func func);
    };

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
     * Register a grid or field by passing a factory.
     * 
     * The domain decomposition will create instances of the grid for each local domain.
     * 
     * A grid registration is passed back for future reference to the field. The registrations
     * are typically used in conjunction with the `getGridContext` method.
     */
    template<class GridType>
    GridRegistration registerField(GridFactory<GridType> &factory);

    /**
     * Get a grid context for calling a function over all local domains
     *
     * multiple contexts can be created
     */
    GridContext getGridContext(std::initializer_list<GridRegistration> registrations);
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
     * This allows implementations to add a local range for grid allocation
     */
    void addLocalRange(RangeType range, DomainType domain);

    /**
     * This allows implementations to add a local range for iteration
     */
    void addLocalIterationRange(RangeType range);
  private:
    struct LocalRangeInfo {
      RangeType range;
      DomainType domain;
    };
    
    std::map<long, internal::pGridRegistrationInterface> registeredFields;

    /**
     * @brief For each grid registration ID, this stores the local
     * list of grids for each allocation range
     */
    std::map<long, std::list<internal::pGridWrapper>> grids;

    /**
     * @brief Contains the local ranges for the grids.
     */
    std::list<LocalRangeInfo> ranges;

    /**
     * @brief Contains the local iteration ranges.
     */
    std::list<RangeType> iterationRanges;

    void checkGlobalWeights();

    void checkLocalWeights();
};

template<size_t rank, template<size_t> class CheckingPolicy>
DomainDecomposition<rank, CheckingPolicy>::DomainDecomposition() :
  globalRange(LimitType(-1), LimitType(0))
{
}


template<size_t rank, template<size_t> class CheckingPolicy>
template<typename Func>
void DomainDecomposition<rank, CheckingPolicy>::GridContext::forEach(Func func)
{
  using AllParameters = internal::FunctionParameterTypesT<Func>;
  constexpr std::size_t totalParams = boost::mpl::size<AllParameters>::value;
  static_assert(totalParams >= 1, "Function must take at least a RangeType argument");

  using RangeParam = typename boost::mpl::front<AllParameters>::type;
  using RangeDecay = typename std::remove_reference<RangeParam>::type;
  using RangeBase = typename std::remove_const<RangeDecay>::type;
  static_assert(std::is_same<RangeBase, RangeType>::value,
    "First function argument must be DomainDecomposition::RangeType (by reference)");

  using ParameterSeq = typename boost::mpl::pop_front<AllParameters>::type;
  constexpr std::size_t paramCount = boost::mpl::size<ParameterSeq>::value;

  if (ids.size() != paramCount)
  {
    SCHNECK_FAIL("Grid registration count (" << ids.size() << ") does not match function arity (" << paramCount << ")");
  }

  std::vector<const std::list<internal::pGridWrapper>*> selectedLists;
  selectedLists.reserve(ids.size());
  for (auto id : ids)
  {
    auto gridIt = grids.find(id);
    if (gridIt == grids.end())
    {
      SCHNECK_FAIL("Unknown grid registration id: " << id);
    }
    selectedLists.push_back(&gridIt->second);
  }

  std::size_t entryCount = ranges.size();
  if (!selectedLists.empty())
  {
    entryCount = selectedLists.front()->size();
    for (std::size_t i = 1; i < selectedLists.size(); ++i)
    {
      if (selectedLists[i]->size() != entryCount)
      {
        SCHNECK_FAIL("Grid list size mismatch for registration index " << i);
      }
    }
    if (ranges.size() != entryCount)
    {
      SCHNECK_FAIL("Range list size mismatch with registered grids");
    }
  }

  using ListIterator = typename std::list<internal::pGridWrapper>::const_iterator;
  std::vector<ListIterator> iterators;
  iterators.reserve(selectedLists.size());
  for (auto listPtr : selectedLists)
  {
    iterators.push_back(listPtr->begin());
  }

  for (std::size_t entry = 0; entry < entryCount; ++entry)
  {
    const RangeType &rangeRef = ranges[entry];
    auto args = internal::GridArgumentBuilder<ParameterSeq>::build(iterators);
    std::apply(
      [&](auto&&... gridArgs)
      {
        func(rangeRef, std::forward<decltype(gridArgs)>(gridArgs)...);
      },
      args
    );
    for (auto &it : iterators)
    {
      ++it;
    }
  }
}


template<size_t rank, template<size_t> class CheckingPolicy>
inline void DomainDecomposition<rank, CheckingPolicy>::setGlobalRange(const RangeType& range)
{
  globalRange = range;
  checkGlobalWeights();
}

template<size_t rank, template<size_t> class CheckingPolicy>
inline void schnek::DomainDecomposition<rank, CheckingPolicy>::setGlobalDomain(const DomainType& domain)
{
  globalDomain = domain;
}

template<size_t rank, template<size_t> class CheckingPolicy>
template<class GridType>
inline void schnek::DomainDecomposition<rank, CheckingPolicy>::setGlobalWeights(const GridType& weights)
{
  globalWeights = weights;
  checkGlobalWeights();
}

template<size_t rank, template<size_t> class CheckingPolicy>
template<class GridType>
inline void schnek::DomainDecomposition<rank, CheckingPolicy>::setLocalWeights(const GridType& weights)
{
  localWeights = weights;
  checkLocalWeights();
}

template<size_t rank, template<size_t> class CheckingPolicy>
template<class GridType>
inline void schnek::DomainDecomposition<rank, CheckingPolicy>::registerField(GridFactory<GridType>& factory)
{
  auto registration = std::make_shared<internal::GridRegistrationImpl<GridType>>(factory);
  long id = registration->getId()
  registeredFields[id] = registration;

  // create the grids for this registration
  std::list<internal::GridWrapper> gridList;
  for (LocalRangeInfo localRange: ranges) {
    gridList.push_back(registration.makeGrid(localRange.range, localRange.domain));
  }

  grids[id] = gridList;

  return GridRegistration{id};
}

template<size_t rank, template<size_t> class CheckingPolicy>
GridContext DomainDecomposition<rank, CheckingPolicy>::getGridContext(std::initializer_list<GridRegistration> registrations) {
    std::vector<long> ids;
    ids.reserve(registrations.size());
    std::transform(registrations.begin(), registrations.end(), std::back_inserter(ids),
                                    [](const GridRegistration& r) { return r.id; });
  std::vector<RangeType> rangeCopies;
  rangeCopies.reserve(ranges.size());
  for (const auto &localRange : ranges) {
    rangeCopies.push_back(localRange.range);
  }
  return GridContext{std::move(ids), grids, std::move(rangeCopies)};
}

template<size_t rank, template<size_t> class CheckingPolicy>
void DomainDecomposition<rank, CheckingPolicy>::addLocalRange(RangeType range, DomainType domain) {
    ranges.push_back(LocalRangeInfo{range, domain});
    for (auto& reg : registeredFields) {
        long id = reg.first;
        grids[id].push_back(reg.second->makeGrid(range, domain));
    }
}

template<size_t rank, template<size_t> class CheckingPolicy>
void DomainDecomposition<rank, CheckingPolicy>::addLocalIterationRange(RangeType range) {
  iterationRanges.push_back(range);
}

template<size_t rank, template<size_t> class CheckingPolicy>
inline void DomainDecomposition<rank, CheckingPolicy>::checkGlobalWeights()
{
  LimitType globalSize = globalRange.getHi() - globalRange.getLo() + 1;
  LimitType weightsSize = globalWeights.getDims();

  if (globalSize.product() == 0 || weightsSize.product() == 0)
  {
    return;
  }

  for (size_t d=0; d<rank; ++d)
  {
    if (globalSize[d] % weightsSize[d] != 0)
    {
      SCHNECK_FAIL("Global weights must evenly divide the global grid size: dim = " << d);
    }
  }
}

template<size_t rank, template<size_t> class CheckingPolicy>
inline void DomainDecomposition<rank, CheckingPolicy>::checkLocalWeights() {}

}
#endif //SCHNEK_DOMAINDECOMPOSITION_HPP
