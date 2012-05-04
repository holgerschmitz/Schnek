/*
 * parsersettings.hpp
 *
 *  Created on: 1 May 2012
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
 */

#ifndef PARSERSETTINGS_HPP_
#define PARSERSETTINGS_HPP_

#include "../variables/variables.hpp"
#include "../variables/function_expression.hpp"
#include "blockclasses.hpp"

namespace schnek {

struct ParserSettings
{
    VariableStorage variables;
    FunctionRegistry funcReg;
    BlockClasses blockClasses;

    ParserSettings(std::string name, std::string classname)
      :variables(name, classname)
    {}

    template<typename func>
    void registerFunction(std::string fname, func f)
    {
      funcReg.registerFunction(fname, f);
    }

    BlockClassDescriptor &addBlockClass(std::string blockClass)
    {
      return blockClasses.addBlockClass(blockClass);
    }

    BlockClassDescriptor &getBlockClass(std::string blockClass)
    {
      return blockClasses(blockClass);
    }
};

} // namespace

#endif /* PARSERSETTINGS_HPP_ */
