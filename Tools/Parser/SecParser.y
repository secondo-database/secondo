%{
#include <cstdio>
#include <string>
#include <string.h>
#include "NestedText.h"

#define YYDEBUG 1

extern int yylex();

string* yacc_outtext;
char*   yacc_error;

typedef char cstring[50];
cstring param, param2, paramname, paramtype, paramtype2;
int paramno, depth;
cstring paramstack[20];

void yyerror( char* s )
{
  yacc_error = s;
  cerr << "Syntax error in textual command: " << s << endl;
}

void PRINTF (char *s)
{
  (*yacc_outtext).append( s );
}

void PRINT (int i)
{
  string temp;
  NestedText::CopyOut( i, temp );
  *yacc_outtext += temp;
}
%}

%token 	IDENTIFIER, FUN, INTEGER, REAL, STRING, BOOLEAN, CONST, TUPLE, GROUP,

	TYPE, DELETE, CREATE, UPDATE, QUERY, MODEL, LET, PERSISTENT,
	BEGIN1, TRANSACTION, COMMIT, ABORT, DATABASE, OPEN, CLOSE, SAVE, TO,
	RESTORE, FROM, LIST, DATABASES, CONSTRUCTORS, OPERATORS, TYPES,
	OBJECTS, ASSIGN, DOUBLE

, PLUS, MINUS, TIMES, DIVIDEDBY, DIV, MOD, GT, LT, LE, GE, NE, EQ, NOT, AND, OR, STARTS, CONTAINS, HEAD, MAX, MIN, AVG, SUM, COUNT, CONCAT, ATTR, PROJECT, FILTER, CANCEL, RDUP, SORT, EXTEND, GROUPBY, GFEED, PRODUCT, LOOPJOIN, MERGESEC, MERGEDIFF, SORTBY, MERGEJOIN, RENAME, EXTRACT, GETX, GETY, PCOUNT, PDIAMETER, LVERTICES, LCOUNT, LDIAMETER, LLENGTH, RVERTICES, RCONTOUR, LINTERIOR, RCOUNT, RDIAMETER, RAREA, RPERIMETER, PPEQUAL, PPDISJOINT, PPINTERSECTION, PPPLUS, PPMINUS, LLEQUAL, LLDISJOINT, LLINTERSECTS, LLMEETS, PLONBORDEROF, LLBORDERINCOMMON, LLINTERSECTION, LLPLUS, LLMINUS, LLCOMMONBORDER, RREQUAL, RRDISJOINT, PRINSIDE, LRINSIDE, RRINSIDE, RRAREADISJOINT, RREDGEDISJOINT, RREDGEINSIDE, RRVERTEXINSIDE, LRINTERSECTS, RRINTERSECTS, LRMEETS, RRMEETS, RRADJACENT, RRENCLOSES, PRONBORDEROF, LRBORDERINCOMMON, RRINTERSECTION, LRINTERSECTION, RRPLUS, RRMINUS, LRCOMMONBORDER
/*
May 15, 1998 RHG Added rule for the ~model~ command.

Feb 3, 1999 Stefan Dieker Added "typexpr : typeexpr ':' typeexpr" rule

August 10, 2000 RHG Changed definition of syntax for constant values. Now to be written as 

	'[' typeexpr 'const' valueexpr ']'

for example, 

----	[points const ((1 -4) (2 3))]
	[int const 954]
----

March 2002 Ulrich Telle Port to C++

*/

%%

/* commands */

commands	:	command
		|	commands ';' command
		;

command		:	basic
		|	transaction
		|	database
		|	inquiries
		;	

basic		:	TYPE IDENTIFIER '=' typeexpr
				{PRINTF("(type "); PRINT($2); 
				PRINTF(" = "); PRINT($4); PRINTF(")\n");}
		|	DELETE TYPE IDENTIFIER	
				{PRINTF("(delete type "); PRINT($3); 
				PRINTF(")\n");}
		|	CREATE IDENTIFIER ':' typeexpr
				{PRINTF("(create "); PRINT($2); 
				PRINTF(" : "); PRINT($4); PRINTF(")\n");}

		|		{paramno=0; depth =0;}
			UPDATE IDENTIFIER ASSIGN valueexpr
				{PRINTF("(update "); PRINT($3); 
				PRINTF(" := "); PRINT($5); PRINTF(")\n");}
		|	DELETE IDENTIFIER
				{PRINTF("(delete "); PRINT($2); 
				PRINTF(")\n");}

		|		{paramno=0; depth =0;}
			QUERY valueexpr
				{PRINTF("(query "); PRINT($3); 
				PRINTF(")\n");}

                |               {paramno=0; depth =0;}
                        MODEL valueexpr
                                {PRINTF("(model "); PRINT($3);
                                PRINTF(")\n");}
 
		|		{paramno=0; depth =0;}
			LET IDENTIFIER '=' valueexpr
				{PRINTF("(let "); PRINT($3); 
				PRINTF(" = "); PRINT($5); PRINTF(")\n");}
		|	PERSISTENT IDENTIFIER
				{PRINTF("(persistent "); PRINT($2); 
				PRINTF(")\n");}
		;

transaction	:	BEGIN1 TRANSACTION
				{PRINTF("(begin transaction)\n");}
		|	COMMIT TRANSACTION
				{PRINTF("(commit transaction)\n");}
		|	ABORT TRANSACTION
				{PRINTF("(abort transaction)\n");}
		;

database	:	CREATE DATABASE IDENTIFIER
				{PRINTF("(create database "); PRINT($3); 
				PRINTF(")\n");}
		|	DELETE DATABASE IDENTIFIER
				{PRINTF("(delete database "); PRINT($3); 
				PRINTF(")\n");}
		|	OPEN DATABASE IDENTIFIER
				{PRINTF("(open database "); PRINT($3); 
				PRINTF(")\n");}
		|	CLOSE DATABASE 
				{PRINTF("(close database)\n");}
		|	SAVE DATABASE TO IDENTIFIER
				{PRINTF("(save database to "); PRINT($4); 
				PRINTF(")\n");}
		|	RESTORE DATABASE IDENTIFIER FROM IDENTIFIER
				{PRINTF("(restore database "); PRINT($3); 
				PRINTF(" from "); PRINT($5); PRINTF(")\n");}
		;

inquiries	:	LIST DATABASES
				{PRINTF("(list databases)\n");}
		|	LIST TYPE CONSTRUCTORS
				{PRINTF("(list type constructors)\n");}
		|	LIST OPERATORS
				{PRINTF("(list operators)\n");}
		|	LIST TYPES
				{PRINTF("(list types)\n");}
		|	LIST OBJECTS
				{PRINTF("(list objects)\n");}
		;


/* typeexpr*/

typeexpr	:	constructor	{$$ = $1;}
		|	constructor '(' typeexprs ')'
				{$$ = NestedText::Concat(NestedText::AtomC("("),
					NestedText::Concat($1,
					  NestedText::Concat(NestedText::AtomC(" "),
					    NestedText::Concat($3, NestedText::AtomC(")")))));}

		|	'(' typeexprs ')'
				{$$ = NestedText::Concat(NestedText::AtomC("("),
					NestedText::Concat($2, NestedText::AtomC(")")));}

		|	'[' typeexprs ']'
				{$$ = NestedText::Concat(NestedText::AtomC("("),
					NestedText::Concat($2, NestedText::AtomC(")")));}

		|	'[' ']'		{$$ = NestedText::AtomC("()");}

		|	typeexpr ':' typeexpr	{$$ = NestedText::Concat(NestedText::AtomC("("),
						NestedText::Concat($1, NestedText::Concat(NestedText::AtomC(" "),
						NestedText::Concat($3,  NestedText::AtomC(")")))));}
		;

constructor	:	IDENTIFIER	{$$ = $1;}
		|	TUPLE		{$$ = $1;}
		|	GROUP		{$$ = $1;}
		;

typeexprs	:	typeexpr	{$$ = $1;}
		|	typeexprs ',' typeexpr	
					{$$ = NestedText::Concat($1, 
						NestedText::Concat(NestedText::AtomC(" "), $3));}
		;

/* valueexpr, general part */

namedfunction	: naming function
			{$$ = NestedText::Concat(NestedText::AtomC("("),
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "),
				NestedText::Concat($2,
					NestedText::AtomC(")")	))));}
		| function		{$$ = $1;}
		;

function	: FUN '(' args ')' valueexpr
				{$$ = NestedText::Concat(NestedText::AtomC("(fun "),
					NestedText::Concat($3,
 					  NestedText::Concat (NestedText::AtomC(" "),
  					    NestedText::Concat($5, NestedText::AtomC(")") ))));}
		;

arg		: IDENTIFIER ':' typeexpr
				{$$ = NestedText::Concat(NestedText::AtomC("("),
					NestedText::Concat($1,
					  NestedText::Concat(NestedText::AtomC(" "),
					    NestedText::Concat($3, NestedText::AtomC(")") ))));}
		;

args		: arg		{$$ = $1;}
		| args ',' arg	{$$ = NestedText::Concat($1, 
					      	NestedText::Concat(NestedText::AtomC(" "), $3));}
		;


attribute	: '.' IDENTIFIER
			{$$ = NestedText::Concat(NestedText::AtomC("(attr "),
				NestedText::Concat(NestedText::AtomC(param),
				NestedText::Concat(NestedText::AtomC(" "),
				NestedText::Concat($2,
					NestedText::AtomC(")")	))));}
		;

attribute2	: DOUBLE IDENTIFIER
			{$$ = NestedText::Concat(NestedText::AtomC("(attr "),
				NestedText::Concat(NestedText::AtomC(param2),
				NestedText::Concat(NestedText::AtomC(" "),
				NestedText::Concat($2,				
					NestedText::AtomC(")")	))));}
		;


constant	: INTEGER | REAL | STRING | BOOLEAN
		; 

nestedlist	: atom			{$$ = $1;}
		| '(' rest		{$$ = NestedText::Concat($1, $2);}
		;

rest		: ')'			{$$ = $1;}
		| atom rest		{$$ = NestedText::Concat($1, 
						NestedText::Concat(NestedText::AtomC(" "), $2));}
		| nestedlist rest	{$$ = NestedText::Concat($1, 
						NestedText::Concat(NestedText::AtomC(" "), $2));}
		;

atom		: constant		{$$ = $1;}
		| IDENTIFIER		{$$ = $1;}
		;

list		: elems			{$$ = NestedText::Concat(NestedText::AtomC("("), 
						NestedText::Concat($1,
						NestedText::AtomC(")")     ));}
		;

elems		: 			{$$ = NestedText::AtomC("");}
		| elem			{$$ = $1;}
		| elems ',' elem	{$$ = NestedText::Concat($1,
						NestedText::Concat(NestedText::AtomC(" "), $3));}
		;

elem		: valueexpr		{$$ = $1;}
		| valueexpr valueexpr	
			{$$ = NestedText::Concat(NestedText::AtomC("("),
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "),
				NestedText::Concat($2,				
					NestedText::AtomC(")")	))));}
		| namedfunction		{$$ = $1;}
		;

naming		: IDENTIFIER ':'	{$$ = $1;}
		;

		
valueexpr	: IDENTIFIER				{$$ = $1;}
		| constant				{$$ = $1;}
		| attribute				{$$ = $1;}
		| attribute2				{$$ = $1;}
		| '(' valueexpr ')'			{$$ = $2;}
		| '[' typeexpr CONST nestedlist ']'
			{$$ = NestedText::Concat(NestedText::AtomC("("),
				NestedText::Concat($2,
				NestedText::Concat(NestedText::AtomC(" "),
				NestedText::Concat($4,				
					NestedText::AtomC(")")	))));}
		| '[' list ']'				{$$ = $2;}

		| '.'					{$$ = NestedText::AtomC(param);}

		| TUPLE					{$$ = NestedText::AtomC(param);}

		| GROUP					{$$ = NestedText::AtomC(param);}

		| valueexpr '=' valueexpr
			{$$ = NestedText::Concat(NestedText::AtomC("(="), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )))));}

		| valueexpr '{' IDENTIFIER '}'
			{$$ = NestedText::Concat(NestedText::AtomC("(rename"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )))));}


/* algebra specific part */

		| valueexpr PLUS valueexpr
			{$$ = NestedText::Concat(NestedText::AtomC("(+"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )))));}
		| valueexpr MINUS valueexpr
			{$$ = NestedText::Concat(NestedText::AtomC("(-"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )))));}
		| valueexpr TIMES valueexpr
			{$$ = NestedText::Concat(NestedText::AtomC("(*"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )))));}
		| valueexpr DIVIDEDBY valueexpr
			{$$ = NestedText::Concat(NestedText::AtomC("(/"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )))));}
		| valueexpr DIV valueexpr
			{$$ = NestedText::Concat(NestedText::AtomC("(div"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )))));}
		| valueexpr MOD valueexpr
			{$$ = NestedText::Concat(NestedText::AtomC("(mod"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )))));}
		| valueexpr GT valueexpr
			{$$ = NestedText::Concat(NestedText::AtomC("(>"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )))));}
		| valueexpr LT valueexpr
			{$$ = NestedText::Concat(NestedText::AtomC("(<"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )))));}
		| valueexpr LE valueexpr
			{$$ = NestedText::Concat(NestedText::AtomC("(<="), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )))));}
		| valueexpr GE valueexpr
			{$$ = NestedText::Concat(NestedText::AtomC("(>="), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )))));}
		| valueexpr NE valueexpr
			{$$ = NestedText::Concat(NestedText::AtomC("(#"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )))));}
		| valueexpr EQ valueexpr
			{$$ = NestedText::Concat(NestedText::AtomC("(="), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )))));}
		| NOT '(' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(not"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )));}
		| valueexpr AND valueexpr
			{$$ = NestedText::Concat(NestedText::AtomC("(and"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )))));}
		| valueexpr OR valueexpr
			{$$ = NestedText::Concat(NestedText::AtomC("(or"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )))));}
		| valueexpr STARTS valueexpr
			{$$ = NestedText::Concat(NestedText::AtomC("(starts"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )))));}
		| valueexpr CONTAINS valueexpr
			{$$ = NestedText::Concat(NestedText::AtomC("(contains"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )))));}
		|  valueexpr HEAD '[' valueexpr ']'
			{$$ = NestedText::Concat(NestedText::AtomC("(head"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($4,
				NestedText::AtomC(")")     )))));}
		|  valueexpr MAX '[' valueexpr ']'
			{$$ = NestedText::Concat(NestedText::AtomC("(max"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($4,
				NestedText::AtomC(")")     )))));}
		|  valueexpr MIN '[' valueexpr ']'
			{$$ = NestedText::Concat(NestedText::AtomC("(min"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($4,
				NestedText::AtomC(")")     )))));}
		|  valueexpr AVG '[' valueexpr ']'
			{$$ = NestedText::Concat(NestedText::AtomC("(avg"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($4,
				NestedText::AtomC(")")     )))));}
		|  valueexpr SUM '[' valueexpr ']'
			{$$ = NestedText::Concat(NestedText::AtomC("(sum"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($4,
				NestedText::AtomC(")")     )))));}
		|  valueexpr COUNT
			{$$ = NestedText::Concat(NestedText::AtomC("(count"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::AtomC(")")     )));}
		|  valueexpr valueexpr CONCAT
			{$$ = NestedText::Concat(NestedText::AtomC("(concat"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($2,
				NestedText::AtomC(")")     )))));}
		| ATTR '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(attr"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		|  valueexpr PROJECT '[' list ']'
			{$$ = NestedText::Concat(NestedText::AtomC("(project"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($4,
				NestedText::AtomC(")")     )))));}
		|  valueexpr FILTER '[' filterfun ']'
			{$$ = NestedText::Concat(NestedText::AtomC("(filter"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($4,
				NestedText::AtomC(")")     )))));}
		|  valueexpr CANCEL '[' cancelfun ']'
			{$$ = NestedText::Concat(NestedText::AtomC("(cancel"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($4,
				NestedText::AtomC(")")     )))));}
		|  valueexpr RDUP
			{$$ = NestedText::Concat(NestedText::AtomC("(rdup"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::AtomC(")")     )));}
		|  valueexpr SORT
			{$$ = NestedText::Concat(NestedText::AtomC("(sort"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::AtomC(")")     )));}
		|  valueexpr EXTEND '[' extendfunlist ']'
			{$$ = NestedText::Concat(NestedText::AtomC("(extend"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($4,
				NestedText::AtomC(")")     )))));}
		|  valueexpr GROUPBY '[' list ';' groupbyfunlist ']'
			{$$ = NestedText::Concat(NestedText::AtomC("(groupby"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($4,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($6,
				NestedText::AtomC(")")     )))))));}
		|  valueexpr GFEED
			{$$ = NestedText::Concat(NestedText::AtomC("(gfeed"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::AtomC(")")     )));}
		|  valueexpr valueexpr PRODUCT
			{$$ = NestedText::Concat(NestedText::AtomC("(product"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($2,
				NestedText::AtomC(")")     )))));}
		|  valueexpr valueexpr LOOPJOIN '[' loopjoinfun ']'
			{$$ = NestedText::Concat(NestedText::AtomC("(loopjoin"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($2,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))))));}
		|  valueexpr valueexpr MERGESEC
			{$$ = NestedText::Concat(NestedText::AtomC("(mergesec"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($2,
				NestedText::AtomC(")")     )))));}
		|  valueexpr valueexpr MERGEDIFF
			{$$ = NestedText::Concat(NestedText::AtomC("(mergediff"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($2,
				NestedText::AtomC(")")     )))));}
		|  valueexpr SORTBY '[' list ']'
			{$$ = NestedText::Concat(NestedText::AtomC("(sortby"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($4,
				NestedText::AtomC(")")     )))));}
		|  valueexpr valueexpr MERGEJOIN '[' valueexpr ',' valueexpr ',' valueexpr ']'
			{$$ = NestedText::Concat(NestedText::AtomC("(mergejoin"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($2,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($7,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($9,
				NestedText::AtomC(")")     )))))))))));}
		|  valueexpr RENAME '[' valueexpr ']'
			{$$ = NestedText::Concat(NestedText::AtomC("(rename"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($4,
				NestedText::AtomC(")")     )))));}
		|  valueexpr EXTRACT '[' valueexpr ']'
			{$$ = NestedText::Concat(NestedText::AtomC("(extract"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($4,
				NestedText::AtomC(")")     )))));}
		| GETX '(' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_get_x"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )));}
		| GETY '(' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_get_y"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )));}
		| PCOUNT '(' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_p_count"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )));}
		| PDIAMETER '(' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_p_diameter"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )));}
		| LVERTICES '(' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_l_vertices"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )));}
		| LCOUNT '(' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_l_count"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )));}
		| LDIAMETER '(' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_l_diameter"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )));}
		| LLENGTH '(' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_l_length"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )));}
		| RVERTICES '(' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_r_vertices"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )));}
		| RCONTOUR '(' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_r_contour"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )));}
		| LINTERIOR '(' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_l_interior"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )));}
		| RCOUNT '(' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_r_count"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )));}
		| RDIAMETER '(' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_r_diameter"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )));}
		| RAREA '(' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_r_area"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )));}
		| RPERIMETER '(' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_r_perimeter"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )));}
		| PPEQUAL '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_pp_equal"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| PPDISJOINT '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_pp_disjoint"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| PPINTERSECTION '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_pp_intersection"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| PPPLUS '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_pp_plus"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| PPMINUS '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_pp_minus"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| LLEQUAL '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_ll_equal"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| LLDISJOINT '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_ll_disjoint"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| LLINTERSECTS '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_ll_intersects"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| LLMEETS '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_ll_meets"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| PLONBORDEROF '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_pl_on_border_of"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| LLBORDERINCOMMON '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_ll_border_in_common"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| LLINTERSECTION '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_ll_intersection"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| LLPLUS '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_ll_plus"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| LLMINUS '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_ll_minus"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| LLCOMMONBORDER '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_ll_common_border"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| RREQUAL '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_rr_equal"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| RRDISJOINT '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_rr_disjoint"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| PRINSIDE '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_pr_inside"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| LRINSIDE '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_lr_inside"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| RRINSIDE '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_rr_inside"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| RRAREADISJOINT '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_rr_area_disjoint"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| RREDGEDISJOINT '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_rr_edge_disjoint"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| RREDGEINSIDE '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_rr_edge_inside"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| RRVERTEXINSIDE '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_rr_vertex_inside"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| LRINTERSECTS '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_lr_intersects"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| RRINTERSECTS '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_rr_intersects"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| LRMEETS '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_lr_meets"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| RRMEETS '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_rr_meets"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| RRADJACENT '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_rr_adjacent"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| RRENCLOSES '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_rr_encloses"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| PRONBORDEROF '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_pr_on_border_of"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| LRBORDERINCOMMON '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_lr_border_in_common"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| RRINTERSECTION '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_rr_intersection"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| LRINTERSECTION '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_lr_intersection"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| RRPLUS '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_rr_plus"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| RRMINUS '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_rr_minus"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		| LRCOMMONBORDER '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(sd_lr_common_border"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		;

filterfun	: 	{paramno++;
			strcpy(paramname, "tuple");
			sprintf(param, "%s%d", paramname, paramno);
			strcpy(paramstack[depth], param); depth++;}
		  valueexpr
			{depth--; strcpy(param, paramstack[depth]);
			strcpy(paramtype, "TUPLE");
			$$ = NestedText::Concat(NestedText::AtomC("(fun ("),
				NestedText::Concat(NestedText::AtomC(param),
				NestedText::Concat(NestedText::AtomC(" "),
				NestedText::Concat(NestedText::AtomC(paramtype),
				NestedText::Concat(NestedText::AtomC(") "),
				NestedText::Concat($2,
					NestedText::AtomC(")")	))))));}
		| function
		        {$$ = $1;}
		;

cancelfun	: 	{paramno++;
			strcpy(paramname, "tuple");
			sprintf(param, "%s%d", paramname, paramno);
			strcpy(paramstack[depth], param); depth++;}
		  valueexpr
			{depth--; strcpy(param, paramstack[depth]);
			strcpy(paramtype, "TUPLE");
			$$ = NestedText::Concat(NestedText::AtomC("(fun ("),
				NestedText::Concat(NestedText::AtomC(param),
				NestedText::Concat(NestedText::AtomC(" "),
				NestedText::Concat(NestedText::AtomC(paramtype),
				NestedText::Concat(NestedText::AtomC(") "),
				NestedText::Concat($2,
					NestedText::AtomC(")")	))))));}
		| function
		        {$$ = $1;}
		;

extendfunlist	: extendfuns
			{$$ = NestedText::Concat(NestedText::AtomC("("), 
				NestedText::Concat($1,
				NestedText::AtomC(")")     ));}
		;

extendfuns	: 			{$$ = NestedText::AtomC("");}
		| extendfun		{$$ = $1;}
		| extendfuns ',' extendfun	
					{$$ = NestedText::Concat($1,
					NestedText::Concat(NestedText::AtomC(" "), $3));}
		;

extendfun	: naming extendsimfun
			{$$ = NestedText::Concat(NestedText::AtomC("("),
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "),
				NestedText::Concat($2,
					NestedText::AtomC(")")	))));}
		| extendsimfun		{$$ = $1;}
		;

extendsimfun	: 	{paramno++;
			strcpy(paramname, "tuple");
			sprintf(param, "%s%d", paramname, paramno);
			strcpy(paramstack[depth], param); depth++;}
		  valueexpr
			{depth--; strcpy(param, paramstack[depth]);
			strcpy(paramtype, "TUPLE");
			$$ = NestedText::Concat(NestedText::AtomC("(fun ("),
				NestedText::Concat(NestedText::AtomC(param),
				NestedText::Concat(NestedText::AtomC(" "),
				NestedText::Concat(NestedText::AtomC(paramtype),
				NestedText::Concat(NestedText::AtomC(") "),
				NestedText::Concat($2,
					NestedText::AtomC(")")	))))));}
		| function
		        {$$ = $1;}
		;

groupbyfunlist	: groupbyfuns
			{$$ = NestedText::Concat(NestedText::AtomC("("), 
				NestedText::Concat($1,
				NestedText::AtomC(")")     ));}
		;

groupbyfuns	: 			{$$ = NestedText::AtomC("");}
		| groupbyfun		{$$ = $1;}
		| groupbyfuns ',' groupbyfun	
					{$$ = NestedText::Concat($1,
					NestedText::Concat(NestedText::AtomC(" "), $3));}
		;

groupbyfun	: naming groupbysimfun
			{$$ = NestedText::Concat(NestedText::AtomC("("),
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "),
				NestedText::Concat($2,
					NestedText::AtomC(")")	))));}
		| groupbysimfun		{$$ = $1;}
		;

groupbysimfun	: 	{paramno++;
			strcpy(paramname, "group");
			sprintf(param, "%s%d", paramname, paramno);
			strcpy(paramstack[depth], param); depth++;}
		  valueexpr
			{depth--; strcpy(param, paramstack[depth]);
			strcpy(paramtype, "TUPLE");
			$$ = NestedText::Concat(NestedText::AtomC("(fun ("),
				NestedText::Concat(NestedText::AtomC(param),
				NestedText::Concat(NestedText::AtomC(" "),
				NestedText::Concat(NestedText::AtomC(paramtype),
				NestedText::Concat(NestedText::AtomC(") "),
				NestedText::Concat($2,
					NestedText::AtomC(")")	))))));}
		| function
		        {$$ = $1;}
		;

loopjoinfun	: 	{paramno++;
			strcpy(paramname, "lefttuple");
			sprintf(param, "%s%d", paramname, paramno);
			strcpy(paramstack[depth], param); depth++;
			paramno++;
			strcpy(paramname, "righttuple");
			sprintf(param2, "%s%d", paramname, paramno);
			strcpy(paramstack[depth], param2); depth++;}
		  valueexpr
			{depth--; strcpy(param2, paramstack[depth]);
			strcpy(paramtype2, "TUPLE2");
			depth--; strcpy(param, paramstack[depth]);
			strcpy(paramtype, "TUPLE");
			$$ = NestedText::Concat(NestedText::AtomC("(fun ("),
				NestedText::Concat(NestedText::AtomC(param),
				NestedText::Concat(NestedText::AtomC(" "),
				NestedText::Concat(NestedText::AtomC(paramtype),
				NestedText::Concat(NestedText::AtomC(") ("),
				NestedText::Concat(NestedText::AtomC(param2),
				NestedText::Concat(NestedText::AtomC(" "),
				NestedText::Concat(NestedText::AtomC(paramtype2),
				NestedText::Concat(NestedText::AtomC(") "),
				NestedText::Concat($2,
					NestedText::AtomC(")")	))))))))));}
		| function
		        {$$ = $1;}
		;


%%

