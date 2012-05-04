/*
 * parsertoken.cpp
 *
 *  Created on: 12 Jan 2011
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
 */

#ifndef SCHNEK_PARSERTOKEN_HPP_
#define SCHNEK_PARSERTOKEN_HPP_

#include "parsercontext.hpp"
#include "tokenlist.hpp"
#include "../variables/types.hpp"
#include "../variables/expression.hpp"

#include "../exception.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <string>

namespace schnek {

class ParserOperator;
class ParserToken;

typedef boost::shared_ptr<ParserToken> pParserToken;


struct ParserError : public SchnekException
{
    std::string message;
    Token atomToken;
    ParserError(std::string message_, Token atomToken_)
      : SchnekException(), message(message_), atomToken(atomToken_) {}
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

#include "parsertoken.t"

} // namespace

#endif /* PARSERTOKEN_HPP_ */
