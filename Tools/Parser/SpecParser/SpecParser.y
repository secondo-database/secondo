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

August 10, 2000 RHG Increased lengths of all symbols (and in copyout) to 40.

March 2002 Ulrich Telle Port to C++, new version of NestedText

February 2003 Ulrich Telle, adjusted for GNU C++ version 3.2

May 2, 2003 RHG Prefixed ``ZZ'' to each operator token to avoid conflicts 
under Windows.

*/

%{
#include <string>
#include <stdio.h>
#include "NestedText.h"
#define ZZTRUE 1;
#define ZZFALSE 0;

#define YYERROR_VERBOSE

extern int yylex();

string operator1, token, parameter, type, parameter2, type2;
FILE *lexrules, *tokens, *yaccrules1, *yaccrules2;
int i, n, noargs, pos[10];		/* i number of argument, n position of 
					argument in pattern, pos[i] the 
					position of the i-th argument in the
					pattern */
int issymbol;
int hasfunction, hasfunctionlist;

void yyerror( char* s )
{
  cerr << endl << s << endl << endl;
}
%}

%token 	ZZIDENTIFIER, ZZSYMBOL, ZZOPERATOR, ZZPATTERN, ZZFUN, ZZOP, ZZINFIXOP, ZZALIAS, ZZLIST,
	ZZIMPLICIT, ZZPARAMETER, ZZPARAMETERS, ZZTYPE, ZZTYPES, ZZFUNLIST, ZZEMPTY, ZZCOMMENT
	

%%

specfile	:               {lexrules = fopen("lexrules", "w");
				tokens = fopen("tokens", "w");
				yaccrules1 = fopen("yaccrules1", "w");
				yaccrules2 = fopen("yaccrules2", "w");}
		  specs
				{fclose(lexrules);
				fprintf(tokens, "\n");
				fclose(tokens);
				fprintf(yaccrules1, "\t\t;\n\n");
				fclose(yaccrules1);
				fclose(yaccrules2);}
		|
		;

specs		: spec
		| ZZCOMMENT
		| specs spec
		| specs ZZCOMMENT
		;


spec		: ZZOPERATOR name ZZALIAS ZZIDENTIFIER ZZPATTERN 
			{hasfunction = ZZFALSE; hasfunctionlist = ZZFALSE;
			NestedText::CopyOut($2, operator1);
			NestedText::CopyOut($4, token); token = "ZZ" + token;
			if (issymbol)
			  fprintf(lexrules, "\"%s\"\t\t{return %s;}\n", 
			    operator1.c_str(), token.c_str());
			else
			  fprintf(lexrules, "%s\t\t{return %s;}\n", 
			  operator1.c_str(), token.c_str());

			fprintf(tokens, ", %s", token.c_str());

			fprintf(yaccrules1, "\t\t| "); i=0; n=0;}
		  pattern
			{noargs = i;
			fprintf(yaccrules1, "\n");

			/* write action */
			fprintf(yaccrules1, 
			  "\t\t\t{$$ = NestedText::Concat(NestedText::AtomC(\"(%s\"), \n", operator1.c_str());

			for (i=1; i<=noargs; i++) {
			  fprintf(yaccrules1, "\t\t\t\tNestedText::Concat(NestedText::AtomC(\" \"), \n");
			  fprintf(yaccrules1, "\t\t\t\tNestedText::Concat($%d,\n", pos[i]);}
			fprintf(yaccrules1, "\t\t\t\tNestedText::AtomC(\")\")     ");
			for (i=1; i<=noargs; i++) {
			  fprintf(yaccrules1, "))");}
			fprintf(yaccrules1, ");}\n");
			}
		  implicit
		;

name		: ZZIDENTIFIER	{$$ = $1; issymbol = ZZFALSE;}
		| ZZSYMBOL	{$$ = $1; issymbol = ZZTRUE;}
		;


pattern		: prefix
		| infix
		| postfix
		;

prefix		: ZZOP '(' 			
			{fprintf(yaccrules1, "%s '('", token.c_str());
			n++; n++;}

		  simpleargscomma ')'
			{fprintf(yaccrules1, " ')'"/*, token.c_str()*/);}

		| ZZOP '_'
			{fprintf(yaccrules1, "%s valueexpr", token.c_str());
			i = 1; pos[1]=2;}
		;

infix		: '_' ZZINFIXOP '_'	
			{fprintf(yaccrules1, "valueexpr %s valueexpr", token.c_str());
			i = 2; pos[1]=1; pos[2]=3;}
		;

postfix		: simpleargsblank ZZOP		
			{fprintf(yaccrules1, " %s", token.c_str());}

		| simpleargsblank ZZOP '[' 	
			{fprintf(yaccrules1, " %s '['", token.c_str());
			n++; n++;}

		  arguments ']'	
			{fprintf(yaccrules1, " ']'");}
		;

simpleargscomma	: '_'
			{fprintf(yaccrules1, " valueexpr");
			i++; n++; pos[i]=n;}

		| simpleargscomma ',' '_'
			{fprintf(yaccrules1, " ',' valueexpr");
			i++; n++; n++; pos[i]=n;}
		;

simpleargsblank	: '_'
			{fprintf(yaccrules1, " valueexpr");
			i++; n++; pos[i]=n;}

		| simpleargsblank '_'
			{fprintf(yaccrules1, " valueexpr");
			i++; n++; pos[i]=n;}
		;

arguments	: sublist
		| arguments ';' 
			{fprintf(yaccrules1, " ';'");
			n++;}

		  sublist
		;

sublist		: argscomma
		| ZZLIST
			{fprintf(yaccrules1, " list");
			i++; n++; pos[i]=n;}
		| ZZFUNLIST
			{fprintf(yaccrules1, " %sfunlist", operator1.c_str());
			i++; n++; pos[i]=n; hasfunctionlist = ZZTRUE;}
		;

argscomma	: arg
		| argscomma ',' 
			{fprintf(yaccrules1, " ','");
			n++;}
		  arg
		;

arg		: '_'
			{fprintf(yaccrules1, " valueexpr");
			i++; n++; pos[i]=n;}

		| ZZFUN
			{fprintf(yaccrules1, " %sfun", operator1.c_str());
			i++; n++; pos[i]=n; hasfunction = ZZTRUE;}
		;



implicit	: ZZIMPLICIT rest

		|
			{if (hasfunction) {

fprintf(yaccrules2, "%sfun\t: function\n"		, operator1.c_str());
fprintf(yaccrules2,      "\t\t        {$$ = $1;}\n"	);
fprintf(yaccrules2,      "\t\t;\n\n"			);

			}
			if (hasfunctionlist) {

fprintf(yaccrules2, 	"%sfunlist\t: list\n"		, operator1.c_str());
fprintf(yaccrules2,      "\t\t        {$$ = $1;}\n"	);
fprintf(yaccrules2,      "\t\t;\n\n"			);

			}}

		;

/* 
Form of the rule generated for the predicate of the filter operator1, for example:  

----
filterfun	: 	{paramno++;
			strcpy(paramname, "tup");
			sprintf(param, "%s%d", paramname, paramno);
			strcpy(paramstack[depth], param); depth++;}
		  valueexpr
			{depth--; strcpy(param, paramstack[depth]);
			strcpy(paramtype, "tuple1");
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
----

*/

rest		: ZZPARAMETER ZZIDENTIFIER ZZTYPE ZZIDENTIFIER
			{NestedText::CopyOut($2, parameter);
			NestedText::CopyOut($4, type);
			if (hasfunction) {

fprintf(yaccrules2, "%sfun\t: \t{paramno++;\n", operator1.c_str());
fprintf(yaccrules2, "\t\t\tstrcpy(paramname, \"%s\");\n", parameter.c_str());
fprintf(yaccrules2, "\t\t\tsprintf(param, \"%%s%%d\", paramname, paramno);\n");
fprintf(yaccrules2, "\t\t\tstrcpy(paramstack[depth], param); depth++;}\n");

fprintf(yaccrules2, "\t\t  valueexpr\n");

fprintf(yaccrules2, "\t\t\t{depth--; strcpy(param, paramstack[depth]);\n");
fprintf(yaccrules2, "\t\t\tstrcpy(paramtype, \"%s\");\n", type.c_str());
fprintf(yaccrules2, "\t\t\t$$ = NestedText::Concat(NestedText::AtomC(\"(fun (\"),\n");
fprintf(yaccrules2, "\t\t\t\tNestedText::Concat(NestedText::AtomC(param),\n");
fprintf(yaccrules2, "\t\t\t\tNestedText::Concat(NestedText::AtomC(\" \"),\n");
fprintf(yaccrules2, "\t\t\t\tNestedText::Concat(NestedText::AtomC(paramtype),\n");
fprintf(yaccrules2, "\t\t\t\tNestedText::Concat(NestedText::AtomC(\") \"),\n");
fprintf(yaccrules2, "\t\t\t\tNestedText::Concat($2,\n");
fprintf(yaccrules2, "\t\t\t\t	NestedText::AtomC(\")\")	))))));}\n");
fprintf(yaccrules2, "\t\t| function\n");
fprintf(yaccrules2, "\t\t        {$$ = $1;}\n");
fprintf(yaccrules2, "\t\t;\n\n");

			}
			if (hasfunctionlist) {

/*
groupbyfunlist	: groupbyfuns		{$$ = NestedText::Concat(NestedText::AtomC("("), 
						NestedText::Concat($1,
						NestedText::AtomC(")")     ));}
		;

*/

fprintf(yaccrules2, "%sfunlist\t: %sfuns\n", operator1.c_str(), operator1.c_str());
fprintf(yaccrules2, "\t\t\t{$$ = NestedText::Concat(NestedText::AtomC(\"(\"), \n");
fprintf(yaccrules2, "\t\t\t\tNestedText::Concat($1,\n");
fprintf(yaccrules2, "\t\t\t\tNestedText::AtomC(\")\")     ));}\n");
fprintf(yaccrules2, "\t\t;\n\n");

/*
groupbyfuns	: 			{$$ = NestedText::AtomC("");}
		| groupbyfun		{$$ = $1;}
		| groupbyfuns ',' groupbyfun	
					{$$ = NestedText::Concat($1,
						NestedText::Concat(NestedText::AtomC(" "), $3));}
		;

*/

fprintf(yaccrules2, "%sfuns\t: \t\t\t{$$ = NestedText::AtomC(\"\");}\n", operator1.c_str());
fprintf(yaccrules2, "\t\t| %sfun\t\t{$$ = $1;}\n", operator1.c_str());
fprintf(yaccrules2, "\t\t| %sfuns ',' %sfun	\n", operator1.c_str(), operator1.c_str());
fprintf(yaccrules2, "\t\t\t\t\t{$$ = NestedText::Concat($1,\n");
fprintf(yaccrules2, "\t\t\t\t\tNestedText::Concat(NestedText::AtomC(\" \"), $3));}\n");
fprintf(yaccrules2, "\t\t;\n\n");

/*
groupbyfun	: naming groupbysimfun
			{$$ = NestedText::Concat(NestedText::AtomC("("),
				NestedText::Concat($1,
				NestedText::Concat(NestedText::AtomC(" "),
				NestedText::Concat($2,			
					NestedText::AtomC(")")	))));}
		| groupbysimfun		{$$ = $1;}
		;

*/

fprintf(yaccrules2, "%sfun\t: naming %ssimfun\n", operator1.c_str(), operator1.c_str());
fprintf(yaccrules2, "\t\t\t{$$ = NestedText::Concat(NestedText::AtomC(\"(\"),\n");
fprintf(yaccrules2, "\t\t\t\tNestedText::Concat($1,\n");
fprintf(yaccrules2, "\t\t\t\tNestedText::Concat(NestedText::AtomC(\" \"),\n");
fprintf(yaccrules2, "\t\t\t\tNestedText::Concat($2,\n");
fprintf(yaccrules2, "\t\t\t\t\tNestedText::AtomC(\")\")	))));}\n");
fprintf(yaccrules2, "\t\t| %ssimfun\t\t{$$ = $1;}\n", operator1.c_str());
fprintf(yaccrules2, "\t\t;\n\n");





/*
groupbysimfun	: 	{paramno++;
			strcpy(paramname, "group");
			sprintf(param, "%s%d", paramname, paramno);
			strcpy(paramstack[depth], param); depth++;}
		  valueexpr
			{depth--; strcpy(param, paramstack[depth]);
			strcpy(paramtype, "GROUP");

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

*/

fprintf(yaccrules2, "%ssimfun\t: \t{paramno++;\n", operator1.c_str());
fprintf(yaccrules2, "\t\t\tstrcpy(paramname, \"%s\");\n", parameter.c_str());
fprintf(yaccrules2, "\t\t\tsprintf(param, \"%%s%%d\", paramname, paramno);\n");
fprintf(yaccrules2, "\t\t\tstrcpy(paramstack[depth], param); depth++;}\n");

fprintf(yaccrules2, "\t\t  valueexpr\n");

fprintf(yaccrules2, "\t\t\t{depth--; strcpy(param, paramstack[depth]);\n");
fprintf(yaccrules2, "\t\t\tstrcpy(paramtype, \"%s\");\n", type.c_str());
fprintf(yaccrules2, "\t\t\t$$ = NestedText::Concat(NestedText::AtomC(\"(fun (\"),\n");
fprintf(yaccrules2, "\t\t\t\tNestedText::Concat(NestedText::AtomC(param),\n");
fprintf(yaccrules2, "\t\t\t\tNestedText::Concat(NestedText::AtomC(\" \"),\n");
fprintf(yaccrules2, "\t\t\t\tNestedText::Concat(NestedText::AtomC(paramtype),\n");
fprintf(yaccrules2, "\t\t\t\tNestedText::Concat(NestedText::AtomC(\") \"),\n");
fprintf(yaccrules2, "\t\t\t\tNestedText::Concat($2,\n");
fprintf(yaccrules2, "\t\t\t\t	NestedText::AtomC(\")\")	))))));}\n");
fprintf(yaccrules2, "\t\t| function\n");
fprintf(yaccrules2, "\t\t        {$$ = $1;}\n");
fprintf(yaccrules2, "\t\t;\n\n");

			}}


/*
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
*/



		| ZZPARAMETERS ZZIDENTIFIER ',' ZZIDENTIFIER ZZTYPES ZZIDENTIFIER ',' ZZIDENTIFIER
			{NestedText::CopyOut($2, parameter); 
			NestedText::CopyOut($4, parameter2);
			NestedText::CopyOut($6, type);
			NestedText::CopyOut($8, type2);

			if (hasfunction) {

fprintf(yaccrules2, "%sfun\t: \t{paramno++;\n", operator1.c_str());
fprintf(yaccrules2, "\t\t\tstrcpy(paramname, \"%s\");\n", parameter.c_str());
fprintf(yaccrules2, "\t\t\tsprintf(param, \"%%s%%d\", paramname, paramno);\n");
fprintf(yaccrules2, "\t\t\tstrcpy(paramstack[depth], param); depth++;\n");

fprintf(yaccrules2, "\t\t\tparamno++;\n");
fprintf(yaccrules2, "\t\t\tstrcpy(paramname, \"%s\");\n", parameter2.c_str());
fprintf(yaccrules2, "\t\t\tsprintf(param2, \"%%s%%d\", paramname, paramno);\n");
fprintf(yaccrules2, "\t\t\tstrcpy(paramstack[depth], param2); depth++;}\n");
fprintf(yaccrules2, "\t\t  valueexpr\n");
fprintf(yaccrules2, "\t\t\t{depth--; strcpy(param2, paramstack[depth]);\n");
fprintf(yaccrules2, "\t\t\tstrcpy(paramtype2, \"%s\");\n", type2.c_str());

fprintf(yaccrules2, "\t\t\tdepth--; strcpy(param, paramstack[depth]);\n");
fprintf(yaccrules2, "\t\t\tstrcpy(paramtype, \"%s\");\n", type.c_str());
fprintf(yaccrules2, "\t\t\t$$ = NestedText::Concat(NestedText::AtomC(\"(fun (\"),\n");
fprintf(yaccrules2, "\t\t\t\tNestedText::Concat(NestedText::AtomC(param),\n");
fprintf(yaccrules2, "\t\t\t\tNestedText::Concat(NestedText::AtomC(\" \"),\n");
fprintf(yaccrules2, "\t\t\t\tNestedText::Concat(NestedText::AtomC(paramtype),\n");
fprintf(yaccrules2, "\t\t\t\tNestedText::Concat(NestedText::AtomC(\") (\"),\n");

fprintf(yaccrules2, "\t\t\t\tNestedText::Concat(NestedText::AtomC(param2),\n");
fprintf(yaccrules2, "\t\t\t\tNestedText::Concat(NestedText::AtomC(\" \"),\n");
fprintf(yaccrules2, "\t\t\t\tNestedText::Concat(NestedText::AtomC(paramtype2),\n");
fprintf(yaccrules2, "\t\t\t\tNestedText::Concat(NestedText::AtomC(\") \"),\n");

fprintf(yaccrules2, "\t\t\t\tNestedText::Concat($2,\n");
fprintf(yaccrules2, "\t\t\t\t	NestedText::AtomC(\")\")	))))))))));}\n");
fprintf(yaccrules2, "\t\t| function\n");
fprintf(yaccrules2, "\t\t        {$$ = $1;}\n");
fprintf(yaccrules2, "\t\t;\n\n");

			}}

		;

%%

