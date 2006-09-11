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

//paragraph [1] Title: [{\Large \bf ] [}]

[1] Parser of Stable Nested Lists

Copyright (C) 1995 Gral Support Team

December 1995 Holger Schenk

February 2002 Ulrich Telle Port to C++

September 26, 2002 RHG Grammar rewritten to be left-recursive so that the parser
stack depth remains bounded for lists of arbitrary length.

December 6, 2002 M. Spiekermann Construction of the list revised. Usage of a stack data
structure avoids to create nodes which were only used in the construction process.

Dec 2004, M. Spiekermann. Macro YYERROR\_VERBOSE defined.

Sept 2006, M. Spiekermann. Code of the classes NLScanner and NLParser was
revised. Refer to NLParser.cpp.

*/
%{
#include <stdio.h>
#include <stack>
#include <vector>

#include "NestedList.h"
#include "LogMsg.h"

using namespace std;

//extern CMsg cmsg;
//extern NestedList* nl;


static stack<ListExpr> lists;

extern NestedList* parseNL_nl;
ListExpr parseNL_list;

// Stack Size for the Parser - by default 200.
//#define YYINITDEPTH 10000
#define YYERROR_VERBOSE
#define YYDEBUG 1

// interaction with flex
extern int yylex();
#define YYSTYPE ListExpr

extern void yyerror(char* s);

%}

%verbose
%token_table

%token ZZINTEGER ZZREAL ZZBOOLEAN ZZSYMBOL ZZSTRING ZZTEXT ZZOPEN ZZCLOSE ZZERROR


%%

ok : list { 
            /* printf("Parser: list ok."); */
            parseNL_list = $1;
          }
   ;


list	: ZZOPEN rest 	{$$ = $2;}
	;

rest	: ZZCLOSE	{$$ = parseNL_nl->TheEmptyList();}
	| seq ZZCLOSE	{$$ = lists.top(); lists.pop();}
	;

seq	: first		{$$ = $1; lists.push($1);}
	| seq elem	{$$ = parseNL_nl->Append($1, $2);}
	;

first	: atom		{$$ = parseNL_nl->OneElemList($1);}
	| list		{$$ = parseNL_nl->OneElemList($1);}
	; 

elem	: atom		{$$ = $1;}
	| list		{$$ = $1;}
	; 


atom : ZZINTEGER    {$$ = $1;}
     | ZZREAL       {$$ = $1; /* printf("Index of Nodes: %d\n",$1); */}
     | ZZBOOLEAN    {$$ = $1;}
     | ZZSYMBOL     {$$ = $1;}
     | ZZSTRING     {$$ = $1;}
     | ZZTEXT       {$$ = $1;}
     ;

%%

