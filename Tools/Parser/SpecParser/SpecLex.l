/*
August 4, 2000 RHG. Changed the definition of ~ident~ to include the underscore symbol. So operator names may now contain underscores, as it happens in the STDB algebra (ROSE) package by Toni.

March 2002 Ulrich Telle Port to C++, using ~flex~ and ~bison~ instead of ~lex~ and ~yacc~.

*/			

%{
#include "SpecParser.tab.hpp"
#include "NestedText.h"

extern int yylval;

#define YY_NO_UNPUT
%}

letter         [a-zA-Z]
digit          [0-9]
ident          {letter}({letter}|{digit}|_)*  
otherChar      [^\(\)\"A-Za-z0-9 \[\]\_\,\;]
math           {otherChar}{otherChar}*

%%

[ \t\n]+  	;

fun		{yylval = NestedText::Atom( yytext, yyleng ); return FUN;}
operator	{return OPERATOR;}
pattern		{return PATTERN;}
op		{return OP;}
infixop		{return INFIXOP;}
alias		{return ALIAS;}
list		{return LIST;}
implicit	{return IMPLICIT;}
parameter	{return PARAMETER;}
parameters	{return PARAMETERS;}
type		{return TYPE;}
types		{return TYPES;}
funlist		{return FUNLIST;}

{ident}		{yylval = NestedText::Atom( yytext, yyleng ); return IDENTIFIER;}
{math}		{yylval = NestedText::Atom( yytext, yyleng ); return SYMBOL;}

.		{yylval = NestedText::Atom( yytext, yyleng ); return yytext[0];}

%%
int yywrap() { return 1; }

