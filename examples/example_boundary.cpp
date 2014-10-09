/*
 * example_array.cpp
 *
 *  Created on: 14 Jul 2014
 *      Author: Holger Schmitz
 *       Email: holger@notjustphysics.com
 */

#include "../src/grid/boundary.hpp"

using namespace schnek;

void writeRange(std::string name, const Range<int, 3> &range)
{
  std::cout << name << " ("
      << range.getLo()[0] << ", "
      << range.getLo()[1] << ", "
      << range.getLo()[2] << ") - ("
      << range.getHi()[0] << ", "
      << range.getHi()[1] << ", "
      << range.getHi()[2] << ")" << std::endl;
}

int main()
{
  // Example 1
  {
    Array<int, 3> lo(0,0,0), hi(10, 10, 10);
    Boundary<3> boundary(lo, hi, 2);

    Range<int, 3> ghost = boundary.getGhostDomain(1,Boundary<3>::Max);
    Range<int, 3> source = boundary.getGhostSourceDomain(1,Boundary<3>::Max);
    Range<int, 3> inner = boundary.getInnerDomain();

    writeRange("ghost  = ", ghost);
    writeRange("source = ", source);
    writeRange("inner  = ", inner);

  }

}
