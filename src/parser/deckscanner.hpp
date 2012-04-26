/*
 * tokenlist.hpp
 *
 *  Created on: 11 Jan 2011
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
 */

#ifndef SCHNEK_DECK_SCANNER_HPP_
#define SCHNEK_DECK_SCANNER_HPP_

#include "tokenlist.hpp"

namespace schnek {


class DeckScanner
{

  private:
    std::istream *input;
    TokenList tokens;

  public:
    DeckScanner(std::string filename_) : tokens(filename_) {}
    void scan(std::istream *input_, bool newscan=true)
    {
      input = input_;
      if (newscan) {
        tokens.reset();
        do_scan(tokens);
      }
      else
      {
        TokenList tklist("");
        do_scan(tklist);
        tokens.insert(tklist);
      }
    }

    const TokenList & getTokens()
    {
      return tokens;
    }
  private:

    void do_scan(TokenList &tlist);
};

} // namespace

#endif
