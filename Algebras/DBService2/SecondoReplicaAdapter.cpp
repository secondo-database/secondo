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
#include "Algebras/DBService2/SecondoReplicaAdapter.hpp"

#include "NestedList.h"

using namespace std;

extern NestedList* nl;

namespace DBService {
  
  shared_ptr<DBService::Replica> 
    SecondoReplicaAdapter::buildObjectFromNestedList(string database, 
    ListExpr recordAsNestedList) {
    /*

        RelationId : 3
      TargetNodeId : 10
            Status : waiting
              TID : 1
    */

    int relationId;
    int targetNodeId;
    string status;
    string type;
    int derivativeId;
    int id;

    int index = 0;

    relationId        = nl->IntValue(nl->Nth(++index, recordAsNestedList));
    targetNodeId      = nl->IntValue(nl->Nth(++index, recordAsNestedList));
    status            = nl->StringValue(nl->Nth(++index, recordAsNestedList));
    type              = nl->StringValue(nl->Nth(++index, recordAsNestedList));
    derivativeId      = nl->IntValue(nl->Nth(++index, recordAsNestedList));
    id                = nl->IntValue(nl->Nth(++index, recordAsNestedList));

    shared_ptr<DBService::Replica> replica = make_shared<DBService::Replica>();

    // Record attribute
    replica->setDatabase(database);

    // Replica attributes
    replica->setStatus(status); 
    replica->setType(type);

    replica->setId(id);
    replica->setClean();
    replica->setNotNew();
    
    replica->setRelationId(relationId);
    replica->setTargetNodeId(targetNodeId);    
    
    // Derivative id and derivative are being set by 
    // Derivative::loadReplicas as Derivative is meant to be the exclusive
    // owner to loading derivative Replicas.

    // WARNING: This may well be an in-memory duplicate.
    auto targetNode = DBService::Node::findByTid(database, targetNodeId);

    if (targetNode == nullptr)
      throw "Couldn't find targetNode of replica.";

    replica->setTargetNode(targetNode);
    return replica;
  }
}