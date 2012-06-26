/*
 * parsercontext.hpp
 *
 *  Created on: 1 May 2012
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
 */

#ifndef PARSERCONTEXT_HPP_
#define PARSERCONTEXT_HPP_

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

#endif /* PARSERCONTEXT_HPP_ */
