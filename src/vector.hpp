/*
 * vector.h
 *
 *  Created on: 8 Oct 2010
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
 */

#ifndef VECTOR_H_
#define VECTOR_H_

#include "array.hpp"

namespace schnek {

template<
  class T,
  int length,
  template<int> class CheckingPolicy = FixedArrayNoArgCheck
>
class Vector : public Array<T,length,CheckingPolicy>
{
  public:
    T product() const;
    T sum() const;
};

}

#endif /* VECTOR_H_ */