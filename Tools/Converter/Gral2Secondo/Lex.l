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

//paragraph [1] Title: [{\Large \bf ] [}]

//paragraph [2]	table8columns:	[\begin{quote}\begin{tabular}{|c|c|c|c|c|c|c|c|}] [\end{tabular}\end{quote}]
				
//[--------]	[\hline]

//[|] [&]

//[NR] [\\]

[1] Converting Tool from GRAL Data Export to SECONDO 

[1] Relation Objects

June 2003, Frank Hoffmann

This tool converts exported GRAL relations to a SECONDO specific format for relation 
objects, which can be stored in a SECONDO database with the 'restore' command. See 
also details about the Secondo command 'restore'.

The general usage is:

---- GralToSec inputfilename [ > outputfilename ]
----

where ~inputfilename~ is the file with the GRAL Data Export. These files normally
have an extension '.ef'. The ~outputfilename~ may have an arbitary identifier.

Please do the following few changes in the schema definition of the GRAL '.ef' file:

  * change type ~integer~ to type ~int~.
  
  * change type ~lines~ to type ~line~.
  
  * change type ~string~ to type ~text~, if the value of one string tuple attribute
    is longer than 48 characters.
    
  * change type ~polygon~ to type ~region~. Doubly defined points are not allowed
    in a polygon.
    
The internal representation for the data types is:
    
[2]	[--------] 
        int [|] real [|] string [|] bool [|] text [|] point [|] region [|] line [NR]
        [--------] 
	1   [|] 2    [|] 3      [|] -    [|] 5    [|] 6     [|] 7      [|] 8    [NR] 
	[--------]

(~File Lex.l~)

*/
%{
static int i=0, j=0, schemaattrno=0, tupleattrno=0;

#define BUFFERSIZE  5000
#define TEMPBUFFERSIZE  50
#define ATTRIBUTENO 20

static char buffer[BUFFERSIZE];
static int attrtype[ATTRIBUTENO];

# include <assert.h>
%}

blank		[ ]
tab		[\t]
newline		[\n]
digit           [0-9]
char		[A-Za-z0-9'\?\.\(\)/\-\"_, :\*]
string          {char}+
intnumber       {digit}({digit})*
realnumber      {digit}({digit})*.{digit}({digit})*
number          {intnumber}|{realnumber}
point           \({digit}*.{digit}*,{digit}*.{digit}*\)
pointseq        ({point};)+{point}
schemaline      ({blank}{blank}{char}+{blank}+:{blank}{string}\n)+
attrtype        {string}|{point}|{number}|{pointseq}
tupleline       ({blank}{blank}{char}+{blank}+:{blank}{attrtype}\n)+ 
start           \*+{blank}GRAL{blank}Editfile{blank}V2{blank}\*+\n
relation        RELATION{blank}{char}+\n
schema          SCHEMA\n{schemaline}+{newline}
tuple           TUPLE\n{tupleline}+{newline}
end             END{blank}RELATION{blank}({newline}|{char})*

%%

{start}           {printf("(OBJECT ");}
{relation}        {  i=9;
                     while ( yytext[i] != '\n' )
                     {
                       putchar(yytext[i]);
                       i++;
                     }
                     printf("\n    ()\n");
                  }
{schema}          {  printf("    (rel\n");
                     printf("        (tuple\n");
                     printf("            (");
                     i=9;
                     schemaattrno=0;
                     while ( i <= (yyleng-2) )
                     {
                       printf("\n                (");
                       while ( yytext[i] != '\n' )                       
                       {
                         if (yytext[i] == ':') 
                         {
                           putchar(' ');
                           
                           switch(yytext[i+2])
                           {
                             case 'i':
                               attrtype[schemaattrno] = 1;
                               break;
                             case 's':
                               attrtype[schemaattrno] = 3;
                               break;
                             case 'p':
                               attrtype[schemaattrno] = 6;
                               break;
                             case 'r':
			     if (yytext[i+4] == 'g')
                               attrtype[schemaattrno] = 7;
			     else attrtype[schemaattrno] = 2;
                               break;
                             case 'l':
                               attrtype[schemaattrno] = 8;
                               break;
                             case 't':
                               attrtype[schemaattrno] = 5;
                               break;
                           }
                           schemaattrno++;
                         } 
                         else if (yytext[i] != ' ')
                           putchar(yytext[i]);                         
                         i++; 
                       }
                       i++;
                       printf(")");
                     }
                     printf(")))\n    (\n");
                  }
{tuple}           {  
                     i=8;
                     tupleattrno=0;
                     printf("\n        ( ");
                     while ( i <= yyleng )
                     {
                       if (yytext[i] == ':')
                       {
                         i=i+2;
                         j=0;
                         while ( yytext[i] != '\n' )
                         {
                           assert (j < BUFFERSIZE);
                           buffer[j] = yytext[i];
                           i++;
                           j++;
                         }
                         buffer[j]='\0';
                         handleBuffer(buffer,attrtype[tupleattrno]);
                         tupleattrno++;
                       }
                       i++;
                     }
                     printf(")");
                  }
.                 ;
{newline}         ;
{end}             printf(")\n    ())\n");

%%

handlePoint(buffer)
char* buffer;
{
  int i=1, m;
  char tempbuffer[TEMPBUFFERSIZE];

  printf("(");
  while (buffer[i] != '\0')
  {
    m=0;
    while ( (buffer[i] != ',') && (buffer[i] != ')') )
    {
      assert (m < TEMPBUFFERSIZE);
      tempbuffer[m] = buffer[i];
      i++;
      m++;
    }
    i++;
    tempbuffer[m]='\0';
    printf("%.1f",atof(tempbuffer));
    if ( buffer[i-1] != ')' ) printf(" ");
  }
    
  printf(")");
  printf(" ");
}

handleRegion(buffer)
char* buffer;
{
  int i=0, m;
  char tempbuffer[TEMPBUFFERSIZE];
  
  printf("((( ");
  while (buffer[i] != '\0')
  {
    m=0;
    while ( buffer[i] != ';' )
    {
      assert(m < TEMPBUFFERSIZE);
      tempbuffer[m] = buffer[i];
      i++;
      m++;
    }
    i++;
    tempbuffer[m]='\0';
    handlePoint(tempbuffer);
  }
  printf("))) ");

}

handleLine(buffer)
char* buffer;
{
  int i, m, first, last;
  char tempbuffer[TEMPBUFFERSIZE];
  float xi,yi;
  
  i=1;
  first=1;
  last=0;
  printf("((");
  while (buffer[i] != '\0')
  {
    m=0;
    while ( buffer[i] != ',' ) 
    {
      assert(m < TEMPBUFFERSIZE);
      tempbuffer[m] = buffer[i];
      i++;
      m++;
    }
    i++;
    tempbuffer[m]='\0';
    xi=atof(tempbuffer);

    m=0;
    while ( buffer[i] != ')' ) 
    {
      assert(m < TEMPBUFFERSIZE);
      tempbuffer[m] = buffer[i];
      i++;
      m++;
    }
    i++;
    if ( buffer[i]=='\0' ) last=1;
    else i=i+2;
    tempbuffer[m]='\0';
    yi=atof(tempbuffer);
    
    if (first || last) 
    {
      printf("%.1f",xi);
      printf(" ");
      printf("%.1f",yi);
      first=0;
    }
    else
    {
      printf("%.1f",xi);
      printf(" ");
      printf("%.1f",yi);
      printf(")(");
      printf("%.1f",xi);
      printf(" ");
      printf("%.1f",yi);
    }
    if ( !last ) printf(" ");
  }
  printf(")) ");
}

handleInt(buffer)
char* buffer;
{
  printf(buffer);
  printf(" ");
}

handleReal(buffer)
char* buffer;
{
  handleInt(buffer);
}

handleString(buffer)
char* buffer;
{
  printf("\"");
  if ( strlen(buffer) <= 48 )
    printf(buffer);
  else
    printf("String too long. Please use type text.");
  printf("\"");
  printf(" ");
}

handleText(buffer)
char* buffer;
{
  printf("<text>");
  printf(buffer);
  printf("</text--->");
  printf(" ");
}

handleBuffer(buffer, attrnr)
char* buffer;
int attrnr;

{
  int buflength;
   
  switch(attrnr){
    case 1:
      handleInt(buffer);    
      break;
    case 2:
      handleReal(buffer);    
      break;
    case 3:
      handleString(buffer);
      break;
    case 5:
      handleText(buffer);
      break;
    case 6:
      handlePoint(buffer);
      break;
    case 7:
      buflength = strlen(buffer);
      buffer[buflength]=';';
      buffer[buflength+1]='\0';
      handleRegion(buffer);
      break;
    case 8:
      handleLine(buffer);
      break;
    default:
      printf(" error ");
      break;
  }
}

main (argc, argv)
int argc;
char** argv;
{
  if (argc == 2)
  {
    FILE *ofile, *ifile;
    char c;
    int cmp1, cmp2;

    cmp1 = strcmp(argv[1],"help");
    cmp2 = strcmp(argv[1],"h");
    if ( (cmp1 == 0) || (cmp2 == 0) )
    {
      printf("\nUSAGE:\n");
      printf("\tGralToSec inputfilename [> outputfilename]\n\n");
      printf("\tChanges in the schema defintion of the GRAL Export File\n");
      printf("\t  - change type integer to type int.\n");
      printf("\t  - change type lines to type line.\n");
      printf("\t  - change type string to type text, if the value of one\n"); 
      printf("\t    string tuple attribute is longer than 48 characters.\n");
      printf("\t  - change type polygon to type region. Doubly defined\n"); 
      printf("\t    points are not allowed in a polygon.\n\n");
      return 0;
    }

    ifile = fopen(argv[1], "r");
    if (ifile != NULL) {
    ofile = fopen("TTL", "w");
    while ((c = getc(ifile)) != EOF)
    {
      if ( c == '\\' )
      {
        c = getc(ifile);
	if (c != '\n') putc('\\', ofile);
	c = getc(ifile);
	if (c != ' ') putc(c, ofile);
      }
      else
      {
        putc(c, ofile);
      }
    }
    fclose(ifile);
    fclose(ofile);
    ofile = fopen("TTL", "r");
    yyin = ofile;
    yylex();
    fclose(ofile);
    fclose(ifile);
    remove("TTL");
    }
    else printf("Error: problems in reading from file\n");
    return 0;    
  }
  else printf("For more information type 'GralToSec (help | h)'\n");
}

