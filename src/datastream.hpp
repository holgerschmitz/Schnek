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

#include "grid/array.hpp"
#include "grid/grid.hpp"

/** A simple stream operator for the Array template class
 *
 * This operator writes out the elements of the array separated by spaces.
 */
template<
  class T, 
  int length, 
  template<int> class CheckingPolicy
>
std::ostream &operator<<(std::ostream&, const schnek::Array<T,length,CheckingPolicy>&);

/** A simple stream operator for the Grid template specialised for rank 1
 *
 * This operator writes out the elements of the grid separated by spaces.
 */
template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
std::ostream &operator<<(std::ostream&, const schnek::Grid<T, 1, CheckingPolicy, StoragePolicy>&);

/** A simple stream operator for the Grid template specialised for rank 2
 *
 * This operator writes out the elements of the grid separated by spaces.
 * Each row is followed by a newline character
 */
template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
std::ostream &operator<<(std::ostream&, const schnek::Grid<T, 2, CheckingPolicy, StoragePolicy>&);

/** A simple stream operator for the Grid template specialised for rank 3
 *
 * This operator writes out the elements of the grid separated by spaces.
 * Each row is followed by a newline character and each 2-d slice is followed
 * by an extra newline character
 */
template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
std::ostream &operator<<(std::ostream&, const schnek::Grid<T, 3, CheckingPolicy, StoragePolicy>&);

/** A simple stream operator for the Grid template specialised for rank 4
 *
 * This operator writes out the elements of the grid separated by spaces.
 * Each row is followed by a newline character and each 2-d slice is followed
 * by an extra newline character
 */
template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
std::ostream &operator<<(std::ostream&, const schnek::Grid<T, 4, CheckingPolicy, StoragePolicy>&);


/** A simple stream operator for the Grid template specialised for rank 5
 *
 * This operator writes out the elements of the grid separated by spaces.
 * Each row is followed by a newline character and each 2-d slice is followed
 * by an extra newline character
 */
template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
std::ostream &operator<<(std::ostream&, const schnek::Grid<T, 5, CheckingPolicy, StoragePolicy>&);

#include "datastream.t"


#endif // SCHNEK_DATASTREAM_H_
