/*
 * mpi_cartesian_decomposition.t
 *
 *  Created on: 30 Apr 2020
 *  Author: Holger Schmitz (holger@notjustphysics.com)
 */

#include <mpi-ext.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

#include "../diagnostic/diagnostic.hpp"
#include "../util/array_io.hpp"
#include "../util/exceptions.hpp"
#include "../util/factor.hpp"
#include "../util/logger.hpp"
#include "../util/volume.hpp"
#include "detail/grid_transfer.hpp"
#include "detail/halo_ranges.hpp"
#include "detail/particle_wrap.hpp"
#include "detail/redistribution.hpp"
#include "detail/weighted_partition.hpp"
#include "mpi_cartesian_decomposition.hpp"

#undef SCHNEK_LOGLEVEL
#define SCHNEK_LOGLEVEL 0

namespace schnek {

  namespace internal {
    inline void fillRangesFromCuts(
        Grid<Range<ptrdiff_t, 1>, 1> &dimRanges, ptrdiff_t lo, ptrdiff_t hi, const std::vector<ptrdiff_t> &cuts
    ) {
      dimRanges.resize(0, static_cast<ptrdiff_t>(cuts.size()));
      dimRanges(0).getLo()[0] = lo;
      dimRanges(static_cast<ptrdiff_t>(cuts.size())).getHi()[0] = hi;

      for (size_t i = 0; i < cuts.size(); ++i) {
        dimRanges(static_cast<ptrdiff_t>(i)).getHi()[0] = cuts[i] - 1;
        dimRanges(static_cast<ptrdiff_t>(i + 1)).getLo()[0] = cuts[i];
      }
    }

    inline int toIntCount(std::size_t count, const char *errorMessage, const char *suffix = "") {
      if (count > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        SCHNECK_FAIL(errorMessage << suffix);
      }
      return static_cast<int>(count);
    }

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
      } else if constexpr (std::is_same_v<T, long long int>) {
        return MPI_LONG_LONG_INT;
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
  }  // namespace internal

  template<size_t rank, template<size_t> class CheckingPolicy>
  MpiCartesianDomainDecomposition<rank, CheckingPolicy>::MpiCartesianDomainDecomposition(MpiContext &mpi) : mpi(mpi) {}

  template<size_t rank, template<size_t> class CheckingPolicy>
  bool MpiCartesianDomainDecomposition<rank, CheckingPolicy>::master() const {
    SCHNEK_ASSERT(initialised, "MpiCartesianDomainDecomposition has not been initialised");
    return ComRank == 0;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  int MpiCartesianDomainDecomposition<rank, CheckingPolicy>::numProcs() const {
    SCHNEK_ASSERT(initialised, "MpiCartesianDomainDecomposition has not been initialised");
    return ComSize;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename T>
  T MpiCartesianDomainDecomposition<rank, CheckingPolicy>::allReduce(T value, MPI_Op op) const {
    SCHNEK_ASSERT(initialised, "MpiCartesianDomainDecomposition has not been initialised");
    T result{};
    int errorCode = mpi.MPI_Allreduce(&value, &result, 1, internal::mpiDatatypeFor<T>(), op, comm);
    SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Allreduce failed");
    return result;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename T>
  T MpiCartesianDomainDecomposition<rank, CheckingPolicy>::avgReduceInteger(T value) const {
    SCHNEK_ASSERT(initialised, "MpiCartesianDomainDecomposition has not been initialised");
    SCHNEK_ASSERT(ComSize > 0, "MPI communicator size is zero");

    const T processCount = static_cast<T>(ComSize);
    const long localQuotient = static_cast<long>(value / processCount);
    const long long localRemainder = static_cast<long long>(value % processCount);

    // Reducing quotient and remainder separately avoids overflowing a direct
    // sum of long values. Each quotient's magnitude is at most |value| /
    // ComSize, so their sum fits in long. The remainders are smaller than
    // ComSize and there are at most INT_MAX ranks, so their sum fits in long long.
    const long quotientSum = allReduce(localQuotient, MPI_SUM);
    const long long remainderSum = allReduce(localRemainder, MPI_SUM);

    long result = quotientSum + static_cast<long>(remainderSum / ComSize);
    const long long fractionalRemainder = remainderSum % ComSize;
    if (fractionalRemainder > 0 && result < 0) {
      ++result;
    } else if (fractionalRemainder < 0 && result > 0) {
      --result;
    }

    return static_cast<T>(result);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  double MpiCartesianDomainDecomposition<rank, CheckingPolicy>::avgReduce(double value) const {
    SCHNEK_ASSERT(initialised, "MpiCartesianDomainDecomposition has not been initialised");
    SCHNEK_ASSERT(ComSize > 0, "MPI communicator size is zero");
    return allReduce(value, MPI_SUM) / double(ComSize);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  int MpiCartesianDomainDecomposition<rank, CheckingPolicy>::avgReduce(int value) const {
    return avgReduceInteger(value);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  long MpiCartesianDomainDecomposition<rank, CheckingPolicy>::avgReduce(long value) const {
    return avgReduceInteger(value);
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
  const std::array<Grid<Range<ptrdiff_t, 1>, 1>, rank> &
  MpiCartesianDomainDecomposition<rank, CheckingPolicy>::getProcRanges() {
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
          entry.second.size() <= 1, "MpiCartesianDomainDecomposition: registration "
                                        << entry.first << " holds " << entry.second.size()
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
    validateProcRanges(newRanges);

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
    std::map<long, std::vector<internal::pGridWrapper>> oldGrids;
    for (auto &entry : gridStorage) {
      oldGrids[entry.first] = std::move(entry.second);
    }

    // 5b. Save old projected grid wrappers (shared ownership via shared_ptr copy).
    // This must be done before clearLocalRanges() resets the union state and
    // clears the projected grid lists.
    auto &projectedStorage = this->getProjectedGridStorage();
    std::map<long, std::vector<internal::pGridWrapper>> oldProjectedGrids;
    for (const auto &entry : projectedStorage) {
      oldProjectedGrids[entry.first] = entry.second;
    }

    // 5c. Save old particle containers (take ownership). This must be done
    // before clearLocalRanges() destroys the per-range containers, otherwise
    // every particle would be lost when the new layout is created.
    auto &particleStorage = this->getParticleStorage();
    std::map<long, std::vector<internal::pParticleWrapper>> oldParticles;
    for (auto &entry : particleStorage) {
      oldParticles[entry.first] = std::move(entry.second);
    }

    // 6. Clear all local ranges and recreate with new layout.
    // clearLocalRanges() also resets the union state of every projected
    // registration so that addLocalRange() builds fresh grids for the new range.
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

      auto registrationSubRange = this->getRegisteredFieldSubRange(id);
      auto registrationSendPlan = sendPlan;
      auto registrationRecvPlan = recvPlan;
      if (registrationSubRange) {
        auto clipPlan = [&](std::vector<TransferBlockType> &plan) {
          std::vector<TransferBlockType> clippedPlan;
          clippedPlan.reserve(plan.size());
          for (const auto &block : plan) {
            RangeType clippedRange;
            if (rangeIntersection(block.range, *registrationSubRange, clippedRange)) {
              clippedPlan.push_back({block.mpiRank, clippedRange});
            }
          }
          plan = std::move(clippedPlan);
        };
        clipPlan(registrationSendPlan);
        clipPlan(registrationRecvPlan);
      }

      SCHNEK_ASSERT(
          !oldWrapper || !registrationSendPlan.empty(),
          "Sub-range registration has an old grid but no clipped send blocks"
      );
      SCHNEK_ASSERT(
          !newWrapper || !registrationRecvPlan.empty(),
          "Sub-range registration has a new grid but no clipped receive blocks"
      );
      if (oldWrapper || newWrapper) {
        redistributeGrid(oldWrapper, newWrapper, registrationSendPlan, registrationRecvPlan);
      }
    }

    // 7b. Redistribute each registered particle container using the same
    // Cartesian transfer plan. Fresh empty containers were created by
    // addLocalRange(); particles are routed from the saved old containers to
    // the process that owns them under the new layout.
    for (auto &entry : particleStorage) {
      long id = entry.first;
      auto &newContainerList = entry.second;
      auto oldParticleIt = oldParticles.find(id);

      if (oldParticleIt == oldParticles.end() || oldParticleIt->second.empty() || newContainerList.empty()) {
        continue;
      }

      auto oldWrapper = oldParticleIt->second.front();
      auto newWrapper = newContainerList.front();

      if (oldWrapper && newWrapper) {
        redistributeParticles(oldWrapper, newWrapper, sendPlan, recvPlan);
      }
    }

    // 8. Redistribute projected grids across processes.  Canonical replicas
    // perform a projected point-to-point exchange to compute the new contents,
    // then broadcast them to non-canonical replicas via an orthogonal
    // sub-communicator so all replicas of a projected grid agree.
    this->redistributeProjectedGrids(oldProjectedGrids, oldRanges, newRanges);

    // 9. Update proc ranges
    procRanges = newRanges;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  int MpiCartesianDomainDecomposition<rank, CheckingPolicy>::getUniqueId() const {
    SCHNEK_ASSERT(initialised, "MpiCartesianDomainDecomposition has not been initialised");
    int id = myCoord[0];
    for (size_t i = 1; i < rank; ++i) id = dims[i] * id + myCoord[i];

    SCHNEK_TRACE_LOG(2, "MpiCartesianDomainDecomposition::getUniqueId() " << id);
    return id;
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::init() {
    SCHNEK_ASSERT(!initAttempted, "MpiCartesianDomainDecomposition::init() may only be called once");
    initAttempted = true;
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
    validateProcRanges(procRanges);

    const LimitType &globalLo = this->globalRange.getLo();
    const LimitType &globalHi = this->globalRange.getHi();

    RangeType localRange;
    for (size_t d = 0; d < rank; ++d) {
      const Range<ptrdiff_t, 1> &dimRange = procRanges[d](myCoord[d]);
      localRange.getLo()[d] = dimRange.getLo()[0];
      localRange.getHi()[d] = dimRange.getHi()[0];
    }
    DomainType localDomain = this->computeLocalDomain(localRange);

    this->addLocalRange(localRange, localDomain);
    initialised = true;

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
    const LimitType lo = this->globalRange.getLo();
    const LimitType hi = this->globalRange.getHi();

    for (size_t d = 0; d < rank; ++d) {
      const ptrdiff_t processCount = static_cast<ptrdiff_t>(dims[d]);
      const __int128 cellCountWide =
          static_cast<__int128>(hi[d]) - static_cast<__int128>(lo[d]) + static_cast<__int128>(1);
      SCHNEK_ASSERT(
          cellCountWide > 0 && cellCountWide <= std::numeric_limits<ptrdiff_t>::max(),
          "Uniform partition cell count exceeds ptrdiff_t range in dimension " << d
      );
      const ptrdiff_t cellCount = static_cast<ptrdiff_t>(cellCountWide);
      if (cellCount < processCount) {
        SCHNECK_FAIL(
            "Cannot partition dimension " << d << ": " << cellCount << " cells for " << processCount << " processes"
        );
      }

      std::vector<ptrdiff_t> cuts;
      cuts.reserve(static_cast<size_t>(dims[d] - 1));

      for (ptrdiff_t i = 1; i < dims[d]; ++i) {
        // Keep the same balanced layout as the local-weight uniform fallback.
        const __int128 cutOffset =
            static_cast<__int128>(i) * static_cast<__int128>(cellCount) / static_cast<__int128>(dims[d]);
        const __int128 cutWide = static_cast<__int128>(lo[d]) + cutOffset;
        SCHNEK_ASSERT(
            cutWide >= std::numeric_limits<ptrdiff_t>::min() && cutWide <= std::numeric_limits<ptrdiff_t>::max(),
            "Uniform partition cut exceeds ptrdiff_t range"
        );
        const ptrdiff_t cut = static_cast<ptrdiff_t>(cutWide);
        cuts.push_back(cut);
      }

      internal::fillRangesFromCuts(ranges[d], lo[d], hi[d], cuts);
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::validateProcRanges(const ProcRanges &ranges) const {
    for (size_t d = 0; d < rank; ++d) {
      const auto &dimRanges = ranges[d];
      const ptrdiff_t processCount = static_cast<ptrdiff_t>(dims[d]);

      SCHNEK_ASSERT(
          processCount > 0 && dimRanges.getLo()[0] == 0 && dimRanges.getHi()[0] == processCount - 1,
          "Invalid process range table in dimension " << d
      );

      ptrdiff_t expectedLo = dimRanges(0).getLo()[0];
      for (ptrdiff_t c = 0; c < processCount; ++c) {
        const auto &range = dimRanges(c);
        const ptrdiff_t rangeLo = range.getLo()[0];
        const ptrdiff_t rangeHi = range.getHi()[0];
        const ptrdiff_t extent = rangeHi - rangeLo + 1;

        if (extent < 1 || rangeLo != expectedLo) {
          SCHNECK_FAIL(
              "Invalid process range in dimension " << d << ": process " << c << " has extent " << extent << " (range ["
                                                    << rangeLo << ", " << rangeHi
                                                    << "]); expected a non-empty contiguous partition"
          );
        }
        expectedLo = rangeHi + 1;
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

        sumTotal += globalWeights[pos];
      }

      weights(i) = sumTotal;
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::calcGridDistributonGlobalWeights(ProcRanges &ranges) {
    typedef Grid<double, 1> Weights;
    typedef Weights::IndexType Index;

    static_assert(sizeof(ptrdiff_t) == sizeof(long), "MPI_LONG must match ptrdiff_t for weighted cuts");

    std::vector<long> transfer;
    size_t transferSize = 0;
    for (size_t d = 0; d < rank; ++d) {
      transferSize += 2 * static_cast<size_t>(dims[d]);
    }
    transfer.resize(transferSize);

    int status = 1;
    std::string error;
    if (ComRank == 0) {
      size_t transferOffset = 0;
      LimitType lo = this->globalWeights.getLo();
      LimitType hi = this->globalWeights.getHi();
      LimitType glo = this->globalRange.getLo();
      LimitType ghi = this->globalRange.getHi();

      for (size_t d = 0; d < rank; ++d) {
        const ptrdiff_t resolution = (ghi[d] - glo[d] + 1) / (hi[d] - lo[d] + 1);
        Weights weights(Index(lo[d] - 1), hi[d]);
        double sumTotal;
        if constexpr (rank == 1) {
          sumGlobalWeights<CheckingPolicy>(this->globalWeights, lo, hi, d, weights, sumTotal);
        } else {
          sumGlobalWeights<rank, CheckingPolicy>(this->globalWeights, lo, hi, d, weights, sumTotal);
        }

        std::vector<ptrdiff_t> cuts;
        std::string dimensionError;
        if (!internal::computeWeightedCuts(weights, resolution, glo[d], ghi[d], dims[d], cuts, dimensionError)) {
          if (status != 0) {
            error = "dimension " + std::to_string(d) + ": " + dimensionError;
          }
          status = 0;
          continue;
        }

        internal::fillRangesFromCuts(ranges[d], glo[d], ghi[d], cuts);

        const auto &dimRanges = ranges[d];
        for (ptrdiff_t i = 0; i < dims[d]; ++i) {
          transfer[transferOffset++] = static_cast<long>(dimRanges(i).getLo()[0]);
          transfer[transferOffset++] = static_cast<long>(dimRanges(i).getHi()[0]);
        }
      }
    }

    int errorCode = mpi.MPI_Bcast(&status, 1, MPI_INT, 0, comm);
    SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Bcast failed while validating global-weight cuts");
    if (status == 0) {
      if (ComRank == 0) {
        SCHNECK_FAIL("Invalid range when attempting to balance load: " << error);
      }
      SCHNECK_FAIL("Invalid range when attempting to balance load from global weights");
    }

    size_t offset = 0;
    for (size_t d = 0; d < rank; ++d) {
      const size_t count = 2 * static_cast<size_t>(dims[d]);
      errorCode = mpi.MPI_Bcast(
          transfer.data() + offset,
          internal::toIntCount(count, "Global-weight process ranges exceed the MPI count limit"), MPI_LONG, 0, comm
      );
      SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Bcast failed while distributing global-weight ranges");

      std::vector<ptrdiff_t> cuts;
      cuts.reserve(static_cast<size_t>(dims[d] - 1));
      ptrdiff_t lo = 0;
      ptrdiff_t hi = 0;
      for (ptrdiff_t i = 0; i < dims[d]; ++i) {
        const ptrdiff_t rangeLo = static_cast<ptrdiff_t>(transfer[offset++]);
        hi = static_cast<ptrdiff_t>(transfer[offset++]);
        if (i == 0) {
          lo = rangeLo;
        } else {
          cuts.push_back(rangeLo);
        }
      }
      internal::fillRangesFromCuts(ranges[d], lo, hi, cuts);
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::calcGridDistributonLocalWeights(ProcRanges &ranges) {
    const LimitType glo = this->globalRange.getLo();
    const LimitType ghi = this->globalRange.getHi();

    auto flattenOrthogonalCoord = [this](size_t excludedDim) -> int {
      long color = 0;
      for (size_t d = 0; d < rank; ++d) {
        if (d == excludedDim) continue;

        color = color * static_cast<long>(dims[d]) + static_cast<long>(myCoord[d]);

        if (color > static_cast<long>(std::numeric_limits<int>::max())) {
          SCHNECK_FAIL("Cartesian communicator colour overflow");
        }
      }
      return static_cast<int>(color);
    };

    auto setUniformRangesForDim = [&](size_t d) {
      const ptrdiff_t nProcs = dims[d];
      const ptrdiff_t globalCells = ghi[d] - glo[d] + 1;

      SCHNEK_ASSERT(nProcs > 0, "Invalid Cartesian process-grid dimension");
      SCHNEK_ASSERT(globalCells >= nProcs, "Cannot assign non-empty ranges: more processes than cells");

      const ptrdiff_t q = globalCells / nProcs;
      const ptrdiff_t r = globalCells % nProcs;
      std::vector<ptrdiff_t> cuts;
      cuts.reserve(static_cast<size_t>(nProcs - 1));

      for (ptrdiff_t p = 1; p < nProcs; ++p) {
        const ptrdiff_t cut = glo[d] + p * q + (p * r) / nProcs;
        cuts.push_back(cut);
      }
      internal::fillRangesFromCuts(ranges[d], glo[d], ghi[d], cuts);
    };

    auto allreduceVectorSum = [&](const std::vector<double> &local, std::vector<double> &reduced, MPI_Comm c) {
      SCHNEK_ASSERT(local.size() == reduced.size(), "Mismatched reduction buffer sizes");

      size_t offset = 0;
      while (offset < local.size()) {
        const size_t remaining = local.size() - offset;
        const size_t chunkSize = std::min<size_t>(remaining, static_cast<size_t>(std::numeric_limits<int>::max()));

        const int count = static_cast<int>(chunkSize);
        int errorCode =
            mpi.MPI_Allreduce(local.data() + offset, reduced.data() + offset, count, MPI_DOUBLE, MPI_SUM, c);
        SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Allreduce failed while reducing local weight marginals");

        offset += chunkSize;
      }
    };

    // Current local range in global grid indices.
    LimitType localLo;
    LimitType localHi;
    for (size_t d = 0; d < rank; ++d) {
      localLo[d] = procRanges[d](myCoord[d]).getLo()[0];
      localHi[d] = procRanges[d](myCoord[d]).getHi()[0];
    }

    const auto weightsDims = this->localWeights.getDims();
    const LimitType weightsLo = this->localWeights.getLo();
    const LimitType weightsHi = this->localWeights.getHi();

    LimitType resolution;
    for (size_t d = 0; d < rank; ++d) {
      const ptrdiff_t localCells = localHi[d] - localLo[d] + 1;

      SCHNEK_ASSERT(weightsDims[d] > 0, "Local weights must be non-empty");
      SCHNEK_ASSERT(
          localCells % ptrdiff_t(weightsDims[d]) == 0, "Local weights dimensions must evenly divide the local range"
      );

      resolution[d] = localCells / ptrdiff_t(weightsDims[d]);

      SCHNEK_ASSERT(resolution[d] > 0, "Invalid local weight coarsening factor");
    }

    // Local per-axis marginals. This is the only storage proportional to the
    // local weight grid dimensions.
    Array<std::vector<double>, rank> localMarginals;
    for (size_t d = 0; d < rank; ++d) {
      localMarginals[d].assign(static_cast<size_t>(weightsDims[d]), 0.0);
    }

    int localWeightError = 0;
    {
      Range<ptrdiff_t, rank> wRange(weightsLo, weightsHi);

      for (const LimitType &p : wRange) {
        const double w = this->localWeights[p];

        if (!std::isfinite(w) || w < 0.0) {
          localWeightError = 1;
          continue;
        }

        for (size_t d = 0; d < rank; ++d) {
          localMarginals[d][static_cast<size_t>(p[d] - weightsLo[d])] += w;
        }
      }
    }

    int globalWeightError = 0;
    int errorCode = mpi.MPI_Allreduce(&localWeightError, &globalWeightError, 1, MPI_INT, MPI_MAX, comm);
    SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Allreduce failed while validating local weights");
    if (globalWeightError != 0) {
      SCHNECK_FAIL("Local weights must be finite and non-negative on every process");
    }

    // An exception on only some ranks during these collective sequences is fatal;
    // the application must MPI_Abort rather than attempt to recover. Scoped
    // communicators below only guarantee local cleanup during exception unwinding.
    for (size_t d = 0; d < rank; ++d) {
      const ptrdiff_t nProcs = dims[d];
      const ptrdiff_t globalCells = ghi[d] - glo[d] + 1;

      SCHNEK_ASSERT(nProcs > 0, "Invalid Cartesian process-grid dimension");
      SCHNEK_ASSERT(globalCells >= nProcs, "Cannot assign non-empty ranges: more processes than cells");

      if (nProcs == 1) {
        internal::fillRangesFromCuts(ranges[d], glo[d], ghi[d], {});
        continue;
      }

      // ---------------------------------------------------------------------
      // 1. Reduce this coordinate segment over the perpendicular Cartesian slice.
      //    All ranks with the same coordinate in dimension d participate.
      // ---------------------------------------------------------------------
      detail::ScopedComm sliceComm(mpi);

      {
        int remainDims[rank];
        for (size_t q = 0; q < rank; ++q) {
          remainDims[q] = (q == d) ? 0 : 1;
        }

        errorCode = mpi.MPI_Cart_sub(comm, remainDims, sliceComm.address());
        SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Cart_sub failed while creating local-weight slice communicator");
      }

      const long localBins = static_cast<long>(weightsDims[d]);

#if defined(SCHNEK_DEBUG) || defined(SCHNEK_TRACE)
      // Validate that all ranks in this perpendicular slice use the same number
      // of local bins in direction d.  The vector Allreduce below requires equal
      // counts on all participants.
      long minBins = 0;
      long maxBins = 0;

      {
        int errorCode =
            mpi.MPI_Allreduce(&localBins, &minBins, 1, internal::mpiDatatypeFor<long>(), MPI_MIN, sliceComm.get());
        SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Allreduce failed while validating local weight shape");

        errorCode =
            mpi.MPI_Allreduce(&localBins, &maxBins, 1, internal::mpiDatatypeFor<long>(), MPI_MAX, sliceComm.get());
        SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Allreduce failed while validating local weight shape");

        if (minBins != maxBins) {
          SCHNECK_FAIL(
              "Local weights must have the same extent in dimension "
              << d << " for all ranks sharing the same Cartesian coordinate in that dimension"
          );
        }
      }
#endif

      std::vector<double> reducedSegment(static_cast<size_t>(localBins), 0.0);
      allreduceVectorSum(localMarginals[d], reducedSegment, sliceComm.get());

      int errorCode = sliceComm.close();
      SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Comm_free failed for local-weight slice communicator");

      double segmentTotal = 0.0;
      for (double w : reducedSegment) {
        segmentTotal += w;
      }

      // ---------------------------------------------------------------------
      // 2. Build an axis communicator ordered by Cartesian coordinate d.
      //    MPI_Exscan now gives the cumulative weight before this segment.
      // ---------------------------------------------------------------------
      detail::ScopedComm axisComm(mpi);

      {
        const int color = flattenOrthogonalCoord(d);
        const int key = static_cast<int>(myCoord[d]);

        errorCode = mpi.MPI_Comm_split(comm, color, key, axisComm.address());
        SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Comm_split failed while creating local-weight axis communicator");
      }

      int axisRank = 0;
      errorCode = mpi.MPI_Comm_rank(axisComm.get(), &axisRank);
      SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Comm_rank failed on local-weight axis communicator");

      double segmentOffset = 0.0;
      errorCode = mpi.MPI_Exscan(&segmentTotal, &segmentOffset, 1, MPI_DOUBLE, MPI_SUM, axisComm.get());
      SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Exscan failed while scanning local-weight segments");

      if (axisRank == 0) {
        segmentOffset = 0.0;
      }

      double totalWeight = 0.0;
      errorCode = mpi.MPI_Allreduce(&segmentTotal, &totalWeight, 1, MPI_DOUBLE, MPI_SUM, axisComm.get());
      SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Allreduce failed while computing total local weight");

      if (!(totalWeight > 0.0)) {
        setUniformRangesForDim(d);

        errorCode = axisComm.close();
        SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Comm_free failed for local-weight axis communicator");

        continue;
      }

      // ---------------------------------------------------------------------
      // 3. Locate the weighted quantile cuts.  A rank contributes a cut only if
      //    the target cumulative weight lies inside its axis segment.
      // ---------------------------------------------------------------------
      const ptrdiff_t numCuts = nProcs - 1;
      const long noCut = std::numeric_limits<long>::min();

      std::vector<long> localCuts(static_cast<size_t>(numCuts), noCut);
      std::vector<long> globalCuts(static_cast<size_t>(numCuts), noCut);

      const double segmentEnd = segmentOffset + segmentTotal;

      for (ptrdiff_t cutIndex = 1; cutIndex < nProcs; ++cutIndex) {
        const double target = totalWeight * (double(cutIndex) / double(nProcs));

        // The strict lower bound makes cuts that fall exactly on a segment
        // boundary belong to the preceding segment, avoiding duplicate owners.
        if ((target > segmentOffset) && (target <= segmentEnd)) {
          double cumulative = segmentOffset;

          for (ptrdiff_t k = 0; k < localBins; ++k) {
            cumulative += reducedSegment[static_cast<size_t>(k)];

            if (target <= cumulative) {
              const ptrdiff_t cutCell = localLo[d] + resolution[d] * (k + 1);
              localCuts[static_cast<size_t>(cutIndex - 1)] = static_cast<long>(cutCell);
              break;
            }
          }
        }
      }

      errorCode = mpi.MPI_Allreduce(
          localCuts.data(), globalCuts.data(),
          internal::toIntCount(globalCuts.size(), "Number of local-weight cuts", " is too large for an MPI int count"),
          internal::mpiDatatypeFor<long>(), MPI_MAX, axisComm.get()
      );
      SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Allreduce failed while collecting local-weight cuts");

      errorCode = axisComm.close();
      SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Comm_free failed for local-weight axis communicator");

      // ---------------------------------------------------------------------
      // 4. Convert cuts into ProcRanges.  The clamp enforces non-empty ranges
      //    when globalCells >= nProcs, even for pathological weights concentrated
      //    in one bin.
      // ---------------------------------------------------------------------
      ptrdiff_t previousCut = glo[d];
      std::vector<ptrdiff_t> cuts;
      cuts.reserve(static_cast<size_t>(numCuts));

      for (ptrdiff_t cutIndex = 1; cutIndex < nProcs; ++cutIndex) {
        long rawCut = globalCuts[static_cast<size_t>(cutIndex - 1)];

        if (rawCut == noCut) {
          // Defensive fallback for numerical edge cases.  This should rarely be
          // used unless the target landed in a zero-weight plateau.
          const ptrdiff_t q = globalCells / nProcs;
          const ptrdiff_t r = globalCells % nProcs;
          rawCut = static_cast<long>(glo[d] + cutIndex * q + (cutIndex * r) / nProcs);
        }

        ptrdiff_t cut = static_cast<ptrdiff_t>(rawCut);

        const ptrdiff_t minCut = glo[d] + cutIndex;
        const ptrdiff_t maxCut = ghi[d] - (nProcs - cutIndex) + 1;

        cut = std::max(cut, minCut);
        cut = std::min(cut, maxCut);
        cut = std::max(cut, previousCut + 1);

        cuts.push_back(cut);
        previousCut = cut;
      }
      internal::fillRangesFromCuts(ranges[d], glo[d], ghi[d], cuts);
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  class MpiCartesianDomainDecomposition<rank, CheckingPolicy>::RedistributeVisitor
      : public internal::GridVisitor<
            typename MpiCartesianDomainDecomposition<rank, CheckingPolicy>::RedistributeVisitor> {
    public:
      RedistributeVisitor(
          MpiCartesianDomainDecomposition &parentIn,
          internal::pGridWrapper oldWrapperIn,
          internal::pGridWrapper newWrapperIn,
          const std::vector<TransferBlock<rank, CheckingPolicy>> &sendPlanIn,
          const std::vector<TransferBlock<rank, CheckingPolicy>> &recvPlanIn
      )
          : parent(parentIn),
            oldWrapper(std::move(oldWrapperIn)),
            newWrapper(std::move(newWrapperIn)),
            sendPlan(sendPlanIn),
            recvPlan(recvPlanIn) {}

      template<typename GridType>
      void handle(GridType & /*grid*/, bool /*flag*/) {
        auto typedOldWrapper = std::dynamic_pointer_cast<internal::GridWrapperImpl<GridType>>(oldWrapper);
        auto typedNewWrapper = std::dynamic_pointer_cast<internal::GridWrapperImpl<GridType>>(newWrapper);
        SCHNEK_ASSERT(!oldWrapper || typedOldWrapper, "Old grid type mismatch during redistribution");
        SCHNEK_ASSERT(!newWrapper || typedNewWrapper, "New grid type mismatch during redistribution");
        parent.template redistributeTyped<GridType>(
            typedOldWrapper ? &typedOldWrapper->grid : nullptr, typedNewWrapper ? &typedNewWrapper->grid : nullptr,
            sendPlan, recvPlan
        );
      }

    private:
      MpiCartesianDomainDecomposition &parent;
      internal::pGridWrapper oldWrapper;
      internal::pGridWrapper newWrapper;
      const std::vector<TransferBlock<rank, CheckingPolicy>> &sendPlan;
      const std::vector<TransferBlock<rank, CheckingPolicy>> &recvPlan;
  };

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<class GridType>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::registerRedistributeHandler() {
    redistributeInitializers.template addHandler<GridType>();
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

    if (!oldWrapper && !newWrapper) {
      SCHNEK_ASSERT(sendPlan.empty() && recvPlan.empty(), "Cannot redistribute non-empty plans without a grid");
      return;
    }

    RedistributeVisitor visitor(*this, oldWrapper, newWrapper, sendPlan, recvPlan);
    redistributeInitializers.initialize(visitor);

    (oldWrapper ? oldWrapper : newWrapper)->accept(visitor, false);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<
      typename GridType,
      typename PlanType,
      typename RangeBuilder,
      typename LocalCopy,
      typename SendScratch,
      typename RecvScratch>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::transferRedistributionSlabs(
      GridType *oldGrid,
      GridType *newGrid,
      const std::vector<RedistributionSlabBlock> &sendBlocks,
      const std::vector<RedistributionSlabBlock> &recvBlocks,
      const PlanType &sendPlan,
      const PlanType &recvPlan,
      std::size_t sendTotal,
      std::size_t recvTotal,
      SendScratch &sendScratch,
      RecvScratch &recvScratch,
      MPI_Datatype mpiType,
      int tag,
      const char *countErrorMessage,
      const char *operation,
      RangeBuilder makeRange,
      LocalCopy localCopy
  ) {
    using ValueType = typename GridType::value_type;

    sendScratch.reserve_bytes(detail::checkedCommunicationBufferBytes(sendTotal, sizeof(ValueType)));
    recvScratch.reserve_bytes(detail::checkedCommunicationBufferBytes(recvTotal, sizeof(ValueType)));

    auto sendSlab = sendScratch.template make_vector<ValueType>(sendTotal);
    auto recvSlab = recvScratch.template make_vector<ValueType>(recvTotal);
    std::vector<MPI_Request> requests;

    // Block vectors preserve plan order so same-source/tag messages match by MPI's non-overtaking rule.
    for (const auto &block : recvBlocks) {
      MPI_Request req;
      const int errorCode = mpi.MPI_Irecv(
          recvSlab.data() + block.offset, internal::toIntCount(block.volume, countErrorMessage), mpiType,
          recvPlan[block.planIndex].mpiRank, tag, comm, &req
      );
      SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Irecv failed during " << operation);
      requests.push_back(req);
    }

    for (const auto &block : sendBlocks) {
      SCHNEK_ASSERT(oldGrid != nullptr, "Send plan requires an old grid during redistribution");
      auto *blockData = sendSlab.data() + block.offset;
      detail::GridTransfer<GridType>::pack(*oldGrid, makeRange(sendPlan[block.planIndex]), blockData);

      // GridTransfer::pack fences device work before MPI reads this slab.
      MPI_Request req;
      const int errorCode = mpi.MPI_Isend(
          blockData, internal::toIntCount(block.volume, countErrorMessage), mpiType, sendPlan[block.planIndex].mpiRank,
          tag, comm, &req
      );
      SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Isend failed during " << operation);
      requests.push_back(req);
    }

    localCopy();

    if (!requests.empty()) {
      const int errorCode = mpi.MPI_Waitall(static_cast<int>(requests.size()), requests.data(), MPI_STATUSES_IGNORE);
      SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Waitall failed during " << operation);
    }

    for (const auto &block : recvBlocks) {
      SCHNEK_ASSERT(newGrid != nullptr, "Receive plan requires a new grid during redistribution");
      detail::GridTransfer<GridType>::unpack(
          *newGrid, makeRange(recvPlan[block.planIndex]), recvSlab.data() + block.offset
      );
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename GridType>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::redistributeTyped(
      GridType *oldGrid,
      GridType *newGrid,
      const std::vector<TransferBlock<rank, CheckingPolicy>> &sendPlan,
      const std::vector<TransferBlock<rank, CheckingPolicy>> &recvPlan
  ) {
    using ValueType = typename GridType::value_type;
    using RangeTypeLocal = typename GridType::RangeType;
    using MemorySpace = detail::GridBufferMemorySpace<GridType>;

    if constexpr (detail::GridCommunicationTraits<GridType>::device_resident) {
      static_assert(
          std::is_same<MemorySpace, typename GridType::storage_type::MemorySpace>::value,
          "Device grid communication buffers must use the grid storage memory space"
      );
    }

    const MPI_Datatype mpiType = internal::mpiDatatypeFor<ValueType>();

    // Separate local copy blocks from remote transfers
    const int localRank = ComRank;

    // Remote blocks are stored back to back in one slab per direction, in plan order.
    auto rangeVolume = [](const auto &block) { return schnek::volume(RangeTypeLocal(block.range)); };
    auto [recvBlocks, recvTotal] = makeRedistributionSlabBlocks(recvPlan, localRank, rangeVolume);
    auto [sendBlocks, sendTotal] = makeRedistributionSlabBlocks(sendPlan, localRank, rangeVolume);

    auto &bufferRegistry = mpiBufferRegistry.template get<MemorySpace>();
    auto &sendScratch = bufferRegistry.send();
    auto &recvScratch = bufferRegistry.receive();

    sendScratch.reset();
    recvScratch.reset();

    auto copyLocalBlocks = [&]() {
      // Copy local data (blocks where source == destination == this process).
      for (size_t i = 0; i < recvPlan.size(); ++i) {
        if (recvPlan[i].mpiRank != localRank) {
          continue;
        }

        RangeTypeLocal copyRange(recvPlan[i].range);
        SCHNEK_ASSERT(oldGrid != nullptr && newGrid != nullptr, "Local copy requires both grids");
        detail::GridTransfer<GridType>::copy(*oldGrid, *newGrid, copyRange);
      }
    };

    auto makeRange = [](const auto &block) { return RangeTypeLocal(block.range); };
    transferRedistributionSlabs(
        oldGrid, newGrid, sendBlocks, recvBlocks, sendPlan, recvPlan, sendTotal, recvTotal, sendScratch, recvScratch,
        mpiType, 0, "Redistribution block too large for MPI count", "redistribution", makeRange, copyLocalBlocks
    );

    sendScratch.reset();
    recvScratch.reset();
  }

  // ---------------------------------------------------------------------------
  // Projected-grid redistribution
  // ---------------------------------------------------------------------------

  template<size_t rank, template<size_t> class CheckingPolicy>
  class MpiCartesianDomainDecomposition<rank, CheckingPolicy>::RedistributeProjectedVisitor
      : public internal::GridVisitor<
            typename MpiCartesianDomainDecomposition<rank, CheckingPolicy>::RedistributeProjectedVisitor> {
    public:
      RedistributeProjectedVisitor(
          MpiCartesianDomainDecomposition &parentIn,
          internal::pGridWrapper newWrapperIn,
          MPI_Comm replicaCommIn,
          bool isCanonicalIn,
          const std::vector<DynProjTransferBlock> &sendPlanIn,
          const std::vector<DynProjTransferBlock> &recvPlanIn
      )
          : parent(parentIn),
            newWrapper(std::move(newWrapperIn)),
            replicaComm(replicaCommIn),
            isCanonical(isCanonicalIn),
            sendPlan(sendPlanIn),
            recvPlan(recvPlanIn) {}

      template<typename GridType>
      void handle(GridType &oldGrid, bool /*flag*/) {
        auto typedNewWrapper = std::dynamic_pointer_cast<internal::GridWrapperImpl<GridType>>(newWrapper);
        SCHNEK_ASSERT(typedNewWrapper, "Grid type mismatch during projected redistribution");
        parent.template redistributeProjectedTyped<GridType>(
            oldGrid, typedNewWrapper->grid, replicaComm, isCanonical, sendPlan, recvPlan
        );
      }

    private:
      MpiCartesianDomainDecomposition &parent;
      internal::pGridWrapper newWrapper;
      MPI_Comm replicaComm;
      bool isCanonical;
      const std::vector<DynProjTransferBlock> &sendPlan;
      const std::vector<DynProjTransferBlock> &recvPlan;
  };

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<class GridType>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::registerRedistributeProjectedHandler() {
    redistributeProjectedInitializers.template addHandler<GridType>();
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::redistributeProjectedGrid(
      const internal::pGridWrapper &oldWrapper,
      const internal::pGridWrapper &newWrapper,
      MPI_Comm replicaComm,
      bool isCanonical,
      const std::vector<DynProjTransferBlock> &sendPlan,
      const std::vector<DynProjTransferBlock> &recvPlan
  ) {
    SCHNEK_ASSERT(
        !redistributeProjectedInitializers.empty(),
        "No redistribute handler registered for projected grid type; "
        "ensure registerFieldProjection is used to register all grid types before balanceLoad"
    );

    RedistributeProjectedVisitor visitor(*this, newWrapper, replicaComm, isCanonical, sendPlan, recvPlan);
    redistributeProjectedInitializers.initialize(visitor);

    oldWrapper->accept(visitor, false);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename GridType>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::redistributeProjectedTyped(
      GridType &oldGrid,
      GridType &newGrid,
      MPI_Comm replicaComm,
      bool isCanonical,
      const std::vector<DynProjTransferBlock> &sendPlan,
      const std::vector<DynProjTransferBlock> &recvPlan
  ) {
    using ValueType = typename GridType::value_type;
    constexpr size_t projRank = GridType::Rank;
    using MemorySpace = detail::GridBufferMemorySpace<GridType>;

    if constexpr (detail::GridCommunicationTraits<GridType>::device_resident) {
      static_assert(
          std::is_same<MemorySpace, typename GridType::storage_type::MemorySpace>::value,
          "Device grid communication buffers must use the grid storage memory space"
      );
    }

    const MPI_Datatype mpiType = internal::mpiDatatypeFor<ValueType>();

    auto makeTypedRange = [](const DynProjTransferBlock &block) {
      typename GridType::IndexType lo, hi;
      for (size_t d = 0; d < projRank; ++d) {
        lo[d] = block.lo[d];
        hi[d] = block.hi[d];
      }
      return typename GridType::RangeType(lo, hi);
    };

    auto &bufferRegistry = mpiBufferRegistry.template get<MemorySpace>();
    auto &sendScratch = bufferRegistry.send();
    auto &recvScratch = bufferRegistry.receive();
    sendScratch.reset();
    recvScratch.reset();

    // Step 1: canonical-only point-to-point redistribution at projected rank.
    if (isCanonical) {
      const int localRank = ComRank;

      auto rangeVolume = [](const auto &block) { return schnek::volume(std::pair{block.lo, block.hi}); };
      auto [recvBlocks, recvTotal] = makeRedistributionSlabBlocks(recvPlan, localRank, rangeVolume);
      auto [sendBlocks, sendTotal] = makeRedistributionSlabBlocks(sendPlan, localRank, rangeVolume);

      auto copyLocalBlocks = [&]() {
        for (size_t i = 0; i < recvPlan.size(); ++i) {
          if (recvPlan[i].mpiRank != localRank) continue;
          auto copyRange = makeTypedRange(recvPlan[i]);
          detail::GridTransfer<GridType>::copy(oldGrid, newGrid, copyRange);
        }
      };

      transferRedistributionSlabs(
          &oldGrid, &newGrid, sendBlocks, recvBlocks, sendPlan, recvPlan, sendTotal, recvTotal, sendScratch,
          recvScratch, mpiType, 1, "Projected redistribution block too large for MPI count", "projected redistribution",
          makeTypedRange, copyLocalBlocks
      );
    }

    sendScratch.reset();
    recvScratch.reset();

    // Step 2: broadcast the new projected grid's inner range within the replica
    // sub-communicator so that non-canonical replicas see identical data.
    if (replicaComm != MPI_COMM_NULL) {
      auto innerRange = internal::RangeGetter<GridType>::get(newGrid);
      size_t volume = schnek::volume<projRank>(innerRange);
      if (volume > 0) {
        sendScratch.reserve_bytes(detail::checkedCommunicationBufferBytes(volume, sizeof(ValueType)));
        {
          auto buf = sendScratch.template make_vector<ValueType>(volume);
          if (isCanonical) {
            detail::GridTransfer<GridType>::pack(newGrid, innerRange, buf.data());
          }
          int errorCode = mpi.MPI_Bcast(
              buf.data(), internal::toIntCount(volume, "Projected redistribution block too large for MPI count"),
              mpiType, 0, replicaComm
          );
          SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Bcast failed during projected redistribution");
          if (!isCanonical) {
            detail::GridTransfer<GridType>::unpack(newGrid, innerRange, buf.data());
          }
        }
      }
    }

    sendScratch.reset();
    recvScratch.reset();
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::redistributeProjectedGrids(
      const std::map<long, std::vector<internal::pGridWrapper>> &oldProjectedGrids,
      const ProcRanges &oldRanges,
      const ProcRanges &newRanges
  ) {
    auto &projectedStorage = this->getProjectedGridStorage();
    auto &registrations = this->getProjectedRegistrations();

    for (auto &regEntry : registrations) {
      long id = regEntry.first;
      auto &reg = regEntry.second;
      const auto axes = reg->getAxes();
      const size_t projRank = reg->getProjRank();

      // Mark which full-rank axes belong to the projection.
      std::vector<bool> isProjAxis(rank, false);
      for (size_t a : axes) {
        SCHNEK_ASSERT(a < rank, "Projected axis out of range");
        isProjAxis[a] = true;
      }

      // Canonical if all non-projected coordinates are zero.
      bool isCanonical = true;
      for (size_t d = 0; d < rank; ++d) {
        if (!isProjAxis[d] && myCoord[d] != 0) {
          isCanonical = false;
          break;
        }
      }

      // Build / retrieve replica sub-communicator (orthogonal complement).
      // Only needed when at least one non-projected axis exists; if every axis
      // is projected there are no non-canonical replicas and no Bcast is needed.
      const bool hasNonProjectedAxis = (projRank < rank);
      MPI_Comm replicaComm = MPI_COMM_NULL;
      if (hasNonProjectedAxis) {
        auto commIt = projectedReplicaComms.find(axes);
        if (commIt != projectedReplicaComms.end()) {
          replicaComm = commIt->second;
        } else {
          int remainDims[rank];
          for (size_t d = 0; d < rank; ++d) {
            remainDims[d] = isProjAxis[d] ? 0 : 1;
          }
          int errorCode = mpi.MPI_Cart_sub(comm, remainDims, &replicaComm);
          SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Cart_sub failed during projected redistribution");
          projectedReplicaComms[axes] = replicaComm;
        }
      }

      // Build dynamic projected transfer plan among canonical processes.
      std::vector<DynProjTransferBlock> sendPlan;
      std::vector<DynProjTransferBlock> recvPlan;

      if (isCanonical) {
        // Project the per-axis bounds: oldProj[i][c] = (lo,hi) along projection axis i at coord c.
        std::vector<std::vector<std::pair<ptrdiff_t, ptrdiff_t>>> oldProj(projRank);
        std::vector<std::vector<std::pair<ptrdiff_t, ptrdiff_t>>> newProj(projRank);
        for (size_t i = 0; i < projRank; ++i) {
          size_t a = axes[i];
          oldProj[i].reserve(dims[a]);
          newProj[i].reserve(dims[a]);
          for (ptrdiff_t c = 0; c < dims[a]; ++c) {
            oldProj[i].emplace_back(oldRanges[a](c).getLo()[0], oldRanges[a](c).getHi()[0]);
            newProj[i].emplace_back(newRanges[a](c).getLo()[0], newRanges[a](c).getHi()[0]);
          }
        }

        // My projected coordinates.
        std::vector<ptrdiff_t> myProjCoord(projRank);
        for (size_t i = 0; i < projRank; ++i) {
          myProjCoord[i] = myCoord[axes[i]];
        }

        // My old/new projected ranges.
        std::vector<std::pair<ptrdiff_t, ptrdiff_t>> myOld(projRank), myNew(projRank);
        for (size_t i = 0; i < projRank; ++i) {
          myOld[i] = oldProj[i][myProjCoord[i]];
          myNew[i] = newProj[i][myProjCoord[i]];
        }

        // For each direction find candidate partner coords that overlap.
        std::vector<std::vector<ptrdiff_t>> sendOverlap(projRank), recvOverlap(projRank);
        for (size_t i = 0; i < projRank; ++i) {
          size_t a = axes[i];
          for (ptrdiff_t c = 0; c < dims[a]; ++c) {
            if (newProj[i][c].first <= myOld[i].second && newProj[i][c].second >= myOld[i].first) {
              sendOverlap[i].push_back(c);
            }
            if (oldProj[i][c].first <= myNew[i].second && oldProj[i][c].second >= myNew[i].first) {
              recvOverlap[i].push_back(c);
            }
          }
        }

        // Helper that resolves a projected coord vector to an MPI rank by
        // building the full coordinate (zeros on non-projection axes).
        auto coordToRank = [&](const std::vector<ptrdiff_t> &projCoord) -> int {
          int fullCoord[rank];
          for (size_t d = 0; d < rank; ++d) fullCoord[d] = 0;
          for (size_t i = 0; i < projRank; ++i) {
            fullCoord[axes[i]] = static_cast<int>(projCoord[i]);
          }
          int outRank = 0;
          int err = mpi.MPI_Cart_rank(comm, fullCoord, &outRank);
          SCHNEK_ASSERT(err == MPI_SUCCESS, "MPI_Cart_rank failed during projected redistribution");
          return outRank;
        };

        auto enumerate = [&](const std::vector<std::vector<ptrdiff_t>> &overlap,
                             const std::vector<std::pair<ptrdiff_t, ptrdiff_t>> &myRange,
                             const std::vector<std::vector<std::pair<ptrdiff_t, ptrdiff_t>>> &partnerProj,
                             std::vector<DynProjTransferBlock> &plan) {
          size_t total = 1;
          for (size_t i = 0; i < projRank; ++i) {
            if (overlap[i].empty()) return;
            total *= overlap[i].size();
          }
          std::vector<size_t> indices(projRank, 0);
          std::vector<ptrdiff_t> projCoord(projRank);
          for (size_t combo = 0; combo < total; ++combo) {
            size_t rem = combo;
            for (size_t i = projRank; i > 0; --i) {
              indices[i - 1] = rem % overlap[i - 1].size();
              rem /= overlap[i - 1].size();
            }
            DynProjTransferBlock block;
            block.lo.resize(projRank);
            block.hi.resize(projRank);
            bool nonEmpty = true;
            for (size_t i = 0; i < projRank; ++i) {
              projCoord[i] = overlap[i][indices[i]];
              ptrdiff_t lo = std::max(myRange[i].first, partnerProj[i][projCoord[i]].first);
              ptrdiff_t hi = std::min(myRange[i].second, partnerProj[i][projCoord[i]].second);
              if (lo > hi) {
                nonEmpty = false;
                break;
              }
              block.lo[i] = lo;
              block.hi[i] = hi;
            }
            if (nonEmpty) {
              block.mpiRank = coordToRank(projCoord);
              plan.push_back(std::move(block));
            }
          }
        };

        enumerate(sendOverlap, myOld, newProj, sendPlan);
        enumerate(recvOverlap, myNew, oldProj, recvPlan);
      }

      // Resolve old and new wrappers (any non-null entry; all entries share one grid).
      auto oldIt = oldProjectedGrids.find(id);
      auto newIt = projectedStorage.find(id);
      if (oldIt == oldProjectedGrids.end() || newIt == projectedStorage.end()) continue;

      internal::pGridWrapper oldWrapper, newWrapper;
      for (const auto &w : oldIt->second) {
        if (w) {
          oldWrapper = w;
          break;
        }
      }
      for (const auto &w : newIt->second) {
        if (w) {
          newWrapper = w;
          break;
        }
      }
      if (!oldWrapper || !newWrapper) continue;

#ifndef NDEBUG
      SCHNEK_ASSERT(
          oldIt->second.size() <= 1,
          "MpiCartesianDomainDecomposition: projected registration "
              << id << " holds " << oldIt->second.size()
              << " old projected grid wrappers but the single-region invariant requires at most 1."
      );
      SCHNEK_ASSERT(
          newIt->second.size() <= 1,
          "MpiCartesianDomainDecomposition: projected registration "
              << id << " holds " << newIt->second.size()
              << " new projected grid wrappers but the single-region invariant requires at most 1."
      );
#endif

      redistributeProjectedGrid(oldWrapper, newWrapper, replicaComm, isCanonical, sendPlan, recvPlan);
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  MpiCartesianDomainDecomposition<rank, CheckingPolicy>::~MpiCartesianDomainDecomposition() {
    if (comm == MPI_COMM_NULL && projectedReplicaComms.empty()) return;

    try {
      int finalized = 0;
      if (mpi.MPI_Finalized(&finalized) != MPI_SUCCESS || finalized) return;

      for (auto &entry : projectedReplicaComms) {
        if (entry.second != MPI_COMM_NULL) {
          try {
            mpi.MPI_Comm_free(&entry.second);
          } catch (...) {
          }
        }
      }
      if (comm != MPI_COMM_NULL) {
        try {
          mpi.MPI_Comm_free(&comm);
        } catch (...) {
        }
      }
    } catch (...) {
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::exchangeGrid(
      const internal::pGridWrapper &wrapper, bool useFieldInfo
  ) {
    if (exchangeVisitor.empty()) {
      SCHNECK_FAIL("No registered grids available for exchange");
    }

    wrapper->accept(*exchangeVisitor, useFieldInfo);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::accumulate(
      const internal::pGridWrapper &wrapper, bool useFieldInfo
  ) {
    if (accumulateVisitor.empty()) {
      SCHNECK_FAIL("No registered grids available for accumulate");
    }

    wrapper->accept(*accumulateVisitor, useFieldInfo);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename GridType>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::exchangeTyped(GridType &grid, bool useFieldInfo) {
    const auto [innerLo, innerHi] = innerBounds(grid, useFieldInfo);
    exchangeWithInteriorBounds(grid, innerLo, innerHi);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename GridType>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::accumulateTyped(GridType &grid, bool useFieldInfo) {
    const auto [innerLo, innerHi] = innerBounds(grid, useFieldInfo);
    accumulateWithInteriorBounds(grid, innerLo, innerHi);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::migrateParticles(const internal::pParticleWrapper &wrapper
  ) {
    if (migrateVisitor.empty()) {
      SCHNECK_FAIL("No registered particle containers available for migration");
    }

    wrapper->accept(*migrateVisitor);
  }

  namespace internal::mpi_cartesian_decomposition {
    SCHNEK_FUNCTION inline bool finitePositionValue(double value) {
      const double lowerBound = static_cast<double>(PTRDIFF_MIN);
      const double upperBoundExclusive = -lowerBound;
      return value >= lowerBound && value < upperBoundExclusive;
    }

    template<typename Particle, typename PositionAccessor, size_t rank>
    struct FinitePosition {
        PositionAccessor accessor;
        SCHNEK_FUNCTION bool operator()(const Particle &p) const {
          for (size_t d = 0; d < rank; ++d) {
            if (!finitePositionValue(accessor(p)[d])) {
              return false;
            }
          }
          return true;
        }
    };

    template<typename Particle, typename PositionAccessor>
    struct OwningCellDim {
        PositionAccessor accessor;
        SCHNEK_FUNCTION ptrdiff_t operator()(const Particle &p, size_t dim) const {
          const double value = accessor(p)[dim];
          if (!finitePositionValue(value)) {
            return PTRDIFF_MIN;
          }
          return static_cast<ptrdiff_t>(std::floor(value));
        }
    };

    /**
     * Classifies a particle along a single dimension into a migration bucket:
     *   - `0` keep (owning cell inside `[lo, hi]`),
     *   - `1` going down (owning cell `< lo`, travels to the lower neighbour),
     *   - `2` going up   (owning cell `> hi`, travels to the higher neighbour).
     *
     * Device-callable, so it can drive a parallel partition on a
     * device-resident particle container.
     */
    template<typename Particle, typename PositionAccessor>
    struct DimBucketClassifier {
        OwningCellDim<Particle, PositionAccessor> owningCell;
        ptrdiff_t lo;
        ptrdiff_t hi;
        size_t dim;
        SCHNEK_FUNCTION int operator()(const Particle &p) const {
          if (!finitePositionValue(owningCell.accessor(p)[dim])) {
            return 0;
          }
          const ptrdiff_t cell = owningCell(p, dim);
          if (cell < lo) {
            return 1;
          }
          if (cell > hi) {
            return 2;
          }
          return 0;
        }
    };

    /// Device-callable predicate: the owning cell of a particle along `dim` lies in `[lo, hi]`.
    template<typename Particle, typename PositionAccessor>
    struct DimInRange {
        OwningCellDim<Particle, PositionAccessor> owningCell;
        ptrdiff_t lo;
        ptrdiff_t hi;
        size_t dim;
        SCHNEK_FUNCTION bool operator()(const Particle &p) const {
          if (!finitePositionValue(owningCell.accessor(p)[dim])) {
            return false;
          }
          const ptrdiff_t cell = owningCell(p, dim);
          return cell >= lo && cell <= hi;
        }
    };

    /**
     * Byte count for `count` serialised particles. Fails instead of wrapping on overflow.
     */
    inline std::size_t checkedParticleBytes(std::size_t count, std::size_t particleBytes) {
      if (particleBytes != 0 && count > std::numeric_limits<std::size_t>::max() / particleBytes) {
        SCHNECK_FAIL("Particle migration byte count overflows size_t");
      }
      return count * particleBytes;
    }

    template<typename Traits, typename Serializer, typename ContainerType, std::size_t particleBytes>
    struct ExchangeParticle {
        MpiContext &mpi;
        MPI_Comm comm;
        ContainerType &container;
        schnek::detail::ParticleCommunicationBuffer<ContainerType> &recvScratchBuffer;

        // Phase A: upward movement. Send to `nextRank`, receive from `prevRank`.
        // Phase B: downward movement. Send to `prevRank`, receive from `nextRank`.
        // Each phase first exchanges the particle count, then the payload, and
        // finally deserialises arrivals back into the container.
        // A zero count is valid: the payload exchange still runs with null buffers.
        void operator()(const std::byte *sendBuffer, std::size_t sendCount, int sendRank, int recvRank) {
          int sendCountInt =
              schnek::internal::toIntCount(sendCount, "Particle migration block too large for MPI_Sendrecv count");
          int recvCountInt = 0;
          this->mpi.MPI_Sendrecv(
              &sendCountInt, 1, MPI_INT, sendRank, 0, &recvCountInt, 1, MPI_INT, recvRank, 0, comm, MPI_STATUS_IGNORE
          );

          // A negative count from the peer is a protocol error. It must not be widened to size_t.
          if (recvCountInt < 0) {
            SCHNECK_FAIL("Particle migration received a negative particle count");
          }
          const std::size_t recvCount = static_cast<std::size_t>(recvCountInt);
          const int sendByteCount = schnek::internal::toIntCount(
              checkedParticleBytes(sendCount, particleBytes),
              "Particle migration block too large for MPI_Sendrecv count"
          );
          const std::size_t recvBytes = checkedParticleBytes(recvCount, particleBytes);
          const int recvByteCount =
              schnek::internal::toIntCount(recvBytes, "Particle migration block too large for MPI_Sendrecv count");
          recvScratchBuffer.reserve_bytes(recvBytes);
          recvScratchBuffer.reset();
          auto recvBuffer = recvScratchBuffer.template make_vector<std::byte>(recvBytes);

          this->mpi.MPI_Sendrecv(
              sendCount > 0 ? sendBuffer : nullptr, sendByteCount, MPI_BYTE, sendRank, 1,
              recvCount > 0 ? recvBuffer.data() : nullptr, recvByteCount, MPI_BYTE, recvRank, 1, comm, MPI_STATUS_IGNORE
          );

          if (recvCount > 0) {
            Traits::template bulkInsert<Serializer>(container, recvBuffer.data(), recvCount);
          }
        };
    };

  }  // namespace internal::mpi_cartesian_decomposition

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename WrapperImpl>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::migrateTyped(WrapperImpl &wrapper) {
    using ContainerType = std::decay_t<decltype(wrapper.container)>;
    using Traits = ParticleContainerTraits<ContainerType>;
    using Particle = typename Traits::value_type;
    using Serializer = ParticleSerializer<Particle>;
    using PositionAccessor = std::decay_t<decltype(wrapper.accessor)>;

    auto &container = wrapper.container;

    const RangeType localRange = getLocalInnerRange();
    const LimitType lo = localRange.getLo();
    const LimitType hi = localRange.getHi();

    constexpr std::size_t particleBytes = Serializer::size();

    auto &bufferRegistry = mpiBufferRegistry.template get<detail::ParticleBufferMemorySpace<ContainerType>>();
    auto &sendScratchBuffer = bufferRegistry.send();
    auto &recvScratchBuffer = bufferRegistry.receive();
    auto &partitionScratchBuffer = bufferRegistry.partition();
    bool invalidWrapMagnitude = false;

    // Sweep one dimension at a time. A particle bound for a diagonal neighbour
    // is forwarded dimension by dimension: received particles are re-inserted
    // into the container and re-examined by the next dimension's sweep.
    for (size_t dim = 0; dim < rank; ++dim) {
      int prevRank = MPI_PROC_NULL;
      int nextRank = MPI_PROC_NULL;
      this->mpi.MPI_Cart_shift(comm, static_cast<int>(dim), 1, &prevRank, &nextRank);

      // Partition the container in place with a count -> scan -> pack primitive.
      // Departing particles are serialised into tightly-sized byte buffers in the
      // container's memory space: bucket 0 travels down to `prevRank`, bucket 1
      // travels up to `nextRank`. Survivors are compacted in place.
      internal::mpi_cartesian_decomposition::DimBucketClassifier<Particle, PositionAccessor> classify{
          {wrapper.accessor}, lo[dim], hi[dim], dim
      };

      std::array<std::size_t, 2> counts{};
      Traits::countBuckets(container, classify, counts);
      const std::size_t downCount = counts[0];
      const std::size_t upCount = counts[1];

      {
        // Down and up departures share one allocation: down first, then up.
        // Offsets are only formed for non-empty buckets, so no pointer arithmetic starts from null.
        const std::size_t downBytes =
            internal::mpi_cartesian_decomposition::checkedParticleBytes(downCount, particleBytes);
        const std::size_t totalBytes =
            internal::mpi_cartesian_decomposition::checkedParticleBytes(downCount + upCount, particleBytes);
        auto sendBytes = sendScratchBuffer.template make_vector<std::byte>(totalBytes);
        std::byte *goingDown = downCount > 0 ? sendBytes.data() : nullptr;
        std::byte *goingUp = upCount > 0 ? sendBytes.data() + downBytes : nullptr;

        // With no departures there is nothing to pack and no survivors to compact, so the local
        // partition and its scratch are skipped. The exchanges below still run to accept arrivals.
        if (downCount + upCount > 0) {
          const std::array<std::byte *, 2> outPtrs{goingDown, goingUp};
          auto partitionScratch =
              partitionScratchBuffer.template make_vector<std::byte>(Traits::partitionScratchBytes(container));
          Traits::template partitionAndPack<Serializer>(container, classify, outPtrs, partitionScratch.data());
        }

        internal::mpi_cartesian_decomposition::ExchangeParticle<Traits, Serializer, ContainerType, particleBytes>
            exchange{this->mpi, comm, container, recvScratchBuffer};

        exchange(goingUp, upCount, nextRank, prevRank);
        exchange(goingDown, downCount, prevRank, nextRank);
      }

      // All pmr views from this sweep are out of scope; the arenas can be released.
      sendScratchBuffer.reset();
      recvScratchBuffer.reset();
      partitionScratchBuffer.reset();

      if constexpr (internal::has_position_shift<PositionAccessor, Particle>::value) {
        const bool atPeriodicEdge = myCoord[dim] == 0 || myCoord[dim] == dims[dim] - 1;
        if (atPeriodicEdge) {
          const ptrdiff_t globalLo = this->globalRange.getLo()[dim];
          const ptrdiff_t globalHi = this->globalRange.getHi()[dim];
          const double length = static_cast<double>(globalHi - globalLo + 1);
          const bool finitePositions = Traits::allOf(
              container, internal::FinitePositionValue<Particle, PositionAccessor>{wrapper.accessor, dim}
          );
          const bool validPeriods = Traits::allOf(
              container,
              internal::ValidWrapPeriods<Particle, PositionAccessor>{
                  wrapper.accessor, dim, static_cast<double>(globalLo), length
              }
          );
          invalidWrapMagnitude = invalidWrapMagnitude || (finitePositions && !validPeriods);
          if (finitePositions && validPeriods) {
            Traits::forEachParticle(
                container,
                internal::WrapDim<Particle, PositionAccessor>{
                    wrapper.accessor, dim, static_cast<double>(globalLo), static_cast<double>(globalHi), length
                }
            );
          }
        }
      }
    }

    // The neighbour sweeps are the fast path: they forward particles one
    // process at a time. Any stragglers are routed directly to their owner.
    int localStatus[2]{};
    if (particleChecks) {
      const bool finitePositions = Traits::allOf(
          container,
          internal::mpi_cartesian_decomposition::FinitePosition<Particle, PositionAccessor, rank>{wrapper.accessor}
      );
      localStatus[0] = finitePositions ? (invalidWrapMagnitude ? 1 : 0) : 2;
    } else if (invalidWrapMagnitude) {
      localStatus[0] = 1;
    }
    for (size_t d = 0; d < rank; ++d) {
      if (!Traits::allOf(
              container,
              internal::mpi_cartesian_decomposition::DimInRange<Particle, PositionAccessor>{
                  {wrapper.accessor}, lo[d], hi[d], d
              }
          )) {
        localStatus[1] = 1;
      }
    }

    int globalStatus[2]{};
    const int errorCode = this->mpi.MPI_Allreduce(localStatus, globalStatus, 2, MPI_INT, MPI_MAX, comm);
    if (errorCode != MPI_SUCCESS) {
      SCHNECK_FAIL("MPI_Allreduce failed while validating particle migration");
    }
    if (globalStatus[0] == 2) {
      SCHNECK_FAIL("non-finite particle position");
    }
    if (globalStatus[0] == 1) {
      SCHNECK_FAIL("Particle periodic wrap count exceeds the supported magnitude");
    }
    if (globalStatus[1] != 0) {
      routeStragglingParticles(wrapper);
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename WrapperImpl>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::routeStragglingParticles(WrapperImpl &wrapper) {
    using ContainerType = std::decay_t<decltype(wrapper.container)>;
    using Particle = typename ParticleContainerTraits<ContainerType>::value_type;
    using PositionAccessor = std::decay_t<decltype(wrapper.accessor)>;
    using TransferBlockType = TransferBlock<rank, CheckingPolicy>;

    // When optional checks are disabled, still reject non-finite positions
    // collectively before the owner router classifies them.
    if (!particleChecks) {
      const bool allFinite = ParticleContainerTraits<ContainerType>::allOf(
          wrapper.container,
          internal::mpi_cartesian_decomposition::FinitePosition<Particle, PositionAccessor, rank>{wrapper.accessor}
      );
      const int localInvalid = allFinite ? 0 : 1;
      const int globalInvalid = allReduce(localInvalid, MPI_MAX);
      if (globalInvalid != 0) {
        SCHNECK_FAIL("non-finite particle position");
      }
    }

    const LimitType globalLo = this->globalRange.getLo();
    const LimitType globalHi = this->globalRange.getHi();
    int localOutOfGlobal = 0;
    for (size_t d = 0; d < rank; ++d) {
      if (!ParticleContainerTraits<ContainerType>::allOf(
              wrapper.container,
              internal::mpi_cartesian_decomposition::DimInRange<Particle, PositionAccessor>{
                  {wrapper.accessor}, globalLo[d], globalHi[d], d
              }
          )) {
        localOutOfGlobal = 1;
      }
    }
    if (allReduce(localOutOfGlobal, MPI_MAX) != 0) {
      if constexpr (!internal::has_position_shift<PositionAccessor, Particle>::value) {
        SCHNECK_FAIL(
            "Particle left the global domain and the position accessor has no shift(); either provide "
            "shift(p, dim, delta) for periodic wrapping or handle the boundary (absorb/reflect) before migrate()"
        );
      } else {
        SCHNECK_FAIL("Particle owning cell lies outside the global range during multi-hop migration");
      }
    }

    std::vector<TransferBlockType> ownerPlan;
    LimitType coord(0);
    bool done = false;
    while (!done) {
      LimitType lo(0);
      LimitType hi(0);
      for (size_t d = 0; d < rank; ++d) {
        const auto &range = procRanges[d](coord[d]);
        lo[d] = range.getLo()[0];
        hi[d] = range.getHi()[0];
      }
      ownerPlan.push_back({coordToMpiRank(coord), RangeType(lo, hi)});

      for (size_t d = rank; d > 0; --d) {
        const size_t axis = d - 1;
        if (coord[axis] + 1 < dims[axis]) {
          ++coord[axis];
          break;
        }
        coord[axis] = 0;
        if (axis == 0) {
          done = true;
        }
      }
    }

    redistributeParticlesTyped(wrapper, wrapper, ownerPlan, ownerPlan);
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  class MpiCartesianDomainDecomposition<rank, CheckingPolicy>::RedistributeParticleVisitor
      : public internal::ParticleVisitor<
            typename MpiCartesianDomainDecomposition<rank, CheckingPolicy>::RedistributeParticleVisitor> {
    public:
      RedistributeParticleVisitor(
          MpiCartesianDomainDecomposition &parentIn,
          internal::pParticleWrapper newWrapperIn,
          const std::vector<TransferBlock<rank, CheckingPolicy>> &sendPlanIn,
          const std::vector<TransferBlock<rank, CheckingPolicy>> &recvPlanIn
      )
          : parent(parentIn), newWrapper(std::move(newWrapperIn)), sendPlan(sendPlanIn), recvPlan(recvPlanIn) {}

      template<typename WrapperImpl>
      void handle(WrapperImpl &oldWrapper) {
        auto typedNewWrapper = std::dynamic_pointer_cast<WrapperImpl>(newWrapper);
        SCHNEK_ASSERT(typedNewWrapper, "Particle container type mismatch during redistribution");
        parent.template redistributeParticlesTyped<WrapperImpl>(oldWrapper, *typedNewWrapper, sendPlan, recvPlan);
      }

    private:
      MpiCartesianDomainDecomposition &parent;
      internal::pParticleWrapper newWrapper;
      const std::vector<TransferBlock<rank, CheckingPolicy>> &sendPlan;
      const std::vector<TransferBlock<rank, CheckingPolicy>> &recvPlan;
  };

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<class ContainerType, class PositionAccessor>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::registerRedistributeParticleHandler() {
    using WrapperImpl = internal::ParticleWrapperImpl<ContainerType, PositionAccessor>;
    redistributeParticleInitializers.template addHandler<WrapperImpl>();
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::redistributeParticles(
      const internal::pParticleWrapper &oldWrapper,
      const internal::pParticleWrapper &newWrapper,
      const std::vector<TransferBlock<rank, CheckingPolicy>> &sendPlan,
      const std::vector<TransferBlock<rank, CheckingPolicy>> &recvPlan
  ) {
    if (redistributeParticleInitializers.empty()) {
      SCHNECK_FAIL("No registered particle containers available for redistribution");
    }

    RedistributeParticleVisitor visitor(*this, newWrapper, sendPlan, recvPlan);
    redistributeParticleInitializers.initialize(visitor);

    oldWrapper->accept(visitor);
  }

  namespace internal::mpi_cartesian_decomposition {
    /// One send-plan block as the device-callable router reads it: a box and its removal bucket.
    template<size_t rank>
    struct RouteBlock {
        ptrdiff_t lo[rank];
        ptrdiff_t hi[rank];
        int bucket;
    };

    /**
     * Routes a particle to the bucket of the send-plan block that contains its owning cell.
     * Returns `0` if no block contains it; the caller treats that as an error.
     * Device-callable, so it can drive a parallel partition on a device-resident container.
     */
    template<size_t rank, typename Particle, typename PositionAccessor>
    struct RouteClassifier {
        OwningCellDim<Particle, PositionAccessor> owningCell;
        const RouteBlock<rank> *blocks;
        std::size_t numBlocks;

        SCHNEK_FUNCTION int operator()(const Particle &p) const {
          ptrdiff_t cell[rank];
          for (size_t d = 0; d < rank; ++d) {
            if (!FinitePosition<Particle, PositionAccessor, rank>{owningCell.accessor}(p)) {
              return 0;
            }
            cell[d] = owningCell(p, d);
          }
          for (std::size_t b = 0; b < numBlocks; ++b) {
            bool inside = true;
            for (size_t d = 0; d < rank; ++d) {
              if (cell[d] < blocks[b].lo[d] || cell[d] > blocks[b].hi[d]) {
                inside = false;
                break;
              }
            }
            if (inside) {
              return blocks[b].bucket;
            }
          }
          return 0;
        }
    };

  }  // namespace internal::mpi_cartesian_decomposition

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename WrapperImpl>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::redistributeParticlesTyped(
      WrapperImpl &oldWrapper,
      WrapperImpl &newWrapper,
      const std::vector<TransferBlock<rank, CheckingPolicy>> &sendPlan,
      const std::vector<TransferBlock<rank, CheckingPolicy>> &recvPlan
  ) {
    using ContainerType = std::decay_t<decltype(oldWrapper.container)>;
    using Traits = ParticleContainerTraits<ContainerType>;
    using Particle = typename Traits::value_type;
    using Serializer = ParticleSerializer<Particle>;
    using PositionAccessor = std::decay_t<decltype(oldWrapper.accessor)>;
    using RouteBlockType = internal::mpi_cartesian_decomposition::RouteBlock<rank>;
    using internal::mpi_cartesian_decomposition::checkedParticleBytes;

    auto &oldContainer = oldWrapper.container;
    auto &newContainer = newWrapper.container;

    constexpr std::size_t particleBytes = Serializer::size();
    const int localRank = ComRank;

    // Destination ranks are the distinct owners in the send plan, this process included.
    // Destination i is removed into bucket i + 1, so every particle leaves the old container.
    std::vector<int> destRanks;
    for (const auto &block : sendPlan) {
      destRanks.push_back(block.mpiRank);
    }
    std::sort(destRanks.begin(), destRanks.end());
    destRanks.erase(std::unique(destRanks.begin(), destRanks.end()), destRanks.end());
    const std::size_t numBuckets = destRanks.size();

    // Partner ranks come from the transfer plans (not from the particle counts) so that the
    // count exchange stays symmetric even when a block carries no particles.
    std::vector<int> recvPartners;
    for (const auto &block : recvPlan) {
      if (block.mpiRank != localRank) {
        recvPartners.push_back(block.mpiRank);
      }
    }
    std::sort(recvPartners.begin(), recvPartners.end());
    recvPartners.erase(std::unique(recvPartners.begin(), recvPartners.end()), recvPartners.end());

    std::vector<RouteBlockType> routeTable;
    routeTable.reserve(sendPlan.size());
    for (const auto &block : sendPlan) {
      RouteBlockType entry{};
      for (size_t d = 0; d < rank; ++d) {
        entry.lo[d] = static_cast<ptrdiff_t>(block.range.getLo()[d]);
        entry.hi[d] = static_cast<ptrdiff_t>(block.range.getHi()[d]);
      }
      entry.bucket =
          static_cast<int>(std::lower_bound(destRanks.begin(), destRanks.end(), block.mpiRank) - destRanks.begin()) + 1;
      routeTable.push_back(entry);
    }

    auto &bufferRegistry = mpiBufferRegistry.template get<detail::ParticleBufferMemorySpace<ContainerType>>();
    auto &routeScratch = bufferRegistry.route();
    auto &sendScratch = bufferRegistry.send();
    auto &recvScratch = bufferRegistry.receive();
    auto &partitionScratch = bufferRegistry.partition();
    routeScratch.reset();
    sendScratch.reset();
    recvScratch.reset();
    partitionScratch.reset();

    auto waitAll = [&](std::vector<MPI_Request> &requests) {
      if (!requests.empty()) {
        int errorCode = mpi.MPI_Waitall(static_cast<int>(requests.size()), requests.data(), MPI_STATUSES_IGNORE);
        SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Waitall failed during particle redistribution");
      }
    };

    // All buffer views live in this block; the arenas are reset once they are out of scope.
    {
      // Classify every particle against the routing table and count per destination.
      // Counting doubles as validation: each old particle must match a send-plan block.
      const std::size_t numRouteBlocks = routeTable.size();
      auto routeVec = routeScratch.template make_vector<RouteBlockType>(numRouteBlocks);
      detail::copyHostToCommunicationBuffer(routeVec, routeTable.data(), numRouteBlocks);
      const RouteBlockType *routeBlocks = numRouteBlocks > 0 ? routeVec.data() : nullptr;

      internal::mpi_cartesian_decomposition::RouteClassifier<rank, Particle, PositionAccessor> classify{
          {oldWrapper.accessor}, routeBlocks, numRouteBlocks
      };

      std::vector<std::size_t> counts(numBuckets, 0);
      Traits::countBuckets(oldContainer, classify, counts.data(), numBuckets);
      std::size_t routedCount = 0;
      for (std::size_t c : counts) {
        routedCount += c;
      }
      if (routedCount != Traits::size(oldContainer)) {
        SCHNECK_FAIL("Particle owning cell lies outside the old local range during redistribution");
      }

      // Pack every bucket back to back in one send allocation. Bucket i starts at bucketStart[i].
      std::vector<std::size_t> bucketStart(numBuckets, 0);
      for (size_t i = 1; i < numBuckets; ++i) {
        bucketStart[i] = bucketStart[i - 1] + counts[i - 1];
      }
      const std::size_t oldCount = Traits::size(oldContainer);
      auto sendBytes = sendScratch.template make_vector<std::byte>(checkedParticleBytes(oldCount, particleBytes));
      std::vector<std::byte *> outPtrs(numBuckets, nullptr);
      for (size_t i = 0; i < numBuckets; ++i) {
        if (counts[i] > 0) {
          outPtrs[i] = sendBytes.data() + checkedParticleBytes(bucketStart[i], particleBytes);
        }
      }
      if (oldCount > 0) {
        auto partitionBytes =
            partitionScratch.template make_vector<std::byte>(Traits::partitionScratchBytes(oldContainer));
        Traits::template partitionAndPack<Serializer>(
            oldContainer, classify, outPtrs.data(), numBuckets, partitionBytes.data()
        );
      }

      // Phase 1: exchange particle counts with every partner.
      std::vector<MPI_Request> requests;
      std::vector<int> recvCounts(recvPartners.size(), 0);
      for (size_t p = 0; p < recvPartners.size(); ++p) {
        MPI_Request req;
        int errorCode = mpi.MPI_Irecv(&recvCounts[p], 1, MPI_INT, recvPartners[p], 0, comm, &req);
        SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Irecv failed during particle redistribution");
        requests.push_back(req);
      }

      std::vector<int> countMessages(numBuckets, 0);
      for (size_t i = 0; i < numBuckets; ++i) {
        if (destRanks[i] == localRank) {
          continue;
        }
        countMessages[i] = internal::toIntCount(counts[i], "Particle redistribution block too large for MPI count");
        MPI_Request req;
        int errorCode = mpi.MPI_Isend(&countMessages[i], 1, MPI_INT, destRanks[i], 0, comm, &req);
        SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Isend failed during particle redistribution");
        requests.push_back(req);
      }
      waitAll(requests);

      for (int c : recvCounts) {
        if (c < 0) {
          SCHNECK_FAIL("Particle redistribution received a negative particle count");
        }
      }

      // Phase 2: exchange the packed payloads. Arrivals from each partner are stored back to back.
      std::vector<std::size_t> recvStart(recvPartners.size(), 0);
      std::size_t recvTotal = 0;
      for (size_t p = 0; p < recvPartners.size(); ++p) {
        recvStart[p] = recvTotal;
        recvTotal += static_cast<std::size_t>(recvCounts[p]);
      }
      auto recvBytes = recvScratch.template make_vector<std::byte>(checkedParticleBytes(recvTotal, particleBytes));

      for (size_t p = 0; p < recvPartners.size(); ++p) {
        const std::size_t count = static_cast<std::size_t>(recvCounts[p]);
        if (count == 0) {
          continue;
        }
        MPI_Request req;
        int errorCode = mpi.MPI_Irecv(
            recvBytes.data() + checkedParticleBytes(recvStart[p], particleBytes),
            internal::toIntCount(
                checkedParticleBytes(count, particleBytes), "Particle redistribution block too large for MPI count"
            ),
            MPI_BYTE, recvPartners[p], 1, comm, &req
        );
        SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Irecv failed during particle redistribution");
        requests.push_back(req);
      }

      for (size_t i = 0; i < numBuckets; ++i) {
        if (destRanks[i] == localRank || counts[i] == 0) {
          continue;
        }
        MPI_Request req;
        int errorCode = mpi.MPI_Isend(
            sendBytes.data() + checkedParticleBytes(bucketStart[i], particleBytes),
            internal::toIntCount(
                checkedParticleBytes(counts[i], particleBytes), "Particle redistribution block too large for MPI count"
            ),
            MPI_BYTE, destRanks[i], 1, comm, &req
        );
        SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Isend failed during particle redistribution");
        requests.push_back(req);
      }
      waitAll(requests);

      // Insert self-owned particles straight from the send buffer, then the arrivals from each partner.
      for (size_t i = 0; i < numBuckets; ++i) {
        if (destRanks[i] == localRank && counts[i] > 0) {
          Traits::template bulkInsert<Serializer>(
              newContainer, sendBytes.data() + checkedParticleBytes(bucketStart[i], particleBytes), counts[i]
          );
        }
      }
      for (size_t p = 0; p < recvPartners.size(); ++p) {
        const std::size_t count = static_cast<std::size_t>(recvCounts[p]);
        if (count > 0) {
          Traits::template bulkInsert<Serializer>(
              newContainer, recvBytes.data() + checkedParticleBytes(recvStart[p], particleBytes), count
          );
        }
      }
    }

    routeScratch.reset();
    sendScratch.reset();
    recvScratch.reset();
    partitionScratch.reset();

#ifndef NDEBUG
    // Every particle now in the new container must be owned by this process's
    // new local range. The new range is the bounding box of the receive plan,
    // which tiles it exactly, so the check does not depend on `procRanges`
    // having been updated yet.
    if (!recvPlan.empty()) {
      LimitType newLo = recvPlan.front().range.getLo();
      LimitType newHi = recvPlan.front().range.getHi();
      for (const auto &block : recvPlan) {
        for (size_t d = 0; d < rank; ++d) {
          if (block.range.getLo()[d] < newLo[d]) {
            newLo[d] = block.range.getLo()[d];
          }
          if (block.range.getHi()[d] > newHi[d]) {
            newHi[d] = block.range.getHi()[d];
          }
        }
      }

      for (size_t d = 0; d < rank; ++d) {
        const bool inside = Traits::allOf(
            newContainer,
            internal::mpi_cartesian_decomposition::DimInRange<Particle, PositionAccessor>{
                {oldWrapper.accessor}, newLo[d], newHi[d], d
            }
        );
        SCHNEK_ASSERT(inside, "Redistributed particle is not owned by the new local range in dimension " << d);
      }
    }
#endif
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
  std::pair<typename GridType::IndexType, typename GridType::IndexType>
  MpiCartesianDomainDecomposition<rank, CheckingPolicy>::innerBounds(GridType &grid, bool useFieldInfo) const {
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
  template<typename IndexType>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::validateHaloWidths(
      const IndexType &gridLo, const IndexType &gridHi, const IndexType &innerLo, const IndexType &innerHi
  ) const {
    int localOk = 1;
    size_t invalidDim = rank;
    ptrdiff_t invalidExtent = 0;
    ptrdiff_t invalidLowerHalo = 0;
    ptrdiff_t invalidUpperHalo = 0;

    for (size_t dim = 0; dim < rank; ++dim) {
      const ptrdiff_t lowerHalo = innerLo[dim] - gridLo[dim];
      const ptrdiff_t upperHalo = gridHi[dim] - innerHi[dim];
      const ptrdiff_t innerExtent = innerHi[dim] - innerLo[dim] + 1;
      if (innerExtent <= 0 || innerExtent < lowerHalo || innerExtent < upperHalo) {
        localOk = 0;
        if (invalidDim == rank) {
          invalidDim = dim;
          invalidExtent = innerExtent;
          invalidLowerHalo = lowerHalo;
          invalidUpperHalo = upperHalo;
        }
      }
    }

    // Agree before any rank enters point-to-point halo communication.
    int globalOk = 0;
    const int errorCode = mpi.MPI_Allreduce(&localOk, &globalOk, 1, MPI_INT, MPI_MIN, comm);
    SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Allreduce failed while validating halo widths");
    if (globalOk != 0) return;

    std::string rankCoordinates = "(";
    for (size_t dim = 0; dim < rank; ++dim) {
      if (dim > 0) rankCoordinates += ",";
      rankCoordinates += std::to_string(myCoord[dim]);
    }
    rankCoordinates += ")";

    if (invalidDim != rank) {
      SCHNECK_FAIL(
          "Halo width exceeds the local inner extent in dimension "
          << invalidDim << " (inner extent=" << invalidExtent << ", lower halo=" << invalidLowerHalo
          << ", upper halo=" << invalidUpperHalo << ") at rank coordinates " << rankCoordinates
      );
    }
    SCHNECK_FAIL(
        "Halo width exceeds the local inner extent in dimension unknown (failure detected on another rank; "
        "local rank coordinates "
        << rankCoordinates << ")"
    );
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename GridType>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::exchangeWithInteriorBounds(
      GridType &grid, const typename GridType::IndexType &innerLo, const typename GridType::IndexType &innerHi
  ) {
    using RangeTypeLocal = typename GridType::RangeType;
    using ValueType = typename GridType::value_type;
    using MemorySpace = detail::GridBufferMemorySpace<GridType>;

    auto &bufferRegistry = mpiBufferRegistry.template get<MemorySpace>();
    auto &sendScratchBuffer = bufferRegistry.send();
    auto &recvScratchBuffer = bufferRegistry.receive();

    const auto gridLo = grid.getLo();
    const auto gridHi = grid.getHi();
    validateHaloWidths(gridLo, gridHi, innerLo, innerHi);

    const MPI_Datatype mpiType = internal::mpiDatatypeFor<ValueType>();

    for (size_t dim = 0; dim < rank; ++dim) {
      int prevRank = MPI_PROC_NULL;
      int nextRank = MPI_PROC_NULL;
      this->mpi.MPI_Cart_shift(comm, static_cast<int>(dim), 1, &prevRank, &nextRank);

      const auto ranges = detail::haloRanges<RangeTypeLocal>(dim, gridLo, gridHi, innerLo, innerHi);
      SCHNEK_ASSERT(
          ranges.lowerHalo == ranges.upperHalo, "MPI halo exchange requires symmetric halo widths in dimension "
                                                    << dim << " (lower=" << ranges.lowerHalo
                                                    << ", upper=" << ranges.upperHalo << ")"
      );

      const auto counts = detail::haloRangeCounts(ranges);
      const auto &loGhostRange = ranges.loGhostRange;
      const auto &hiGhostRange = ranges.hiGhostRange;
      const auto &loSourceRange = ranges.loSourceRange;
      const auto &hiSourceRange = ranges.hiSourceRange;
      const size_t sendLowerCount = counts.sendLowerCount;
      const size_t recvLowerCount = counts.recvLowerCount;
      const size_t sendUpperCount = counts.sendUpperCount;
      const size_t recvUpperCount = counts.recvUpperCount;

      size_t maxSendCount = std::max(sendLowerCount, sendUpperCount);
      size_t maxRecvCount = std::max(recvLowerCount, recvUpperCount);

      sendScratchBuffer.reserve_bytes(detail::checkedCommunicationBufferBytes(maxSendCount, sizeof(ValueType)));
      recvScratchBuffer.reserve_bytes(detail::checkedCommunicationBufferBytes(maxRecvCount, sizeof(ValueType)));

      auto sendBuffer = sendScratchBuffer.template make_vector<ValueType>(maxSendCount);
      auto recvBuffer = recvScratchBuffer.template make_vector<ValueType>(maxRecvCount);

      // Exchange to fill lower ghost cells (receive from prev, send to next)
      if (hiSourceRange && sendLowerCount > 0) {
        detail::packGridRange(grid, *hiSourceRange, sendBuffer);
      }

      this->mpi.MPI_Sendrecv(
          sendLowerCount > 0 ? sendBuffer.data() : nullptr,
          internal::toIntCount(sendLowerCount, "Halo exchange block too large for MPI_Sendrecv count"), mpiType,
          nextRank, 0, recvLowerCount > 0 ? recvBuffer.data() : nullptr,
          internal::toIntCount(recvLowerCount, "Halo exchange block too large for MPI_Sendrecv count"), mpiType,
          prevRank, 0, comm, MPI_STATUS_IGNORE
      );
      if (loGhostRange && recvLowerCount > 0) {
        detail::unpackGridRange(grid, *loGhostRange, recvBuffer);
      }

      // Exchange to fill upper ghost cells (receive from next, send to prev)
      if (loSourceRange && sendUpperCount > 0) {
        detail::packGridRange(grid, *loSourceRange, sendBuffer);
      }

      this->mpi.MPI_Sendrecv(
          sendUpperCount > 0 ? sendBuffer.data() : nullptr,
          internal::toIntCount(sendUpperCount, "Halo exchange block too large for MPI_Sendrecv count"), mpiType,
          prevRank, 0, recvUpperCount > 0 ? recvBuffer.data() : nullptr,
          internal::toIntCount(recvUpperCount, "Halo exchange block too large for MPI_Sendrecv count"), mpiType,
          nextRank, 0, comm, MPI_STATUS_IGNORE
      );
      if (hiGhostRange && recvUpperCount > 0) {
        detail::unpackGridRange(grid, *hiGhostRange, recvBuffer);
      }
      sendScratchBuffer.reset();
      recvScratchBuffer.reset();
    }
  }

  // ==============================================================================================================================
  // ==============================================================================================================================
  // ==============================================================================================================================

  template<size_t rank, template<size_t> class CheckingPolicy>
  template<typename GridType>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::accumulateWithInteriorBounds(
      GridType &grid, const typename GridType::IndexType &innerLo, const typename GridType::IndexType &innerHi
  ) {
    using RangeTypeLocal = typename GridType::RangeType;
    using ValueType = typename GridType::value_type;
    using MemorySpace = detail::GridBufferMemorySpace<GridType>;

    auto &bufferRegistry = mpiBufferRegistry.template get<MemorySpace>();
    auto &sendScratchBuffer = bufferRegistry.send();
    auto &recvScratchBuffer = bufferRegistry.receive();

    const auto gridLo = grid.getLo();
    const auto gridHi = grid.getHi();
    validateHaloWidths(gridLo, gridHi, innerLo, innerHi);

    const MPI_Datatype mpiType = internal::mpiDatatypeFor<ValueType>();

    for (size_t dim = 0; dim < rank; ++dim) {
      int prevRank = MPI_PROC_NULL;
      int nextRank = MPI_PROC_NULL;
      this->mpi.MPI_Cart_shift(comm, static_cast<int>(dim), 1, &prevRank, &nextRank);

      const auto ranges = detail::haloRanges<RangeTypeLocal>(dim, gridLo, gridHi, innerLo, innerHi);
      SCHNEK_ASSERT(
          ranges.lowerHalo == ranges.upperHalo, "MPI halo accumulate requires symmetric halo widths in dimension "
                                                    << dim << " (lower=" << ranges.lowerHalo
                                                    << ", upper=" << ranges.upperHalo << ")"
      );

      const auto counts = detail::haloRangeCounts(ranges);
      const auto &loGhostRange = ranges.loGhostRange;
      const auto &hiGhostRange = ranges.hiGhostRange;
      const auto &loSourceRange = ranges.loSourceRange;
      const auto &hiSourceRange = ranges.hiSourceRange;
      const size_t sendLowerCount = counts.sendLowerCount;
      const size_t recvLowerCount = counts.recvLowerCount;
      const size_t sendUpperCount = counts.sendUpperCount;
      const size_t recvUpperCount = counts.recvUpperCount;

      const size_t maxSendCount = std::max(sendLowerCount, sendUpperCount);
      const size_t maxRecvCount = std::max(recvLowerCount, recvUpperCount);
      const size_t maxCount = std::max(maxSendCount, maxRecvCount);

      sendScratchBuffer.reserve_bytes(detail::checkedCommunicationBufferBytes(maxCount, sizeof(ValueType)));
      recvScratchBuffer.reserve_bytes(detail::checkedCommunicationBufferBytes(maxCount, sizeof(ValueType)));

      auto sendBuffer = sendScratchBuffer.template make_vector<ValueType>(maxCount);
      auto recvBuffer = recvScratchBuffer.template make_vector<ValueType>(maxCount);

      // Lower side: add incoming data to lower ghost cells, then return result to update upper inner cells in prev.
      if (hiSourceRange && sendLowerCount > 0) {
        detail::packGridRange(grid, *hiSourceRange, sendBuffer);
      }

      this->mpi.MPI_Sendrecv(
          sendLowerCount > 0 ? sendBuffer.data() : nullptr,
          internal::toIntCount(sendLowerCount, "Halo accumulate block too large for MPI_Sendrecv count"), mpiType,
          nextRank, 0, recvLowerCount > 0 ? recvBuffer.data() : nullptr,
          internal::toIntCount(recvLowerCount, "Halo accumulate block too large for MPI_Sendrecv count"), mpiType,
          prevRank, 0, comm, MPI_STATUS_IGNORE
      );

      if (loGhostRange && recvLowerCount > 0) {
        detail::accumulateGridRange(grid, *loGhostRange, recvBuffer, sendBuffer);
      }

      this->mpi.MPI_Sendrecv(
          recvLowerCount > 0 ? sendBuffer.data() : nullptr,
          internal::toIntCount(recvLowerCount, "Halo accumulate block too large for MPI_Sendrecv count"), mpiType,
          prevRank, 0, sendLowerCount > 0 ? recvBuffer.data() : nullptr,
          internal::toIntCount(sendLowerCount, "Halo accumulate block too large for MPI_Sendrecv count"), mpiType,
          nextRank, 0, comm, MPI_STATUS_IGNORE
      );
      if (hiSourceRange && sendLowerCount > 0) {
        detail::unpackGridRange(grid, *hiSourceRange, recvBuffer);
      }

      // Upper side: add incoming data to upper ghost cells, then return result to update lower inner cells in next.
      if (loSourceRange && sendUpperCount > 0) {
        detail::packGridRange(grid, *loSourceRange, sendBuffer);
      }

      this->mpi.MPI_Sendrecv(
          sendUpperCount > 0 ? sendBuffer.data() : nullptr,
          internal::toIntCount(sendUpperCount, "Halo accumulate block too large for MPI_Sendrecv count"), mpiType,
          prevRank, 0, recvUpperCount > 0 ? recvBuffer.data() : nullptr,
          internal::toIntCount(recvUpperCount, "Halo accumulate block too large for MPI_Sendrecv count"), mpiType,
          nextRank, 0, comm, MPI_STATUS_IGNORE
      );

      if (hiGhostRange && recvUpperCount > 0) {
        detail::accumulateGridRange(grid, *hiGhostRange, recvBuffer, sendBuffer);
      }

      this->mpi.MPI_Sendrecv(
          recvUpperCount > 0 ? sendBuffer.data() : nullptr,
          internal::toIntCount(recvUpperCount, "Halo accumulate block too large for MPI_Sendrecv count"), mpiType,
          nextRank, 0, sendUpperCount > 0 ? recvBuffer.data() : nullptr,
          internal::toIntCount(sendUpperCount, "Halo accumulate block too large for MPI_Sendrecv count"), mpiType,
          prevRank, 0, comm, MPI_STATUS_IGNORE
      );
      if (loSourceRange && sendUpperCount > 0) {
        detail::unpackGridRange(grid, *loSourceRange, recvBuffer);
      }

      sendScratchBuffer.reset();
      recvScratchBuffer.reset();
    }
  }

#undef SCHNEK_LOGLEVEL
#define SCHNEK_LOGLEVEL 0

}  // namespace schnek
