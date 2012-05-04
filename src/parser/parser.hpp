/*
 * parser.hpp
 *
 *  Created on: 21 Jun 2011
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
 */

#ifndef SCHNEK_PARSER_HPP_
#define SCHNEK_PARSER_HPP_

#include <iostream>
#include "blockclasses.hpp"
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

#endif
