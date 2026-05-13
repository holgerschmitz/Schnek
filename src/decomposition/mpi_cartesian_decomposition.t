/*
 * mpi_cartesian_decomposition.t
 *
 *  Created on: 30 Apr 2020
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 */

#include <limits>
#include <optional>
#include <type_traits>

#include "../diagnostic/diagnostic.hpp"
#include "../util/exceptions.hpp"
#include "../util/factor.hpp"
#include "../util/interpolate1d.hpp"
#include "../util/logger.hpp"
#include "detail/redistribution.hpp"
#include "mpi_cartesian_decomposition.hpp"

#undef SCHNEK_LOGLEVEL
#define SCHNEK_LOGLEVEL 0

namespace schnek {

  namespace detail {
    template<typename T>
    MPI_Datatype mpiDatatypeFor() {
      if constexpr (std::is_same_v<T, signed char>) {
        return MPI_CHAR;
      } else if constexpr (std::is_same_v<T, signed short int>) {
        return MPI_SHORT;
      } else if constexpr (std::is_same_v<T, signed int>) {
        return MPI_INT;
      } else if constexpr (std::is_same_v<T, signed long int>) {
        return MPI_LONG;
      } else if constexpr (std::is_same_v<T, unsigned char>) {
        return MPI_UNSIGNED_CHAR;
      } else if constexpr (std::is_same_v<T, unsigned short int>) {
        return MPI_UNSIGNED_SHORT;
      } else if constexpr (std::is_same_v<T, unsigned int>) {
        return MPI_UNSIGNED;
      } else if constexpr (std::is_same_v<T, unsigned long int>) {
        return MPI_UNSIGNED_LONG;
      } else if constexpr (std::is_same_v<T, float>) {
        return MPI_FLOAT;
      } else if constexpr (std::is_same_v<T, double>) {
        return MPI_DOUBLE;
      } else if constexpr (std::is_same_v<T, long double>) {
        return MPI_LONG_DOUBLE;
      } else {
        SCHNECK_FAIL("Unsupported grid value type for MPI exchange");
      }
    }
  }  // namespace detail

  template<size_t rank, template<size_t> class CheckingPolicy>
  MpiCartesianDomainDecomposition<rank, CheckingPolicy>::MpiCartesianDomainDecomposition(MpiContext &mpi) : mpi(mpi) {}

  template<size_t rank, template<size_t> class CheckingPolicy>
  bool MpiCartesianDomainDecomposition<rank, CheckingPolicy>::master() const {
    return ComRank == 0;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  int MpiCartesianDomainDecomposition<rank, CheckingPolicy>::numProcs() const {
    return ComSize;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename T>
  T MpiCartesianDomainDecomposition<rank, CheckingPolicy>::allReduce(T value, MPI_Op op) const {
    T result{};
    int errorCode = mpi.MPI_Allreduce(&value, &result, 1, detail::mpiDatatypeFor<T>(), op, comm);
    SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Allreduce failed");
    return result;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  double MpiCartesianDomainDecomposition<rank, CheckingPolicy>::avgReduce(double value) const {
    SCHNEK_ASSERT(ComSize > 0, "MPI communicator size is zero");
    return allReduce(value, MPI_SUM) / double(ComSize);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  int MpiCartesianDomainDecomposition<rank, CheckingPolicy>::avgReduce(int value) const {
    SCHNEK_ASSERT(ComSize > 0, "MPI communicator size is zero");
    return static_cast<int>(allReduce(value, MPI_SUM) / double(ComSize));
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  long MpiCartesianDomainDecomposition<rank, CheckingPolicy>::avgReduce(long value) const {
    SCHNEK_ASSERT(ComSize > 0, "MPI communicator size is zero");
    return static_cast<long>(allReduce(value, MPI_SUM) / double(ComSize));
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  double MpiCartesianDomainDecomposition<rank, CheckingPolicy>::sumReduce(double value) const {
    return allReduce(value, MPI_SUM);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  int MpiCartesianDomainDecomposition<rank, CheckingPolicy>::sumReduce(int value) const {
    return allReduce(value, MPI_SUM);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  long MpiCartesianDomainDecomposition<rank, CheckingPolicy>::sumReduce(long value) const {
    return allReduce(value, MPI_SUM);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  double MpiCartesianDomainDecomposition<rank, CheckingPolicy>::maxReduce(double value) const {
    return allReduce(value, MPI_MAX);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  int MpiCartesianDomainDecomposition<rank, CheckingPolicy>::maxReduce(int value) const {
    return allReduce(value, MPI_MAX);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  long MpiCartesianDomainDecomposition<rank, CheckingPolicy>::maxReduce(long value) const {
    return allReduce(value, MPI_MAX);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  double MpiCartesianDomainDecomposition<rank, CheckingPolicy>::minReduce(double value) const {
    return allReduce(value, MPI_MIN);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  int MpiCartesianDomainDecomposition<rank, CheckingPolicy>::minReduce(int value) const {
    return allReduce(value, MPI_MIN);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  long MpiCartesianDomainDecomposition<rank, CheckingPolicy>::minReduce(long value) const {
    return allReduce(value, MPI_MIN);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  const Array<Grid<Range<ptrdiff_t, 1>, 1>, rank> &MpiCartesianDomainDecomposition<rank, CheckingPolicy>::getProcRanges(
  ) {
    return procRanges;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  int MpiCartesianDomainDecomposition<rank, CheckingPolicy>::coordToMpiRank(const LimitType &coord) const {
    int coordRaw[rank];
    for (size_t i = 0; i < rank; ++i) {
      coordRaw[i] = static_cast<int>(coord[i]);
    }
    int result;
    int errorCode = mpi.MPI_Cart_rank(comm, coordRaw, &result);
    SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Cart_rank failed");
    return result;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  typename MpiCartesianDomainDecomposition<rank, CheckingPolicy>::DomainType
  MpiCartesianDomainDecomposition<rank, CheckingPolicy>::computeLocalDomain(const RangeType &localRange) const {
    DomainType localDomain(this->globalDomain);
    const LimitType &globalLo = this->globalRange.getLo();
    const LimitType &globalHi = this->globalRange.getHi();

    for (size_t d = 0; d < rank; ++d) {
      const ptrdiff_t globalCells = globalHi[d] - globalLo[d] + 1;
      const ptrdiff_t localCells = localRange.getHi()[d] - localRange.getLo()[d] + 1;
      const double cellSize =
          globalCells > 0 ? (this->globalDomain.getHi()[d] - this->globalDomain.getLo()[d]) / double(globalCells) : 0.0;
      const ptrdiff_t startOffset = localRange.getLo()[d] - globalLo[d];

      localDomain.getLo()[d] = this->globalDomain.getLo()[d] + cellSize * double(startOffset);
      localDomain.getHi()[d] = localDomain.getLo()[d] + cellSize * double(localCells);
    }

    return localDomain;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::balanceLoad() {
    using TransferBlockType = TransferBlock<rank, CheckingPolicy>;

#ifndef NDEBUG
    // Invariant: this backend creates exactly one local range per process.
    // Every registration's grid list must therefore hold at most one wrapper.
    for (const auto &entry : this->getGridStorage()) {
      SCHNEK_ASSERT(
          entry.second.size() <= 1,
          "MpiCartesianDomainDecomposition: registration " << entry.first
              << " holds " << entry.second.size()
              << " grid wrappers but the single-region invariant requires at most 1."
      );
    }
#endif

    // 1. Save the old proc ranges
    ProcRanges oldRanges;
    for (size_t d = 0; d < rank; ++d) {
      oldRanges[d].resize(procRanges[d].getLo(), procRanges[d].getHi());
      for (ptrdiff_t c = procRanges[d].getLo()[0]; c <= procRanges[d].getHi()[0]; ++c) {
        oldRanges[d](c) = procRanges[d](c);
      }
    }

    // 2. Compute new grid distribution
    ProcRanges newRanges;
    calcGridDistributon(newRanges);

    // 3. Compute new local range and domain
    RangeType newLocalRange;
    for (size_t d = 0; d < rank; ++d) {
      newLocalRange.getLo()[d] = newRanges[d](myCoord[d]).getLo()[0];
      newLocalRange.getHi()[d] = newRanges[d](myCoord[d]).getHi()[0];
    }

    DomainType newLocalDomain = computeLocalDomain(newLocalRange);

    // 4. Compute transfer plans
    std::vector<TransferBlockType> sendPlan;
    std::vector<TransferBlockType> recvPlan;

    computeCartesianTransferPlan<rank, CheckingPolicy>(
        oldRanges, newRanges, dims, myCoord, ComRank,
        [this](const LimitType &coord) -> int { return coordToMpiRank(coord); }, sendPlan, recvPlan
    );

    // 5. Save old grid wrappers (take ownership)
    auto &gridStorage = this->getGridStorage();
    std::map<long, std::list<internal::pGridWrapper>> oldGrids;
    for (auto &entry : gridStorage) {
      oldGrids[entry.first] = std::move(entry.second);
    }

    // 6. Clear all local ranges and recreate with new layout
    this->clearLocalRanges();
    this->addLocalRange(newLocalRange, newLocalDomain);

    // 7. Redistribute each registered grid one at a time
    for (auto &entry : gridStorage) {
      long id = entry.first;
      auto &newGridList = entry.second;
      auto oldGridIt = oldGrids.find(id);

      if (oldGridIt == oldGrids.end() || oldGridIt->second.empty() || newGridList.empty()) {
        continue;
      }

      auto oldWrapper = oldGridIt->second.front();
      auto newWrapper = newGridList.front();

      if (oldWrapper && newWrapper) {
        redistributeGrid(oldWrapper, newWrapper, sendPlan, recvPlan);
      }
    }

    // 8. Update proc ranges
    procRanges = newRanges;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  int MpiCartesianDomainDecomposition<rank, CheckingPolicy>::getUniqueId() const {
    int id = myCoord[0];
    for (size_t i = 1; i < rank; ++i) id = dims[i] * id + myCoord[i];

    SCHNEK_TRACE_LOG(2, "MpiCartesianDomainDecomposition::getUniqueId() " << id);
    return id;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::init() {
    int errorCode;
    // Arrange the processes in a Cartesian grid topology
    LimitType lo = this->globalRange.getLo();
    LimitType hi = this->globalRange.getHi();

    // if global weights are specified, use the resolution of those for the processor layout
    if (this->globalWeights.getDims().product() != 0) {
      lo = this->globalWeights.getLo();
      hi = this->globalWeights.getHi();
    }

    errorCode = mpi.MPI_Comm_size(mpi.getCommWorld(), &ComSize);
    SCHNEK_ASSERT(
        errorCode == MPI_SUCCESS, "Could not obtain MPI Comm size (" + boost::lexical_cast<std::string>(errorCode) + ")"
    );

    int periodic[rank];

    std::vector<size_t> box(rank);

    for (size_t i = 0; i < rank; ++i) {
      box[i] = hi[i] - lo[i] + 1;
      periodic[i] = true;
    }

    std::vector<size_t> eqDims;

    equalFactors(ComSize, rank, eqDims, box);

    int dimsRaw[rank];
    int myCoordRaw[rank];

    for (size_t i = 0; i < rank; ++i) {
      dimsRaw[i] = eqDims[i];
    }

    errorCode = this->mpi.MPI_Cart_create(this->mpi.getCommWorld(), rank, dimsRaw, periodic, true, &comm);
    SCHNEK_ASSERT(
        errorCode == MPI_SUCCESS,
        "Could not create MPI Cartesian topology (" + boost::lexical_cast<std::string>(errorCode) + ")"
    );

    errorCode = this->mpi.MPI_Comm_rank(comm, &ComRank);
    SCHNEK_ASSERT(
        errorCode == MPI_SUCCESS, "Could not determine MPI rank (" + boost::lexical_cast<std::string>(errorCode) + ")"
    );

    errorCode = this->mpi.MPI_Cart_coords(comm, ComRank, rank, myCoordRaw);
    SCHNEK_ASSERT(
        errorCode == MPI_SUCCESS,
        "Could not determine MPI Cartesian coordinates (" + boost::lexical_cast<std::string>(errorCode) + ")"
    );

    for (size_t i = 0; i < rank; ++i) {
      dims[i] = dimsRaw[i];
      myCoord[i] = myCoordRaw[i];
    }

    // Determine the sizes of the local grids
    calcGridDistributon(procRanges);

    RangeType localRange(this->globalRange);
    DomainType localDomain(this->globalDomain);
    const LimitType &globalLo = this->globalRange.getLo();
    const LimitType &globalHi = this->globalRange.getHi();

    for (size_t d = 0; d < rank; ++d) {
      const Range<ptrdiff_t, 1> &dimRange = procRanges[d](myCoord[d]);
      localRange.getLo()[d] = dimRange.getLo()[0];
      localRange.getHi()[d] = dimRange.getHi()[0];

      const ptrdiff_t globalCells = globalHi[d] - globalLo[d] + 1;
      const ptrdiff_t localCells = localRange.getHi()[d] - localRange.getLo()[d] + 1;
      const double cellSize =
          globalCells > 0 ? (this->globalDomain.getHi()[d] - this->globalDomain.getLo()[d]) / double(globalCells) : 0.0;
      const ptrdiff_t startOffset = localRange.getLo()[d] - globalLo[d];

      localDomain.getLo()[d] = this->globalDomain.getLo()[d] + cellSize * double(startOffset);
      localDomain.getHi()[d] = localDomain.getLo()[d] + cellSize * double(localCells);
    }

    this->addLocalRange(localRange, localDomain);

#ifndef NDEBUG
    // Invariant: init() must add exactly one local range.
    SCHNEK_ASSERT(
        this->localRangeCount() == 1,
        "MpiCartesianDomainDecomposition::init(): expected exactly 1 local range after init, got "
            << this->localRangeCount()
    );
#endif
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::calcGridDistributon(ProcRanges &ranges) {
    if (this->localWeights.getDims().product() > 0) {
      calcGridDistributonLocalWeights(ranges);
    } else if (this->globalWeights.getDims().product() > 0) {
      calcGridDistributonGlobalWeights(ranges);
    } else {
      calcGridDistributonUniform(ranges);
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::calcGridDistributonUniform(ProcRanges &ranges) {
    // typedef Grid<double, 1> Weights;
    // typedef Weights::IndexType Index;
    // typedef Range<size_t, rank - 1> Orth;

    const LimitType lo = this->globalRange.getLo();
    const LimitType hi = this->globalRange.getHi();
    const LimitType dm = hi - lo + 1;

    for (size_t d = 0; d < rank; ++d) {
      // finding cut points in the cumulative weights
      Grid<Range<ptrdiff_t, 1>, 1> &dimRanges = ranges[d];

      dimRanges.resize(0, dims[d] - 1);
      dimRanges(0).getLo()[0] = lo[d];
      dimRanges(dims[d] - 1).getHi()[0] = hi[d];

      for (ptrdiff_t i = 1; i < dims[d]; ++i) {
        int cut = lo[d] + (long(i) * long(dm[d])) / dims[d];
        dimRanges(i - 1).getHi()[0] = cut - 1;
        dimRanges(i).getLo()[0] = cut;
      }
    }
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

        //      std::cout << "Sum: " << pos[0] << " " << pos[1] << " " << globalWeights[pos] << std::endl;

        sumTotal += globalWeights[pos];
      }

      weights(i) = sumTotal;
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::calcGridDistributonGlobalWeights(ProcRanges &ranges) {
    typedef Grid<double, 1> Weights;
    typedef Weights::IndexType Index;

    if (master()) {
      LimitType lo = this->globalWeights.getLo();
      LimitType hi = this->globalWeights.getHi();
      LimitType glo = this->globalRange.getLo();
      LimitType ghi = this->globalRange.getHi();

      for (size_t d = 0; d < rank; ++d) {
        int resolution = (ghi[d] - glo[d] + 1) / (hi[d] - lo[d] + 1);
        Weights weights(Index(lo[d] - 1), hi[d]);
        double sumTotal;
        if constexpr (rank == 1) {
          sumGlobalWeights<CheckingPolicy>(this->globalWeights, lo, hi, d, weights, sumTotal);
        } else {
          sumGlobalWeights<rank, CheckingPolicy>(this->globalWeights, lo, hi, d, weights, sumTotal);
        }

        // normalised cumulative sum
        for (int i = lo[d]; i <= hi[d]; ++i) {
          weights(i) = weights(i) / sumTotal;
        }

        // finding cut points in the cumulative weights
        Grid<Range<ptrdiff_t, 1>, 1> &dimRanges = ranges[d];
        dimRanges.resize(0, dims[d] - 1);
        dimRanges(0).getLo()[0] = glo[d];
        dimRanges(dims[d] - 1).getHi()[0] = ghi[d];

        double delta = 1.0 / double(dims[d]);
        for (ptrdiff_t i = 1; i < dims[d]; ++i) {
          int ins = findInsertIndex(weights, i * delta);
          if ((ins <= lo[d]) || (ins + 1 >= hi[d])) {
            for (int k = lo[d] - 1; k <= hi[d]; ++k) {
              std::cout << "w: " << k << " " << weights(k) << std::endl;
            }
            std::cout << "FAIL: " << d << " " << dims[d] << " " << delta << " " << i * delta << std::endl;
            std::cout << "      " << lo[d] - 1 << " " << hi[d] << " " << ins << std::endl;

            SCHNECK_FAIL("Invalid range when attempting to balance load: dim=" << i);
          }

          int cut = glo[d] + resolution * (ins - lo[d] + 1);

          dimRanges(i - 1).getHi()[0] = cut - 1;
          dimRanges(i).getLo()[0] = cut;
        }

        // broadcasting the layout in dimRanges to other processes
        std::vector<int> transfer(2 * dims[d]);
        for (ptrdiff_t i = 0; i < dims[d]; ++i) {
          transfer[2 * i] = dimRanges(i).getLo()[0];
          transfer[2 * i + 1] = dimRanges(i).getHi()[0];
        }
        mpi.MPI_Bcast(transfer.data(), 2 * dims[d], MPI_INT, 0, comm);
      }
    } else {
      // receiving dim ranges for each dimension from the master process
      for (size_t d = 0; d < rank; ++d) {
        Grid<Range<ptrdiff_t, 1>, 1> &dimRanges = ranges[d];
        dimRanges.resize(0, dims[d] - 1);

        std::vector<int> transfer(2 * dims[d]);
        mpi.MPI_Bcast(transfer.data(), 2 * dims[d], MPI_INT, 0, comm);
        for (ptrdiff_t i = 0; i < dims[d]; ++i) {
          dimRanges(i).getLo()[0] = transfer[2 * i];
          dimRanges(i).getHi()[0] = transfer[2 * i + 1];
        }
      }
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void
  MpiCartesianDomainDecomposition<rank, CheckingPolicy>::calcGridDistributonLocalWeights(ProcRanges & /* ranges */) {}

  template<size_t rank, template<size_t> class CheckingPolicy>
  class MpiCartesianDomainDecomposition<rank, CheckingPolicy>::ExchangeVisitor
      : public internal::GridVisitor<typename MpiCartesianDomainDecomposition<rank, CheckingPolicy>::ExchangeVisitor> {
    public:
      explicit ExchangeVisitor(MpiCartesianDomainDecomposition &parentIn) : parent(parentIn) {}

      template<typename GridType>
      void handle(GridType &grid, bool flag) {
        parent.template exchangeTyped<GridType>(grid, flag);
      }

    private:
      MpiCartesianDomainDecomposition &parent;
  };

  template<size_t rank, template<size_t> class CheckingPolicy>
  class MpiCartesianDomainDecomposition<rank, CheckingPolicy>::AccumulateVisitor
      : public internal::GridVisitor<
            typename MpiCartesianDomainDecomposition<rank, CheckingPolicy>::AccumulateVisitor> {
    public:
      explicit AccumulateVisitor(MpiCartesianDomainDecomposition &parentIn) : parent(parentIn) {}

      template<typename GridType>
      void handle(GridType &grid, bool flag) {
        parent.template accumulateTyped<GridType>(grid, flag);
      }

    private:
      MpiCartesianDomainDecomposition &parent;
  };

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<class GridType>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::registerExchangeHandler() {
    exchangeInitializers.emplace_back([](ExchangeVisitor &visitor) { visitor.template registerHandler<GridType>(); });
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<class GridType>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::registerAccumulateHandler() {
    accumulateInitializers.emplace_back([](AccumulateVisitor &visitor) { visitor.template registerHandler<GridType>(); }
    );
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  class MpiCartesianDomainDecomposition<rank, CheckingPolicy>::RedistributeVisitor
      : public internal::GridVisitor<
            typename MpiCartesianDomainDecomposition<rank, CheckingPolicy>::RedistributeVisitor> {
    public:
      RedistributeVisitor(
          MpiCartesianDomainDecomposition &parentIn,
          internal::pGridWrapper newWrapperIn,
          const std::vector<TransferBlock<rank, CheckingPolicy>> &sendPlanIn,
          const std::vector<TransferBlock<rank, CheckingPolicy>> &recvPlanIn
      )
          : parent(parentIn), newWrapper(std::move(newWrapperIn)), sendPlan(sendPlanIn), recvPlan(recvPlanIn) {}

      template<typename GridType>
      void handle(GridType &oldGrid, bool /*flag*/) {
        auto typedNewWrapper = std::dynamic_pointer_cast<internal::GridWrapperImpl<GridType>>(newWrapper);
        SCHNEK_ASSERT(typedNewWrapper, "Grid type mismatch during redistribution");
        parent.template redistributeTyped<GridType>(oldGrid, typedNewWrapper->grid, sendPlan, recvPlan);
      }

    private:
      MpiCartesianDomainDecomposition &parent;
      internal::pGridWrapper newWrapper;
      const std::vector<TransferBlock<rank, CheckingPolicy>> &sendPlan;
      const std::vector<TransferBlock<rank, CheckingPolicy>> &recvPlan;
  };

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<class GridType>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::registerRedistributeHandler() {
    redistributeInitializers.emplace_back([](RedistributeVisitor &visitor) {
      visitor.template registerHandler<GridType>();
    });
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::redistributeGrid(
      const internal::pGridWrapper &oldWrapper,
      const internal::pGridWrapper &newWrapper,
      const std::vector<TransferBlock<rank, CheckingPolicy>> &sendPlan,
      const std::vector<TransferBlock<rank, CheckingPolicy>> &recvPlan
  ) {
    if (redistributeInitializers.empty()) {
      SCHNECK_FAIL("No registered grids available for redistribution");
    }

    RedistributeVisitor visitor(*this, newWrapper, sendPlan, recvPlan);
    for (auto &initializer : redistributeInitializers) {
      initializer(visitor);
    }

    oldWrapper->accept(visitor, false);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename GridType>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::redistributeTyped(
      GridType &oldGrid,
      GridType &newGrid,
      const std::vector<TransferBlock<rank, CheckingPolicy>> &sendPlan,
      const std::vector<TransferBlock<rank, CheckingPolicy>> &recvPlan
  ) {
    using ValueType = typename GridType::value_type;
    using RangeTypeLocal = typename GridType::RangeType;

    const MPI_Datatype mpiType = detail::mpiDatatypeFor<ValueType>();

    auto gridRangeVolume = [](const RangeTypeLocal &range) -> size_t {
      size_t volume = 1;
      for (size_t d = 0; d < GridType::Rank; ++d) {
        ptrdiff_t extent = range.getHi()[d] - range.getLo()[d] + 1;
        if (extent <= 0) {
          return 0;
        }
        volume *= static_cast<size_t>(extent);
      }
      return volume;
    };

    auto toIntCount = [](size_t count) -> int {
      if (count > static_cast<size_t>(std::numeric_limits<int>::max())) {
        SCHNECK_FAIL("Redistribution block too large for MPI count");
      }
      return static_cast<int>(count);
    };

    // Separate local copy blocks from remote transfers
    int localRank = ComRank;

    // Post non-blocking receives for remote blocks
    std::vector<std::vector<ValueType>> recvBuffers;
    std::vector<MPI_Request> requests;

    for (size_t i = 0; i < recvPlan.size(); ++i) {
      if (recvPlan[i].mpiRank == localRank) {
        continue;  // local copy handled separately
      }

      RangeTypeLocal recvRange(recvPlan[i].range);
      size_t volume = gridRangeVolume(recvRange);
      if (volume == 0) {
        continue;
      }

      recvBuffers.emplace_back(volume);
      MPI_Request req;
      int errorCode =
          mpi.MPI_Irecv(recvBuffers.back().data(), toIntCount(volume), mpiType, recvPlan[i].mpiRank, 0, comm, &req);
      SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Irecv failed during redistribution");
      requests.push_back(req);
    }

    // Pack and post non-blocking sends for remote blocks
    std::vector<std::vector<ValueType>> sendBuffers;

    for (size_t i = 0; i < sendPlan.size(); ++i) {
      if (sendPlan[i].mpiRank == localRank) {
        continue;  // local copy handled separately
      }

      RangeTypeLocal sendRange(sendPlan[i].range);
      size_t volume = gridRangeVolume(sendRange);
      if (volume == 0) {
        continue;
      }

      // Try to use the raw data pointer if the range is the entire grid
      // Otherwise, pack into a temporary buffer
      sendBuffers.emplace_back(volume);
      auto &buffer = sendBuffers.back();
      size_t idx = 0;
      for (auto it = sendRange.begin(); it != sendRange.end(); ++it) {
        buffer[idx++] = oldGrid[*it];
      }

      MPI_Request req;
      int errorCode = mpi.MPI_Isend(buffer.data(), toIntCount(volume), mpiType, sendPlan[i].mpiRank, 0, comm, &req);
      SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Isend failed during redistribution");
      requests.push_back(req);
    }

    // Copy local data (blocks where source == destination == this process)
    for (size_t i = 0; i < recvPlan.size(); ++i) {
      if (recvPlan[i].mpiRank != localRank) {
        continue;
      }

      RangeTypeLocal copyRange(recvPlan[i].range);
      for (auto it = copyRange.begin(); it != copyRange.end(); ++it) {
        newGrid[*it] = oldGrid[*it];
      }
    }

    // Wait for all non-blocking operations to complete
    if (!requests.empty()) {
      int errorCode = mpi.MPI_Waitall(static_cast<int>(requests.size()), requests.data(), MPI_STATUSES_IGNORE);
      SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Waitall failed during redistribution");
    }

    // Unpack received data into the new grid
    size_t bufferIdx = 0;
    for (size_t i = 0; i < recvPlan.size(); ++i) {
      if (recvPlan[i].mpiRank == localRank) {
        continue;
      }

      RangeTypeLocal recvRange(recvPlan[i].range);
      size_t volume = gridRangeVolume(recvRange);
      if (volume == 0) {
        continue;
      }

      const auto &buffer = recvBuffers[bufferIdx++];
      size_t idx = 0;
      for (auto it = recvRange.begin(); it != recvRange.end(); ++it) {
        newGrid[*it] = buffer[idx++];
      }
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::exchangeGrid(
      const internal::pGridWrapper &wrapper, bool useFieldInfo
  ) {
    if (exchangeInitializers.empty()) {
      SCHNECK_FAIL("No registered grids available for exchange");
    }

    ExchangeVisitor visitor(*this);
    for (auto &initializer : exchangeInitializers) {
      initializer(visitor);
    }

    wrapper->accept(visitor, useFieldInfo);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::accumulate(
      const internal::pGridWrapper &wrapper, bool useFieldInfo
  ) {
    if (accumulateInitializers.empty()) {
      SCHNECK_FAIL("No registered grids available for accumulate");
    }

    AccumulateVisitor visitor(*this);
    for (auto &initializer : accumulateInitializers) {
      initializer(visitor);
    }

    wrapper->accept(visitor, useFieldInfo);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename GridType>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::exchangeTyped(GridType &grid, bool useFieldInfo) {
    if constexpr (internal::is_field_v<GridType>) {
      handleFieldExchange(grid, useFieldInfo);
    } else {
      handleGridExchange(grid, useFieldInfo);
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename GridType>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::accumulateTyped(GridType &grid, bool useFieldInfo) {
    if constexpr (internal::is_field_v<GridType>) {
      handleFieldAccumulate(grid, useFieldInfo);
    } else {
      handleGridAccumulate(grid, useFieldInfo);
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  typename MpiCartesianDomainDecomposition<rank, CheckingPolicy>::RangeType
  MpiCartesianDomainDecomposition<rank, CheckingPolicy>::getLocalInnerRange() const {
    typename RangeType::LimitType lo = this->globalRange.getLo();
    typename RangeType::LimitType hi = this->globalRange.getHi();
    for (size_t d = 0; d < rank; ++d) {
      const auto dimRange = procRanges[d](myCoord[d]);
      lo[d] = dimRange.getLo()[0];
      hi[d] = dimRange.getHi()[0];
    }
    return RangeType(lo, hi);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename GridType>
  void
  MpiCartesianDomainDecomposition<rank, CheckingPolicy>::handleGridExchange(GridType &grid, bool /*useFieldInfo*/) {
    const auto localRange = getLocalInnerRange();
    typename GridType::IndexType innerLo(localRange.getLo());
    typename GridType::IndexType innerHi(localRange.getHi());
    exchangeWithInteriorBounds(grid, innerLo, innerHi);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename GridType>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::handleGridAccumulate(
      GridType &grid, bool /*useFieldInfo*/
  ) {
    const auto localRange = getLocalInnerRange();
    typename GridType::IndexType innerLo(localRange.getLo());
    typename GridType::IndexType innerHi(localRange.getHi());
    accumulateWithInteriorBounds(grid, innerLo, innerHi);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename FieldType>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::handleFieldExchange(FieldType &field, bool useFieldInfo) {
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
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::handleFieldAccumulate(
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
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::exchangeWithInteriorBounds(
      GridType &grid, const typename GridType::IndexType &innerLo, const typename GridType::IndexType &innerHi
  ) {
    using IndexType = typename GridType::IndexType;
    using RangeTypeLocal = typename GridType::RangeType;
    using ValueType = typename GridType::value_type;

    const IndexType gridLo = grid.getLo();
    const IndexType gridHi = grid.getHi();

    auto makeRange = [](const IndexType &loIdx, const IndexType &hiIdx) { return RangeTypeLocal(loIdx, hiIdx); };

    auto rangeVolume = [](const RangeTypeLocal &range) -> size_t {
      size_t volume = 1;
      for (size_t d = 0; d < rank; ++d) {
        ptrdiff_t extent = range.getHi()[d] - range.getLo()[d] + 1;
        if (extent <= 0) {
          return 0;
        }
        volume *= static_cast<size_t>(extent);
      }
      return volume;
    };

    auto packRange = [&](RangeTypeLocal range, std::vector<ValueType> &buffer) {
      const size_t volume = rangeVolume(range);
      buffer.resize(volume);
      size_t idx = 0;
      for (auto it = range.begin(); it != range.end(); ++it) {
        buffer[idx++] = grid[*it];
      }
    };

    auto unpackRange = [&](RangeTypeLocal range, const std::vector<ValueType> &buffer) {
      size_t idx = 0;
      for (auto it = range.begin(); it != range.end(); ++it) {
        grid[*it] = buffer[idx++];
      }
    };

    auto toIntCount = [](size_t count) -> int {
      if (count > static_cast<size_t>(std::numeric_limits<int>::max())) {
        SCHNECK_FAIL("Halo exchange block too large for MPI_Sendrecv count");
      }
      return static_cast<int>(count);
    };

    const MPI_Datatype mpiType = detail::mpiDatatypeFor<ValueType>();

    for (size_t dim = 0; dim < rank; ++dim) {
      int prevRank = MPI_PROC_NULL;
      int nextRank = MPI_PROC_NULL;
      this->mpi.MPI_Cart_shift(comm, static_cast<int>(dim), 1, &prevRank, &nextRank);

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

      // Exchange to fill lower ghost cells (receive from prev, send to next)
      size_t sendLowerCount = hiSourceRange ? rangeVolume(*hiSourceRange) : 0;
      size_t recvLowerCount = loGhostRange ? rangeVolume(*loGhostRange) : 0;
      std::vector<ValueType> sendLowerBuffer;
      if (hiSourceRange && sendLowerCount > 0) {
        packRange(*hiSourceRange, sendLowerBuffer);
      }
      std::vector<ValueType> recvLowerBuffer(recvLowerCount);
      this->mpi.MPI_Sendrecv(
          sendLowerCount > 0 ? sendLowerBuffer.data() : nullptr, toIntCount(sendLowerCount), mpiType, nextRank, 0,
          recvLowerCount > 0 ? recvLowerBuffer.data() : nullptr, toIntCount(recvLowerCount), mpiType, prevRank, 0, comm,
          MPI_STATUS_IGNORE
      );
      if (loGhostRange && recvLowerCount > 0) {
        unpackRange(*loGhostRange, recvLowerBuffer);
      }

      // Exchange to fill upper ghost cells (receive from next, send to prev)
      size_t sendUpperCount = loSourceRange ? rangeVolume(*loSourceRange) : 0;
      size_t recvUpperCount = hiGhostRange ? rangeVolume(*hiGhostRange) : 0;
      std::vector<ValueType> sendUpperBuffer;
      if (loSourceRange && sendUpperCount > 0) {
        packRange(*loSourceRange, sendUpperBuffer);
      }
      std::vector<ValueType> recvUpperBuffer(recvUpperCount);
      this->mpi.MPI_Sendrecv(
          sendUpperCount > 0 ? sendUpperBuffer.data() : nullptr, toIntCount(sendUpperCount), mpiType, prevRank, 0,
          recvUpperCount > 0 ? recvUpperBuffer.data() : nullptr, toIntCount(recvUpperCount), mpiType, nextRank, 0, comm,
          MPI_STATUS_IGNORE
      );
      if (hiGhostRange && recvUpperCount > 0) {
        unpackRange(*hiGhostRange, recvUpperBuffer);
      }
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename GridType>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::accumulateWithInteriorBounds(
      GridType &grid, const typename GridType::IndexType &innerLo, const typename GridType::IndexType &innerHi
  ) {
    using IndexType = typename GridType::IndexType;
    using RangeTypeLocal = typename GridType::RangeType;
    using ValueType = typename GridType::value_type;

    const IndexType gridLo = grid.getLo();
    const IndexType gridHi = grid.getHi();

    auto makeRange = [](const IndexType &loIdx, const IndexType &hiIdx) { return RangeTypeLocal(loIdx, hiIdx); };

    auto rangeVolume = [](const RangeTypeLocal &range) -> size_t {
      size_t volume = 1;
      for (size_t d = 0; d < rank; ++d) {
        ptrdiff_t extent = range.getHi()[d] - range.getLo()[d] + 1;
        if (extent <= 0) {
          return 0;
        }
        volume *= static_cast<size_t>(extent);
      }
      return volume;
    };

    auto packRange = [&](RangeTypeLocal range, std::vector<ValueType> &buffer) {
      const size_t volume = rangeVolume(range);
      buffer.resize(volume);
      size_t idx = 0;
      for (auto it = range.begin(); it != range.end(); ++it) {
        buffer[idx++] = grid[*it];
      }
    };

    auto assignRange = [&](RangeTypeLocal range, const std::vector<ValueType> &buffer) {
      size_t idx = 0;
      for (auto it = range.begin(); it != range.end(); ++it) {
        grid[*it] = buffer[idx++];
      }
    };

    auto addIntoRange = [&](RangeTypeLocal range, const std::vector<ValueType> &buffer, std::vector<ValueType> &out) {
      const size_t volume = rangeVolume(range);
      out.resize(volume);
      size_t idx = 0;
      for (auto it = range.begin(); it != range.end(); ++it) {
        ValueType &val = grid[*it];
        val = val + buffer[idx];
        out[idx] = val;
        ++idx;
      }
    };

    auto toIntCount = [](size_t count) -> int {
      if (count > static_cast<size_t>(std::numeric_limits<int>::max())) {
        SCHNECK_FAIL("Halo accumulate block too large for MPI_Sendrecv count");
      }
      return static_cast<int>(count);
    };

    const MPI_Datatype mpiType = detail::mpiDatatypeFor<ValueType>();

    for (size_t dim = 0; dim < rank; ++dim) {
      int prevRank = MPI_PROC_NULL;
      int nextRank = MPI_PROC_NULL;
      this->mpi.MPI_Cart_shift(comm, static_cast<int>(dim), 1, &prevRank, &nextRank);

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

      // Lower side: add incoming data to lower ghost cells, then return result to update upper inner cells in prev.
      const size_t sendLowerCount = hiSourceRange ? rangeVolume(*hiSourceRange) : 0;
      const size_t recvLowerCount = loGhostRange ? rangeVolume(*loGhostRange) : 0;

      std::vector<ValueType> sendLowerBuffer;
      if (hiSourceRange && sendLowerCount > 0) {
        packRange(*hiSourceRange, sendLowerBuffer);
      }
      std::vector<ValueType> recvLowerBuffer(recvLowerCount);
      this->mpi.MPI_Sendrecv(
          sendLowerCount > 0 ? sendLowerBuffer.data() : nullptr, toIntCount(sendLowerCount), mpiType, nextRank, 0,
          recvLowerCount > 0 ? recvLowerBuffer.data() : nullptr, toIntCount(recvLowerCount), mpiType, prevRank, 0, comm,
          MPI_STATUS_IGNORE
      );

      std::vector<ValueType> sendBackLowerBuffer;
      if (loGhostRange && recvLowerCount > 0) {
        addIntoRange(*loGhostRange, recvLowerBuffer, sendBackLowerBuffer);
      }

      std::vector<ValueType> recvBackLowerBuffer(sendLowerCount);
      this->mpi.MPI_Sendrecv(
          sendBackLowerBuffer.empty() ? nullptr : sendBackLowerBuffer.data(), toIntCount(recvLowerCount), mpiType,
          prevRank, 0, sendLowerCount > 0 ? recvBackLowerBuffer.data() : nullptr, toIntCount(sendLowerCount), mpiType,
          nextRank, 0, comm, MPI_STATUS_IGNORE
      );
      if (hiSourceRange && sendLowerCount > 0) {
        assignRange(*hiSourceRange, recvBackLowerBuffer);
      }

      // Upper side: add incoming data to upper ghost cells, then return result to update lower inner cells in next.
      const size_t sendUpperCount = loSourceRange ? rangeVolume(*loSourceRange) : 0;
      const size_t recvUpperCount = hiGhostRange ? rangeVolume(*hiGhostRange) : 0;

      std::vector<ValueType> sendUpperBuffer;
      if (loSourceRange && sendUpperCount > 0) {
        packRange(*loSourceRange, sendUpperBuffer);
      }
      std::vector<ValueType> recvUpperBuffer(recvUpperCount);
      this->mpi.MPI_Sendrecv(
          sendUpperCount > 0 ? sendUpperBuffer.data() : nullptr, toIntCount(sendUpperCount), mpiType, prevRank, 0,
          recvUpperCount > 0 ? recvUpperBuffer.data() : nullptr, toIntCount(recvUpperCount), mpiType, nextRank, 0, comm,
          MPI_STATUS_IGNORE
      );

      std::vector<ValueType> sendBackUpperBuffer;
      if (hiGhostRange && recvUpperCount > 0) {
        addIntoRange(*hiGhostRange, recvUpperBuffer, sendBackUpperBuffer);
      }

      std::vector<ValueType> recvBackUpperBuffer(sendUpperCount);
      this->mpi.MPI_Sendrecv(
          sendBackUpperBuffer.empty() ? nullptr : sendBackUpperBuffer.data(), toIntCount(recvUpperCount), mpiType,
          nextRank, 0, sendUpperCount > 0 ? recvBackUpperBuffer.data() : nullptr, toIntCount(sendUpperCount), mpiType,
          prevRank, 0, comm, MPI_STATUS_IGNORE
      );
      if (loSourceRange && sendUpperCount > 0) {
        assignRange(*loSourceRange, recvBackUpperBuffer);
      }
    }
  }

#undef SCHNEK_LOGLEVEL
#define SCHNEK_LOGLEVEL 0

}  // namespace schnek
