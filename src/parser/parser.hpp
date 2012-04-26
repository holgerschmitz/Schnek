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

namespace schnek {

class VariableStorage;
class FunctionRegistry;

class Parser {
  private:
    VariableStorage *variables;
    FunctionRegistry *funcReg;
  public:
    Parser(VariableStorage &variables_, FunctionRegistry &funcReg_)
      : variables(&variables_), funcReg(&funcReg_)
    {}

    void parse(std::istream &input, std::string filename);
};

} // namespace

#endif
