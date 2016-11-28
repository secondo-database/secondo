/*
----
This file is part of SECONDO.

Copyright (C) 2016, 
University in Hagen, 
Faculty of Mathematocs and Computer Science,
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

#include "dateconvertparser.h"


%}
%option prefix="dateconv"
%option noyywrap

digits		[0-9]+
%%
"Jan"		{dateconvlval = 1; return(MONTH);}
"Feb"		{dateconvlval = 2; return(MONTH);}
"Mar"		{dateconvlval = 3; return(MONTH);}
"Apr"		{dateconvlval = 4; return(MONTH);}
"May"		{dateconvlval = 5; return(MONTH);}
"Jun"		{dateconvlval = 6; return(MONTH);}
"Jul"		{dateconvlval = 7; return(MONTH);}
"Aug"		{dateconvlval = 8; return(MONTH);}
"Sep"		{dateconvlval = 9; return(MONTH);}
"Oct"		{dateconvlval = 10; return(MONTH);}
"Nov"		{dateconvlval = 11; return(MONTH);}
"Dec"		{dateconvlval = 12; return(MONTH);}
{digits}	{dateconvlval = atoi(yytext); return(NUMBER);}
.|\n		{dateconvlval = 0; return yytext[0];}

%%



