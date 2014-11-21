/*
 * example_setup_first.cpp
 *
 *  Created on: 8 Jul 2014
 *      Author: Holger Schmitz
 *       Email: holger@notjustphysics.com
 */


#include "../src/parser/parser.hpp"
#include "../src/variables/block.hpp"
#include "../src/grid/array.hpp"

#include <iostream>
#include <fstream>

using namespace schnek;

class SimulationBlock : public Block
{
  private:
    Array<double, 3> pos;

  protected:
    void initParameters(BlockParameters &parameters)
    {
      parameters.addArrayParameter("pos", pos, -1.0);
    }

  public:
    void writeValues()
    {
      std::cout << "pos = (" << pos[0] << ", "<< pos[1] << ", "<< pos[2] << ")" << std::endl;
    }
};

int main()
{
  BlockClasses blocks;

  blocks.registerBlock("sim").setClass<SimulationBlock>();

//  blocks.addBlock("sim");
//  blocks("sim").setClass<SimulationBlock>();

  std::ifstream in("example_setup_array.setup");

  Parser P("my_simulation", "sim", blocks);
  registerCMath(P.getFunctionRegistry());
  pBlock application = P.parse(in);

  SimulationBlock &mysim = dynamic_cast<SimulationBlock&>(*application);
  mysim.evaluateParameters();
  mysim.writeValues();

  return 0;
}




