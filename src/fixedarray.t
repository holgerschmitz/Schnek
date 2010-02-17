#include "ctassert.h"
#include "farrexpression.h"

namespace schnek
{

template<class T, int length, template <int> class CheckingPolicy>
inline FixedArray<T,length,CheckingPolicy>::FixedArray()
{}

template<class T, int length, template <int> class CheckingPolicy>
inline FixedArray<T,length,CheckingPolicy>::FixedArray(const FixedArray &fixarr)
{
  for (int i=0; i<length; ++i)
    data[i] = fixarr[i];
}

template<class T, int length, template <int> class CheckingPolicy>
inline FixedArray<T,length,CheckingPolicy>::FixedArray(const T& v0)
{
  CTASSERT(1==length,NUMBER_OF_ARGUMENTS_DOESNT_MATCH_ARRAY_LENGTH)
  data[0] = v0;
}

template<class T, int length, template <int> class CheckingPolicy>
inline FixedArray<T,length,CheckingPolicy>::FixedArray(
                  const T& v0, const T& v1
)
{
  CTASSERT(2==length,NUMBER_OF_ARGUMENTS_DOESNT_MATCH_ARRAY_LENGTH)
  data[0] = v0;
  data[1] = v1;
}

template<class T, int length, template <int> class CheckingPolicy>
inline FixedArray<T,length,CheckingPolicy>::FixedArray(
                  const T& v0, const T& v1, const T& v2
)
{
  CTASSERT(3==length,NUMBER_OF_ARGUMENTS_DOESNT_MATCH_ARRAY_LENGTH)
  data[0] = v0;
  data[1] = v1;
  data[2] = v2;
}

template<class T, int length, template <int> class CheckingPolicy>
inline FixedArray<T,length,CheckingPolicy>::FixedArray(
                  const T& v0, const T& v1, const T& v2, const T& v3
)
{
  CTASSERT(4==length,NUMBER_OF_ARGUMENTS_DOESNT_MATCH_ARRAY_LENGTH)
  data[0] = v0;
  data[1] = v1;
  data[2] = v2;
  data[3] = v3;
}

template<class T, int length, template <int> class CheckingPolicy>
inline FixedArray<T,length,CheckingPolicy>::FixedArray(
                  const T& v0, const T& v1, const T& v2, const T& v3, const T& v4
)
{
  CTASSERT(5==length,NUMBER_OF_ARGUMENTS_DOESNT_MATCH_ARRAY_LENGTH)
  data[0] = v0;
  data[1] = v1;
  data[2] = v2;
  data[3] = v3;
  data[4] = v4;
}

template<class T, int length, template <int> class CheckingPolicy>
inline FixedArray<T,length,CheckingPolicy>::FixedArray(
                  const T& v0, const T& v1, const T& v2, const T& v3, const T& v4, 
                  const T& v5
)
{
  CTASSERT(6==length,NUMBER_OF_ARGUMENTS_DOESNT_MATCH_ARRAY_LENGTH)
  data[0] = v0;
  data[1] = v1;
  data[2] = v2;
  data[3] = v3;
  data[4] = v4;
  data[5] = v5;
}

template<class T, int length, template <int> class CheckingPolicy>
inline FixedArray<T,length,CheckingPolicy>::FixedArray(
                  const T& v0, const T& v1, const T& v2, const T& v3, const T& v4, 
                  const T& v5, const T& v6
)
{
  CTASSERT(7==length,NUMBER_OF_ARGUMENTS_DOESNT_MATCH_ARRAY_LENGTH)
  data[0] = v0;
  data[1] = v1;
  data[2] = v2;
  data[3] = v3;
  data[4] = v4;
  data[5] = v5;
  data[6] = v6;
}

template<class T, int length, template <int> class CheckingPolicy>
inline FixedArray<T,length,CheckingPolicy>::FixedArray(
                  const T& v0, const T& v1, const T& v2, const T& v3, const T& v4, 
                  const T& v5, const T& v6, const T& v7
)
{
  CTASSERT(8==length,NUMBER_OF_ARGUMENTS_DOESNT_MATCH_ARRAY_LENGTH)
  data[0] = v0;
  data[1] = v1;
  data[2] = v2;
  data[3] = v3;
  data[4] = v4;
  data[5] = v5;
  data[6] = v6;
  data[7] = v7;
}

template<class T, int length, template <int> class CheckingPolicy>
inline FixedArray<T,length,CheckingPolicy>::FixedArray(
                  const T& v0, const T& v1, const T& v2, const T& v3, const T& v4, 
                  const T& v5, const T& v6, const T& v7, const T& v8
)
{
  CTASSERT(9==length,NUMBER_OF_ARGUMENTS_DOESNT_MATCH_ARRAY_LENGTH)
  data[0] = v0;
  data[1] = v1;
  data[2] = v2;
  data[3] = v3;
  data[4] = v4;
  data[5] = v5;
  data[6] = v6;
  data[7] = v7;
  data[8] = v8;
}

template<class T, int length, template <int> class CheckingPolicy>
inline FixedArray<T,length,CheckingPolicy>::FixedArray(
                  const T& v0, const T& v1, const T& v2, const T& v3, const T& v4, 
                  const T& v5, const T& v6, const T& v7, const T& v8, const T& v9
)
{
  CTASSERT(10==length,NUMBER_OF_ARGUMENTS_DOESNT_MATCH_ARRAY_LENGTH)
  data[0] = v0;
  data[1] = v1;
  data[2] = v2;
  data[3] = v3;
  data[4] = v4;
  data[5] = v5;
  data[6] = v6;
  data[7] = v7;
  data[8] = v8;
  data[9] = v9;
}


template<class T, int length, template <int> class CheckingPolicy>
template<class Operator>
inline FixedArray<T,length,CheckingPolicy>::FixedArray(const FArrExpression<Operator> &expr)
{
  for (int i=0; i<length; ++i)
    data[i] = expr[i];
}
      
template<class T, int length, template <int> class CheckingPolicy>
inline T& FixedArray<T,length,CheckingPolicy>::operator[](int pos)
{
   return at(pos);
}

template<class T, int length, template <int> class CheckingPolicy>
inline const T& FixedArray<T,length,CheckingPolicy>::operator[](int pos) const
{
   return at(pos);
}

template<class T, int length, template <int> class CheckingPolicy>
inline T& FixedArray<T,length,CheckingPolicy>::at(int pos)
{
   this->check(pos);
   return data[pos];
}

template<class T, int length, template <int> class CheckingPolicy>
inline const T& FixedArray<T,length,CheckingPolicy>::at(int pos) const
{
   this->check(pos);
   return data[pos];
}

template<class T, int length, template <int> class CheckingPolicy>
template<class Operator>
inline FixedArray<T,length,CheckingPolicy>& FixedArray<T,length,CheckingPolicy>
  ::operator=(const FArrExpression<Operator> &expr)
{
  for (int i=0; i<length; ++i)
    data[i] = expr[i];
  return *this;
}

template<class T, int length, template <int> class CheckingPolicy>
inline FixedArray<T,length,CheckingPolicy>& FixedArray<T,length,CheckingPolicy>::clear()
{
  for (int i=0; i<length; ++i)
    data[i] = 0;
  return *this;
}

template<class T, int length, template <int> class CheckingPolicy>
inline FixedArray<T,length,CheckingPolicy>& FixedArray<T,length,CheckingPolicy>::fill(const T &value)
{
  for (int i=0; i<length; ++i)
    data[i] = value;
  return *this;
}


template<class T, int length, template <int> class CheckingPolicy>
inline FixedArray<T,length,CheckingPolicy> FixedArray<T,length,CheckingPolicy>::Zero()
{
  return FixedArray().fill(0);
}

template<class T, int length, template <int> class CheckingPolicy>
inline FixedArray<T,length,CheckingPolicy> FixedArray<T,length,CheckingPolicy>::Unity()
{
  return FixedArray().fill(1);
}



}


template<
  class T1, class T2,
  int length, 
  template<int> class CheckingPolicy1, template<int> class CheckingPolicy2
>
bool operator==(
  const schnek::FixedArray<T1,length,CheckingPolicy1>& arr1,
  const schnek::FixedArray<T2,length,CheckingPolicy2>& arr2
)
{
  for (int i=0; i<length; ++i) if (arr1[i]!=arr2[i]) return false;
  return true;
}

template<
  class T1, class T2,
  int length, 
  template<int> class CheckingPolicy1, template<int> class CheckingPolicy2
>
bool operator!=(
  const schnek::FixedArray<T1,length,CheckingPolicy1>& arr1,
  const schnek::FixedArray<T2,length,CheckingPolicy2>& arr2
)
{
  for (int i=0; i<length; ++i) if (arr1[i]!=arr2[i]) return true;
  return false;
}

template<
  class T1, class T2,
  int length, 
  template<int> class CheckingPolicy1, template<int> class CheckingPolicy2
>
bool operator<(
  const schnek::FixedArray<T1,length,CheckingPolicy1>& arr1,
  const schnek::FixedArray<T2,length,CheckingPolicy2>& arr2
)
{
  for (int i=0; i<length; ++i) if (arr1[i]>=arr2[i]) return false;
  return true;
}

template<
  class T1, class T2,
  int length, 
  template<int> class CheckingPolicy1, template<int> class CheckingPolicy2
>
bool operator<=(
  const schnek::FixedArray<T1,length,CheckingPolicy1>& arr1,
  const schnek::FixedArray<T2,length,CheckingPolicy2>& arr2
)
{
  for (int i=0; i<length; ++i) if (arr1[i]>arr2[i]) return false;
  return true;
}
