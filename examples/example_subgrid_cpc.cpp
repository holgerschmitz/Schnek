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
  typedef Grid<double, 3>::IndexType IndexType;
  Grid<double, 3> grid(IndexType(10, 10, 10), IndexType(20, 20, 20));

  Range<int, 3> range(grid.getLo(), grid.getHi());
  range.grow(-2);

  SubGrid<Grid<double, 3> > subGrid(range,grid);


  for (int i=subGrid.getLo(0); i<=subGrid.getHi(0); ++i)
    for (int j=subGrid.getLo(1); j<=subGrid.getHi(1); ++j)
      for (int k=subGrid.getLo(2); k<=subGrid.getHi(2); ++k)
        subGrid(i,j,k) = sqrt(subGrid(i,j,k));

}
