/*
 * domainsubdivision.hpp
 *
 * Created on: 27 Sep 2012
 * Author: Holger Schmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2012 Holger Schmitz
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

/** @file domainsubdivision.hpp
 *  @brief Interface for wrapping and exchanging boundaries
 *
 *  This interface is used to exchange the boundaries of distribution
 *  functions and scalar fields. It can be implemented to define
 *  periodic boundaries or exchange data with other processes.
 */

#ifndef SCHNEK_DOMAINSUBDIVISION_HPP
#define SCHNEK_DOMAINSUBDIVISION_HPP

#include "boundary.hpp"

#include <memory>

namespace schnek {

/** @brief Interface for wrapping and exchanging boundaries .
 *
 *  This interface is used to exchange the boundaries of grids
 *  between processes. Any implementation should treat the fields as periodic.
 *  The boundary conditions can be appplied afterwards.
 */
template<class GridType>
class DomainSubdivision {
  public:
    enum {Rank = GridType::Rank};

    typedef typename GridType::IndexType LimitType;
    typedef typename GridType::value_type value_type;
    typedef typename GridType::CheckingPolicyType CheckingPolicyType;
    typedef typename GridType::StoragePolicyType StoragePolicyType;

    //typedef Grid<unsigned char, 1, GridAssertCheck, LazyArrayGridStorage> BufferType;
    typedef Grid<unsigned char, 1, GridAssertCheck> BufferType;

    typedef Range<int, Rank> DomainType;
    typedef Boundary<Rank> BoundaryType;
    typedef std::shared_ptr<BoundaryType> pBoundaryType;

  protected:
    pBoundaryType bounds;
  public:

    /// Default constructor
    DomainSubdivision() {}

    /** @brief Virtual destructor
     *
     *  Needed for virtual methods.
     */
    virtual ~DomainSubdivision() {}

    int getDelta() { return bounds->getDelta(); }

    /** Initialize the domain subdivision.
     *
     *  The DomainSubdivision class is responsible for subdividing the domain for
     *  the different processes. The size of the local domain will be returned
     *  by the getDomain, getHi, and getLo methods.
     */
    virtual void init(const LimitType &low, const LimitType &high, int delta) = 0;

    /** Convenience method.
     *  Initialise the boundary with the extent of a grid.
     */
    void init(const DomainType &domain, int delta)
    {
      init(domain.getLo(), domain.getHi(), delta);
    }

    /** Convenience method.
     *  Initialise the boundary with the extent of the grid.
     */
    void init(const GridType &grid, int delta)
    {
      init(grid.getLo(), grid.getHi(), delta);
    }

    /** Convenience method.
     *  Initialise the boundary with the extent of the grid.
     */
    void init(const LimitType &size, int delta)
    {
      LimitType sizem(size);
      for (size_t i=0; i<Rank; ++i) --sizem[i];
      init(LimitType(0), sizem, delta);
    }

    /// Return the global domain size excluding ghost cells
    virtual const DomainType &getGlobalDomain() const = 0;

    /// Return the local domain size
    const DomainType &getDomain() const { return bounds->getDomain(); }
    /// Return the minimum of the local domain
    const LimitType &getLo() const {return bounds->getDomain().getLo();}
    /// Return the maximum of the local domain
    const LimitType &getHi() const {return bounds->getDomain().getHi();}

    /// Return the local inner domain size
    DomainType getInnerDomain() const { return bounds->getInnerDomain(); }
    /// Return the minimum of the local inner domain
    LimitType getInnerLo() const {return bounds->getInnerDomain().getLo();}
    /// Return the maximum of the local inner domain
    LimitType getInnerHi() const {return bounds->getInnerDomain().getHi();}


    /// Return the local inner physical extent
    template<typename T, template<size_t> class CheckingPolicy>
    Range<T, Rank, CheckingPolicy> getInnerExtent(const Range<T, Rank, CheckingPolicy> &globalExtent) const
    {
      const DomainType &globalGridSize = this->getGlobalDomain();
      typename Range<T, Rank, CheckingPolicy>::LimitType localDomainMin, localDomainMax;
      for (size_t i=0; i<Rank; ++i)
      {
        const T &lo = globalExtent.getLo()[i];
        T dx = (globalExtent.getHi()[i]-lo) / (T)(globalGridSize.getHi()[i] - globalGridSize.getLo()[i] + 1);
        localDomainMin[i] = lo + this->getInnerLo()[i]*dx;
        localDomainMax[i] = lo + (this->getInnerHi()[i] + 1)*dx;
      }
      return Range<T, Rank, CheckingPolicy>(localDomainMin,localDomainMax);
    }

    /** Return the local inner physical extent
     *
     * Convenience method. This assumes the lower bound of the global extent is 0.0
     */
    template<typename T, template<size_t> class CheckingPolicy>
    Range<T, Rank, CheckingPolicy> getInnerExtent(const Array<T, Rank, CheckingPolicy> &globalExtent) const
    {
      return this->getInnerExtent(
          Range<T, Rank, CheckingPolicy>(Array<T, Rank, CheckingPolicy>(0), globalExtent));
    }

    /** @brief Exchange the boundaries of a field function
     *  in the direction given by dim.
     */
    virtual void exchange(GridType &grid, size_t dim) = 0;

    /** @brief Exchange the boundaries of a field function
     *  summing the data from ghost cells and inner cells
     */
    virtual void accumulate(GridType &grid, size_t dim) = 0;

    virtual void exchangeData(size_t dim, int orientation, BufferType &in, BufferType &out) = 0;

    /// Return the average of a single value over all the processes
    virtual double avgReduce(double) const = 0;

    /// Return the average of a single value over all the processes
    virtual int avgReduce(int) const = 0;

    /// Return the average of a single value over all the processes
    virtual double sumReduce(double) const = 0;

    /// Return the average of a single value over all the processes
    virtual int sumReduce(int) const = 0;

    /// Return the maximum of a single value over all the processes
    virtual double maxReduce(double) const = 0;

    /// Return the maximum of a single value over all the processes
    virtual int maxReduce(int) const = 0;

    /// Return the minimum of a single value over all the processes
    virtual double minReduce(double) const = 0;

    /// Return the minimum of a single value over all the processes
    virtual int minReduce(int) const = 0;


    /// Return true if this is the master process and false otherwise
    virtual bool master() const = 0;

    /// Return the process number
    virtual int procnum() const = 0;

    /// Return the total number of processes
    virtual int procCount() const = 0;

    /// get a unique Id
    virtual int getUniqueId() const = 0;

    /** Returns true if this process is on the lower bound of the
     * global domain
     *
     * @param dim The dimension in which to inspect
     * @return A boolean indicating if this process is on the lower boud of the
     * global domain
     */
    virtual bool isBoundLo(size_t dim) = 0;

    /** Returns true if this process is on the upper bound of the
     * global domain
     *
     * @param dim The dimension in which to inspect
     * @return A boolean indicating if this process is on the upper boud of the
     * global domain
     */
    virtual bool isBoundHi(size_t dim) = 0;

    void exchange(GridType &grid) {
      for (size_t i=0; i<Rank; ++i) exchange(grid,i);
    }

    void accumulate(GridType &grid) {
      for (size_t i=0; i<Rank; ++i) accumulate(grid,i);
    }
};

template<class GridType>
class SerialSubdivision : public DomainSubdivision<GridType>
{
  public:
    typedef typename Boundary<GridType::Rank>::LimitType LimitType;
    typedef typename GridType::value_type value_type;
    typedef typename DomainSubdivision<GridType>::DomainType DomainType;
    typedef typename DomainSubdivision<GridType>::BoundaryType BoundaryType;
    typedef typename DomainSubdivision<GridType>::BufferType BufferType;

  private:
    /// The positions of the lower corner of the local piece of the grid
    LimitType Low;

    /// The positions of the upper corner of the local piece of the grid
    LimitType High;
  public:
    using DomainSubdivision<GridType>::init;
    using DomainSubdivision<GridType>::exchange;

    SerialSubdivision();

    ~SerialSubdivision();

    /** Initialize the boundary with the extent of the global domain
     *
     *  No subdivision is carried out for serial simulations.
     *  The size of the global domain will be returned
     *  by the getDomain, getHi, and getLo methods.
     */
    void init(const LimitType &low, const LimitType &high, int delta);

    /// Return the global domain size excluding ghost cells
    const DomainType &getGlobalDomain() const { return this->bounds->getDomain(); }

    /** @brief Exchanges the boundaries in direction specified by dim.
     *
     *  The outermost simulated cells are sent and the surrounding
     *  ghost cells are filled with values
     */
    void exchange(GridType &grid, size_t dim);

    /** @brief Exchange the boundaries of a field function
     *  summing the data from ghost cells and inner cells
     */
    void accumulate(GridType &grid, size_t dim);

    void exchangeData(size_t dim, int orientation, BufferType &in, BufferType &out);

    /// The average of a single value is the value
    double avgReduce(double val) const { return val; }

    /// The average of a single value is the value
    int avgReduce(int val) const { return val; }

    /// The maximum of a single value is the value
    double maxReduce(double val) const { return val; }

    /// The maximum of a single value is the value
    int maxReduce(int val) const { return val; }

    /// Return the minimum of a single value over all the processes
    double minReduce(double val) const { return val; }

    /// Return the minimum of a single value over all the processes
    int minReduce(int val) const { return val; }

    /// The sum of a single value is the value
    double sumReduce(double val) const { return val; }

    /// The sum of a single value is the value
    int sumReduce(int val) const { return val; }

    /// The process with the rank zero is designated master process
    bool master() const { return true; }

    /// Returns the comm rank as given by mpi
    int procnum() const { return 0; }

    /// Return the total number of processes
    int procCount() const { return 1; }

    ///returns an ID, which consists of the Dimensions and coordinates
    int getUniqueId() const { return 0; }

    /** Returns true if this process is on the lower bound of the
     * global domain
     *
     *  The serial domain subdivision does not subdivide. It is always on the bound
     *  of the domain.
     *
     * @param dim The dimension in which to inspect
     * @return A boolean indicating if this process is on the lower boud of the
     * global domain
     */
    bool isBoundLo(size_t) { return true; }

    /** Returns true if this process is on the upper bound of the
     * global domain.
     *
     *  The serial domain subdivision does not subdivide. It is always on the bound
     *  of the domain.
     *
     * @param dim The dimension in which to inspect
     * @return A boolean indicating if this process is on the upper boud of the
     * global domain
     */
    bool isBoundHi(size_t) { return true; }

};


//DomainSubdivision

} // namespace schnek

#include "domainsubdivision.t"

#endif // SCHNEK_DOMAINSUBDIVISION_HPP

