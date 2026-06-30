/*
 * mpi_cartesian_decomposition.hpp
 *
 *  Created on: 10 Apr 2020
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 */

#ifndef SCHNEK_DECOMPOSITION_MPI_CARTESIAN_DECOMPOSITION_HPP_
#define SCHNEK_DECOMPOSITION_MPI_CARTESIAN_DECOMPOSITION_HPP_

#include "../config.hpp"
#include "detail/redistribution.hpp"
#include "domaindecomposition.hpp"
#include "mpi_context.hpp"
#include "../util/scratchbuffer.hpp"

#ifdef SCHNEK_HAVE_MPI

#include <mpi.h>

#include <cmath>
#include <cstddef>
#include <functional>
#include <map>
#include <set>
#include <vector>

namespace schnek {

  /**
   * MPI Cartesian domain decomposition backend.
   *
   * This backend partitions the global index range into a Cartesian grid of
   * processes. Each process owns **exactly one** rectangular local range; the
   * base-class abstraction of multiple local ranges per process is not used
   * here.  All internal helpers (`balanceLoad`, `exchangeGrid`, etc.) rely on
   * this single-region invariant and will assert in debug builds if it is
   * violated.
   *
   * Future backends that support multiple regions per process (e.g. SFC, AMR)
   * must not derive from this class; they should derive from
   * `DomainDecomposition` directly.
   */
  template<size_t rank, template<size_t> class CheckingPolicy = ArrayNoArgCheck>
  class MpiCartesianDomainDecomposition : public DomainDecomposition<rank, CheckingPolicy> {
    public:
      typedef typename DomainDecomposition<rank, CheckingPolicy>::LimitType LimitType;
      typedef typename DomainDecomposition<rank, CheckingPolicy>::RangeType RangeType;
      typedef typename DomainDecomposition<rank, CheckingPolicy>::DomainType DomainType;

      template<size_t projRank>
      using ProjectedRegistration = ProjectedGridRegistration<projRank, rank>;

      /**
       * A type alias for the local ranges of the domain decomposition.
       * 
       * For each dimension, the local ranges of all processes in that dimension are stored in a 1d Grid.
       */
      typedef Array<Grid<Range<ptrdiff_t, 1>, 1>, rank> ProcRanges;
      
      /**
       * Constructor creating the domain decomposition object
       *
       * @param mpi  the MPI context to use for MPI calls
       */
      MpiCartesianDomainDecomposition(MpiContext &mpi = schnek::detail::mpiContextImpl);

      ~MpiCartesianDomainDecomposition();

      MpiCartesianDomainDecomposition(const MpiCartesianDomainDecomposition &) = delete;
      MpiCartesianDomainDecomposition(MpiCartesianDomainDecomposition &&) = delete;
      MpiCartesianDomainDecomposition &operator=(const MpiCartesianDomainDecomposition &) = delete;
      MpiCartesianDomainDecomposition &operator=(MpiCartesianDomainDecomposition &&) = delete;

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
       * @return the number of processes in the MPI communicator
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
       * Exchange halo cells between processes. Function overload accepting a single grid registration.
       *
       * The extent of the halo is determined by the the `useFieldInfo` flag. If false, the halo is determined
       * by the size of the grid in relation to the local index range. If true (default), for `Field`-type grids,
       * it is taken from `ghostCells` parameter of the field. For plain `Grid`-type grids, the flag has no effect.
       */
      //   void exchange(std::initializer_list<GridRegistration> registrations, bool useFieldInfo = true) override;

      /**
       * Return the grid index ranges of each process coordinates in each direction
       *
       * @return An array with an entry for each dimension. For each dimension multiple ranges
       *         are stored inside a 1d Grid
       */
      const ProcRanges &getProcRanges();

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
        registerRedistributeHandler<GridType>();
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
        registerRedistributeProjectedHandler<GridType>();
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
        registerRedistributeHandler<GridType>();
        return registration;
      }

      void exchangeGrid(const internal::pGridWrapper &wrapper, bool useFieldInfo = true) override;
      void accumulate(const internal::pGridWrapper &wrapper, bool useFieldInfo = true) override;

      /**
       * Register a particle container by passing a factory and a position
       * accessor.
       *
       * The decomposition creates one container per local range (one for this
       * single-region backend). The accessor maps a particle to its continuous
       * grid position.
       */
      template<class ContainerType, class PositionAccessor>
      ParticleRegistration registerParticleData(
          const ParticleContainerFactory<ContainerType> &factory, PositionAccessor accessor
      ) {
        auto registration = this->registerParticleDataImpl(factory, accessor);
        registerMigrateHandler<ContainerType, PositionAccessor>();
        registerRedistributeParticleHandler<ContainerType, PositionAccessor>();
        return registration;
      }

      void migrateParticles(const internal::pParticleWrapper &wrapper) override;

    private:
      /// The MPI context
      MpiContext &mpi;

      /// The number of processes
      int ComSize;

      /// The rank of the current process
      int ComRank;

      /// The Comm object referring to the Cartesian process grid
      MPI_Comm comm;

      /// Dimensions of the Cartesian topology
      LimitType dims;

      /// The Cartesian coordinates of this process
      LimitType myCoord;

      /**
       * The grid index ranges of each process coordinates in each direction
       */
      ProcRanges procRanges;

      /**
       * Determine the new grid layout based on the local or global weights
       */
      void calcGridDistributon(ProcRanges &ranges);

      /**
       * Determine the new grid layout when no weights are given
       */
      void calcGridDistributonUniform(ProcRanges &ranges);

      /**
       * Determine the new grid layout based on the global weights
       */
      void calcGridDistributonGlobalWeights(ProcRanges &ranges);

      /**
       * Determine the new grid layout based on the local weights
       */
      void calcGridDistributonLocalWeights(ProcRanges &ranges);

      template<typename T>
      T allReduce(T value, MPI_Op op) const;

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

      /**
       * Migrate particles of a single container to the processes that now own
       * them. The visitor recovers the concrete container/accessor types so the
       * per-particle bounds test, serialisation and movement are inlined.
       */
      class MigrateVisitor;
      template<class ContainerType, class PositionAccessor>
      void registerMigrateHandler();
      template<typename WrapperImpl>
      void migrateTyped(WrapperImpl &wrapper);

      std::unique_ptr<MigrateVisitor> migrateVisitor;

      /**
       * Redistribute the particles of a single container between the old and
       * new sub-domain layouts during `balanceLoad`.
       *
       * Reuses the serialise/exchange/insert path of migration but routes each
       * particle to its new owner using the same Cartesian transfer plan that
       * drives grid redistribution, so particles may move more than one
       * sub-domain when boundaries shift.
       */
      class RedistributeParticleVisitor;
      template<class ContainerType, class PositionAccessor>
      void registerRedistributeParticleHandler();
      template<typename WrapperImpl>
      void redistributeParticlesTyped(
          WrapperImpl &oldWrapper,
          WrapperImpl &newWrapper,
          const std::vector<TransferBlock<rank, CheckingPolicy>> &sendPlan,
          const std::vector<TransferBlock<rank, CheckingPolicy>> &recvPlan
      );

      void redistributeParticles(
          const internal::pParticleWrapper &oldWrapper,
          const internal::pParticleWrapper &newWrapper,
          const std::vector<TransferBlock<rank, CheckingPolicy>> &sendPlan,
          const std::vector<TransferBlock<rank, CheckingPolicy>> &recvPlan
      );

      std::vector<std::function<void(RedistributeParticleVisitor &)>> redistributeParticleInitializers;

      schnek::ScratchBuffer mpiSendScratchBuffer;
      schnek::ScratchBuffer mpiRecvScratchBuffer;

      /**
       * Convert Cartesian process coordinates to MPI rank
       */
      int coordToMpiRank(const LimitType &coord) const;

      /**
       * Compute local domain from a local range using the global range and domain
       */
      DomainType computeLocalDomain(const RangeType &localRange) const;

      /**
       * Redistribute a single grid between old and new layouts
       *
       * The visitor pattern is used to dispatch on the grid type. The method
       * handles packing, MPI non-blocking communication, local data copying,
       * and unpacking.
       */
      class RedistributeVisitor;
      template<class GridType>
      void registerRedistributeHandler();
      template<typename GridType>
      void redistributeTyped(
          GridType &oldGrid,
          GridType &newGrid,
          const std::vector<TransferBlock<rank, CheckingPolicy>> &sendPlan,
          const std::vector<TransferBlock<rank, CheckingPolicy>> &recvPlan
      );

      void redistributeGrid(
          const internal::pGridWrapper &oldWrapper,
          const internal::pGridWrapper &newWrapper,
          const std::vector<TransferBlock<rank, CheckingPolicy>> &sendPlan,
          const std::vector<TransferBlock<rank, CheckingPolicy>> &recvPlan
      );

      std::vector<std::function<void(RedistributeVisitor &)>> redistributeInitializers;

      /// Dynamic-rank transfer block used for projected-grid redistribution.
      struct DynProjTransferBlock {
          int mpiRank;
          std::vector<ptrdiff_t> lo;
          std::vector<ptrdiff_t> hi;
      };

      /**
       * Redistribute projected grids across processes after balanceLoad.
       *
       * For each projected registration:
       *   1. Among the "canonical" replicas (processes whose Cartesian
       *      coordinates are zero on every non-projected axis) perform a
       *      point-to-point redistribution at projected rank.
       *   2. Broadcast the new projected grid contents within each replica
       *      group (the orthogonal sub-communicator), so non-canonical
       *      replicas see identical data.
       */
      void redistributeProjectedGrids(
          const std::map<long, std::vector<internal::pGridWrapper>> &oldProjectedGrids,
          const ProcRanges &oldRanges,
          const ProcRanges &newRanges
      );

      /// Lazily-created orthogonal sub-communicators keyed by registration id.
      std::map<std::vector<size_t>, MPI_Comm> projectedReplicaComms;

      class RedistributeProjectedVisitor;
      template<class GridType>
      void registerRedistributeProjectedHandler();
      template<typename GridType>
      void redistributeProjectedTyped(
          GridType &oldGrid,
          GridType &newGrid,
          MPI_Comm replicaComm,
          bool isCanonical,
          const std::vector<DynProjTransferBlock> &sendPlan,
          const std::vector<DynProjTransferBlock> &recvPlan
      );

      void redistributeProjectedGrid(
          const internal::pGridWrapper &oldWrapper,
          const internal::pGridWrapper &newWrapper,
          MPI_Comm replicaComm,
          bool isCanonical,
          const std::vector<DynProjTransferBlock> &sendPlan,
          const std::vector<DynProjTransferBlock> &recvPlan
      );

      std::vector<std::function<void(RedistributeProjectedVisitor &)>> redistributeProjectedInitializers;
  };

}  // namespace schnek

#include "mpi_cartesian_decomposition.t"

#endif  // HAVE_MPI

#endif /* SCHNEK_DECOMPOSITION_MPI_CARTESIAN_DECOMPOSITION_HPP_ */
