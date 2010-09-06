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
//[=>] [\ensuremath{\Rightarrow}]
//[<=>] [\ensuremath{\Leftrightarrow}]
//[interior_A] [\ensuremath{A^\circ}]
//[interior_B] [\ensuremath{B^\circ}]
//[intersection] [\ensuremath{\cap}]
//[!=] [\ensuremath{\not=}]
//[emptyset] [\ensuremath{\symbol{31}}]

//[\n] [\\]
//[star] [\ensuremath{\ast}]

2 The Parser Specification

This Parser constructs an nfa from a regular expression over
integer numbers. The Grammar is:

  expr -> term | term [] expr
  term -> factor | factor term
  factor -> atom STARS | atom
  atom   -> NUMBER | ( expr )

*/

%{

/*

2.1 Needed includes

*/

#include "IntNfa.h"
#include "RegExParser.y.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define YYERROR_VERBOSE


/*

2.2 Some variables holding the return value

*/



void deleteCurrentBuffer();
int yylex();
int yyerror (const char *error);
void yy_scan_string(const char* argument);


int parse_success;
IntNfa* result;
char* last_message=0;

// %name-prefix="regex"
%}

/*

2.3 The Definition of the Parser


*/


%union {
   struct IntNfa* theNfa;
   int numval;
}


%token OPEN CLOSE STARS OR EPSILON ERROR 
%token <numval> NUMBER
%type <theNfa> expr term factor atom


%%

expr : term {
         $$ = $1;
       }

     | term OR expr {
         $$ = $1;
         IntNfa* n = $$;
         n->nfa.disjunction($3->nfa);
         delete $3;
       }
     ;

term : factor {
         $$ = $1;
       }
     | factor term {
         $$ = $1;
         IntNfa* n = $$;
         n->nfa.concat($2->nfa);
         delete $2;
       }
     ;

factor : atom {
          $$ = $1;
         }
       | atom STARS {
          $$ = $1;
          IntNfa* n = $$;
          n->nfa.star();
         }
       ;

atom  : OPEN expr CLOSE {
           $$ = $2;
        }
      | NUMBER {
        int number = $1;
        $$ = new IntNfa(number); 
        }
      | EPSILON {
          $$ = new IntNfa();
        }
      ;

%%

/*

2.4 The Error function

This functions is automatically called when the parser detects
an error. 

*/

int yyerror (const char *error)
{
  parse_success=0; 
  result = 0;
  return 0;
}


/*
2.5 The parseString function

This function  is the only one which should be called from 
outside. When this function is called, the string given as
the first argument is parsed according to the rules given above.


*/
int parseString(const char* argument, IntNfa** T){
    yy_scan_string(argument);
    yyparse();
    if(parse_success && last_message){
          free(last_message);
          last_message=0;
    }
    (*T) = result;
    deleteCurrentBuffer();
    return parse_success;
}

/* 
2.6 The GetLastMessage function

This function returns the last occured error. If no message
is available, the result will be NULL. Otherwise you shoul don't forget
to deallocate the memory of the result.

*/
char* GetLastMessage(){
  if(!last_message){
    return 0;
  }
  char* M = (char*) malloc(strlen(last_message)+1);
  strcpy(M,last_message);
  return M; 

}

