/*
 * boundary.t
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

namespace schnek {
        
template<
  int rank,
  template<int> class CheckingPolicy
>
Boundary<rank,CheckingPolicy>::Boundary() : size(), delta(0)
{}

template<
  int rank,
  template<int> class CheckingPolicy
>
Boundary<rank,CheckingPolicy>::Boundary(const LimitType &low, const LimitType &high, int delta_)
        : size(low, high), delta(delta_)
{}

template<
  int rank,
  template<int> class CheckingPolicy
>
Boundary<rank,CheckingPolicy>::Boundary(DomainType &size_, int delta_)
        : size(size_), delta(delta_)
{}

template<
  int rank,
  template<int> class CheckingPolicy
>
typename Boundary<rank,CheckingPolicy>::DomainType
        Boundary<rank,CheckingPolicy>::getGhostDomain(int dim, bound b)
{
  typename DomainType::LimitType boundsLo = size.getLo();
  typename DomainType::LimitType boundsHi = size.getHi();

  switch (b)
  {
    case Min:
      boundsHi[dim] = boundsLo[dim] + delta - 1;
      break;
    case Max:
    default:
      boundsLo[dim] = boundsHi[dim] - delta + 1;
      break;
  }
  return DomainType(boundsLo, boundsHi);
}

template<
  int rank,
  template<int> class CheckingPolicy
>
typename Boundary<rank,CheckingPolicy>::DomainType
        Boundary<rank,CheckingPolicy>::getGhostSourceDomain(int dim, bound b)
{
  typename DomainType::LimitType boundsLo = size.getLo();
  typename DomainType::LimitType boundsHi = size.getHi();

  switch (b)
  {
    case Min:
      boundsLo[dim] = boundsLo[dim] + delta;
      boundsHi[dim] = boundsLo[dim] + delta - 1;
      break;
    case Max:
    default:
      boundsHi[dim] = boundsHi[dim] - delta;
      boundsLo[dim] = boundsHi[dim] - delta + 1;
      break;
  }
  return DomainType(boundsLo, boundsHi);
}


template<
  int rank,
  template<int> class CheckingPolicy
>
typename Boundary<rank,CheckingPolicy>::DomainType
        Boundary<rank,CheckingPolicy>::getBoundaryDomain(int dim, bound b, bool stagger)
{
  DomainType bounds = size;
  switch (b)
  {
    case Min:
      bounds.getHi()[dim] = bounds.getLo()[dim] + delta - 1 - int(stagger);
      break;
    case Max:
    default:
      bounds.getLo()[dim] = bounds.getHi()[dim] - delta + 1;
      break;
  }
  return bounds;
}

template<
  int rank,
  template<int> class CheckingPolicy
>
typename Boundary<rank,CheckingPolicy>::DomainType
    Boundary<rank,CheckingPolicy>::getInnerDomain()
{
  typename DomainType::LimitType lo = size.getLo();
  typename DomainType::LimitType hi = size.getHi();
  for (int d=0; d<rank; ++d)
  {
    lo[d] += delta;
    hi[d] -= delta;
  }
  return DomainType(lo,hi);
}
        
template<
  int rank,
  template<int> class CheckingPolicy
>
template<class GridType>
SubGrid<GridType, CheckingPolicy> Boundary<rank,CheckingPolicy>::getGhostBoundary(int dim, bound b, GridType &grid)
{
        DomainType bounds = getGhostDomain(dim, b);
        return SubGrid<GridType, CheckingPolicy>(bounds.getLo(), bounds.getHi(), grid);
}

template<
  int rank,
  template<int> class CheckingPolicy
>
template<
  typename T,
  template<int> class CheckingPolicy2,
  template<typename, int> class StoragePolicy
>
SubGrid<Field<T,rank,CheckingPolicy2, StoragePolicy>, CheckingPolicy>
  Boundary<rank,CheckingPolicy>::getGhostBoundary(int dim, bound b, Field<T,rank,CheckingPolicy2,StoragePolicy> &field)
{
        DomainType bounds = getBoundaryDomain(dim, b, field.getStagger()[dim]);
        return SubGrid<Field<T,rank,CheckingPolicy2, StoragePolicy>, CheckingPolicy>(bounds.getLo(), bounds.getHi(), field);
}

} // namespace schnek
