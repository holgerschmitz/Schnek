/*
 * example_diagnostic_hdf5.cpp
 *
 *  Created on: 17 Jan 2017
 *      Author: Holger Schmitz
 *       Email: holger@notjustphysics.com
 */

#include "../src/grid.hpp"
#include "../src/diagnostic/diagnostic.hpp"
#include "../src/diagnostic/hdfdiagnostic.hpp"

#include "../src/grid/arrayexpression.hpp"
#include "../src/parser.hpp"
#include "../src/variables/block.hpp"
#include <cmath>
#include <boost/make_shared.hpp>

using namespace schnek;

typedef Array<double, 2> Coord;

std::ostream &operator<<(std::ostream &out, const Coord &c)
{
  out << c[0] << " " << c[1] << std::endl;
}

typedef SimpleFileDiagnostic<Coord, Coord*> Diagnostic;

class Simulation : public Block
{
  private:
    Coord pos;
    Coord vel;
    double G;
    double dt;

    int step;
    int Nsteps;
  protected:
    void initParameters(BlockParameters &parameters);
  public:
    void registerData();
    void runSimulation();
};

void Simulation::initParameters(BlockParameters &parameters)
{
  parameters.addArrayParameter("", pos);
  parameters.addArrayParameter("v", vel);
  parameters.addParameter("G", &G);
  parameters.addParameter("dt", &dt);
  parameters.addParameter("Nsteps", &Nsteps);
}

void Simulation::registerData() {
  addData("pos", pos);
  addData("vel", vel);
}

void Simulation::runSimulation() {
  DiagnosticManager::instance().setTimeCounter(&step);
  DiagnosticManager::instance().execute();

  for (step=0; step<=Nsteps; ++step) {
    Coord acc = -G*pos/pow(pos.sqr(),1.5);
    vel += dt*acc;
    pos += dt*vel;
    DiagnosticManager::instance().execute();
  }
}

int main(int argc, char **argv)
{
  MPI_Init(&argc, &argv);

  BlockClasses blocks;

  blocks.registerBlock("Simulation").setClass<Simulation>();
  blocks.registerBlock("Diagnostic").setClass<Diagnostic>();

  blocks("Simulation").addChildren("Diagnostic");

  std::ifstream in("example_diagnostic_simple.setup");
  Simulation *mysim;
  try
  {
    Parser P("simulation", "Simulation", blocks);
    registerCMath(P.getFunctionRegistry());
    pBlock application = P.parse(in);

    mysim = dynamic_cast<Simulation*>(application.get());
    mysim->initAll();
    mysim->runSimulation();
  }
  catch (schnek::ParserError &e)
  {
    std::cerr << "Parse error in " << e.getFilename() << " at line "
        << e.getLine() << ": " << e.message << std::endl;
    return -1;
  }
  catch (schnek::VariableNotInitialisedException &e)
  {
    std::cerr << "Variable was not initialised: " << e.getVarName() << std::endl;
    return -1;
  }
  catch (schnek::EvaluationException &e)
  {
    std::cerr << "Error in evaluation: " << e.getMessage() << std::endl;
    return -1;
  }
  catch (schnek::VariableNotFoundException &e)
  {
    std::cerr << "Error: " << e.getMessage() << std::endl;
    return -1;
  }
  catch (SchnekException &e)
  {
    std::cerr << "An error occured" << std::endl;
    return -1;
  }
  catch (std::string &err)
  {
    std::cerr << "FATAL ERROR: " << err << std::endl;
    return -1;
  }

  MPI_Finalize();
}
