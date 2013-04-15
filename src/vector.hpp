/*
 * vector.hpp
 *
 * Created on: 8 Oct 2010
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

#ifndef SCHNEK_VECTOR_H_
#define SCHNEK_VECTOR_H_

#include "grid/array.hpp"

namespace schnek {

template<
  class T,
  int length,
  template<int> class CheckingPolicy = ArrayNoArgCheck
>
class Vector : public Array<T,length,CheckingPolicy>
{
  public:
    T product() const;
    T sum() const;
};

}

#endif // SCHNEK_VECTOR_H_
