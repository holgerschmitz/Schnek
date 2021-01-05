/*
 * hdfdiagnostic.cpp
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

#include "../schnek_config.hpp"

#ifdef SCHNEK_HAVE_HDF5

#include "hdfdiagnostic.hpp"
#include "../util/logger.hpp"

#undef LOGLEVEL
#define LOGLEVEL 0

using namespace schnek;

#if defined (H5_HAVE_PARALLEL) && defined (SCHNEK_USE_HDF_PARALLEL)
#include <mpi.h>
#endif

HdfStream::HdfStream()
  : file_id(-1),
    status(0),
    blockname("data"),
    attributes(boost::make_shared<HdfAttributes>()),
    sets_count(0),
    active(true),
    activeModified(false)
#if defined (H5_HAVE_PARALLEL) && defined (SCHNEK_USE_HDF_PARALLEL)
    , commSet(false)
#endif
{}

HdfStream::HdfStream(const HdfStream& hdf)
  : file_id(hdf.file_id),
    status(hdf.status),
    blockname(hdf.blockname),
    attributes(hdf.attributes),
    sets_count(hdf.sets_count),
    active(true),
    activeModified(false)
#if defined (H5_HAVE_PARALLEL) && defined (SCHNEK_USE_HDF_PARALLEL)
    , commSet(false)
#endif
{}

HdfStream &HdfStream::operator=(const HdfStream& hdf)
{
  file_id = hdf.file_id;
  status = hdf.status;
  sets_count = hdf.sets_count;
  blockname = hdf.blockname;
  attributes = hdf.attributes;
  active = hdf.active;
  activeModified = hdf.activeModified;
#if defined (H5_HAVE_PARALLEL) && defined (SCHNEK_USE_HDF_PARALLEL)
  mpiComm = hdf.mpiComm;
  commSet = hdf.commSet;
#endif
  return *this;
}

HdfStream::~HdfStream()
{
  close();
}

void HdfStream::close()
{
  if (file_id >= 0) {
    H5Fclose (file_id);
  }
  file_id = -1;
}

bool HdfStream::good() const
{
  return (file_id>=0);
}

void HdfStream::setBlockName(std::string blockname_)
{
  SCHNEK_TRACE_LOG(2,"setBlockName("<<blockname_<<")")
  blockname = blockname_;
  sets_count = -1;
}

void HdfStream::setAttributes(pHdfAttributes attributes_) {
  attributes = attributes_;
}


std::string HdfStream::getNextBlockName()
{
  std::ostringstream bname;
  bname << blockname;

  if (sets_count<0) sets_count = 1;
  else
  {
    bname << sets_count++;
  }

  SCHNEK_TRACE_LOG(2,"getBlockName = " << bname.str())

  return bname.str();
}

#if defined (H5_HAVE_PARALLEL) && defined (SCHNEK_USE_HDF_PARALLEL)
void HdfStream::makeMPIGroup()
{
  if (!activeModified) {
    if (!commSet)
    {
      mpiComm = MPI_COMM_WORLD;
      commSet = true;
    }
    return;
  }

  int rank, size;
  MPI_Group worldGroup, activeGroup;

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_group(MPI_COMM_WORLD, &worldGroup);

  int *inputArr = new int[size];
  int *activeArr = new int[size];

  for (int i=0; i<size; ++i) inputArr[i] = 0;
  if (active) inputArr[rank] = 1;

  MPI_Allreduce(inputArr, activeArr, size, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

  int count = 0;
  for (int i=0; i<size; ++i)
  {
    if (activeArr[i]>0) {
      inputArr[count] = i;
      ++count;
    }
  }

  MPI_Group_incl(worldGroup, count, inputArr, &activeGroup);
  MPI_Comm_create(MPI_COMM_WORLD, activeGroup, &mpiComm);

  delete[] activeArr;
  delete[] inputArr;

  activeModified = false;
}
#endif


// ----------------------------------------------------------------------

HdfIStream::HdfIStream()
  : HdfStream() {}

HdfIStream::HdfIStream(const char* fname)
  : HdfStream()
{
  open(fname);
}

HdfIStream::HdfIStream(const HdfIStream& hdf)
  : HdfStream(hdf)
{}

int HdfIStream::open(const char* fname)
{
  close();

#if defined (H5_HAVE_PARALLEL) && defined (SCHNEK_USE_HDF_PARALLEL)
  const int sieve_buf_size = 262144;
  const int align_threshold = 1; //524288;
  const int alignment = 1; // 262144;

  MPI_Info mpi_info;

  makeMPIGroup();
  if (active) {

    /* setup file access template */
    hid_t plist_id = H5Pcreate (H5P_FILE_ACCESS);

    H5Pset_sieve_buf_size(plist_id, sieve_buf_size);
    H5Pset_alignment(plist_id, align_threshold, alignment);

    MPI_Info_create(&mpi_info);

    // MPI
    const char access_style[]         = "access_style";
    const char write_once[]           = "write_once";
    const char collective_buffering[] = "collective_buffering";
    const char strue[]                = "true";
    const char cb_block_size[]        = "cb_block_size";
    const char n1048576[]             = "1048576";
    const char cb_buffer_size[]       = "cb_buffer_size";
    const char n4194304[]             = "4194304";

    MPI_Info_set(mpi_info, access_style, write_once);
    MPI_Info_set(mpi_info, collective_buffering, strue);
    MPI_Info_set(mpi_info, cb_block_size, n1048576);
    MPI_Info_set(mpi_info, cb_buffer_size, n4194304);

    /* set Parallel access with communicator */
    H5Pset_fapl_mpio(plist_id, mpiComm, mpi_info);

    /* open the file collectively */
    file_id = H5Fopen (fname, H5F_ACC_RDONLY, plist_id);

    /* Release file-access template */
    MPI_Info_free(&mpi_info);
    H5Pclose(plist_id);

    dxpl_id = H5Pcreate(H5P_DATASET_XFER);
    SCHNEK_TRACE_LOG(3,"Data Transfer Property List Id (0) " << dxpl_id)
    H5Pset_dxpl_mpio(dxpl_id, H5FD_MPIO_COLLECTIVE);
  }
#else
  if (active) {
    file_id = H5Fopen (fname, H5F_ACC_RDONLY, H5P_DEFAULT);
  }
#endif
  sets_count = 0;
  return 1;
}


// ----------------------------------------------------------------------

HdfOStream::HdfOStream()
   : HdfStream(), initialised(false)
{}

HdfOStream::HdfOStream(const HdfOStream& hdf)
  : HdfStream(hdf), initialised(hdf.initialised)
{}

HdfOStream::HdfOStream(const char* fname)
   : HdfStream(), initialised(false)
{
  open(fname);
}

int HdfOStream::open(const char* fname) {
  sets_count = 0;

#if defined (H5_HAVE_PARALLEL) && defined (SCHNEK_USE_HDF_PARALLEL)

  const int sieve_buf_size = 262144;
  const int align_threshold = 1; //524288;
  const int alignment = 1; // 262144;


  makeMPIGroup();
  if (active) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (!initialised) {
      /* setup file access template */

      plist_id = H5Pcreate(H5P_FILE_ACCESS);

      H5Pset_sieve_buf_size(plist_id, sieve_buf_size);
      H5Pset_alignment(plist_id, align_threshold, alignment);

      MPI_Info_create(&mpi_info);

      // MPI
      const char access_style[]         = "access_style";
      const char write_once[]           = "write_once";
      const char collective_buffering[] = "collective_buffering";
      const char strue[]                = "true";
      const char cb_block_size[]        = "cb_block_size";
      const char n1048576[]             = "1048576";
      const char cb_buffer_size[]       = "cb_buffer_size";
      const char n4194304[]             = "4194304";

      MPI_Info_set(mpi_info, access_style, write_once);
      MPI_Info_set(mpi_info, collective_buffering, strue);
      MPI_Info_set(mpi_info, cb_block_size, n1048576);
      MPI_Info_set(mpi_info, cb_buffer_size, n4194304);

      /* set Parallel access with communicator */
      H5Pset_fapl_mpio(plist_id, mpiComm, mpi_info);
    }

    /* open the file collectively */
    //H5Pset_fapl_mpiposix(plist_id, mpiComm, 0);
    file_id = H5Fcreate (fname, H5F_ACC_TRUNC, H5P_DEFAULT, plist_id);

    if (!initialised) {
      dxpl_id = H5Pcreate(H5P_DATASET_XFER);
      SCHNEK_TRACE_LOG(3,"Data Transfer Property List Id (0) " << dxpl_id)
      H5Pset_dxpl_mpio(dxpl_id, H5FD_MPIO_COLLECTIVE);
      initialised = true;
    }
  }
#else
  if (active)
    file_id = H5Fcreate (fname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

  dxpl_id = H5P_DEFAULT;
#endif

  if (active) assert(file_id != -1);

  SCHNEK_TRACE_LOG(3,"Data Transfer Property List Id " << dxpl_id)

  return file_id;
}

// ----------------------------------------------------------------------

template<>
const hid_t H5DataType<int>::type = H5T_NATIVE_INT;

template<>
const hid_t H5DataType<float>::type = H5T_NATIVE_FLOAT;

template<>
const hid_t H5DataType<double>::type = H5T_NATIVE_DOUBLE;

#endif
