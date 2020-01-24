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

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <map>

namespace schnek {

template<typename TYPE>
struct H5DataType{
  static const hid_t type;
};


/**
 * A container type for grids that are being passed to the HDFGridDiagnostic
 */
template<typename FieldType>
struct GridContainer
{
  /// The pointer to the grid
  FieldType *grid;

  /// The global minimum coordinate
  typename FieldType::IndexType global_min;

  /// The global maximum coordinate
  typename FieldType::IndexType global_max;

  /// The local minimum coordinate
  typename FieldType::IndexType local_min;

  /// The local maximum coordinate
  typename FieldType::IndexType local_max;
};

/**
 * HDF5 attributes to a data set
 *
 * Attributes can contain numerical values indexed by name
 */
struct HdfAttributes {
  friend class HdfOStream;
  private:
    struct Info {
      hid_t type;
      hsize_t dims;
      const void *buffer;
    };
    typedef boost::shared_ptr<Info> pInfo;

    std::map<std::string, pInfo> attributes;
  public:

    /**
     * Set a value on the dataset's attributes
     *
     * @param name   the name of the attribute
     * @param value  the value to be stored
     */
    template<typename T>
    void set(std::string name, const T *value, hsize_t dims = 1);

    /**
     * Set a value on the dataset's attributes
     *
     * @param name   the name of the attribute
     * @param value  the value to be stored
     */
    template<typename T>
    void set(std::string name, const T &value, hsize_t dims = 1);
};

typedef boost::shared_ptr<HdfAttributes> pHdfAttributes;

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

    /// any attributes attached to the data
    pHdfAttributes attributes;

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

    /// set the name of the data block
    void setBlockName(std::string blockname_);

    /// set the attributes
    void setAttributes(pHdfAttributes attributes_);

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
  private:
    hid_t dxpl_id;
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
/**
 * Abstract diagnostic class for writing Grids into HDF5 data files
 */
template<typename Type, typename PointerType = boost::shared_ptr<Type>, class DiagnosticType = IntervalDiagnostic >
class HDFGridDiagnostic : public SimpleDiagnostic<Type, PointerType, DiagnosticType> {
  public:
    typedef typename Type::IndexType IndexType;
  protected:
    HdfOStream output;
    GridContainer<Type> container;
  protected:
    /// Open the output file
    void open(const std::string &);
    /// Write into the touput file
    void write();
    /// Close the output file
    void close();

    /// Block inititialisation
    void init();
    /// Get the global minimum of the simulation bounds
    virtual IndexType getGlobalMin() = 0;
    /// Get the global maximum of the simulation bounds
    virtual IndexType getGlobalMax() = 0;

    /**
     * Get the name of the data set in the HDF file
     *
     * @return  the field name `data`
     */
    virtual std::string getDatasetName();

    /**
     * Get the attributes to be stored with the dataset.
     *
     * Override this to store additional attributes with the dataset
     *
     * @return  an empty attributes set
     */
    virtual pHdfAttributes getAttributes() {
      return boost::make_shared<HdfAttributes>();
    };
  public:
    virtual ~HDFGridDiagnostic() {}
};

/**
 * Reader for HDF grid data
 *
 * An interface that
 */
template<typename Type, typename PointerType = boost::shared_ptr<Type> >
class HDFGridReader : public Block
{
  public:
    typedef typename Type::IndexType IndexType;
  protected:
    /// The HDF output stream
    HdfIStream input;
    /// The container for the grid holding additional data
    GridContainer<Type> container;
    /// The field that the data will be read into
    PointerType field;
    /// The name of the field to read the data into
    std::string fieldName;
    /// The name of the file to read the data from
    std::string fileName;
  public:
    /// Default constructor
    HDFGridReader();
    /// Virtual destructor
    virtual ~HDFGridReader() {}
  protected:
    /// Open the input file
    void open();

    /// Read from the input file
    void read();

    /// Close the output file
    void close();

    /**
     * Run the grid reader
     *
     * This will open the file, read the data and close the file again.
     *
     * This method needs to be run after the block has been initialised during
     * the `init` lifecycle stage.
     */
    void execute();

    /// Block inititialisation
    void init();

    /// Get the global minimum of the simulation bounds
    virtual IndexType getGlobalMin() = 0;

    /// Get the global maximum of the simulation bounds
    virtual IndexType getGlobalMax() = 0;

    /**
     * Get the name of the data set in the HDF file
     *
     * @return  the field name `data`
     */
    virtual std::string getDatasetName();

    /// Block callback to initialise the parameters
    void initParameters(BlockParameters &blockPars);
};

} // namespace schnek

#include "hdfdiagnostic.t"

#endif

#endif // SCHNEK_HDFDIAGNOSTIC_HPP_
