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
#include "Algebras/DBService2/DatabaseSchema.hpp"
#include "Algebras/DBService2/Node.hpp"
#include "Algebras/DBService2/Relation.hpp"
#include "Algebras/DBService2/Replica.hpp"
#include "Algebras/DBService2/Derivative.hpp"
#include "Algebras/DBService2/DatabaseAdapter.hpp"

namespace DBService {
  void DatabaseSchema::migrate(string database) {    
    shared_ptr<DatabaseAdapter> adapter = DatabaseAdapter::getInstance();

    // Ensure the database exists.
    adapter->createDatabase(database);

    // TODO Find a way to avoid repeating the RecordTypes
    createRelation<DBService::Node>(database, adapter);    
    createRelation<DBService::Relation>(database, adapter);    
    createRelation<DBService::Replica>(database, adapter);    
    createRelation<DBService::Derivative>(database, adapter);    
  }

  void DatabaseSchema::truncate(std::string database) {

    // TODO Find a way to avoid repeating the RecordTypes
    DBService::Node::deleteAll(database);
    DBService::Relation::deleteAll(database);
    DBService::Replica::deleteAll(database);
    DBService::Derivative::deleteAll(database);
  }
}