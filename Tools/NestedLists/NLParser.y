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
#include "NLParser.h"

using namespace std;

// Stack Size for the Parser - by default 200.
//#define YYINITDEPTH 10000
#define YYERROR_VERBOSE
#define YYDEBUG 1

// Both are implemented in NLParser.cpp and take the context along, so that
// nothing about one parse is reachable from another. Spelled ListExpr rather
// than YYSTYPE: this prologue is emitted before bison defines that name.
extern int yylex(ListExpr* yylval_param, NLParseCtx* ctx);
extern void yyerror(NLParseCtx* ctx, const char* s);

%}

/*
A pure (reentrant) parser: the state of a parse lives in ~NLParseCtx~ and in
yyparse's own frame instead of in globals, so two threads may parse at once.
~api.value.type~ also puts the semantic type into the generated header, which
~[#]define YYSTYPE~ did not -- the scanner's translation unit used to see the
default ~int~ and wrote four bytes into an eight-byte ~ListExpr~.

*/
%define api.pure full
%param { NLParseCtx* ctx }
%define api.value.type {ListExpr}

%code requires {
#include "NestedList.h"
struct NLParseCtx;
}

%verbose
%token-table

%token ZZINTEGER ZZREAL ZZBOOLEAN ZZSYMBOL ZZSTRING ZZTEXT ZZOPEN ZZCLOSE ZZERROR


%%

ok : list {
            /* printf("Parser: list ok."); */
            ctx->result = $1;
          }
    | atom {
            ctx->result = $1;
          }
   ;


list	: ZZOPEN rest 	{$$ = $2;}
	;

rest	: ZZCLOSE	{$$ = ctx->nl->TheEmptyList();}
	| seq ZZCLOSE	{$$ = ctx->lists.top(); ctx->lists.pop();}
	;

seq	: first		{$$ = $1; ctx->lists.push($1);}
	| seq elem	{$$ = ctx->nl->Append($1, $2,false);
              }
	;

first	: atom		{$$ = ctx->nl->OneElemList($1,false);}
	| list		{$$ = ctx->nl->OneElemList($1,false);}
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

