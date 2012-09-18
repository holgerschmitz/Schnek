/*
 * subgrid.t
 *
 * Created on: 18 Sep 2012
 * Author: hschmitz
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

SubGridStorage::SubGridStorage()
  : baseGrid(NULL), domain(0,0), dims(0)
{}

SubGridStorage(const IndexType &low_, const IndexType &high_, BaseGridType &baseGrid_)
  : baseGrid(&baseGrid_), domain(low_, high_)
{
  for (int d = 0; d < rank; d++)
    dims[d] = high_[d] - low_[d] + 1;
}

void resize(const IndexType &low_, const IndexType &high_)
{
  domain = DomainType(low_, high_);
  for (int d = 0; d < rank; d++)
    dims[d] = high_[d] - low_[d] + 1;
}

} // namespace schnek
