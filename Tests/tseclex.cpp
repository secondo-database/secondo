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

*/
#include <iostream>
#include <sstream>
#include "NestedText.h"

#include <stdio.h>
#include <string.h>

#ifndef yyFlexLexer
#define yyFlexLexer xxFlexLexer
#include <FlexLexer.h>
#endif

int xxlval;

int main()
{
  string str = "query Staedte feed filter[.Bev > 100000] consume";
  istringstream is( str );
  xxFlexLexer lex( &is );
  int token;
  xxlval = 0;

  cout << "lex input=" << str << endl;
  token = lex.yylex();
  while (token != 0)
  {
    cout << "Token " << token << " lval=" << xxlval << " "; 
    if (xxlval != 0)
    {
      NestedText::Print( xxlval );
      xxlval = 0;
    }
    cout << endl;
    token = lex.yylex();
  }
}

