/*
 * example_fields_cpc.cpp
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
  typedef Field<double, 3>::IndexType IndexType;

  Field<double, 3>::IndexType lo(0,0,0), hi(10,10,10);
  Field<double, 3>::RangeLimit physLo(-0.5, -0.5, -0.5), physHi(0.5, 0.5, 0.5);
  Field<double, 3>::DomainType physRange(physLo, physHi);
  Field<double, 3>::StaggerType stagger(true, false, true);

  Field<double, 3> field(lo, hi, physRange, stagger, 2);

  std::cout << field.indexToPosition(0,5) << std::endl;
  std::cout << field.indexToPosition(1,5) << std::endl;
  std::cout << field.indexToPosition(2,5) << std::endl;

  lo = field.getInnerLo();
  hi = field.getInnerHi();

}
