/*

//paragraph [1] Title: [{\Large \bf ] [}]

[1] Parser of Stable Nested Lists

Copyright (C) 1995 Gral Support Team

December 1995 Holger Schenk

February 2002 Ulrich Telle Port to C++

September 26, 2002 RHG Grammar rewritten to be left-recursive so that the parser
stack depth remains bounded for lists of arbitrary length.

*/
%{
#include <stdio.h>
#include "NestedList.h"
#include "NLParser.h"
 
// Stack Size for the Parser - by default 200.
//#define YYINITDEPTH 10000

%}

%token INTEGER REAL BOOLEAN SYMBOL STRING TEXT OPEN CLOSE 
%%

ok : list { 
            /* printf("Parser: list ok."); */
            listExpression = $1;
          }
   ;


list	: OPEN rest 	{$$ = $2;}
	;

rest	: CLOSE		{$$ = nl->TheEmptyList();}
	| seq CLOSE	{$$ = nl->First($1);}
	;

seq	: first		{$$ = nl->TwoElemList($1, $1);}
	| seq elem	{$$ = nl->TwoElemList(nl->First($1), 
			  	nl->Append(nl->Second($1), $2));}
	;

first	: atom		{$$ = nl->OneElemList($1);}
	| list		{$$ = nl->OneElemList($1);}
	; 

elem	: atom		{$$ = $1;}
	| list		{$$ = $1;}
	; 


atom : INTEGER     {$$ = $1; /* printf("Index of Nodes: %d\n",$1); */}
     | REAL        {$$ = $1; /* printf("Index of Nodes: %d\n",$1); */}
     | BOOLEAN     {$$ = $1;}
     | SYMBOL      {$$ = $1;}
     | STRING      {$$ = $1;}
     | TEXT        {$$ = $1;}
     ;

%%

