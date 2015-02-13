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

// #include "RegExParser.y.h"
#include "IntNfa.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Stack.h"
#include "IVector.h"
#include "Functions.h"


#define YYERROR_VERBOSE


/*

2.2 Some variables holding the return value

*/

#ifdef __cplusplus
extern "C"{
 int pregexlex();
 int pregexerror (const char *error);
 void pregex_scan_string(const char* argument);
}
#endif

int successful;
IntNfa* resultNfa;
Stack<IntNfa*>* stackNfa = 0;
char* pregex_last_message = 0;

%}

/*
2.3 The Definition of the Parser

*/

%union {
   struct IntNfa* theNfa;
   int numval;
}

%name-prefix "pregex"


%token OPEN CLOSE STAR OR PLUS OPTION
%token <numval> INTEGER 
%type <theNfa> expr term factor atom start

%%

start: expr{
         resultNfa = $1;
       }
     ;

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
       | atom STAR {
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
      | INTEGER {
          int number = $1;
          $$ = new IntNfa(number); 
          stackNfa->push($$);
        }
      ;

%%

/*

2.4 The Error function

This functions is automatically called when the parser detects
an error. 

*/

int pregexerror (const char *error) {
  std::cout << error << std::endl;
  successful = 1;
  resultNfa = 0;
  return 1;
}



extern "C"{void destroyLex();}

/*
2.5 The parseString function

This function  is the only one which should be called from 
outside. When this function is called, the string given as
the first argument is parsed according to the rules given above.

*/
int parsePatternRegEx(const char* argument, IntNfa** T) {
  successful = 0;
  destroyLex();
  stackNfa = new Stack<IntNfa*>();
  pregex_scan_string(argument);
  yyparse();
  if(successful && pregex_last_message) {
    free(pregex_last_message);
    pregex_last_message = 0;
  }
  // kill the stack
  while (!stackNfa->isEmpty()) {
    IntNfa* elem = stackNfa->pop();
    if (elem != resultNfa || successful) {
      delete elem;
    }
  }
  if (successful) {
    *T = 0;
  }
  else {
    (*T) = resultNfa;
  }
  delete stackNfa;
  stackNfa = 0;
  destroyLex();
  return successful;
}

/* 
2.6 The GetPreviousMessage function

This function returns the last occured error. If no message
is available, the result will be NULL. Otherwise you shoul don't forget
to deallocate the memory of the result.

*/
char* GetPreviousMessage() {
  if(!pregex_last_message) {
    return 0;
  }
  char* M = (char*) malloc(strlen(pregex_last_message) + 1);
  strcpy(M, pregex_last_message);
  return M; 
}

