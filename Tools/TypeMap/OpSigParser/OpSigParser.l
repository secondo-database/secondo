/*
----  /Tools/TypeMap/OpSigParser/OpSigParser.l
---- 

----
This file is part of SECONDO.

Copyright (C) 2014,
Faculty of Mathematics and Computer Science,
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

#include "OpSigParser.tab.h"
#include <iostream>
#include <string>

using namespace std;



%}

%option prefix="opsig"


letter	      [a-zA-Z]
uppercase     [A-Z]    
lowercase     [a-z]
digit	      [0-9]
intindex      [1-9]
emptyspace    [ \t\n]+
otherChar     [^\(\)\{\}\"A-Za-z0-9 \:\_\,\;\.\n  ]
identifier    {letter}({letter}|{digit})*
math          {otherChar}+


%option yylineno
%option noyywrap

%%

attr/[(]        { opsiglval.tokenchar = strdup(yytext); return (ZZATTR);
                 /* because it is also an operator */ }
attrs           { opsiglval.tokenchar = strdup(yytext); return (ZZATTRS); }
combine         { opsiglval.tokenchar = strdup(yytext); return (ZZCOMBINE); }
concat          { opsiglval.tokenchar = strdup(yytext); return (ZZCONCAT); }
distinctAttrs   { opsiglval.tokenchar = strdup(yytext); return (ZZDISTATTRS); }
minus           { opsiglval.tokenchar = strdup(yytext); return (ZZMINUS); }
createAttr      { opsiglval.tokenchar = strdup(yytext); return (ZZCREATEATTR); }
where           { return (ZZWHERE); }
in              { return (ZZIN); }
^algebra        { return (ZZALG); }
"::"            { return (ZZPARAM); }
"x"             { return (ZZCROSSPRODUCT); }
"->"            { return (ZZFOLLOWS); }
";"             { return (ZZSEMICOLON); }
{identifier}    { opsiglval.tokenchar = strdup(yytext); return (ZZIDENT); }
{math}          { opsiglval.tokenchar = strdup(yytext); return (ZZSYMBOL); }
{intindex}      { opsiglval.tokenchar = strdup(yytext); return (ZZINTI); }
{emptyspace}    {}
.               {/*opsiglval.tokenchar = strdup(yytext);*/ return yytext[0];}

%%

