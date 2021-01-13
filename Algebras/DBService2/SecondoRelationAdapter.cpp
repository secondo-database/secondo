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
#include "Algebras/DBService2/SecondoRelationAdapter.hpp"

#include "NestedList.h"

using namespace std;

extern NestedList* nl;

namespace DBService {
  
  shared_ptr<DBService::Relation> 
    SecondoRelationAdapter::buildObjectFromNestedList(string database, 
    ListExpr recordAsNestedList) {
    /*

      Name: string, Database: string, OriginalNodeId: int

              Name : relation1
          Database : dbservice_test
    OriginalNodeId : 7
               TID : 1
    */

    string relationDatabase;
    string relationName;
    int originalNodeId;
    int id;

    /*
      TODO try to make generic.

      make_shared -> use default constructor () (without args)

      create a map:
        nestedListIndex (1, ...) ->
          { 
            SecondoType function pointer (e.g. nl->StringValue),
            Record setter function pointer (e.g. record->setDatabase)
          }
      
      Then each Record type only has to offer the attribute mapping.
    */

    relationName      = nl->StringValue(nl->Nth(1, recordAsNestedList));
    relationDatabase  = nl->StringValue(nl->Nth(2, recordAsNestedList));
    originalNodeId    = nl->IntValue(nl->Nth(3, recordAsNestedList));
    id                = nl->IntValue(nl->Nth(4, recordAsNestedList));

    shared_ptr<DBService::Relation> relation = DBService::Relation::build(
      relationDatabase, relationName);
    
    // Record
    relation->setDatabase(database);    
    relation->setId(id);

    // Eager load original node.
    /* TODO Does it make more sense to trigger this is an afterLoad callback?
        For this to happen, the originalNodeId needed to be an attribute of 
        the relation object.
    */
    relation->loadOriginalNode(originalNodeId);
    
    // Eacher load replicas
    relation->loadReplicas();
    relation->loadDerivatives();
    relation->setClean();
    relation->setNotNew();

    return relation;
  }
}