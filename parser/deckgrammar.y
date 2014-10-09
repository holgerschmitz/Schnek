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
  throw ParserInternalError();
}

deck ::= statementlist. // --done--

statementlist ::= statementlist statement. // --done--
statementlist ::= statement. // --done--

statement ::= typedecl(T) decllist(DECL) SEMICOLON. { SCHNEK_TRACE_LOG(5,"Parser: Rule 1"<< toString(T.getType()) << " " << T.getString()); DECL.storeVariable(T); }  // --done--
statement ::= assignment(T) SEMICOLON. { SCHNEK_TRACE_LOG(5,"Parser: Rule 2"<< toString(T.getType()) << " " << T.getString()); T.updateVariable(); }  // --done--
statement ::= INCLUDE STRING SEMICOLON.
statement ::= startblock.
statement ::= RBRACE(T). { SCHNEK_TRACE_LOG(5,"Parser: Rule 3"<< toString(T.getType()) << " " << T.getString()); T.endBlock(); }
statement ::= error.

startblock ::= IDENTIFIER(CLASS) IDENTIFIER(NAME) LBRACE. { SCHNEK_TRACE_LOG(5,"Parser: Rule 4"<< toString(NAME.getType()) << " " << NAME.getString()); CLASS.createBlock(NAME); } 
startblock ::= IDENTIFIER(CLASS) LBRACE. { SCHNEK_TRACE_LOG(5,"Parser: Rule 5"<< toString(CLASS.getType()) << " " << CLASS.getString()); CLASS.createBlock(CLASS); } 

decllist(LHS) ::= decllist(A) COMMA declaration(B). { SCHNEK_TRACE_LOG(5,"Parser: Rule 6"<< toString(B.getType()) << " " << B.getString()); B.append(A); LHS = B; }  // --done--
decllist(LHS) ::= declaration(RHS). { SCHNEK_TRACE_LOG(5,"Parser: Rule 7"<< toString(RHS.getType()) << " " << RHS.getString()); LHS = RHS; }  // --done--

declaration(LHS) ::= IDENTIFIER(RHS). { SCHNEK_TRACE_LOG(5,"Parser: Rule 8"<< toString(RHS.getType()) << " " << RHS.getString()); LHS = RHS; }  // --done--
declaration(LHS) ::= assignment(RHS). { SCHNEK_TRACE_LOG(5,"Parser: Rule 9"<< toString(RHS.getType()) << " " << RHS.getString()); LHS = RHS; }  // --done--
declaration ::= error.

assignment(LHS) ::= IDENTIFIER(iden) EQUAL signed_expression(expr). { SCHNEK_TRACE_LOG(5,"Parser: Rule 10"<< toString(expr.getType()) << " " << expr.getString()); LHS.evaluateExpression(iden,expr); }   // --done--

signed_expression(LHS) ::= PLUS expression(RHS). { SCHNEK_TRACE_LOG(5,"Parser: Rule 11"<< toString(RHS.getType()) << " " << RHS.getString()); LHS = RHS; }  // --done--
signed_expression(LHS) ::= MINUS expression(A). { SCHNEK_TRACE_LOG(5,"Parser: Rule 12"<< toString(A.getType()) << " " << A.getString()); LHS.assignUnaryOperator<expression::OperatorNeg>(A); }  // --done--
signed_expression(LHS) ::= expression(RHS). { SCHNEK_TRACE_LOG(5,"Parser: Rule 13"<< toString(RHS.getType()) << " " << RHS.getString()); LHS = RHS; }  // --done--

expression(LHS) ::= expression(A) PLUS expression(B). { SCHNEK_TRACE_LOG(5,"Parser: Rule 14"<< toString(A.getType()) << " " << A.getString()); LHS.assignBinaryOperator<expression::OperatorAdd>(A,B); }  // --done--
expression(LHS) ::= expression(A) MINUS expression(B). { SCHNEK_TRACE_LOG(5,"Parser: Rule 15"<< toString(A.getType()) << " " << A.getString()); LHS.assignBinaryOperator<expression::OperatorSubtract>(A,B); }  // --done--

expression(LHS) ::= expression(A) TIMES expression(B). { SCHNEK_TRACE_LOG(5,"Parser: Rule 16"<< toString(A.getType()) << " " << A.getString()); LHS.assignBinaryOperator<expression::OperatorMultiply>(A,B); }  // --done--
expression(LHS) ::= expression(A) DIVIDE expression(B). { SCHNEK_TRACE_LOG(5,"Parser: Rule 17"<< toString(A.getType()) << " " << A.getString()); LHS.assignBinaryOperator<expression::OperatorDivide>(A,B); }  // --done--
expression(LHS) ::= expression(A) EXPONENT expression(B). { SCHNEK_TRACE_LOG(5,"Parser: Rule 18"<< toString(A.getType()) << " " << A.getString()); LHS.assignBinaryOperator<expression::OperatorExponent>(A,B); }  // --done--

expression(LHS) ::= LBRACKET signed_expression(RHS) RBRACKET. { SCHNEK_TRACE_LOG(5,"Parser: Rule 19"<< toString(RHS.getType()) << " " << RHS.getString()); LHS = RHS; }
expression(LHS) ::= IDENTIFIER(A) LBRACKET expressionlist(B) RBRACKET. { SCHNEK_TRACE_LOG(5,"Parser: Rule 20"<< toString(A.getType()) << " " << A.getString()<< " "<< toString(B.getType()) << " " << B.getString()); LHS.assignFunction(A,B); }

expression(LHS) ::= value(RHS). { SCHNEK_TRACE_LOG(5,"Parser: Rule 21"<< toString(RHS.getType()) << " " << RHS.getString()); LHS.assignUnaryOperator<expression::OperatorId>(RHS); }

expressionlist(LHS) ::= expressionlist(A) COMMA signed_expression(B). { SCHNEK_TRACE_LOG(5,"Parser: Rule 22"<< toString(B.getType()) << " " << B.getString()); B.append(A); LHS=B; }
expressionlist(LHS) ::= signed_expression(RHS). { SCHNEK_TRACE_LOG(5,"Parser: Rule 23"<< toString(RHS.getType()) << " " << RHS.getString()); LHS = RHS; LHS.makeExpressionList(); }

value(LHS) ::= PATHIDENTIFIER(A).  { SCHNEK_TRACE_LOG(5,"Parser: Rule 24"<< toString(A.getType()) << " " << A.getString()); LHS.assignIdentifier(A); }  // --done--
value(LHS) ::= IDENTIFIER(A).  { SCHNEK_TRACE_LOG(5,"Parser: Rule 25"<< toString(A.getType()) << " " << A.getString()); LHS.assignIdentifier(A); }  // --done--
value(LHS) ::= INTEGER(A). { SCHNEK_TRACE_LOG(5,"Parser: Rule 26"<< toString(A.getType()) << " " << A.getString()); LHS.assignInteger(A); }  // --done--
value(LHS) ::= FLOAT(A). { SCHNEK_TRACE_LOG(5,"Parser: Rule 27"<< toString(A.getType()) << " " << A.getString()); LHS.assignFloat(A); }  // --done--
value(LHS) ::= STRING(A). { SCHNEK_TRACE_LOG(5,"Parser: Rule 28"<< toString(A.getType()) << " " << A.getString()); LHS.assignString(A); }  // --done--

typedecl(LHS) ::= INT_DECL(RHS). { SCHNEK_TRACE_LOG(5,"Parser: Rule 29"<< toString(RHS.getType()) << " " << RHS.getString()); LHS = RHS; } // --done--
typedecl(LHS) ::= FLOAT_DECL(RHS). {  SCHNEK_TRACE_LOG(5,"Parser: Rule 30"<< toString(RHS.getType()) << " " << RHS.getString()); LHS = RHS; } // --done--
typedecl(LHS) ::= STRING_DECL(RHS). { SCHNEK_TRACE_LOG(5,"Parser: Rule 31 "<< toString(RHS.getType()) << " " << RHS.getString()); LHS = RHS; } // --done--
