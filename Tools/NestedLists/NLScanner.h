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

December 2004, M. Spiekermann. A debug mode, and variables for the position were 
introduced.

*/

#ifndef NL_SCANNER_H
#define NL_SCANNER_H

#include "NestedList.h"

#ifndef yyFlexLexer
#define yyFlexLexer nlFlexLexer
#include <FlexLexer.h>
#endif

class NLScanner: public yyFlexLexer
{
 public:         
  enum Error
  {
    invalidInclude,
    circularInclusion,
    nestingTooDeep,
    cantRead,
  };
                
  NLScanner( NestedList* nestedList, 
             std::istream* yyin = 0, std::ostream* yyout = 0 );

  int yylex();  // overruling yyFlexLexer's yylex()

  int lines;    // position in the input
  int cols;

  // toggle debug mode
  void SetDebug(const int value);

 private:
  // no Scanner copy-initialization
  NLScanner( NLScanner const &other ); 

  // no assignment either
  NLScanner &operator=( NLScanner const &other );   

  NestedList* nl;
};

#endif

