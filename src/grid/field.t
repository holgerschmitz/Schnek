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
template<
  template<int> class ArrayCheckingPolicy,
  template<int> class RangeCheckingPolicy,
  template<int> class StaggerCheckingPolicy>
Field<T, rank, CheckingPolicy, StoragePolicy>
  ::Field(const Array<int,rank,ArrayCheckingPolicy> &size_,
      const Range<double, rank,RangeCheckingPolicy> &range_,
      const Array<bool, rank, StaggerCheckingPolicy> &stagger_, int ghostCells_)
  : Grid<T, rank, CheckingPolicy, StoragePolicy>(),
    range(range_),
    stagger(stagger_),
    ghostCells(ghostCells_)
{
  IndexType low(IndexType::Zero());
  IndexType high(size_);
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
template<
  template<int> class ArrayCheckingPolicy,
  template<int> class RangeCheckingPolicy,
  template<int> class StaggerCheckingPolicy>
Field<T, rank, CheckingPolicy, StoragePolicy>
  ::Field(const Array<int,rank,ArrayCheckingPolicy> &low_,
      const Array<int,rank,ArrayCheckingPolicy> &high_,
      const Range<double, rank,RangeCheckingPolicy> &range_,
      const Array<bool, rank, StaggerCheckingPolicy> &stagger_, int ghostCells_)
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
//  std::cerr << "Field Constructor " << std::endl;
//  for (int i=0; i<rank; ++i)
//    std::cerr << low[i] << " " << high[i] << std::endl;

  this->resize(low, high);
}


template<
  typename T,
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
Field<T, rank, CheckingPolicy, StoragePolicy>::Field(const Field<T, rank, CheckingPolicy, StoragePolicy> &field)
  : Grid<T, rank, CheckingPolicy, StoragePolicy>(field),
    range(field.range),
    stagger(field.stagger),
    ghostCells(field.ghostCells)
{
}

template<
  typename T,
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
inline void Field<T, rank, CheckingPolicy, StoragePolicy>::positionToIndex(int dim, double pos, int &index, double &offset)
{
    int lo = this->getLo()[dim];
    int hi = this->getHi()[dim];
    double xnorm = (pos - range.getLo()[dim])*(hi-lo-2*ghostCells+1)
        /(range.getHi()[dim] - range.getLo()[dim])
        - 0.5*int(stagger[dim]) + ghostCells + lo;
    index = int(floor(xnorm));
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
  int lo = this->getLo()[dim];
  int hi = this->getHi()[dim];
  return int(floor((pos - range.getLo()[dim])*(hi-lo-2*ghostCells+1)
      /(range.getHi()[dim] - range.getLo()[dim])
      - 0.5*int(stagger[dim]) + ghostCells + lo));
}

template<
  typename T,
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
inline double Field<T, rank, CheckingPolicy, StoragePolicy>::indexToPosition(int dim, int index)
{
  int lo = this->getLo()[dim];
  int hi = this->getHi()[dim];

//  if ((dim==0) && (index==50))
//  {
//    double pos = (range.getHi()[dim] - range.getLo()[dim])
//          * (index- lo + 0.5*int(stagger[dim])-ghostCells)/(hi - lo - 2*ghostCells + 1) + range.getLo()[dim];
//    std::cerr << index << " " << lo << " " << hi << " "
//        << range.getLo()[dim] << " " << range.getHi()[dim] << " "
//        << ghostCells << " " << int(stagger[dim]) << std::endl;
//    std::cerr << (range.getHi()[dim] - range.getLo()[dim]) << " "
//        << (index- lo + 0.5*int(stagger[dim])-ghostCells) << " "
//        << (hi - lo - 2*ghostCells + 1) << " "
//        << pos << std::endl;
//  }

  return (range.getHi()[dim] - range.getLo()[dim])
      * (index- lo + 0.5*int(stagger[dim])-ghostCells)/(hi - lo - 2*ghostCells + 1) + range.getLo()[dim];
}

} // namespace
