/*
 * example_blocks_hieararchy.cpp
 *
 *  Created on: 11 Aug 2014
 *      Author: Holger Schmitz
 *       Email: holger@notjustphysics.com
 */


#include "../src/grid/array.hpp"
#include "../src/parser.hpp"
#include "../src/variables/block.hpp"

#include <iostream>
#include <fstream>

using namespace schnek;

class NBody : public Block
{
  private:
    double dt;
    double totalTime;

  protected:
    void initParameters(BlockParameters &parameters)
    {
      parameters.addParameter("dt", &dt);
      parameters.addParameter("totalTime", &totalTime);
    }

  public:
    void showChildren()
    {
      BlockList children = getChildren();
      BOOST_FOREACH(pBlock block, children)
      {
        std::cout << "Child: " << block->getName() << std::endl;
      }
    }

    void runSimulation()
    {
      for (double time=0.0; time<totalTime; time += dt)
      {
      }
    }
};

class Particle : public Block
{
  public:
    Array<double,3> pos;
    Array<double,3> velocity;
    double mass;

  protected:
    void initParameters(BlockParameters &parameters)
    {
      parameters.addArrayParameter("pos", pos);
      parameters.addArrayParameter("velocity", pos);
      parameters.addParameter("mass", &mass);
    }
};



int main()
{
  BlockClasses blocks;

  blocks.addBlock("NBody").setClass<NBody>();
  blocks.addBlock("Particle").setClass<Particle>();

  blocks("NBody").addChildren("Particle");

  std::ifstream in("example_blocks_hierarchy01.setup");
  NBody *mysim;
  try
  {
    Parser P("nbody_simulation", "NBody", blocks);
    registerCMath(P.getFunctionRegistry());
    pBlock application = P.parse(in);

    mysim = dynamic_cast<NBody*>(application.get());
    mysim->evaluateParameters();
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
  mysim->showChildren();
  mysim->runSimulation();
  return 0;
}







