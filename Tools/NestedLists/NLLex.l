/*

//paragraph [1] Title: [{\Large \bf ] [}]

[1] Scanner of Stable Nested Lists

Copyright (C) 1996 Gral Support Team

Februar 1996 Holger Schenk

Textatoms are divided into parts of size 60 and appended to the previously 
created text atom. If the final pattern of a text atom \verb+</text--->+ 
can't be matched the text atom is destroyed. A separate list is only 
creating a text atom and appending the parts to the atom if lexing is O.K.
There is certainly a better solution, but there is no C-Interface to the 
list module at the moment ..

*/
                        

%{
#include <malloc.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <sstream>

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

file		"<file>".*"</file--->"       

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

<TEXT>"<text>"       {  // change to state TEXT
                        BEGIN(TEXT);
                        text = new ostringstream();

                     }

<TEXT>.|\\n       { text->put( yytext[0] );

                  }


<TEXT>"</text--->"  {   BEGIN(INITIAL);
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

<INITIAL>{symbol}       { 
                 /* ECHO; printf(" : is a symbol\n"); */
                 yylval = nl->SymbolAtom( yytext );
                 return (ZZSYMBOL);
               }

<INITIAL>{file}  { /* files are expanded to base64 encoded text atoms */

                 ListExpr newText = nl->TextAtom(); 
                 
                 string fileNameStr(yytext);
                 fileNameStr = fileNameStr.substr(6,yyleng-16);
                 ifstream inFile(fileNameStr.c_str(), ios::binary); 
                 ofstream outFile((fileNameStr+".base64").c_str());                 

                 /*   Read  3/4 of the length of a base64 line (max 72 bytes) into a buffer, 
                  *   since base64 uses 4/3 of the binary size.
                  */ 
                 const int inLength = 54;
                 char buf[inLength+1]; 
                 buf[inLength] = 0;
                 int readFaults = 0;                 

                 string textFragment="";
                 Base64* encoder = new Base64();
                 do {
                    inFile.read(buf, inLength);
                    int noBytes = inFile.gcount();
                    if (noBytes != inLength) {
                     readFaults++; 
                    } 
                    assert( readFaults <= 1 );
                    encoder->encode(buf, textFragment, noBytes);
                    
                    nl->AppendText( newText, textFragment );   
                    outFile << textFragment;
                 } while (inFile.good());
                 delete encoder;                 

                 if (!inFile.eof()) {
                    cerr << "Problems reading file " << fileNameStr << endl;
                    exit(1);
                 } 

                 /* pass text node and token to the parser */
                 yylval = newText;
                 return (ZZTEXT);
               }


<INITIAL>"("   {  
                 /* ECHO; printf("\n"); */ 
                 return (ZZOPEN);
               }

<INITIAL>")"            {
                 /* ECHO; printf("\n"); */
                 return (ZZCLOSE);
               }

.              { 
                 /* ECHO; printf("Fail in scanner\n"); */ 
               }
%%
int yywrap() { return 1; }

