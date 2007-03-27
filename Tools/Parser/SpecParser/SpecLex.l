/*
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

August 4, 2000 RHG. Changed the definition of ~ident~ to include the underscore symbol. So operator names may now contain underscores, as it happens in the STDB algebra (ROSE) package by Toni.

March 2002 Ulrich Telle Port to C++, using ~flex~ and ~bison~ instead of ~lex~ and ~yacc~.

*/

%{
#include "SpecParser.tab.hpp"
#include "NestedText.h"


#define YY_NO_UNPUT
%}

letter         [a-zA-Z]
digit          [0-9]
ident          {letter}({letter}|{digit}|_)*  
otherChar      [^\(\)\"A-Za-z0-9 \[\]\_\,\;\n  ]
math           {otherChar}{1,20}
comment        ^#.*
space          [   \n]+

%option yylineno

%%

{comment}       {return ZZCOMMENT;} 
{space}    ; /* ignore spaces */

fun          {yylval.text = strdup(yytext); return ZZFUN;}
operator     {return ZZOPERATOR;}
pattern      {return ZZPATTERN;}
op           {return ZZOP;}
infixop      {return ZZINFIXOP;}
alias        {return ZZALIAS;}
list         {return ZZLIST;}
implicit     {return ZZIMPLICIT;}
parameter    {return ZZPARAMETER;}
parameters   {return ZZPARAMETERS;}
type         {return ZZTYPE;}
types        {return ZZTYPES;}
funlist      {return ZZFUNLIST;}

{ident}      {yylval.text = strdup(yytext); return ZZIDENTIFIER;}
{math}       {yylval.text = strdup(yytext); return ZZSYMBOL;}
.            {yylval.text = strdup(yytext); return yytext[0];}



%%
int yywrap() { return 1; }

