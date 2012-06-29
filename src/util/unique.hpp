/*
 * unique.hpp
 *
 * Created on: 26 Jun 2012
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

#ifndef SCHNEK_UNIQUE_HPP_
#define SCHNEK_UNIQUE_HPP_

#include <boost/utility.hpp>

namespace schnek
{

template<class T>
class Unique : public boost::noncopyable
{
  private:
    static long runningId;
    long Id;
  public:
    Unique() : Id(runningId++) {}
    long getId() { return Id; }
};

template<class T>
long Unique<T>::runningId = 0;

} // namespace

#endif // SCHNEK_UNIQUE_HPP_
