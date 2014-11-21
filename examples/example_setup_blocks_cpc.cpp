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

class ForceModule : public Block
{
  private:
    double strength;
  protected:
    void initParameters(BlockParameters &parameters)
    {
      parameters.addParameter("strength", &strength);
    }
  public:
    void writeValues()
    {
      std::cout << "strength = " << strength << std::endl;
    }
};

class SimulationBlock : public Block
{
  private:
    int size;
    double dx;
    std::string info;
    Array<double, 3> velocity;

  protected:
    void initParameters(BlockParameters &parameters)
    {
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
      std::cout << "velocity = " << velocity[0] << " " << velocity[1] << " "
          << velocity[2] << std::endl;

      BlockList children = this->getChildren();
      for (BlockList::iterator it=children.begin(); it!=children.end(); ++it)
      {
        ForceModule &force = dynamic_cast<ForceModule&>(**it);
        std::cout << "\n== Force ==\n\n";
        force.writeValues();
      }
    }
};


int main()
{
  try
  {
    BlockClasses blocks;

    blocks.registerBlock("sim").setClass<SimulationBlock>();
    blocks.registerBlock("Force").setClass<ForceModule>();
    blocks("sim").addChildren("Force");

    std::ifstream in("example_setup_blocks_cpc.setup");

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

