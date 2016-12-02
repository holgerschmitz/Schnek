/*
 * example_blocks_lifecycle.cpp
 *
 *  Created on: 30 Nov 2016
 *      Author: Holger Schmitz
 *       Email: holger@notjustphysics.com
 */


#include "../src/parser.hpp"
#include "../src/variables/block.hpp"
#include "../src/variables/blockcontainer.hpp"

#include <iostream>
#include <fstream>

using namespace schnek;

class Particle;
class Force;
class Nested;

class NBody : public Block, public BlockContainer<Particle>, public BlockContainer<Force>, public BlockContainer<Nested>
{
  protected:
    void initParameters(BlockParameters &parameters)
    {
      std::cout << "Root Block initParameters" << std::endl;
    }
    virtual void registerData()
    {
      std::cout << "Root Block registerData" << std::endl;
    }
    virtual void preInit()
    {
      std::cout << "Root Block preInit" << std::endl;
    }
    virtual void init()
    {
      std::cout << "Root Block init" << std::endl;
    }
    virtual void postInit()
    {
      std::cout << "Root Block postInit" << std::endl;
    }
};

class Particle : public ChildBlock<Particle>, public BlockContainer<Nested>
{
  protected:
    void initParameters(BlockParameters &parameters)
    {
      std::cout << "Particle " << getName() << " initParameters" << std::endl;
    }
    virtual void registerData()
    {
      std::cout << "Particle " << getName() << " registerData" << std::endl;
    }
    virtual void preInit()
    {
      std::cout << "Particle " << getName() << " preInit" << std::endl;
    }
    virtual void init()
    {
      std::cout << "Particle " << getName() << " init" << std::endl;
    }
    virtual void postInit()
    {
      std::cout << "Particle " << getName() << " postInit" << std::endl;
    }
};

class Nested : public ChildBlock<Nested>
{
  protected:
    void initParameters(BlockParameters &parameters)
    {
      std::cout << "Nested " << getName() << " initParameters" << std::endl;
    }
    virtual void registerData()
    {
      std::cout << "Nested " << getName() << " registerData" << std::endl;
    }
    virtual void preInit()
    {
      std::cout << "Nested " << getName() << " preInit" << std::endl;
    }
    virtual void init()
    {
      std::cout << "Nested " << getName() << " init" << std::endl;
    }
    virtual void postInit()
    {
      std::cout << "Nested " << getName() << " postInit" << std::endl;
    }
};


class Force : public ChildBlock<Force>
{
  public:
    void initParameters(BlockParameters &parameters)
    {
      std::cout << "Force " << getName() << " initParameters" << std::endl;
    }
    virtual void registerData()
    {
      std::cout << "Force " << getName() << " registerData" << std::endl;
    }
    virtual void preInit()
    {
      std::cout << "Force " << getName() << " preInit" << std::endl;
    }
    virtual void init()
    {
      std::cout << "Force " << getName() << " init" << std::endl;
    }
    virtual void postInit()
    {
      std::cout << "Force " << getName() << " postInit" << std::endl;
    }
};


int main()
{
  BlockClasses blocks;

  blocks.registerBlock("NBody").setClass<NBody>();
  blocks.registerBlock("Particle").setClass<Particle>();
  blocks.registerBlock("Nested").setClass<Nested>();
  blocks.registerBlock("Force").setClass<Force>();

  blocks("NBody").addChildren("Particle")("Force");
  blocks("Particle").addChildren("Nested");

  std::ifstream in("example_blocks_lifecycle.setup");
  NBody *mysim;
  try
  {
    Parser P("nbody_simulation", "NBody", blocks);
    registerCMath(P.getFunctionRegistry());
    std::cout << "Before parsing setup file" << std::endl;
    pBlock application = P.parse(in);

    mysim = dynamic_cast<NBody*>(application.get());
    std::cout << "Before calling initAll()" << std::endl;
    mysim->initAll();
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
  return 0;
}







