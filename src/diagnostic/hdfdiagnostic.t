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

  hsize_t dims[FieldType::Rank];
  hsize_t locdims[FieldType::Rank];
  hsize_t start[FieldType::Rank];

  for (int i=0; i<FieldType::Rank; ++i)
  {
    int gmin = g.global_min[i];
    dims[i] = 1 + g.global_max[i] - gmin;
    locdims[i] = mdims[i];
    start[i] = mlo[i] - gmin;
  }

  T *data = *g.grid->getRawData();

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

  hid_t ret=H5Sselect_hyperslab(file_dataspace,
                                H5S_SELECT_SET,
                                start,
                                NULL,
                                locdims,
                                NULL);
  assert(ret != -1);

  /* create a memory dataspace independently */
  hid_t mem_dataspace = H5Screate_simple(FieldType::Rank, locdims, NULL);
  assert (mem_dataspace != -1);


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

  hsize_t dims[FieldType::Rank];
  hsize_t locdims[FieldType::Rank];
  hsize_t start[FieldType::Rank];

  //bool empty = false;

  for (int i=0; i<FieldType::Rank; ++i)
  {
    int gmin = g.global_min[i];
    dims[i] = 1 + g.global_max[i] - gmin;
    locdims[FieldType::Rank-1-i] = mdims[i];
    start[i] = mlo[i] - gmin;

    //if (locdims[FieldType::Rank-1-i]<=0) empty = true;

    if (dims[i]<(start[i]+locdims[FieldType::Rank-1-i]))
    {
      std::cerr << "FATAL ERROR!\n"
        << "  in HdfOStream::writeGrid\n"
        << "Dimension " << i << ":\n  global size: " << dims[i]
        << "\n  global min: " << gmin
        << "\n  global max: " << g.global_max[i]
        << "\n  local start: " << start[i]
        << "\n  local size: " << locdims[FieldType::Rank-1-i]
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
  hid_t sid = H5Screate_simple (FieldType::Rank, locdims, NULL);
#endif

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

#if defined (H5_HAVE_PARALLEL) && defined (SCHNEK_USE_HDF_PARALLEL)
  /* create a file dataspace independently */
  hid_t file_dataspace = H5Dget_space(dataset);

  ret = H5Sselect_hyperslab(file_dataspace,  H5S_SELECT_SET,
                              start, NULL, locdims, NULL);
  assert(ret != -1);

  /* create a memory dataspace independently */
  hid_t mem_dataspace = H5Screate_simple (FieldType::Rank, locdims, NULL);
//  hid_t mem_dataspace = H5Dget_space(dataset);

  /* write data independently */
  ret = H5Dwrite(dataset,
                 H5DataType<T>::type,
                 mem_dataspace,
                 file_dataspace,
                 H5P_DEFAULT,
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

template<typename Type, typename PointerType>
void HDFGridDiagnostic<Type, PointerType>::init()
{
  SimpleDiagnostic<Type, PointerType>::init();

  if (!this->isDerived())
  {
    container.grid = &(*this->field);
    container.global_min = this->getGlobalMin();
    container.global_max = this->getGlobalMax();
  }
}


} // namespace 
