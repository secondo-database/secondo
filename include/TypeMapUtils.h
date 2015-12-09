/*
---- 
This file is part of SECONDO.

Copyright (C) 2004-2007, University in Hagen, Faculty of Mathematics
 and Computer Science, 
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

May 2007, M. Spiekermann. Initial version.
Dec 2009, M. Spiekermann. Comment added.

This file contains some utility functions which can be used for 
simple type mappings. Typically much operators have very simple signatures
and are candidates for this general implementaton.

*/

#ifndef SEC_TYPEMAP_UTILS
#define SEC_TYPEMAP_UTILS


#include <string>
#include <sstream>

#include "CharTransform.h"
#include "NestedList.h"
#include "NList.h"


namespace mappings {

/*
1 Auxiliary functions not intended as interface

*/
bool
CheckSimpleMap(const std::string map[], size_t n, ListExpr args, ListExpr& res);


/*
2 Programming Interface


The function below can be used for type mappings which map
n-1 atomic types to another atomic type, e.g. 

----
    int x int -> bool
----

The atomic types must be encoded as string constants in an array map.
Code examples which use this interface can be studied in the 
module ~StandardAlgebra~.

*/	

ListExpr
SimpleMap(const std::string map[], size_t n, ListExpr arg);

template<int n, int m>
ListExpr
SimpleMaps(const std::string map[n][m], ListExpr args) 
{ 
  ListExpr res;	
  bool ok = false;	
  for(size_t i = 0; i < n; i++ ) {
    ok = CheckSimpleMap(map[i], m, args, res);
    if (ok)
      return res;	    
  }
  std::stringstream err;
  err << "None of " << n << " alternative mappings matches!";
  // to do: more precise error messages
  return NList().typeError(err.str());
}	

/*
The next two functions are a variant for overloaded operators.
The template parameters n and m denote that the operator has n-1 
arguments and m signatures.

*/

template<int n, int m>
int
SimpleSelect(const std::string map[n][m], ListExpr args) 
{ 	
  ListExpr res; // dummy arg	
  bool ok = false;	
  for(size_t i = 0; i < n; i++ ) {
    ok = CheckSimpleMap(map[i], m, args, res);
    if (ok)
      return i;	    
  }
  std::string err="SimpleSelect failed!";
  // to do: more precise error messages
  NList().typeError(err);
  return -1;
}

}

#endif
