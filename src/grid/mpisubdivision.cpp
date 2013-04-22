/*
 * mpisubdivision.cpp
 *
 * Created on: 22 Apr 2013
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

#include "mpisubdivision.hpp"

using namespace schnek;


/* **************************************************************
 *                 MpiValueType                                 *
 ****************************************************************/

template<>
const MPI_Datatype MpiValueType<signed char>::value = MPI_CHAR;

template<>
const MPI_Datatype MpiValueType<signed short int>::value = MPI_SHORT;

template<>
const MPI_Datatype MpiValueType<signed int>::value = MPI_INT;

template<>
const MPI_Datatype MpiValueType<signed long int>::value = MPI_LONG;

template<>
const MPI_Datatype MpiValueType<unsigned char>::value = MPI_UNSIGNED_CHAR;

template<>
const MPI_Datatype MpiValueType<unsigned short int>::value = MPI_UNSIGNED_SHORT;

template<>
const MPI_Datatype MpiValueType<unsigned int>::value = MPI_UNSIGNED;

template<>
const MPI_Datatype MpiValueType<unsigned long int>::value = MPI_UNSIGNED_LONG;

template<>
const MPI_Datatype MpiValueType<float>::value = MPI_FLOAT;

template<>
const MPI_Datatype MpiValueType<double>::value = MPI_DOUBLE;

template<>
const MPI_Datatype MpiValueType<long double>::value = MPI_LONG_DOUBLE;


