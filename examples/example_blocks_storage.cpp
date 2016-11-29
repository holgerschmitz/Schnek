/*
 * example_blocks_storage.cpp
 *
 *  Created on: 28 Nov 2016
 *      Author: Holger Schmitz
 *       Email: holger@notjustphysics.com
 */


#include "../src/grid/array.hpp"
#include "../src/grid/arrayexpression.hpp"
#include "../src/parser.hpp"
#include "../src/variables/block.hpp"
#include "../src/variables/blockcontainer.hpp"

#include <iostream>
#include <fstream>

using namespace schnek;

class Particle;
class Force;
class WriteVector;

class NBody : public Block, public BlockContainer<Particle>, public BlockContainer<Force>, public BlockContainer<WriteVector>
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

class WriteVector : public ChildBlock<WriteVector> {
  private:
    Array<double,3> *pVector;
    std::string value;
  protected:
    void initParameters(BlockParameters &parameters)
    {
      parameters.addParameter("value", &value);
    }

    void init() {
      retrieveData(value, pVector);
    }

  public:
    void display() {
      std::cout << (*pVector)[0] << " " << (*pVector)[1] << " " << (*pVector)[2] << std::endl;
    }
};

typedef boost::shared_ptr<WriteVector> pWriteVector;

class Particle : public ChildBlock<Particle>
{
  private:
    Array<double,3> pos;
    Array<double,3> velocity;
    double mass;

  protected:
    void registerData() {
      addData("pos", pos);
      addData("velocity", velocity);
    }

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

    const Array<double,3>& getPos() { return pos; }

    void display()
    {
      std::cout << "Mass: " << mass << std::endl;
      std::cout << "Position: (" << pos[0] << ", " << pos[1] << ", " << pos[2] << ")" << std::endl;
      std::cout << "Velocity: (" << velocity[0] << ", " << velocity[1] << ", " << velocity[2] << ")" << std::endl;
    }
};

typedef boost::shared_ptr<Particle> pParticle;

class Force : public ChildBlock<Force>
{
  public:
    virtual ~Force() {};
    virtual Array<double,3> getForce(const Array<double,3> &pos) = 0;
};

typedef boost::shared_ptr<Force> pForce;

class LinearForce : public Force
{
  private:
    Array<double,3> center;
    double k;
  protected:
    void initParameters(BlockParameters &parameters)
    {
      parameters.addArrayParameter("center", center);
      parameters.addParameter("k", &k);
    }

  public:
    Array<double,3> getForce(const Array<double,3> &pos)
    {
      return k*(center-pos);
    }
};

class NonLinearForce : public Force
{
  private:
    Array<double,3> center;
    double k;
    double d;
  protected:
    void initParameters(BlockParameters &parameters)
    {
      parameters.addArrayParameter("center", center);
      parameters.addParameter("k", &k);
      parameters.addParameter("d", &d);
    }

  public:
    Array<double,3> getForce(const Array<double,3> &pos)
    {
      Array<double,3> delta = center-pos;

      return k*delta / (delta.sqr() + d*d);
    }
};


void NBody::runSimulation()
{
  for (double time=0.0; time<=totalTime; time+=dt)
  {
    BOOST_FOREACH(pParticle particle, BlockContainer<Particle>::childBlocks())
    {
      Array<double,3> force(0.0,0.0,0.0);

      BOOST_FOREACH(pForce F, schnek::BlockContainer<Force>::childBlocks()) {
        force += F->getForce(particle->getPos());
      }

      particle->advance(force, dt);

      BOOST_FOREACH(pWriteVector write, BlockContainer<WriteVector>::childBlocks()) {
        write->display();
      }
    }
  }
}


int main()
{
  BlockClasses blocks;

  blocks.registerBlock("NBody").setClass<NBody>();
  blocks.registerBlock("Particle").setClass<Particle>();
  blocks.registerBlock("LinearForce").setClass<LinearForce>();
  blocks.registerBlock("NonLinearForce").setClass<NonLinearForce>();
  blocks.registerBlock("WriteVector").setClass<WriteVector>();

  blocks("NBody").addChildren("Particle")("LinearForce")("NonLinearForce")("WriteVector");

  std::ifstream in("example_blocks_storage.setup");
  NBody *mysim;
  try
  {
    Parser P("nbody_simulation", "NBody", blocks);
    registerCMath(P.getFunctionRegistry());
    pBlock application = P.parse(in);

    mysim = dynamic_cast<NBody*>(application.get());
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
  mysim->runSimulation();
  return 0;
}







