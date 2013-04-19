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

#include <boost/shared_ptr.hpp>

namespace schnek {

/** @brief Interface for wrapping and exchanging boundaries .
 *
 *  This interface is used to exchange the boundaries of distribution
 *  functions and scalar fields between processes. Any implementation should
 *  treat the fields as periodic. Othe boundary conditions can be appplied afterwards.
 */
template<class GridType>
class DomainSubdivision {
  public:
    enum {Rank = GridType::Rank};

    typedef typename GridType::IndexType LimitType;
    typedef typename GridType::value_type value_type;
    typedef typename GridType::CheckingPolicyType CheckingPolicyType;
    typedef typename GridType::StoragePolicyType StoragePolicyType;

    typedef Grid<unsigned char, 1, GridNoArgCheck, LazyArrayGridStorage> BufferType;

    typedef RecDomain<Rank> DomainType;
    typedef Boundary<Rank> BoundaryType;
    typedef boost::shared_ptr<BoundaryType> pBoundaryType;

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

    /** Initialize the boundary with the extent of the global domain
     *
     *  The DomainSubdivision class is responsible for subdividing the domain for
     *  the different processes. The size of the local domain will be returned
     *  by the getDomain, getHi, and getLo methods.
     */
    virtual void init(const LimitType &low, const LimitType &high, int delta);

    /** Convenience method.
     *  Initialise the boundary with the extent of a grid.
     */
    void init(const DomainType &domain, int delta)
    {
      init(domain.getLo(), domain.getHi, delta);
    }

    /** Convenience method.
     *  Initialise the boundary with the extent of the grid.
     */
    void init(const GridType &grid, int delta)
    {
      init(grid.getLo(), grid.getHi, delta);
    }

    /// Return the local domain size
    const DomainType &getDomain() { return bounds->getDomain(); }
    /// Return rectangle minimum
    const LimitType &getLo() const {return bounds->getDomain().getLo();}
    /// Return rectangle maximum
    const LimitType &getHi() const {return bounds->getDomain().getHi();}

    /** @brief Exchange the boundaries of a field function
     *  in the direction given by dim.
     */
    virtual void exchange(GridType &grid, int dim) = 0;

    virtual void exchangeData(int dim, int orientation, BufferType &in, BufferType &out) = 0;

    /// Return the average of a single value over all the processes
    virtual double avgReduce(double) const = 0;

    /// Return the average of a single value over all the processes
    virtual double sumReduce(double) const = 0;

    /// Return the maximum of a single value over all the processes
    virtual double maxReduce(double) const = 0;

    /// Return true if this is the master process and false otherwise
    virtual bool master() const = 0;

    /// Return the process number
    virtual int procnum() const = 0;

    /// get a unique Id
    virtual int getUniqueId() const = 0;

    /** Returns true if this process is on the lower bound of the
     * global domain
     *
     * @param dim The dimension in which to inspect
     * @return A boolean indicating if this process is on the lower boud of the
     * global domain
     */
    virtual bool isBoundLo(int dim) = 0;

    /** Returns true if this process is on the upper bound of the
     * global domain
     *
     * @param dim The dimension in which to inspect
     * @return A boolean indicating if this process is on the upper boud of the
     * global domain
     */
    virtual bool isBoundHi(int dim) = 0;
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

    SerialSubdivision();

    ~SerialSubdivision();

    /** @brief Exchanges the boundaries in x-direction.
     *
     *  The two outmost simulated cells are sent and the surrounding
     *  two ghost cells are filled with values
     */
    void exchange(GridType &grid, int dim);

    void exchangeData(int dim, int orientation, BufferType &in, BufferType &out);

    /// The average of a single value is the value
    double avgReduce(double val) const { return val; }

    /// The maximum of a single value is the value
    double maxReduce(double val) const { return val; }

    /// The sum of a single value is the value
    double sumReduce(double val) const { return val; }

    /// The process with the rank zero is designated master process
    bool master() const { return true; }

    /// Returns the comm rank as given by mpi
    int procnum() const { return 0; }

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
    bool isBoundLo(int) { return true; }

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
    bool isBoundHi(int) { return true; }

};


//DomainSubdivision

} // namespace schnek

#include "domainsubdivision.t"

#endif // SCHNEK_DOMAINSUBDIVISION_HPP

