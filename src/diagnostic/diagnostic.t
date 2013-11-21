/*
 * diagnostic.t
 *
 * Created on: 23 Oct 2012
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


#include "../util/logger.hpp""

#undef LOGLEVEL
#define LOGLEVEL 0

namespace schnek {


template<class Type, typename PointerType>
SimpleDiagnostic<Type, PointerType>::~SimpleDiagnostic()
{
  close();
}

template<class Type, typename PointerType>
void SimpleDiagnostic<Type, PointerType>::initParameters(BlockParameters &blockPars)
{
  DiagnosticInterface::initParameters(blockPars);
  blockPars.addParameter("field", &fieldName);
}

template<class Type, typename PointerType>
void SimpleDiagnostic<Type, PointerType>::init()
{
  if (!isDerived()) retrieveData(fieldName, field);
  SCHNEK_TRACE_LOG(2, "got field " << field)
}

template<class Type, typename PointerType>
void SimpleFileDiagnostic<Type, PointerType>::open(const std::string &fname)
{
  output.open(fname.c_str());
//  output.precision(14);
}

template<class Type, typename PointerType>
void SimpleFileDiagnostic<Type, PointerType>::write()
{
  output << *(this->field);
}

template<class Type, typename PointerType>
void SimpleFileDiagnostic<Type, PointerType>::close()
{
  output.close();
}


#undef LOGLEVEL
#define LOGLEVEL 0


} // namespace schnek
