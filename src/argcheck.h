#ifndef SCHNEK_ARGCHECK_H
#define SCHNEK_ARGCHECK_H

namespace schnek {

/** Class to plug into the FixedArray as CheckingPolicy.
 *  Performs no argument checking at all.
 */
template<int limit>
class FixedArrayNoArgCheck
{
  public:
    /** The check method does not do anything */
    void check(int) const {} 
};

}

#endif
