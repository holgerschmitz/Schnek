/*
 * parsersettings.hpp
 *
 * Created on: 1 May 2012
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

#ifndef SCHNEK_PARSERSETTINGS_HPP_
#define SCHNEK_PARSERSETTINGS_HPP_

#include "../variables/variables.hpp"
#include "../variables/function_expression.hpp"
#include "../variables/blockclasses.hpp"

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
      return blockClasses.registerBlock(blockClass);
    }

    BlockClassDescriptor &getBlock(std::string blockClass)
    {
      return blockClasses(blockClass);
    }
};

} // namespace

#endif // SCHNEK_PARSERSETTINGS_HPP_
