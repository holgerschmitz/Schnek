/*
 * serial_decomposition.hpp
 *
 *  Created on: 23 Jun 2026
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 */

#ifndef SCHNEK_DECOMPOSITION_SERIAL_DECOMPOSITION_HPP_
#define SCHNEK_DECOMPOSITION_SERIAL_DECOMPOSITION_HPP_

#include "config.hpp"
#include "detail/grid_visitor.hpp"
#include "domaindecomposition.hpp"

#include <functional>
#include <map>
#include <vector>
#include <optional>

namespace schnek {

  /**
   * Serial domain decomposition backend.
   */
  template<size_t rank, template<size_t> class CheckingPolicy = ArrayNoArgCheck>
  class SerialDomainDecomposition : public DomainDecomposition<rank, CheckingPolicy> {
    public:
      typedef typename DomainDecomposition<rank, CheckingPolicy>::LimitType LimitType;
      typedef typename DomainDecomposition<rank, CheckingPolicy>::RangeType RangeType;
      typedef typename DomainDecomposition<rank, CheckingPolicy>::DomainType DomainType;

      template<size_t projRank>
      using ProjectedRegistration = ProjectedGridRegistration<projRank, rank>;
      
      /**
       * Constructor creating the domain decomposition object
       */
      SerialDomainDecomposition() {};

      ~SerialDomainDecomposition() {};

      SerialDomainDecomposition(const SerialDomainDecomposition &) = delete;
      SerialDomainDecomposition(SerialDomainDecomposition &&) = delete;
      SerialDomainDecomposition &operator=(const SerialDomainDecomposition &) = delete;
      SerialDomainDecomposition &operator=(SerialDomainDecomposition &&) = delete;

      /**
       * Initialisation and load balancing based on global weights
       *
       * The processor layout is determined by the global grid size or, if specified, by the
       * global weights.
       */
      void init() override;

      /**
       * Perform load balancing based on local weights if present, otherwise based
       * on global weights.
       *
       * Should automatically redistribute all grids added on any local domain
       * context obtained from this decomposition.
       */
      void balanceLoad() override;

      /**
       * Return a unique reproducible ID of the current process
       *
       * @return the unique ID
       */
      int getUniqueId() const override;

      /**
       * Check if the current process is the master
       *
       * @return `true` if this is the master process
       */
      bool master() const override;

      /**
       * Get the number of processes
       *
       * @return the number of processes is 1 for serial decomposition
       */
      int numProcs() const override;

      double avgReduce(double value) const override;
      int avgReduce(int value) const override;
      long avgReduce(long value) const override;

      double sumReduce(double value) const override;
      int sumReduce(int value) const override;
      long sumReduce(long value) const override;

      double maxReduce(double value) const override;
      int maxReduce(int value) const override;
      long maxReduce(long value) const override;

      double minReduce(double value) const override;
      int minReduce(int value) const override;
      long minReduce(long value) const override;

      /**
       * Register a grid or field by passing a factory.
       *
       * The domain decomposition will create instances of the grid for each local domain.
       *
       * A grid registration is passed back for future reference to the field. The registrations
       * are typically used in conjunction with the `getGridContext` method.
       */
      template<class GridType>
      GridRegistration registerField(const GridFactory<GridType> &factory) {
        auto registration = this->registerFieldImpl(factory);
        registerExchangeHandler<GridType>();
        registerAccumulateHandler<GridType>();
        return registration;
      }

      /**
       * Register a grid or field by passing a factory, but only allocate for a sub-range.
       */
      template<class GridType>
      ProjectedRegistration<GridType::Rank> registerFieldProjection(
          const GridFactory<GridType> &factory, const std::array<size_t, GridType::Rank> &axes
      ) {
        auto registration = this->registerFieldProjectionImpl(factory, axes);
        return registration;
      }

      /**
       * Register a grid or field by passing a factory, but only allocate for a sub-range.
       */
      template<class GridType>
      GridRegistration registerField(
          const GridFactory<GridType> &factory,
          const typename DomainDecomposition<rank, CheckingPolicy>::RangeType &subRange
      ) {
        auto registration = this->registerFieldImpl(factory, subRange);
        registerExchangeHandler<GridType>();
        registerAccumulateHandler<GridType>();
        return registration;
      }

      void exchangeGrid(const internal::pGridWrapper &wrapper, bool useFieldInfo = true) override;
      void accumulate(const internal::pGridWrapper &wrapper, bool useFieldInfo = true) override;

      /**
       * Register a particle container by passing a factory and a position
       * accessor.
       *
       * The serial decomposition creates a single container covering the global
       * range.
       */
      template<class ContainerType, class PositionAccessor>
      ParticleRegistration registerParticleData(
          const ParticleContainerFactory<ContainerType> &factory, PositionAccessor accessor
      ) {
        return this->registerParticleDataImpl(factory, std::move(accessor));
      }

      void migrateParticles(const internal::pParticleWrapper &wrapper) override;

    private:
      class ExchangeVisitor;
      class AccumulateVisitor;
      template<class GridType>
      void registerExchangeHandler();
      template<class GridType>
      void registerAccumulateHandler();
      template<typename GridType>
      void exchangeTyped(GridType &grid, bool useFieldInfo);
      template<typename GridType>
      void accumulateTyped(GridType &grid, bool useFieldInfo);
      template<typename GridType>
      void handleGridExchange(GridType &grid, bool useFieldInfo);
      template<typename FieldType>
      void handleFieldExchange(FieldType &field, bool useFieldInfo);
      template<typename GridType>
      void handleGridAccumulate(GridType &grid, bool useFieldInfo);
      template<typename FieldType>
      void handleFieldAccumulate(FieldType &field, bool useFieldInfo);
      RangeType getLocalInnerRange() const;
      template<typename GridType>
      void exchangeWithInteriorBounds(
          GridType &grid, const typename GridType::IndexType &innerLo, const typename GridType::IndexType &innerHi
      );
      template<typename GridType>
      void accumulateWithInteriorBounds(
          GridType &grid, const typename GridType::IndexType &innerLo, const typename GridType::IndexType &innerHi
      );
      
      std::unique_ptr<ExchangeVisitor> exchangeVisitor;
      std::unique_ptr<AccumulateVisitor> accumulateVisitor;
  };

  template<size_t rank, template<size_t> class CheckingPolicy>
  bool SerialDomainDecomposition<rank, CheckingPolicy>::master() const {
    return true;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  int SerialDomainDecomposition<rank, CheckingPolicy>::numProcs() const {
    return 1;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  double SerialDomainDecomposition<rank, CheckingPolicy>::avgReduce(double value) const {
    return value;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  int SerialDomainDecomposition<rank, CheckingPolicy>::avgReduce(int value) const {
    return value;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  long SerialDomainDecomposition<rank, CheckingPolicy>::avgReduce(long value) const {
    return value;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  double SerialDomainDecomposition<rank, CheckingPolicy>::sumReduce(double value) const {
    return value;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  int SerialDomainDecomposition<rank, CheckingPolicy>::sumReduce(int value) const {
    return value;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  long SerialDomainDecomposition<rank, CheckingPolicy>::sumReduce(long value) const {
    return value;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  double SerialDomainDecomposition<rank, CheckingPolicy>::maxReduce(double value) const {
    return value;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  int SerialDomainDecomposition<rank, CheckingPolicy>::maxReduce(int value) const {
    return value;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  long SerialDomainDecomposition<rank, CheckingPolicy>::maxReduce(long value) const {
    return value;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  double SerialDomainDecomposition<rank, CheckingPolicy>::minReduce(double value) const {
    return value;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  int SerialDomainDecomposition<rank, CheckingPolicy>::minReduce(int value) const {
    return value;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  long SerialDomainDecomposition<rank, CheckingPolicy>::minReduce(long value) const {
    return value;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void SerialDomainDecomposition<rank, CheckingPolicy>::balanceLoad() {
    // No load balancing needed for serial decomposition
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  int SerialDomainDecomposition<rank, CheckingPolicy>::getUniqueId() const {
    return 0;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void SerialDomainDecomposition<rank, CheckingPolicy>::init() {
    this->addLocalRange(this->globalRange, this->globalDomain);
  }

  /**
   * Sum up the global weights over the perpendicular directions and store
   * them in the 1d `weights` grid
   */
  template<template<size_t> class CheckingPolicy>
  void sumGlobalWeights(
      const Grid<double, 1> &globalWeights,
      typename DomainDecomposition<1, CheckingPolicy>::LimitType &lo,
      typename DomainDecomposition<1, CheckingPolicy>::LimitType &hi,
      size_t d,
      Grid<double, 1> &weights,
      double &sumTotal
  ) {
    SCHNEK_TRACE_ENTER_FUNCTION(2);
    double sum = 0;
    weights(lo[d] - 1) = 0.0;
    for (ptrdiff_t i = lo[d]; i <= hi[d]; ++i) {
      sum += globalWeights(i);

      weights(i) = sum;
    }

    sumTotal = sum;
  }

  /**
   * Sum up the global weights over the perpendicular directions and store
   * them in the 1d `weights` grid
   */
  template<size_t rank, template<size_t> class CheckingPolicy>
  void sumGlobalWeights(
      const Grid<double, rank> &globalWeights,
      typename DomainDecomposition<rank, CheckingPolicy>::LimitType &lo,
      typename DomainDecomposition<rank, CheckingPolicy>::LimitType &hi,
      size_t d,
      Grid<double, 1> &weights,
      double &sumTotal
  ) {
    typedef Range<size_t, rank - 1> Orth;
    Orth orth;
    Array<size_t, rank - 1> orthInd;

    // calculating orthogonal directions
    size_t oi = 0;
    for (size_t o = 0; o < rank; ++o) {
      if (o != d) {
        orth.getLo()[oi] = lo[o];
        orth.getHi()[oi] = hi[o];
        orthInd[oi] = o;
        oi++;
      }
    }

    // summing global weights
    sumTotal = 0;

    typename DomainDecomposition<rank, CheckingPolicy>::LimitType pos;
    weights(lo[d] - 1) = 0.0;
    for (ptrdiff_t i = lo[d]; i <= hi[d]; ++i) {
      pos[d] = i;
      typename Orth::iterator e = orth.end();
      for (typename Orth::iterator pi = orth.begin(); pi != e; ++pi) {
        const Array<size_t, rank - 1> &p = *pi;
        for (size_t oi = 0; oi < rank - 1; ++oi) {
          pos[orthInd[oi]] = p[oi];
        }

        sumTotal += globalWeights[pos];
      }

      weights(i) = sumTotal;
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  class SerialDomainDecomposition<rank, CheckingPolicy>::ExchangeVisitor
      : public internal::GridVisitor<typename SerialDomainDecomposition<rank, CheckingPolicy>::ExchangeVisitor> {
    public:
      explicit ExchangeVisitor(SerialDomainDecomposition &parentIn) : parent(parentIn) {}

      template<typename GridType>
      void handle(GridType &grid, bool flag) {
        parent.template exchangeTyped<GridType>(grid, flag);
      }

    private:
      SerialDomainDecomposition &parent;
  };

  template<size_t rank, template<size_t> class CheckingPolicy>
  class SerialDomainDecomposition<rank, CheckingPolicy>::AccumulateVisitor
      : public internal::GridVisitor<
            typename SerialDomainDecomposition<rank, CheckingPolicy>::AccumulateVisitor> {
    public:
      explicit AccumulateVisitor(SerialDomainDecomposition &parentIn) : parent(parentIn) {}

      template<typename GridType>
      void handle(GridType &grid, bool flag) {
        parent.template accumulateTyped<GridType>(grid, flag);
      }

    private:
      SerialDomainDecomposition &parent;
  };

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<class GridType>
  void SerialDomainDecomposition<rank, CheckingPolicy>::registerExchangeHandler() {
    if (exchangeVisitor.get() == nullptr) {
      exchangeVisitor = std::make_unique<ExchangeVisitor>(*this);
    }
    exchangeVisitor->template registerHandler<GridType>();
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<class GridType>
  void SerialDomainDecomposition<rank, CheckingPolicy>::registerAccumulateHandler() {
    if (accumulateVisitor.get() == nullptr) {
      accumulateVisitor = std::make_unique<AccumulateVisitor>(*this);
    }
    accumulateVisitor->template registerHandler<GridType>();
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void SerialDomainDecomposition<rank, CheckingPolicy>::exchangeGrid(
      const internal::pGridWrapper &wrapper, bool useFieldInfo
  ) {
    if (exchangeVisitor.get() == nullptr || exchangeVisitor->empty()) {
      SCHNECK_FAIL("No registered grids available for exchange");
    }

    wrapper->accept(*exchangeVisitor, useFieldInfo);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void SerialDomainDecomposition<rank, CheckingPolicy>::accumulate(
      const internal::pGridWrapper &wrapper, bool useFieldInfo
  ) {
    if (accumulateVisitor.get() == nullptr || accumulateVisitor->empty()) {
      SCHNECK_FAIL("No registered grids available for accumulate");
    }

    wrapper->accept(*accumulateVisitor, useFieldInfo);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void SerialDomainDecomposition<rank, CheckingPolicy>::migrateParticles(
      const internal::pParticleWrapper & /*wrapper*/
  ) {
    // A single process owns the whole domain, so no particle can ever leave its
    // local range. Migration is a no-op for the serial backend.
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename GridType>
  void SerialDomainDecomposition<rank, CheckingPolicy>::exchangeTyped(GridType &grid, bool useFieldInfo) {
    if constexpr (internal::is_field_v<GridType>) {
      handleFieldExchange(grid, useFieldInfo);
    } else {
      handleGridExchange(grid, useFieldInfo);
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename GridType>
  void SerialDomainDecomposition<rank, CheckingPolicy>::accumulateTyped(GridType &grid, bool useFieldInfo) {
    if constexpr (internal::is_field_v<GridType>) {
      handleFieldAccumulate(grid, useFieldInfo);
    } else {
      handleGridAccumulate(grid, useFieldInfo);
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  typename SerialDomainDecomposition<rank, CheckingPolicy>::RangeType
  SerialDomainDecomposition<rank, CheckingPolicy>::getLocalInnerRange() const {
    return this->globalRange;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename GridType>
  void
  SerialDomainDecomposition<rank, CheckingPolicy>::handleGridExchange(GridType &grid, bool /*useFieldInfo*/) {
    const auto localRange = getLocalInnerRange();
    typename GridType::IndexType innerLo(localRange.getLo());
    typename GridType::IndexType innerHi(localRange.getHi());
    exchangeWithInteriorBounds(grid, innerLo, innerHi);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename GridType>
  void SerialDomainDecomposition<rank, CheckingPolicy>::handleGridAccumulate(
      GridType &grid, bool /*useFieldInfo*/
  ) {
    const auto localRange = getLocalInnerRange();
    typename GridType::IndexType innerLo(localRange.getLo());
    typename GridType::IndexType innerHi(localRange.getHi());
    accumulateWithInteriorBounds(grid, innerLo, innerHi);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename FieldType>
  void SerialDomainDecomposition<rank, CheckingPolicy>::handleFieldExchange(FieldType &field, bool useFieldInfo) {
    typename FieldType::IndexType innerLo;
    typename FieldType::IndexType innerHi;
    if (useFieldInfo) {
      auto innerRange = field.getInnerRange();
      innerLo = innerRange.getLo();
      innerHi = innerRange.getHi();
    } else {
      const auto localRange = getLocalInnerRange();
      innerLo = localRange.getLo();
      innerHi = localRange.getHi();
    }
    exchangeWithInteriorBounds(field, innerLo, innerHi);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename FieldType>
  void SerialDomainDecomposition<rank, CheckingPolicy>::handleFieldAccumulate(
      FieldType &field, bool useFieldInfo
  ) {
    typename FieldType::IndexType innerLo;
    typename FieldType::IndexType innerHi;
    if (useFieldInfo) {
      auto innerRange = field.getInnerRange();
      innerLo = innerRange.getLo();
      innerHi = innerRange.getHi();
    } else {
      const auto localRange = getLocalInnerRange();
      innerLo = localRange.getLo();
      innerHi = localRange.getHi();
    }
    accumulateWithInteriorBounds(field, innerLo, innerHi);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename GridType>
  void SerialDomainDecomposition<rank, CheckingPolicy>::exchangeWithInteriorBounds(
      GridType &grid, const typename GridType::IndexType &innerLo, const typename GridType::IndexType &innerHi
  ) {
    using IndexType = typename GridType::IndexType;
    using RangeTypeLocal = typename GridType::RangeType;
    using ValueType = typename GridType::value_type;

    const IndexType gridLo = grid.getLo();
    const IndexType gridHi = grid.getHi();

    auto makeRange = [](const IndexType &loIdx, const IndexType &hiIdx) { return RangeTypeLocal(loIdx, hiIdx); };

    for (size_t dim = 0; dim < rank; ++dim) {
      ptrdiff_t lowerHalo = innerLo[dim] - gridLo[dim];
      ptrdiff_t upperHalo = gridHi[dim] - innerHi[dim];

      std::optional<RangeTypeLocal> loGhostRange;
      if (lowerHalo > 0) {
        IndexType lo = gridLo;
        IndexType hi = gridHi;
        hi[dim] = gridLo[dim] + lowerHalo - 1;
        loGhostRange = makeRange(lo, hi);
      }

      std::optional<RangeTypeLocal> hiGhostRange;
      if (upperHalo > 0) {
        IndexType lo = gridLo;
        IndexType hi = gridHi;
        lo[dim] = gridHi[dim] - upperHalo + 1;
        hi[dim] = gridHi[dim];
        hiGhostRange = makeRange(lo, hi);
      }

      std::optional<RangeTypeLocal> loSourceRange;
      if (lowerHalo > 0) {
        IndexType lo = gridLo;
        IndexType hi = gridHi;
        lo[dim] = innerLo[dim];
        hi[dim] = innerLo[dim] + lowerHalo - 1;
        loSourceRange = makeRange(lo, hi);
      }

      std::optional<RangeTypeLocal> hiSourceRange;
      if (upperHalo > 0) {
        IndexType lo = gridLo;
        IndexType hi = gridHi;
        lo[dim] = innerHi[dim] - upperHalo + 1;
        hi[dim] = innerHi[dim];
        hiSourceRange = makeRange(lo, hi);
      }

      if (loGhostRange && loSourceRange) {
        auto itGhost = loGhostRange->begin();
        auto itSource = loSourceRange->begin();
        for (; itGhost != loGhostRange->end() && itSource != loSourceRange->end(); ++itGhost, ++itSource) {
          grid[*itGhost] = grid[*itSource];
        }
      }

      if (hiGhostRange && hiSourceRange) {
        auto itGhost = hiGhostRange->begin();
        auto itSource = hiSourceRange->begin();
        for (; itGhost != hiGhostRange->end() && itSource != hiSourceRange->end(); ++itGhost, ++itSource) {
          grid[*itGhost] = grid[*itSource];
        }
      }
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename GridType>
  void SerialDomainDecomposition<rank, CheckingPolicy>::accumulateWithInteriorBounds(
      GridType &grid, const typename GridType::IndexType &innerLo, const typename GridType::IndexType &innerHi
  ) {
    using IndexType = typename GridType::IndexType;
    using RangeTypeLocal = typename GridType::RangeType;
    using ValueType = typename GridType::value_type;

    const IndexType gridLo = grid.getLo();
    const IndexType gridHi = grid.getHi();

    auto makeRange = [](const IndexType &loIdx, const IndexType &hiIdx) { return RangeTypeLocal(loIdx, hiIdx); };


    // For the serial decomposition, we can simply copy and accumulate the ghost cells from the inner cells.

    for (size_t dim = 0; dim < rank; ++dim) {
      ptrdiff_t lowerHalo = innerLo[dim] - gridLo[dim];
      ptrdiff_t upperHalo = gridHi[dim] - innerHi[dim];

      std::optional<RangeTypeLocal> loGhostRange;
      if (lowerHalo > 0) {
        IndexType lo = gridLo;
        IndexType hi = gridHi;
        hi[dim] = gridLo[dim] + lowerHalo - 1;
        loGhostRange = makeRange(lo, hi);
      }

      std::optional<RangeTypeLocal> hiGhostRange;
      if (upperHalo > 0) {
        IndexType lo = gridLo;
        IndexType hi = gridHi;
        lo[dim] = gridHi[dim] - upperHalo + 1;
        hi[dim] = gridHi[dim];
        hiGhostRange = makeRange(lo, hi);
      }

      std::optional<RangeTypeLocal> loSourceRange;
      if (lowerHalo > 0) {
        IndexType lo = gridLo;
        IndexType hi = gridHi;
        lo[dim] = innerLo[dim];
        hi[dim] = innerLo[dim] + lowerHalo - 1;
        loSourceRange = makeRange(lo, hi);
      }

      std::optional<RangeTypeLocal> hiSourceRange;
      if (upperHalo > 0) {
        IndexType lo = gridLo;
        IndexType hi = gridHi;
        lo[dim] = innerHi[dim] - upperHalo + 1;
        hi[dim] = innerHi[dim];
        hiSourceRange = makeRange(lo, hi);
      }

      if (loGhostRange && loSourceRange) {
        auto itGhost = loGhostRange->begin();
        auto itSource = loSourceRange->begin();
        for (; itGhost != loGhostRange->end() && itSource != loSourceRange->end(); ++itGhost, ++itSource) {
          grid[*itGhost] += grid[*itSource];
        }
      }

      if (hiGhostRange && hiSourceRange) {
        auto itGhost = hiGhostRange->begin();
        auto itSource = hiSourceRange->begin();
        for (; itGhost != hiGhostRange->end() && itSource != hiSourceRange->end(); ++itGhost, ++itSource) {
          grid[*itGhost] += grid[*itSource];
        }
      }
    }
  }

}  // namespace schnek

#endif /* SCHNEK_DECOMPOSITION_SERIAL_DECOMPOSITION_HPP_ */
