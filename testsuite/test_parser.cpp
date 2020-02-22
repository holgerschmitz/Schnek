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

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/progress.hpp>

#include <boost/math/special_functions/gamma.hpp>
#include <boost/math/special_functions/digamma.hpp>
#include <boost/math/special_functions/bessel.hpp>

using namespace schnek;

std::string parser_input_basic =
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

std::string parser_input_precedence =
    "int a = xi;\n"
    "int b = yi;\n"
    "int c = 2;\n"
    "test_int1 = 2*a - b;\n"
    "test_int2 = 2*a - b - 3;\n"
    "test_int3 = 2*a - b - c;\n"
    "test_int4 = 2* a-3 *b + c;\n"
    "test_int5 = 2*(a-3)*b + c;\n"
    ;

std::string parser_input_utility =
    "test1 = min(x,y);\n"
    "test2 = max(x,y);\n"
    "test_int1 = minI(xi,yi);\n"
    "test_int2 = maxI(xi,yi);\n";

std::string parser_input_cmath =
    "test1 = sin(x);\n"
    "test2 = cos(x);\n"
    "test3 = exp(-x*x);\n"
    "test4 = log(1+x*x);\n"
    "test5 = sqrt(1+x*x);\n";

std::string parser_input_special_functions_gamma =
    "test1 = gamma(x);\n"
    "test2 = lgamma(x);\n"
    "test3 = digamma(x);\n";

std::string parser_input_special_functions_bessel =
    "test1 = besselj(1,x);\n"
    "test2 = besselj(2,x);\n"
    "test3 = bessely(1,x);\n"
    "test4 = bessely(2,x);\n";

std::string parser_input_special_functions_normal =
    "test1 = normal(x,y,1.0);\n"
    "test2 = normal(x,y,-1.0);\n"
    "test3 = normal(x,y,2.0);\n"
    "test4 = normal(x,y,-2.0);\n";

std::string parser_input_count_evaluation =
    "test4 = eval4();\n"
    "test2 = eval2(x);\n"
    "test1 = eval1();\n"
    "test3 = eval3(x,y);\n";

int NSteps;

double dx;
double dy;
double dz;

double x;
double y;

int xi;
int yi;

double test1;
double test2;
double test3;
double test4;
double test5;

int test_int1;
int test_int2;
int test_int3;
int test_int4;
int test_int5;

std::string output;

pParameter xVar;
pParameter yVar;
pParameter xiVar;
pParameter yiVar;
pParameter dxVar;
pParameter dyVar;
pParameter dzVar;

pParameter test1Var;
pParameter test2Var;
pParameter test3Var;
pParameter test4Var;
pParameter test5Var;

pParameter test_int1Var;
pParameter test_int2Var;
pParameter test_int3Var;
pParameter test_int4Var;
pParameter test_int5Var;

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

        xiVar = blockPars.addParameter("xi", &xi, BlockParameters::readonly);
        yiVar = blockPars.addParameter("yi", &yi, BlockParameters::readonly);

        test1Var = blockPars.addParameter("test1", &test1, 0.0);
        test2Var = blockPars.addParameter("test2", &test2, 0.0);
        test3Var = blockPars.addParameter("test3", &test3, 0.0);
        test4Var = blockPars.addParameter("test4", &test4, 0.0);
        test5Var = blockPars.addParameter("test5", &test5, 0.0);

        test_int1Var = blockPars.addParameter("test_int1", &test_int1, 0);
        test_int2Var = blockPars.addParameter("test_int2", &test_int2, 0);
        test_int3Var = blockPars.addParameter("test_int3", &test_int3, 0);
        test_int4Var = blockPars.addParameter("test_int4", &test_int4, 0);
        test_int5Var = blockPars.addParameter("test_int5", &test_int5, 0);

        dxVar = blockPars.addParameter("dx", &dx);
        dyVar = blockPars.addParameter("dy", &dy);
        dzVar = blockPars.addParameter("dz", &dz);

        blockPars.addParameter("nsteps", &NSteps);
        blockPars.addParameter("output", &output);
      }
  };

  virtual void registerFunctions()
  {

  }

  ParserTest() : vars("test_parser", "app") {}

  void init(std::string parser_input)
  {
    this->registerFunctions();

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

    catch (schnek::ParserError &e)
    {
      BOOST_FAIL("Parse error in " + e.getFilename() + " at line " + boost::lexical_cast<std::string>(e.getLine()) + ": " + e.message);
    }
    catch (schnek::VariableNotInitialisedException &e)
    {
      BOOST_FAIL("Variable was not initialised: " + e.getVarName());
    }
    catch (schnek::EvaluationException &e)
    {
      BOOST_FAIL("Error in evaluation: " + e.getMessage());
    }
    catch (schnek::VariableNotFoundException &e)
    {
      BOOST_FAIL("Variable was not found: " + e.getMessage());
    }
    catch (SchnekException &e)
    {
      BOOST_FAIL("An error occured");
    }
    catch (std::string &err)
    {
      BOOST_FAIL("FATAL ERROR: " + err);
    }
  }

  void checkParsedVars(double x_) {
    x = x_;
    application->evaluateParameters();

    BOOST_CHECK_EQUAL(output, "Hello");
    BOOST_CHECK_EQUAL(NSteps, 7*7*7);
    BOOST_CHECK_EQUAL(dz, 1.0);
  }
};

BOOST_AUTO_TEST_SUITE( parser )

BOOST_FIXTURE_TEST_CASE( parser_values, ParserTest )
{
  registerCMath(freg);
  init(parser_input_basic);

  checkParsedVars(1.0);
  checkParsedVars(2.0);
}

BOOST_FIXTURE_TEST_CASE( parser_dependency, ParserTest)
{
  registerCMath(freg);
  init(parser_input_basic);

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

BOOST_FIXTURE_TEST_CASE( parser_independency, ParserTest )
{
  registerCMath(freg);
  init(parser_input_basic);

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

BOOST_FIXTURE_TEST_CASE( parser_precedence, ParserTest )
{
  registerCMath(freg);
  init(parser_input_precedence);

  pDependencyMap depMap(new DependencyMap(vars.getRootBlock()));
  DependencyUpdater updater(depMap);

  updater.addIndependent(xiVar);
  updater.addIndependent(yiVar);
  updater.addDependent(test_int1Var);
  updater.addDependent(test_int2Var);
  updater.addDependent(test_int3Var);
  updater.addDependent(test_int4Var);
  updater.addDependent(test_int5Var);

  double dx_save = dx;
  double dy_save = dy;

  for (xi=-5; xi<=5; ++xi)
    for (yi=-5; yi<=5; ++yi)
    {
      int a = xi;
      int b = yi;
      int c = 2;
      updater.update();
      BOOST_CHECK_EQUAL(test_int1, 2*a - b);
      BOOST_CHECK_EQUAL(test_int2, 2*a - b - 3);
      BOOST_CHECK_EQUAL(test_int3, 2*a - b - c);
      BOOST_CHECK_EQUAL(test_int4, 2* a-3 *b + c);
      BOOST_CHECK_EQUAL(test_int5, 2*(a-3)*b + c);
    }
}


BOOST_FIXTURE_TEST_CASE( parser_utility, ParserTest )
{
  registerUtilityFunctions(freg);
  init(parser_input_utility);

  const int N = 100000;
  boost::progress_display show_progress(2*N);

  {
    boost::random::mt19937 rGen;
    boost::random::uniform_real_distribution<> dist(-1e6, 1e6);

    pDependencyMap depMap(new DependencyMap(vars.getRootBlock()));
    DependencyUpdater updater(depMap);

    updater.addIndependent(xVar);
    updater.addIndependent(yVar);
    updater.addDependent(test1Var);
    updater.addDependent(test2Var);


    for (int i=0; i<N; ++i)
    {
      x = dist(rGen);
      y = dist(rGen);
      updater.update();
      BOOST_CHECK_EQUAL(test1, std::min(x,y));
      BOOST_CHECK_EQUAL(test2, std::max(x,y));

      ++show_progress;
    }
  }
  {
    boost::random::mt19937 rGen;
    boost::random::uniform_int_distribution<> dist(100000, 100000);

    pDependencyMap depMap(new DependencyMap(vars.getRootBlock()));
    DependencyUpdater updater(depMap);

    updater.addIndependent(xiVar);
    updater.addIndependent(yiVar);
    updater.addDependent(test_int1Var);
    updater.addDependent(test_int2Var);


    for (int i=0; i<N; ++i)
    {
      xi = dist(rGen);
      yi = dist(rGen);
      updater.update();
      BOOST_CHECK_EQUAL(test_int1, std::min(xi,yi));
      BOOST_CHECK_EQUAL(test_int2, std::max(xi,yi));

      ++show_progress;
    }
  }
}


BOOST_FIXTURE_TEST_CASE( parser_cmath, ParserTest )
{
  registerCMath(freg);
  init(parser_input_cmath);

  const int N = 100000;
  boost::progress_display show_progress(N);

  boost::random::mt19937 rGen;
  boost::random::uniform_real_distribution<> dist(-10, 10);

  pDependencyMap depMap(new DependencyMap(vars.getRootBlock()));
  DependencyUpdater updater(depMap);

  updater.addIndependent(xVar);
  updater.addIndependent(yVar);
  updater.addDependent(test1Var);
  updater.addDependent(test2Var);
  updater.addDependent(test3Var);
  updater.addDependent(test4Var);
  updater.addDependent(test5Var);


  for (int i=0; i<N; ++i)
  {
    x = dist(rGen);
    y = dist(rGen);
    updater.update();
    BOOST_CHECK_CLOSE(test1, sin(x), 1e-8);
    BOOST_CHECK_CLOSE(test2, cos(x), 1e-8);
    BOOST_CHECK_CLOSE(test3, exp(-x*x), 1e-8);
    BOOST_CHECK_CLOSE(test4, log(1+x*x), 1e-8);
    BOOST_CHECK_CLOSE(test5, sqrt(1+x*x), 1e-8);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( parser_special_functions_gamma, ParserTest )
{
  x=1.0;
  y=1.0;
  registerSpecialFunctions(freg);
  init(parser_input_special_functions_gamma);

  const int N = 100000;
  boost::progress_display show_progress(N);

  boost::random::mt19937 rGen;
  boost::random::uniform_real_distribution<> dist(1.0, 10.0);

  pDependencyMap depMap(new DependencyMap(vars.getRootBlock()));
  DependencyUpdater updater(depMap);

  updater.addIndependent(xVar);
  updater.addIndependent(yVar);
  updater.addDependent(test1Var);
  updater.addDependent(test2Var);
  updater.addDependent(test3Var);

  for (int i=0; i<N; ++i)
  {
    x = dist(rGen);
    y = dist(rGen);

    updater.update();
    BOOST_CHECK_CLOSE(test1, boost::math::tgamma(x), 1e-8);
    BOOST_CHECK_CLOSE(test2, boost::math::lgamma(x), 1e-8);
    BOOST_CHECK_CLOSE(test3, boost::math::digamma(x), 1e-8);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( parser_special_functions_bessel, ParserTest )
{
  x=1.0;
  y=1.0;
  registerSpecialFunctions(freg);
  init(parser_input_special_functions_bessel);

  const int N = 100000;
  boost::progress_display show_progress(N);

  boost::random::mt19937 rGen;
  boost::random::uniform_real_distribution<> dist(1.0, 10.0);

  pDependencyMap depMap(new DependencyMap(vars.getRootBlock()));
  DependencyUpdater updater(depMap);

  updater.addIndependent(xVar);
  updater.addIndependent(yVar);
  updater.addDependent(test1Var);
  updater.addDependent(test2Var);
  updater.addDependent(test3Var);
  updater.addDependent(test4Var);

  for (int i=0; i<N; ++i)
  {
    x = dist(rGen);
    y = dist(rGen);

    updater.update();
    BOOST_CHECK_CLOSE(test1, boost::math::cyl_bessel_j(1,x), 1e-8);
    BOOST_CHECK_CLOSE(test2, boost::math::cyl_bessel_j(2,x), 1e-8);
    BOOST_CHECK_CLOSE(test3, boost::math::cyl_neumann(1,x), 1e-8);
    BOOST_CHECK_CLOSE(test4, boost::math::cyl_neumann(2,x), 1e-8);

    ++show_progress;
  }
}

double normal(double x, double sigma, double m)
{
  double y = (x-m)/sigma;
  return exp(-y*y/2.0);
}

BOOST_FIXTURE_TEST_CASE( parser_special_functions_normal, ParserTest )
{
  x=1.0;
  y=1.0;
  registerSpecialFunctions(freg);
  init(parser_input_special_functions_normal);

  const int N = 100000;
  boost::progress_display show_progress(N);

  boost::random::mt19937 rGen;
  boost::random::uniform_real_distribution<> dist(1.0, 10.0);

  pDependencyMap depMap(new DependencyMap(vars.getRootBlock()));
  DependencyUpdater updater(depMap);

  updater.addIndependent(xVar);
  updater.addIndependent(yVar);
  updater.addDependent(test1Var);
  updater.addDependent(test2Var);
  updater.addDependent(test3Var);
  updater.addDependent(test4Var);

  for (int i=0; i<N; ++i)
  {
    x = dist(rGen);
    y = dist(rGen);

    updater.update();
    BOOST_CHECK_CLOSE(test1, normal(x,y,1.0), 1e-8);
    BOOST_CHECK_CLOSE(test2, normal(x,y,-1.0), 1e-8);
    BOOST_CHECK_CLOSE(test3, normal(x,y,2.0), 1e-8);
    BOOST_CHECK_CLOSE(test4, normal(x,y,-2.0), 1e-8);

    ++show_progress;
  }
}

int evaluation_counter1;
int evaluation_counter2;
int evaluation_counter3;
int evaluation_counter4;
double evaluation_counter4_return_value;

double count_evaluation1() {
  ++evaluation_counter1;
  return 2.0;
}

double count_evaluation2(double x) {
  ++evaluation_counter2;
  return 2.0*x;
}

double count_evaluation3(double x, double y) {
  ++evaluation_counter3;
  return 2.0*x + 3.0*y;
}

double count_evaluation4() {
  ++evaluation_counter4;
  return evaluation_counter4_return_value;
}

BOOST_FIXTURE_TEST_CASE( parser_count_evaluations, ParserTest )
{
  evaluation_counter4_return_value = 1.0;
  const int N = 100;

  freg.registerFunction("eval1", count_evaluation1);
  freg.registerFunction("eval2", count_evaluation2);
  freg.registerFunction("eval3", count_evaluation3);
  freg.registerFunction("eval4", count_evaluation4, true);

  init(parser_input_count_evaluation);

  boost::progress_display show_progress(N*N);

  boost::random::mt19937 rGen;
  boost::random::uniform_real_distribution<> dist(1.0, 10.0);

  pDependencyMap depMap(new DependencyMap(vars.getRootBlock()));
  DependencyUpdater updater(depMap);

  updater.addIndependent(xVar);
  updater.addIndependent(yVar);
  updater.addDependent(test1Var);
  updater.addDependent(test2Var);
  updater.addDependent(test3Var);
  updater.addDependent(test4Var);

  int evaluation_counter1_ref = 0;
  int evaluation_counter2_ref = 0;
  int evaluation_counter3_ref = 0;
  int evaluation_counter4_ref = 0;

  evaluation_counter1 = 0;
  evaluation_counter2 = 0;
  evaluation_counter3 = 0;
  evaluation_counter4 = 0;

  for (int i=0; i<N; ++i)
  {
    x = 2.0*i;
    for (int j=0; j<N; ++j)
    {
      y = 3.0*j;
      evaluation_counter4_return_value = N*x + y;

      ++evaluation_counter2_ref;
      ++evaluation_counter3_ref;
      ++evaluation_counter4_ref;

      updater.update();
      BOOST_CHECK_CLOSE(test1, 2.0, 1e-8);
      BOOST_CHECK_CLOSE(test2, 2.0*x, 1e-8);
      BOOST_CHECK_CLOSE(test3, 2.0*x + 3.0*y, 1e-8);
      BOOST_CHECK_CLOSE(test4, evaluation_counter4_return_value, 1e-8);

      BOOST_CHECK_EQUAL(evaluation_counter1, evaluation_counter1_ref);
      BOOST_CHECK_EQUAL(evaluation_counter2, evaluation_counter2_ref);
      BOOST_CHECK_EQUAL(evaluation_counter3, evaluation_counter3_ref);
      BOOST_CHECK_EQUAL(evaluation_counter4, evaluation_counter4_ref);

      ++show_progress;
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
