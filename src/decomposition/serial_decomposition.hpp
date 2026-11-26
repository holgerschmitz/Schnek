/*
 * serial_decomposition.hpp
 *
 *  Created on: 23 Jun 2026
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 */

#ifndef SCHNEK_DECOMPOSITION_SERIAL_DECOMPOSITION_HPP_
#define SCHNEK_DECOMPOSITION_SERIAL_DECOMPOSITION_HPP_

#include <functional>
#include <map>
#include <optional>
#include <utility>
#include <vector>

#include "../util/exceptions.hpp"
#include "config.hpp"
#include "detail/grid_visitor.hpp"
#include "detail/halo_ranges.hpp"
#include "detail/particle_visitor.hpp"
#include "detail/particle_wrap.hpp"
#include "domaindecomposition.hpp"

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
      SerialDomainDecomposition(){};

      ~SerialDomainDecomposition(){};

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
       * Register a lower-rank projected grid or field by selecting axes from the full-rank domain.
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
       *
       * Sub-range registrations are not supported by `exchange` or `accumulate`.
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
       * range. Accessors that provide `shift(p, dim, delta)` enable automatic
       * periodic position wrapping; otherwise wrapping remains the user's
       * responsibility.
       */
      template<class ContainerType, class PositionAccessor>
      ParticleRegistration registerParticleData(
          const ParticleContainerFactory<ContainerType> &factory, PositionAccessor accessor
      ) {
        auto registration = this->registerParticleDataImpl(factory, std::move(accessor));
        registerMigrateHandler<ContainerType, PositionAccessor>();
        return registration;
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
      std::pair<typename GridType::IndexType, typename GridType::IndexType> innerBounds(
          GridType &grid, bool useFieldInfo
      ) const;
      class MigrateVisitor;
      template<class ContainerType, class PositionAccessor>
      void registerMigrateHandler();
      template<typename WrapperImpl>
      void migrateTyped(WrapperImpl &wrapper);

      std::unique_ptr<MigrateVisitor> migrateVisitor;
      RangeType getLocalInnerRange() const;
      template<typename GridType>
      void exchangeWithInteriorBounds(
          GridType &grid, const typename GridType::IndexType &innerLo, const typename GridType::IndexType &innerHi
      );
      template<typename GridType>
      void accumulateWithInteriorBounds(
          GridType &grid, const typename GridType::IndexType &innerLo, const typename GridType::IndexType &innerHi
      );
      template<typename GridType, typename Operation>
      void applyWithInteriorBounds(
          GridType &grid,
          const typename GridType::IndexType &innerLo,
          const typename GridType::IndexType &innerHi,
          Operation operation
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
      : public internal::GridVisitor<typename SerialDomainDecomposition<rank, CheckingPolicy>::AccumulateVisitor> {
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
  class SerialDomainDecomposition<rank, CheckingPolicy>::MigrateVisitor
      : public internal::ParticleVisitor<typename SerialDomainDecomposition<rank, CheckingPolicy>::MigrateVisitor> {
    public:
      explicit MigrateVisitor(SerialDomainDecomposition &parentIn) : parent(parentIn) {}

      template<typename WrapperImpl>
      void handle(WrapperImpl &wrapper) {
        parent.template migrateTyped<WrapperImpl>(wrapper);
      }

    private:
      SerialDomainDecomposition &parent;
  };

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<class ContainerType, class PositionAccessor>
  void SerialDomainDecomposition<rank, CheckingPolicy>::registerMigrateHandler() {
    if (migrateVisitor.get() == nullptr) {
      migrateVisitor = std::make_unique<MigrateVisitor>(*this);
    }
    migrateVisitor->template registerHandler<internal::ParticleWrapperImpl<ContainerType, PositionAccessor>>();
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename WrapperImpl>
  void SerialDomainDecomposition<rank, CheckingPolicy>::migrateTyped(WrapperImpl &wrapper) {
    using ContainerType = std::decay_t<decltype(wrapper.container)>;
    using Traits = ParticleContainerTraits<ContainerType>;
    using Particle = typename Traits::value_type;
    using PositionAccessor = std::decay_t<decltype(wrapper.accessor)>;

    for (std::size_t dim = 0; dim < rank; ++dim) {
      if (!Traits::allOf(
              wrapper.container, internal::FinitePositionValue<Particle, PositionAccessor>{wrapper.accessor, dim}
          )) {
        SCHNECK_FAIL("non-finite particle position");
      }
    }

    if constexpr (internal::has_position_shift<PositionAccessor, Particle>::value) {
      for (std::size_t dim = 0; dim < rank; ++dim) {
        const ptrdiff_t globalLo = this->globalRange.getLo()[dim];
        const ptrdiff_t globalHi = this->globalRange.getHi()[dim];
        const double length = static_cast<double>(globalHi - globalLo + 1);
        if (!Traits::allOf(
                wrapper.container,
                internal::ValidWrapPeriods<Particle, PositionAccessor>{
                    wrapper.accessor, dim, static_cast<double>(globalLo), length
                }
            )) {
          SCHNECK_FAIL("Particle periodic wrap count exceeds the supported magnitude");
        }
        Traits::forEachParticle(
            wrapper.container,
            internal::WrapDim<Particle, PositionAccessor>{
                wrapper.accessor, dim, static_cast<double>(globalLo), static_cast<double>(globalHi), length
            }
        );
      }
    } else {
      for (std::size_t dim = 0; dim < rank; ++dim) {
        const ptrdiff_t globalLo = this->globalRange.getLo()[dim];
        const ptrdiff_t globalHi = this->globalRange.getHi()[dim];
        if (!Traits::allOf(
                wrapper.container,
                internal::PositionCellInRange<Particle, PositionAccessor>{
                    wrapper.accessor, dim, static_cast<double>(globalLo), static_cast<double>(globalHi)
                }
            )) {
          SCHNECK_FAIL(
              "Particle left the global domain and the position accessor has no shift(); either provide "
              "shift(p, dim, delta) for periodic wrapping or handle the boundary (absorb/reflect) before migrate()"
          );
        }
      }
    }
  }

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
  void SerialDomainDecomposition<rank, CheckingPolicy>::migrateParticles(const internal::pParticleWrapper &wrapper) {
    if (migrateVisitor.get() == nullptr || migrateVisitor->empty()) {
      SCHNECK_FAIL("No registered particle containers available for migration");
    }

    wrapper->accept(*migrateVisitor);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename GridType>
  void SerialDomainDecomposition<rank, CheckingPolicy>::exchangeTyped(GridType &grid, bool useFieldInfo) {
    const auto [innerLo, innerHi] = innerBounds(grid, useFieldInfo);
    exchangeWithInteriorBounds(grid, innerLo, innerHi);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename GridType>
  void SerialDomainDecomposition<rank, CheckingPolicy>::accumulateTyped(GridType &grid, bool useFieldInfo) {
    const auto [innerLo, innerHi] = innerBounds(grid, useFieldInfo);
    accumulateWithInteriorBounds(grid, innerLo, innerHi);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  typename SerialDomainDecomposition<rank, CheckingPolicy>::RangeType
  SerialDomainDecomposition<rank, CheckingPolicy>::getLocalInnerRange() const {
    return this->globalRange;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename GridType>
  std::pair<typename GridType::IndexType, typename GridType::IndexType>
  SerialDomainDecomposition<rank, CheckingPolicy>::innerBounds(GridType &grid, bool useFieldInfo) const {
    using IndexType = typename GridType::IndexType;
    if (useFieldInfo) {
      if constexpr (internal::is_field_v<GridType>) {
        const auto innerRange = grid.getInnerRange();
        return {IndexType(innerRange.getLo()), IndexType(innerRange.getHi())};
      }
    }
    const auto localRange = getLocalInnerRange();
    return {IndexType(localRange.getLo()), IndexType(localRange.getHi())};
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename GridType>
  void SerialDomainDecomposition<rank, CheckingPolicy>::exchangeWithInteriorBounds(
      GridType &grid, const typename GridType::IndexType &innerLo, const typename GridType::IndexType &innerHi
  ) {
    applyWithInteriorBounds(grid, innerLo, innerHi, [](auto &ghost, auto &source) { ghost = source; });
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename GridType>
  void SerialDomainDecomposition<rank, CheckingPolicy>::accumulateWithInteriorBounds(
      GridType &grid, const typename GridType::IndexType &innerLo, const typename GridType::IndexType &innerHi
  ) {
    // Periodic ghost cells and their matching interior images both receive the accumulated sum.
    applyWithInteriorBounds(grid, innerLo, innerHi, [](auto &ghost, auto &source) {
      ghost += source;
      source = ghost;
    });
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename GridType, typename Operation>
  void SerialDomainDecomposition<rank, CheckingPolicy>::applyWithInteriorBounds(
      GridType &grid,
      const typename GridType::IndexType &innerLo,
      const typename GridType::IndexType &innerHi,
      Operation operation
  ) {
    using IndexType = typename GridType::IndexType;
    using RangeTypeLocal = typename GridType::RangeType;

    const IndexType gridLo = grid.getLo();
    const IndexType gridHi = grid.getHi();

    for (size_t dim = 0; dim < rank; ++dim) {
      const auto ranges = detail::haloRanges<RangeTypeLocal>(dim, gridLo, gridHi, innerLo, innerHi);

      if (ranges.loGhostRange && ranges.hiSourceRange) {
        auto itGhost = ranges.loGhostRange->begin();
        auto itSource = ranges.hiSourceRange->begin();
        for (; itGhost != ranges.loGhostRange->end() && itSource != ranges.hiSourceRange->end();
             ++itGhost, ++itSource) {
          operation(grid[*itGhost], grid[*itSource]);
        }
      }

      if (ranges.hiGhostRange && ranges.loSourceRange) {
        auto itGhost = ranges.hiGhostRange->begin();
        auto itSource = ranges.loSourceRange->begin();
        for (; itGhost != ranges.hiGhostRange->end() && itSource != ranges.loSourceRange->end();
             ++itGhost, ++itSource) {
          operation(grid[*itGhost], grid[*itSource]);
        }
      }
    }
  }

}  // namespace schnek

#endif /* SCHNEK_DECOMPOSITION_SERIAL_DECOMPOSITION_HPP_ */
