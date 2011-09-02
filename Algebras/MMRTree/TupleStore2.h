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

#ifndef TUPLESTORE2_H
#define TUPLESTORE2_H

#include "RelationAlgebra.h"
#include "TupleIdentifier.h"
#include <vector>


class TupleStore2{
  public:
/*
~Constructor~

The parameter memSite is ignored in this implementation

*/
    TupleStore2(const size_t memSize);

/*
~Destructor~

*/
    ~TupleStore2();

/*
~AppendTuple~

Appends a tuple to this TupleStore. The return value can be used to
access the tuple from the store. The tuple is not changed by this 
operation.

*/
    TupleId AppendTuple(Tuple* t);

/*
~GetTuple~

Returns the tuple for a given tupleId. If the tuple does not exist,
0 is returned.

*/

  Tuple* GetTuple(const TupleId id);

  private:
     vector<Tuple*> elems;
};

#endif

