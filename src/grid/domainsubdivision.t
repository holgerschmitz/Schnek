/*
 * domainsubdivision.t
 *
 * Created on: 3 Oct 2012
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

#include <boost/make_shared.hpp>

namespace schnek {

template<class GridType>
SerialSubdivision<GridType>::SerialSubdivision()
{}

template<class GridType>
SerialSubdivision<GridType>::~SerialSubdivision()
{}

template<class GridType>
void SerialSubdivision<GridType>::init(const LimitType &low, const LimitType &high, int delta)
{
  this->bounds = boost::make_shared<BoundaryType>(low, high, delta);
}

template<class GridType>
void SerialSubdivision<GridType>::exchange(GridType &grid, int dim)
{
  DomainType loGhost = this->bounds->getGhostDomain(dim, BoundaryType::Min);
  DomainType hiGhost = this->bounds->getGhostDomain(dim, BoundaryType::Max);
  DomainType loSource = this->bounds->getGhostSourceDomain(dim, BoundaryType::Min);
  DomainType hiSource = this->bounds->getGhostSourceDomain(dim, BoundaryType::Max);

  {
    typename DomainType::iterator loIt = loGhost.begin();
    typename DomainType::iterator hiIt = hiSource.begin();
    typename DomainType::iterator loEnd = loGhost.end();

    while (loIt != loEnd)
    {
      grid[*loIt] = grid[*hiIt];
      ++loIt; ++hiIt;
    }
  }

  {
    typename DomainType::iterator loIt = loSource.begin();
    typename DomainType::iterator hiIt = hiGhost.begin();
    typename DomainType::iterator loEnd = loSource.end();

    while (loIt != loEnd)
    {
      grid[*hiIt] = grid[*loIt];
      ++loIt; ++hiIt;
    }
  }
}

template<class GridType>
void SerialSubdivision<GridType>::accumulate(GridType &grid, int dim)
{
  DomainType loGhost = this->bounds->getGhostDomain(dim, BoundaryType::Min);
  DomainType hiGhost = this->bounds->getGhostDomain(dim, BoundaryType::Max);
  DomainType loSource = this->bounds->getGhostSourceDomain(dim, BoundaryType::Min);
  DomainType hiSource = this->bounds->getGhostSourceDomain(dim, BoundaryType::Max);

  {
    typename DomainType::iterator loIt = loGhost.begin();
    typename DomainType::iterator hiIt = hiSource.begin();
    typename DomainType::iterator loEnd = loGhost.end();

    while (loIt != loEnd)
    {
      grid[*loIt] += grid[*hiIt];
      grid[*hiIt] = grid[*loIt];
      ++loIt; ++hiIt;
    }
  }

  {
    typename DomainType::iterator loIt = loSource.begin();
    typename DomainType::iterator hiIt = hiGhost.begin();
    typename DomainType::iterator loEnd = loSource.end();

    while (loIt != loEnd)
    {
      grid[*hiIt] += grid[*loIt];
      grid[*loIt] = grid[*hiIt];
      ++loIt; ++hiIt;
    }
  }
}

template<class GridType>
void SerialSubdivision<GridType>::exchangeData(int dim, int orientation, BufferType &in, BufferType &out)
{
  out = in;
}

} // namespace schnek
