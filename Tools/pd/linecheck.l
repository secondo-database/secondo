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
  {\tt linecheck [maxlines] [file]}

*/
%x comment
%{

#define TABSIZE 8
#define DEFAULT_MAX_LENGTH 80

int length;
int maxlength;
int line;
int errors;

%}


/* regular definitions */
%%

\t  {length += TABSIZE; }
^"/*"  {BEGIN(comment);}
<comment>"*/" {BEGIN(INITIAL);}
<comment>.  ;
<comment>\n { line++; }



.   {length += 1;}
\n  { line++;
      if(length > maxlength){
         printf("%d symbols in line %d\n",length,line);
         errors++;
      }
      length=0;
    }

%%

int main (int argc, char** argv)
{
  FILE  *ifile;
  char c;
  int token;
  int filepos;

  if(argc<2){ // using default maxlength
     maxlength = DEFAULT_MAX_LENGTH;
  } else{
     maxlength = atoi(argv[1]);
  }

  if(maxlength<=0){
     maxlength = DEFAULT_MAX_LENGTH;
     filepos=1;
  } else{
     filepos=2;
  }

  if(argc>filepos){
    ifile = fopen(argv[filepos], "r");
    if (ifile == NULL)
    {
      fprintf(stderr,"ERROR: cannot open file ");
      fprintf(stderr,"%s",argv[filepos]);
      fprintf(stderr,"\n");
      return -1;
    }
    yyin = ifile;
  }

  length=0;
  line=0;
  errors=0;
  token = yylex();
  while (token != 0)
    token = yylex();

  if(argc>filepos){
     fclose(ifile);
  }
  return errors;
}

