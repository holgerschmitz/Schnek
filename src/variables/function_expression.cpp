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
#include <cmath>

using namespace schnek;

void schnek::registerCMath(FunctionRegistry &freg)
{
  freg.registerFunction("cos", cos);
  freg.registerFunction("sin", sin);
  freg.registerFunction("tan", tan);
  freg.registerFunction("acos", acos);
  freg.registerFunction("asin", asin);
  freg.registerFunction("atan", atan);
  freg.registerFunction("atan2", atan2);

  freg.registerFunction("cosh", cosh);
  freg.registerFunction("sinh", sinh);
  freg.registerFunction("tanh", tanh);

  freg.registerFunction("exp", exp);
  freg.registerFunction("ldexp", ldexp);
  freg.registerFunction("log", log);
  freg.registerFunction("log10", log10);

//  The function frexp and modf take pointers as arguments and have side effects
//  This behaviour is not supported
//  freg.registerFunction("frexp", frexp);
//  freg.registerFunction("modf", modf);

  freg.registerFunction("pow", pow);
  freg.registerFunction("sqrt", sqrt);

  freg.registerFunction("ceil", ceil);
  freg.registerFunction("fabs", fabs);
  freg.registerFunction("floor", floor);
  freg.registerFunction("fmod", fmod);
}
