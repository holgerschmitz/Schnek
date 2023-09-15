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
  size_t rank,
  template<size_t> class CheckingPolicy,
  template<typename, size_t> class StoragePolicy
>
Field<T, rank, CheckingPolicy, StoragePolicy>::Field()
{}

template<
  typename T,
  size_t rank,
  template<size_t> class CheckingPolicy,
  template<typename, size_t> class StoragePolicy
>
template<
  template<size_t> class ArrayCheckingPolicy,
  template<size_t> class RangeCheckingPolicy,
  template<size_t> class StaggerCheckingPolicy>
Field<T, rank, CheckingPolicy, StoragePolicy>::Field(
    const Array<int,rank,ArrayCheckingPolicy> &size,
    const Range<double, rank,RangeCheckingPolicy> &domain,
    const Array<bool, rank, StaggerCheckingPolicy> &stagger,
    int ghostCells
) : Grid<T, rank, CheckingPolicy, StoragePolicy>(),
    domain(domain),
    stagger(stagger),
    ghostCells(ghostCells)
{
  IndexType low{IndexType::Zero()};
  IndexType high{size};
  for (size_t i=0; i<rank; ++i)
  {
    low[i]  -= ghostCells;
    high[i] += ghostCells - 1;
  }
  this->Grid<T, rank, CheckingPolicy, StoragePolicy>::resize(low, high);
}

template<
  typename T,
  size_t rank,
  template<size_t> class CheckingPolicy,
  template<typename, size_t> class StoragePolicy
>
template<
  template<size_t> class ArrayCheckingPolicy,
  template<size_t> class RangeCheckingPolicy,
  template<size_t> class StaggerCheckingPolicy>
Field<T, rank, CheckingPolicy, StoragePolicy>::Field(
    const Array<int,rank,ArrayCheckingPolicy> &low,
    const Array<int,rank,ArrayCheckingPolicy> &high,
    const Range<double, rank,RangeCheckingPolicy> &domain,
    const Array<bool, rank, StaggerCheckingPolicy> &stagger,
    int ghostCells
) : Grid<T, rank, CheckingPolicy, StoragePolicy>(),
    domain(domain),
    stagger(stagger),
    ghostCells(ghostCells)
{
  IndexType lo{low};
  IndexType hi{high};
  for (size_t i=0; i<rank; ++i)
  {
    lo[i] -= ghostCells;
    hi[i] += ghostCells;
  }
  
  this->Grid<T, rank, CheckingPolicy, StoragePolicy>::resize(lo, hi);
}

template<
  typename T,
  size_t rank,
  template<size_t> class CheckingPolicy,
  template<typename, size_t> class StoragePolicy
>
template<
  template<size_t> class ArrayCheckingPolicy,
  template<size_t> class RangeCheckingPolicy,
  template<size_t> class StaggerCheckingPolicy>
Field<T, rank, CheckingPolicy, StoragePolicy>::Field(
    const Range<int,rank,ArrayCheckingPolicy> &range,
    const Range<double, rank,RangeCheckingPolicy> &domain,
    const Array<bool, rank, StaggerCheckingPolicy> &stagger,
    int ghostCells
) : Grid<T, rank, CheckingPolicy, StoragePolicy>(),
    domain(domain),
    stagger(stagger),
    ghostCells(ghostCells)
{
  IndexType lo{range.getLo()};
  IndexType hi{range.getHi()};
  for (size_t i=0; i<rank; ++i)
  {
    lo[i] -= ghostCells;
    hi[i] += ghostCells;
  }
  
  this->Grid<T, rank, CheckingPolicy, StoragePolicy>::resize(lo, hi);
}

template<
  typename T,
  size_t rank,
  template<size_t> class CheckingPolicy,
  template<typename, size_t> class StoragePolicy
>
Field<T, rank, CheckingPolicy, StoragePolicy>::Field(const Field<T, rank, CheckingPolicy, StoragePolicy> &field)
  : Grid<T, rank, CheckingPolicy, StoragePolicy>(field),
    domain(field.domain),
    stagger(field.stagger),
    ghostCells(field.ghostCells)
{
}

template<
  typename T,
  size_t rank,
  template<size_t> class CheckingPolicy,
  template<typename, size_t> class StoragePolicy
>
inline void Field<T, rank, CheckingPolicy, StoragePolicy>::positionToIndex(int dim, double pos, int &index, double &offset)
{
    int lo = this->getLo()[dim];
    int hi = this->getHi()[dim];
    double xnorm = (pos - domain.getLo()[dim])*(hi-lo-2*ghostCells+1)
        /(domain.getHi()[dim] - domain.getLo()[dim])
        - 0.5*int(stagger[dim]) + ghostCells + lo;
    index = int(floor(xnorm));
    offset = xnorm - index;
}

template<
  typename T,
  size_t rank,
  template<size_t> class CheckingPolicy,
  template<typename, size_t> class StoragePolicy
>
template<
  template<size_t> class ArrayCheckingPolicy,
  template<size_t> class RangeCheckingPolicy,
  template<size_t> class StaggerCheckingPolicy
>
void Field<T, rank, CheckingPolicy, StoragePolicy>::resize(
    const Array<int,rank,ArrayCheckingPolicy> &size_,
    const Range<double, rank,RangeCheckingPolicy> &domain_,
    const Array<bool, rank, StaggerCheckingPolicy> &stagger_,
    int ghostCells_
)
{
  domain = domain_;
  stagger = stagger_;
  ghostCells = ghostCells_;
  IndexType low(IndexType::Zero());
  IndexType high(size_);
  for (size_t i=0; i<rank; ++i)
  {
    low[i]  -= ghostCells;
    high[i] += ghostCells - 1;
  }
  this->Grid<T, rank, CheckingPolicy, StoragePolicy>::resize(low, high);
}

template<
  typename T,
  size_t rank,
  template<size_t> class CheckingPolicy,
  template<typename, size_t> class StoragePolicy
>
template<
  template<size_t> class ArrayCheckingPolicy,
  template<size_t> class RangeCheckingPolicy,
  template<size_t> class StaggerCheckingPolicy
>
void Field<T, rank, CheckingPolicy, StoragePolicy>::resize(
    const Array<int,rank,ArrayCheckingPolicy> &low_,
    const Array<int,rank,ArrayCheckingPolicy> &high_,
    const Range<double, rank,RangeCheckingPolicy> &domain_,
    const Array<bool, rank, StaggerCheckingPolicy> &stagger_,
    int ghostCells_
)
{
  domain = domain_;
  stagger = stagger_;
  ghostCells = ghostCells_;
  IndexType low(low_);
  IndexType high(high_);
  for (size_t i=0; i<rank; ++i)
  {
    low[i]  -= ghostCells;
    high[i] += ghostCells;
  }

  this->Grid<T, rank, CheckingPolicy, StoragePolicy>::resize(low, high);
}

template<
  typename T,
  size_t rank,
  template<size_t> class CheckingPolicy,
  template<typename, size_t> class StoragePolicy
>
template<
  template<size_t> class ArrayCheckingPolicy,
  template<size_t> class RangeCheckingPolicy,
  template<size_t> class StaggerCheckingPolicy
>
void Field<T, rank, CheckingPolicy, StoragePolicy>::resize(
    const Range<int,rank,ArrayCheckingPolicy> &range_,
    const Range<double, rank,RangeCheckingPolicy> &domain_,
    const Array<bool, rank, StaggerCheckingPolicy> &stagger_,
    int ghostCells_
)
{
  domain = domain_;
  stagger = stagger_;
  ghostCells = ghostCells_;
  IndexType low(range_.getLo());
  IndexType high(range_.getHi());
  for (size_t i=0; i<rank; ++i)
  {
    low[i]  -= ghostCells;
    high[i] += ghostCells;
  }

  this->Grid<T, rank, CheckingPolicy, StoragePolicy>::resize(low, high);
}


template<
  typename T,
  size_t rank,
  template<size_t> class CheckingPolicy,
  template<typename, size_t> class StoragePolicy
>
inline int Field<T, rank, CheckingPolicy, StoragePolicy>::positionToIndex(int dim, double pos)
{
  int lo = this->getLo()[dim];
  int hi = this->getHi()[dim];
  return int(floor((pos - domain.getLo()[dim])*(hi-lo-2*ghostCells+1)
      /(domain.getHi()[dim] - domain.getLo()[dim])
      - 0.5*int(stagger[dim]) + ghostCells + lo));
}

template<
  typename T,
  size_t rank,
  template<size_t> class CheckingPolicy,
  template<typename, size_t> class StoragePolicy
>
inline double Field<T, rank, CheckingPolicy, StoragePolicy>::indexToPosition(int dim, int index)
{
  int lo = this->getLo()[dim];
  int hi = this->getHi()[dim];

//  if ((dim==0) && (index==50))
//  {
//    double pos = (domain.getHi()[dim] - domain.getLo()[dim])
//          * (index- lo + 0.5*int(stagger[dim])-ghostCells)/(hi - lo - 2*ghostCells + 1) + domain.getLo()[dim];
//    std::cerr << index << " " << lo << " " << hi << " "
//        << domain.getLo()[dim] << " " << domain.getHi()[dim] << " "
//        << ghostCells << " " << int(stagger[dim]) << std::endl;
//    std::cerr << (domain.getHi()[dim] - domain.getLo()[dim]) << " "
//        << (index- lo + 0.5*int(stagger[dim])-ghostCells) << " "
//        << (hi - lo - 2*ghostCells + 1) << " "
//        << pos << std::endl;
//  }

  return (domain.getHi()[dim] - domain.getLo()[dim])
      * (index- lo + 0.5*int(stagger[dim])-ghostCells)/(hi - lo - 2*ghostCells + 1) + domain.getLo()[dim];
}

} // namespace
