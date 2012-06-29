/*
 * functions.hpp
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

#ifndef SCHNEK_FUNCTIONS_H_
#define SCHNEK_FUNCTIONS_H_

namespace schnek {

template<typename T>
T min(T a, T b) { return a>b?b:a; }

template<typename T>
T max(T a, T b) { return a>b?a:b; }

template<typename T>
int signum0(T x) { return x>0?1:(x<0?-1:0); }

template<typename T>
int signum(T x) { return x<0?-1:1; }

double drand();

int irand(int range);

} // namespace

#endif // SCHNEK_FUNCTIONS_H_
