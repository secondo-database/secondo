/*
---- 
This file is part of SECONDO.

Copyright (C) 2014, University in Hagen, Department of Computer Science, 
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

*/

%{
#include "OpSpecParser.tab.h"



#define YY_NO_UNPUT

%}

%option prefix="opspec"

letter         [a-zA-Z]
digit          [0-9]
ident          {letter}({letter}|{digit}|_)*  
otherChar      [^\(\)\"A-Za-z0-9 \[\]\_\,\;\n  ]
math           {otherChar}{1,20}
comment        ^#.*
space          [   \n]+
autobuffer     "!!"

%option yylineno
%option nounput

%%

{comment}       {return ZZCOMMENT;} 
{space}    ; /* ignore spaces */

{autobuffer}  {return ZZFORCEBUFFER; }
fun          {return ZZFUN;}
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

{ident}      {opspeclval.text = strdup(yytext); return ZZIDENTIFIER;}
{math}       {opspeclval.text = strdup(yytext); return ZZSYMBOL;}
.            {opspeclval.text = strdup(yytext); return yytext[0];}



%%
int yywrap() { return 1; }

