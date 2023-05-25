/*
 * parsertoken.cpp
 *
 * Created on: 12 Jan 2011
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

#ifndef SCHNEK_PARSERTOKEN_HPP_
#define SCHNEK_PARSERTOKEN_HPP_

#include "parsercontext.hpp"
#include "tokenlist.hpp"
#include "../variables/types.hpp"
#include "../variables/expression.hpp"

#include "../exception.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <boost/variant/apply_visitor.hpp>

#pragma GCC diagnostic pop

#include <memory>
#include <string>

namespace schnek {

class ParserOperator;
class ParserToken;

typedef std::shared_ptr<ParserToken> pParserToken;


struct ParserError : public SchnekException
{
    std::string message;
    Token atomToken;
    ParserError(std::string message_, Token atomToken_)
      : SchnekException(), message(message_), atomToken(atomToken_) {}
    std::string getFilename() { return atomToken.getFilename(); }
    int getLine() { return atomToken.getLine(); }
};


class ParserToken
{
  public:
    enum TokenType {
        deck, blocklist, block, statementlist, statement,
        expression, expressionlist, value, int_type, float_type, string_type, atom, none
    };

    ParserToken();
    ParserToken(const Token atomTok_, ParserContext context_);
    ParserToken(const ParserToken &tok);
    ParserToken& operator=(const ParserToken &tok);

    TokenType getType() const;
    std::string getString() const { return atomTok.getString(); }

    void append(ParserToken &parTok);

    // assignment functions
    void assignInteger(ParserToken &parTok);
    void assignFloat(ParserToken &parTok);
    void assignString(ParserToken &parTok);
    void assignIdentifier(ParserToken &parTok);

    template<template<class> class OpType>
    void assignUnaryOperator(ParserToken &parTok);

    template<template<class> class OpType>
    void assignBinaryOperator(ParserToken &parTok1, ParserToken &parTok2);

    void makeExpressionList();
    void assignFunction(ParserToken &parTok1, ParserToken &parTok2);
    void assignFunction(ParserToken &parTok1);

    void evaluateExpression(ParserToken &identifier, ParserToken &expression);

    void storeVariable(ParserToken &parTok);
    void updateVariable();

    void createBlock(ParserToken &parTok);
    void endBlock();

  private:
    void ensureVariable(ParserToken &parTok);

    ParserContext context;
    Token atomTok;
    TokenType type;
    ExpressionVariant data;
    pVariable var;

    pParserToken chainedToken;

};

inline std::string toString(ParserToken::TokenType type)
{
  switch (type)
  {
    case ParserToken::deck:
      return "deck";
    case ParserToken::blocklist:
      return "blocklist";
    case ParserToken::block:
      return "block";
    case ParserToken::statementlist:
      return "statementlist";
    case ParserToken::statement:
      return "statement";
    case ParserToken::expression:
      return "expression";
    case ParserToken::expressionlist:
      return "expressionlist";
    case ParserToken::value:
      return "value";
    case ParserToken::int_type:
      return "int_type";
    case ParserToken::float_type:
      return "float_type";
    case ParserToken::string_type:
      return "string_type";
    case ParserToken::atom:
      return "atom";
    case ParserToken::none:
      return "none";
    default:
      return "[Unknown ParserToken::TokenType]";
  }
}


/** This class creates result types from two argument types and stores them in a
 * ExpressionVariant.
 */
class TypePromoter : public boost::static_visitor<void>
{
  private:
    ExpressionVariant result1;
    ExpressionVariant result2;

  public:
    template<class ExpressionPointer1, class ExpressionPointer2>
    void operator()(ExpressionPointer1 e1, ExpressionPointer2 e2);

    template<class ExpressionPointer>
    void operator()(ExpressionPointer e1, ExpressionPointer e2);

    const ExpressionVariant &getResultA() { return result1; }
    const ExpressionVariant &getResultB() { return result2; }
};

/** This class creates result types from two argument types and stores them in a
 * ExpressionVariant.
 */
class TypePromoterAssign : public boost::static_visitor<ExpressionVariant>
{
  public:
    template<class ExpressionPointer1, class ExpressionPointer2>
    ExpressionVariant operator()(ExpressionPointer1, ExpressionPointer2 e2);

    template<class ExpressionPointer>
    ExpressionVariant operator()(ExpressionPointer, ExpressionPointer e2);

};

#include "parsertoken.t"

} // namespace

#endif // SCHNEK_PARSERTOKEN_HPP_
