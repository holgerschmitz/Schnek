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
    int size;
    double dx;
    std::string info;

  protected:
    void initParameters(BlockParameters &parameters)
    {
      parameters.addParameter("size", &size, 100);
      parameters.addParameter("dx", &dx, 0.01);
      parameters.addParameter("info", &info, std::string("No information supplied."));
    }

  public:
    void writeValues()
    {
      std::cout << "size = " << size << std::endl;
      std::cout << "dx = " << dx << std::endl;
      std::cout << "info = " << info << std::endl;
    }
};

int main()
{
  BlockClasses blocks;

  blocks.registerBlock("sim").setClass<SimulationBlock>();

//  blocks.addBlock("sim");
//  blocks("sim").setClass<SimulationBlock>();

  std::ifstream in("example_setup_defaults01.setup");

  Parser P("my_simulation", "sim", blocks);
  registerCMath(P.getFunctionRegistry());
  pBlock application = P.parse(in);

  SimulationBlock &mysim = dynamic_cast<SimulationBlock&>(*application);
  mysim.evaluateParameters();
  mysim.writeValues();

  return 0;
}




