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


We generate a C++ scanner since we want to use C++ ~ostream~ references instead of
files for the input sequences.

*/

#ifndef NL_SCANNER_H
#define NL_SCANNER_H

#include <cassert>
#include <sstream>
#include <string>

#include "NestedList.h"

#ifndef yyFlexLexer
#define yyFlexLexer nlFlexLexer
#include <FlexLexer.h>
#endif

#ifndef YY_CURRENT_BUFFER
#define YY_CURRENT_BUFFER ( (yy_buffer_stack) \
                          ? (yy_buffer_stack)[(yy_buffer_stack_top)] \
                          : NULL)
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

  ~NLScanner();

  // Overrules yyFlexLexer's yylex(). Takes the place to put the semantic
  // value, because the parser is a pure one and so has no global yylval.
  int yylex( ListExpr* yylval_param );

  // With %option yyclass, flex generates the overload above and leaves the
  // no-argument yylex() that yyFlexLexer declares unimplemented. Overridden
  // here rather than merely hidden by the overload, so that the compiler does
  // not have to warn about it. It must never be called.
  int yylex() { assert( false ); return 0; }

/*
Where the scanner has got to, for the error message. Kept per scanner rather
than in globals, so that a second parse running at the same time reports its
own position instead of this one's.

*/
  int getLine() const { return line; }
  int getCol() const { return col; }
  const std::string& getCurrentLine() const { return currentLine; }


 private:
  // no Scanner copy-initialization
  NLScanner( NLScanner const &other );

  // no assignment either
  NLScanner &operator=( NLScanner const &other );

  NestedList* lexnl;

  // Input position, maintained by the rules in NLLex.l.
  int line;
  int col;
  std::string currentLine;

  // The text atom being collected while the scanner is inside <text>...
  // </text---> or a quoted string. Per scan, so it belongs to the scanner.
  std::ostringstream* text;
};

#endif

