template<
  class T, 
  int length, 
  template<int> class CheckingPolicy
>
      std::ostream &operator<<(std::ostream &out, const schnek::FixedArray<T,length,CheckingPolicy> &arr)
{
  if (length==0) return out;
  out << arr[0];
  for (int i=1; i<length; ++i) out << " " << arr[i];
  return out;
}

template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
std::ostream &operator<<(
  std::ostream& out, 
  const schnek::Matrix<T, 1, CheckingPolicy, StoragePolicy>&M)
{
  typedef typename schnek::Matrix<T, 1, CheckingPolicy, StoragePolicy>::IndexType IndexType;
  
  const IndexType &low = M.getLow();
  const IndexType &high = M.getHigh();
  
  if ( !(low<=high) ) return out;
  out << M(low[0]);
  for (int i=low[0]+1; i<=high[0]; ++i) out << " " << M(i);
  return out;
}


template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
std::ostream &operator<<(
  std::ostream& out, 
  const schnek::Matrix<T, 2, CheckingPolicy, StoragePolicy> &M
)
{
  typedef typename schnek::Matrix<T, 2, CheckingPolicy, StoragePolicy>::IndexType IndexType;
  
  const IndexType &low = M.getLow();
  const IndexType &high = M.getHigh();
  
  if ( !(low<=high) ) return out;
  for (int j=low[1]; j<=high[1]; ++j)
  {
    out << M(low[0],j);
    for (int i=low[0]+1; i<=high[0]; ++i) out << " " << M(i,j);
    out << std::endl;
  }
  
  return out;
}

template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
std::ostream &operator<<(
  std::ostream& out, 
  const schnek::Matrix<T, 3, CheckingPolicy, StoragePolicy> &M
)
{
  typedef typename schnek::Matrix<T, 3, CheckingPolicy, StoragePolicy>::IndexType IndexType;
  
  const IndexType &low = M.getLow();
  const IndexType &high = M.getHigh();
  
  if ( !(low<=high) ) return out;
  for (int k=low[2]; k<=high[2]; ++k)
  {
    for (int j=low[1]; j<=high[1]; ++j)
    {
      out << M(low[0],j,k);
      for (int i=low[0]+1; i<=high[0]; ++i) out << " " << M(i,j,k);
      out << std::endl;
    }
    out << std::endl;
  }
  
  return out;
}

template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
std::ostream &operator<<(
  std::ostream& out, 
  const schnek::Matrix<T, 4, CheckingPolicy, StoragePolicy> &M
)
{
  typedef typename schnek::Matrix<T, 4, CheckingPolicy, StoragePolicy>::IndexType IndexType;
  
  const IndexType &low = M.getLow();
  const IndexType &high = M.getHigh();
  
  if ( !(low<=high) ) return out;
  for (int l=low[3]; l<=high[3]; ++l)
  {
    for (int k=low[2]; k<=high[2]; ++k)
    {
      for (int j=low[1]; j<=high[1]; ++j)
      {
        out << M(low[0],j,k,l);
        for (int i=low[0]+1; i<=high[0]; ++i) out << " " << M(i,j,k,l);
        out << std::endl;
      }
      out << std::endl;
    }
  }
  
  return out;
}

template<
  typename T,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
std::ostream &operator<<(
  std::ostream& out, 
  const schnek::Matrix<T, 5, CheckingPolicy, StoragePolicy> &M
)
{
  typedef typename schnek::Matrix<T, 5, CheckingPolicy, StoragePolicy>::IndexType IndexType;
  
  const IndexType &low = M.getLow();
  const IndexType &high = M.getHigh();
  
  if ( !(low<=high) ) return out;
  for (int m=low[4]; m<=high[4]; ++m)
  {
    for (int l=low[3]; l<=high[3]; ++l)
    {
      for (int k=low[2]; k<=high[2]; ++k)
      {
        for (int j=low[1]; j<=high[1]; ++j)
        {
          out << M(low[0],j,k,l,m);
          for (int i=low[0]+1; i<=high[0]; ++i) out << " " << M(i,j,k,l,m);
          out << std::endl;
        }
        out << std::endl;
      }
    }
  }
    
  return out;
}
