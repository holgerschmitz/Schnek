/*
 * gridtransform.t
 *
 * Created on: 24 Sep 2012
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
  class BaseGrid,
  typename Transformation
>
GridTransformStorage<T, rank, BaseGrid, Transformation>::GridTransformStorage()
  : baseGrid(NULL)
{}

template<
  class BaseGrid,
  typename Transformation,
  template<int> class CheckingPolicy
>
GridTransform<BaseGrid, Transformation, CheckingPolicy>::GridTransform()
  : ParentType()
{}

template<
  class BaseGrid,
  typename Transformation,
  template<int> class CheckingPolicy
>
GridTransform<BaseGrid, Transformation, CheckingPolicy>::GridTransform(BaseGridType &baseGrid_)
  : ParentType(size)
{
    setBaseGrid(baseGrid_);
}


} // namespace schnek
