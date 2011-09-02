

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

#ifndef TUPLESTORE3_H
#define TUPLESTORE3_H

#include "RelationAlgebra.h"
#include "TupleIdentifier.h"


/*
1.4.3 class TupleStore3 

This class stores all incoming tuples within a relation.
No cache is used for the tuples.

*/

class TupleStore3{
   public:
/*
~Constructor~

The maxMem parameter is ignored.

*/
     TupleStore3(size_t maxMem);

/*
~Destructor~

*/
     ~TupleStore3();

/*
~AppendTuple~

Appends a tuple to this store. The id of the tuple is
changed by this operation. The new tuple id is also the
return value of this function.

*/
     TupleId AppendTuple(Tuple* t);

/*
~GetTuple~

Retrieves a tuple from this store. If no tuple with given
id is present, 0 is returned.

*/

     Tuple* GetTuple(const TupleId tid);

   private:
     Relation* rel; // internal store
};





#endif


