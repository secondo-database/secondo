/*

1.1.1 Class Implementation

----
This file is part of SECONDO.

Copyright (C) 2017,
Faculty of Mathematics and Computer Science,
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
#include "Algebras/DBService2/SecondoRecordAdapter.hpp"
#include "Algebras/DBService2/SecondoDerivativeAdapter.hpp"

#include "NestedList.h"

using namespace std;

extern NestedList* nl;


// extern boost::recursive_mutex nlparsemtx;

namespace DBService {
  
  shared_ptr<DBService::Derivative> 
    SecondoDerivativeAdapter::buildObjectFromNestedList(string recordDatabase, 
    ListExpr recordAsNestedList, int offset) {
    /*

      
    */

    string derivativeName;
    string derivativeFunction;
    int relationId;    
    int id;

// The lock must be acquired in the invoking method
//    boost::lock_guard<boost::recursive_mutex> guard(nlparsemtx);

    derivativeName      = nl->StringValue(nl->Nth(
        offset + 1, recordAsNestedList));

    derivativeFunction = nl->StringValue(nl->Nth(
      offset + 2, recordAsNestedList));

    relationId = nl->IntValue(nl->Nth(offset + 3, recordAsNestedList));

    id = nl->IntValue(nl->Nth(offset + 4, recordAsNestedList));

    shared_ptr<DBService::Derivative> derivative = DBService::Derivative::build(
      derivativeName, derivativeFunction);
    
    // Record
    derivative->setDatabase(recordDatabase);    
    derivative->setId(id);

    //Load Derivative Replicas
    derivative->loadReplicas();
  
    derivative->setClean();
    derivative->setNotNew();

    return derivative;
  }
}