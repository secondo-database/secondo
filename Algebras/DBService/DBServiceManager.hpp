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

#include <memory>

#include <boost/shared_ptr.hpp>

#include "ConnectionInfo.h"
#include "RelationInfo.hpp"
#include "DBServiceCommunicationServer.hpp"

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
    void addNode(const std::string host,
                 const int port,
                 std::string config,
				 const int commPort);

/*
1.2 replicateRelation

//TODO

*/

    bool replicateRelation(const std::string& relationName);

/*
1.2 replicateRelation

//TODO

*/
    distributed2::ConnectionInfo* getConnection(ConnectionID id);

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
    ConnectionID getNextConnectionID();
    void getWorkerNodesForReplication(std::vector<
                                      ConnectionID>& nodes);
    bool startFileTransferServer(distributed2::ConnectionInfo* connectionInfo,
    		const int commPort);
    bool persistLocationInformation();

    static DBServiceManager* _instance;
    static std::map<ConnectionID, distributed2::ConnectionInfo*> connections;
    std::vector<boost::shared_ptr<RelationInfo> > replicaLocations;
    boost::shared_ptr<DBServiceCommunicationServer> commServer;

};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_DBSERVICEMANAGER_HPP_ */
