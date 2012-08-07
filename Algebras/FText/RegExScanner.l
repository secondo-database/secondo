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

*/

%{

/*

1 The Scannerfile


*/


#include "RegExParser.y.h"


  void lexDestroy(){
     yy_delete_buffer(YY_CURRENT_BUFFER);
    cregexlex_destroy();
  }

%}

%option prefix="cregex"
%option noyywrap

%x SET


whitespace [ \n\r\t\a\b\f\t\v]
stars "*"("*"|whitespace)*

%%




<INITIAL>{whitespace}   { ; } 
<INITIAL>"\\\\"          { cregexlval.numval = (int) '\\'; return CHAR; }
<INITIAL>"\\("           { cregexlval.numval = (int)'('; return CHAR; }
<INITIAL>"\\)"           { cregexlval.numval = (int)')'; return CHAR;}
<INITIAL>"\\|"           { cregexlval.numval =  (int)'|'; return CHAR;}
<INITIAL>"\\*"           { cregexlval.numval =  (int)'*'; return CHAR;}
<INITIAL>"\\."           { cregexlval.numval =  (int)'.'; return CHAR;}
<INITIAL>"\\+"           { cregexlval.numval = (int)'+'; return CHAR;}
<INITIAL>"\\?"           { cregexlval.numval = (int)'?'; return CHAR;}
<INITIAL>"\\["           { cregexlval.numval = (int)'['; return CHAR;}
<INITIAL>"("             { return OPEN; }
<INITIAL>")"             { return CLOSE; }
<INITIAL>{stars}         { return STARS; } 
<INITIAL>"|"             { return OR; }
<INITIAL>"+"             { return PLUS; }
<INITIAL>"."             { return ANY; }
<INITIAL>"?"             { return OPTION; }
<INITIAL>"["             { BEGIN(SET);
                           return OPENSET;
                         }
<INITIAL>"[^"            { BEGIN(SET);
                           return OPENINVERSESET;
                         }
<INITIAL>.               { cregexlval.numval = (int)(unsigned char)yytext[0];
                           return CHAR;}

<SET>"\\-"               { cregexlval.numval = (int)'-'; return CHAR;}
<SET>"-"                 { return TO; }
<SET>"\\\\"              { cregexlval.numval = (int)'\\'; return CHAR;}
<SET>"\\]"               { cregexlval.numval = (int)']'; return CHAR; }
<SET>"]"                 { BEGIN(INITIAL);
                           return CLOSESET;
                         }
<SET>.                   { cregexlval.numval = (int)(unsigned char)yytext[0];
                           return CHAR; 
                         }

%%


