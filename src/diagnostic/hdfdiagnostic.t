/*
 * hdfdiagnostic.t
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

#include "../grid/field.hpp"
#include "../util/logger.hpp"

#undef LOGLEVEL
#define LOGLEVEL 0

namespace schnek {


template<typename FieldType>
void HdfIStream::readGrid(GridContainer<FieldType> &g)
{
  std::string dset_name = getNextBlockName();

  typedef typename FieldType::IndexType IndexType;
  typedef typename FieldType::value_type T;

  IndexType mdims = g.grid->getDims();
  IndexType mlo = g.grid->getLo();
  IndexType mhi = g.grid->getHi();

  IndexType llo = g.local_min;
  IndexType lhi = g.local_max;


  hsize_t dims[FieldType::Rank];

  hsize_t locdims[FieldType::Rank];
  hsize_t memdims[FieldType::Rank];
  hsize_t locstart[FieldType::Rank];
  hsize_t memstart[FieldType::Rank];

  for (int i=0; i<FieldType::Rank; ++i)
  {
    // maybe use [FieldType::Rank-1-i] as index on the LHS
    int gmin = g.global_min[i];
    dims[i] = 1 + g.global_max[i] - gmin;
    locdims[i]  = lhi[i] - llo[i] + 1;
    locstart[i] = llo[i] - gmin;
    memdims[i] = mhi[i] - mlo[i] + 1;
    memstart[i] = llo[i] - mlo[i];
  }

  T *data = g.grid->getRawData();

  /* open the dataset collectively */
#if H5Dopen_vers == 2
  hid_t dataset = H5Dopen(file_id, dset_name.c_str(),H5P_DEFAULT);
#else
  hid_t dataset = H5Dopen(file_id, dset_name.c_str());
#endif

  assert(dataset != -1);

#if defined (H5_HAVE_PARALLEL) && defined (SCHNEK_USE_HDF_PARALLEL)
  /* create a file dataspace independently */
  hid_t file_dataspace = H5Dget_space(dataset);
  assert(file_dataspace != -1);

  hid_t ret=H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET,
                                locstart, NULL,
                                locdims, NULL);
  assert(ret != -1);

  /* create a memory dataspace independently */
  hid_t mem_dataspace = H5Screate_simple(FieldType::Rank, memdims, NULL);
  assert (mem_dataspace != -1);
  ret = H5Sselect_hyperslab(mem_dataspace,  H5S_SELECT_SET,
                            memstart, NULL, locdims, NULL);
  assert(ret != -1);


  /* read the data independently */
  ret = H5Dread(dataset,
                H5DataType<T>::type,
                mem_dataspace,
                file_dataspace,
                H5P_DEFAULT,
                data);
  assert(ret != -1);

  /* release all IDs created */
  H5Sclose(mem_dataspace);
  H5Sclose(file_dataspace);
#else
  /* read the data on single processor */
  hid_t ret = H5Dread(dataset,
                      H5DataType<T>::type,
                      H5S_ALL,
                      H5S_ALL,
                      H5P_DEFAULT,
                      data);
  assert(ret != -1);
#endif

  /* close dataset collectively */
  ret=H5Dclose(dataset);
  assert(ret != -1);

}

template<typename FieldType>
void HdfOStream::writeGrid(GridContainer<FieldType> &g)
{
  std::string dset_name = getNextBlockName();

  typedef typename FieldType::IndexType IndexType;
  typedef typename FieldType::value_type T;

  IndexType mdims = g.grid->getDims();
  IndexType mlo = g.grid->getLo();
  IndexType mhi = g.grid->getHi();

  IndexType llo = g.local_min;
  IndexType lhi = g.local_max;


  hsize_t dims[FieldType::Rank];

  hsize_t locdims[FieldType::Rank];
  hsize_t memdims[FieldType::Rank];
  hsize_t locstart[FieldType::Rank];
  hsize_t memstart[FieldType::Rank];

  //bool empty = false;

  for (int i=0; i<FieldType::Rank; ++i)
  {
    // maybe use [FieldType::Rank-1-i] as index on the LHS
    int gmin = g.global_min[i];
    dims[i] = 1 + g.global_max[i] - gmin;
    locdims[i]  = lhi[i] - llo[i] + 1;
    locstart[i] = llo[i] - gmin;
    memdims[i] = mhi[i] - mlo[i] + 1;
    memstart[i] = llo[i] - mlo[i];

    SCHNEK_TRACE_LOG(2,"HdfOStream::writeGrid("<<i<<") "<< gmin <<" "<< g.global_max[i]<<" " << llo[i]<<" " << lhi[i])

    //if (locdims[FieldType::Rank-1-i]<=0) empty = true;

    if (dims[i]<(locstart[i]+locdims[i]))
    {
      std::cerr << "FATAL ERROR!\n"
        << "  in HdfOStream::writeGrid\n"
        << "Dimension " << i << ":\n  global size: " << dims[i]
        << "\n  global min: " << gmin
        << "\n  global max: " << g.global_max[i]
        << "\n  local start: " << locstart[i]
        << "\n  mlo: " << mlo[i]
        << "\n  mhi: " << mhi[i]
        << "\n  llo: " << llo[i]
        << "\n  lhi: " << lhi[i]
        << "\n  local size: " << locdims[i]
        << "\n  global min: " << gmin << "\n";
      exit(-1);
    }
  }

  const T *data = g.grid->getRawData();
  hid_t ret;

  /* setup dimensionality object */
#if defined (H5_HAVE_PARALLEL) && defined (SCHNEK_USE_HDF_PARALLEL)
  hid_t sid = H5Screate_simple (FieldType::Rank, dims, NULL);
#else
  hid_t sid = H5Screate_simple (FieldType::Rank, memdims, NULL);
#endif

  assert(sid > -1);

  /* create a dataset */
#if H5Dcreate_vers==2
  hid_t dataset = H5Dcreate(file_id,
                            dset_name.c_str(),
                            H5DataType<T>::type,
                            sid,
                            H5P_DEFAULT,
                            H5P_DEFAULT,
                            H5P_DEFAULT);
#else
  hid_t dataset = H5Dcreate(file_id,
                            dset_name.c_str(),
                            H5DataType<T>::type,
                            sid,
                            H5P_DEFAULT);
#endif

  assert(dataset > -1);

#if defined (H5_HAVE_PARALLEL) && defined (SCHNEK_USE_HDF_PARALLEL)
  /* create a file dataspace independently */
  hid_t file_dataspace = H5Dget_space(dataset);

  assert(file_dataspace > -1);

  ret = H5Sselect_hyperslab(file_dataspace,  H5S_SELECT_SET,
                            locstart, NULL, locdims, NULL);
  assert(ret != -1);

  /* create a memory dataspace independently */
  hid_t mem_dataspace = H5Screate_simple (FieldType::Rank, memdims, NULL);

  assert(mem_dataspace > -1);
  ret = H5Sselect_hyperslab(mem_dataspace,  H5S_SELECT_SET,
                              memstart, NULL, locdims, NULL);
  assert(ret != -1);
//  hid_t mem_dataspace = H5Dget_space(dataset);

  /* write data independently */
  ret = H5Dwrite(dataset,
                 H5DataType<T>::type,
                 mem_dataspace,
                 file_dataspace,
                 dxpl_id,
                 data);

  assert(ret != -1);

  /* release dataspace ID */
  H5Sclose(mem_dataspace);
  H5Sclose(file_dataspace);
#else
  /* write data on single processor */
  ret = H5Dwrite(dataset,
                       H5DataType<T>::type,
                       H5S_ALL,
                       H5S_ALL,
                       H5P_DEFAULT,
                       data);
  assert(ret != -1);
#endif

  /* close dataset collectively */
  ret=H5Dclose(dataset);
  assert(ret != -1);

  /* release all IDs created */
  H5Sclose(sid);
}


template<typename Type, typename PointerType>
void HDFGridDiagnostic<Type, PointerType>::open(const std::string &fname)
{
  output.open(fname.c_str());
}

template<typename Type, typename PointerType>
void HDFGridDiagnostic<Type, PointerType>::write()
{
  output.writeGrid(container);
}

template<typename Type, typename PointerType>
void HDFGridDiagnostic<Type, PointerType>::close()
{
  output.close();
}


template<typename InnerType>
struct CopyToContainer
{
  static void copy(InnerType *field, GridContainer<InnerType> &container)
  {
    container.grid = field;
    container.local_min = field->getLo();
    container.local_max = field->getHi();
  }
};

template<
  typename T,
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
struct CopyToContainer<Field<T, rank, CheckingPolicy, StoragePolicy> >
{
  static void copy(Field<T, rank, CheckingPolicy, StoragePolicy> *field,
           GridContainer<Field<T, rank, CheckingPolicy, StoragePolicy> > &container)
  {
    container.grid = field;
    container.local_min = field->getInnerLo();
    container.local_max = field->getInnerHi();

  }
};

template<typename Type, typename PointerType>
void HDFGridDiagnostic<Type, PointerType>::init()
{
  SimpleDiagnostic<Type, PointerType>::init();

  if (!this->isDerived())
  {
    CopyToContainer<Type>::copy(&(*this->field), container);
    container.global_min = this->getGlobalMin();
    container.global_max = this->getGlobalMax();
  }
}

#undef LOGLEVEL
#define LOGLEVEL 0

} // namespace 
