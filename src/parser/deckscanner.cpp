
#line 1 "deckscanner.rl"
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


#line 97 "deckscanner.rl"



#line 24 "deckscanner.c"
static const char _deck_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1, 
	3, 1, 4, 1, 13, 1, 14, 1, 
	15, 1, 16, 1, 17, 1, 18, 1, 
	19, 1, 20, 1, 21, 1, 22, 1, 
	23, 1, 24, 1, 25, 1, 26, 1, 
	28, 1, 29, 1, 30, 1, 31, 1, 
	32, 1, 33, 1, 34, 1, 35, 1, 
	36, 2, 0, 26, 2, 0, 27, 2, 
	4, 5, 2, 4, 6, 2, 4, 7, 
	2, 4, 8, 2, 4, 9, 2, 4, 
	10, 2, 4, 11, 2, 4, 12
};

static const short _deck_key_offsets[] = {
	0, 0, 3, 4, 7, 8, 12, 14, 
	15, 17, 20, 58, 62, 64, 66, 71, 
	79, 86, 96, 104, 113, 122, 131, 140, 
	149, 158, 167, 176, 185, 194, 203, 212, 
	221, 230, 239, 248, 257, 266, 275, 284, 
	293, 302, 311
};

static const char _deck_trans_keys[] = {
	10, 34, 92, 10, 10, 39, 92, 10, 
	43, 45, 48, 57, 48, 57, 10, 10, 
	42, 10, 42, 47, 10, 34, 39, 40, 
	41, 42, 43, 44, 45, 46, 47, 59, 
	61, 69, 94, 95, 100, 101, 102, 105, 
	114, 115, 123, 125, 33, 38, 48, 57, 
	58, 64, 65, 90, 91, 96, 97, 122, 
	124, 126, 69, 101, 48, 57, 48, 57, 
	42, 47, 46, 69, 101, 48, 57, 58, 
	95, 48, 57, 65, 90, 97, 122, 95, 
	48, 58, 65, 90, 97, 122, 43, 45, 
	58, 95, 48, 57, 65, 90, 97, 122, 
	58, 95, 48, 57, 65, 90, 97, 122, 
	58, 95, 111, 48, 57, 65, 90, 97, 
	122, 58, 95, 117, 48, 57, 65, 90, 
	97, 122, 58, 95, 98, 48, 57, 65, 
	90, 97, 122, 58, 95, 108, 48, 57, 
	65, 90, 97, 122, 58, 95, 101, 48, 
	57, 65, 90, 97, 122, 58, 95, 108, 
	48, 57, 65, 90, 97, 122, 58, 95, 
	111, 48, 57, 65, 90, 97, 122, 58, 
	95, 97, 48, 57, 65, 90, 98, 122, 
	58, 95, 116, 48, 57, 65, 90, 97, 
	122, 58, 95, 110, 48, 57, 65, 90, 
	97, 122, 58, 95, 116, 48, 57, 65, 
	90, 97, 122, 58, 95, 101, 48, 57, 
	65, 90, 97, 122, 58, 95, 103, 48, 
	57, 65, 90, 97, 122, 58, 95, 101, 
	48, 57, 65, 90, 97, 122, 58, 95, 
	114, 48, 57, 65, 90, 97, 122, 58, 
	95, 101, 48, 57, 65, 90, 97, 122, 
	58, 95, 97, 48, 57, 65, 90, 98, 
	122, 58, 95, 108, 48, 57, 65, 90, 
	97, 122, 58, 95, 116, 48, 57, 65, 
	90, 97, 122, 58, 95, 114, 48, 57, 
	65, 90, 97, 122, 58, 95, 105, 48, 
	57, 65, 90, 97, 122, 58, 95, 110, 
	48, 57, 65, 90, 97, 122, 58, 95, 
	103, 48, 57, 65, 90, 97, 122, 0
};

static const char _deck_single_lengths[] = {
	0, 3, 1, 3, 1, 2, 0, 1, 
	2, 3, 24, 2, 0, 2, 3, 2, 
	1, 4, 2, 3, 3, 3, 3, 3, 
	3, 3, 3, 3, 3, 3, 3, 3, 
	3, 3, 3, 3, 3, 3, 3, 3, 
	3, 3, 0
};

static const char _deck_range_lengths[] = {
	0, 0, 0, 0, 0, 1, 1, 0, 
	0, 0, 7, 1, 1, 0, 1, 3, 
	3, 3, 3, 3, 3, 3, 3, 3, 
	3, 3, 3, 3, 3, 3, 3, 3, 
	3, 3, 3, 3, 3, 3, 3, 3, 
	3, 3, 0
};

static const short _deck_index_offsets[] = {
	0, 0, 4, 6, 10, 12, 16, 18, 
	20, 23, 27, 59, 63, 65, 68, 73, 
	79, 84, 92, 98, 105, 112, 119, 126, 
	133, 140, 147, 154, 161, 168, 175, 182, 
	189, 196, 203, 210, 217, 224, 231, 238, 
	245, 252, 259
};

static const char _deck_indicies[] = {
	1, 2, 3, 0, 1, 0, 5, 6, 
	7, 4, 5, 4, 9, 9, 10, 8, 
	10, 8, 13, 12, 15, 16, 14, 15, 
	16, 17, 14, 19, 0, 4, 21, 22, 
	23, 24, 25, 26, 27, 28, 30, 31, 
	33, 34, 32, 35, 33, 36, 37, 38, 
	39, 40, 41, 20, 29, 20, 32, 20, 
	32, 20, 18, 43, 43, 27, 42, 10, 
	42, 45, 12, 44, 27, 43, 43, 29, 
	46, 47, 32, 32, 32, 32, 8, 47, 
	47, 47, 47, 48, 9, 9, 47, 32, 
	50, 32, 32, 49, 47, 32, 50, 32, 
	32, 49, 47, 32, 51, 32, 32, 32, 
	49, 47, 32, 52, 32, 32, 32, 49, 
	47, 32, 53, 32, 32, 32, 49, 47, 
	32, 54, 32, 32, 32, 49, 47, 32, 
	55, 32, 32, 32, 49, 47, 32, 56, 
	32, 32, 32, 49, 47, 32, 57, 32, 
	32, 32, 49, 47, 32, 58, 32, 32, 
	32, 49, 47, 32, 59, 32, 32, 32, 
	49, 47, 32, 60, 32, 32, 32, 49, 
	47, 32, 61, 32, 32, 32, 49, 47, 
	32, 63, 32, 32, 32, 62, 47, 32, 
	64, 32, 32, 32, 49, 47, 32, 65, 
	32, 32, 32, 49, 47, 32, 66, 32, 
	32, 32, 49, 47, 32, 67, 32, 32, 
	32, 49, 47, 32, 68, 32, 32, 32, 
	49, 47, 32, 69, 32, 32, 32, 49, 
	47, 32, 70, 32, 32, 32, 49, 47, 
	32, 71, 32, 32, 32, 49, 47, 32, 
	72, 32, 32, 32, 49, 47, 32, 73, 
	32, 32, 32, 49, 47, 32, 74, 32, 
	32, 32, 49, 20, 0
};

static const char _deck_trans_targs[] = {
	1, 1, 10, 2, 3, 3, 10, 4, 
	10, 6, 12, 10, 7, 10, 8, 8, 
	9, 42, 10, 10, 0, 10, 10, 10, 
	10, 10, 10, 11, 13, 14, 10, 10, 
	15, 17, 10, 19, 24, 28, 34, 37, 
	10, 10, 10, 5, 10, 10, 10, 16, 
	10, 10, 18, 20, 21, 22, 23, 15, 
	25, 26, 27, 15, 29, 30, 10, 31, 
	32, 33, 15, 35, 36, 15, 38, 39, 
	40, 41, 15
};

static const char _deck_trans_actions[] = {
	0, 1, 35, 0, 0, 1, 33, 0, 
	55, 0, 0, 53, 0, 60, 0, 1, 
	0, 3, 37, 57, 0, 17, 19, 29, 
	25, 23, 27, 84, 9, 81, 21, 11, 
	78, 78, 31, 0, 0, 0, 0, 0, 
	13, 15, 51, 0, 41, 39, 49, 0, 
	47, 45, 0, 0, 0, 0, 0, 69, 
	0, 0, 0, 66, 0, 0, 43, 0, 
	0, 0, 63, 0, 0, 72, 0, 0, 
	0, 0, 75
};

static const char _deck_to_state_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	5, 0, 5, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0
};

static const char _deck_from_state_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 7, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0
};

static const short _deck_eof_trans[] = {
	0, 0, 0, 0, 0, 9, 9, 12, 
	0, 0, 0, 43, 43, 45, 47, 9, 
	49, 50, 50, 50, 50, 50, 50, 50, 
	50, 50, 50, 50, 50, 50, 63, 50, 
	50, 50, 50, 50, 50, 50, 50, 50, 
	50, 50, 0
};

static const int deck_start = 10;
static const int deck_error = 0;

static const int deck_en_c_comment = 8;
static const int deck_en_main = 10;


#line 100 "deckscanner.rl"

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

  
#line 230 "deckscanner.c"
	{
	cs = deck_start;
	ts = 0;
	te = 0;
	act = 0;
	}

#line 116 "deckscanner.rl"

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
            
    
#line 263 "deckscanner.c"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_acts = _deck_actions + _deck_from_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 3:
#line 1 "NONE"
	{ts = p;}
	break;
#line 284 "deckscanner.c"
		}
	}

	_keys = _deck_trans_keys + _deck_key_offsets[cs];
	_trans = _deck_index_offsets[cs];

	_klen = _deck_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _deck_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _deck_indicies[_trans];
_eof_trans:
	cs = _deck_trans_targs[_trans];

	if ( _deck_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _deck_actions + _deck_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 19 "deckscanner.rl"
	{cur_line += 1;}
	break;
	case 1:
#line 23 "deckscanner.rl"
	{{cs = 10; goto _again;}}
	break;
	case 4:
#line 1 "NONE"
	{te = p+1;}
	break;
	case 5:
#line 55 "deckscanner.rl"
	{act = 14;}
	break;
	case 6:
#line 57 "deckscanner.rl"
	{act = 15;}
	break;
	case 7:
#line 58 "deckscanner.rl"
	{act = 16;}
	break;
	case 8:
#line 59 "deckscanner.rl"
	{act = 17;}
	break;
	case 9:
#line 61 "deckscanner.rl"
	{act = 18;}
	break;
	case 10:
#line 65 "deckscanner.rl"
	{act = 19;}
	break;
	case 11:
#line 89 "deckscanner.rl"
	{act = 26;}
	break;
	case 12:
#line 94 "deckscanner.rl"
	{act = 27;}
	break;
	case 13:
#line 37 "deckscanner.rl"
	{te = p+1;{ tlist.insert(cur_line, EQUAL); }}
	break;
	case 14:
#line 38 "deckscanner.rl"
	{te = p+1;{ tlist.insert(cur_line, LBRACE); }}
	break;
	case 15:
#line 39 "deckscanner.rl"
	{te = p+1;{ tlist.insert(cur_line, RBRACE); }}
	break;
	case 16:
#line 40 "deckscanner.rl"
	{te = p+1;{ tlist.insert(cur_line, LBRACKET); }}
	break;
	case 17:
#line 41 "deckscanner.rl"
	{te = p+1;{ tlist.insert(cur_line, RBRACKET); }}
	break;
	case 18:
#line 42 "deckscanner.rl"
	{te = p+1;{ tlist.insert(cur_line, SEMICOLON); }}
	break;
	case 19:
#line 43 "deckscanner.rl"
	{te = p+1;{ tlist.insert(cur_line, COMMA); }}
	break;
	case 20:
#line 45 "deckscanner.rl"
	{te = p+1;{ tlist.insert(cur_line, PLUS); }}
	break;
	case 21:
#line 46 "deckscanner.rl"
	{te = p+1;{ tlist.insert(cur_line, MINUS); }}
	break;
	case 22:
#line 47 "deckscanner.rl"
	{te = p+1;{ tlist.insert(cur_line, TIMES); }}
	break;
	case 23:
#line 49 "deckscanner.rl"
	{te = p+1;{ tlist.insert(cur_line, EXPONENT); }}
	break;
	case 24:
#line 72 "deckscanner.rl"
	{te = p+1;{ tlist.insert(cur_line, STRING,ts+1,te-ts-2); }}
	break;
	case 25:
#line 76 "deckscanner.rl"
	{te = p+1;{ tlist.insert(cur_line, STRING,ts+1,te-ts-2); }}
	break;
	case 26:
#line 79 "deckscanner.rl"
	{te = p+1;}
	break;
	case 27:
#line 84 "deckscanner.rl"
	{te = p+1;}
	break;
	case 28:
#line 86 "deckscanner.rl"
	{te = p+1;{ {cs = 8; goto _again;} }}
	break;
	case 29:
#line 48 "deckscanner.rl"
	{te = p;p--;{ tlist.insert(cur_line, DIVIDE); }}
	break;
	case 30:
#line 54 "deckscanner.rl"
	{te = p;p--;{ tlist.insert(cur_line, INT_DECL); }}
	break;
	case 31:
#line 65 "deckscanner.rl"
	{te = p;p--;{ tlist.insert(cur_line, IDENTIFIER,ts,te-ts); }}
	break;
	case 32:
#line 68 "deckscanner.rl"
	{te = p;p--;{ tlist.insert(cur_line, PATHIDENTIFIER,ts,te-ts); }}
	break;
	case 33:
#line 89 "deckscanner.rl"
	{te = p;p--;{ tlist.insert(cur_line, INTEGER,ts,te-ts); }}
	break;
	case 34:
#line 94 "deckscanner.rl"
	{te = p;p--;{ tlist.insert(cur_line, FLOAT,ts,te-ts); }}
	break;
	case 35:
#line 48 "deckscanner.rl"
	{{p = ((te))-1;}{ tlist.insert(cur_line, DIVIDE); }}
	break;
	case 36:
#line 1 "NONE"
	{	switch( act ) {
	case 14:
	{{p = ((te))-1;} tlist.insert(cur_line, INT_DECL); }
	break;
	case 15:
	{{p = ((te))-1;} tlist.insert(cur_line, FLOAT_DECL); }
	break;
	case 16:
	{{p = ((te))-1;} tlist.insert(cur_line, FLOAT_DECL); }
	break;
	case 17:
	{{p = ((te))-1;} tlist.insert(cur_line, FLOAT_DECL); }
	break;
	case 18:
	{{p = ((te))-1;} tlist.insert(cur_line, STRING_DECL); }
	break;
	case 19:
	{{p = ((te))-1;} tlist.insert(cur_line, IDENTIFIER,ts,te-ts); }
	break;
	case 26:
	{{p = ((te))-1;} tlist.insert(cur_line, INTEGER,ts,te-ts); }
	break;
	case 27:
	{{p = ((te))-1;} tlist.insert(cur_line, FLOAT,ts,te-ts); }
	break;
	}
	}
	break;
#line 516 "deckscanner.c"
		}
	}

_again:
	_acts = _deck_actions + _deck_to_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 2:
#line 1 "NONE"
	{ts = 0;}
	break;
#line 529 "deckscanner.c"
		}
	}

	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	if ( _deck_eof_trans[cs] > 0 ) {
		_trans = _deck_eof_trans[cs] - 1;
		goto _eof_trans;
	}
	}

	_out: {}
	}

#line 140 "deckscanner.rl"

    if ( cs == deck_error ) {
      std::cerr << "PARSE ERROR\n";
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
