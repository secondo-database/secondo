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

April 2006, M. Spiekermann. Support for text atoms and variable separator
added. 

*/

%{
   char separator;
   char floatSep1;
   char floatSep2;
   bool convert;
%}

otherChar      [^\(\)\"A-Za-z0-9_ \t\n]
letter         [a-zA-Z]
digit          [0-9]
ident          {letter}({letter}|{digit}|_)*  
math           {otherChar}{otherChar}*
symbol         {math}|{ident}
num1	       [-+]?{digit}+\.?([eE][-+]?{digit}+)?
num2	       [-+]?{digit}*\.{digit}+([eE][-+]?{digit}+)?
number	       {num1}|{num2}

string         \"[^\"]*\"         
text           "<text>".*"</text--->"

%%

[ \t\n(]+       ;

-?[0-9]+       {for (int i=0; i<yyleng; i++) 
                  putchar(yytext[i]); 
                putchar(separator);
               }

"TRUE"         {for (int i=0; i<yyleng; i++) 
                  putchar(yytext[i]); 
                putchar(separator);
               }

"FALSE"        {for (int i=0; i<yyleng; i++) 
                  putchar(yytext[i]); 
                putchar(separator);
               }

{number}       {for (int i=0; i<yyleng; i++) { 
                  char c = yytext[i];
                  if (c == floatSep1 && convert) {
                    c = floatSep2;
                  }
                  putchar(c);
                }
                putchar(separator);
               }

{symbol}       {for (int i=0; i<yyleng; i++) 
                  putchar(yytext[i]); 
                putchar(separator);}

{string}       {for (int i=1; i<(yyleng-1); i++) 
                  putchar(yytext[i]); 
                putchar(separator);
               }

{text}         {for (int i=6; i<(yyleng-10); i++) 
                  putchar(yytext[i]); 
                putchar(separator);
               }

")"            {putchar('\n');}

%%

main (int argc, char* argv[])
{
  // default csv separator is a tabulator
  separator = '\t';

  char* sep2 = getenv("CSV_SEP"); 
  if (sep2 != 0) {
    separator = sep2[0];
  }

  
  floatSep1 = '.';
  floatSep2 = ',';
  convert = false;
  char* fsep = getenv("FLOAT_SEP");
  if (fsep != 0) {
    convert = true;
    floatSep1 = fsep[0];
    floatSep2 = fsep[1];
  }

  if (argc == 2)
  {
    FILE *ifile;
    int cmp1, cmp2;

    cmp1 = strcmp(argv[1],"help");
    cmp2 = strcmp(argv[1],"h");
    if ( (cmp1 == 0) || (cmp2 == 0) )
    {
      printf("\nUSAGE:\n");
      printf("\tListToTable inputfilename [> outputfilename]\n\n"); 
      return 0;
    }

    ifile = fopen(argv[1], "r");
    if (ifile == NULL)
    {
      printf("ERROR: cannot open file ");
      printf(argv[1]);
      printf("\n");
      return 0;
    } 
    yyin = ifile;
    yylex();
    fclose(ifile);
    return 0;    
  }
  else
  {
    printf("ERROR: wrong number of parameters\n");
    printf("Try ListToTable h or ListToTable help for more information\n");
    return 0;
  }
}


