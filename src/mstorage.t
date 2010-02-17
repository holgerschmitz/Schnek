#include <cstddef>

namespace schnek {

template<typename T, int rank>
SingleArrayMatrixStorage<T, rank>::SingleArrayMatrixStorage()
{
  data = NULL;
  data_fast = NULL;
  size = 0;
}


template<typename T, int rank>
SingleArrayMatrixStorage<T, rank>::SingleArrayMatrixStorage(
  const IndexType &low_, 
  const IndexType &high_
)
{
  data = NULL;
  size = 0;
  newData(low_, high_);
}


template<class T, int rank> 
SingleArrayMatrixStorage<T, rank>::~SingleArrayMatrixStorage()
{
  deleteData();
}


template<typename T, int rank>
inline T& SingleArrayMatrixStorage<T, rank>::get(const IndexType &index)
{
  int pos = index[rank-1];
  for (int i=rank-2; i>=0; --i)
  {
    pos = index[i] + dims[i]*pos; 
  }
  return data_fast[pos];
}

template<typename T, int rank>
inline const T& SingleArrayMatrixStorage<T, rank>::get(const IndexType &index) const
{
  int pos = index[rank-1];
  for (int i=rank-2; i>=0; --i)
  {
    pos = index[i] + dims[i]*pos; 
  }
  return data_fast[pos];
}

template<typename T, int rank>
void SingleArrayMatrixStorage<T, rank>::resize(const IndexType &low_, const IndexType &high_)
{
//  if ( (low != low_) || (high != high_) )
//  {
    deleteData();
    newData(low_,high_);
//  }
}

template<typename T, int rank>
void SingleArrayMatrixStorage<T, rank>::deleteData()
{
  if (data)
    delete[] data;
  data = NULL;
  size = 0;
}

template<typename T, int rank>
void SingleArrayMatrixStorage<T, rank>::newData(
  const IndexType &low_, 
  const IndexType &high_
)
{
  size = 1;
  int d;
  
  low = low_;
  high = high_;
  
  for (d = 0; d < rank; d++) {
    dims[d] = high[d] - low[d] + 1;
    size *= dims[d];
  }
  data = new T[size];
  int p = -low[rank-1];

  for (d = rank-2; d >= 0 ; d--) {
    p = p*dims[d] -low[d];
  }
  data_fast = data + p;
}

}
