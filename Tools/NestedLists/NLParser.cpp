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


#include <mutex>

#include "LogMsg.h"
#include "NLScanner.h"
#include "NLParser.h"



using namespace std;

extern CMsg cmsg;


/*
The generated parser is a pure one, so what a parse works on is passed to it as
an ~NLParseCtx~ rather than exchanged through variables of file scope. Nothing
about a parse is reachable from anywhere else, and two of them may run at once.

*/

extern int yydebug;
extern int yyparse( NLParseCtx* ctx );

int
NLParser::parse()
{
  NLScanner scanner( yaccnl, isp, osp );
  NLParseCtx ctx( yaccnl, &scanner );

  // yydebug is one variable for the whole generated parser, not something a
  // pure parser keeps per call, so it is written once instead of on every
  // parse -- otherwise each parse writes what the others are reading. The
  // runtime flags do not change after start-up, so once is enough.
  static std::once_flag debugSet;
  std::call_once( debugSet,
                  []() { yydebug = RTFlag::isActive("NLParser:Debug") 
                    ? 1 : 0; } );

  // The scanner's own flag, one per instance, so this one is per parse.
  scanner.set_debug( RTFlag::isActive("NLScanner:Debug") ? 1 : 0 );

  ctx.result = yaccnl->Empty();
  int rc = yyparse( &ctx );
  list = ctx.result;

  return rc;
}

/*
Providing function ~yyerror~

*/

void
yyerror( NLParseCtx* ctx, const char* s )
{
  cmsg.error()
    << "Nested-List Parser: " << endl << "  " << s
    << " processing token ~" << ctx->scanner->YYText() << "~"
    //<< setiosflags(ios::hex|ios::showbase)
    //<< static_cast<unsigned short>( yychar )
    //<< resetiosflags(ios::hex|ios::showbase)
    << " at line " << ctx->scanner->getLine()
    << " and col " << ctx->scanner->getCol() << "!" << endl
    << "LINE: " << ctx->scanner->getCurrentLine() << "$" << endl
    << endl;
  cmsg.send();
}

/*
Since function ~yylex()~ is a member function of class NLScanner we need to
wrap the call into another function which has global scope. Which scanner to
ask comes along in the context, so the wrapper holds nothing of its own.

*/

int
yylex( ListExpr* yylval_param, NLParseCtx* ctx )
{
  return ctx->scanner->yylex( yylval_param );
}
