/*
 * algo.hpp
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

#ifndef SCHNEK_ALGO_H_
#define SCHNEK_ALGO_H_

namespace schnek {

/** Calculate the average of the values provided.
 *
 * InputIterator::value_type must support
 * - zero initialisation
 * - += operator
 * - division by a double
 *
 * @param begin An iterator pointing to the start of the value range
 * @param end An iterator pointing behind the end of the value range
 * @return The average of all the values in the range
 */
template<class InputIterator>
typename InputIterator::value_type mean(InputIterator begin, InputIterator end);

#include "algo.t"

}


#endif // SCHNEK_ALGO_H_
