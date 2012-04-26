/*
 * parsertoken.cpp
 *
 *  Created on: 26 Jan 2011
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
 */

#include "parsertoken.hpp"
#include "deckgrammar.hpp"

#include "../variables/operators.hpp"

#include <boost/lexical_cast.hpp>

#include <iostream>

using namespace schnek;



ParserToken::ParserToken() : context(), type(none), chainedToken()
{}


ParserToken::ParserToken(const Token atomTok_, ParserContext context_)
  : atomTok(atomTok_), context(context_), type(atom)
{}

ParserToken::ParserToken(const ParserToken &tok)
  : context(tok.context), atomTok(tok.atomTok), type(tok.type),
    data(tok.data), var(tok.var), chainedToken(tok.chainedToken)
{}

ParserToken& ParserToken::operator=(const ParserToken &tok)
{
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
  pParserToken pt(new ParserToken(parTok));
  chainedToken = pt;
}

void ParserToken::assignInteger(ParserToken &parTok)
{
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
  context = parTok.context;
  atomTok = parTok.atomTok;
  if (parTok.getType() != atom) throw ParserError("Can't convert non-atom token to integer", atomTok);
  if (atomTok.getToken() != IDENTIFIER) throw ParserError("Can't convert non-identifier atom to identifier", atomTok);

  // convert identifier to named value from variables.hpp

  std::string varname = atomTok.getString();

  try
  {
    Variable v = context.variables->getCurrentBlock()->getVariable(varname);
    switch (v.getType())
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
  if (type != expression) throw ParserError("Non-expression found in expression list!",atomTok);
  type = expressionlist;
}

void ParserToken::assignFunction(ParserToken &parTok1, ParserToken &parTok2)
{
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
    template<class ExpressionPointer>
    pVariable operator()(ExpressionPointer e)
    {
        if (e->isConstant())
        {
          // std::cerr << " * evaluating " << e->eval() << std::endl;
          pVariable pV(new Variable(e->eval()));
          return pV;
        }
        else
        {
          // std::cerr << " * assigning variable" << std::endl;
          pVariable pV(new Variable(e));
          return pV;
        }
    }
};

void ParserToken::evaluateExpression(ParserToken &identifier, ParserToken &expression)
{
  context = identifier.context;
  atomTok = identifier.atomTok;
  if (identifier.getType() != atom) throw ParserError("Can't convert non-atom token to identifier (left value)", atomTok);
  if (atomTok.getToken() != IDENTIFIER) throw ParserError("Can't convert non-identifier atom to identifier", atomTok);

  evaluateVisitor visit;
  var = boost::apply_visitor(visit, expression.data);

  type = statement;
}

void ParserToken::storeVariable(ParserToken &parTok)
{
  ensureVariable(parTok);
  std::string varname = atomTok.getString();

  // std::cerr << "Storing variable " << varname << std::endl;

  try
  {
    context.variables->addVariable(varname, var);
  }
  catch (DuplicateVariableException&)
  {
    throw ParserError("Redeclaration of variable "+varname, atomTok);
  }

  if (chainedToken) chainedToken->storeVariable(parTok);
}

void ParserToken::updateVariable()
{
  std::string varname = atomTok.getString();

  // std::cerr << "Updating variable " << varname << std::endl;
  try
  {
    context.variables->getCurrentBlock()->getVariable(varname) = *var;
  }
  catch (VariableNotFoundException&)
  {
    throw ParserError("Variable "+varname+" was not declared!", atomTok);
  }
}

void ParserToken::ensureVariable(ParserToken &parTok)
{

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
  if (getType() != atom) throw ParserError("Can't convert non-atom token to identifier (block class)", atomTok);
  if (atomTok.getToken() != IDENTIFIER) throw ParserError("Can't convert non-identifier atom to identifier (block class)", atomTok);

  if (parTok.getType() != atom) throw ParserError("Can't convert non-atom token to identifier (block name)", atomTok);
  if (parTok.atomTok.getToken() != IDENTIFIER) throw ParserError("Can't convert non-identifier atom to identifier (block name)", atomTok);
  try
  {
//    std::cerr << "creating block " << atomTok.getString() << "(" << parTok.atomTok.getString() << ")\n";
    context.variables->createBlock(atomTok.getString(), parTok.atomTok.getString());
  }
  catch (DuplicateBlockException&)
  {
    throw ParserError("Duplicate block definition "+ parTok.atomTok.getString(), parTok.atomTok);
  }
}

void ParserToken::endBlock()
{
  if (context.variables->getCurrentBlock()->getParent() == 0)
    throw ParserError("Extra } found.", atomTok);
  context.variables->cursorUp();
}

