/*
 * datastream.t
 *
 * Created on: 23 Jan 2007
 * Author: Holger Schmitz
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

template<
  class T, 
  int length, 
  template<int> class CheckingPolicy
>
      std::ostream &operator<<(std::ostream &out, const schnek::Array<T,length,CheckingPolicy> &arr)
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
  const schnek::Grid<T, 1, CheckingPolicy, StoragePolicy>&M)
{
  typedef typename schnek::Grid<T, 1, CheckingPolicy, StoragePolicy>::IndexType IndexType;
  
  const IndexType &low = M.getLo();
  const IndexType &high = M.getHi();
  
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
  const schnek::Grid<T, 2, CheckingPolicy, StoragePolicy> &M
)
{
  typedef typename schnek::Grid<T, 2, CheckingPolicy, StoragePolicy>::IndexType IndexType;
  
  const IndexType &low = M.getLo();
  const IndexType &high = M.getHi();
  
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
  const schnek::Grid<T, 3, CheckingPolicy, StoragePolicy> &M
)
{
  typedef typename schnek::Grid<T, 3, CheckingPolicy, StoragePolicy>::IndexType IndexType;
  
  const IndexType &low = M.getLo();
  const IndexType &high = M.getHi();
  
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
  const schnek::Grid<T, 4, CheckingPolicy, StoragePolicy> &M
)
{
  typedef typename schnek::Grid<T, 4, CheckingPolicy, StoragePolicy>::IndexType IndexType;
  
  const IndexType &low = M.getLo();
  const IndexType &high = M.getHi();
  
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
  const schnek::Grid<T, 5, CheckingPolicy, StoragePolicy> &M
)
{
  typedef typename schnek::Grid<T, 5, CheckingPolicy, StoragePolicy>::IndexType IndexType;
  
  const IndexType &low = M.getLo();
  const IndexType &high = M.getHi();
  
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
