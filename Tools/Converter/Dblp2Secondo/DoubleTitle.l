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

int i;

%}

/* regular definitions */

author                 <author>.*<\/author>
title                  <title>.*<\/title>
newline                [\n]

%%


{title}{newline}*{title}+  { 
                             i=0;
                             while(yytext[i] != '\n') i++;                  
                             yytext[i+1]='\0';     
                             printf(yytext);
                           }



%%

main (argc, argv)
int argc;
char** argv;
{
  FILE *ofile, *ifile;
  char c;
  int token;

  if(argc > 1) {
    ifile = fopen(argv[1], "r");
    if (ifile == NULL) {
         fprintf(stderr,"DoubleTitle::ERROR: cannot open file ");
         fprintf(stderr,argv[1]);
         fprintf(stderr,"\n");
         return -1;
    }  
    yyin = ifile;
  }  
  token = yylex();
  while (token != 0)
    token = yylex();
  return 0; 
}
