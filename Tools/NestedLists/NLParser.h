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
#ifndef NL_PARSER_H
#define NL_PARSER_H

#include "NestedList.h"

class NLParser
{
 public:
  NLParser( NestedList* nestedList, 
            std::istream* ip = 0, 
            std::ostream* op = 0 ) 
  : 
  list(0), 
  isp( ip ), 
  osp( op ), 
  nl( nestedList )
  {}

  virtual ~NLParser() {};
  
  int         parse();
  ListExpr    GetNestedList() { return list; }

 private:
  ListExpr       list; // the constructed memory structure 
                       // of a nested list
  std::istream*  isp;  // istream being parsed
  std::ostream*  osp;  // ostream being output to
  
  NestedList*    nl;  // reference to the global NestedList instance

};

#endif
