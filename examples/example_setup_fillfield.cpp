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
#include "../src/grid/field.hpp"
#include "../src/tools/fieldtools.hpp"

#include <iostream>
#include <fstream>

using namespace schnek;

class SimulationBlock : public Block
{
  private:

    Array<schnek::pParameter, 2> x_parameters;

    Array<int, 2> N;
    Array<double, 2> L;
    Array<double, 2> x;
    double fieldInit;
    pParameter paramField;
    pParametersGroup spaceVars;


  protected:
    void initParameters(BlockParameters &parameters)
    {
      parameters.addArrayParameter("N", N);
      parameters.addArrayParameter("L", L);
      x_parameters = parameters.addArrayParameter("", x, BlockParameters::readonly);
      paramField = parameters.addParameter("F", &fieldInit, 0.0);

      spaceVars = pParametersGroup(new ParametersGroup());
      spaceVars->addArray(x_parameters);
    }

  public:
    void printValues()
    {
      Range<double, 2> range(Array<double, 2>(0,0), L);

      Array<bool,2> stagger(false, true);
      Field<double, 2> dataField(N, range, stagger, 1);

      Array<bool,2> stagger2(true, false);
      Field<double, 2> dataField2(N, range, stagger2, 1);

      pBlockVariables blockVars = getVariables();
      pDependencyMap depMap(new DependencyMap(blockVars));

      DependencyUpdater updater(depMap);

      updater.addIndependentArray(x_parameters);
      fill_field(dataField, x, fieldInit, updater, paramField);
      fill_field(dataField2, x, fieldInit, updater, paramField);


      for (int i=0; i<=N[0]; ++i)
      {
        for (int j=0; j<=N[1]; ++j)
        {
          double px = dataField.indexToPosition(0,i);
          double py = dataField.indexToPosition(1,j);
          std::cout << px << " " << py  << " " << dataField(i,j) << std::endl;
        }
        std::cout << std::endl;
      }
      std::cout << std::endl;
      for (int i=0; i<=N[0]; ++i)
      {
        for (int j=0; j<=N[1]; ++j)
        {
          double px = dataField2.indexToPosition(0,i);
          double py = dataField2.indexToPosition(1,j);
          std::cout << px << " " << py  << " " << dataField2(i,j) << std::endl;
        }
        std::cout << std::endl;
      }
    }
};

int main()
{
  try
  {
    BlockClasses blocks;
    blocks.registerBlock("sim").setClass<SimulationBlock>();

    std::ifstream in("example_setup_fillfield.setup");
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

