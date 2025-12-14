/*
 * mpi_cartesian_decomposition.hpp
 *
 *  Created on: 10 Apr 2020
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 */

#ifndef SCHNEK_DECOMPOSITION_MPI_CARTESIAN_DECOMPOSITION_HPP_
#define SCHNEK_DECOMPOSITION_MPI_CARTESIAN_DECOMPOSITION_HPP_

#include "../config.hpp"
#include "domaindecomposition.hpp"
#include "mpi_context.hpp"

#ifdef SCHNEK_HAVE_MPI

#include <mpi.h>

#include <functional>
#include <vector>

namespace schnek {

  template<size_t rank, template<size_t> class CheckingPolicy = ArrayNoArgCheck>
  class MpiCartesianDomainDecomposition : public DomainDecomposition<rank, CheckingPolicy> {
    public:
      typedef Array<Grid<Range<ptrdiff_t, 1>, 1>, rank> ProcRanges;
      /**
       * Constructor creating the domain decomposition object
       *
       * @param mpi  the MPI context to use for MPI calls
       */
      MpiCartesianDomainDecomposition(MpiContext &mpi = schnek::detail::mpiContextImpl);

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
      GridRegistration registerField(GridFactory<GridType> &factory) {
        auto registration = this->registerFieldImpl(factory);
        registerExchangeHandler<GridType>();
        registerAccumulateHandler<GridType>();
        return registration;
      }

      void exchange(const internal::pGridWrapper &wrapper, bool useFieldInfo = true) override;
      void accumulate(const internal::pGridWrapper &wrapper, bool useFieldInfo = true) override;

    private:
      typedef typename DomainDecomposition<rank, CheckingPolicy>::LimitType LimitType;
      typedef typename DomainDecomposition<rank, CheckingPolicy>::RangeType RangeType;
      typedef typename DomainDecomposition<rank, CheckingPolicy>::DomainType DomainType;

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
      std::vector<std::function<void(ExchangeVisitor &)>> exchangeInitializers;
        std::vector<std::function<void(AccumulateVisitor &)>> accumulateInitializers;
  };

}  // namespace schnek

#include "mpi_cartesian_decomposition.t"

#endif  // HAVE_MPI

#endif /* SCHNEK_DECOMPOSITION_MPI_CARTESIAN_DECOMPOSITION_HPP_ */
