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

December 2004, M. Spiekermann. A debug mode for the scanner and parser have been
introduced. The error message will now help much better to locate errors.

*/

using namespace std;

#include "LogMsg.h"
#include "NLParser.h"
#include "NLScanner.h"
#include <iomanip>


extern NestedList* nl;
extern CMsg cmsg;

NLParser::NLParser( NestedList* nestedList, istream* ip, ostream* op )
  : isp( ip ), osp( op ), nl( nestedList )
{
  listExpression = 0;
  nlScanner = new NLScanner( nestedList, isp, osp );
}

NLParser::~NLParser()
{
  delete nlScanner;
}

int
NLParser::lex()
{
  if ( RTFlag::isActive("NLParser:Debug") ) {
    debug = 1;
  } else {
    debug = 0;
  }
  
  if ( RTFlag::isActive("NLScanner:Debug") ) {
    nlScanner->SetDebug(1);
  } else {
    nlScanner->SetDebug(0);
  }


  return (nlScanner->yylex());
}

void
NLParser::error( char* s )
{
  cmsg.error() << "Nested-List Parser: " << endl << "  " << s 
       << " processing character '" << nlScanner->YYText() 
       << "' (= " 
       << setiosflags(ios::hex|ios::showbase) 
       << static_cast<unsigned short>( *(nlScanner->YYText()) ) 
       << resetiosflags(ios::hex|ios::showbase) 
       << ") at line " << nlScanner->lines 
       << " and col " << nlScanner->cols << "!"
       << endl;
  cmsg.send();
}
