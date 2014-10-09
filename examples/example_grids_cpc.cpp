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
  Grid<double, 3> grid(IndexType(5, 7, 10));
  Grid<double, 3> gridB(IndexType(2, 2, 2), IndexType(10, 10, 10));

  grid = 1.0;
  grid(2, 3, 7) = 2.5;

  double sum = 0.0;
  for (Grid<double, 3>::storage_iterator iter = grid.begin();
      iter != grid.end(); ++iter)
  {
    sum += *iter;
  }
  std::cout << "Sum " << sum << std::endl;

  for (int i=gridB.getLo(0); i<=gridB.getHi(0); ++i)
    for (int j=gridB.getLo(1); j<=gridB.getHi(1); ++j)
      for (int k=gridB.getLo(2); k<=gridB.getHi(2); ++k)
        gridB(i,j,k) = sqrt(gridB(i,j,k));

  grid.resize(IndexType(5, 5, 5));

  Grid<double, 3, GridAssertCheck> gridCheck(IndexType(3, 3, 3));

  // causes assert failure
  // gridCheck(2,2,5) = 1.0;


  Grid<double, 3,
       GridNoArgCheck,
       SingleArrayGridStorageFortran> gridF(IndexType(3, 3, 3));

  double val = 1.0;
  for (int k=0; k<3; ++k)
    for (int j=0; j<3; ++j)
      for (int i=0; i<3; ++i)
      {
        gridF(i,j,k) = val;
        val += 1.0;
      }

  double *data = gridF.getRawData();
  for (int i=0; i<gridF.getSize(); ++i)
    std::cout << data[i] << " ";
  std::cout << std::endl;


}
