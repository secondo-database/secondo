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

September 2006, M. Spiekermann. The Parser and Scanner have been revised in
order to avoid that the generated bison file needs to be modified. The old
version altered the generated code by sed scripts into a C++ class (which was
incompatible with newer bison versions). Now we have still a class NLParser but
it calls the external function ~yyparse~ provided by bison. 

*/

using namespace std;

#include "LogMsg.h"
#include "NLScanner.h"
#include "NLParser.h"

extern NestedList* nl;
extern CMsg cmsg;


/*
Below we declare some external variables for data exchange between class
NLParser and the bison and flex generated code. 

*/

NestedList* parseNL_nl = 0;
extern ListExpr parseNL_list;

extern int scanNL_lines;
extern int scanNL_cols;
extern string scanNL_str;
extern int yydebug;
extern int yyparse();


/*
Before we can call yyparse we need to construct a new scanner instance.

*/

static NLScanner* nlScanner = 0;

int
NLParser::parse() 
{ 
  nlScanner = new NLScanner( nl, isp, osp );
  if ( RTFlag::isActive("NLParser:Debug") ) {
    yydebug = 1;
  } else {
    yydebug = 0;
  }
  
  if ( RTFlag::isActive("NLScanner:Debug") ) {
    nlScanner->set_debug(1);
  } else {
    nlScanner->set_debug(0);
  }
 
  scanNL_lines = 1;
  scanNL_cols = 0;
  scanNL_str = "";
  parseNL_nl = nl;
  parseNL_list = nl->Empty();  
  int rc = yyparse();
  list = parseNL_list;
  
  delete nlScanner;
  nlScanner = 0;
  return rc;
}

/*
Providing function ~yyerror~

*/

void
yyerror( char* s )
{
  cmsg.error() 
    << "Nested-List Parser: " << endl << "  " << s
    << " processing token ~" << nlScanner->YYText() << "~"
    //<< setiosflags(ios::hex|ios::showbase)
    //<< static_cast<unsigned short>( yychar )
    //<< resetiosflags(ios::hex|ios::showbase)
    << " at line " << scanNL_lines
    << " and col " << scanNL_cols << "!" << endl
    << "LINE: " << scanNL_str << "$" << endl
    << endl;
  cmsg.send();
}

/*
Since function ~yylex()~ is a member function of class NLScanner we need to
wrap the call into another function which has global scope.

*/

int
yylex()
{
  return nlScanner->yylex();
}
