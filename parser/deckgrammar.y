// This is the file specifying the grammar for the input deck
// It is used with the lemon parser generator to generate deck_grammar.cpp
// Usually this will only have to be used when the input deck grammar changes

%token_type {ParserToken}

%left PLUS MINUS.
%left DIVIDE TIMES.
%left EXPONENT.

%include {
  #include <iostream>
#include "parsertoken.hpp"
#include "../variables/operators.hpp"
}

%syntax_error {
  std::cout << "Syntax error!" << std::endl;
}

deck ::= statementlist. // --done--

statementlist ::= statementlist statement. // --done--
statementlist ::= statement. // --done--

statement ::= typedecl(T) decllist(DECL) SEMICOLON. { DECL.storeVariable(T); }  // --done--
statement ::= assignment(T) SEMICOLON. { T.updateVariable(); }  // --done--
statement ::= INCLUDE STRING SEMICOLON.
statement ::= startblock.
statement ::= RBRACE(T). { T.endBlock(); }
statement ::= error.

startblock ::= IDENTIFIER(CLASS) IDENTIFIER(NAME) LBRACE. { CLASS.createBlock(NAME); } 
startblock ::= IDENTIFIER(CLASS) LBRACE. { CLASS.createBlock(CLASS); } 

decllist(LHS) ::= decllist(A) COMMA declaration(B). { B.append(A); LHS = B; }  // --done--
decllist(LHS) ::= declaration(RHS). { LHS = RHS; }  // --done--

declaration(LHS) ::= IDENTIFIER(RHS). { LHS = RHS; }  // --done--
declaration(LHS) ::= assignment(RHS). { LHS = RHS; }  // --done--
declaration ::= error.

assignment(LHS) ::= IDENTIFIER(iden) EQUAL signed_expression(expr). { LHS.evaluateExpression(iden,expr); }   // --done--

signed_expression(LHS) ::= PLUS expression(RHS). { LHS = RHS; }  // --done--
signed_expression(LHS) ::= MINUS expression(A). { LHS.assignUnaryOperator<expression::OperatorNeg>(A); }  // --done--
signed_expression(LHS) ::= expression(RHS). { LHS = RHS; }  // --done--

expression(LHS) ::= expression(A) PLUS expression(B). { LHS.assignBinaryOperator<expression::OperatorAdd>(A,B); }  // --done--
expression(LHS) ::= expression(A) MINUS expression(B). { LHS.assignBinaryOperator<expression::OperatorSubtract>(A,B); }  // --done--

expression(LHS) ::= expression(A) TIMES expression(B). { LHS.assignBinaryOperator<expression::OperatorMultiply>(A,B); }  // --done--
expression(LHS) ::= expression(A) DIVIDE expression(B). { LHS.assignBinaryOperator<expression::OperatorDivide>(A,B); }  // --done--
expression(LHS) ::= expression(A) EXPONENT expression(B). { LHS.assignBinaryOperator<expression::OperatorExponent>(A,B); }  // --done--

expression(LHS) ::= LBRACKET signed_expression(RHS) RBRACKET. { LHS = RHS; }
expression(LHS) ::= IDENTIFIER(A) LBRACKET expressionlist(B) RBRACKET. { LHS.assignFunction(A,B); }

expression(LHS) ::= value(RHS). { LHS = RHS; }

expressionlist(LHS) ::= expressionlist(A) COMMA signed_expression(B). { B.append(A); LHS=B; }
expressionlist(LHS) ::= signed_expression(RHS). { LHS = RHS; LHS.makeExpressionList(); }

value(LHS) ::= PATHIDENTIFIER(A).  { LHS.assignIdentifier(A); }  // --done--
value(LHS) ::= IDENTIFIER(A).  { LHS.assignIdentifier(A); }  // --done--
value(LHS) ::= INTEGER(A). { LHS.assignInteger(A); }  // --done--
value(LHS) ::= FLOAT(A). { LHS.assignFloat(A); }  // --done--
value(LHS) ::= STRING(A). { LHS.assignString(A); }  // --done--

typedecl(LHS) ::= INT_DECL(RHS). { LHS = RHS; } // --done--
typedecl(LHS) ::= FLOAT_DECL(RHS). { LHS = RHS; } // --done--
typedecl(LHS) ::= STRING_DECL(RHS). { LHS = RHS; } // --done--
