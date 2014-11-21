/*
 * example_setup_first.cpp
 *
 *  Created on: 8 Jul 2014
 *      Author: Holger Schmitz
 *       Email: holger@notjustphysics.com
 */

#include "../src/parser.hpp"
#include "../src/variables/block.hpp"
#include "../src/grid/array.hpp"

#include <iostream>
#include <fstream>

using namespace schnek;

class SimulationBlock : public Block
{
  private:
    int size;
    double dx;
    std::string info;
    Array<double, 2> velocity;

  protected:
    void initParameters(BlockParameters &parameters)
    {
      parameters.addConstant("pi", M_PI);
      parameters.addParameter("size", &size);
      parameters.addParameter("dx", &dx, 0.01);
      parameters.addParameter("info", &info,
          std::string("No information supplied."));
      parameters.addArrayParameter("v", velocity, 0.0);
    }

  public:
    void writeValues()
    {
      std::cout << "size = " << size << std::endl;
      std::cout << "dx = " << dx << std::endl;
      std::cout << "info = " << info << std::endl;
      std::cout << "velocity = " << velocity[0] << " " << velocity[1]
          << std::endl;
    }
};

int main()
{
  BlockClasses blocks;

  blocks.registerBlock("sim").setClass<SimulationBlock>();

  std::ifstream in("example_setup_cpc.setup");
  try
  {
    Parser P("my_simulation", "sim", blocks);
    registerCMath(P.getFunctionRegistry());
    pBlock application = P.parse(in);

    SimulationBlock &mysim = dynamic_cast<SimulationBlock&>(*application);
    mysim.evaluateParameters();
    mysim.writeValues();
  }
  catch (ParserError &e)
  {
    std::cerr << "Parse error in " << e.getFilename() << " at line "
        << e.getLine() << ": " << e.message << std::endl;
    exit(-1);
  }
  catch (EvaluationException &e)
  {
    std::cerr << "Evaluation Error: " << e.getMessage() << std::endl;
    exit(-1);
  }

  return 0;
}

