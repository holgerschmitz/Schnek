#ifndef SCHNEK_CTASSERT_H
#define SCHNEK_CTASSERT_H


template<int> 
struct ctAssert;

template<> struct 
ctAssert<true> 
{
};

#define CTASSERT(expr, msg) \
  { \
    ctAssert<((expr) != 0)> ERROR_##msg; \
    (void)ERROR_##msg;  \
  } 


#endif
