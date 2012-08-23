/*
 * test_parser.cpp
 *
 *  Created on: 21 Jun 2011
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
 */

#include <parser/parser.hpp>
#include <parser/parsertoken.hpp>
#include <variables/blockclasses.hpp>
#include <variables/blockparameters.hpp>
#include <variables/block.hpp>
#include <variables/variables.hpp>
#include <variables/function_expression.hpp>
#include <iostream>
#include <fstream>
#include <cmath>

#include <boost/foreach.hpp>

using namespace schnek;


int NSteps;
double dx;
std::string output;
double x;


class SimulationBlock : public Block
{
  protected:
    void initParameters(BlockParameters &blockPars)
    {
      blockPars.addParameter("x", &x, BlockParameters::readonly);
      blockPars.addParameter("nsteps", &NSteps);
      blockPars.addParameter("dx", &dx);
      blockPars.addParameter("output", &output);
    }
};

void writeBlockVars(pBlockVariables block)
{
  std::cout << "Block: " << block->getBlockName() << "(" << block->getClassName() << ")" << std::endl;
  typedef std::pair<std::string, pVariable> VMapType;
  BOOST_FOREACH(VMapType var, block->getVariables())
  {
    std::cout << "  " << var.first << " = " << var.second->getValue() << std::endl;
  }
  BOOST_FOREACH(pBlockVariables child, block->getChildren())
  {
    std::cout << "==================\n";
    writeBlockVars(child);
    std::cout << "========END=======\n";
  }
}

int main()
{
  VariableStorage vars("test_parser", "app");
  FunctionRegistry freg;
  BlockClasses blocks;

  freg.registerFunction("exp", exp);
  freg.registerFunction("sin", sin);
  freg.registerFunction("cos", cos);

  blocks.addBlockClass("app");
  blocks("app").addChildren("Collection");
  blocks("app").setBlockClass<SimulationBlock>();

  blocks.addBlockClass("Collection").addChildren("Values")("Constants");

  Parser P(vars, freg, blocks);
  pBlock application;

  std::ifstream in("test_parser_sample.txt");
  if (!in) {
    std::cerr << "Could not open file\n";
  }
  try
  {
    application = P.parse(in, "test_parser_sample.txt");
  }
  catch (ParserError &e)
  {
    std::cerr << "Parse error, " << e.atomToken.getFilename() << "(" << e.atomToken.getLine() << "): "<< e.message << "\n";
    throw -1;
  }

  x = 1;
  application->evaluateParameters();
  writeBlockVars(vars.getRootBlock());

  x = 2;
  application->evaluateParameters();
  writeBlockVars(vars.getRootBlock());

  std::cout << "\n\n";
  std::cout << "Application variables:\n";
  std::cout << "NSteps = " << NSteps << std::endl;
  std::cout << "dx = " << dx << std::endl;
  std::cout << "output = " << output << std::endl;
}

