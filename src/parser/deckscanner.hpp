/*
 * deckscanner.hpp
 *
 * Created on: 26 Apr 2012
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

#ifndef SCHNEK_DECKSCANNER_HPP_
#define SCHNEK_DECKSCANNER_HPP_

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

#endif // SCHNEK_DECKSCANNER_HPP_
