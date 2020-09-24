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

int dateconvlex();
int dateconverror (const char *error);
void dateconv_scan_string(const char* argument);



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dateconvertparser.h"



int dateconv_success;
char* result;


%}

%name-prefix "dateconv"

%token NUMBER MONTH
%%

lines	: date
	| lines date
	| lines '\n'
	;

date	: '[' NUMBER '/' MONTH '/' NUMBER ':' 
		NUMBER ':' NUMBER ':' NUMBER 
	  {
            result = malloc(48); 
            sprintf(result,"%d-%d-%d-%d:%d:%d", $6, $4, $2, $8, $10, $12);
	  }
	;

%%

int dateconverror (const char *error)
{ 
  printf("%s\n",error);
  dateconv_success=0;
  return 1;
}


char* convertDate(const char* input)
{
  result = 0;
  dateconv_scan_string(input);
  yyparse();
  char* r =   result;
  result = 0;
  return r;
}
