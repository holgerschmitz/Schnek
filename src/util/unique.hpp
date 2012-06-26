/*
 * unique.hpp
 *
 *  Created on: 26 Jun 2012
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
 */


#ifndef UNIQUE_HPP_
#define UNIQUE_HPP_

#include <boost/utility.hpp>

namespace schnek
{

template<class T>
class Unique : public boost::noncopyable
{
  private:
    static long runningId;
    long Id;
  public:
    Unique() : Id(runningId++) {}
    long getId() { return Id; }
};

template<class T>
long Unique<T>::runningId = 0;

} // namespace

#endif /* UNIQUE_HPP_ */
