/*
 * example_grids_basics.cpp
 *
 *  Created on: 2 Jul 2014
 *      Author: Holger Schmitz
 *       Email: holger@notjustphysics.com
 */


#include "../src/grid.hpp"

int main()
{
  schnek::Grid<double,2> grid(schnek::Grid<double,2>::IndexType(5,5));

  grid = 1;

  for (int i=0; i<5; ++i)
    for (int j=0; j<5; ++j)
      grid(j,j) = i + 2*i*j;


  for (int i=0; i<5; ++i)
  {
    for (int j=0; j<5; ++j)
      std::cout << grid(i,j) << " ";
    std::cout << std::endl;
  }

}
