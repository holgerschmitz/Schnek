/*
 * field.t
 *
 * Created on: 9 May 2012
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

namespace schnek
{

template<
  typename T,
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
Field<T, rank, CheckingPolicy, StoragePolicy>::Field()
{}

template<
  typename T,
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
Field<T, rank, CheckingPolicy, StoragePolicy>
  ::Field(const IndexType &size_, const FieldRange &range_, const Stagger &stagger_, int ghostCells_)
  : Grid<T, rank, CheckingPolicy, StoragePolicy>(),
    range(range_),
    stagger(stagger_),
    size(size_),
    ghostCells(ghostCells_)
{
  IndexType low(IndexType::Zero());
  IndexType high(size);
  for (int i=0; i<rank; ++i)
  {
    low[i]  -= ghostCells;
    high[i] += ghostCells - 1;
  }
  this->resize(low, high);
}

template<
  typename T,
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
Field<T, rank, CheckingPolicy, StoragePolicy>
  ::Field(const IndexType &low_, const IndexType &high_, const FieldRange &range_, const Stagger &stagger_, int ghostCells_)
  : Grid<T, rank, CheckingPolicy, StoragePolicy>(),
    range(range_),
    stagger(stagger_),
    ghostCells(ghostCells_)
{
  IndexType low(low_);
  IndexType high(high_);
  for (int i=0; i<rank; ++i)
  {
    low[i]  -= ghostCells;
    high[i] += ghostCells;
  }
  std::cerr << "Field Constructor " << std::endl;
  for (int i=0; i<rank; ++i)
    std::cerr << low[i] << " " << high[i] << std::endl;

  this->resize(low, high);
}

template<
  typename T,
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
inline void Field<T, rank, CheckingPolicy, StoragePolicy>::positionToIndex(int dim, double pos, int &index, double &offset)
{
    double xnorm = pos*(size[dim]-2*ghostCells)/(range.getHi()[dim] - range.getLo()[dim])
        - 0.5*int(stagger[dim]) + ghostCells + this->getLo()[dim];
    index = int(xnorm);
    offset = xnorm - index;
}

template<
  typename T,
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
inline int Field<T, rank, CheckingPolicy, StoragePolicy>::positionToIndex(int dim, double pos)
{
    return int(pos*(size[dim]-2*ghostCells)/(range.getHi()[dim] - range.getLo()[dim])
        - 0.5*int(stagger[dim])) + ghostCells + this->getLo()[dim];
}

template<
  typename T,
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
inline double Field<T, rank, CheckingPolicy, StoragePolicy>::indexToPosition(int dim, int index)
{
    return (range.getHi()[dim] - range.getLo()[dim])
        * (index-this->getLo()[dim]+0.5*int(stagger[dim])-ghostCells)/(size[dim]-2*ghostCells);
}

} // namespace
