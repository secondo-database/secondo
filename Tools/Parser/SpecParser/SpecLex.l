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
otherChar      [^\(\)\"A-Za-z0-9 \[\]\_\,\;\n\t]
math           {otherChar}{1,20}
comment        ^#.*
space	       [ \t\n]+


%%

{comment}       {return ZZCOMMENT;} 
{space}  	; /* ignore spaces */

fun		{yylval = NestedText::Atom( yytext, yyleng ); return ZZFUN;}
operator	{return ZZOPERATOR;}
pattern		{return ZZPATTERN;}
op		{return ZZOP;}
infixop		{return ZZINFIXOP;}
alias		{return ZZALIAS;}
list		{return ZZLIST;}
implicit	{return ZZIMPLICIT;}
parameter	{return ZZPARAMETER;}
parameters	{return ZZPARAMETERS;}
type		{return ZZTYPE;}
types		{return ZZTYPES;}
funlist		{return ZZFUNLIST;}

{ident}		{yylval = NestedText::Atom( yytext, yyleng ); return ZZIDENTIFIER;}
{math}		{yylval = NestedText::Atom( yytext, yyleng ); return ZZSYMBOL;}
.               {yylval = NestedText::Atom( yytext, yyleng ); return yytext[0];}



%%
int yywrap() { return 1; }

