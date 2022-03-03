/*
 * Token scanner for the input deck.
 * C++ code is generate with Ragel.
 */

#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include "deckscanner.hpp"
#include "deckgrammar.hpp"
#include "tokenlist.hpp"

using namespace schnek;

%%{
	machine deck;
	
	newline = '\n' @{cur_line += 1;};
	any_count_line = any | newline;

	# Consume a C comment.
	c_comment := any_count_line* :>> '*/' @{fgoto main;};

	main := |*

	# Alpha numberic characters or underscore.
	alnum_u = alnum | '_';

	# Alpha charactres or underscore.
	alpha_u = alpha | '_';

        # Alpha charactres or underscore.
        alnum_uc = alnum_u | ':';

        # Individual symbol tokens
        '='   { tlist.insert(cur_line, EQUAL); };
        '{'   { tlist.insert(cur_line, LBRACE); };
        '}'   { tlist.insert(cur_line, RBRACE); };
        '('   { tlist.insert(cur_line, LBRACKET); };
        ')'   { tlist.insert(cur_line, RBRACKET); };
        ';'   { tlist.insert(cur_line, SEMICOLON); };
        ','   { tlist.insert(cur_line, COMMA); };

        '+'   { tlist.insert(cur_line, PLUS); };
        '-'   { tlist.insert(cur_line, MINUS); };
        '*'   { tlist.insert(cur_line, TIMES); };
        '/'   { tlist.insert(cur_line, DIVIDE); };
        '^'   { tlist.insert(cur_line, EXPONENT); };

        # Keywords 
        # 'include' { tlist.insert(cur_line, INCLUDE); };
	
        'int' { tlist.insert(cur_line, INT_DECL); };
        'integer' { tlist.insert(cur_line, INT_DECL); };
        
	'float' { tlist.insert(cur_line, FLOAT_DECL); };
        'double' { tlist.insert(cur_line, FLOAT_DECL); };
        'real' { tlist.insert(cur_line, FLOAT_DECL); };
        
	'string' { tlist.insert(cur_line, STRING_DECL); };
        
	# Identifiers. 
        # Variable identifiers in declarations, class identifiers and block identifiers 
	alpha_u alnum_u* { tlist.insert(cur_line, IDENTIFIER,ts,te-ts); };

        # Variable identifiers in expressions
        alpha_u alnum_uc* { tlist.insert(cur_line, PATHIDENTIFIER,ts,te-ts); };
	
	# Single Quote string literal.
	sliteralChar = [^'\\] | newline | ( '\\' . any_count_line );
	'\'' . sliteralChar* . '\'' { tlist.insert(cur_line, STRING,ts+1,te-ts-2); };

	# Double Quote.
	dliteralChar = [^"\\] | newline | ( '\\' any_count_line );
	'"' . dliteralChar* . '"' { tlist.insert(cur_line, STRING,ts+1,te-ts-2); };

	# Whitespace is standard ws, newlines and control codes.
	any_count_line - 0x21..0x7e;

	# Describe both c style comments and c++ style comments. The
	# priority bump on tne terminator of the comments brings us
	# out of the extend* which matches everything.
	'//' [^\n]* newline;

	'/*' { fgoto c_comment; };

	# Match an integer. 
	digit+ { tlist.insert(cur_line, INTEGER,ts,te-ts); };

	# Match a floating point number.
	float_exp = [eE] [+\-]? digit+;
	float_dec = '.' digit* float_exp?;
	digit* (float_dec | float_exp) { tlist.insert(cur_line, FLOAT,ts,te-ts); };

	*|;
}%%

%% write data nofinal;

#define BUFSIZE 128

    
void DeckScanner::do_scan(TokenList &tlist)
{
  static char buf[BUFSIZE];
  int cs; 
  int act; 
  int have = 0;
  int cur_line = 1;
  
  char *ts, *te = 0;
  int done = 0;

  %% write init;

  while ( !done ) {
    char *p = buf + have, *pe, *eof = 0;
    int len, space = BUFSIZE - have;
    
    if ( space == 0 ) {
      /* We've used up the entire buffer storing an already-parsed token
       * prefix that must be preserved. */
      std::cerr << "OUT OF BUFFER SPACE\n";
      // exit(1);
    }

    input->read(p,space);
    len = input->gcount();
    //len = fread( p, 1, space, *input );
    pe = p + len;

    /* Check if this is the end of file. */
    if ( len < space ) {
      eof = pe;
      done = 1;
    }
            
    %% write exec;

    if ( cs == deck_error ) {
      std::cerr << "TOKENISER ERROR " << cur_line << "\n";
      break;
    }

    if ( ts == 0 )
      have = 0;
    else {
      /* There is a prefix to preserve, shift it over. */
      have = pe - ts;
      memmove( buf, ts, have );
      te = buf + (te-ts);
      ts = buf;
    }
  }
}
