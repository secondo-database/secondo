/*
----
This file is part of SECONDO.

Copyright (C) 2004-2007, University in Hagen, Faculty of Mathematics and Computer Science,
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

July 2007, M. Spiekermann. Initial version.

This file contains some functions which can be used for simple type mappings.

*/


#include <string>
#include <sstream>

#include "CharTransform.h"
#include "NestedList.h"
#include "NList.h"

#include "TypeMapUtils.h"

using namespace std;

bool
mappings::CheckSimpleMap( const string map[], size_t n,
		          ListExpr args, ListExpr& res  ) {

  assert(n>=1);

  stringstream err;
  NList l(args);
  size_t i = 0;

  if ( n == 1 ) { // operator maps (empty) -> map[0]
    if( !l.isEmpty() ) {
      err << "Expecting an empty input list but got " << l << "!";
      res = l.typeError( err.str() );
      return false;
    }
  }
  else
  {
  if (l.length() != n-1) {
    err << "Expecting a list of length " << n-1
        << " but got " << l << "!";
    res = l.typeError( err.str() );
    return false;
  }

  while ( i < n-1 ) {

    NList sym = l.elem(i+1);
    if ( !sym.isSymbol(map[i]) )
    {
     string err;
     if(sym.hasStringValue())
     {
       err = "Symbol number " + int2Str(i) + " has incorrect type."
             " Expected " + map[i] + " but got " + sym.str() + "!";
     }
     else
     {
       err = "Symbol number " + int2Str(i) + " has incorrect type."
             " Expected " + map[i] + " but got a complex type!";
     }

      res = l.typeError(err);
      return false;
    }
    i++;
  }
  }
  res = NList(map[i]).listExpr();
  return true;
}

ListExpr
mappings::SimpleMap(const string map[], size_t n, ListExpr args)
{
  ListExpr res;
  CheckSimpleMap(map, n, args, res);
  return res;
}

