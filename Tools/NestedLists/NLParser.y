/*

//paragraph [1] Title: [{\Large \bf ] [}]

[1] Parser of Stable Nested Lists

Copyright (C) 1995 Gral Support Team

December 1995 Holger Schenk

February 2002 Ulrich Telle Port to C++

*/
%{
#include <stdio.h>
#include "NestedList.h"
#include "NLParser.h"
 
 // Stack Size for the Parser - by default only 200.
#define YYINITDEPTH 10000

%}

%token INTEGER REAL BOOLEAN SYMBOL STRING TEXT OPEN CLOSE 
%%

ok : list { 
            /* printf("Parser: list ok."); */
            listExpression = $1;
          }
   ;

list : OPEN rest  {$$ = $2;}
     ;

rest : CLOSE       {$$ = nl->TheEmptyList();}
     | atom rest   {$$ = nl->Cons( $1, $2 );}
     | list rest   {$$ = nl->Cons( $1, $2 );}
     ;

atom : INTEGER     {$$ = $1; /* printf("Index of Nodes: %d\n",$1); */}
     | REAL        {$$ = $1; /* printf("Index of Nodes: %d\n",$1); */}
     | BOOLEAN     {$$ = $1;}
     | SYMBOL      {$$ = $1;}
     | STRING      {$$ = $1;}
     | TEXT        {$$ = $1;}
     ;

%%

