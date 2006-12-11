/*
----
This file is part of the PD system

Copyright (C) 1998 Ralf Hartmut Gueting, 
          (C) 2006 Markus Spiekermann

Fachbereich Informatik, FernUniversitaet in Hagen

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
----

File PDLib.c: This file contains some auxiliary functions.

*/

#include <stdio.h>
#include <stdlib.h>
#include "PDLib.h"

// debugging flags for flex and bison
extern int yydebug;
extern int yy_flex_debug;

void CheckDebugEnv() {

  yydebug=0;
  yy_flex_debug=0;
  char* d = getenv("PD_DEBUG");
  if (d != 0) {
    yydebug = 1;
    yy_flex_debug = yydebug;
  }  
}

extern char* yytext;
extern int yylineno;

int yyerror(const char *msg)
{
  if (yytext[0] == '\n' && yytext[1] == '\n')
    fprintf(stderr, "%s in paragraph before line %d.\n", msg, yylineno); 
  else
    fprintf(stderr, 
            "%s at line %d reading symbol '%s'.\n", msg, yylineno, yytext);
  exit(1);

}

// define LaTeX Commands for verbatim typesetting
char*  startProgram = "{\\small \\begin{quote} \\begin{verbatim}\n";
char*  endProgram = "\n\\end{verbatim} \\end{quote}}\n\n";

char*  startVerbatim = "\\begin{pdverbatim}\n    ";
char*  endVerbatim = "\n\\end{pdverbatim}\n"; 

void InitHeader() {

  char* s = getenv("PD_HEADER");
  if (s) {
    const char* lst="listing";
    // check if value of s ends with "listing"
    char* cs = strstr(s,lst);
    if ( cs && strcmp(cs,lst) == 0 ) 
    {
      startProgram = "\n\\begin{pdprogram}\\begin{lstlisting}\n";
      endProgram = "\\end{lstlisting}\\end{pdprogram}\n";
    } 
  }
}

void PrintTail()
{
  printf("\\end{document}\n");
}	

