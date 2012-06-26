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
#include "block.hpp"
#include <boost/foreach.hpp>

#include "deckgrammar.hpp"

using namespace schnek;

#include "deckgrammar.inc"

pBlock Parser::parse(std::istream &input, std::string filename)
{
  DeckScanner scanner(filename);
  scanner.scan(&input);
  const TokenList &tokens = scanner.getTokens();

  yyParser* pParser = ParseAlloc();
  FILE *trace = fopen("parsetrace.out", "w");
  ParseTrace(trace,"trace: ");

  pBlockTree blockTree(new BlockTree());
  ParserContext context(variables, funcReg, blockClasses, blockTree);

  std::string rootClass = variables.getRootBlock()->getClassName();

  BlockClassDescriptor &blockClassDescr = context.blockClasses->get(rootClass);
  if (blockClassDescr.hasBlockFactory())
  {
    pBlock block = blockClassDescr.makeBlock();
    block->setContext(context.variables->getCurrentBlock());
    block->setup();
    context.blockTree->addChild(block);
  }
  else
  {
    context.blockTree->moveDown();
  }



  BOOST_FOREACH(Token tok, tokens)
  {
    Parse(pParser, tok.getToken(), ParserToken(tok, context));
  }

  Parse(pParser, 0, ParserToken());

  ParseFree(pParser);
  return blockTree->getRoot();
}
