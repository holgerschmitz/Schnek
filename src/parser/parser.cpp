/*
 * parser.cpp
 *
 * Created on: 21 Jun 2011
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

#include "parser.hpp"
#include "deckscanner.hpp"
#include "tokenlist.hpp"
#include "../variables/block.hpp"
#include "../variables/dependencies.hpp"
#include <boost/foreach.hpp>

#include "deckgrammar.hpp"

using namespace schnek;

struct ParserInternalError : public SchnekException
{
    ParserInternalError() : SchnekException() {}
};

#undef LOGLEVEL
#define LOGLEVEL 0

#include "deckgrammar.inc"

#undef LOGLEVEL
#define LOGLEVEL 0

pBlock Parser::parse(std::istream &input, std::string filename)
{
  DeckScanner scanner(filename);
  scanner.scan(&input);
  const TokenList &tokens = scanner.getTokens();

  yyParser* pParser = ParseAlloc();
  //FILE *trace = fopen("parsetrace.out", "w");
  //ParseTrace(trace,"trace: ");

  pBlockTree blockTree(new BlockTree());
  ParserContext context(variables, funcReg, blockClasses, blockTree);

  std::string rootClass = variables.getRootBlock()->getClassName();

  BlockClassDescriptor &blockClassDescr = context.blockClasses->get(rootClass);
  if (blockClassDescr.hasBlockFactory())
  {
    pBlock block = blockClassDescr.makeBlock(variables.getRootBlock()->getBlockName());
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
    try{
      Parse(pParser, tok.getToken(), ParserToken(tok, context));
    }
    catch (ParserInternalError & e) {
      throw ParserError("Syntax Error", tok);
    }
  }

  Parse(pParser, 0, ParserToken());

  ParseFree(pParser);

  DependencyMap depMap(variables.getRootBlock());
  depMap.updateAll();


  return blockTree->getRoot();
}
