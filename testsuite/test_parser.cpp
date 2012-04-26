/*
 * test_parser.cpp
 *
 *  Created on: 21 Jun 2011
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
 */

#include <parser/parser.hpp>
#include <parser/parsertoken.hpp>
#include <variables/variables.hpp>
#include <variables/function_expression.hpp>
#include <iostream>
#include <fstream>
#include <cmath>

#include <boost/foreach.hpp>

using namespace schnek;


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

  freg.registerFunction("exp", exp);
  freg.registerFunction("sin", sin);
  freg.registerFunction("cos", cos);

  Parser P(vars, freg);

  std::ifstream in("test_parser_sample.txt");
  if (!in) {
    std::cerr << "Could not open file\n";
  }
  try
  {
    P.parse(in, "test_parser_sample.txt");
  }
  catch (ParserError &e)
  {
    std::cerr << "Parse error, " << e.atomToken.getFilename() << "(" << e.atomToken.getLine() << "): "<< e.message << "\n";
  }

  writeBlockVars(vars.getRootBlock());
}

