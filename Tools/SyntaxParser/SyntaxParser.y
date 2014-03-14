/*
---- 
This file is part of SECONDO.

Copyright (C) 2004-2007, 
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

//[_] [\_]

*/

%{


/*
1.0 Includes

*/

#include <iostream>
#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>
#include <set>
#include <cstdlib>
#include "Types.h"
using namespace std;

/*
1.1 Some external defined variables 

*/
extern int yylineno;
extern int yylex();
extern FILE* yyin;

/*
1.2  Files for writing the rules 

There are four files for writing entries. 

*/
ofstream* lexrules;
ofstream* yaccrules1;
ofstream* yaccrules2;
ofstream* tokens;
int tokenslength; // length of the current tokens line
int maxtokenslength = 70;

/* 
1.3 A set containing tokens for simple postfix operators 

Postfix operators need a special treatment because the different
length of the valuelist. For each different number of arguments before the
operator, a new rule must be inserted and a new token is created. The tokennames
build the content for this set. By looking in the set for already contained
tokens, we can decide whether a rule for this operator already exists.

*/


/*
1.4 Recognition of used Operators

The next two sets collects the names of already used operators as well as
the operatorname together with its pattern. Therewith it is possible 
to ignore operators already transformed as well as operators with the
same name but different patterns.

*/
set<string> operatornames;
set<string> opnamesWithPattern;





/*
1.5 Some variables for statistical informations

*/



/*
1.4 The error function

This function will be called by the parser in case of an error.
The function writes an error message as well as the line number
which has caused the error to the standard error output.

*/
void yyerror( const char* s )
{
  cerr << endl << s << endl << endl;
  cerr << "error at line " << yylineno << endl;
}




ofstream pFile;
ofstream pFile1;

bool init(){
  // set the translation to be empty
  
  string priorityFileName = "opprios.pl";
  pFile.open(priorityFileName.c_str());
  pFile1.open("opsyntax.pl");
}
%}

%union{
  char* text;
  OpPatternType* pat;
  int   len;
  bool  flag;
   bool  flag1;
}

%token   ZZOPERATOR ZZPATTERN ZZFUN ZZOP ZZINFIXOP  ZZLIST
         ZZIMPLICIT ZZPARAMETER ZZPARAMETERS ZZTYPE ZZTYPES ZZFUNLIST 
         ZZCOMMENT ZZFORCEBUFFER 

%token<text> ZZIDENTIFIER  ZZSYMBOL ZZALIAS

%type<text> name
%type<len> argscomma simpleargscomma simpleargsblank
%type<pat> pattern prefix infix postfix 
%type<flag> sublist arguments
%type<flag1> implicit
 

%%

specfile  :  specs
          |
          ;

specs      : spec
          | ZZCOMMENT
          | specs spec
          | specs ZZCOMMENT
          ;

spec      :  ZZOPERATOR name ZZALIAS ZZIDENTIFIER ZZPATTERN pattern implicit bufferforced
            {   
               
               free($4); // we don't need this here 
               OpPatternType* s = $6;
               string op = string($2);
               free( $2);
               if(operatornames.find(op)==operatornames.end())
                 {
                   operatornames.insert(op);
                 switch(s->optype)
        {     case PREFIX:
             {
//             pFile << ":-op(800 , fx , " << op << ")." << endl;
             pFile1 << " secondoOpG(("<< op <<") ," << " prefix, " << s->no_args << ")." << endl;
             break;   
	     }
             case INFIX:
            {
              pFile  << ":-op(800 , xfx , " << op << ")." << endl;
              pFile1 << " secondoOpG(("<< op <<") ," << " infix," << s->no_args << ")." << endl;
              break;
            }
             case POSTFIX:
          {
              bool implicit = $7;
              pFile  << ":-op(800 , xf , "<< op <<")." << endl; // not for postfixbrackets!
              if(  (s->isSpecial== true) /* || (implicit == true) */ ){
                // pFile1 << " secondoOpG(("<< op <<") ," << " special, " << s->no_args << ")." << endl;
                pFile1 << " secondoOpG(("<< op <<") ," << " postfix, " << s->no_args << ")." << endl;
              } else {
                pFile1 << " secondoOpG(("<< op <<") ," << " postfix, " << s->no_args << ")." << endl;
              }
              break;
          }
             case POSTFIXBRACKETS:
           {
              if(  (s->isSpecial== true) /* || (implicit == true) */ ){
                // pFile1 << " secondoOpG(("<< op <<") ," << " special, " << s->no_args << ")." << endl;
                pFile1 << " secondoOpG(("<< op <<") ," << " postfixbrackets, " << s->no_args << ")." << endl;
              } else {
                pFile1 << " secondoOpG(("<< op <<") ," << " postfixbrackets, " << s->no_args << ")." << endl;
              }
              break;
  }
          default : {
             cerr << "invalid operator type found" << endl;
             cerr << "the operator name is " << op << endl;
          }
          
       }}
          delete $6;
         
}
          ;

name      : ZZIDENTIFIER  
            { $$ = $1; }
          | ZZSYMBOL
            { $$ = $1; }  
          ;


pattern    : prefix {$$ = $1; }
          | infix   {$$ = $1; } 
          | postfix{ $$ = $1; }                    
          ;

infix     : '_' ZZINFIXOP '_'
            {  $$ = new OpPatternType();
               $$->optype = INFIX;
               $$->no_args = 2;  
               $$->isSpecial = false;
            }
          ;

prefix    : ZZOP '('simpleargscomma')'    

             
            {  $$ = new OpPatternType();
               $$->optype = PREFIX;
               $$->no_args = 0;   
               $$->isSpecial = false;
              
            }
          | ZZOP '(' ')'
            { 
               $$ = new OpPatternType();
               $$->optype = PREFIX;
               $$->no_args = 0;   
               $$->isSpecial = false;
            }
            
          ;

postfix    : simpleargsblank ZZOP

          {  $$ = new OpPatternType();
             $$->optype = POSTFIX;
             $$->no_args = $1;  
             $$->isSpecial = false;
          }    

          | simpleargsblank ZZOP '['   arguments ']'
          { 
               $$ = new OpPatternType();
               $$->optype = POSTFIXBRACKETS;
               $$->no_args = $1; 
               $$->isSpecial =  $4; 
          }   
          ;


simpleargscomma  : '_'
                 {$$ = 1; }
                | simpleargscomma ',' '_'
                 {$$ = $1 + 1;}
                ;

simpleargsblank  : '_'
                {   
                    $$ = 1;
                }
                | simpleargsblank '_'
                {   
                    $$ = $1 + 1;
                }
                ;

arguments  : sublist { 
               $$ = $1;
             }
           | arguments ';' sublist {
               $$ = true;
             }
           ;

sublist    : argscomma
          { $$ = false;
          }
          | ZZLIST
          { 
            $$ = true;
          } 
          | ZZFUNLIST{
            $$ = true;
            
          }
          ;

argscomma  : arg
          { $$ = 1; }
          | argscomma ',' arg
          { $$ = $1 + 1; }
          ;

arg    : '_'
      {
      }
      | ZZFUN
      {
         
      }
      ;

implicit  : ZZIMPLICIT rest
           { $$ = true;
           }
          | { $$ = false; }
          
          ;

rest    : ZZPARAMETER parameterlist ZZTYPE typelist
        |  ZZPARAMETERS parameterlist ZZTYPES typelist
        ;

bufferforced : ZZFORCEBUFFER 
              { 
               }
             |{ 
              }
             ; 

parameterlist : ZZIDENTIFIER
                { 
                  free($1);
                }
              | parameterlist ',' ZZIDENTIFIER
                { 
                  free($3);
                }
              ;
typelist : ZZIDENTIFIER
                { 
                  free($1);
                }
        | typelist ',' ZZIDENTIFIER
                { 
                 free($3);
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
        yyin = ifile;
    }
    init();
    if(yyparse()!=0){
       cerr << " Error in parsing specification" << endl;
       pFile.close();
       pFile1.close();
       return -1;
    }
    pFile.close();
    pFile1.close();
    
    return 0;
}
