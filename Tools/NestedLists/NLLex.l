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

[1] Scanner of Stable Nested Lists

Copyright (C) 1996 Gral Support Team

Februar 1996 Holger Schenk

December 2003 Markus Spiekermann, A new {file} pattern and Scanner states have been introduced.

December 2004 M. Spiekermann. The pattern otherchar was redefined Ansi-C backslash codes are
removed. Morevocer, a calculation of the input position has been implemented.

The {file} pattern can be used in lists for reading in (binary) data from a file. Since the 
lists are designed as textual representation of data, the content of these files will be encoded
into a format called base64 which maps 24 bits of input to 4 letters into an alphabet of 
64 characters. An interface for encoding and decoding is provided by the class Base64. 


*/


%{
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

#include "CharTransform.h"
#include "LogMsg.h"


// variables only visible for this
// compilation unit
static char buffer[MAX_STRINGSIZE+1];
static char *s = buffer;
static ostringstream* text = 0;
static double f;

// global variables
int scanNL_lines;
int scanNL_cols;

#define UPDATE_COLS scanNL_cols += yyleng;

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
otherChar       [^\(\)\"\'A-Za-z0-9_ \a\b\f\n\r\t\v]
letter          [a-zA-Z]
digit           [0-9]
ident           {letter}({letter}|{digit}|_)*
math            {otherChar}{otherChar}*
symbol          {math}|{ident}
num1	        [-+]?{digit}+\.?([eE][-+]?{digit}+)?
num2	        [-+]?{digit}*\.{digit}+([eE][-+]?{digit}+)?
number	        {num1}|{num2}

commentstart	"_!"
commentend		"!_"

string          \"[^\"]*\"
simpletext	"'"[^']*"'"		

file		"<file>"[^<]+"</file--->"


%option noyywrap

%x TEXT
%x COMMENT
%%

<INITIAL>\n|\r            { scanNL_lines++; scanNL_cols=0; }

<INITIAL>[ \a\b\f\t\v]+   { UPDATE_COLS }

<INITIAL>-?[0-9]+       { scanNL_cols += yyleng; 
                        yylval = nl->IntAtom( atoi( yytext ) );
                        return (ZZINTEGER);
                        }

<INITIAL>{number}       { scanNL_cols += yyleng; 
                        f = atof( yytext );
                        yylval = nl->RealAtom( f );
			return (ZZREAL);
		        }

<INITIAL>"TRUE"         { scanNL_cols += yyleng; 
			yylval = nl->BoolAtom( true );
			return (ZZBOOLEAN);
		        }

<INITIAL>"FALSE"        { scanNL_cols += yyleng; 
			yylval = nl->BoolAtom( false );
			return (ZZBOOLEAN);
		        }


<INITIAL>{simpletext}	{ string value(yytext);
                          value = value.substr(1,value.length()-2);
                          ListExpr newText = nl->TextAtom(value);
                          yylval = newText;
			  return (ZZTEXT);
                        }

<INITIAL>"<text>"       { scanNL_cols += yyleng;
                        // change to state TEXT
                        BEGIN(TEXT);
                        text = new ostringstream();
		        }

 /*
<TEXT>[^<]{1,4096}      {
                        //cerr << "Match length: " << yyleng << endl;
                        text->write(yytext, yyleng);
                        }
 */
<INITIAL>{commentstart} { scanNL_cols += yyleng;
                          BEGIN(COMMENT);
                        }
<COMMENT>{commentend}   { scanNL_cols += yyleng;
                          BEGIN(INITIAL);
                        }
<COMMENT>\n             { scanNL_cols=0;
                          scanNL_lines++;
                        }

<COMMENT>.              { scanNL_cols++;
                        }


<TEXT>.|\n              { scanNL_cols += yyleng;
                          if(yytext[0]=='\n'){
                             scanNL_lines++;
                             scanNL_cols = 0;
                          } 
                          text->put( yytext[0] );
                        }


<TEXT>"</text--->"      { scanNL_cols += yyleng;  
                        BEGIN(INITIAL);
                        ListExpr newText = nl->TextAtom();
                        nl->AppendText( newText, text->str() );
                        delete text;
                        /* pass text node and token to the parser */
                        yylval = newText;
		        return (ZZTEXT);
                        }


<INITIAL>{string}       { scanNL_cols += yyleng;
			if ( yyleng - 2 > MAX_STRINGSIZE )
			{
			  yyleng = MAX_STRINGSIZE + 2;
			  cmsg.warning() << "Warning: string " << yytext << " was truncated!" << endl;
                          cmsg.send();
			}

			s = strncpy( s, &yytext[1], yyleng-2 );
			s[yyleng-2] = '\0';
			yylval = nl->StringAtom( s );
			return (ZZSTRING);
		        }

<INITIAL>{symbol}  { scanNL_cols += yyleng;
		     if ( yyleng > MAX_STRINGSIZE )
	             {
	               cmsg.error() << "Symbol " << yytext << " too long!" << endl;
                       cmsg.send();
                       return(ZZERROR);
		     }
                     yylval = nl->SymbolAtom( yytext );
                     return (ZZSYMBOL);
                   }

<INITIAL>{file}  { /* files are expanded to base64 encoded text atoms */
                 scanNL_cols += yyleng;
                 ListExpr newText = nl->TextAtom();

                 string fileNameStr(yytext);
                 static const int textStartLen = string("<file>").size();
                 static const int textEndLen = string("</file--->").size();
                 int contentLen = yyleng - textStartLen - textEndLen;

                 fileNameStr = trim( fileNameStr.substr(textStartLen,contentLen) );
                 fileNameStr = expandVar(fileNameStr);

                 ifstream inFile;
		 stringstream textStream;

		 inFile.open(fileNameStr.c_str(), ios::binary);
		 if ( !inFile.is_open() ) {

		    cmsg.error() << "Can not open file '" << fileNameStr << "'" << endl;
                    cmsg.send();
                    return (ZZERROR);

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


<INITIAL>"("   { scanNL_cols += yyleng;
                 return (ZZOPEN);
               }

<INITIAL>")"   { scanNL_cols += yyleng;
                 return (ZZCLOSE);
               }

.              { scanNL_cols += yyleng;
                 return (ZZERROR); 
               }
%%

