/*
 * example_diagnostic_hdf5.cpp
 *
 *  Created on: 17 Jan 2017
 *      Author: Holger Schmitz
 *       Email: holger@notjustphysics.com
 */

#ifdef SCHNEK_HAVE_HDF5

#include "../src/grid.hpp"
#include "../src/diagnostic/diagnostic.hpp"
#include "../src/diagnostic/hdfdiagnostic.hpp"

#include "../src/grid/arrayexpression.hpp"
#include "../src/parser.hpp"
#include "../src/variables/block.hpp"
#include <cmath>

using namespace schnek;

typedef Field<double, 2> SimField;

const int N = 400;
double dx = 2.0/N;
double dt = 1e-3;

Array<int,2> lo(0,0);
Array<int,2> hi(N,N);

class Diagnostic : public HDFGridDiagnostic<SimField, SimField*, DeltaTimeDiagnostic>
{
  protected:
    Array<int,2> getGlobalMin() {
      return lo;
    }

    Array<int,2> getGlobalMax() {
      return hi;
    }
};

class Simulation : public Block
{
  private:
    MPICartSubdivision<Field<double, 2> > subdivision;
    Field<double, 2> field;
  public:
    void preInit();
    void runSimulation();
};

void Simulation::preInit() {
  Array<double,2> domainLo(-1.0, -1.0);
  Array<double,2> domainHi( 1.0,  1.0);

  Range<double, 2> domain(domainLo, domainHi);
  Array<bool, 2> stagger(false, false);

  subdivision.init(lo, hi, 1);

  Array<int,2> localLo = subdivision.getInnerLo();
  Array<int,2> localHi = subdivision.getInnerHi();
  Range<double, 2> localDomain = subdivision.getInnerExtent(domain);

  field.resize(localLo, localHi, localDomain, stagger, 1);

  addData("field", field);
}

void Simulation::runSimulation() {
  Array<int,2> localLo = subdivision.getInnerLo();
  Array<int,2> localHi = subdivision.getInnerHi();

  Field<double, 2> *pfield = &field;
  double time = 0.0;

  DiagnosticManager::instance().setPhysicalTime(&time);

  for (int i=localLo[0]; i<=localHi[0]; ++i)
    for (int j=localLo[1]; j<=localHi[1]; ++j) {
      double x = field.indexToPosition(0,i);
      double y = field.indexToPosition(1,j);
      field(i,j) = exp(-25*(x*x + y*y));
    }

  subdivision.exchange(field);

  DiagnosticManager::instance().execute();

  for (int t=0; t<1000; ++t) {

    for (int i=localLo[0]; i<=localHi[0]; ++i)
      for (int j=localLo[1]; j<=localHi[1]; j+=2) {
        field(i,j) = field(i,j)
            + dt*(field(i-1,j) + field(i+1,j)
                + field(i,j-1) + field(i,j+1) - 4*field(i,j));
      }

    subdivision.exchange(field);

    time += dt;
    DiagnosticManager::instance().execute();
  }

  std::cout << subdivision.getUniqueId() << " ("
      << localLo[0] << " "<< localLo[1] << ") ("
      << localHi[0] << " "<< localHi[1] << ")"
      << std::endl;

}

int main(int argc, char **argv)
{
  MPI_Init(&argc, &argv);

  BlockClasses blocks;

  blocks.registerBlock("Simulation").setClass<Simulation>();
  blocks.registerBlock("Diagnostic").setClass<Diagnostic>();

  blocks("Simulation").addChildren("Diagnostic");

  std::ifstream in("example_diagnostic_hdf5.setup");
  Simulation *mysim;
  try
  {
    Parser P("simulation", "Simulation", blocks);
    registerCMath(P.getFunctionRegistry());
    pBlock application = P.parse(in);

    mysim = dynamic_cast<Simulation*>(application.get());
    mysim->initAll();
  }
  catch (ParserError &e)
  {
    std::cerr << "Parse error in " << e.getFilename() << " at line " << e.getLine() << ": "<< e.message << std::endl;
    exit(-1);
  }
  catch (EvaluationException &e)
  {
    std::cerr << "Evaluation Error: " << e.getMessage() << std::endl;
    exit(-1);
  }
  mysim->runSimulation();

  MPI_Finalize();
}

#else // SCHNEK_HAVE_HDF5

int main(int argc, char **argv)
{
}

#endif // SCHNEK_HAVE_HDF5
