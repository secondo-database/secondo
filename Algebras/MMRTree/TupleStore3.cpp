


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

#include "TupleStore3.h"
#include "RelationAlgebra.h"
#include "TupleIdentifier.h"


/*
~Constructor~

The maxMem parameter is ignored.

*/
 TupleStore3::TupleStore3(size_t maxMem) : rel(0){}

/*
~Destructor~

*/
 TupleStore3::~TupleStore3(){
   if(rel){
      rel->DeleteAndTruncate();
      rel = 0;
   }
 }

/*
~AppendTuple~

Appends a tuple to this store. The id of the tuple is
changed by this operation. The new tuple id is also the
return value of this function.

*/
 TupleId TupleStore3::AppendTuple(Tuple* t){
    assert(t);
    if(!rel){
      rel = new Relation(t->GetTupleType(), true);
    }
    t->PinAttributes();
    rel->AppendTupleNoLOBs(t);
    return t->GetTupleId();
 }

/*
~GetTuple~

Retrieves a tuple from this store. If no tuple with given
id is present, 0 is returned.

*/

 Tuple* TupleStore3::GetTuple(const TupleId tid){
    if(!rel){
      return 0;
    }
    return rel->GetTuple(tid, false);
 }



