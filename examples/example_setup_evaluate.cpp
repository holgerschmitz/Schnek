/*
 * example_setup_first.cpp
 *
 *  Created on: 8 Jul 2014
 *      Author: Holger Schmitz
 *       Email: holger@notjustphysics.com
 */

#include "../src/parser.hpp"
#include "../src/variables/block.hpp"
#include "../src/variables/dependencies.hpp"
#include "../src/grid/array.hpp"

#include <iostream>
#include <fstream>

using namespace schnek;

class SimulationBlock : public Block
{
  private:
    double t;
    pParameter paramT;
    double value;
    pParameter paramValue;


  protected:
    void initParameters(BlockParameters &parameters)
    {
      paramT = parameters.addParameter("t", &t, BlockParameters::readonly);
      paramValue = parameters.addParameter("value", &value);
    }

  public:
    void printValues()
    {
      pBlockVariables blockVars = getVariables();
      pDependencyMap depMap(new DependencyMap(blockVars));
      DependencyUpdater updater(depMap);

      updater.addIndependent(paramT);
      updater.addDependent(paramValue);


      for (int i=0; i<=20; ++i)
      {
        t = 0.5*i;
        updater.update();
        std::cout << t << " " << value << std::endl;
      }
    }
};

int main()
{
  try
  {
    BlockClasses blocks;
    blocks.registerBlock("sim").setClass<SimulationBlock>();

    std::ifstream in("example_setup_evaluate.setup");
    Parser P("my_simulation", "sim", blocks);
    registerCMath(P.getFunctionRegistry());
    pBlock application = P.parse(in);

    SimulationBlock &mysim = dynamic_cast<SimulationBlock&>(*application);
    mysim.evaluateParameters();

    mysim.printValues();

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

