/*
 * example_grids_basics.cpp
 *
 *  Created on: 2 Jul 2014
 *      Author: Holger Schmitz
 *       Email: holger@notjustphysics.com
 */

#include "../src/grid/range.hpp"

#include <iostream>

using namespace schnek;

int main()
{
  Range<int, 2>::LimitType low(1, 2);
  Range<int, 2>::LimitType high(4, 5);

  // create range using limits
  Range<int, 2> range(low, high);

  // read lower and upper bounds
  low  = range.getLo();
  high = range.getHi();

  // modify bounds
  range.getLo() = Range<int, 2>::LimitType(5,2);
  range.getHi() = Range<int, 2>::LimitType(9,7);

  range.grow(-1);

  // iterate over range
  for (Range<int, 2>::iterator it=range.begin(); it!=range.end(); ++it)
  {
    const Range<int, 2>::LimitType &pos = *it;
    std::cout << pos[0] << " " << pos[1] << std::endl;
  }

}
