/*
----
This file is part of the PD system
Copyright (C) 1998 Ralf Hartmut Gueting, Fachbereich Informatik, FernUniversitaet Hagen

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
----

1 linecheck

This little program checks a file for too long lines.
The maximal number of characters in a singe line is readed
as first argument. If the first argument is less than zero
(or not a number), the default value defined as
DEFAULT\_MAX\_LENGTH is taken. The Program prints out the
line numbers of too long lines. The return value is
the number of too long lines in the file.
If no file specified, this tool reads from stdin. So it can be
used within a pipe.

The programm is called via\\
  {\tt tabcheck [file]}

*/
%x comment
%{

#define TABSIZE 8
#define DEFAULT_MAX_LENGTH 80

int errors;
int line;
int ignoretabs;

%}


/* regular definitions */
%%

\t  { if(!ignoretabs){
          printf("found tabulators in line %d\n",line);
          ignoretabs = 1;
      }
      errors++;
     }
^"/*"  {BEGIN(comment);}
<comment>"*/" {BEGIN(INITIAL);}
<comment>.  ;
<comment>\n { line++; }



.   ; 
\n  { line++;
      ignoretabs=0;
    }

%%

int main (int argc, char** argv)
{
  FILE  *ifile;
  char c;
  int token;
  int filepos=1;
  ignoretabs = 0;

  if(argc>filepos){
    ifile = fopen(argv[filepos], "r");
    if (ifile == NULL)
    {
      fprintf(stderr,"ERROR: cannot open file ");
      fprintf(stderr,argv[filepos]);
      fprintf(stderr,"\n");
      return -1;
    }
    yyin = ifile;
  }

  line=1;
  token = yylex();
  while (token != 0)
    token = yylex();

  if(argc>filepos){
     fclose(ifile);
  }
  return errors;
}

