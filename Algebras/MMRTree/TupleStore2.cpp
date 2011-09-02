

/*
----
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software{} you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation{} either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY{} without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO{} if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----


//[_] [\_]

*/
#include "TupleStore2.h"
#include "TupleIdentifier.h"
#include "RelationAlgebra.h"
#include <vector>

/*
~Constructor~

The parameter memSite is ignored in this implementation

*/
TupleStore2::TupleStore2(const size_t memSize) {}

/*
~Destructor~

*/
TupleStore2::~TupleStore2(){
  for(size_t i=0;i<elems.size();i++){
     elems[i]->DeleteIfAllowed();
  }
}

/*
~AppendTuple~

Appends a tuple to this TupleStore. The return value can be used to
access the tuple from the store. The tuple is not changed by this 
operation.

*/
TupleId TupleStore2::AppendTuple(Tuple* t){
   TupleId res = elems.size();
   elems.push_back(t);
   t->IncReference();
   return res; 
}

/*
~GetTuple~

Returns the tuple for a given tupleId. If the tuple does not exist,
0 is returned.

*/

Tuple* TupleStore2::GetTuple(const TupleId id){
   if( (id<0) || (id>=elems.size())){
      return 0;
   }
   Tuple* res = elems[id];
   res->IncReference();
   return res;
}

