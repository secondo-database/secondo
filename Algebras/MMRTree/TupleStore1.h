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
/*
1.4.3 TupleStore1

This tuple store used a chache and a relation to store tuples.


*/
#ifndef TUPLESTORE1_H
#define TUPLESTORE1_H

#include "RelationAlgebra.h"
#include "TupleIdentifier.h"
#include <vector>


class TupleStore1{
public:

/*
~Constructor~

This constructor creates a new tuple store. The memSize determines the
size of the used cache in kB.

*/
  TupleStore1(const size_t memSize);


/*
~AppendTuple~

This function appends a tuple to this tuple store.
The id of the tuple may be changed or not. To search for a certain
tuple within this tuple store, only the return value of this funtion
can be used.  Note that the returned tuple ids are not nessecary increasing.

*/
  TupleId AppendTuple(Tuple* t);
/*
~Destructor~

*/
  ~TupleStore1();

/*
~GetTuple~

Retrieves a tuple by id. If no corresponding tuple is found,
0 is returned.

*/

  Tuple* GetTuple(const TupleId tid);

private:
   vector<Tuple*> firstElems; // cache
   Relation*      overflow;   // persistent structure
   size_t         maxMem;     // maximum cache size
   size_t         usedMem;    // current cache size
   bool           useOverflow;
};


#endif

