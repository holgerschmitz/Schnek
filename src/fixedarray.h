#ifndef SCHNEK_FIXEDARRAY_H
#define SCHNEK_FIXEDARRAY_H

#include "argcheck.h"

namespace schnek {

template<class Operator>
class FArrExpression;

/**A Fixed size array.
 * The three template parameters are:<br>
 * T: the type of data stored<br>
 * length: the length of the array<br>
 * CheckingPolicy: A policy class defining how to check the index passed to 
 * the accessor method
 */
template<
  class T, 
  int length, 
  template<int> class CheckingPolicy = FixedArrayNoArgCheck
>
class FixedArray : public CheckingPolicy<length> {
  private:
    /// Holds the data in a simple C-style array
    T data[length];
    /// ThisType defined for convenience
    typedef FixedArray<T,length,CheckingPolicy> ThisType;
  public:
    typedef T value_type;
    
    /// The default constructor
    FixedArray();
    /// Copy constructor copies the values
    FixedArray(const FixedArray &);
    /// Constructor for length=1 arrays setting the data explicitely
    FixedArray(const T&);
    /// Constructor for length=2 arrays setting the data explicitely
    FixedArray(const T&, const T&);
    /// Constructor for length=3 arrays setting the data explicitely
    FixedArray(const T&, const T&, const T&);
    /// Constructor for length=4 arrays setting the data explicitely
    FixedArray(const T&, const T&, const T&, const T&);
    /// Constructor for length=5 arrays setting the data explicitely
    FixedArray(const T&, const T&, const T&, const T&, const T&);
    /// Constructor for length=6 arrays setting the data explicitely
    FixedArray(const T&, const T&, const T&, const T&, const T&, 
               const T&);
    /// Constructor for length=7 arrays setting the data explicitely
    FixedArray(const T&, const T&, const T&, const T&, const T&, 
               const T&, const T&);
    /// Constructor for length=8 arrays setting the data explicitely
    FixedArray(const T&, const T&, const T&, const T&, const T&, 
               const T&, const T&, const T&);
    /// Constructor for length=9 arrays setting the data explicitely
    FixedArray(const T&, const T&, const T&, const T&, const T&, 
               const T&, const T&, const T&, const T&);
    /// Constructor for length=10 arrays setting the data explicitely
    FixedArray(const T&, const T&, const T&, const T&, const T&, 
               const T&, const T&, const T&, const T&, const T&);

    /** Constructing from an FArrExpression.
     *  The expression is evaluated at construction time so there are no temporary 
     *  variables
     */
    template<class Operator>
    FixedArray(const FArrExpression<Operator>&);
    
    ~FixedArray() {}

    /// Accessor operator
    T& operator[](int);
    /// Constant accessor operator
    const T& operator[](int) const;
    
    /// Accessor operator
    T& at(int);
    /// Constant accessor operator
    const T& at(int) const;
    
    /** Operator for assigning a FArrExpression to the array.
     *  The expression is evaluated at assign time so there are no temporary 
     *  variables
     */
    template<class Operator>
    FixedArray<T,length,CheckingPolicy>& operator=(const FArrExpression<Operator>&);
    

  public:
    /// Sets all fields to zero
    FixedArray<T,length,CheckingPolicy>& clear();
    /// Fills all fields with a given value
    FixedArray<T,length,CheckingPolicy>& fill(const T&);
    
  public:
  
    /** Returns an array filled with zeros.
     *  Only available if int can be cast to the type T
     */
    static FixedArray<T,length,CheckingPolicy> Zero();
    
    /** Returns an array filled with ones.
     *  Only available if int can be cast to the type T
     */
    static FixedArray<T,length,CheckingPolicy> Unity();
};



} // namespace

template<
  class T1, class T2,
  int length, 
  template<int> class CheckingPolicy1, template<int> class CheckingPolicy2
>
bool operator==(
  const schnek::FixedArray<T1,length,CheckingPolicy1>&,
  const schnek::FixedArray<T2,length,CheckingPolicy2>&
);

template<
  class T1, class T2,
  int length, 
  template<int> class CheckingPolicy1, template<int> class CheckingPolicy2
>
bool operator!=(
  const schnek::FixedArray<T1,length,CheckingPolicy1>&,
  const schnek::FixedArray<T2,length,CheckingPolicy2>&
);

template<
  class T1, class T2,
  int length, 
  template<int> class CheckingPolicy1, template<int> class CheckingPolicy2
>
bool operator<(
  const schnek::FixedArray<T1,length,CheckingPolicy1>&,
  const schnek::FixedArray<T2,length,CheckingPolicy2>&
);

template<
  class T1, class T2,
  int length, 
  template<int> class CheckingPolicy1, template<int> class CheckingPolicy2
>
bool operator<=(
  const schnek::FixedArray<T1,length,CheckingPolicy1>&,
  const schnek::FixedArray<T2,length,CheckingPolicy2>&
);


#include "fixedarray.t"

#endif
