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

template<class GridType>
void squareValues(GridType &grid) {
  Array<int,2> lo = grid.getLo();
  Array<int,2> hi = grid.getHi();

  for (int i=lo[0]; i<=hi[0]; ++i)
    for (int j=lo[1]; j<=hi[1]; ++j)
        grid(i,j) = grid(i,j)*grid(i,j);
}

int main()
{
  Array<int,2> lo(10,10);
  Array<int,2> hi(20,20);
  Grid<int, 2> grid(lo,hi);

  grid = 2;

  Range<int, 2> range(grid.getLo(), grid.getHi());
  range.grow(-3);

  SubGrid<Grid<int, 2> > subGrid(range,grid);

  squareValues(subGrid);

  for (int i=lo[0]; i<=hi[0]; ++i) {
    for (int j=lo[1]; j<=hi[1]; ++j)
        std::cout << grid(i,j) << " ";
    std::cout << std::endl;
  }
}
