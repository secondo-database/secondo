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

#include "RegExParser.y.h"
#include "IntNfa.h"
#include "../../include/Stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define YYERROR_VERBOSE


/*

2.2 Some variables holding the return value

*/

#ifdef __cplusplus
extern "C"{
 int regexlex();
 int regexerror (const char *error);
 void regex_scan_string(const char* argument);
}
#endif

int mtoprel_parse_success;
IntNfa* mtoprel_result;
Stack<IntNfa*>* mtoprel_stack=0;
char* regex_last_message=0;

%}

/*

2.3 The Definition of the Parser


*/


%union {
   struct IntNfa* theNfa;
   int numval;
}

%name-prefix="regex"


%token OPEN CLOSE STARS OR EPSILON ERROR 
%token <numval> NUMBER
%type <theNfa> expr term factor atom start


%%

start: expr{

         //std::cout << "start -> expr" << std::endl;
         mtoprel_result = $1;
      }

expr : term {
         //std::cout << "expr -> term " << std::endl;
         $$ = $1;
       }

     | term OR expr {
         //std::cout << "expr -> term | expr " << std::endl;
         $$ = $1;
         IntNfa* n = $$;
         n->nfa.disjunction($3->nfa);
       }
     ;

term : factor {
         //std::cout << "term -> factor" << std::endl;
         $$ = $1;

       }
     | factor term {
         //std::cout << "term -> factor term" << std::endl;
         $$ = $1;
         IntNfa* n = $$;
         n->nfa.concat($2->nfa);
       }
     ;

factor : atom {
         //std::cout << "factor -> atom " << std::endl;
          $$ = $1;
         }
       | atom STARS {
         //std::cout << "factor -> atom stars" << std::endl;
          $$ = $1;
          IntNfa* n = $$;
          n->nfa.star();
         }
       ;

atom  : OPEN expr CLOSE {
         //std::cout << "atom -> ( epxr ) " << std::endl;
           $$ = $2;
        }
      | NUMBER {
         //std::cout << "atom -> number" << std::endl;
        int number = $1;
        $$ = new IntNfa(number); 
        mtoprel_stack->push($$);
        }
      | EPSILON {
         //std::cout << "atom -> epsilon " << std::endl;
          $$ = new IntNfa();
          mtoprel_stack->push($$);
        }
      ;

%%

/*

2.4 The Error function

This functions is automatically called when the parser detects
an error. 

*/

int regexerror (const char *error)
{
  mtoprel_parse_success=1; 
  mtoprel_result = 0; // force deleting of result
  return 0;
}



extern "C"{void mtoprel_lexDestroy();}

/*
2.5 The parseString function

This function  is the only one which should be called from 
outside. When this function is called, the string given as
the first argument is parsed according to the rules given above.


*/
int parseString(const char* argument, IntNfa** T){
    mtoprel_lexDestroy();

    mtoprel_stack = new Stack<IntNfa*>();

    regex_scan_string(argument);

    yyparse();

    if(mtoprel_parse_success && regex_last_message){
          free(regex_last_message);
          regex_last_message=0;
    }

    // kill the stack
    while(!mtoprel_stack->isEmpty()){
      IntNfa* elem = mtoprel_stack->pop();
      if(elem!=mtoprel_result){
        delete elem;
      }
    }

    (*T) = mtoprel_result;

    delete mtoprel_stack;
    mtoprel_stack = 0;

    mtoprel_lexDestroy();
    return mtoprel_parse_success;
}

/* 
2.6 The GetLastMessage function

This function returns the last occured error. If no message
is available, the result will be NULL. Otherwise you shoul don't forget
to deallocate the memory of the result.

*/
char* MTopRel_GetLastMessage(){
  if(!regex_last_message){
    return 0;
  }
  char* M = (char*) malloc(strlen(regex_last_message)+1);
  strcpy(M,regex_last_message);
  return M; 
}

