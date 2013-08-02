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

#include "PatternRegExParser.y.h"


  void destroyLex(){
     yy_delete_buffer(YY_CURRENT_BUFFER);
    pregexlex_destroy();
  }

%}

%option prefix="pregex"
%option noyywrap


whitespace [ \n\r\t\a\b\f\t\v]
integer [0-9]+

%%

<INITIAL>{whitespace}    { ; } 
<INITIAL>{integer}       { pregexlval.numval = atoi(pregextext); return INTEGER;}
<INITIAL>"("             { return OPEN; }
<INITIAL>")"             { return CLOSE; }
<INITIAL>"*"             { return STAR; }
<INITIAL>"|"             { return OR; }
<INITIAL>"+"             { return PLUS; }
<INITIAL>"?"             { return OPTION; }
%%


