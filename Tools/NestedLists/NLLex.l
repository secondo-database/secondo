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
#include "StringUtils.h"

#include "CharTransform.h"
#include "LogMsg.h"


// The scanner's state -- the input position and the text atom being collected
// -- lives in the NLScanner instance rather than in variables of this file, so
// that two scanners running at the same time do not share it. The rules below
// reach it directly; they are compiled as members of the class.

#define UPDATE_COL col += yyleng; currentLine += string(yytext);
#define RESET_COL col = 0; currentLine = ""; line++;

// The yylex() function that's actually used. The parser is a pure one, so
// where to put the semantic value is passed in rather than being a global.
#undef YY_DECL
#define YY_DECL int NLScanner::yylex( ListExpr* yylval_param )

#include "NLScanner.h"        // The interface of the derived class


%}
%option yyclass="NLScanner"

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

string          \"([^\"]|\\\")*\"

file		"<file>"[^<]+"</file--->"


%option noyywrap

%x TEXT
%x TEXTSIMPLE
%x COMMENT

%%

<INITIAL>\n|\r            { line++; col=0; currentLine=""; }

<INITIAL>[ \a\b\f\t\v]+   { UPDATE_COL }

<INITIAL>-?[0-9]+       { UPDATE_COL
                        *yylval_param = lexnl->IntAtom( atoi( yytext ) );
                        return (ZZINTEGER);
                        }

<INITIAL>{number}       { UPDATE_COL
                        std::stringstream ss(yytext);
                        double f = 0;
                        ss >> f;
                        *yylval_param = lexnl->RealAtom( f );
                        return (ZZREAL);
                        }

<INITIAL>"TRUE"         { UPDATE_COL
			*yylval_param = lexnl->BoolAtom( true );
			return (ZZBOOLEAN);
		        }

<INITIAL>"FALSE"        { UPDATE_COL
			*yylval_param = lexnl->BoolAtom( false );
			return (ZZBOOLEAN);
		        }



<INITIAL>{commentstart} { UPDATE_COL
                          BEGIN(COMMENT);
                        }
<COMMENT>{commentend}   { UPDATE_COL
                          BEGIN(INITIAL);
                        }
<COMMENT>\n             { RESET_COL
                          line++;
                        }

<COMMENT>.              { UPDATE_COL
                        }


<INITIAL>"<text>"       { UPDATE_COL
                        // change to state TEXT
                        BEGIN(TEXT);
                        text = new ostringstream();
            }

<TEXT>.|\n              { UPDATE_COL
                          if(yytext[0]=='\n'){
                             line++;
                             col = 0;
                          }
                          text->put( yytext[0] );
                        }

<TEXT>"\\</text--->"     { UPDATE_COL
                         (*text) << "</text--->";
                        }

<TEXT>"\\\\"     { UPDATE_COL
                         (*text) << "\\";
                        }


<TEXT>"</text--->"      { UPDATE_COL
                        BEGIN(INITIAL);
                        ListExpr newText = lexnl->TextAtom();
                        lexnl->AppendText( newText, text->str() );
                        delete text;
                        text = 0;
                        /* pass text node and token to the parser */
                        *yylval_param = newText;
                       return (ZZTEXT);
                        }

<INITIAL>"'"       { UPDATE_COL
                        // change to state TEXT
                        BEGIN(TEXTSIMPLE);
                        text = new ostringstream();
            }

<TEXTSIMPLE>[^']|\n              { UPDATE_COL
                          if(yytext[0]=='\n'){
                             line++;
                             col = 0;
                          }
                          text->put( yytext[0] );
                        }

<TEXTSIMPLE>"\\'"     { UPDATE_COL
                         (*text) << "'";
                        }

<TEXTSIMPLE>"\\\\"     { UPDATE_COL
                         (*text) << "\\";
                        }


<TEXTSIMPLE>"'"      { UPDATE_COL
                        BEGIN(INITIAL);
                        ListExpr newText = lexnl->TextAtom();
                        lexnl->AppendText( newText, text->str() );
                        delete text;
                        text = 0;
                        /* pass text node and token to the parser */
                        *yylval_param = newText;
                       return (ZZTEXT);
                        }

<INITIAL>{string}       { UPDATE_COL
      string s(yytext);
      s = stringutils::replaceAll(s,"\\\"","\"");
      s = stringutils::replaceAll(s,"\\\\","\\");
      yyleng = s.length();
      if ( s.length() - 2 > MAX_STRINGSIZE )
			{
			  yyleng = MAX_STRINGSIZE + 2;
			  cmsg.warning() << "Warning: string " << yytext << " was truncated!" << endl;
                          cmsg.send();
			}
      s = s.substr(1,yyleng-2);
			*yylval_param = lexnl->StringAtom( s );
			return (ZZSTRING);
		        }


<INITIAL>{symbol}  { UPDATE_COL
		     if ( yyleng > MAX_STRINGSIZE )
	             {
	               cmsg.error() << "Symbol " << yytext << " too long!" << endl;
                       cmsg.send();
                       return(ZZERROR);
		     }
                     *yylval_param = lexnl->SymbolAtom( yytext );
                     return (ZZSYMBOL);
                   }

<INITIAL>{file}  { /* files are expanded to base64 encoded text atoms */
                 UPDATE_COL
                 ListExpr newText = lexnl->TextAtom();

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

		 lexnl->AppendText(newText,textStream.str());

		 /* pass text node and token to the parser */
		 *yylval_param = newText;
		 return (ZZTEXT);
		 }


<INITIAL>"("   { UPDATE_COL
                 return (ZZOPEN);
               }

<INITIAL>")"   { UPDATE_COL
                 return (ZZCLOSE);
               }

.              { UPDATE_COL
                 return (ZZERROR);
               }
%%

