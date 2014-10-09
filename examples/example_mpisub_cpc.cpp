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

int main(int argc, char **argv)
{
  typedef Grid<double, 3>::IndexType IndexType;
  IndexType globalSize(200,200,400);

  MPI_Init(&argc, &argv);
  {
  MPICartSubdivision<Grid<double, 3> > subdivision;

  subdivision.init(globalSize, 2);
  IndexType localLo = subdivision.getLo();
  IndexType localHi = subdivision.getHi();

  Grid<double, 3> grid(localLo, localHi);

  subdivision.exchange(grid);

  std::cout << subdivision.getUniqueId() << " ("
      << localLo[0] << " "<< localLo[1] << " "<< localLo[2] << ") ("
      << localHi[0] << " "<< localHi[1] << " "<< localHi[2] << ")"
      << std::endl;
  }
  MPI_Finalize();
}
