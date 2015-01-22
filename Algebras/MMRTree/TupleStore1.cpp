

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

#include "TupleStore1.h"
#include "RelationAlgebra.h"
#include "TupleIdentifier.h"


/*
~Constructor~

This constructor creates a new tuple store. The memSize determines the
size of the used cache in kB.

*/
  TupleStore1::TupleStore1(const size_t memSize):
     firstElems(), overflow(0),maxMem(0), usedMem(0), useOverflow(false){
     if(memSize>0){
         maxMem = memSize;
         maxMem = maxMem * 1024u;
     }
  }


/*
~AppendTuple~

This function appends a tuple to this tuple store.
The id of the tuple may be changed or not. To search for a certain
tuple within this tuple store, only the return value of this funtion
can be used.  Note that the returned tuple ids are not nessecary increasing.

*/
  TupleId TupleStore1::AppendTuple(Tuple* t){
      assert(t); // don't allow inserting of null pointers
      size_t tm = t->GetMemSize();
      if(!useOverflow){
        if(usedMem + tm  < maxMem){
          size_t id = firstElems.size();
          firstElems.push_back(t);
          t->IncReference();
          usedMem += tm; 
          return (TupleId) id;
        }
        useOverflow = true; // switch to relation
      }
      // tuple must be inserted into relation
      if(!overflow){
         overflow = new Relation(t->GetTupleType(), true);
      }
      t->PinAttributes();
      overflow->AppendTupleNoLOBs(t);
      TupleId newId = t->GetTupleId();
      return (TupleId) (newId + firstElems.size()-1);
  }
/*
~Destructor~

*/
  TupleStore1::~TupleStore1(){
     if(overflow){
       overflow->DeleteAndTruncate();
     }
     for(size_t i=0; i< firstElems.size();i++){
        firstElems[i]->DeleteIfAllowed();
     }
     firstElems.clear();
  }

/*
~GetTuple~

Retrieves a tuple by id. If no corresponding tuple is found,
0 is returned.

*/

  Tuple* TupleStore1::GetTuple(const TupleId tid){
    if(tid < firstElems.size()){ // get from vector
       Tuple* res = firstElems[tid];
       res->IncReference();
       assert(res);
       return res;
    } else {
       return overflow->GetTuple(tid - firstElems.size()+1,false);
    }
  }

