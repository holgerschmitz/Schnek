/*
 * parsertoken.cpp
 *
 * Created on: 26 Jan 2011
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

#include "parsertoken.hpp"
#include "deckgrammar.hpp"
#include "../variables/block.hpp"

#include "../variables/operators.hpp"
#include "../util/logger.hpp"

#include <boost/lexical_cast.hpp>

#include <iostream>

using namespace schnek;

#undef LOGLEVEL
#define LOGLEVEL 0

ParserToken::ParserToken() : context(), type(none), chainedToken()
{
  SCHNEK_TRACE_ENTER_FUNCTION(4);
}


ParserToken::ParserToken(const Token atomTok_, ParserContext context_)
  : context(context_), atomTok(atomTok_), type(atom)
{
  SCHNEK_TRACE_ENTER_FUNCTION(4);
}

ParserToken::ParserToken(const ParserToken &tok)
  : context(tok.context), atomTok(tok.atomTok), type(tok.type),
    data(tok.data), var(tok.var), chainedToken(tok.chainedToken)
{
  SCHNEK_TRACE_ENTER_FUNCTION(4);
}

ParserToken& ParserToken::operator=(const ParserToken &tok)
{
  SCHNEK_TRACE_ENTER_FUNCTION(4);
  context = tok.context;
  atomTok = tok.atomTok;
  type = tok.type;
  data = tok.data;
  var = tok.var;
  chainedToken = tok.chainedToken;
  return *this;
}

ParserToken::TokenType ParserToken::getType() const
{
  return type;
}

void ParserToken::append(ParserToken &parTok)
{
  SCHNEK_TRACE_ENTER_FUNCTION(4);
  pParserToken pt(new ParserToken(parTok));
  chainedToken = pt;
}

void ParserToken::assignInteger(ParserToken &parTok)
{
  SCHNEK_TRACE_ENTER_FUNCTION(4);
  context = parTok.context;
  atomTok = parTok.atomTok;
  if (parTok.getType() != atom) throw ParserError("Can't convert non-atom token to integer", atomTok);
  if (atomTok.getToken() != INTEGER) throw ParserError("Can't convert non-integer atom to integer", atomTok);

  pIntExpression pI(new Value<int>(boost::lexical_cast<int>(atomTok.getString())));
  data = pI;
  type = int_type;
  // std::cerr << "Read Integer " << atomTok.getString() << "\n";
}

void ParserToken::assignFloat(ParserToken &parTok)
{
  SCHNEK_TRACE_ENTER_FUNCTION(4);
  context = parTok.context;
  atomTok = parTok.atomTok;
  if (parTok.getType() != atom) throw ParserError("Can't convert non-atom token to integer", atomTok);
  if (atomTok.getToken() != FLOAT) throw ParserError("Can't convert non-float atom to float", atomTok);

  pFloatExpression pF(new Value<double>(boost::lexical_cast<double>(atomTok.getString())));
  data = pF;
  type = float_type;
  // std::cerr << "Read Float " << atomTok.getString() << "\n";
}

void ParserToken::assignString(ParserToken &parTok)
{
  SCHNEK_TRACE_ENTER_FUNCTION(4);
  context = parTok.context;
  atomTok = parTok.atomTok;
  if (parTok.getType() != atom) throw ParserError("Can't convert non-atom token to integer", atomTok);
  if (atomTok.getToken() != STRING) throw ParserError("Can't convert non-string atom to string", atomTok);

  pStringExpression pS(new Value<std::string>(atomTok.getString()));
  data = pS;
  type = string_type;
  // std::cerr << "Read String " << atomTok.getString() << "\n";
}

void ParserToken::assignIdentifier(ParserToken &parTok)
{
  SCHNEK_TRACE_ENTER_FUNCTION(4);
  context = parTok.context;
  atomTok = parTok.atomTok;
  if (parTok.getType() != atom) throw ParserError("Can't convert non-atom token to integer", atomTok);
  if (atomTok.getToken() != IDENTIFIER) throw ParserError("Can't convert non-identifier atom to identifier", atomTok);

  // convert identifier to named value from variables.hpp

  std::string varname = atomTok.getString();

  try
  {
    pVariable v = context.variables->getCurrentBlock()->getVariable(varname);
    switch (v->getType())
    {
      case Variable::int_type:
      {
        // ReferencedValue should hold a Variable object and return it's value
        // std::cerr << "int reference: ";
        pIntExpression pI(new ReferencedValue<int>(v));
        data = pI;
        break;
      }
      case Variable::float_type:
      {
        // ReferencedValue should hold a Variable object and return it's value
        // std::cerr << "float reference: ";
        pFloatExpression pF(new ReferencedValue<double>(v));
        data = pF;
        break;
      }
      case Variable::string_type:
      {
        // ReferencedValue should hold a Variable object and return it's value
        // std::cerr << "string reference: ";
        pStringExpression pS(new ReferencedValue<std::string>(v));
        data = pS;
        break;
      }
      default:
        throw ParserError("Could not identify type of '"+varname +"'",atomTok);
    }
  }
  catch (VariableNotFoundException&)
  {
    throw ParserError("Variable '"+varname +"' not defined",atomTok);
  }

  type = expression;
  // std::cerr << "Read Identifier " << varname << "\n";
}


void ParserToken::makeExpressionList()
{
  SCHNEK_TRACE_ENTER_FUNCTION(4);
  if (type != expression)
    throw ParserError("Non-expression "+toString(type)+" found in expression list!",atomTok);
  type = expressionlist;
}

void ParserToken::assignFunction(ParserToken &parTok1, ParserToken &parTok2)
{
  SCHNEK_TRACE_ENTER_FUNCTION(4);
  context = parTok2.context;
  atomTok = parTok2.atomTok;

  ExpressionList args;

  args.push_front(parTok2.data);
  pParserToken tok = parTok2.chainedToken;

  while (tok)
  {
    args.push_front(tok->data);
    tok = tok->chainedToken;
  }

  std::string funcName = parTok1.atomTok.getString();
  data = context.funcReg->getExpression(funcName, args);
  type = expression;
}

struct evaluateVisitor : public boost::static_visitor<pVariable>
{
    Token &atomTok;

    evaluateVisitor(Token &atomTok_) : atomTok(atomTok_) {}

    template<class ExpressionPointer>
    pVariable operator()(ExpressionPointer e)
    {
        SCHNEK_TRACE_ENTER_FUNCTION(4);
        if (!e) throw ParserError("Could not evaluate empty expression (local variables must be initialised where they are declared)", atomTok);
        if (e->isConstant())
        {
//          std::cerr << " * evaluating " << e->eval() << std::endl;
          pVariable pV(new Variable(e->eval()));
          return pV;
        }
        else
        {
//          std::cerr << " * assigning variable" << std::endl;
          pVariable pV(new Variable(e));
          return pV;
        }
    }
};

void ParserToken::evaluateExpression(ParserToken &identifier, ParserToken &expression)
{
  SCHNEK_TRACE_ENTER_FUNCTION(4);
  context = identifier.context;
  atomTok = identifier.atomTok;
  if (identifier.getType() != atom) throw ParserError("Can't convert non-atom token to identifier (left value)", atomTok);
  if (atomTok.getToken() != IDENTIFIER) throw ParserError("Can't convert non-identifier atom to identifier", atomTok);

  data = expression.data;

  type = statement;
}

class ValueToExpressionVisitor : public boost::static_visitor<ExpressionVariant>
{
  public:
    template<typename T>
    ExpressionVariant operator()(T var)
    {
      SCHNEK_TRACE_ENTER_FUNCTION(4);
      boost::shared_ptr< Expression<T> > e(new Value<T>(var));
      return e;
    }
};

void ParserToken::storeVariable(ParserToken &parTok)
{
  SCHNEK_TRACE_ENTER_FUNCTION(4);
  evaluateVisitor visit(atomTok);;

  var = boost::apply_visitor(visit, data);

  ensureVariable(parTok);
  std::string varname = atomTok.getString();

  // std::cerr << "Storing variable " << varname << std::endl;

  try
  {
    context.variables->addVariable(varname, var);
  }
  catch (DuplicateVariableException&)
  {
    throw ParserError("Redeclaration of variable '"+varname+"'", atomTok);
  }

  if (chainedToken) chainedToken->storeVariable(parTok);
}

void ParserToken::updateVariable()
{
  SCHNEK_TRACE_ENTER_FUNCTION(4);
  std::string varname = atomTok.getString();

  // std::cerr << "Updating variable " << varname << std::endl;
  try
  {
    pVariable storedVar = context.variables->getCurrentBlock()->getVariable(varname);
    ExpressionVariant expr;

    if (storedVar->isConstant())
    {
      ValueToExpressionVisitor visit;
      ValueVariant val = storedVar->getValue();
      expr =  boost::apply_visitor(visit, val);
    }
    else
    {
      expr = storedVar->getExpression();
    }

    TypePromoterAssign promote;
    ExpressionVariant typeMatched = boost::apply_visitor(promote, expr, data);

    evaluateVisitor eval(atomTok);
    *storedVar = *(boost::apply_visitor(eval, typeMatched));
  }
  catch (VariableNotFoundException&)
  {
    throw ParserError("Variable '"+varname+"' was not declared!", atomTok);
  }
  catch (ReadOnlyAssignmentException&)
  {
    throw ParserError("Variable '"+varname+"' is read only! It can not be assigned a value!", atomTok);
  }
}

void ParserToken::ensureVariable(ParserToken &parTok)
{
  SCHNEK_TRACE_ENTER_FUNCTION(4);

  Token typedecl = parTok.atomTok;

  if (parTok.getType() != atom) throw ParserError("Can't convert non-atom token to identifier", typedecl);

  if (type == statement)
  {
    // checking for correct type
    switch(typedecl.getToken())
    {
      case INT_DECL:
        if (var->getType() != Variable::int_type)
          throw ParserError("Can't convert expression to integer", typedecl);
        break;
      case FLOAT_DECL:
        if (var->getType() != Variable::float_type)
          throw ParserError("Can't convert expression to float", typedecl);
        break;
      case STRING_DECL:
        if (var->getType() != Variable::string_type)
          throw ParserError("Can't convert expression to string", typedecl);
        break;
      default:
        throw ParserError("Unexpected token type", typedecl);
    }
  }
  else if (type == atom)
  {
    if (atomTok.getToken() != IDENTIFIER) throw ParserError("Can't convert non-identifier atom to identifier", atomTok);

    switch(typedecl.getToken())
    {
      case INT_DECL:
      {
        pVariable pV(new Variable((int)0));
        var = pV;
        break;
      }
      case FLOAT_DECL:
      {
        pVariable pV(new Variable((double)0.0));
        var = pV;
        break;
      }
      case STRING_DECL:
      {
        pVariable pV(new Variable(std::string("")));
        var = pV;
        break;
      }
      default:
        throw ParserError("Unexpected token type", typedecl);
    }
  }
  else
    throw ParserError("Parser confused! Non-declaration found in a declaration", atomTok);

}

void ParserToken::createBlock(ParserToken &parTok)
{
  SCHNEK_TRACE_ENTER_FUNCTION(4);
  if (getType() != atom) throw ParserError("Can't convert non-atom token to identifier (block class)", atomTok);
  if (atomTok.getToken() != IDENTIFIER) throw ParserError("Can't convert non-identifier atom to identifier (block class)", atomTok);

  if (parTok.getType() != atom) throw ParserError("Can't convert non-atom token to identifier (block name)", atomTok);
  if (parTok.atomTok.getToken() != IDENTIFIER) throw ParserError("Can't convert non-identifier atom to identifier (block name)", atomTok);

  std::string blockName = parTok.atomTok.getString();
  std::string blockClass = atomTok.getString();

  //std::cerr << "Block name = " << blockName << std::endl;
  //std::cerr << "Block class = " << blockClass << std::endl;
  try
  {
    std::string parentClass = context.variables->getCurrentBlock()->getClassName();
    if (context.blockClasses->restrictBlocks() && (!context.blockClasses->hasChild(parentClass, blockClass)))
      throw ParserError("Block class "+ blockClass +" not allowed inside "+ parentClass, parTok.atomTok);
//    std::cerr << "creating block: name=" << blockName << ", class=" << blockClass << ")\n";

    pBlockVariables blockVars = context.variables->createBlock(blockName, blockClass);
    BlockClassDescriptor &blockClassDescr = context.blockClasses->get(blockClass);
    if (blockClassDescr.hasBlockFactory())
    {
      pBlock block = blockClassDescr.makeBlock(blockName);
      block->setContext(context.variables->getCurrentBlock());
      block->setup();
      context.blockTree->addChild(block);
    }
    else
    {
      context.blockTree->moveDown();
    }
  }
  catch (DuplicateBlockException&)
  {
    throw ParserError("Duplicate block definition '"+ blockName+"'", parTok.atomTok);
  }
  catch (BlockNotFoundException&)
  {
    throw ParserError("Unknown block class '"+ blockClass+"'", parTok.atomTok);
  }
}

void ParserToken::endBlock()
{
  SCHNEK_TRACE_ENTER_FUNCTION(4);
  if (context.variables->getCurrentBlock()->getParent() == 0)
    throw ParserError("Extra } found.", atomTok);
//  std::cerr << "end block\n";
  context.variables->cursorUp();
  context.blockTree->moveUp();
}

