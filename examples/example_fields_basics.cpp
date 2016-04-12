/*
 * example_fields_basics.cpp
 *
 *  Created on: 5 Apr 2016
 *  Author: Holger Schmitz
 *  Email: holger@notjustphysics.com
 */

#include "../src/grid.hpp"
#include <cmath>

using namespace schnek;

int main()
{
  Field<double, 3>::IndexType lo(0,0,0), hi(9,9,9);
  Field<double, 3>::RangeLimit physLo(-0.5, -0.5, -0.5), physHi(0.5, 0.5, 0.5);
  Field<double, 3>::RangeType physRange(physLo, physHi);
  Field<double, 3>::Stagger stagger(false, true, false);

  Field<double, 3> field(lo, hi, physRange, stagger, 2);


  lo = field.getLo();
  hi = field.getHi();

  std::cout << "Outer Lo Index (" << lo[0] << ", " << lo[1] << ", " << lo[2] << ")" << std::endl;
  std::cout << "Outer Hi Index (" << hi[0] << ", " << hi[1] << ", " << hi[2] << ")" << std::endl;

  lo = field.getInnerLo();
  hi = field.getInnerHi();

  std::cout << "Inner Lo Index (" << lo[0] << ", " << lo[1] << ", " << lo[2] << ")" << std::endl;
  std::cout << "Inner Hi Index (" << hi[0] << ", " << hi[1] << ", " << hi[2] << ")" << std::endl;

  std::cout << "x_5 = " << field.indexToPosition(0, 5) << std::endl;
  std::cout << "y_5 = " << field.indexToPosition(1, 5) << std::endl;
  std::cout << "z_5 = " << field.indexToPosition(2, 5) << std::endl;

  std::cout << "Index at x=0.2: " << field.positionToIndex(0, 0.2) << std::endl;
  std::cout << "Index at y=0.2: " << field.positionToIndex(1, 0.2) << std::endl;
  std::cout << "Index at z=0.2: " << field.positionToIndex(2, 0.2) << std::endl;

  int index;
  double offset;

  field.positionToIndex(0, 0.2, index, offset);
  std::cout << "Index (offset) at x=0.2: " << index << " (" << offset << ")" << std::endl;
  field.positionToIndex(1, 0.2, index, offset);
  std::cout << "Index (offset) at y=0.2: " << index << " (" << offset << ")" << std::endl;
  field.positionToIndex(2, 0.2, index, offset);
  std::cout << "Index (offset) at z=0.2: " << index << " (" << offset << ")" << std::endl;


}
