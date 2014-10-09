/*
 * example_grids_basics.cpp
 *
 *  Created on: 2 Jul 2014
 *      Author: Holger Schmitz
 *       Email: holger@notjustphysics.com
 */

#include "../src/grid.hpp"
#include <cmath>

using namespace schnek;

int main()
{
  Array<int, 3> low(2,2,2);
  Array<int, 3> high(12,12,12);
  Range<int, 3> range(low, high);

  range.grow(-2);

  for (Range<int, 3>::iterator iter = range.begin();
       iter != range.end();
       ++iter)
  {
    Array<int, 3> p = *iter;
    std::cout << p[0] << " " << p[1] << " " << p[2] << std::endl;
  }

  bool isInRange = range.inside(low);
}
