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
#define YYNOCODE 35
#define YYACTIONTYPE unsigned char
#define ParseTOKENTYPE ParserToken
struct YYMINORTYPE {
  int yyinit;
  ParseTOKENTYPE yy0;
  YYMINORTYPE(int yyinit_, ParseTOKENTYPE yy0_=ParseTOKENTYPE()) : yyinit(yyinit_), yy0(yy0_) {}
  YYMINORTYPE(ParseTOKENTYPE yy0_=ParseTOKENTYPE()) : yyinit(0), yy0(yy0_) {}
  YYMINORTYPE &operator=(const YYMINORTYPE &rhs) { yyinit = rhs.yyinit; yy0 = rhs.yy0; return *this;}
  int yy69;
};
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define ParseARG_SDECL
#define ParseARG_PDECL
#define ParseARG_FETCH
#define ParseARG_STORE
#define YYNSTATE 59
#define YYNRULE 37
#define YYERRORSYMBOL 21
#define YYERRSYMDT yy69
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
 /*     0 */    52,   97,    1,   58,    3,   59,   28,   50,   29,   23,
 /*    10 */    51,   20,   29,    4,   51,   20,   12,   11,   13,   55,
 /*    20 */    56,   57,   13,   55,   56,   57,    8,   14,   42,   15,
 /*    30 */    26,   38,   46,   42,    5,   26,   40,   41,   45,    5,
 /*    40 */    34,   40,   41,   52,    2,    6,   32,    3,   37,   28,
 /*    50 */    50,    9,   10,   12,   11,   13,   43,   17,   22,   38,
 /*    60 */    44,   46,   31,   54,   30,    4,   21,   45,   18,   47,
 /*    70 */    38,   49,   35,   17,   19,   38,   38,   27,   17,   48,
 /*    80 */    38,   39,   17,   53,   38,   24,   25,   38,   38,   36,
 /*    90 */    16,   38,   38,   33,   98,   98,   98,   98,   98,    7,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    21,   22,   23,   24,   25,    0,   27,   28,    7,   10,
 /*    10 */     9,   10,    7,   13,    9,   10,    3,    4,    5,   18,
 /*    20 */    19,   20,    5,   18,   19,   20,    1,    2,    8,   31,
 /*    30 */    10,   33,   21,    8,   14,   10,   16,   17,   27,   14,
 /*    40 */    29,   16,   17,   21,   14,   12,   24,   25,   15,   27,
 /*    50 */    28,    1,    2,    3,    4,    5,   30,   31,   32,   33,
 /*    60 */    15,   21,   10,   11,    8,   13,   26,   27,   31,   29,
 /*    70 */    33,    6,   30,   31,   31,   33,   33,   30,   31,    6,
 /*    80 */    33,   30,   31,   11,   33,   31,   31,   33,   33,   31,
 /*    90 */    31,   33,   33,    6,   34,   34,   34,   34,   34,   12,
};
#define YY_SHIFT_USE_DFLT (-2)
#define YY_SHIFT_MAX 31
static const signed char yy_shift_ofst[] = {
 /*     0 */     1,    5,   25,   -1,   25,   25,   25,   -1,   20,   20,
 /*    10 */    20,   20,   20,   20,   20,   50,   50,   50,   13,   13,
 /*    20 */    52,   87,   33,    0,   17,   17,   30,   45,   73,   56,
 /*    30 */    65,   72,
};
#define YY_REDUCE_USE_DFLT (-22)
#define YY_REDUCE_MAX 14
static const signed char yy_reduce_ofst[] = {
 /*     0 */   -21,   22,   26,   40,   42,   47,   51,   11,   -2,   37,
 /*    10 */    43,   54,   55,   58,   59,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */    96,   96,   96,   96,   96,   96,   96,   96,   96,   96,
 /*    10 */    96,   96,   96,   96,   96,   76,   77,   78,   79,   80,
 /*    20 */    96,   96,   96,   72,   81,   82,   89,   96,   96,   96,
 /*    30 */    96,   96,   60,   62,   70,   75,   83,   85,   86,   87,
 /*    40 */    90,   91,   92,   88,   84,   73,   74,   71,   63,   64,
 /*    50 */    65,   66,   67,   68,   69,   93,   94,   95,   61,
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
  "INTEGER",       "FLOAT",         "INT_DECL",      "FLOAT_DECL",  
  "STRING_DECL",   "error",         "deck",          "statementlist",
  "statement",     "typedecl",      "decllist",      "assignment",  
  "startblock",    "declaration",   "signed_expression",  "expression",  
  "expressionlist",  "value",       
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
 /*  16 */ "assignment ::= IDENTIFIER EQUAL signed_expression",
 /*  17 */ "signed_expression ::= PLUS expression",
 /*  18 */ "signed_expression ::= MINUS expression",
 /*  19 */ "signed_expression ::= expression",
 /*  20 */ "expression ::= expression PLUS expression",
 /*  21 */ "expression ::= expression MINUS expression",
 /*  22 */ "expression ::= expression TIMES expression",
 /*  23 */ "expression ::= expression DIVIDE expression",
 /*  24 */ "expression ::= expression EXPONENT expression",
 /*  25 */ "expression ::= LBRACKET signed_expression RBRACKET",
 /*  26 */ "expression ::= IDENTIFIER LBRACKET expressionlist RBRACKET",
 /*  27 */ "expression ::= value",
 /*  28 */ "expressionlist ::= expressionlist COMMA signed_expression",
 /*  29 */ "expressionlist ::= signed_expression",
 /*  30 */ "value ::= IDENTIFIER",
 /*  31 */ "value ::= INTEGER",
 /*  32 */ "value ::= FLOAT",
 /*  33 */ "value ::= STRING",
 /*  34 */ "typedecl ::= INT_DECL",
 /*  35 */ "typedecl ::= FLOAT_DECL",
 /*  36 */ "typedecl ::= STRING_DECL",
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
  { 22, 1 },
  { 23, 2 },
  { 23, 1 },
  { 24, 3 },
  { 24, 2 },
  { 24, 3 },
  { 24, 1 },
  { 24, 1 },
  { 24, 1 },
  { 28, 3 },
  { 28, 2 },
  { 26, 3 },
  { 26, 1 },
  { 29, 1 },
  { 29, 1 },
  { 29, 1 },
  { 27, 3 },
  { 30, 2 },
  { 30, 2 },
  { 30, 1 },
  { 31, 3 },
  { 31, 3 },
  { 31, 3 },
  { 31, 3 },
  { 31, 3 },
  { 31, 3 },
  { 31, 4 },
  { 31, 1 },
  { 32, 3 },
  { 32, 1 },
  { 33, 1 },
  { 33, 1 },
  { 33, 1 },
  { 33, 1 },
  { 25, 1 },
  { 25, 1 },
  { 25, 1 },
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
{ yymsp[-1].minor.yy0.storeVariable(yymsp[-2].minor.yy0); }
#line 741 "deckgrammar.c"
        break;
      case 4: /* statement ::= assignment SEMICOLON */
#line 27 "deckgrammar.y"
{ yymsp[-1].minor.yy0.updateVariable(); }
#line 746 "deckgrammar.c"
        break;
      case 7: /* statement ::= RBRACE */
#line 30 "deckgrammar.y"
{ yymsp[0].minor.yy0.endBlock(); }
#line 751 "deckgrammar.c"
        break;
      case 9: /* startblock ::= IDENTIFIER IDENTIFIER LBRACE */
#line 33 "deckgrammar.y"
{ yymsp[-2].minor.yy0.createBlock(yymsp[-1].minor.yy0); }
#line 756 "deckgrammar.c"
        break;
      case 10: /* startblock ::= IDENTIFIER LBRACE */
#line 34 "deckgrammar.y"
{ yymsp[-1].minor.yy0.createBlock(yymsp[-1].minor.yy0); }
#line 761 "deckgrammar.c"
        break;
      case 11: /* decllist ::= decllist COMMA declaration */
#line 36 "deckgrammar.y"
{ yymsp[0].minor.yy0.append(yymsp[-2].minor.yy0); yygotominor.yy0 = yymsp[0].minor.yy0; }
#line 766 "deckgrammar.c"
        break;
      case 12: /* decllist ::= declaration */
      case 13: /* declaration ::= IDENTIFIER */ yytestcase(yyruleno==13);
      case 14: /* declaration ::= assignment */ yytestcase(yyruleno==14);
      case 17: /* signed_expression ::= PLUS expression */ yytestcase(yyruleno==17);
      case 19: /* signed_expression ::= expression */ yytestcase(yyruleno==19);
      case 27: /* expression ::= value */ yytestcase(yyruleno==27);
      case 34: /* typedecl ::= INT_DECL */ yytestcase(yyruleno==34);
      case 35: /* typedecl ::= FLOAT_DECL */ yytestcase(yyruleno==35);
      case 36: /* typedecl ::= STRING_DECL */ yytestcase(yyruleno==36);
#line 37 "deckgrammar.y"
{ yygotominor.yy0 = yymsp[0].minor.yy0; }
#line 779 "deckgrammar.c"
        break;
      case 16: /* assignment ::= IDENTIFIER EQUAL signed_expression */
#line 43 "deckgrammar.y"
{ yygotominor.yy0.evaluateExpression(yymsp[-2].minor.yy0,yymsp[0].minor.yy0); }
#line 784 "deckgrammar.c"
        break;
      case 18: /* signed_expression ::= MINUS expression */
#line 46 "deckgrammar.y"
{ yygotominor.yy0.assignUnaryOperator<expression::OperatorNeg>(yymsp[0].minor.yy0); }
#line 789 "deckgrammar.c"
        break;
      case 20: /* expression ::= expression PLUS expression */
#line 49 "deckgrammar.y"
{ yygotominor.yy0.assignBinaryOperator<expression::OperatorAdd>(yymsp[-2].minor.yy0,yymsp[0].minor.yy0); }
#line 794 "deckgrammar.c"
        break;
      case 21: /* expression ::= expression MINUS expression */
#line 50 "deckgrammar.y"
{ yygotominor.yy0.assignBinaryOperator<expression::OperatorSubtract>(yymsp[-2].minor.yy0,yymsp[0].minor.yy0); }
#line 799 "deckgrammar.c"
        break;
      case 22: /* expression ::= expression TIMES expression */
#line 52 "deckgrammar.y"
{ yygotominor.yy0.assignBinaryOperator<expression::OperatorMultiply>(yymsp[-2].minor.yy0,yymsp[0].minor.yy0); }
#line 804 "deckgrammar.c"
        break;
      case 23: /* expression ::= expression DIVIDE expression */
#line 53 "deckgrammar.y"
{ yygotominor.yy0.assignBinaryOperator<expression::OperatorDivide>(yymsp[-2].minor.yy0,yymsp[0].minor.yy0); }
#line 809 "deckgrammar.c"
        break;
      case 24: /* expression ::= expression EXPONENT expression */
#line 54 "deckgrammar.y"
{ yygotominor.yy0.assignBinaryOperator<expression::OperatorExponent>(yymsp[-2].minor.yy0,yymsp[0].minor.yy0); }
#line 814 "deckgrammar.c"
        break;
      case 25: /* expression ::= LBRACKET signed_expression RBRACKET */
#line 56 "deckgrammar.y"
{ yygotominor.yy0 = yymsp[-1].minor.yy0; }
#line 819 "deckgrammar.c"
        break;
      case 26: /* expression ::= IDENTIFIER LBRACKET expressionlist RBRACKET */
#line 57 "deckgrammar.y"
{ yygotominor.yy0.assignFunction(yymsp[-3].minor.yy0,yymsp[-1].minor.yy0); }
#line 824 "deckgrammar.c"
        break;
      case 28: /* expressionlist ::= expressionlist COMMA signed_expression */
#line 61 "deckgrammar.y"
{ yymsp[0].minor.yy0.append(yymsp[-2].minor.yy0); yygotominor.yy0=yymsp[0].minor.yy0; }
#line 829 "deckgrammar.c"
        break;
      case 29: /* expressionlist ::= signed_expression */
#line 62 "deckgrammar.y"
{ yygotominor.yy0 = yymsp[0].minor.yy0; yygotominor.yy0.makeExpressionList(); }
#line 834 "deckgrammar.c"
        break;
      case 30: /* value ::= IDENTIFIER */
#line 64 "deckgrammar.y"
{ yygotominor.yy0.assignIdentifier(yymsp[0].minor.yy0); }
#line 839 "deckgrammar.c"
        break;
      case 31: /* value ::= INTEGER */
#line 65 "deckgrammar.y"
{ yygotominor.yy0.assignInteger(yymsp[0].minor.yy0); }
#line 844 "deckgrammar.c"
        break;
      case 32: /* value ::= FLOAT */
#line 66 "deckgrammar.y"
{ yygotominor.yy0.assignFloat(yymsp[0].minor.yy0); }
#line 849 "deckgrammar.c"
        break;
      case 33: /* value ::= STRING */
#line 67 "deckgrammar.y"
{ yygotominor.yy0.assignString(yymsp[0].minor.yy0); }
#line 854 "deckgrammar.c"
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

  std::cout << "Syntax error!" << std::endl;
#line 926 "deckgrammar.c"
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
