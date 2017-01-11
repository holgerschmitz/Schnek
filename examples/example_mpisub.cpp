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

void runSimulation() {
  const int N = 400;
  double dx = 2.0/N;
  double dt = 1e-3;
  Array<int,2> lo(-N/2,-N/2);
  Array<int,2> hi( N/2, N/2);

  Array<double,2> domainLo(-1.0, -1.0);
  Array<double,2> domainHi( 1.0,  1.0);

  Range<double, 2> domain(domainLo, domainHi);
  Array<bool, 2> stagger(false, false);

  MPICartSubdivision<Field<double, 2> > subdivision;

  subdivision.init(lo, hi, 1);

  Array<int,2> localLo = subdivision.getInnerLo();
  Array<int,2> localHi = subdivision.getInnerHi();
  Range<double, 2> localDomain = subdivision.getInnerExtent(domain);

  Field<double, 2> field(localLo, localHi, localDomain, stagger, 1);

  for (int i=localLo[0]; i<=localHi[0]; ++i)
    for (int j=localLo[1]; j<=localHi[1]; ++j) {
      double x = field.indexToPosition(0,i);
      double y = field.indexToPosition(1,j);
      field(i,j) = exp(-25*(x*x + y*y));
    }

  subdivision.exchange(field);

  for (int t=0; t<1000; ++t) {

    for (int i=localLo[0]; i<=localHi[0]; ++i)
      for (int j=localLo[1]; j<=localHi[1]; j+=2) {
        field(i,j) = field(i,j)
            + dt*(field(i-1,j) + field(i+1,j)
                + field(i,j-1) + field(i,j+1) - 4*field(i,j));
      }

    subdivision.exchange(field);
  }

  std::cout << subdivision.getUniqueId() << " ("
      << localLo[0] << " "<< localLo[1] << ") ("
      << localHi[0] << " "<< localHi[1] << ")"
      << std::endl;

}

int main(int argc, char **argv)
{
  MPI_Init(&argc, &argv);

  runSimulation();

  MPI_Finalize();
}
