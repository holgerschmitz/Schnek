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

class SuperGauss {
  private:
    double r;
  public:
    SuperGauss(double r_ = 1.0) : r(r_) {}
    double operator()(double x) const {
      return exp(-std::pow(x/r, 4));
    }
};

template<class GridTypeIn, class GridTypeOut>
void laplace_sq(GridTypeIn &gridIn, GridTypeOut &gridOut) {
  Array<int,2> lo = gridIn.getLo();
  Array<int,2> hi = gridIn.getHi();

  for (int i=lo[0]+1; i<hi[0]; ++i)
    for (int j=lo[1]+1; j<hi[1]; ++j) {
         double laplace = gridIn(i-1,j) + gridIn(i+1,j)
                        + gridIn(i,j-1) + gridIn(i,j+1)
                        - 4*gridIn(i,j);
         gridOut(i,j) = laplace*laplace;
    }
}

int main()
{
  const int N = 200;
  Array<int,2> lo(-N,-N);
  Array<int,2> hi( N, N);
  Grid<double, 2> grid(lo, hi);
  Grid<double, 2> gridOut(lo, hi);

  for (int i=lo[0]; i<=hi[0]; ++i)
    for (int j=lo[1]; j<=hi[1]; ++j)
      grid(i,j) = sqrt(i*i +j*j);

  GridTransform<Grid<double, 2>, SuperGauss> gridT(grid);

  gridT.setTransformation(SuperGauss(100.0));

  laplace_sq(gridT, gridOut);

  for (int i=lo[0]; i<=hi[0]; ++i) {
    for (int j=lo[1]; j<=hi[1]; ++j)
        std::cout << gridT(i,j) << " ";
    std::cout << std::endl;
  }
}
