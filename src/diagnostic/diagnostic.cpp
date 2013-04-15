/*
 * diagnostic.cpp
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

#include <sstream>
#include "diagnostic.hpp"

using namespace schnek;

DiagnosticInterface::DiagnosticInterface() :
  fname(""), append(false), interval(100)
{
  DiagnosticManager::instance().addDiagnostic(this);
}

void DiagnosticInterface::execute(bool master, int rank, int timeCounter)
{
  if (singleOut() && !master) return;

  if ((0 == timeCounter) && appending()) open(fname);
  if ((timeCounter % interval) == 0)
  {
    if (!appending()) open(parsedFileName(rank, timeCounter));
    write();
    if (!appending()) close();
  }
}

void DiagnosticInterface::initParameters(BlockParameters &blockPars)
{
  Block::initParameters(blockPars);

  blockPars.addParameter("file", &fname);
  blockPars.addParameter("append", &append);
  blockPars.addParameter("interval", &interval);
}

bool DiagnosticInterface::appending()
{
  return bool(append);
}

std::string DiagnosticInterface::parsedFileName(int rank, int timeCounter)
{

  std::string parsed = fname;
  std::string comrank = boost::lexical_cast<std::string>(rank);
  std::string tstep = boost::lexical_cast<std::string>(timeCounter);

  size_t pos;
  pos = parsed.find("#p");
  if (pos != std::string::npos) parsed.replace(pos, 2, comrank);
  pos = parsed.find("#t");
  if (pos != std::string::npos) parsed.replace(pos, 2, tstep);

  return parsed;
}

DiagnosticManager::DiagnosticManager() :
  master(true), rank(0)
{
}

void DiagnosticManager::addDiagnostic(DiagnosticInterface *diag)
{
  diags.push_back(diag);
}

void DiagnosticManager::execute()
{
  BOOST_FOREACH(DiagnosticInterface *diag, diags)
  {
    diag->execute(master, rank, *timecounter);
  }
}

