/*
 * datastream.hpp
 *
 * Created on: 23 Jan 2007
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

#include <iostream>

#ifndef SCHNEK_DATASTREAM_H_
#define SCHNEK_DATASTREAM_H_

#include "array.hpp"
#include "grid.hpp"

template<
  class T, 
  int length, 
  template<int> class CheckingPolicy
>
      std::ostream &operator<<(std::ostream&, const schnek::Array<T,length,CheckingPolicy>&);

template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
    std::ostream &operator<<(std::ostream&, const schnek::Grid<T, 1, CheckingPolicy, StoragePolicy>&);

template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
    std::ostream &operator<<(std::ostream&, const schnek::Grid<T, 2, CheckingPolicy, StoragePolicy>&);

template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
    std::ostream &operator<<(std::ostream&, const schnek::Grid<T, 3, CheckingPolicy, StoragePolicy>&);

template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
    std::ostream &operator<<(std::ostream&, const schnek::Grid<T, 4, CheckingPolicy, StoragePolicy>&);

template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
    std::ostream &operator<<(std::ostream&, const schnek::Grid<T, 5, CheckingPolicy, StoragePolicy>&);

#include "datastream.t"


#endif // SCHNEK_DATASTREAM_H_
