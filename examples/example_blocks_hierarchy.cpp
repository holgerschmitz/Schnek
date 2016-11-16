/*
 * example_blocks_hieararchy.cpp
 *
 *  Created on: 11 Aug 2014
 *      Author: Holger Schmitz
 *       Email: holger@notjustphysics.com
 */


#include "../src/grid/array.hpp"
#include "../src/grid/arrayexpression.hpp"
#include "../src/parser.hpp"
#include "../src/variables/block.hpp"

#include <iostream>
#include <fstream>

using namespace schnek;

class Particle;

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
    void runSimulation();
};

class Particle : public Block
{
  private:
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
  public:
    void advance(const Array<double,3> &force, double dt) {
      pos = pos+velocity*dt;
      velocity = velocity + force*dt/mass;
    }

    void display()
    {
      std::cout << "Mass: " << mass << std::endl;
      std::cout << "Position: (" << pos[0] << ", " << pos[1] << ", " << pos[1] << ")" << std::endl;
      std::cout << "Velocity: (" << velocity[0] << ", " << velocity[1] << ", " << velocity[1] << ")" << std::endl;
    }
};

void NBody::runSimulation()
{
  BlockList children = getChildren();
  Array<double,3> force(0.0,0.0,0.0);
  for (double time=0.0; time<=totalTime; time+=dt)
  {
    BOOST_FOREACH(pBlock block, children)
    {
      boost::shared_ptr<Particle> particle = boost::dynamic_pointer_cast<Particle>(block);
      particle->advance(force, dt);
    }
  }
  BOOST_FOREACH(pBlock block, children)
  {
    std::cout << "Child: " << block->getName() << std::endl;
    boost::shared_ptr<Particle> particle = boost::dynamic_pointer_cast<Particle>(block);
    particle->display();
  }
}


int main()
{
  BlockClasses blocks;

  blocks.registerBlock("NBody").setClass<NBody>();
  blocks.registerBlock("Particle").setClass<Particle>();

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
  mysim->runSimulation();
  return 0;
}







