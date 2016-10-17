/*
 * diagnostic.hpp
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

#ifndef SCHNEK_DIAGNOSTIC_HPP_
#define SCHNEK_DIAGNOSTIC_HPP_

#include "../variables/block.hpp"
#include "../util/singleton.hpp"

#include <boost/shared_ptr.hpp>
#include <fstream>

namespace schnek {

/** Interface for diagnostic tasks.
 *
 * This interface can be used to implement different types of diagnostics.
 * The DiagnosticInterface is closely related to the DiagnosticManager. When an
 * instance of the interface is created it will register itself with the
 * DiagnosticManager. This then takes the responsibility of calling the execute
 * method of the DiagnosticInterface.
 */
class DiagnosticInterface : public Block
{
  private:
    /// The file name into which to write
    std::string fname;
    /// Append data at every write to the same file?
    int append;
  public:
    DiagnosticInterface();
    virtual ~DiagnosticInterface() {}
  protected:
    virtual void open(const std::string &) {}
    virtual void write() {}
    virtual void close() {}
    virtual bool singleOut() { return false; }
    void initParameters(BlockParameters&);

    bool appending();
    std::string parsedFileName(int rank, int timeCounter);
    std::string parsedFileName(int rank, double physicalTime);
};

class IntervalDiagnostic : public DiagnosticInterface
{
  private:
    /// The interval at which to write
    int interval;
  public:
    IntervalDiagnostic();
    void execute(bool master, int rank, int timeCounter);
  protected:
    void initParameters(BlockParameters&);
};

class DeltaTimeDiagnostic : public DiagnosticInterface
{
  private:
    /// The physical time interval at which to write
    double deltaTime;
    double nextOutput;
  public:
    DeltaTimeDiagnostic();
    void execute(bool master, int rank, double physicalTime);
    double getNextOutput();
  protected:
    void initParameters(BlockParameters&);
};

typedef boost::shared_ptr<DiagnosticInterface> pDiagnosticInterface;
typedef std::list<pDiagnosticInterface> DiagList;

class DiagnosticManager : public Singleton<DiagnosticManager>
{
  private:
    std::list<IntervalDiagnostic*> intervalDiags;
    std::list<DeltaTimeDiagnostic*> deltaTimeDiags;

    /// The current time step
    int *timecounter;
    double *physicalTime;
    bool usePhysicalTime;
    bool master;
    int rank;

    friend class Singleton<DiagnosticManager>;
    friend class CreateUsingNew<DiagnosticManager>;
  public:
    void addIntervalDiagnostic(IntervalDiagnostic*);
    void addDeltaTimeDiagnostic(DeltaTimeDiagnostic*);
    void execute();

    void setTimeCounter(int *timecounter);
    void setPhysicalTime(double *physicalTime);
    void setMaster(bool master);
    void setRank(int rank);

    double adjustDeltaT(double deltaT);
  private:
    DiagnosticManager();
};

template<class Type, typename PointerType = boost::shared_ptr<Type>, class DiagnosticType = IntervalDiagnostic >
class SimpleDiagnostic : public DiagnosticType
{
  private:
    /// The name of the field to write out
    std::string fieldName;
    bool single_out;
  protected:
    PointerType field;
  public:
    SimpleDiagnostic() { single_out=false; }
    virtual ~SimpleDiagnostic();
  protected:
    bool singleOut() { return single_out; }
    void initParameters(BlockParameters&);
    void init();
    std::string getFieldName() { return fieldName; }

    /**
     * Is the field to be written derived from another field and not available through
     * the Block::retrieveData mechanism.
     *
     * If a class overrides and returns true then SimpleDiagnostic::field has
     * to be set manually.
     */
    virtual bool isDerived() { return false; }
  public:
    void setSingleOut(bool single_out_) { single_out = single_out_; }
};

template<class Type, typename PointerType = boost::shared_ptr<Type>, class DiagnosticType = IntervalDiagnostic >
class SimpleFileDiagnostic : public SimpleDiagnostic<Type, PointerType, DiagnosticType>
{
  private:
    std::ofstream output;
  protected:
    void open(const std::string &);
    void write();
    void close();
};

} // namespace schnek


#include "diagnostic.t"

#endif // SCHNEK_DIAGNOSTIC_HPP_
