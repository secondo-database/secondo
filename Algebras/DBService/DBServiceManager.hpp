/*
----
This file is part of SECONDO.

Copyright (C) 2016,
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


//[$][\$]
//[_][\_]

*/
#ifndef ALGEBRAS_DBSERVICE_DBSERVICEMANAGER_HPP_
#define ALGEBRAS_DBSERVICE_DBSERVICEMANAGER_HPP_

#include "ConnectionInfo.h"
#include "RelationInfo.hpp"

namespace DBService
{

class DBServiceManager
{
public:

/*
1.1 getInstance

Returns the DBServiceManager instance (singleton).

*/
    static DBServiceManager* getInstance();

/*
1.1 getNodes

Returns pointers to the selected alternative storage locations
when provided with a node.

*/
    void getNodes();

/*
1.2 addNode

Adds a node to the connection manager's pool that can be used for
storing relation replicas.

*/
    static void addNode(const std::string host,
                        const int port,
                        std::string config);

/*
1.2 initialize

//TODO

*/
    static void initialize();
    static bool isInitialized();

    static bool replicateRelation(const std::string& relationName);
    static distributed2::ConnectionInfo* getConnection(ConnectionID id);

protected:
/*
1.2 Constructor

Creates a new DBServiceManager instance.

*/
    DBServiceManager();
/*
1.3 Copy Constructor

Does not do anything.

*/
    DBServiceManager(const DBServiceManager&)
    {}

/*
1.3 Destructor

Deletes existing DBServiceManager instance.

*/
    ~DBServiceManager();

private:
    static ConnectionID getNextConnectionID();
    static void getWorkerNodesForReplication(std::vector<
                                             ConnectionID>& nodes);

    static DBServiceManager* _instance;
    static bool initialized;
    static std::map<ConnectionID, distributed2::ConnectionInfo*> connections;
    static std::vector<std::shared_ptr<RelationInfo> > replicaLocations;

};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_DBSERVICEMANAGER_HPP_ */
