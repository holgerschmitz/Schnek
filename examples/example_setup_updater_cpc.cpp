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
#include "../src/tools/fieldtools.hpp"

#include <iostream>
#include <fstream>

using namespace schnek;

class SimulationBlock : public Block
{
  private:
    double fieldInit;
    pParameter fieldParam;

    Array<double, 2> size;
    Array<int, 2> gridSize;

    Array<double, 2> coord;
    Array<pParameter, 2> coord_parameters;

  protected:
    void initParameters(BlockParameters &parameters)
    {
      coord_parameters = parameters.addArrayParameter("", coord,
          BlockParameters::readonly);

      parameters.addArrayParameter("size", size);
      parameters.addArrayParameter("N", gridSize);
      fieldParam = parameters.addParameter("F", &fieldInit);
    }

  public:
    void initField(pBlockVariables blockVars)
    {
      Range<double,2> physDomain(Array<double, 2>(0,0), size);
      Array<bool, 2> stagger(false, false);

      pDependencyMap depMap(new DependencyMap(blockVars));
      DependencyUpdater updater(depMap);
      updater.addIndependentArray(coord_parameters);

      Field<double, 2> field(gridSize, physDomain, stagger, 2);

      fill_field(field, coord, fieldInit, updater, fieldParam);

      for (int i=field.getLo(0); i<=field.getHi(0); ++i)
      {
        for (int j=field.getLo(1); j<=field.getHi(1); ++j)
          std::cout << i << " " << j << " " << field(i,j) << std::endl;

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

    std::ifstream in("example_setup_updater_cpc.setup");
    Parser P("my_simulation", "sim", blocks);
    registerCMath(P.getFunctionRegistry());
    pBlock application = P.parse(in);

    SimulationBlock &mysim = dynamic_cast<SimulationBlock&>(*application);
    mysim.evaluateParameters();

    mysim.initField(P.getVariableStorage().getRootBlock());

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

