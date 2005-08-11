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

This file is the specification of a parser. It can be used as an 
input file for the parser generator yacc or the open source equivalent
bison. 
The grammar represents a boolean algebra including 

  * operators: and or not xor [=>] [<=>]

  * constants: true false

  * variables: ii ib ie bi bb be ei eb ee


The names of the variables refer to the entries in the 9 intersection matrix.
This means ii=true iff [interior_A][intersection][interior_B] [!=] [emptyset].

The letters have the following meaning:

  * i  the interior

  * b  the boundary

  * e  the exterior

The position of the letter denotes the position of the argument.

This main function provided by this parser is int parseString(char[star] , struct tree[star][star]).
If the string describes a valid boolean expression, the created tree is returned
in the second argument and the result will be 1. If the parsing failed, all created 
subtrees are destroyed automatically. The tree argument is not changed in this case and the
result value will be 0. 


*/

%{

/*

2.1 Needed includes

*/

#include "Tree.h"
#include "TreeParser.y.h"
#include <stdio.h>

#define YYERROR_VERBOSE

/*

2.2 Some variables holding the return value

*/

int parse_success;
struct tree* result;
char* last_message=0;

/*

2.3 Realization of the automatical destroying of created objects

Some operations called by the parser create new objects. To avoid
memory leaks, its required to destroy the objects in case of an
occured error. To do this, a stack picks up all created objects. 
In the yyerror (here called treeerror) function, all elements in the
stack und also the stack itself are destroyed.

*/ 

struct stack{
           struct tree* entry;
           struct stack* next;
        };

struct stack* treestack;

/*
2.3.1 Push

Pushes a new element on the stack.

*/
push(struct tree* entry){
    struct stack*  newelem = (struct stack*) malloc(sizeof(struct stack));
    newelem->next = treestack;
    newelem->entry= entry;
    treestack=newelem;
} 

/*
2.3.2 DestroyStack

Deallocates all memory occupied by the stack and the trees in the stack.

*/
void destroyStack(){
   while(treestack){
      struct stack* victim = treestack;
      treestack = treestack->next;
      free(victim->entry);
      free(victim);
   }
}

/*
2.3.3 DestroyStackWithoutEntries

Destroys the stack structure without touching the trees included in the stack.

*/
void destroyStackWithoutEntries(){
   while(treestack){
      struct stack* victim = treestack;
      treestack = treestack->next;
      free(victim);
   }

}

%}


/*

2.3 The Definition of the Parser

In the following some options for the parser are given and
the grammar is specified. For more information about the 
options and the syntax of the grammar consult the bison manual.
This manual is avaiable in different formats at [\n]
http://www.gnu.org/software/bison/manual/


*/


%union {
    struct tree* theTree;
}

%name-prefix="tree"

%token OR XOR NOT CONDITIONAL BICONDITIONAL OPEN CLOSE AND ERROR
%token II IB IE BI BB BE EI EB EE TRUE FALSE  
%type <theTree> expr term factor formula


%%

formula	: expr { $$ = $1;
                 result= $1; 
                 parse_success=1;
                 destroyStackWithoutEntries();
                 treestack=0;
               }
        ;

expr    : expr CONDITIONAL term {$$ = createConditional($1,$3);
                                 push($$); }
        | expr BICONDITIONAL term {$$ = createBiconditional($1,$3);
                                   push($$); }
        | term {$$ = $1; }
        ;

term    : term AND factor {$$ = createAnd($1,$3);push($$); }
        | term OR factor  {$$ = createOr($1,$3); push($$);}
        | term XOR factor {$$ = createXor($1,$3); push($$);}
        | factor { $$ = $1; }
        ;  

factor  : OPEN expr CLOSE { $$ = $2; }
        | NOT factor { $$ = createNot($2); push($$);}
        | TRUE { $$ =  createConstant(1); push($$);}
        | FALSE { $$ = createConstant(0); push($$);}
        | II    { $$ = createVariable(ii); push($$);}
        | IB    { $$ = createVariable(ib); push($$);}
        | IE    { $$ = createVariable(ie); push($$);}
        | BI    { $$ = createVariable(bi); push($$);}
        | BB    { $$ = createVariable(bb); push($$);}
        | BE    { $$ = createVariable(be); push($$);}
        | EI    { $$ = createVariable(ei); push($$);}
        | EB    { $$ = createVariable(eb); push($$);}
        | EE    { $$ = createVariable(ee); push($$);}
        ;

%%

/*

2.4 The Error function

This functions is automatically called when the parser detects
an error. The main task of this function is to destroy all objects
created while parsing and to set all results to the right values.

*/

int treeerror (const char *error)
{
 // save the error message
 if( last_message ) { 
      free(last_message); 
  }
  last_message = (char*) malloc(strlen(error) +1);
  strcpy( last_message, error );
  
  parse_success=0; 
  result = 0;
  destroyStack(); // destroy all created trees 
  return 0;
}


/*
2.5 The parseString function

This function  is the only one which should be called from 
outside. When this function is called, the string given as
the first argument is parsed according to the rules given above.
If the strng represents a valid boolean expression with operators
and variables described above, the result will be 1. In this case,
the second argument is the created operator tree. You have to ensure
that T is destroyed after use. 

*/
int parseString(const char* argument, struct tree** T){
    tree_scan_string(argument);
    treestack=0;
    treeparse();
    if(parse_success && last_message){
          free(last_message);
          last_message=0;
    }
    (*T) = result;
    return parse_success;
}

/* 
2.6 The GetLastMessage function

This function returns the last occured error. If no message
is available, the result will be NULL. Otherwise you shoul don't forget
to deallocate the memory of the result.

*/
char* GetLastMessage(){
  if(!last_message)
    return NULL;
  char* M = (char*) malloc(strlen(last_message)+1);
  strcpy(M,last_message);
  return M; 

}


