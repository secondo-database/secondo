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

*/

%{

#include <string.h>
#define true 1
#define false 0


void deleteCurrentBuffer(){
    yy_delete_buffer(YY_CURRENT_BUFFER);
}

char isHeader;
char usingfound;
char isFile;
char* filename;

char* lastSymbol=0;


%}


%option noyywrap

whitespace [ \n\r\t\a\b\f\t\v]
simplecomment [/][/].*[\n]
stringdelimiter \"
stringcontent (\\\"|[^"])
commentstart [/]\*
commentend  \*[/]
letter [a-zA-Z_#]
digit [0-9]
symbol {letter}({letter}|{digit})*


%x COMMENT
%x STRING
 
%%

<INITIAL>{whitespace} {}
<INITIAL>{simplecomment} {}
<INITIAL>{symbol} {
   if((strcmp(yytext,"namespace") == 0)  &&
       (lastSymbol != 0) &&
       strcmp(lastSymbol,"using") == 0){
        if(isHeader){
           fprintf(stderr,"found using namespace in header file");
           if(isFile){
               fprintf(stderr," in file %s\n", filename);
            } else {
               fprintf(stderr," in input\n");
            }
            return 1;
        } else {
            usingfound = true;
        }
   }
   if(lastSymbol){
     free(lastSymbol);
   } 
   lastSymbol = strdup(yytext);

   if(strcmp(yytext,"#include")==0){
         if(usingfound){
                fprintf(stderr,"found include after using");
                if(isFile){
                   fprintf(stderr," in file %s\n", filename);
                } else {
                   fprintf(stderr," in input\n");
                }
                return 1;
         }
   }

}


<INITIAL>{commentstart} { BEGIN(COMMENT); }
<INITIAL>{stringdelimiter} { BEGIN(STRING); }
<INITIAL>. {}
<COMMENT>{commentend} {BEGIN(INITIAL); }
<COMMENT>{whitespace} {}
<COMMENT>. {}
<STRING>{stringcontent} {}
<STRING>{stringdelimiter} { BEGIN(INITIAL);}


%%

int main(int argc, char** argv){


  isHeader = false;
  int offset = 0;


  if( (argc>=2) && !strcmp(argv[1],"-header")){ 
    isHeader = true;     
    offset = 1;
  }
  isFile = false;

  FILE* ifile;

  if(argc >= 2+offset){
    ifile = fopen(argv[1+offset], "r");
    if (ifile == NULL) {
         fprintf(stderr,"ERROR: cannot open file %s\n",argv[1+offset]);
         fprintf(stderr,"%s",argv[1]);
         fprintf(stderr,"\n");
         return -1; 
     }   
     isFile = true;
     filename = argv[1+offset];
     yyin = ifile;
  }

  usingfound = false;
  
  //fprintf(stderr,"start to process file %s\n", argv[1+offset] );

  int res  = yylex();

  //fprintf(stderr,"file processed\n" );

  if(lastSymbol){
     free(lastSymbol);
  }
  return res;

}
