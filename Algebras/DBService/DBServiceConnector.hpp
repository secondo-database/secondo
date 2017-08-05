/*
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
#ifndef ALGEBRAS_DBSERVICE_DBSERVICECONNECTOR_HPP_
#define ALGEBRAS_DBSERVICE_DBSERVICECONNECTOR_HPP_

#include <string>

#include <boost/shared_ptr.hpp>

#include "NList.h"

#include "Algebras/DBService/CommunicationClient.hpp"
#include "Algebras/DBService/LocationInfo.hpp"

namespace DBService {

/*

1 \textit{DBServiceConnector}

The \textit{DBServiceConnector} is the central component of the system which
shall be equipped with fault-tolerant query execution. The
\textit{DBServiceConnector} is the single point of entry for all operators
which need access to the \textit{DBService}.

*/

class DBServiceConnector {
public:

/*

1.1 Function Definitions

The \textit{DBServiceConnector} provides several member functions of which each
covers a scenario where an operator needs to access \textit{DBService}
functionality.


1.1.1 getInstance

Returns the DBServiceConnector instance (singleton).

*/
    static DBServiceConnector* getInstance();

/*
1.1.1 \textit{triggerReplication}

This function needs to be called if a relation shall be replicated.

*/
    bool triggerReplication(
            const std::string& databaseName,
            const std::string& relationName,
            const ListExpr relType);


/*
1.1.1 \textit{retrieveReplicaAndGetFileName}

This function triggers the transfer of a file containing the replica of the
specified relation and returns the file name to the caller. The file name can
then be used to read a tuple stream from the file.

*/
    std::string retrieveReplicaAndGetFileName(
            const std::string& databaseName,
            const std::string& relationName,
            const std::string&
            functionAsNestedListString);

/*
1.1.1 \textit{deleteReplicas}

In case a relation is deleted locally, its replicas are also not longer needed.
This function triggers the deletion of the replicas as well as the removal of
all metadata related to this relation on the \textit{DBService} master node.

*/
    bool deleteReplicas(
            const std::string& databaseName,
            const std::string& relationName);

private:
/*
1.1.1 Constructor

*/
    DBServiceConnector();

/*
1.1.1 Destructor

*/
    ~DBServiceConnector();

/*
1.1.1 \textit{startReplicationServer}

This function is called by the constructor and starts a
\textit{ReplicationServer} on the current node.

*/
    void startReplicationServer();

/*
1.1.1 \textit{getReplicaLocation}

This function retrieves one of the replica locations of a relation from the
\textit{DBService} master. It is only called internally by the
\textit{retrieveReplicaAndGetFileName} function.

*/
    bool getReplicaLocation(
            const std::string& databaseName,
            const std::string& relationName,
            std::string& host,
            std::string& transferPort);

/*

1.1 Member Definitions

The \textit{DBServiceConnector} has some members that store information which
is important for its functionality.

1.1.1 \textit{dbServiceHost}

Hostname of the node on which the \textit{DBService} master resides.

*/
    std::string dbServiceHost;

/*

1.1.1 \textit{dbServicePort}

Port of the \textit{CommunicationServer} on the \textit{DBService} master node.

*/
    std::string dbServicePort;

/*

1.1.1 \textit{\_instance}

Pointer to the \textit{DBServiceConnector} instance (singleton).

*/
    static DBServiceConnector* _instance;
};


} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_DBSERVICECONNECTOR_HPP_ */
