/*
 * tokenlist.hpp
 *
 *  Created on: 11 Jan 2011
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
 */

#ifndef SCHNEK_TOKENLIST_HPP_
#define SCHNEK_TOKENLIST_HPP_

#include <boost/shared_ptr.hpp>
#include <list>
#include <string>
#include <iostream>
#include "deckgrammar.hpp"

namespace schnek {

/* Stores an individual token.
 *
 * Along with the token id and a string value (for identifiers of string literals)
 * the location of the token is also stored by filename and line.
 */
class Token
{
  public:
    // default constructor
    Token() {}

    /// Construct a token
    Token(std::string filename_, int line_, int token_, std::string str_ = "")
        : filename(filename_), line(line_), token(token_), str(str_) {
      // std::cerr << "Creating token " << token << " in line " << line << " ("<<str<<")\n";
      if (token==PATHIDENTIFIER) token = IDENTIFIER;
    }

    /// Copy constructor
    Token(const Token &t)
      : filename(t.filename), line(t.line), token(t.token), str(t.str) {}

    Token& operator=(const Token &t)
    {
      filename = t.filename;
      line = t.line;
      token = t.token;
      str = t.str;
      return *this;
    }

    /// Returns the name of the file from which the token was read
    std::string getFilename() const
    {
        return filename;
    }

    /// Returns the line number where the token was encountered
    int getLine() const
    {
        return line;
    }

    /// Returns the token id
    int getToken() const
    {
        return token;
    }

    /// Returns the string constant associated with the token
    std::string getString() const
    {
        return str;
    }

private:
    /// The name of the file from which the token was read
    std::string filename;
    /// The line number where the token was encountered
    int line;
    /// The token id
    int token;
    /// The string constant associated with the token
    std::string str;
};

typedef boost::shared_ptr<Token> pToken;

/* Contains a list of parser tokens.
 *
 * The list is filled by the scanner i.e. DeckScanner and then read by the
 * parser, i.e. DeckParser
 *
 * Tokens are stored together with the line numbers to allow user friendly
 * error messages.
 */
class TokenList
{
  private:
    std::list<Token> tlist;
    std::string filename;
  public:
    typedef std::list<Token>::const_iterator const_iterator;

    /// Default constructor
    TokenList(std::string filename_) : filename(filename_) {}

    /// Empties the list and reassigns the filename

    void reset(std::string filename_)
    {
      tlist.clear();
      filename = filename_;
    }

    /// Empties the list and reassigns the filename

    void reset()
    {
      tlist.clear();
    }

    /// Insert an individual token onto the list.
    void insert(int line, int token)
    {
      tlist.push_back(Token(filename, line, token));
    }

    /* Insert a token that is associated with a string onto the list.
     *
     * This stores tokens such as identifiers or string literals.
     */
    void insert(int line, int token, char *str, size_t len)
    {
      tlist.push_back(Token(filename, line, token, std::string(str,len)));
    }

    /* Insert another list of tokens in front of the current one.
     *
     * This is used when encountering include statements
     */
    void insert(const TokenList &tokens)
    {
      tlist.insert(tlist.begin(), tokens.begin(), tokens.end());
    }

    /// Returns an iterator to the beginning of the list
    const_iterator begin() const
    {
      return tlist.begin();
    }

    /// Returns an iterator to the end of the list
    const_iterator end() const
    {
      return tlist.end();
    }
};

} // namespace

#endif /* SCHNEK_TOKENLIST_HPP_ */
