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

#include <algorithm>
#include <array>
#include <boost/function_types/parameter_types.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/size.hpp>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "../grid/array.hpp"
#include "../grid/arraycheck.hpp"
#include "../grid/boundary.hpp"
#include "../grid/range.hpp"
#include "../util/exceptions.hpp"
#include "detail/grid_factory.hpp"

namespace schnek {
  namespace internal {
    template<typename Callable, typename Enable = void>
    struct FunctionParameterTypesImpl;

    template<typename Callable>
    struct FunctionParameterTypesImpl<Callable, typename std::enable_if<!std::is_class<Callable>::value>::type> {
        using type = typename boost::function_types::parameter_types<Callable>::type;
    };

    template<typename Callable>
    struct FunctionParameterTypesImpl<Callable, typename std::enable_if<std::is_class<Callable>::value>::type> {
        using RawTypes = typename FunctionParameterTypesImpl<decltype(&Callable::operator())>::type;
        static_assert(boost::mpl::size<RawTypes>::value > 0, "Callable must have operator() parameters");
        using type = typename boost::mpl::pop_front<RawTypes>::type;
    };

    template<typename Callable>
    struct FunctionParameterTypes {
        using type = typename FunctionParameterTypesImpl<typename std::decay<Callable>::type>::type;
    };

    template<typename Callable>
    using FunctionParameterTypesT = typename FunctionParameterTypes<Callable>::type;

    template<typename Param>
    struct GridReferenceExtractor {
        static_assert(std::is_lvalue_reference<Param>::value, "Function parameters must be lvalue references");
        using Reference = Param;
        using ValueType = typename std::remove_reference<Reference>::type;
        using GridType = typename std::remove_const<ValueType>::type;

        static Reference extract(const pGridWrapper &wrapper) {
          auto typedWrapper = std::dynamic_pointer_cast<GridWrapperImpl<GridType>>(wrapper);
          if (!typedWrapper) {
            SCHNECK_FAIL("Grid type mismatch for registered field");
          }
          return typedWrapper->grid;
        }
    };

    template<typename ParameterSeq>
    struct GridArgumentBuilder {
        template<typename IteratorVec, std::size_t... Is>
        static auto buildImpl(const IteratorVec &iterators, std::index_sequence<Is...>) {
          return std::tuple<typename boost::mpl::at_c<ParameterSeq, Is>::type...>(
              GridReferenceExtractor<typename boost::mpl::at_c<ParameterSeq, Is>::type>::extract(*iterators[Is])...
          );
        }

        template<typename IteratorVec>
        static auto build(const IteratorVec &iterators) {
          return buildImpl(iterators, std::make_index_sequence<boost::mpl::size<ParameterSeq>::value>{});
        }
    };

    template<typename RangeType, size_t rank, typename ParameterSeq>
    struct RangeSubsetChecker {
        static bool rangeSubset(const RangeType &subset, const RangeType &superset) {
          for (size_t d = 0; d < rank; ++d) {
            if (subset.getLo()[d] < superset.getLo()[d] || subset.getHi()[d] > superset.getHi()[d]) {
              return false;
            }
          }
          return true;
        }

        template<std::size_t I, typename IteratorVec>
        static void check(const RangeType &rangeRef, const IteratorVec &iters) {
          if constexpr (I < boost::mpl::size<ParameterSeq>::value) {
            using Param = typename boost::mpl::at_c<ParameterSeq, I>::type;
            const auto &gridRef = GridReferenceExtractor<Param>::extract(*iters[I]);
            if constexpr (std::is_constructible<RangeType, decltype(gridRef.getRange())>::value) {
              SCHNEK_ASSERT(
                  rangeSubset(rangeRef, RangeType(gridRef.getRange())),
                  "First grid range must be a subset of all full-rank grid ranges in GridContext::forEach"
              );
            }
            check<I + 1>(rangeRef, iters);
          }
        }
    };
  }  // namespace internal

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
  class DomainDecomposition {
    public:
      typedef Range<ptrdiff_t, rank, CheckingPolicy> RangeType;
      typedef Range<double, rank, CheckingPolicy> DomainType;
      typedef Boundary<rank, CheckingPolicy> BoundaryType;
      typedef std::shared_ptr<BoundaryType> pBoundaryType;
      typedef Array<ptrdiff_t, rank> LimitType;
      typedef Array<size_t, rank> SizeType;
      template<size_t projRank>
      using ProjectedRegistration = ProjectedGridRegistration<projRank, rank>;
      template<size_t FullRank, typename Seq>
      struct RegistrationVariantBuilder;
      template<size_t FullRank, size_t... Indices>
      struct RegistrationVariantBuilder<FullRank, std::index_sequence<Indices...>> {
          using type = std::variant<GridRegistration, ProjectedGridRegistration<Indices + 1, FullRank>...>;
      };
      using RegistrationVariant =
          typename RegistrationVariantBuilder<rank, std::make_index_sequence<(rank > 1 ? rank - 1 : 0)>>::type;

      class GridContext {
        private:
          friend class DomainDecomposition;
          /**
           * For each registration, contains a list of pointers to the grids allocated for each range.
           * Each list has the same length.
           * If a grid is defined on a sub-range that does not intersect with the corresponding range, NULL_PTR is
           * stored.
           */
          std::vector<const std::list<internal::pGridWrapper> *> gridLists;
          std::vector<RangeType> ranges;
          GridContext(std::vector<const std::list<internal::pGridWrapper> *> gridLists, std::vector<RangeType> ranges)
              : gridLists(std::move(gridLists)), ranges(std::move(ranges)) {}

        public:
          GridContext() = delete;
          GridContext(const GridContext &) = default;

          /**
           * @brief Calls the function for each local domain.
           *
           * The arguments to the function are the local grids corresponding to the registrations.
           *
           * The `RangeType` passed to the callback is derived from the first grid argument
           * (the first entry in the registration list) when that grid exposes a compatible
           * full-rank range. Otherwise the local range is used. In debug builds, the method
           * asserts that the first full-rank grid range is a subset of every other full-rank
           * grid range.
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
       * Get the global grid size of the simulation
       *
       * Calling this before calling `setGlobalRange()` results in undefined behaviour.
       */
      const RangeType &getGlobalRange() { return globalRange; }

      /**
       * Set the global physical size of the simulation domain
       *
       * This needs to be called before the domain decomposition can be initialised by calling
       * init().
       */
      void setGlobalDomain(const DomainType &domain);

      /**
       * Get the global physical size of the simulation domain
       *
       * Calling this before calling `setGlobalDomain()` results in undefined behaviour.
       */
      const DomainType &getGlobalDomain() { return globalDomain; }

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

      /*
       * Accumulate halo cells with neighbouring data.
       */
      virtual void accumulate(const internal::pGridWrapper &wrapper, bool useFieldInfo = true) = 0;
      void accumulate(GridRegistration registration, bool useFieldInfo = true);
      void accumulate(std::initializer_list<GridRegistration> registrations, bool useFieldInfo = true);

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

      /// Return the average of a single value over all the processes
      virtual double avgReduce(double) const = 0;

      /// Return the average of a single value over all the processes
      virtual int avgReduce(int) const = 0;

      /// Return the average of a single value over all the processes
      virtual long avgReduce(long) const = 0;

      /// Return the sum of a single value over all the processes
      virtual double sumReduce(double) const = 0;

      /// Return the sum of a single value over all the processes
      virtual int sumReduce(int) const = 0;

      /// Return the sum of a single value over all the processes
      virtual long sumReduce(long) const = 0;

      /// Return the maximum of a single value over all the processes
      virtual double maxReduce(double) const = 0;

      /// Return the maximum of a single value over all the processes
      virtual int maxReduce(int) const = 0;

      /// Return the maximum of a single value over all the processes
      virtual long maxReduce(long) const = 0;

      /// Return the minimum of a single value over all the processes
      virtual double minReduce(double) const = 0;

      /// Return the minimum of a single value over all the processes
      virtual int minReduce(int) const = 0;

      /// Return the minimum of a single value over all the processes
      virtual long minReduce(long) const = 0;

      /**
       * Get a grid context for calling a function over all local domains
       *
       * multiple contexts can be created
       */
      GridContext getGridContext(std::initializer_list<RegistrationVariant> registrations);

      /**
       * Exchange halo cells between processes by visiting a single grid wrapper.
       *
       * The extent of the halo is determined by the the `useFieldInfo` flag. If false, the halo is determined
       * by the size of the grid in relation to the local index range. If true (default), for `Field`-type grids,
       * it is taken from `ghostCells` parameter of the field. For plain `Grid`-type grids, the flag has no effect.
       */
      virtual void exchangeGrid(const internal::pGridWrapper &wrapper, bool useFieldInfo = true) = 0;

      /**
       * Exchange halo cells between processes using a single registration.
       *
       * The extent of the halo is determined by the the `useFieldInfo` flag. If false, the halo is determined
       * by the size of the grid in relation to the local index range. If true (default), for `Field`-type grids,
       * it is taken from `ghostCells` parameter of the field. For plain `Grid`-type grids, the flag has no effect.
       */
      void exchange(GridRegistration registration, bool useFieldInfo = true);

      /**
       * Exchange halo cells between processes for multiple registrations.
       *
       * The extent of the halo is determined by the the `useFieldInfo` flag. If false, the halo is determined
       * by the size of the grid in relation to the local index range. If true (default), for `Field`-type grids,
       * it is taken from `ghostCells` parameter of the field. For plain `Grid`-type grids, the flag has no effect.
       */
      void exchange(std::initializer_list<GridRegistration> registrations, bool useFieldInfo = true);

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

      /**
       * Register a grid or field by passing a factory.
       *
       * The domain decomposition will create instances of the grid for each local domain.
       *
       * A grid registration is passed back for future reference to the field. The registrations
       * are typically used in conjunction with the `getGridContext` method.
       */
      template<class GridType>
      GridRegistration registerFieldImpl(const GridFactory<GridType> &factory);

      /**
       * Register a grid or field by passing a factory, but only allocate for a sub-range.
       *
       * Only local ranges that intersect the sub-range will allocate a grid. For all other
       * local ranges, a null pointer is stored in the internal grid list.
       */
      template<class GridType>
      GridRegistration registerFieldImpl(const GridFactory<GridType> &factory, const RangeType &subRange);

      /**
       * Register a grid or field projection by passing a factory and axes.
       *
       * The projection rank is given by `GridType::Rank` and must satisfy 1 <= Rank < `rank`.
       * The axes array defines which dimensions of the full domain are projected.
       */
      template<class GridType>
      ProjectedRegistration<GridType::Rank> registerFieldProjectionImpl(
          const GridFactory<GridType> &factory, const std::array<size_t, GridType::Rank> &axes
      );

    private:
      struct LocalRangeInfo {
          RangeType range;
          DomainType domain;
      };

      using pGridRegistrationInterface = internal::pGridRegistrationInterface<rank, CheckingPolicy>;
      std::map<long, pGridRegistrationInterface> registeredFields;

      /**
       * @brief For each grid registration ID, this stores the local
       * list of grids for each allocation range
       */
      std::map<long, std::list<internal::pGridWrapper>> grids;

      /**
       * @brief For each projected registration ID, this stores the local
       * list of projected grids for each allocation range
       */
      std::map<long, std::list<internal::pGridWrapper>> projectedGrids;

      struct ProjectedRegistrationInterface : public Unique<ProjectedRegistrationInterface> {
          virtual ~ProjectedRegistrationInterface() = default;
          virtual internal::pGridWrapper makeGrid(const RangeType &fullRange, const DomainType &fullDomain) = 0;
          virtual internal::pGridWrapper ensureSharedGrid(
              const RangeType &fullRange, const DomainType &fullDomain, std::list<internal::pGridWrapper> &gridList
          ) = 0;
      };

      template<class GridType, size_t projRank>
      struct ProjectedRegistrationImpl : public ProjectedRegistrationInterface {
          using ProjectedRangeType = Range<ptrdiff_t, projRank, CheckingPolicy>;
          using ProjectedDomainType = Range<double, projRank, CheckingPolicy>;

          ProjectedRegistrationImpl(const GridFactory<GridType> &factoryIn, const std::array<size_t, projRank> &axesIn)
              : factory(factoryIn), axes(axesIn), hasUnion(false) {}

          internal::pGridWrapper makeGrid(const RangeType &fullRange, const DomainType &fullDomain) override {
            typename ProjectedRangeType::LimitType lo;
            typename ProjectedRangeType::LimitType hi;
            typename ProjectedDomainType::LimitType domainLo;
            typename ProjectedDomainType::LimitType domainHi;

            for (size_t d = 0; d < projRank; ++d) {
              const size_t axis = axes[d];
              lo[d] = fullRange.getLo()[axis];
              hi[d] = fullRange.getHi()[axis];
              domainLo[d] = fullDomain.getLo()[axis];
              domainHi[d] = fullDomain.getHi()[axis];
            }

            ProjectedRangeType projectedRange(lo, hi);
            ProjectedDomainType projectedDomain(domainLo, domainHi);
            return factory.newGrid(projectedRange, projectedDomain);
          }

          internal::pGridWrapper ensureSharedGrid(
              const RangeType &fullRange, const DomainType &fullDomain, std::list<internal::pGridWrapper> &gridList
          ) override {
            bool expanded = updateUnion(fullRange, fullDomain);
            if (expanded || !sharedGrid) {
              sharedGrid = makeGrid(unionRange, unionDomain);
              for (auto &entry : gridList) {
                entry = sharedGrid;
              }
            }
            return sharedGrid;
          }

          bool updateUnion(const RangeType &fullRange, const DomainType &fullDomain) {
            if (!hasUnion) {
              unionRange = fullRange;
              unionDomain = fullDomain;
              hasUnion = true;
              return true;
            }

            bool changed = false;
            for (size_t d = 0; d < projRank; ++d) {
              const size_t axis = axes[d];
              if (fullRange.getLo()[axis] < unionRange.getLo()[axis]) {
                unionRange.getLo()[axis] = fullRange.getLo()[axis];
                changed = true;
              }
              if (fullRange.getHi()[axis] > unionRange.getHi()[axis]) {
                unionRange.getHi()[axis] = fullRange.getHi()[axis];
                changed = true;
              }
              if (fullDomain.getLo()[axis] < unionDomain.getLo()[axis]) {
                unionDomain.getLo()[axis] = fullDomain.getLo()[axis];
                changed = true;
              }
              if (fullDomain.getHi()[axis] > unionDomain.getHi()[axis]) {
                unionDomain.getHi()[axis] = fullDomain.getHi()[axis];
                changed = true;
              }
            }

            return changed;
          }

          GridFactory<GridType> factory;
          std::array<size_t, projRank> axes;
          bool hasUnion;
          RangeType unionRange;
          DomainType unionDomain;
          internal::pGridWrapper sharedGrid;
      };

      std::map<long, std::shared_ptr<ProjectedRegistrationInterface>> projectedRegisteredFields;

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
  DomainDecomposition<rank, CheckingPolicy>::DomainDecomposition() : globalRange(LimitType(-1), LimitType(0)) {}

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename Func>
  void DomainDecomposition<rank, CheckingPolicy>::GridContext::forEach(Func func) {
    using AllParameters = internal::FunctionParameterTypesT<Func>;
    constexpr std::size_t totalParams = boost::mpl::size<AllParameters>::value;
    static_assert(totalParams >= 1, "Function must take at least a RangeType argument");

    using RangeParam = typename boost::mpl::front<AllParameters>::type;
    using RangeDecay = typename std::remove_reference<RangeParam>::type;
    using RangeBase = typename std::remove_const<RangeDecay>::type;
    static_assert(
        std::is_same<RangeBase, RangeType>::value,
        "First function argument must be DomainDecomposition::RangeType (by reference)"
    );

    using ParameterSeq = typename boost::mpl::pop_front<AllParameters>::type;
    constexpr std::size_t paramCount = boost::mpl::size<ParameterSeq>::value;

    if (gridLists.size() != paramCount) {
      SCHNECK_FAIL(
          "Grid registration count (" << gridLists.size() << ") does not match function arity (" << paramCount << ")"
      );
    }

    const auto &selectedLists = gridLists;

    // checking that all gridLists have the same number of entries that are equal to the number of ranges
    std::size_t entryCount = ranges.size();
    if (!selectedLists.empty()) {
      entryCount = selectedLists.front()->size();
      for (std::size_t i = 1; i < selectedLists.size(); ++i) {
        if (selectedLists[i]->size() != entryCount) {
          SCHNECK_FAIL("Grid list size mismatch for registration index " << i);
        }
      }
      if (ranges.size() != entryCount) {
        SCHNECK_FAIL("Range list size mismatch with registered grids");
      }
    }

    // iterators contains an iterator for each grid registration
    // Each iterator iterates over the range allocations
    using ListIterator = typename std::list<internal::pGridWrapper>::const_iterator;
    std::vector<ListIterator> iterators;
    iterators.reserve(selectedLists.size());
    for (auto listPtr : selectedLists) {
      iterators.push_back(listPtr->begin());
    }

    for (std::size_t entry = 0; entry < entryCount; ++entry) {
      bool hasNullGrid = false;
      for (auto &it : iterators) {
        if (!(*it)) {
          hasNullGrid = true;
          break;
        }
      }

      // If any of the grids are null, we don't do anything
      if (!hasNullGrid) {
        RangeType rangeRef = ranges[entry];
        bool hasFullRange = false;
        if constexpr (paramCount > 0) {
          using FirstParam = typename boost::mpl::at_c<ParameterSeq, 0>::type;
          const auto &firstGrid = internal::GridReferenceExtractor<FirstParam>::extract(*iterators.front());
          if constexpr (std::is_constructible<
            RangeType, 
            decltype(internal::RangeGetter<std::decay_t<decltype(firstGrid)>>::get(firstGrid))
          >::value) {
            rangeRef = RangeType(internal::RangeGetter<std::decay_t<decltype(firstGrid)>>::get(firstGrid));
            hasFullRange = true;
          }
#ifndef NDEBUG
          if (hasFullRange) {
            internal::RangeSubsetChecker<RangeType, rank, ParameterSeq>::template check<0>(rangeRef, iterators);
          }
#endif
        }
        auto args = internal::GridArgumentBuilder<ParameterSeq>::build(iterators);
        std::apply([&](auto &&...gridArgs) { func(rangeRef, std::forward<decltype(gridArgs)>(gridArgs)...); }, args);
      }

      for (auto &it : iterators) {
        ++it;
      }
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  inline void DomainDecomposition<rank, CheckingPolicy>::setGlobalRange(const RangeType &range) {
    globalRange = range;
    checkGlobalWeights();
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  inline void schnek::DomainDecomposition<rank, CheckingPolicy>::setGlobalDomain(const DomainType &domain) {
    globalDomain = domain;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<class GridType>
  inline void schnek::DomainDecomposition<rank, CheckingPolicy>::setGlobalWeights(const GridType &weights) {
    globalWeights = weights;
    checkGlobalWeights();
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<class GridType>
  inline void schnek::DomainDecomposition<rank, CheckingPolicy>::setLocalWeights(const GridType &weights) {
    localWeights = weights;
    checkLocalWeights();
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<class GridType>
  inline GridRegistration schnek::DomainDecomposition<rank, CheckingPolicy>::registerFieldImpl(
      const GridFactory<GridType> &factory
  ) {
    using Registration = internal::GridRegistrationImpl<rank, CheckingPolicy, GridType>;
    auto registration = std::make_shared<Registration>(factory);
    long id = registration->getId();
    registeredFields[id] = registration;

    std::list<internal::pGridWrapper> gridList;
    for (const auto &localRange : ranges) {
      gridList.push_back(registration->makeGrid(localRange.range, localRange.domain));
    }

    grids[id] = std::move(gridList);

    return GridRegistration{id};
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<class GridType>
  inline GridRegistration schnek::DomainDecomposition<rank, CheckingPolicy>::registerFieldImpl(
      const GridFactory<GridType> &factory, const RangeType &subRange
  ) {
    using Registration = internal::GridRegistrationImpl<rank, CheckingPolicy, GridType>;
    auto registration = std::make_shared<Registration>(factory);
    long id = registration->getId();
    registeredFields[id] = registration;

    std::list<internal::pGridWrapper> gridList;
    for (const auto &localRange : ranges) {
      RangeType intersection;
      bool overlaps = true;
      for (size_t d = 0; d < rank; ++d) {
        const auto lo = std::max(localRange.range.getLo()[d], subRange.getLo()[d]);
        const auto hi = std::min(localRange.range.getHi()[d], subRange.getHi()[d]);
        if (lo > hi) {
          overlaps = false;
          break;
        }
        intersection.getLo()[d] = lo;
        intersection.getHi()[d] = hi;
      }

      if (!overlaps) {
        gridList.push_back(nullptr);
        continue;
      }

      DomainType subDomain(localRange.domain);
      for (size_t d = 0; d < rank; ++d) {
        const ptrdiff_t localCells = localRange.range.getHi()[d] - localRange.range.getLo()[d] + 1;
        const ptrdiff_t subCells = intersection.getHi()[d] - intersection.getLo()[d] + 1;

        if (localCells <= 0) {
          continue;
        }

        const double cellSize =
            (localRange.domain.getHi()[d] - localRange.domain.getLo()[d]) / static_cast<double>(localCells);
        const ptrdiff_t offsetLo = intersection.getLo()[d] - localRange.range.getLo()[d];

        subDomain.getLo()[d] = localRange.domain.getLo()[d] + cellSize * static_cast<double>(offsetLo);
        subDomain.getHi()[d] = subDomain.getLo()[d] + cellSize * static_cast<double>(subCells);
      }

      gridList.push_back(registration->makeGrid(intersection, subDomain));
    }

    grids[id] = std::move(gridList);

    return GridRegistration{id};
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<class GridType>
  inline typename DomainDecomposition<rank, CheckingPolicy>::template ProjectedRegistration<GridType::Rank>
  DomainDecomposition<rank, CheckingPolicy>::registerFieldProjectionImpl(
      const GridFactory<GridType> &factory, const std::array<size_t, GridType::Rank> &axes
  ) {
    constexpr size_t projRank = GridType::Rank;
    static_assert(projRank >= 1, "Projected grid rank must be at least 1");
    static_assert(projRank < rank, "Projected grid rank must be smaller than domain decomposition rank");

    for (size_t i = 0; i < projRank; ++i) {
      if (axes[i] >= rank) {
        SCHNECK_FAIL("Projection axis index out of bounds: " << axes[i]);
      }
      for (size_t j = i + 1; j < projRank; ++j) {
        if (axes[i] == axes[j]) {
          SCHNECK_FAIL("Duplicate projection axis index: " << axes[i]);
        }
      }
    }

    using Registration = ProjectedRegistrationImpl<GridType, projRank>;
    auto registration = std::make_shared<Registration>(factory, axes);
    long id = registration->getId();
    projectedRegisteredFields[id] = registration;

    std::list<internal::pGridWrapper> gridList;
    for (const auto &localRange : ranges) {
      gridList.push_back(registration->ensureSharedGrid(localRange.range, localRange.domain, gridList));
    }

    projectedGrids[id] = std::move(gridList);

    ProjectedRegistration<projRank> result{};
    result.id = id;
    result.axes = axes;
    return result;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void DomainDecomposition<rank, CheckingPolicy>::exchange(GridRegistration registration, bool useFieldInfo) {
    auto gridIt = grids.find(registration.id);
    if (gridIt == grids.end()) {
      SCHNECK_FAIL("Unknown grid registration id: " << registration.id);
    }

    for (const auto &wrapper : gridIt->second) {
      if (!wrapper) {
        continue;
      }
      this->exchangeGrid(wrapper, useFieldInfo);
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void DomainDecomposition<rank, CheckingPolicy>::exchange(
      std::initializer_list<GridRegistration> registrations, bool useFieldInfo
  ) {
    for (const auto &registration : registrations) {
      exchange(registration, useFieldInfo);
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void DomainDecomposition<rank, CheckingPolicy>::accumulate(GridRegistration registration, bool useFieldInfo) {
    auto gridIt = grids.find(registration.id);
    if (gridIt == grids.end()) {
      SCHNECK_FAIL("Unknown grid registration id: " << registration.id);
    }

    for (const auto &wrapper : gridIt->second) {
      if (!wrapper) {
        continue;
      }
      this->accumulate(wrapper, useFieldInfo);
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void DomainDecomposition<rank, CheckingPolicy>::accumulate(
      std::initializer_list<GridRegistration> registrations, bool useFieldInfo
  ) {
    for (const auto &registration : registrations) {
      accumulate(registration, useFieldInfo);
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  typename DomainDecomposition<rank, CheckingPolicy>::GridContext
  DomainDecomposition<rank, CheckingPolicy>::getGridContext(std::initializer_list<RegistrationVariant> registrations) {
    std::vector<const std::list<internal::pGridWrapper> *> selectedLists;
    selectedLists.reserve(registrations.size());

    for (const auto &registration : registrations) {
      std::visit(
          [&](const auto &typedReg) {
            using RegType = std::decay_t<decltype(typedReg)>;
            const std::map<long, std::list<internal::pGridWrapper>> *targetMap = nullptr;
            if constexpr (std::is_same<RegType, GridRegistration>::value) {
              targetMap = &grids;
            } else {
              targetMap = &projectedGrids;
            }

            auto gridIt = targetMap->find(typedReg.id);
            if (gridIt == targetMap->end()) {
              SCHNECK_FAIL("Unknown grid registration id: " << typedReg.id);
            }
            selectedLists.push_back(&gridIt->second);
          },
          registration
      );
    }

    std::vector<RangeType> rangeCopies;
    rangeCopies.reserve(ranges.size());
    for (const auto &localRange : ranges) {
      rangeCopies.push_back(localRange.range);
    }
    return GridContext{std::move(selectedLists), std::move(rangeCopies)};
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void DomainDecomposition<rank, CheckingPolicy>::addLocalRange(RangeType range, DomainType domain) {
    ranges.push_back(LocalRangeInfo{range, domain});
    for (auto &reg : registeredFields) {
      long id = reg.first;
      grids[id].push_back(reg.second->makeGrid(range, domain));
    }
    for (auto &reg : projectedRegisteredFields) {
      long id = reg.first;
      auto &gridList = projectedGrids[id];
      gridList.push_back(reg.second->ensureSharedGrid(range, domain, gridList));
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void DomainDecomposition<rank, CheckingPolicy>::addLocalIterationRange(RangeType range) {
    iterationRanges.push_back(range);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  inline void DomainDecomposition<rank, CheckingPolicy>::checkGlobalWeights() {
    LimitType globalSize = globalRange.getHi() - globalRange.getLo() + 1;
    SizeType weightsSize = globalWeights.getDims();

    if (globalSize.product() == 0 || weightsSize.product() == 0) {
      return;
    }

    for (size_t d = 0; d < rank; ++d) {
      if (globalSize[d] % weightsSize[d] != 0) {
        SCHNECK_FAIL("Global weights must evenly divide the global grid size: dim = " << d);
      }
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  inline void DomainDecomposition<rank, CheckingPolicy>::checkLocalWeights() {}

}  // namespace schnek
#endif  // SCHNEK_DOMAINDECOMPOSITION_HPP
