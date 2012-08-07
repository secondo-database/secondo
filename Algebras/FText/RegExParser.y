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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Stack.h"
#include "IVector.h"
#include "Functions.h"

#define NUMCHARS 256


#define YYERROR_VERBOSE


/*

2.2 Some variables holding the return value

*/

#ifdef __cplusplus
extern "C"{
 int cregexlex();
 int cregexerror (const char *error);
 void cregex_scan_string(const char* argument);
}
#endif

int parse_success;
IntNfa* result;
Stack<IntNfa*>* stack=0;
char* cregex_last_message=0;

%}

/*

2.3 The Definition of the Parser


*/


%union {
   struct IntNfa* theNfa;
   int numval;
   struct ivec*  range;
}

%name-prefix="cregex"


%token OPEN CLOSE STARS OR EPSILON PLUS ANY  OPTION TO OPENSET OPENINVERSESET CLOSESET
%token <numval> CHAR 
%type <theNfa> expr term factor atom start
%type <range> set singlerange


%%

start: expr{
         result = $1;
      }

expr : term {
         $$ = $1;
       }

     | term OR expr {
         $$ = $1;
         IntNfa* n = $$;
         n->nfa.disjunction($3->nfa);
       }
     ;

term : factor {
         $$ = $1;

       }
     | factor term {
         $$ = $1;
         IntNfa* n = $$;
         n->nfa.concat($2->nfa);
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
       | atom PLUS {
           $$ = $1;
           Nfa<int> tmp($$->nfa);
           tmp.star();
           $$->nfa.concat(tmp); 
        }
       | atom OPTION {
          $$ = $1;
          Nfa<int> tmp; 
          IntNfa* n = $$;
          n->nfa.disjunction(tmp);
        }
       ;

atom  : OPEN expr CLOSE {
           $$ = $2;
        }
      | CHAR {
        int number = $1;
        $$ = new IntNfa(number); 
        stack->push($$);
        }
      | ANY {
        $$ = new IntNfa(0,255);
        stack->push($$);  
       }
      | OPENSET set CLOSESET {
         $$ = new IntNfa($2->theVector);
         stack->push($$);
         delete $2;
      }
      | OPENINVERSESET set CLOSESET {
        std::vector<int>  v;
        for(int i=0;i<NUMCHARS;i++){
           if(!contains<int>($2->theVector, i)){
               v.push_back(i);
           }
        }
        delete $2;
        $$  = new IntNfa(v);
        stack->push($$); 
      }
      ;

set : singlerange {
        $$ = $1;
      }
    | set singlerange {
        $$ = $1;
        for(unsigned int i = 0 ; i< $2->theVector.size(); i++){
           int k = $2->theVector[i];
           if(!contains<int>($$->theVector, k)){
               $$->theVector.push_back(k);
           }
        }
        delete $2;
      }
    ;

singlerange : CHAR TO CHAR{
                $$ = new ivec();
                for(int i=$1;i<=$3;i++){
                  $$->theVector.push_back(i);
                }
             }
      | CHAR {
          $$ = new ivec();
          $$->theVector.push_back($1);
        }
      ;


%%

/*

2.4 The Error function

This functions is automatically called when the parser detects
an error. 

*/

int cregexerror (const char *error)
{
  std::cout << error << std::endl;
  parse_success=1; 
  result = 0;
  return 1;
}



extern "C"{void lexDestroy();}

/*
2.5 The parseString function

This function  is the only one which should be called from 
outside. When this function is called, the string given as
the first argument is parsed according to the rules given above.


*/
int parseRegEx(const char* argument, IntNfa** T){

    parse_success=0;
    lexDestroy();

    stack = new Stack<IntNfa*>();


    cregex_scan_string(argument);

    yyparse();


    if(parse_success && cregex_last_message){
          free(cregex_last_message);
          cregex_last_message=0;
    }


    // kill the stack
    while(!stack->isEmpty()){
      IntNfa* elem = stack->pop();
      if(elem!=result || parse_success){
        delete elem;
      }
    }

    if(parse_success){
      *T = 0; 
    }  else {
       (*T) = result;
    }

    delete stack;
    stack = 0;

    lexDestroy();

    return parse_success;
}

/* 
2.6 The GetLastMessage function

This function returns the last occured error. If no message
is available, the result will be NULL. Otherwise you shoul don't forget
to deallocate the memory of the result.

*/
char* GetLastMessage(){
  if(!cregex_last_message){
    return 0;
  }
  char* M = (char*) malloc(strlen(cregex_last_message)+1);
  strcpy(M,cregex_last_message);
  return M; 
}

