/*

//paragraph [1] Title: [{\Large \bf ] [}]

[1] Scanner of Stable Nested Lists

Copyright (C) 1996 Gral Support Team

Februar 1996 Holger Schenk

December 2003 Markus Spiekermann, A new {file} pattern and Scanner states have been introduced.


The {file} pattern can be used in lists for reading in (binary) data from a file. Since the 
lists are designed as textual representation of data, the content of these files will be encoded
into a format called base64 which maps 24 bits of input to 4 letters into an alphabet of 
64 characters. An interface for encoding and decoding is provided by the class Base64. 


*/


%{
#include <malloc.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;
#include "Base64.h"
#include "NestedList.h"
#include "NLParser.h"
#include "NLParser.tab.hpp"

#define yylval NLParser::yylval

int yywrap();

const int MAX_STRING_LENGTH = 48;
char buffer[MAX_STRING_LENGTH+2];
char *s = buffer;
ostringstream* text=0;

double f;

// the yylex() function that's actually used
#define YY_DECL int NLScanner::yylex()

#include "NLScanner.h"        // The interface of the derived class

/*
The following yylex function is never called, but overruled by
NLScanner::yylex()

*/
int yyFlexLexer::yylex()
{
    return 0;
}

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

file		"<file>"[^<\n]+"</file--->"

%x TEXT
%%

<INITIAL>[ \t\n]+       ;

<INITIAL>-?[0-9]+       {
                        /* ECHO; printf(" : is an integer\n"); */
                        yylval = nl->IntAtom( atoi( yytext ) );
                        return (ZZINTEGER);
                        }

<INITIAL>{number}       {
                        /* ECHO; printf(" : is a real\n"); */
                        f = atof( yytext );
                        yylval = nl->RealAtom( f );
			/* printf("\n Parser reals: %10.3f \n", f); */
			return (ZZREAL);
		        }

<INITIAL>"TRUE"         {
			/* ECHO; printf(" : is a boolean\n"); */
			yylval = nl->BoolAtom( true );
			return (ZZBOOLEAN);
		        }

<INITIAL>"FALSE"        {
			/* ECHO; printf(" : is a boolean\n"); */
			yylval = nl->BoolAtom( false );
			return (ZZBOOLEAN);
		        }

<INITIAL>"<text>"       {  // change to state TEXT
                        BEGIN(TEXT);
                        text = new ostringstream();
		        }

 /*
<TEXT>[^<]{1,4096}      {
                        //cerr << "Match length: " << yyleng << endl;
                        text->write(yytext, yyleng);
                        }
 */

<TEXT>.|\n              { 
                        text->put( yytext[0] );
                        }


<TEXT>"</text--->"      {   
                        BEGIN(INITIAL);
                        ListExpr newText = nl->TextAtom();
                        nl->AppendText( newText, text->str() );
                        delete text;
                        /* pass text node and token to the parser */
                        yylval = newText;
		        return (ZZTEXT);
                        }


<INITIAL>{string}       {
			/* ECHO; printf(" : is a string\n"); */
			if ( yyleng - 2 > MAX_STRING_LENGTH )
			{
			  yyleng = MAX_STRING_LENGTH;
			  printf("Warning: string %s was truncated!\n", yytext);
			}

			s = strncpy( s, &yytext[1], yyleng-2 );
			s[yyleng-2] = '\0';
			yylval = nl->StringAtom( s );
			return (ZZSTRING);
		        }

<INITIAL>{symbol}  {
                   /* ECHO; printf(" : is a symbol\n"); */
                   yylval = nl->SymbolAtom( yytext );
                   return (ZZSYMBOL);
                   }

<INITIAL>{file}  { /* files are expanded to base64 encoded text atoms */

                 ListExpr newText = nl->TextAtom();

                 string fileNameStr(yytext);
                 fileNameStr = fileNameStr.substr(6,yyleng-16);
                 int start = fileNameStr.find_first_not_of(' ');
                 int end = fileNameStr.find_last_not_of(' ');
                 fileNameStr = fileNameStr.substr(start,end-start+1);
                 ifstream inFile;
		 stringstream textStream;

		 inFile.open(fileNameStr.c_str(), ios::binary);
		 if ( !inFile.is_open() ) {

		    cerr << "can not open file '" << fileNameStr << "'" << endl;
                    return (ZZNOFILE);

                 } else {

		   Base64* encoder = new Base64();
		   encoder->encodeStream(inFile, textStream);
                   delete encoder;
                 }

		 nl->AppendText(newText,textStream.str());
		 
		 /* pass text node and token to the parser */
		 yylval = newText;
		 return (ZZTEXT);
		 }


<INITIAL>"("   {
                 /* ECHO; printf("\n"); */
                 return (ZZOPEN);
               }

<INITIAL>")"   {
                 /* ECHO; printf("\n"); */
                 return (ZZCLOSE);
               }

.              { 
                 /* ECHO; printf("Fail in scanner\n"); */ 
               }
%%
int yywrap() { return 1; }

