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

This file is the input for lex or the open source equivalent flex.
It is used to generate a special tree for boolean expressions 
with respect to the topological relationshsips between spatial objects
represented by the 9 intersection matrix. 

*/


#include "Tree.h" 
#include "TreeParser.y.h"

%}

%option prefix="tree"
%option noyywrap

whitespace [ \n\r\t\a\b\f\t\v]
and        ([aA][nN][dD])|"&"
or         ([oO][rR])|"|"
xor        ([xX][oO][rR])|"^"
not        ([nN][oO][tT])|"!"
true       [tT][rR][uU][eE]
false      [fF][aA][lL][sS][eE]
conditional "=>"|"->"
biconditional "<=>"|"<->"

%%

{whitespace}   { ; }
"ii"           { return II; }
"ib"           { return IB; }
"ie"           { return IE; }
"bi"           { return BI; }
"bb"           { return BB; }
"be"           { return BE; }
"ei"           { return EI; }
"eb"           { return EB; }
"ee"           { return EE; }
{and}          { return AND; }
{or}           { return OR; }
{xor}          { return XOR; }
{not}          { return NOT; }
{true}         {  return TRUE; }
{false}        { return FALSE; }
"("            { return OPEN; }
")"            { return CLOSE; }
{conditional}  { return CONDITIONAL; }
{biconditional} { return BICONDITIONAL;}
.               { return ERROR; }

%%


