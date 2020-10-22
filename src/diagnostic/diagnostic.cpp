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

#include "diagnostic.hpp"

#include <sstream>

#include "../util/logger.hpp"

using namespace schnek;

DiagnosticInterface::DiagnosticInterface() : fname(""), append(false) {}

void DiagnosticInterface::initParameters(BlockParameters &blockPars) {
  Block::initParameters(blockPars);

  blockPars.addParameter("file", &fname);
  blockPars.addParameter("append", &append, 0);
}

bool DiagnosticInterface::appending() {
  return bool(append);
}

std::string DiagnosticInterface::parsedFileName(int mpiRank, ptrdiff_t timeCounter) {
  std::string parsed = fname;
  std::string comrank = boost::lexical_cast<std::string>(mpiRank);
  std::string tstep = boost::lexical_cast<std::string>(timeCounter);
  if (timeCounter < 0) tstep = "final";

  SCHNEK_TRACE_LOG(2, "DiagnosticInterface::parsedFileName " << mpiRank << " " << comrank << " " << timeCounter)
  size_t pos;

#if !defined(H5_HAVE_PARALLEL) || !defined(SCHNEK_USE_HDF_PARALLEL)
  pos = parsed.find("#p");
  if (pos != std::string::npos) parsed.replace(pos, 2, comrank);
#endif

  pos = parsed.find("#t");
  if (pos != std::string::npos) parsed.replace(pos, 2, tstep);

  return parsed;
}

std::string DiagnosticInterface::parsedFileName(int mpiRank, double physicalTime) {
  std::string parsed = fname;
  std::string comrank = boost::lexical_cast<std::string>(mpiRank);
  std::string time = boost::lexical_cast<std::string>(physicalTime);

  SCHNEK_TRACE_LOG(2, "DiagnosticInterface::parsedFileName " << mpiRank << " " << comrank << " " << timeCounter)
  size_t pos;

#if !defined(H5_HAVE_PARALLEL) || !defined(SCHNEK_USE_HDF_PARALLEL)
  pos = parsed.find("#p");
  if (pos != std::string::npos) parsed.replace(pos, 2, comrank);
#endif

  pos = parsed.find("#t");
  if (pos != std::string::npos) parsed.replace(pos, 2, time);

  return parsed;
}

IntervalDiagnostic::IntervalDiagnostic() : interval(100) {
  DiagnosticManager::instance().addIntervalDiagnostic(this);
}

void IntervalDiagnostic::execute(bool master, int mpiRank, ptrdiff_t timeCounter) {
  SCHNEK_TRACE_LOG(2, "IntervalDiagnostic::execute" << fname << " " << mpiRank)
  if (singleOut() && !master) return;

  if ((0 == timeCounter) && appending()) open(fname);
  if ((timeCounter < 0) || ((timeCounter % interval) == 0)) {
    if (!appending()) open(parsedFileName(mpiRank, timeCounter));
    write();
    if (!appending()) close();
  }
}

void IntervalDiagnostic::initParameters(BlockParameters &blockPars) {
  DiagnosticInterface::initParameters(blockPars);
  blockPars.addParameter("interval", &interval, 100);
}

int IntervalDiagnostic::getInterval() {
  return interval;
}

DeltaTimeDiagnostic::DeltaTimeDiagnostic() : deltaTime(1.0), nextOutput(0.0), count(0) {
  DiagnosticManager::instance().addDeltaTimeDiagnostic(this);
}

void DeltaTimeDiagnostic::execute(bool master, int mpiRank, double physicalTime) {
  SCHNEK_TRACE_LOG(2, "DeltaTimeDiagnostic::execute" << fname << " " << mpiRank)
  if (singleOut() && !master) return;

  if ((0.0 == physicalTime) && appending()) open(fname);

  if (physicalTime >= nextOutput) {
    if (!appending()) open(parsedFileName(mpiRank, count));
    write();
    if (!appending()) close();
    nextOutput += deltaTime;
    ++count;
  }
}

double DeltaTimeDiagnostic::getNextOutput() {
  return nextOutput;
}

double DeltaTimeDiagnostic::getDeltaTime() {
  return deltaTime;
}

void DeltaTimeDiagnostic::initParameters(BlockParameters &blockPars) {
  DiagnosticInterface::initParameters(blockPars);
  blockPars.addParameter("deltaTime", &deltaTime, 1.0);
  blockPars.addParameter("delay", &nextOutput, 0.0);
}

DiagnosticManager::DiagnosticManager()
    : timecounter(0), physicalTime(0), usePhysicalTime(false), master(true), mpiRank(0) {}

void DiagnosticManager::setTimeCounter(size_t *timecounter_) {
  timecounter = timecounter_;
  usePhysicalTime = false;
}

void DiagnosticManager::setPhysicalTime(double *physicalTime_) {
  physicalTime = physicalTime_;
  usePhysicalTime = true;
}

void DiagnosticManager::setMaster(bool master_) {
  master = master_;
}

void DiagnosticManager::setRank(int mpiRank_) {
  mpiRank = mpiRank_;
}

void DiagnosticManager::addIntervalDiagnostic(IntervalDiagnostic *diag) {
  intervalDiags.push_back(diag);
}

void DiagnosticManager::addDeltaTimeDiagnostic(DeltaTimeDiagnostic *diag) {
  deltaTimeDiags.push_back(diag);
}

void DiagnosticManager::execute() {
  SCHNEK_TRACE_LOG(
      2, "DiagnosticManager::execute " << mpiRank << " " << usePhysicalTime
             ? boost::lexical_cast<std::string>(*physicalTime)
             : boost::lexical_cast<std::string>(*timecounter)
  );

  if ((!usePhysicalTime && !timecounter) || (usePhysicalTime && !physicalTime))
    throw schnek::VariableNotInitialisedException(
        "In DiagnosticManager: A time counter or physical time must be specified!"
    );

  for (IntervalDiagnostic *diag : intervalDiags) {
    diag->execute(master, mpiRank, *timecounter);
  }

  for (DeltaTimeDiagnostic *diag : deltaTimeDiags) {
    diag->execute(master, mpiRank, *physicalTime);
  }
}

double DiagnosticManager::adjustDeltaT(double deltaT) {
  double adjustedDt = deltaT;

  for (DeltaTimeDiagnostic *diag : deltaTimeDiags) {
    double dt = diag->getNextOutput() - *physicalTime;
    if (dt > 0) adjustedDt = std::min(adjustedDt, dt);
  }

  return adjustedDt;
}
