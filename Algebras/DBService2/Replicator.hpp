/*

1.1 ~Replicator~

The ~Replicator~ is used to replicate a relation in a separate thread.

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
#ifndef ALGEBRAS_DBSERVICE_REPLICATOR_HPP_
#define ALGEBRAS_DBSERVICE_REPLICATOR_HPP_

#include <boost/thread.hpp>

namespace DBService {

/*

1.1.1 Class Definition

*/

class Replicator {

/*

1.1.1.1 Constructor

*/
public:
    explicit Replicator(
            std::string& dbName,
            std::string& relName,
            ListExpr type);

/*

1.1.1.1 Destructor

*/
    ~Replicator();

/*

1.1.1.1 ~run~

When calling this function, a new thread is created in which the replication
will be triggered.

*/
    void run(const bool async);

/*

1.1.1.1 ~createReplica~

This function is passed to the new thread. It triggers the replication of a
relation.

*/
private:
    void createReplica(
            const std::string databaseName,
            const std::string relationName,
            const ListExpr relType,
            const bool async);

/*

1.1.1.1 ~runner~

Stores a pointer to the created thread.

*/
    boost::thread* runner;

/*

1.1.1.1 ~databaseName~

Stores the name of the database in which the relation that shall be replicated
resides.

*/
    std::string databaseName;

/*

1.1.1.1 ~relationName~

Stores the name of the relation that shall be replicated.

*/
    std::string& relationName;

/*

1.1.1.1 ~relType~

Stores the type of the relation in a nested list.

*/
    ListExpr relType;
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_Replicator_HPP_ */
