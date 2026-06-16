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

#include "../util/array_io.hpp"

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

    // 5b. Save old projected grid wrappers (shared ownership via shared_ptr copy).
    // This must be done before clearLocalRanges() resets the union state and
    // clears the projected grid lists.
    auto &projectedStorage = this->getProjectedGridStorage();
    std::map<long, std::list<internal::pGridWrapper>> oldProjectedGrids;
    for (const auto &entry : projectedStorage) {
      oldProjectedGrids[entry.first] = entry.second;
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

      if (oldWrapper && newWrapper) {
        redistributeGrid(oldWrapper, newWrapper, sendPlan, recvPlan);
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
void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::calcGridDistributonLocalWeights(ProcRanges &ranges) {
  const LimitType glo = this->globalRange.getLo();
  const LimitType ghi = this->globalRange.getHi();

  auto toIntCount = [](size_t count, const char *what) -> int {
    if (count > static_cast<size_t>(std::numeric_limits<int>::max())) {
      SCHNECK_FAIL(what << " is too large for an MPI int count");
    }
    return static_cast<int>(count);
  };

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
    Grid<Range<ptrdiff_t, 1>, 1> &dimRanges = ranges[d];

    const ptrdiff_t nProcs = dims[d];
    const ptrdiff_t globalCells = ghi[d] - glo[d] + 1;

    SCHNEK_ASSERT(nProcs > 0, "Invalid Cartesian process-grid dimension");
    SCHNEK_ASSERT(globalCells >= nProcs, "Cannot assign non-empty ranges: more processes than cells");

    dimRanges.resize(0, nProcs - 1);
    dimRanges(0).getLo()[0] = glo[d];
    dimRanges(nProcs - 1).getHi()[0] = ghi[d];

    const ptrdiff_t q = globalCells / nProcs;
    const ptrdiff_t r = globalCells % nProcs;

    for (ptrdiff_t p = 1; p < nProcs; ++p) {
      const ptrdiff_t cut = glo[d] + p * q + (p * r) / nProcs;
      dimRanges(p - 1).getHi()[0] = cut - 1;
      dimRanges(p).getLo()[0] = cut;
    }
  };

  auto allreduceVectorSum = [&](const std::vector<double> &local, std::vector<double> &reduced, MPI_Comm c) {
    SCHNEK_ASSERT(local.size() == reduced.size(), "Mismatched reduction buffer sizes");

    size_t offset = 0;
    while (offset < local.size()) {
      const size_t remaining = local.size() - offset;
      const size_t chunkSize =
          std::min<size_t>(remaining, static_cast<size_t>(std::numeric_limits<int>::max()));

      const int count = static_cast<int>(chunkSize);
      int errorCode = mpi.MPI_Allreduce(
          local.data() + offset,
          reduced.data() + offset,
          count,
          MPI_DOUBLE,
          MPI_SUM,
          c
      );
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
        localCells % ptrdiff_t(weightsDims[d]) == 0,
        "Local weights dimensions must evenly divide the local range"
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

  {
    Range<ptrdiff_t, rank> wRange(weightsLo, weightsHi);

    for (const LimitType &p : wRange) {
      const double w = this->localWeights[p];

      if (!(w >= 0.0)) {
        SCHNECK_FAIL("Local weights must be finite and non-negative");
      }

      for (size_t d = 0; d < rank; ++d) {
        localMarginals[d][static_cast<size_t>(p[d] - weightsLo[d])] += w;
      }
    }
  }

  for (size_t d = 0; d < rank; ++d) {
    Grid<Range<ptrdiff_t, 1>, 1> &dimRanges = ranges[d];

    const ptrdiff_t nProcs = dims[d];
    const ptrdiff_t globalCells = ghi[d] - glo[d] + 1;

    SCHNEK_ASSERT(nProcs > 0, "Invalid Cartesian process-grid dimension");
    SCHNEK_ASSERT(globalCells >= nProcs, "Cannot assign non-empty ranges: more processes than cells");

    dimRanges.resize(0, nProcs - 1);
    dimRanges(0).getLo()[0] = glo[d];
    dimRanges(nProcs - 1).getHi()[0] = ghi[d];

    if (nProcs == 1) {
      continue;
    }

    // ---------------------------------------------------------------------
    // 1. Reduce this coordinate segment over the perpendicular Cartesian slice.
    //    All ranks with the same coordinate in dimension d participate.
    // ---------------------------------------------------------------------
    MPI_Comm sliceComm = MPI_COMM_NULL;

    {
      int remainDims[rank];
      for (size_t q = 0; q < rank; ++q) {
        remainDims[q] = (q == d) ? 0 : 1;
      }

      int errorCode = mpi.MPI_Cart_sub(comm, remainDims, &sliceComm);
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
          mpi.MPI_Allreduce(&localBins, &minBins, 1, detail::mpiDatatypeFor<long>(), MPI_MIN, sliceComm);
      SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Allreduce failed while validating local weight shape");

      errorCode =
          mpi.MPI_Allreduce(&localBins, &maxBins, 1, detail::mpiDatatypeFor<long>(), MPI_MAX, sliceComm);
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
    allreduceVectorSum(localMarginals[d], reducedSegment, sliceComm);

    int errorCode = mpi.MPI_Comm_free(&sliceComm);
    SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Comm_free failed for local-weight slice communicator");

    double segmentTotal = 0.0;
    for (double w : reducedSegment) {
      segmentTotal += w;
    }

    // ---------------------------------------------------------------------
    // 2. Build an axis communicator ordered by Cartesian coordinate d.
    //    MPI_Exscan now gives the cumulative weight before this segment.
    // ---------------------------------------------------------------------
    MPI_Comm axisComm = MPI_COMM_NULL;

    {
      const int color = flattenOrthogonalCoord(d);
      const int key = static_cast<int>(myCoord[d]);

      errorCode = mpi.MPI_Comm_split(comm, color, key, &axisComm);
      SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Comm_split failed while creating local-weight axis communicator");
    }

    int axisRank = 0;
    errorCode = mpi.MPI_Comm_rank(axisComm, &axisRank);
    SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Comm_rank failed on local-weight axis communicator");

    double segmentOffset = 0.0;
    errorCode = mpi.MPI_Exscan(&segmentTotal, &segmentOffset, 1, MPI_DOUBLE, MPI_SUM, axisComm);
    SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Exscan failed while scanning local-weight segments");

    if (axisRank == 0) {
      segmentOffset = 0.0;
    }

    double totalWeight = 0.0;
    errorCode = mpi.MPI_Allreduce(&segmentTotal, &totalWeight, 1, MPI_DOUBLE, MPI_SUM, axisComm);
    SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Allreduce failed while computing total local weight");

    if (!(totalWeight > 0.0)) {
      setUniformRangesForDim(d);

      errorCode = mpi.MPI_Comm_free(&axisComm);
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
        localCuts.data(),
        globalCuts.data(),
        toIntCount(globalCuts.size(), "Number of local-weight cuts"),
        detail::mpiDatatypeFor<long>(),
        MPI_MAX,
        axisComm
    );
    SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Allreduce failed while collecting local-weight cuts");

    errorCode = mpi.MPI_Comm_free(&axisComm);
    SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Comm_free failed for local-weight axis communicator");

    // ---------------------------------------------------------------------
    // 4. Convert cuts into ProcRanges.  The clamp enforces non-empty ranges
    //    when globalCells >= nProcs, even for pathological weights concentrated
    //    in one bin.
    // ---------------------------------------------------------------------
    ptrdiff_t previousCut = glo[d];

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

      dimRanges(cutIndex - 1).getHi()[0] = cut - 1;
      dimRanges(cutIndex).getLo()[0] = cut;

      previousCut = cut;
    }
  }
}

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
    redistributeProjectedInitializers.emplace_back([](RedistributeProjectedVisitor &visitor) {
      visitor.template registerHandler<GridType>();
    });
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
    for (auto &initializer : redistributeProjectedInitializers) {
      initializer(visitor);
    }

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

    const MPI_Datatype mpiType = detail::mpiDatatypeFor<ValueType>();

    auto toIntCount = [](size_t count) -> int {
      if (count > static_cast<size_t>(std::numeric_limits<int>::max())) {
        SCHNECK_FAIL("Projected redistribution block too large for MPI count");
      }
      return static_cast<int>(count);
    };

    auto blockVolume = [](const DynProjTransferBlock &block) -> size_t {
      size_t volume = 1;
      for (size_t d = 0; d < block.lo.size(); ++d) {
        ptrdiff_t extent = block.hi[d] - block.lo[d] + 1;
        if (extent <= 0) {
          return 0;
        }
        volume *= static_cast<size_t>(extent);
      }
      return volume;
    };

    auto makeTypedRange = [](const DynProjTransferBlock &block) {
      typename GridType::IndexType lo, hi;
      for (size_t d = 0; d < projRank; ++d) {
        lo[d] = block.lo[d];
        hi[d] = block.hi[d];
      }
      return typename GridType::RangeType(lo, hi);
    };

    // Step 1: canonical-only point-to-point redistribution at projected rank.
    if (isCanonical) {
      const int localRank = ComRank;

      std::vector<std::vector<ValueType>> recvBuffers;
      std::vector<MPI_Request> requests;

      for (size_t i = 0; i < recvPlan.size(); ++i) {
        if (recvPlan[i].mpiRank == localRank) continue;
        size_t volume = blockVolume(recvPlan[i]);
        if (volume == 0) continue;
        recvBuffers.emplace_back(volume);
        MPI_Request req;
        int errorCode =
            mpi.MPI_Irecv(recvBuffers.back().data(), toIntCount(volume), mpiType, recvPlan[i].mpiRank, 1, comm, &req);
        SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Irecv failed during projected redistribution");
        requests.push_back(req);
      }

      std::vector<std::vector<ValueType>> sendBuffers;
      for (size_t i = 0; i < sendPlan.size(); ++i) {
        if (sendPlan[i].mpiRank == localRank) continue;
        size_t volume = blockVolume(sendPlan[i]);
        if (volume == 0) continue;
        sendBuffers.emplace_back(volume);
        auto &buf = sendBuffers.back();
        auto sendRange = makeTypedRange(sendPlan[i]);
        size_t idx = 0;
        for (auto it = sendRange.begin(); it != sendRange.end(); ++it) {
          buf[idx++] = oldGrid[*it];
        }
        MPI_Request req;
        int errorCode = mpi.MPI_Isend(buf.data(), toIntCount(volume), mpiType, sendPlan[i].mpiRank, 1, comm, &req);
        SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Isend failed during projected redistribution");
        requests.push_back(req);
      }

      // Local copy
      for (size_t i = 0; i < recvPlan.size(); ++i) {
        if (recvPlan[i].mpiRank != localRank) continue;
        auto copyRange = makeTypedRange(recvPlan[i]);
        for (auto it = copyRange.begin(); it != copyRange.end(); ++it) {
          newGrid[*it] = oldGrid[*it];
        }
      }

      if (!requests.empty()) {
        int errorCode = mpi.MPI_Waitall(static_cast<int>(requests.size()), requests.data(), MPI_STATUSES_IGNORE);
        SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Waitall failed during projected redistribution");
      }

      // Unpack received data
      size_t bufferIdx = 0;
      for (size_t i = 0; i < recvPlan.size(); ++i) {
        if (recvPlan[i].mpiRank == localRank) continue;
        size_t volume = blockVolume(recvPlan[i]);
        if (volume == 0) continue;
        const auto &buf = recvBuffers[bufferIdx++];
        auto recvRange = makeTypedRange(recvPlan[i]);
        size_t idx = 0;
        for (auto it = recvRange.begin(); it != recvRange.end(); ++it) {
          newGrid[*it] = buf[idx++];
        }
      }
    }

    // Step 2: broadcast the full new projected grid contents within the replica
    // sub-communicator so that non-canonical replicas see identical data.
    if (replicaComm != MPI_COMM_NULL) {
      typename GridType::RangeType fullRange(newGrid.getLo(), newGrid.getHi());
      size_t volume = 1;
      for (size_t d = 0; d < projRank; ++d) {
        ptrdiff_t extent = newGrid.getHi()[d] - newGrid.getLo()[d] + 1;
        if (extent <= 0) {
          volume = 0;
          break;
        }
        volume *= static_cast<size_t>(extent);
      }
      if (volume > 0) {
        std::vector<ValueType> buf(volume);
        if (isCanonical) {
          size_t idx = 0;
          for (auto it = fullRange.begin(); it != fullRange.end(); ++it) {
            buf[idx++] = newGrid[*it];
          }
        }
        int errorCode = mpi.MPI_Bcast(buf.data(), toIntCount(volume), mpiType, 0, replicaComm);
        SCHNEK_ASSERT(errorCode == MPI_SUCCESS, "MPI_Bcast failed during projected redistribution");
        if (!isCanonical) {
          size_t idx = 0;
          for (auto it = fullRange.begin(); it != fullRange.end(); ++it) {
            newGrid[*it] = buf[idx++];
          }
        }
      }
    }
  }

  template<size_t rank, template<size_t> class CheckingPolicy>
  void MpiCartesianDomainDecomposition<rank, CheckingPolicy>::redistributeProjectedGrids(
      const std::map<long, std::list<internal::pGridWrapper>> &oldProjectedGrids,
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
    for (auto &entry : projectedReplicaComms) {
      if (entry.second != MPI_COMM_NULL) {
        mpi.MPI_Comm_free(&entry.second);
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

    // Helpers to pack/unpack a multi-dimensional range to/from a contiguous buffer for MPI communication.
    // buffer is assumed to be pre-allocated to the correct size (range volume).
    auto packRange = [&](RangeTypeLocal range, std::pmr::vector<ValueType> &buffer) {
      size_t idx = 0;
      for (auto it = range.begin(); it != range.end(); ++it) {
        buffer[idx++] = grid[*it];
      }
    };

    auto unpackRange = [&](RangeTypeLocal range, const std::pmr::vector<ValueType> &buffer) {
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

      size_t sendLowerCount = hiSourceRange ? rangeVolume(*hiSourceRange) : 0;
      size_t recvLowerCount = loGhostRange ? rangeVolume(*loGhostRange) : 0;

      size_t sendUpperCount = loSourceRange ? rangeVolume(*loSourceRange) : 0;
      size_t recvUpperCount = hiGhostRange ? rangeVolume(*hiGhostRange) : 0;

      size_t maxSendCount = std::max(sendLowerCount, sendUpperCount);
      size_t maxRecvCount = std::max(recvLowerCount, recvUpperCount);

      mpiSendScratchBuffer.reserve_bytes(maxSendCount * sizeof(ValueType));
      mpiRecvScratchBuffer.reserve_bytes(maxRecvCount * sizeof(ValueType));

      auto sendBuffer = mpiSendScratchBuffer.make_vector<ValueType>(maxSendCount);
      auto recvBuffer = mpiRecvScratchBuffer.make_vector<ValueType>(maxRecvCount);

      // Exchange to fill lower ghost cells (receive from prev, send to next)
      if (hiSourceRange && sendLowerCount > 0) {
        packRange(*hiSourceRange, sendBuffer);
      }

      this->mpi.MPI_Sendrecv(
          sendLowerCount > 0 ? sendBuffer.data() : nullptr, toIntCount(sendLowerCount), mpiType, nextRank, 0,
          recvLowerCount > 0 ? recvBuffer.data() : nullptr, toIntCount(recvLowerCount), mpiType, prevRank, 0, comm,
          MPI_STATUS_IGNORE
      );
      if (loGhostRange && recvLowerCount > 0) {
        unpackRange(*loGhostRange, recvBuffer);
      }

      // Exchange to fill upper ghost cells (receive from next, send to prev)
      if (loSourceRange && sendUpperCount > 0) {
        packRange(*loSourceRange, sendBuffer);
      }

      this->mpi.MPI_Sendrecv(
          sendUpperCount > 0 ? sendBuffer.data() : nullptr, toIntCount(sendUpperCount), mpiType, prevRank, 0,
          recvUpperCount > 0 ? recvBuffer.data() : nullptr, toIntCount(recvUpperCount), mpiType, nextRank, 0, comm,
          MPI_STATUS_IGNORE
      );
      if (hiGhostRange && recvUpperCount > 0) {
        unpackRange(*hiGhostRange, recvBuffer);
      }

      mpiSendScratchBuffer.reset();
      mpiRecvScratchBuffer.reset();
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

    auto packRange = [&](RangeTypeLocal range, std::pmr::vector<ValueType> &buffer) {
      size_t idx = 0;
      for (auto it = range.begin(); it != range.end(); ++it) {
        buffer[idx++] = grid[*it];
      }
    };

    auto assignRange = [&](RangeTypeLocal range, const std::pmr::vector<ValueType> &buffer) {
      size_t idx = 0;
      for (auto it = range.begin(); it != range.end(); ++it) {
        grid[*it] = buffer[idx++];
      }
    };

    auto addIntoRange = [&](RangeTypeLocal range, const std::pmr::vector<ValueType> &buffer, std::pmr::vector<ValueType> &out) {
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

      const size_t sendLowerCount = hiSourceRange ? rangeVolume(*hiSourceRange) : 0;
      const size_t recvLowerCount = loGhostRange ? rangeVolume(*loGhostRange) : 0;
      const size_t sendUpperCount = loSourceRange ? rangeVolume(*loSourceRange) : 0;
      const size_t recvUpperCount = hiGhostRange ? rangeVolume(*hiGhostRange) : 0;

      const size_t maxSendCount = std::max(sendLowerCount, sendUpperCount);
      const size_t maxRecvCount = std::max(recvLowerCount, recvUpperCount);
      const size_t maxCount = std::max(maxSendCount, maxRecvCount);

      mpiSendScratchBuffer.reserve_bytes(maxCount * sizeof(ValueType));
      mpiRecvScratchBuffer.reserve_bytes(maxCount * sizeof(ValueType));

      auto sendBuffer = mpiSendScratchBuffer.make_vector<ValueType>(maxCount);
      auto recvBuffer = mpiRecvScratchBuffer.make_vector<ValueType>(maxCount);

      // Lower side: add incoming data to lower ghost cells, then return result to update upper inner cells in prev.
      if (hiSourceRange && sendLowerCount > 0) {
        packRange(*hiSourceRange, sendBuffer);
      }

      this->mpi.MPI_Sendrecv(
          sendLowerCount > 0 ? sendBuffer.data() : nullptr, toIntCount(sendLowerCount), mpiType, nextRank, 0,
          recvLowerCount > 0 ? recvBuffer.data() : nullptr, toIntCount(recvLowerCount), mpiType, prevRank, 0, comm,
          MPI_STATUS_IGNORE
      );

      if (loGhostRange && recvLowerCount > 0) {
        addIntoRange(*loGhostRange, recvBuffer, sendBuffer);
      }

      this->mpi.MPI_Sendrecv(
          recvLowerCount > 0 ? sendBuffer.data() : nullptr, toIntCount(recvLowerCount), mpiType,
          prevRank, 0, sendLowerCount > 0 ? recvBuffer.data() : nullptr, toIntCount(sendLowerCount), mpiType,
          nextRank, 0, comm, MPI_STATUS_IGNORE
      );
      if (hiSourceRange && sendLowerCount > 0) {
        assignRange(*hiSourceRange, recvBuffer);
      }

      // Upper side: add incoming data to upper ghost cells, then return result to update lower inner cells in next.
      if (loSourceRange && sendUpperCount > 0) {
        packRange(*loSourceRange, sendBuffer);
      }

      this->mpi.MPI_Sendrecv(
          sendUpperCount > 0 ? sendBuffer.data() : nullptr, toIntCount(sendUpperCount), mpiType, prevRank, 0,
          recvUpperCount > 0 ? recvBuffer.data() : nullptr, toIntCount(recvUpperCount), mpiType, nextRank, 0, comm,
          MPI_STATUS_IGNORE
      );

      if (hiGhostRange && recvUpperCount > 0) {
        addIntoRange(*hiGhostRange, recvBuffer, sendBuffer);
      }

      this->mpi.MPI_Sendrecv(
          recvUpperCount > 0 ? sendBuffer.data() : nullptr, toIntCount(recvUpperCount), mpiType,
          nextRank, 0, sendUpperCount > 0 ? recvBuffer.data() : nullptr, toIntCount(sendUpperCount), mpiType,
          prevRank, 0, comm, MPI_STATUS_IGNORE
      );
      if (loSourceRange && sendUpperCount > 0) {
        assignRange(*loSourceRange, recvBuffer);
      }

      mpiSendScratchBuffer.reset();
      mpiRecvScratchBuffer.reset();
    }
  }

#undef SCHNEK_LOGLEVEL
#define SCHNEK_LOGLEVEL 0

}  // namespace schnek
