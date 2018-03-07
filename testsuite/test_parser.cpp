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
#include <string>
#include <cmath>

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

using namespace schnek;

std::string parser_input =
"// general test of variables assignment and evaluation\n"
"float var = 2/1.9;\n"
"string hello = \"Hello\";\n"
"string greet = hello + \" world!\";\n"
"int n = 7^3;\n"
"int a = n/6;\n"
"int m = n + a;\n"
"int k = - n + a;\n"
"string assembled = a + \".\" + n;\n"
"greet = greet + \" Again!\";\n"
"\n"
"float e = exp(a);\n"
"float s = sin(var);\n"
"\n"
"\n"
"Collection globals {\n"
"  float a = exp(s);\n"
"  float b = cos(e);\n"
"  string result = \"Result is \"+a;\n"
"  Values physics {\n"
"    float qe = 1.602e-19;\n"
"  }\n"
"  Constants maths {\n"
"    float pi = 3.14159;\n"
"  }\n"
"}\n"
"\n"
"float other = globals:a;\n"
"\n"
"nsteps = n;\n"
"output = hello;\n"
"\n"
"//dx = 1.0;\n"
"//dy = 1.0;\n"
"\n"
"// from here on we test dependencies\n"
"\n"
"float depA = x;\n"
"float depB = y+x;\n"
"float depC = depA * depB;\n"
"float depD = depC / (y+1);\n"
"float depE = depA * depD;\n"
"\n"
"dx = depB*depD;\n"
"dy = depC;\n"
"\n"
"// initialising without using independent variables\n"
"dz = 1.0;\n";

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

struct ParserTest
{
  VariableStorage vars;
  FunctionRegistry freg;
  BlockClasses blocks;
  pBlock application;

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

  ParserTest() : vars("test_parser", "app") {
    registerCMath(freg);

    blocks.registerBlock("app");
    blocks("app").setClass<SimulationBlock>();
    blocks("app").addChildren("Collection");

    blocks("Collection").setClass<Block>();
    blocks("Values").setClass<Block>();
    blocks("Constants").setClass<Block>();

    blocks("Collection").addChildren("Values")("Constants");

    Parser P(vars, freg, blocks);

    std::istringstream in(parser_input);
    try
    {
      application = P.parse(in);
    }
    catch (ParserError &e)
    {
      std::cerr << "Parse error, " << e.atomToken.getFilename() << "(" << e.atomToken.getLine() << "): "<< e.message << "\n";
      throw -1;
    }
  }

  void checkParsedVars(double x_) {
    x = x_;
    application->evaluateParameters();

    BOOST_CHECK_EQUAL(output, "Hello");
    BOOST_CHECK_EQUAL(NSteps, 7*7*7);
    BOOST_CHECK_EQUAL(dz, 1.0);
  }

  void checkDependency(VariableStorage &vars) {
    pDependencyMap depMap(new DependencyMap(vars.getRootBlock()));
    DependencyUpdater updater(depMap);

    updater.addIndependent(xVar);
    updater.addIndependent(yVar);
    updater.addDependent(dxVar);
    updater.addDependent(dyVar);

    for (x=1.0; x<=2.0; x+= 0.01)
      for (y=1.0; y<=2.0; y+= 0.01)
      {
        double x_save = x;
        double y_save = y;
        updater.update();
        BOOST_CHECK_CLOSE(x_save, x, 1e-10);
        BOOST_CHECK_CLOSE(y_save, y, 1e-10);

        BOOST_CHECK_CLOSE(dx, (y+x) * x * (y+x) / (y+1), 1e-10);
        BOOST_CHECK_CLOSE(dy, x * (y+x), 1e-10);
      }
  }

  void checkIndependency(VariableStorage &vars) {
    pDependencyMap depMap(new DependencyMap(vars.getRootBlock()));
    DependencyUpdater updater(depMap);

    updater.addIndependent(xVar);
    updater.addIndependent(yVar);
    updater.addDependent(dzVar);

    double dx_save = dx;
    double dy_save = dy;

    for (x=0.0; x<=1.0; x+= 0.125)
      for (y=0.0; y<=1.0; y+= 0.125)
      {
        double x_save = x;
        double y_save = y;
        updater.update();
        BOOST_CHECK_CLOSE(x_save, x, 1e-10);
        BOOST_CHECK_CLOSE(y_save, y, 1e-10);
        BOOST_CHECK_CLOSE(dx_save, dx, 1e-10);
        BOOST_CHECK_CLOSE(dy_save, dy, 1e-10);

        BOOST_CHECK_CLOSE(dz, 1.0, 1e-10);
      }
  }
};

BOOST_AUTO_TEST_SUITE( parser )

BOOST_FIXTURE_TEST_CASE( parser_values, ParserTest )
{
  checkParsedVars(1.0);
  checkParsedVars(2.0);
}

BOOST_FIXTURE_TEST_CASE( parser_dependency, ParserTest )
{
  checkDependency(vars);
}

BOOST_FIXTURE_TEST_CASE( parser_independency, ParserTest )
{
  checkIndependency(vars);
}


BOOST_AUTO_TEST_SUITE_END()
