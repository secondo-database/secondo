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

August 8, 2000 RHG Added print command to show result of text to list translation.

March 2002 Ulrich Telle Port to C++

December 2004 M. Spiekermann. The error function was moved here and revised to
report messages to a ~cmsg~ which will read in the ~SecondoInterface~. The Secondo
parser is a bit messy because Scanner and Parser classes are not well separated.
During implementation of ~xxerror~ this was confusing. 

*/

using namespace std;

#include <iostream>
#include <sstream>
#include <string>

#include "NestedText.h"
#include "SecParser.h"
#include "LogMsg.h"

#ifndef yyFlexLexer
#define yyFlexLexer xxFlexLexer
#include <FlexLexer.h>
#endif

extern string* yacc_outtext;
extern int xxparse();
extern int xxdebug;

extern CMsg cmsg;

static xxFlexLexer* lexPtr;

class Text2ListScan : public xxFlexLexer {

public:
  Text2ListScan( istream* is ) : xxFlexLexer( is ) {};
  ~Text2ListScan(){};

  int getLine() { return yylineno; }
  void SetDebug( const int value ) { yy_flex_debug = value; }

};


int xxlex()
{
  return (lexPtr->yylex());
}

void xxerror( char* s )
{
  cmsg.error() << "SECONDO Parser: " << s 
               << " in line " 
               << dynamic_cast<Text2ListScan*>(lexPtr)->getLine() 
               << endl;
  cmsg.send();
}

SecParser::SecParser()
{
}

SecParser::~SecParser()
{
}

int
SecParser::Text2List( const string& inputString, string& outputString )
{
  istringstream is( inputString );
  outputString = "";
  
  Text2ListScan lex( &is );
 
  // this must be a global variable, where is it defined?
  lexPtr = &lex;

  yacc_outtext = &outputString;
  NestedText::ReleaseStorage();

  if ( RTFlag::isActive("SecondoScanner:Debug") ) {
    lex.SetDebug(1);
  } else {
    lex.SetDebug(0);
  }
  if ( RTFlag::isActive("SecondoParser:Debug") ) {
    xxdebug = 1;
  } else {
    xxdebug = 0;
  }


  return ( xxparse() );
}


