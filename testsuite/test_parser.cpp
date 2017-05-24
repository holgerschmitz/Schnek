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
#include <variables/dependencies.hpp>
#include <iostream>
#include <fstream>
#include <cmath>

#include <boost/foreach.hpp>

using namespace schnek;


int NSteps;

double dx;
double dy;
double dz;

double x;
double y;

std::string output;

pParameter xVar;
pParameter yVar;
pParameter dxVar;
pParameter dyVar;
pParameter dzVar;

class SimulationBlock : public Block
{
  protected:
    void initParameters(BlockParameters &blockPars)
    {
      xVar = blockPars.addParameter("x", &x, BlockParameters::readonly);
      yVar = blockPars.addParameter("y", &y, BlockParameters::readonly);

      dxVar = blockPars.addParameter("dx", &dx);
      dyVar = blockPars.addParameter("dy", &dy);
      dzVar = blockPars.addParameter("dz", &dz);

      blockPars.addParameter("nsteps", &NSteps);
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

  blocks.registerBlock("app");
  blocks("app").addChildren("Collection");
  blocks("app").setClass<SimulationBlock>();

  blocks.registerBlock("Collection").addChildren("Values")("Constants");

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
  std::cout << "dy = " << dy << std::endl;
  std::cout << "output = " << output << std::endl;


  std::cout << "Automatic update:\n";
  std::cout << "id(x) = " << xVar->getVariable()->getId() << std::endl;
  std::cout << "id(y) = " << yVar->getVariable()->getId() << std::endl;
  std::cout << "id(dx) = " << dxVar->getVariable()->getId() << std::endl;
  std::cout << "id(dy) = " << dyVar->getVariable()->getId() << std::endl;

  pDependencyMap depMap(new DependencyMap(vars.getRootBlock()));
  DependencyUpdater updater(depMap);

  updater.addIndependent(xVar);
  updater.addIndependent(yVar);
  updater.addDependent(dxVar);
  updater.addDependent(dyVar);

  for (x=0.0; x<=1.0; x+= 0.125)
    for (y=0.0; y<=1.0; y+= 0.125)
    {
      updater.update();
      std::cout << x << " " << y << " " << dx << " " << dy << std::endl;
    }

  std::cout << "Automatic update for constant expressions:\n";
  DependencyUpdater updater_const(depMap);

  updater_const.addIndependent(xVar);
  updater_const.addIndependent(yVar);
  updater_const.addDependent(dzVar);

  for (x=0.0; x<=1.0; x+= 0.125)
    for (y=0.0; y<=1.0; y+= 0.125)
    {
      updater_const.update();
      std::cout << x << " " << y << " " << dz << std::endl;
    }

}

