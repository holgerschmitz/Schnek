/*
 * function_expression.cpp
 *
 * Created on: 12 Sep 2012
 * Author: Holger Schmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2012 Holger Schmitz
 *
 * This file is part of Schnek.
 *
 * Schnek is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Schnek is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Schnek.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "function_expression.hpp"

#include <boost/math/special_functions/gamma.hpp>
#include <boost/math/special_functions/digamma.hpp>
#include <boost/math/special_functions/bessel.hpp>

#include <cmath>
#include <algorithm>


namespace schnek {
  namespace detail {
    double min(double a, double b) { return std::min(a,b); }
    double max(double a, double b) { return std::max(a,b); }
    double minI(int a, int b) { return std::min(a,b); }
    double maxI(int a, int b) { return std::max(a,b); }
    double normal(double x, double sigma, double m)
    {
      double y = (x-m)/sigma;
      return exp(-y*y/2.0);
    }
  }
}

using namespace schnek;

void schnek::registerCMath(FunctionRegistry &freg)
{
  freg.registerFunction("cos", static_cast<double (*)(double)>(cos));
  freg.registerFunction("sin", static_cast<double (*)(double)>(sin));
  freg.registerFunction("tan", static_cast<double (*)(double)>(tan));
  freg.registerFunction("acos", static_cast<double (*)(double)>(acos));
  freg.registerFunction("asin", static_cast<double (*)(double)>(asin));
  freg.registerFunction("atan", static_cast<double (*)(double)>(atan));
  freg.registerFunction("atan2", static_cast<double (*)(double, double)>(atan2));

  freg.registerFunction("cosh", static_cast<double (*)(double)>(cosh));
  freg.registerFunction("sinh", static_cast<double (*)(double)>(sinh));
  freg.registerFunction("tanh", static_cast<double (*)(double)>(tanh));

  freg.registerFunction("exp", static_cast<double (*)(double)>(exp));
  freg.registerFunction("ldexp", static_cast<double (*)(double, int)>(ldexp));
  freg.registerFunction("log", static_cast<double (*)(double)>(log));
  freg.registerFunction("log10", static_cast<double (*)(double)>(log10));

//  The function frexp and modf take pointers as arguments and have side effects
//  This behaviour is not supported
//  freg.registerFunction("frexp", frexp);
//  freg.registerFunction("modf", modf);

  freg.registerFunction("pow", static_cast<double (*)(double, double)>(pow));
  freg.registerFunction("sqrt", static_cast<double (*)(double)>(sqrt));

  freg.registerFunction("ceil", static_cast<double (*)(double)>(ceil));
  freg.registerFunction("fabs", static_cast<double (*)(double)>(fabs));
  freg.registerFunction("floor", static_cast<double (*)(double)>(floor));
  freg.registerFunction("fmod", static_cast<double (*)(double, double)>(fmod));
}

void schnek::registerUtilityFunctions(FunctionRegistry &freg)
{
  freg.registerFunction("min", schnek::detail::min);
  freg.registerFunction("max", schnek::detail::max);
  freg.registerFunction("minI", schnek::detail::minI);
  freg.registerFunction("maxI", schnek::detail::maxI);
}


void schnek::registerSpecialFunctions(FunctionRegistry &freg)
{
  freg.registerFunction("gamma", static_cast<double (*)(double)>(boost::math::tgamma));
  freg.registerFunction("lgamma", static_cast<double (*)(double)>(boost::math::lgamma));
  freg.registerFunction("digamma", static_cast<double (*)(double)>(boost::math::digamma));

  freg.registerFunction("besselj", static_cast<double (*)(int, double)>(boost::math::cyl_bessel_j));
  freg.registerFunction("bessely", static_cast<double (*)(int, double)>(boost::math::cyl_neumann));

  freg.registerFunction("normal", schnek::detail::normal);
}


void schnek::registerAllFunctions(FunctionRegistry &freg)
{
  schnek::registerCMath(freg);
  schnek::registerUtilityFunctions(freg);
  schnek::registerSpecialFunctions(freg);
}
