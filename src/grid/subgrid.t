/*
 * subgrid.t
 *
 * Created on: 18 Sep 2012
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
  class BaseGrid
>
SubGridStorage<T, rank, BaseGrid>::SubGridStorage()
  : baseGrid(NULL), domain(0,0), dims(0)
{}

template<
  typename T,
  size_t rank,
  class BaseGrid
>
SubGridStorage<T, rank, BaseGrid>::SubGridStorage(const IndexType &low_, const IndexType &high_)
  : baseGrid(NULL), domain(low_, high_)
{
  for (size_t d = 0; d < rank; d++)
    dims[d] = high_[d] - low_[d] + 1;
}

template<
  typename T,
  size_t rank,
  class BaseGrid
>
void SubGridStorage<T, rank, BaseGrid>::resize(const IndexType &low_, const IndexType &high_)
{
    domain = DomainType(low_, high_);
}

template<
  class BaseGrid,
  template<size_t> class CheckingPolicy
>
SubGrid<BaseGrid, CheckingPolicy>::SubGrid()
  : ParentType()
{}

template<
  class BaseGrid,
  template<size_t> class CheckingPolicy
>
SubGrid<BaseGrid, CheckingPolicy>::SubGrid(const IndexType &size, BaseGridType &baseGrid_)
  : ParentType(size)
{
	this->check(this->getLo(), baseGrid_.getLo(), baseGrid_.getHi());
	this->check(this->getHi(), baseGrid_.getLo(), baseGrid_.getHi());
    this->setBaseGrid(baseGrid_);
}

template<
  class BaseGrid,
  template<size_t> class CheckingPolicy
>
SubGrid<BaseGrid, CheckingPolicy>::SubGrid(const IndexType &low, const IndexType &high, BaseGridType &baseGrid_)
  : ParentType(low, high)
{
	this->check(this->getLo(), baseGrid_.getLo(), baseGrid_.getHi());
	this->check(this->getHi(), baseGrid_.getLo(), baseGrid_.getHi());
    this->setBaseGrid(baseGrid_);
}

template<
  class BaseGrid,
  template<size_t> class CheckingPolicy
>
SubGrid<BaseGrid, CheckingPolicy>::SubGrid(const RangeType &range, BaseGridType &baseGrid_)
  : ParentType(range.getLo(), range.getHi())
{
  this->check(this->getLo(), baseGrid_.getLo(), baseGrid_.getHi());
  this->check(this->getHi(), baseGrid_.getLo(), baseGrid_.getHi());
  this->setBaseGrid(baseGrid_);
}



} // namespace schnek
