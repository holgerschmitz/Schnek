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

class Square {
  public:
    typedef double value_type;
  public:
    double operator()(double x) {return x*x; }
};

int main()
{
  typedef Grid<double, 3>::IndexType IndexType;
  Grid<double, 3> grid(IndexType(2, 2, 2));

  grid = 2.0;

  GridTransform<Grid<double, 3>, Square> gridT(grid);


  for (int i=gridT.getLo(0); i<=gridT.getHi(0); ++i)
    for (int j=gridT.getLo(1); j<=gridT.getHi(1); ++j)
      for (int k=gridT.getLo(2); k<=gridT.getHi(2); ++k)
        std::cout << gridT(i,j,k) << std::endl;

}
