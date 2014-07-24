/*
----  /Tools/TypeMap/OpSpecParser/OpSpecParser.y
---- 

---- 
This file is part of SECONDO.

Copyright (C) 2014, 
University in Hagen, 
Faculty of Mathematics and Computer Science, 
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


/*
Includes

*/
#include "OpSpecParser.tab.h"

#include <iostream>
#include <string>
#include <fstream>

#define YYDEBUG 1
#define YYERROR_VERBOSE 1

using namespace std;

/*
Some variables 

*/
extern FILE* opspecin;
const char* infile = "../Algebras/specs";
string outfile = "../Tools/TypeMap/OpSpecParser/OpSpecs.tmp";
ofstream ofile;

extern int opspeclex();


void opspecerror( const char* s ) {
cerr << endl << s << endl << endl;
}

/*
Type string is not possible in union

*/
string collectSpec      = "";
string collectPattern   = "";
string collectPrefix    = "";
string collectInfix     = "";
string collectPostfix   = "";
string collectArguments = "";
string collectSublist   = "";
string collectArgscomma = "";
string collectArg       = "";

%}

%union{
  const char* text;
  int   len;
}

/*
 use opspec instead of yy to avoid naming conflicts with other 
 parsers of the system

*/

%name-prefix="opspec"

%token   ZZOPERATOR ZZPATTERN ZZFUN ZZOP ZZINFIXOP  ZZLIST
         ZZIMPLICIT ZZPARAMETER ZZPARAMETERS ZZTYPE ZZTYPES ZZFUNLIST 
         ZZCOMMENT ZZFORCEBUFFER 

%token<text> ZZIDENTIFIER  ZZSYMBOL ZZALIAS

%type<text> name
%type<len> argscomma simpleargscomma simpleargsblank

%%

start	  : specs
          |
          ;

specs     : spec
	    {
	      ofile << collectSpec;
	    }

          | ZZCOMMENT
          | specs spec
	    {
	      ofile << collectSpec;
	    }
          | specs ZZCOMMENT
          ;

spec      : ZZOPERATOR name ZZALIAS ZZIDENTIFIER ZZPATTERN pattern
						 implicit bufferforced
            { 
	      string str2 = $2;
	      collectSpec = "";
	      if (str2 == "rename") {
		collectPattern = "\"o {p}\"";
	      }
	      collectSpec  += "(" + str2 + " " + collectPattern + ")\n";
	    }
          ;

name      : ZZIDENTIFIER  
            { $$ = $1; }
          | ZZSYMBOL
            { $$ = $1; }  
          ;


pattern   : prefix
	    {
	      collectPattern = collectPrefix;
	    }
          | infix
	    {
	      collectPattern = collectInfix;
	    } 
          | postfix
	    {
              collectPattern = collectPostfix;
            }
          ;

infix     : '_' ZZINFIXOP '_'
            {  
	      collectInfix = "";
	      collectInfix += "\"(o # o)\"";
            }
          ;

prefix    : ZZOP '('simpleargscomma')'    
            { 
	      collectPrefix = "";
	      collectPrefix += "\"#(o";
	      for(int i=1;i<$3;i++) {
		collectPrefix += "; o";
	      }
	      collectPrefix += ")\"";
            }
          | ZZOP '(' ')'
            { 
	      collectPrefix = "";
	      collectPrefix += "\"#(o)\"";
            }
          ;

postfix   : simpleargsblank ZZOP
	    {
	      collectPostfix = "";
	      collectPostfix += "\"o";
	      for(int i=1;i<$1;i++) {
		collectPostfix += " o";
	      }
	      collectPostfix += " #\"";
	    }    
          | simpleargsblank ZZOP '[' arguments ']'
	    { 
	      collectPostfix = "";
	      collectPostfix += "\"o";
	      for(int i=1;i<$1;i++) {
		collectPostfix += " o";
	      }
	      collectPostfix += " #[" + collectArguments + "]\"";
	    }    
          ;


simpleargscomma	  : '_'
		    { $$ = 1; }
		  | simpleargscomma ',' '_'
		    { $$ = $1 + 1; }
		  ;

simpleargsblank	  : '_'
		    { $$ = 1; }
                  | simpleargsblank '_'
	            { $$ = $1 + 1; }
		  ;

arguments   : sublist
	      {
		collectArguments = "";
		collectArguments += collectSublist;
	      }
	    | arguments ';' sublist
	      {
		collectArguments += "; " + collectSublist;
	      }
	    ;

sublist	    : argscomma
	      {
		collectSublist = "";
		collectSublist += collectArgscomma;
	      }
	    | ZZLIST
	      {	      } 
	    | ZZFUNLIST
	      {	      }
	    ;

argscomma   : arg
	      { $$ = 1;
		collectArgscomma = "";
		collectArgscomma += collectArg;
	      }
	    | argscomma ',' arg
	      { $$ = $1 + 1;
		collectArgscomma += "; " + collectArg;
	      }
	    ;

arg	    : '_'
	      {	 
		collectArg = "";
		collectArg += "p";
	      }
	    | ZZFUN
	      {	     }
	    ;

implicit      : ZZIMPLICIT rest
	      |
	      ;

rest	      : ZZPARAMETER parameterlist ZZTYPE typelist
	      | ZZPARAMETERS parameterlist ZZTYPES typelist
	      ;

bufferforced  : ZZFORCEBUFFER 
	      |	{ }
              ; 

parameterlist : ZZIDENTIFIER
                {  
		 /* cout << (string($1)) << endl;*/
                }
              | parameterlist ',' ZZIDENTIFIER
                { 
		 /* cout << (string($3)) << endl;*/
                }
              ;
typelist      : ZZIDENTIFIER
                {
		 /* cout << (string($1)) << endl;*/
                }
	      | typelist ',' ZZIDENTIFIER
                { 
		 /* cout << (string($3)) << endl;*/
                }
	      ;
%%


int main(int argc, char** argv) {
  FILE* ifile;
  if(argc > 1){
    ifile = fopen(argv[1], "r");
    if (ifile == NULL) {
      fprintf(stderr,"ERROR: cannot open file %s\n",argv[1]);
      fprintf(stderr,"%s",argv[1]);
      fprintf(stderr,"\n");
      return -1;
    }
    opspecin = ifile;
  }
  else {
    ifile = fopen(infile, "r");
    opspecin = ifile;
  }

  ofile.open(outfile.c_str(), ios_base::out);
  ofile << "(\n\n";    

  if(opspecparse()!=0){
    ofile.close();
    cerr << " Error in parsing specification" << endl;
    return -1;
  }

  ofile << "\n)";
  ofile.close();

  return 0;
}
