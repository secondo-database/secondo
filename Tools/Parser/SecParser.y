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
	OBJECTS, ASSIGN, DOUBLE, EQ

, PLUS, MINUS, TIMES, DIVIDEDBY, DIV, MOD, GT, LT, LE, GE, NE, EQ, NOT, AND, OR, STARTS, CONTAINS, HEAD, MAX, MIN, AVG, SUM, COUNT, CONCAT, ATTR, PROJECT, FILTER, CANCEL, RDUP, SORT, EXTEND, GROUPBY, GFEED, PRODUCT, LOOPJOIN, MERGESEC, MERGEDIFF, SORTBY, MERGEJOIN, RENAME, EXTRACT, INTERSECTS, INSIDE, INTSTREAM, COUNT, PRINTINTSTREAM
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

basic		:	TYPE IDENTIFIER EQ typeexpr
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
			LET IDENTIFIER EQ valueexpr
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

		| valueexpr EQ valueexpr
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
		| valueexpr INTERSECTS valueexpr
			{$$ = NestedText::Concat(NestedText::AtomC("(intersects"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )))));}
		| valueexpr INSIDE valueexpr
			{$$ = NestedText::Concat(NestedText::AtomC("(inside"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::AtomC(")")     )))));}
		| INTSTREAM '(' valueexpr ',' valueexpr ')'
			{$$ = NestedText::Concat(NestedText::AtomC("(intstream"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($3,
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($5,
				NestedText::AtomC(")")     )))));}
		|  valueexpr COUNT
			{$$ = NestedText::Concat(NestedText::AtomC("(count"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::AtomC(")")     )));}
		|  valueexpr PRINTINTSTREAM
			{$$ = NestedText::Concat(NestedText::AtomC("(printintstream"), 
				NestedText::Concat(NestedText::AtomC(" "), 
				NestedText::Concat($1,
				NestedText::AtomC(")")     )));}
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

