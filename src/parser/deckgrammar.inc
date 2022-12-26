/* Driver template for the LEMON parser generator.
** The author disclaims copyright to this source code.
*/
/* First off, code is included that follows the "include" declaration
** in the input grammar file. */
#include <stdio.h>
#line 11 "deckgrammar.y"

  #include <iostream>
#include "parsertoken.hpp"
#include "../variables/operators.hpp"
#line 13 "deckgrammar.c"
/* Next is all token values, in a form suitable for use by makeheaders.
** This section will be null unless lemon is run with the -m switch.
*/
/* 
** These constants (all generated automatically by the parser generator)
** specify the various kinds of tokens (terminals) that the parser
** understands. 
**
** Each symbol here is a terminal symbol in the grammar.
*/
/* Make sure the INTERFACE macro is defined.
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/* The next thing included is series of defines which control
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 terminals
**                       and nonterminals.  "int" is used otherwise.
**    YYNOCODE           is a number of type YYCODETYPE which corresponds
**                       to no legal terminal or nonterminal number.  This
**                       number is used to fill in empty slots of the hash 
**                       table.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       have fall-back values which should be used if the
**                       original value of the token will not parse.
**    YYACTIONTYPE       is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 rules and
**                       states combined.  "int" is used otherwise.
**    ParseTOKENTYPE     is the data type used for minor tokens given 
**                       directly to the parser from the tokenizer.
**    YYMINORTYPE        is the data type used for all minor tokens.
**                       This is typically a union of many types, one of
**                       which is ParseTOKENTYPE.  The entry in the union
**                       for base tokens is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    ParseARG_SDECL     A static variable declaration for the %extra_argument
**    ParseARG_PDECL     A parameter declaration for the %extra_argument
**    ParseARG_STORE     Code to store %extra_argument into yypParser
**    ParseARG_FETCH     Code to extract %extra_argument from yypParser
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
*/
#define YYCODETYPE unsigned char
#define YYNOCODE 41
#define YYACTIONTYPE unsigned char
#define ParseTOKENTYPE ParserToken
struct YYMINORTYPE {
  int yyinit;
  ParseTOKENTYPE yy0;
  YYMINORTYPE(int yyinit_, ParseTOKENTYPE yy0_=ParseTOKENTYPE()) : yyinit(yyinit_), yy0(yy0_) {}
  YYMINORTYPE(ParseTOKENTYPE yy0_=ParseTOKENTYPE()) : yyinit(0), yy0(yy0_) {}
  YYMINORTYPE &operator=(const YYMINORTYPE &rhs) { yyinit = rhs.yyinit; yy0 = rhs.yy0; return *this;}
  int yy81;
};
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define ParseARG_SDECL
#define ParseARG_PDECL
#define ParseARG_FETCH
#define ParseARG_STORE
#define YYNSTATE 74
#define YYNRULE 48
#define YYERRORSYMBOL 22
#define YYERRSYMDT yy81
#define YY_NO_ACTION      (YYNSTATE+YYNRULE+2)
#define YY_ACCEPT_ACTION  (YYNSTATE+YYNRULE+1)
#define YY_ERROR_ACTION   (YYNSTATE+YYNRULE)

/* The yyzerominor constant is used to initialize instances of
** YYMINORTYPE objects to zero. */
static const YYMINORTYPE yyzerominor(0);

/* Define the yytestcase() macro to be a no-op if is not already defined
** otherwise.
**
** Applications can choose to define yytestcase() in the %include section
** to a macro that can assist in verifying code coverage.  For production
** code the yytestcase() macro should be turned off.  But it is useful
** for testing.
*/
#ifndef yytestcase
# define yytestcase(X)
#endif


/* Next are the tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.  
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N < YYNSTATE                  Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   YYNSTATE <= N < YYNSTATE+YYNRULE   Reduce by rule N-YYNSTATE.
**
**   N == YYNSTATE+YYNRULE              A syntax error has occurred.
**
**   N == YYNSTATE+YYNRULE+1            The parser accepts its input.
**
**   N == YYNSTATE+YYNRULE+2            No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as
**
**      yy_action[ yy_shift_ofst[S] + X ]
**
** If the index value yy_shift_ofst[S]+X is out of range or if the value
** yy_lookahead[yy_shift_ofst[S]+X] is not equal to X or if yy_shift_ofst[S]
** is equal to YY_SHIFT_USE_DFLT, it means that the action is not in the table
** and that yy_default[S] should be used instead.  
**
** The formula above is for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
*/
static const YYACTIONTYPE yy_action[] = {
 /*     0 */    13,   12,   69,   23,   21,   31,    3,   58,   54,   32,
 /*    10 */    15,   17,   18,    4,   62,   55,   56,   57,   58,   44,
 /*    20 */    32,    1,   13,   12,    4,   16,   55,   56,   57,   58,
 /*    30 */    30,   32,   73,   66,   50,    4,   54,   55,   56,   57,
 /*    40 */     6,    7,   28,   63,   24,   74,   59,   21,   31,   33,
 /*    50 */    22,   54,   27,   37,   35,   19,   68,   23,   21,   31,
 /*    60 */    65,  124,   54,   54,   39,   40,   41,   64,   36,  123,
 /*    70 */     5,   42,   14,  124,   26,   34,   27,   36,   35,   19,
 /*    80 */    43,   14,    2,   26,   34,   60,    9,    8,   39,   40,
 /*    90 */    41,  124,   47,   23,   21,   31,   61,   24,   54,   59,
 /*   100 */    21,   31,   46,   24,   54,   59,   21,   31,   25,   24,
 /*   110 */    54,   59,   21,   31,  124,   58,   54,   32,  124,   29,
 /*   120 */    38,    4,    3,   55,   56,   57,   48,   23,   21,   31,
 /*   130 */    10,   11,   54,   53,   21,   31,  124,  124,   54,   67,
 /*   140 */    21,   31,  124,   71,   54,   49,   21,   31,   71,   70,
 /*   150 */    54,   45,   51,   20,   70,   54,   72,  124,   52,   21,
 /*   160 */    31,  124,  124,   54,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     1,    2,   33,   34,   35,   36,   13,    8,   39,   10,
 /*    10 */     5,    1,    2,   14,   15,   16,   17,   18,    8,    6,
 /*    20 */    10,   14,    1,    2,   14,   12,   16,   17,   18,    8,
 /*    30 */    10,   10,    6,   36,   37,   14,   39,   16,   17,   18,
 /*    40 */     1,    2,    8,   31,   32,    0,   34,   35,   36,    6,
 /*    50 */    38,   39,    7,   11,    9,   10,   33,   34,   35,   36,
 /*    60 */    36,   40,   39,   39,   19,   20,   21,   15,   22,   23,
 /*    70 */    24,   25,   26,   40,   28,   29,    7,   22,    9,   10,
 /*    80 */    25,   26,   12,   28,   29,   15,    1,    2,   19,   20,
 /*    90 */    21,   40,   33,   34,   35,   36,   31,   32,   39,   34,
 /*   100 */    35,   36,   31,   32,   39,   34,   35,   36,   31,   32,
 /*   110 */    39,   34,   35,   36,   40,    8,   39,   10,   40,   10,
 /*   120 */    11,   14,   13,   16,   17,   18,   33,   34,   35,   36,
 /*   130 */     3,    4,   39,   34,   35,   36,   40,   40,   39,   34,
 /*   140 */    35,   36,   40,   22,   39,   34,   35,   36,   22,   28,
 /*   150 */    39,   30,   36,   27,   28,   39,   30,   40,   34,   35,
 /*   160 */    36,   40,   40,   39,
};
#define YY_SHIFT_USE_DFLT (-8)
#define YY_SHIFT_MAX 32
static const signed char yy_shift_ofst[] = {
 /*     0 */    69,   -1,   21,   21,   21,   45,  107,  107,  107,  107,
 /*    10 */   107,  107,  107,  107,   20,   10,   20,  107,  107,  109,
 /*    20 */    13,  127,   70,   39,   85,   52,   26,   34,   43,   42,
 /*    30 */    -7,    5,    7,
};
#define YY_REDUCE_USE_DFLT (-32)
#define YY_REDUCE_MAX 18
static const signed char yy_reduce_ofst[] = {
 /*     0 */    46,   12,   65,   71,   77,   55,   93,   23,  -31,   59,
 /*    10 */   105,  111,   99,  124,  126,   -3,  121,  116,   24,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   122,  122,  122,  122,  122,  122,  122,  122,  122,  122,
 /*    10 */   122,  122,  122,  122,  122,  122,  122,  122,  122,  122,
 /*    20 */   122,  102,  122,   96,   93,  122,  122,  122,  122,  122,
 /*    30 */    87,  104,  115,   79,   80,   81,   82,   83,   84,  119,
 /*    40 */   120,  121,   76,   75,   77,   85,   90,   91,   94,  100,
 /*    50 */   103,  105,   97,   98,  111,  114,  116,  117,  118,   99,
 /*    60 */   109,  112,  110,  113,  108,  106,  107,  101,   95,   92,
 /*    70 */    88,   89,   86,   78,
};
#define YY_SZ_ACTTAB (int)(sizeof(yy_action)/sizeof(yy_action[0]))

/* The next table maps tokens into fallback tokens.  If a construct
** like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammar, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
};
#endif /* YYFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
*/
struct yyStackEntry {
  YYACTIONTYPE stateno;  /* The state-number */
  YYCODETYPE major;      /* The major token value.  This is the code
                         ** number for the token at this stack level */
  YYMINORTYPE minor;     /* The user-supplied minor token value.  This
                         ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int yyidx;                    /* Index of top element in stack */
#ifdef YYTRACKMAXSTACKDEPTH
  int yyidxMax;                 /* Maximum value of yyidx */
#endif
  int yyerrcnt;                 /* Shifts left before out of the error */
  ParseARG_SDECL                /* A place to hold %extra_argument */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
#else
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
#endif
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
#endif /* NDEBUG */

#ifndef NDEBUG
/* 
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL 
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
void ParseTrace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = { 
  "$",             "PLUS",          "MINUS",         "DIVIDE",      
  "TIMES",         "EXPONENT",      "SEMICOLON",     "INCLUDE",     
  "STRING",        "RBRACE",        "IDENTIFIER",    "LBRACE",      
  "COMMA",         "EQUAL",         "LBRACKET",      "RBRACKET",    
  "PATHIDENTIFIER",  "INTEGER",       "FLOAT",         "INT_DECL",    
  "FLOAT_DECL",    "STRING_DECL",   "error",         "deck",        
  "statementlist",  "statement",     "typedecl",      "decllist",    
  "assignment",    "startblock",    "declaration",   "expression",  
  "signed_term_expression",  "unsigned_expression",  "term_expression",  "exp_expression",
  "atomic_expression",  "signed_atomic_expression",  "expressionlist",  "value",       
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "deck ::= statementlist",
 /*   1 */ "statementlist ::= statementlist statement",
 /*   2 */ "statementlist ::= statement",
 /*   3 */ "statement ::= typedecl decllist SEMICOLON",
 /*   4 */ "statement ::= assignment SEMICOLON",
 /*   5 */ "statement ::= INCLUDE STRING SEMICOLON",
 /*   6 */ "statement ::= startblock",
 /*   7 */ "statement ::= RBRACE",
 /*   8 */ "statement ::= error",
 /*   9 */ "startblock ::= IDENTIFIER IDENTIFIER LBRACE",
 /*  10 */ "startblock ::= IDENTIFIER LBRACE",
 /*  11 */ "decllist ::= decllist COMMA declaration",
 /*  12 */ "decllist ::= declaration",
 /*  13 */ "declaration ::= IDENTIFIER",
 /*  14 */ "declaration ::= assignment",
 /*  15 */ "declaration ::= error",
 /*  16 */ "assignment ::= IDENTIFIER EQUAL expression",
 /*  17 */ "expression ::= signed_term_expression PLUS unsigned_expression",
 /*  18 */ "expression ::= signed_term_expression MINUS unsigned_expression",
 /*  19 */ "expression ::= signed_term_expression",
 /*  20 */ "unsigned_expression ::= term_expression PLUS unsigned_expression",
 /*  21 */ "unsigned_expression ::= term_expression MINUS unsigned_expression",
 /*  22 */ "unsigned_expression ::= term_expression",
 /*  23 */ "signed_term_expression ::= PLUS term_expression",
 /*  24 */ "signed_term_expression ::= MINUS term_expression",
 /*  25 */ "signed_term_expression ::= term_expression",
 /*  26 */ "term_expression ::= exp_expression TIMES term_expression",
 /*  27 */ "term_expression ::= exp_expression DIVIDE term_expression",
 /*  28 */ "term_expression ::= exp_expression",
 /*  29 */ "exp_expression ::= atomic_expression EXPONENT signed_atomic_expression",
 /*  30 */ "exp_expression ::= atomic_expression",
 /*  31 */ "signed_atomic_expression ::= PLUS atomic_expression",
 /*  32 */ "signed_atomic_expression ::= MINUS atomic_expression",
 /*  33 */ "signed_atomic_expression ::= atomic_expression",
 /*  34 */ "atomic_expression ::= LBRACKET expression RBRACKET",
 /*  35 */ "atomic_expression ::= IDENTIFIER LBRACKET expressionlist RBRACKET",
 /*  36 */ "atomic_expression ::= IDENTIFIER LBRACKET RBRACKET",
 /*  37 */ "atomic_expression ::= value",
 /*  38 */ "expressionlist ::= expressionlist COMMA expression",
 /*  39 */ "expressionlist ::= expression",
 /*  40 */ "value ::= PATHIDENTIFIER",
 /*  41 */ "value ::= IDENTIFIER",
 /*  42 */ "value ::= INTEGER",
 /*  43 */ "value ::= FLOAT",
 /*  44 */ "value ::= STRING",
 /*  45 */ "typedecl ::= INT_DECL",
 /*  46 */ "typedecl ::= FLOAT_DECL",
 /*  47 */ "typedecl ::= STRING_DECL",
};
#endif /* NDEBUG */


#if YYSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.
*/
static void yyGrowStack(yyParser *p){
  int newSize;
  yyStackEntry *pNew;

  newSize = p->yystksz*2 + 100;
  pNew = realloc(p->yystack, newSize*sizeof(pNew[0]));
  if( pNew ){
    p->yystack = pNew;
    p->yystksz = newSize;
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sStack grows to %d entries!\n",
              yyTracePrompt, p->yystksz);
    }
#endif
  }
}
#endif

/* 
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to Parse and ParseFree.
*/
yyParser *ParseAlloc(){
  yyParser *pParser;
  pParser = new yyParser;
  if( pParser ){
    pParser->yyidx = -1;
#ifdef YYTRACKMAXSTACKDEPTH
    pParser->yyidxMax = 0;
#endif
#if YYSTACKDEPTH<=0
    pParser->yystack = NULL;
    pParser->yystksz = 0;
    yyGrowStack(pParser);
#endif
  }
  return pParser;
}

/* The following function deletes the value associated with a
** symbol.  The symbol can be either a terminal or nonterminal.
** "yymajor" is the symbol code, and "yypminor" is a pointer to
** the value.
*/
static void yy_destructor(
  yyParser *yypParser,    /* The parser */
  YYCODETYPE yymajor,     /* Type code for object to destroy */
  YYMINORTYPE *yypminor   /* The object to be destroyed */
){
  ParseARG_FETCH;
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is 
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are not used
    ** inside the C code.
    */
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
**
** Return the major token number for the symbol popped.
*/
static int yy_pop_parser_stack(yyParser *pParser){
  YYCODETYPE yymajor;
  yyStackEntry *yytos = &pParser->yystack[pParser->yyidx];

  if( pParser->yyidx<0 ) return 0;
#ifndef NDEBUG
  if( yyTraceFILE && pParser->yyidx>=0 ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yymajor = yytos->major;
  yy_destructor(pParser, yymajor, &yytos->minor);
  pParser->yyidx--;
  return yymajor;
}

/* 
** Deallocate and destroy a parser.  Destructors are all called for
** all stack elements before shutting the parser down.
**
** Inputs:
** <ul>
** <li>  A pointer to the parser.  This should be a pointer
**       obtained from ParseAlloc.
** <li>  A pointer to a function used to reclaim memory obtained
**       from malloc.
** </ul>
*/
void ParseFree(
    yyParser *pParser                  /* The parser to be deleted */
){
  if( pParser==0 ) return;
  while( pParser->yyidx>=0 ) yy_pop_parser_stack(pParser);
#if YYSTACKDEPTH<=0
  free(pParser->yystack);
#endif
  delete pParser;
}

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int ParseStackPeak(void *p){
  yyParser *pParser = (yyParser*)p;
  return pParser->yyidxMax;
}
#endif

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;
 
  if( stateno>YY_SHIFT_MAX || (i = yy_shift_ofst[stateno])==YY_SHIFT_USE_DFLT ){
    return yy_default[stateno];
  }
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
  if( i<0 || i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
    if( iLookAhead>0 ){
#ifdef YYFALLBACK
      YYCODETYPE iFallback;            /* Fallback token */
      if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
             && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
             yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
        }
#endif
        return yy_find_shift_action(pParser, iFallback);
      }
#endif
#ifdef YYWILDCARD
      {
        int j = i - iLookAhead + YYWILDCARD;
        if( j>=0 && j<YY_SZ_ACTTAB && yy_lookahead[j]==YYWILDCARD ){
#ifndef NDEBUG
          if( yyTraceFILE ){
            fprintf(yyTraceFILE, "%sWILDCARD %s => %s\n",
               yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[YYWILDCARD]);
          }
#endif /* NDEBUG */
          return yy_action[j];
        }
      }
#endif /* YYWILDCARD */
    }
    return yy_default[stateno];
  }else{
    return yy_action[i];
  }
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_reduce_action(
  int stateno,              /* Current state number */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
#ifdef YYERRORSYMBOL
  if( stateno>YY_REDUCE_MAX ){
    return yy_default[stateno];
  }
#else
  assert( stateno<=YY_REDUCE_MAX );
#endif
  i = yy_reduce_ofst[stateno];
  assert( i!=YY_REDUCE_USE_DFLT );
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
#ifdef YYERRORSYMBOL
  if( i<0 || i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }
#else
  assert( i>=0 && i<YY_SZ_ACTTAB );
  assert( yy_lookahead[i]==iLookAhead );
#endif
  return yy_action[i];
}

/*
** The following routine is called if the stack overflows.
*/
static void yyStackOverflow(yyParser *yypParser, YYMINORTYPE *yypMinor){
   ParseARG_FETCH;
   yypParser->yyidx--;
#ifndef NDEBUG
   if( yyTraceFILE ){
     fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
   }
#endif
   while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */
   ParseARG_STORE; /* Suppress warning about unused %extra_argument var */
}

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  YYMINORTYPE *yypMinor         /* Pointer to the minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yyidx++;
#ifdef YYTRACKMAXSTACKDEPTH
  if( yypParser->yyidx>yypParser->yyidxMax ){
    yypParser->yyidxMax = yypParser->yyidx;
  }
#endif
#if YYSTACKDEPTH>0 
  if( yypParser->yyidx>=YYSTACKDEPTH ){
    yyStackOverflow(yypParser, yypMinor);
    return;
  }
#else
  if( yypParser->yyidx>=yypParser->yystksz ){
    yyGrowStack(yypParser);
    if( yypParser->yyidx>=yypParser->yystksz ){
      yyStackOverflow(yypParser, yypMinor);
      return;
    }
  }
#endif
  yytos = &yypParser->yystack[yypParser->yyidx];
  yytos->stateno = (YYACTIONTYPE)yyNewState;
  yytos->major = (YYCODETYPE)yyMajor;
  yytos->minor = *yypMinor;
#ifndef NDEBUG
  if( yyTraceFILE && yypParser->yyidx>0 ){
    int i;
    fprintf(yyTraceFILE,"%sShift %d\n",yyTracePrompt,yyNewState);
    fprintf(yyTraceFILE,"%sStack:",yyTracePrompt);
    for(i=1; i<=yypParser->yyidx; i++)
      fprintf(yyTraceFILE," %s",yyTokenName[yypParser->yystack[i].major]);
    fprintf(yyTraceFILE,"\n");
  }
#endif
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 23, 1 },
  { 24, 2 },
  { 24, 1 },
  { 25, 3 },
  { 25, 2 },
  { 25, 3 },
  { 25, 1 },
  { 25, 1 },
  { 25, 1 },
  { 29, 3 },
  { 29, 2 },
  { 27, 3 },
  { 27, 1 },
  { 30, 1 },
  { 30, 1 },
  { 30, 1 },
  { 28, 3 },
  { 31, 3 },
  { 31, 3 },
  { 31, 1 },
  { 33, 3 },
  { 33, 3 },
  { 33, 1 },
  { 32, 2 },
  { 32, 2 },
  { 32, 1 },
  { 34, 3 },
  { 34, 3 },
  { 34, 1 },
  { 35, 3 },
  { 35, 1 },
  { 37, 2 },
  { 37, 2 },
  { 37, 1 },
  { 36, 3 },
  { 36, 4 },
  { 36, 3 },
  { 36, 1 },
  { 38, 3 },
  { 38, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 26, 1 },
  { 26, 1 },
  { 26, 1 },
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  int yyruleno                 /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  YYMINORTYPE yygotominor;        /* The LHS of the rule reduced */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  ParseARG_FETCH;
  yymsp = &yypParser->yystack[yypParser->yyidx];
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno>=0 
        && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    fprintf(yyTraceFILE, "%sReduce [%s].\n", yyTracePrompt,
      yyRuleName[yyruleno]);
  }
#endif /* NDEBUG */

  /* Silence complaints from purify about yygotominor being uninitialized
  ** in some cases when it is copied into the stack after the following
  ** switch.  yygotominor is uninitialized when a rule reduces that does
  ** not set the value of its left-hand side nonterminal.  Leaving the
  ** value of the nonterminal uninitialized is utterly harmless as long
  ** as the value is never used.  So really the only thing this code
  ** accomplishes is to quieten purify.  
  **
  ** 2007-01-16:  The wireshark project (www.wireshark.org) reports that
  ** without this code, their parser segfaults.  I'm not sure what there
  ** parser is doing to make this happen.  This is the second bug report
  ** from wireshark this week.  Clearly they are stressing Lemon in ways
  ** that it has not been previously stressed...  (SQLite ticket #2172)
  */
  /*memset(&yygotominor, 0, sizeof(yygotominor));*/
  yygotominor = yyzerominor;


  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
      case 3: /* statement ::= typedecl decllist SEMICOLON */
#line 26 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 1"<< toString(yymsp[-2].minor.yy0.getType()) << " " << yymsp[-2].minor.yy0.getString()); yymsp[-1].minor.yy0.storeVariable(yymsp[-2].minor.yy0); }
#line 780 "deckgrammar.c"
        break;
      case 4: /* statement ::= assignment SEMICOLON */
#line 27 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 2"<< toString(yymsp[-1].minor.yy0.getType()) << " " << yymsp[-1].minor.yy0.getString()); yymsp[-1].minor.yy0.updateVariable(); }
#line 785 "deckgrammar.c"
        break;
      case 7: /* statement ::= RBRACE */
#line 30 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 3"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yymsp[0].minor.yy0.endBlock(); }
#line 790 "deckgrammar.c"
        break;
      case 9: /* startblock ::= IDENTIFIER IDENTIFIER LBRACE */
#line 33 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 4"<< toString(yymsp[-1].minor.yy0.getType()) << " " << yymsp[-1].minor.yy0.getString()); yymsp[-2].minor.yy0.createBlock(yymsp[-1].minor.yy0); }
#line 795 "deckgrammar.c"
        break;
      case 10: /* startblock ::= IDENTIFIER LBRACE */
#line 34 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 5"<< toString(yymsp[-1].minor.yy0.getType()) << " " << yymsp[-1].minor.yy0.getString()); yymsp[-1].minor.yy0.createBlock(yymsp[-1].minor.yy0); }
#line 800 "deckgrammar.c"
        break;
      case 11: /* decllist ::= decllist COMMA declaration */
#line 36 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 6"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yymsp[0].minor.yy0.append(yymsp[-2].minor.yy0); yygotominor.yy0 = yymsp[0].minor.yy0; }
#line 805 "deckgrammar.c"
        break;
      case 12: /* decllist ::= declaration */
#line 37 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 7"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yygotominor.yy0 = yymsp[0].minor.yy0; }
#line 810 "deckgrammar.c"
        break;
      case 13: /* declaration ::= IDENTIFIER */
#line 39 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 8"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yygotominor.yy0 = yymsp[0].minor.yy0; }
#line 815 "deckgrammar.c"
        break;
      case 14: /* declaration ::= assignment */
#line 40 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 9"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yygotominor.yy0 = yymsp[0].minor.yy0; }
#line 820 "deckgrammar.c"
        break;
      case 16: /* assignment ::= IDENTIFIER EQUAL expression */
#line 43 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 10"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yygotominor.yy0.evaluateExpression(yymsp[-2].minor.yy0,yymsp[0].minor.yy0); }
#line 825 "deckgrammar.c"
        break;
      case 17: /* expression ::= signed_term_expression PLUS unsigned_expression */
#line 45 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 11"<< toString(yymsp[-2].minor.yy0.getType()) << " " << yymsp[-2].minor.yy0.getString()); yygotominor.yy0.assignBinaryOperator<expression::OperatorAdd>(yymsp[-2].minor.yy0,yymsp[0].minor.yy0); }
#line 830 "deckgrammar.c"
        break;
      case 18: /* expression ::= signed_term_expression MINUS unsigned_expression */
#line 46 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 12"<< toString(yymsp[-2].minor.yy0.getType()) << " " << yymsp[-2].minor.yy0.getString()); yygotominor.yy0.assignBinaryOperator<expression::OperatorSubtract>(yymsp[-2].minor.yy0,yymsp[0].minor.yy0); }
#line 835 "deckgrammar.c"
        break;
      case 19: /* expression ::= signed_term_expression */
#line 47 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 13"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yygotominor.yy0 = yymsp[0].minor.yy0; }
#line 840 "deckgrammar.c"
        break;
      case 20: /* unsigned_expression ::= term_expression PLUS unsigned_expression */
#line 49 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 14"<< toString(yymsp[-2].minor.yy0.getType()) << " " << yymsp[-2].minor.yy0.getString()); yygotominor.yy0.assignBinaryOperator<expression::OperatorAdd>(yymsp[-2].minor.yy0,yymsp[0].minor.yy0); }
#line 845 "deckgrammar.c"
        break;
      case 21: /* unsigned_expression ::= term_expression MINUS unsigned_expression */
#line 50 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 15"<< toString(yymsp[-2].minor.yy0.getType()) << " " << yymsp[-2].minor.yy0.getString()); yygotominor.yy0.assignBinaryOperator<expression::OperatorSubtract>(yymsp[-2].minor.yy0,yymsp[0].minor.yy0); }
#line 850 "deckgrammar.c"
        break;
      case 22: /* unsigned_expression ::= term_expression */
#line 51 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 16"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yygotominor.yy0 = yymsp[0].minor.yy0; }
#line 855 "deckgrammar.c"
        break;
      case 23: /* signed_term_expression ::= PLUS term_expression */
#line 53 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 17"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yygotominor.yy0 = yymsp[0].minor.yy0; }
#line 860 "deckgrammar.c"
        break;
      case 24: /* signed_term_expression ::= MINUS term_expression */
#line 54 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 18"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yygotominor.yy0.assignUnaryOperator<expression::OperatorNeg>(yymsp[0].minor.yy0); }
#line 865 "deckgrammar.c"
        break;
      case 25: /* signed_term_expression ::= term_expression */
#line 55 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 19"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yygotominor.yy0 = yymsp[0].minor.yy0; }
#line 870 "deckgrammar.c"
        break;
      case 26: /* term_expression ::= exp_expression TIMES term_expression */
#line 57 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 20"<< toString(yymsp[-2].minor.yy0.getType()) << " " << yymsp[-2].minor.yy0.getString()); yygotominor.yy0.assignBinaryOperator<expression::OperatorMultiply>(yymsp[-2].minor.yy0,yymsp[0].minor.yy0); }
#line 875 "deckgrammar.c"
        break;
      case 27: /* term_expression ::= exp_expression DIVIDE term_expression */
#line 58 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 21"<< toString(yymsp[-2].minor.yy0.getType()) << " " << yymsp[-2].minor.yy0.getString()); yygotominor.yy0.assignBinaryOperator<expression::OperatorDivide>(yymsp[-2].minor.yy0,yymsp[0].minor.yy0); }
#line 880 "deckgrammar.c"
        break;
      case 28: /* term_expression ::= exp_expression */
#line 59 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 22"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yygotominor.yy0 = yymsp[0].minor.yy0; }
#line 885 "deckgrammar.c"
        break;
      case 29: /* exp_expression ::= atomic_expression EXPONENT signed_atomic_expression */
#line 60 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 23"<< toString(yymsp[-2].minor.yy0.getType()) << " " << yymsp[-2].minor.yy0.getString()); yygotominor.yy0.assignBinaryOperator<expression::OperatorExponent>(yymsp[-2].minor.yy0,yymsp[0].minor.yy0); }
#line 890 "deckgrammar.c"
        break;
      case 30: /* exp_expression ::= atomic_expression */
#line 61 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 23b"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yygotominor.yy0 = yymsp[0].minor.yy0; }
#line 895 "deckgrammar.c"
        break;
      case 31: /* signed_atomic_expression ::= PLUS atomic_expression */
#line 63 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 24"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yygotominor.yy0 = yymsp[0].minor.yy0; }
#line 900 "deckgrammar.c"
        break;
      case 32: /* signed_atomic_expression ::= MINUS atomic_expression */
#line 64 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 25"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yygotominor.yy0.assignUnaryOperator<expression::OperatorNeg>(yymsp[0].minor.yy0); }
#line 905 "deckgrammar.c"
        break;
      case 33: /* signed_atomic_expression ::= atomic_expression */
#line 65 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 26"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yygotominor.yy0 = yymsp[0].minor.yy0; }
#line 910 "deckgrammar.c"
        break;
      case 34: /* atomic_expression ::= LBRACKET expression RBRACKET */
#line 67 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 27"<< toString(yymsp[-1].minor.yy0.getType()) << " " << yymsp[-1].minor.yy0.getString()); yygotominor.yy0.assignUnaryOperator<expression::OperatorId>(yymsp[-1].minor.yy0); }
#line 915 "deckgrammar.c"
        break;
      case 35: /* atomic_expression ::= IDENTIFIER LBRACKET expressionlist RBRACKET */
#line 68 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 28"<< toString(yymsp[-3].minor.yy0.getType()) << " " << yymsp[-3].minor.yy0.getString()<< " "<< toString(yymsp[-1].minor.yy0.getType()) << " " << yymsp[-1].minor.yy0.getString()); yygotominor.yy0.assignFunction(yymsp[-3].minor.yy0,yymsp[-1].minor.yy0); }
#line 920 "deckgrammar.c"
        break;
      case 36: /* atomic_expression ::= IDENTIFIER LBRACKET RBRACKET */
#line 69 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 28b"<< toString(yymsp[-2].minor.yy0.getType()) << " " << yymsp[-2].minor.yy0.getString()); yygotominor.yy0.assignFunction(yymsp[-2].minor.yy0); }
#line 925 "deckgrammar.c"
        break;
      case 37: /* atomic_expression ::= value */
#line 71 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 29"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yygotominor.yy0.assignUnaryOperator<expression::OperatorId>(yymsp[0].minor.yy0); }
#line 930 "deckgrammar.c"
        break;
      case 38: /* expressionlist ::= expressionlist COMMA expression */
#line 73 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 30"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yymsp[0].minor.yy0.append(yymsp[-2].minor.yy0); yygotominor.yy0=yymsp[0].minor.yy0; }
#line 935 "deckgrammar.c"
        break;
      case 39: /* expressionlist ::= expression */
#line 74 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 31"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yygotominor.yy0 = yymsp[0].minor.yy0; yygotominor.yy0.makeExpressionList(); }
#line 940 "deckgrammar.c"
        break;
      case 40: /* value ::= PATHIDENTIFIER */
#line 76 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 32"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yygotominor.yy0.assignIdentifier(yymsp[0].minor.yy0); }
#line 945 "deckgrammar.c"
        break;
      case 41: /* value ::= IDENTIFIER */
#line 77 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 33"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yygotominor.yy0.assignIdentifier(yymsp[0].minor.yy0); }
#line 950 "deckgrammar.c"
        break;
      case 42: /* value ::= INTEGER */
#line 78 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 34"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yygotominor.yy0.assignInteger(yymsp[0].minor.yy0); }
#line 955 "deckgrammar.c"
        break;
      case 43: /* value ::= FLOAT */
#line 79 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 35"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yygotominor.yy0.assignFloat(yymsp[0].minor.yy0); }
#line 960 "deckgrammar.c"
        break;
      case 44: /* value ::= STRING */
#line 80 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 36"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yygotominor.yy0.assignString(yymsp[0].minor.yy0); }
#line 965 "deckgrammar.c"
        break;
      case 45: /* typedecl ::= INT_DECL */
#line 82 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 37"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yygotominor.yy0 = yymsp[0].minor.yy0; }
#line 970 "deckgrammar.c"
        break;
      case 46: /* typedecl ::= FLOAT_DECL */
#line 83 "deckgrammar.y"
{  SCHNEK_TRACE_LOG(5,"Parser: Rule 38"<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yygotominor.yy0 = yymsp[0].minor.yy0; }
#line 975 "deckgrammar.c"
        break;
      case 47: /* typedecl ::= STRING_DECL */
#line 84 "deckgrammar.y"
{ SCHNEK_TRACE_LOG(5,"Parser: Rule 39 "<< toString(yymsp[0].minor.yy0.getType()) << " " << yymsp[0].minor.yy0.getString()); yygotominor.yy0 = yymsp[0].minor.yy0; }
#line 980 "deckgrammar.c"
        break;
      default:
      /* (0) deck ::= statementlist */ yytestcase(yyruleno==0);
      /* (1) statementlist ::= statementlist statement */ yytestcase(yyruleno==1);
      /* (2) statementlist ::= statement */ yytestcase(yyruleno==2);
      /* (5) statement ::= INCLUDE STRING SEMICOLON */ yytestcase(yyruleno==5);
      /* (6) statement ::= startblock */ yytestcase(yyruleno==6);
      /* (8) statement ::= error */ yytestcase(yyruleno==8);
      /* (15) declaration ::= error */ yytestcase(yyruleno==15);
        break;
  };
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yypParser->yyidx -= yysize;
  yyact = yy_find_reduce_action(yymsp[-yysize].stateno,(YYCODETYPE)yygoto);
  if( yyact < YYNSTATE ){
#ifdef NDEBUG
    /* If we are not debugging and the reduce action popped at least
    ** one element off the stack, then we can push the new element back
    ** onto the stack here, and skip the stack overflow test in yy_shift().
    ** That gives a significant speed improvement. */
    if( yysize ){
      yypParser->yyidx++;
      yymsp -= yysize-1;
      yymsp->stateno = (YYACTIONTYPE)yyact;
      yymsp->major = (YYCODETYPE)yygoto;
      yymsp->minor = yygotominor;
    }else
#endif
    {
      yy_shift(yypParser,yyact,yygoto,&yygotominor);
    }
  }else{
    assert( yyact == YYNSTATE + YYNRULE + 1 );
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
#ifndef YYNOERRORRECOVERY
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  ParseARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
  ParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}
#endif /* YYNOERRORRECOVERY */

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  YYMINORTYPE yyminor            /* The minor type of the error token */
){
  ParseARG_FETCH;
#define TOKEN (yyminor.yy0)
#line 17 "deckgrammar.y"

  throw ParserInternalError();
#line 1052 "deckgrammar.c"
  ParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  ParseARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
  ParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "ParseAlloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void Parse(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  ParseTOKENTYPE yyminor       /* The value for the token */
  ParseARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  int yyact;            /* The parser action. */
  int yyendofinput;     /* True if we are at the end of input */
#ifdef YYERRORSYMBOL
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
#endif
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->yyidx<0 ){
#if YYSTACKDEPTH<=0
    if( yypParser->yystksz <=0 ){
      /*memset(&yyminorunion, 0, sizeof(yyminorunion));*/
      yyminorunion = yyzerominor;
      yyStackOverflow(yypParser, &yyminorunion);
      return;
    }
#endif
    yypParser->yyidx = 0;
    yypParser->yyerrcnt = -1;
    yypParser->yystack[0].stateno = 0;
    yypParser->yystack[0].major = 0;
  }
  yyminorunion.yy0 = yyminor;
  yyendofinput = (yymajor==0);
  ParseARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput %s\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,(YYCODETYPE)yymajor);
    if( yyact<YYNSTATE ){
      assert( !yyendofinput );  /* Impossible to shift the $ token */
      yy_shift(yypParser,yyact,yymajor,&yyminorunion);
      yypParser->yyerrcnt--;
      yymajor = YYNOCODE;
    }else if( yyact < YYNSTATE + YYNRULE ){
      yy_reduce(yypParser,yyact-YYNSTATE);
    }else{
      assert( yyact == YY_ERROR_ACTION );
#ifdef YYERRORSYMBOL
      int yymx;
#endif
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE,"%sSyntax Error!\n",yyTracePrompt);
      }
#endif
#ifdef YYERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".  
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( yypParser->yyerrcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yymx = yypParser->yystack[yypParser->yyidx].major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yypParser, (YYCODETYPE)yymajor,&yyminorunion);
        yymajor = YYNOCODE;
      }else{
         while(
          yypParser->yyidx >= 0 &&
          yymx != YYERRORSYMBOL &&
          (yyact = yy_find_reduce_action(
                        yypParser->yystack[yypParser->yyidx].stateno,
                        YYERRORSYMBOL)) >= YYNSTATE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yyidx < 0 || yymajor==0 ){
          yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          YYMINORTYPE u2;
          u2.YYERRSYMDT = 0;
          yy_shift(yypParser,yyact,YYERRORSYMBOL,&u2);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
#elif defined(YYNOERRORRECOVERY)
      /* If the YYNOERRORRECOVERY macro is defined, then do not attempt to
      ** do any kind of error recovery.  Instead, simply invoke the syntax
      ** error routine and continue going as if nothing had happened.
      **
      ** Applications can set this macro (for example inside %include) if
      ** they intend to abandon the parse upon the first syntax error seen.
      */
      yy_syntax_error(yypParser,yymajor,yyminorunion);
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      yymajor = YYNOCODE;
      
#else  /* YYERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( yypParser->yyerrcnt<=0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
      }
      yymajor = YYNOCODE;
#endif
    }
  }while( yymajor!=YYNOCODE && yypParser->yyidx>=0 );
  return;
}
