#ifndef SCHNEK_ARGCHECK_HPP
#define SCHNEK_ARGCHECK_HPP

namespace schnek {

/** Class to plug into the Array as CheckingPolicy.
 *  Performs no argument checking at all.
 */
template<int limit>
class ArrayNoArgCheck
{
  public:
    /** The check method does not do anything */
    void check(int) const {} 
};

}

#endif
