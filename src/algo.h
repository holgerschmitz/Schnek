#ifndef SCHNEK_ALGO_H
#define SCHNEK_ALGO_H

namespace schnek {

template<class InputIterator>
typename InputIterator::value_type mean(InputIterator begin, InputIterator end);

#include "algo.t"

}


#endif
