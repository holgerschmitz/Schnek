/*
 * parser.cpp
 *
 *  Created on: 21 Jun 2011
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
 */

#include "parser.hpp"
#include "deckscanner.hpp"
#include "tokenlist.hpp"
#include <boost/foreach.hpp>

#include "deckgrammar.hpp"

using namespace schnek;

#include "deckgrammar.inc"

void Parser::parse(std::istream &input, std::string filename)
{
  DeckScanner scanner(filename);
  scanner.scan(&input);
  const TokenList &tokens = scanner.getTokens();

  yyParser* pParser = ParseAlloc();
  FILE *trace = fopen("parsetrace.out", "w");
  ParseTrace(trace,"trace: ");

  ParserContext context(variables, funcReg);

  BOOST_FOREACH(Token tok, tokens)
  {
    Parse(pParser, tok.getToken(), ParserToken(tok, context));
  }

  Parse(pParser, 0, ParserToken());

  ParseFree(pParser);

}
