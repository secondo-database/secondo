/*

//paragraph [1] Title: [{\Large \bf ] [}]

[1] Parser of Stable Nested Lists

Copyright (C) 1995 Gral Support Team

December 1995 Holger Schenk

February 2002 Ulrich Telle Port to C++

September 26, 2002 RHG Grammar rewritten to be left-recursive so that the parser
stack depth remains bounded for lists of arbitrary length.

December 6, 2002 M. Spiekermann Construction of the list revised. Usage of a stack data
structure avoids to create nodes which were only used in the construction process.

Dec 2004, M. Spiekermann. Macr YYERROR\_VERBOSE defined.

*/
%{
#include <stdio.h>
#include "NestedList.h"
#include "NLParser.h"
 
// Stack Size for the Parser - by default 200.
//#define YYINITDEPTH 10000
#define YYERROR_VERBOSE
#define YYDEBUG 1

%}

%token ZZINTEGER ZZREAL ZZBOOLEAN ZZSYMBOL ZZSTRING ZZTEXT ZZOPEN ZZCLOSE ZZNOFILE ZZERROR

%verbose
%locations

%%

ok : list { 
            /* printf("Parser: list ok."); */
            listExpression = $1;
          }
   ;


list	: ZZOPEN rest 	{$$ = $2;}
	;

rest	: ZZCLOSE		{$$ = nl->TheEmptyList();}
	| seq ZZCLOSE	{$$ = lists.top(); lists.pop();}
	;

seq	: first		{$$ = $1; lists.push($1);}
	| seq elem	{$$ = nl->Append($1, $2);}
	;

first	: atom		{$$ = nl->OneElemList($1);}
	| list		{$$ = nl->OneElemList($1);}
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

