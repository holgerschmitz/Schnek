/*
 * field.hpp
 *
 *  Created on: 9 May 2012
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
 */

#ifndef FIELD_HPP_
#define FIELD_HPP_

#include "range.hpp"
#include "domain.hpp"

namespace schnek {

template<
  typename T,
  int rank,
  template<int> class CheckingPolicy = GridNoArgCheck,
  template<typename, int> class StoragePolicy = SingleArrayGridStorage
>
class Field : public Grid<T, rank, CheckingPolicy, StoragePolicy>
{
  public:
    typedef Range<double, rank> FieldRange;
    typedef RecDomain<rank> FieldDomain;
    typedef Array<bool, rank> Stagger;
  private:
    FieldRange range;
    Stagger stagger;
    /// Stores the size of the simulation domain
    IndexType size;

    static IndexType &calcGridSize(const IndexType &size, const Stagger &stagger);
  public:
    /** default constructor creates an empty grid */
    Field();

    /** Constructs a grid with a given number of cells in each direction
     *
     * The actual number of grid points depends on the stagger.
     * For a non-staggered grid, the grid contains one more point, because the
     * boundaries lie at 0 and N.
     */
    Field(const IndexType &size, const FieldRange &range_, const Stagger &stagger_);

    /** copy constructor */
    Field(const Grid<T, rank, CheckingPolicy, StoragePolicy>&);

    /** Calculates index and offset from a position on the field
     *
     * The index returned is the index of the grid point left of the position.
     */
    void positionToIndex(int dim, double pos, int &index, double &offset);

    /** Calculates index from a position on the field
     *
     * The index returned is the index of the grid point left of the position.
     */
    int positionToIndex(int dim, double pos);

    /// Calculates the position of a grid point
    double indexToPosition(int dim, int index);
};

} //namespace

#include field.t

#endif /* FIELD_HPP_ */
