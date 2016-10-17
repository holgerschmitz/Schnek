/*
 * hdfdiagnostic.hpp
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

#ifndef SCHNEK_HDFDIAGNOSTIC_HPP_
#define SCHNEK_HDFDIAGNOSTIC_HPP_

#include "../schnek_config.hpp"
#ifdef SCHNEK_HAVE_HDF5

#include "../grid/grid.hpp"
#include "diagnostic.hpp"

#include <hdf5.h>

namespace schnek {

template<typename FieldType>
struct GridContainer
{
  FieldType *grid;
  typename FieldType::IndexType global_min;
  typename FieldType::IndexType global_max;
  typename FieldType::IndexType local_min;
  typename FieldType::IndexType local_max;
};

/** @brief IO class for handling HDF files
  *
  * This is the abstract base class for HDF-IO- classes.
  * Implements the basic operations on HdfStreams as virtual methods.
  */
class HdfStream {
  protected:
    /// HDF5 File id
    hid_t       file_id;

    /// HDF5 Error status
    herr_t      status;

    /// name of the datablock to be read or written
    std::string blockname;
    /// counter for the sets with a given blockname read from or written to the file
    int sets_count;

    /// Specifies if the stream is active in this process (in case of parallel execution)
    bool active;
    bool activeModified;

  public:
    /// constructor
    HdfStream();
    ///copy constructur
    HdfStream(const HdfStream&);
    /// destructor
    virtual ~HdfStream();

    /// open  file
    virtual int open(const char*)=0;

    /// close file
    virtual void close();

    /// return true=1 if data are still available
    virtual bool good() const;

    void setBlockName(std::string blockname_);
    /// assign
    HdfStream& operator = (const HdfStream&);

    void setActive(bool active_) { active = active_; activeModified = true; }

  protected:
    std::string getNextBlockName();

#if defined (H5_HAVE_PARALLEL) && defined (SCHNEK_USE_HDF_PARALLEL)
    void makeMPIGroup();

    MPI_Comm mpiComm;
    bool commSet;
#endif

};


/** @brief Input stream for HDF files */
class HdfIStream : public HdfStream {
  public:
    /// constructor
    HdfIStream();

    /// copy constructor */
    HdfIStream(const HdfIStream&);

    /// constructor, opens HDF file "fname", selects first dataset
    HdfIStream(const char* fname);

    /// opens HDF file "fname", selects first dataset
    int open(const char*);

    /// stream input operator for a schnek::Matrix
    template<typename FieldType>
    void readGrid(GridContainer<FieldType> &g);
};


/** @brief output stream for HDF files */
class HdfOStream : public HdfStream {
  private:
    hid_t dxpl_id;
  public:
    /// constructor
    HdfOStream();

    /// copy constructor
    HdfOStream(const HdfOStream&);

    /// constructor, opens HDF file "fname"
    HdfOStream(const char* fname);

    /// open file
    int open(const char*);

    /// stream output operator for a matrix
    template<typename FieldType>
    void writeGrid(GridContainer<FieldType> &g);
};

template<typename TYPE>
struct H5DataType{
  static const hid_t type;
};

template<typename Type, typename PointerType = boost::shared_ptr<Type>, class DiagnosticType = IntervalDiagnostic >
class HDFGridDiagnostic : public SimpleDiagnostic<Type, PointerType, DiagnosticType> {
  protected:
    HdfOStream output;
    GridContainer<Type> container;
  protected:
    typedef typename Type::IndexType IndexType;
    void open(const std::string &);
    void write();
    void close();
    void init();
    virtual IndexType getGlobalMin() = 0;
    virtual IndexType getGlobalMax() = 0;
  public:
    virtual ~HDFGridDiagnostic() {}
};

} // namespace schnek

#include "hdfdiagnostic.t"

#endif

#endif // SCHNEK_HDFDIAGNOSTIC_HPP_
