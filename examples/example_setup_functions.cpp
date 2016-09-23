/*
 * example_setup_first.cpp
 *
 *  Created on: 8 Jul 2014
 *      Author: Holger Schmitz
 *       Email: holger@notjustphysics.com
 */


#include "../src/parser.hpp"
#include "../src/variables/block.hpp"

#include <iostream>
#include <fstream>
#include <cmath>

using namespace schnek;

class SimulationBlock : public Block
{
  private:
    double value;
    int n;
  protected:
    void initParameters(BlockParameters &parameters)
    {
      parameters.addParameter("value", &value);
      parameters.addParameter("n", &n);
    }

  public:
    void writeValues()
    {
      std::cout << "value = " << value << std::endl;
      std::cout << "n = " << n << std::endl;
    }
};

double normal(double x)
{
  return exp(-x*x)/sqrt(2.0*M_PI);
}

int length(std::string s)
{
  return s.length();
}

double series(double x, int n)
{
  double result = 0;
  double factor = 2/M_PI;
  for (int i=1; i<=n; ++i)
  {
    result += factor/i * sin(i*x);
    factor = -factor;
  }
  return result;
}

int main()
{
  BlockClasses blocks;

  blocks.registerBlock("sim").setClass<SimulationBlock>();

//  blocks.addBlock("sim");
//  blocks("sim").setClass<SimulationBlock>();

  std::ifstream in("example_setup_functions.setup");

  Parser P("my_simulation", "sim", blocks);

  registerCMath(P.getFunctionRegistry());

  P.getFunctionRegistry().registerFunction("normal",normal);
  P.getFunctionRegistry().registerFunction("length",length);
  P.getFunctionRegistry().registerFunction("series",series);

  try
  {
    pBlock application = P.parse(in);

    SimulationBlock &mysim = dynamic_cast<SimulationBlock&>(*application);
    mysim.evaluateParameters();
    mysim.writeValues();
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




