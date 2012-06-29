/*
 * parsercontext.hpp
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

#ifndef SCHNEK_PARSERCONTEXT_HPP_
#define SCHNEK_PARSERCONTEXT_HPP_

#include "../variables/blockclasses.hpp"
#include "../variables/variables.hpp"
#include "../variables/function_expression.hpp"

namespace schnek {

class BlockTree;
typedef boost::shared_ptr<BlockTree> pBlockTree;

struct ParserContext
{
    VariableStorage *variables;
    FunctionRegistry *funcReg;
    BlockClasses *blockClasses;
    pBlockTree blockTree;

    ParserContext() {}
    ParserContext(
        VariableStorage &variables_,
        FunctionRegistry &funcReg_,
        BlockClasses &blockClasses_,
        pBlockTree blockTree_)
          : variables(&variables_),
            funcReg(&funcReg_),
            blockClasses(&blockClasses_) ,
            blockTree(blockTree_)
    {}
    ParserContext(const ParserContext &con)
          : variables(con.variables),
            funcReg(con.funcReg),
            blockClasses(con.blockClasses) ,
            blockTree(con.blockTree)
    {}

    ParserContext &operator=(const ParserContext &con)
    {
      variables = con.variables;
      funcReg = con.funcReg;
      blockClasses = con.blockClasses;
      blockTree = con.blockTree;
      return *this;
    }
};

} //namespace

#endif // SCHNEK_PARSERCONTEXT_HPP_
