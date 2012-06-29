/*
 * parser.hpp
 *
 * Created on: 21 Jun 2011
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

#ifndef SCHNEK_PARSER_HPP_
#define SCHNEK_PARSER_HPP_

#include <iostream>
#include "../variables/blockclasses.hpp"
#include "parsersettings.hpp"
#include "../variables/variables.hpp"
#include "../variables/function_expression.hpp"

namespace schnek {

class Parser {
  private:
    VariableStorage variables;
    FunctionRegistry funcReg;
    BlockClasses blockClasses;
  public:
    Parser(const VariableStorage &variables_, const FunctionRegistry &funcReg_)
      : variables(variables_), funcReg(funcReg_), blockClasses(false)
    {}

    Parser(const VariableStorage &variables_, const FunctionRegistry &funcReg_, const BlockClasses &blockClasses_)
      : variables(variables_), funcReg(funcReg_), blockClasses(blockClasses_)
    {}

    Parser(const ParserSettings &settings)
      : variables(settings.variables), funcReg(settings.funcReg), blockClasses(settings.blockClasses)
    {}


    pBlock parse(std::istream &input, std::string filename);
};

} // namespace

#endif // SCHNEK_PARSER_HPP_
