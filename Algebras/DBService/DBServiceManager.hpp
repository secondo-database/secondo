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


//[$][\$]
//[_][\_]

*/
#ifndef ALGEBRAS_DBSERVICE_DBSERVICEMANAGER_HPP_
#define ALGEBRAS_DBSERVICE_DBSERVICEMANAGER_HPP_

#include <memory>

#include <boost/shared_ptr.hpp>

#include "Algebras/Distributed2/ConnectionInfo.h"

#include "Algebras/DBService/CommunicationServer.hpp"
#include "Algebras/DBService/LocationInfo.hpp"
#include "Algebras/DBService/RelationInfo.hpp"

namespace DBService
{

/*

1 \textit{DBServiceManager}

The \textit{DBServiceManager} is the central component of the
\textit{DBService} system. It is involved by the \textit{CommunicationServer}
on the \textit{DBService} master node in order to coordinate the execution of
all \textit{DBService} functionality.

*/

class DBServiceManager
{
public:

/*
1.1 Function Definitions

The \textit{DBServiceManager} provides several member functions that help
realizing the necessary functionality.

1.1.1 getInstance

Returns the DBServiceManager instance (singleton).

*/
    static DBServiceManager* getInstance();

/*

1.1.1 \textit{addNode}

This function adds a node to the connection manager's pool that can be used for
storing relation replicas.

*/
    bool addNode(const std::string host,
                 const int port,
                 std::string config);

/*

1.1.1 \textit{getConnection}

This function returns a pointer to the \textit{ConnectionInfo} object identified
by the specified \textit{ConnectionID}.

*/
    distributed2::ConnectionInfo* getConnection(ConnectionID id);

/*

1.1.1 \textit{getLocation}

This function returns a reference to the \textit{LocationInfo} object identified
by the specified \textit{ConnectionID}.

*/
    LocationInfo& getLocation(ConnectionID id);

/*

1.1.1 \textit{getRelationInfo}

This function returns a reference to the \textit{RelationInfo} object identified
by the specified string.

*/
    RelationInfo& getRelationInfo(const std::string& relationAsString);

/*

1.1.1 \textit{determineReplicaLocations}

This function determines the replica locations for a certain relation and is
therefore provided with the name of the database and relation as well as all
relevant information on the original location.

*/
    void determineReplicaLocations(
            const std::string& databaseName,
            const std::string& relationName,
            const std::string& host,
            const std::string& port,
            const std::string& disk);

/*

1.1.1 \textit{persistReplicaLocations}

This function persists the identified replica locations in case the replication
shall be executed. It triggers storing them in a persistent SECONDO table
as well as adding them to internal data structures.

*/
    void persistReplicaLocations(
            const std::string& databaseName,
            const std::string& relationName);

/*

1.1.1 \textit{getReplicaLocations}

This function retrieves all replica locations of the relation identified by the
given string and stores them in the provided vector.

*/
    void getReplicaLocations(const std::string& relationAsString,
                             std::vector<std::pair<ConnectionID, bool> >& ids);

/*

1.1.1 \textit{deleteReplicaLocations}

This function deletes the replica information of the specified relation from
the internal data structures.

*/
    void deleteReplicaLocations(const std::string& databaseName,
                                const std::string& relationName);

/*

1.1.1 \textit{maintainSuccessfulReplication}

This function updates the internal data structures after a successful
replication has been reported. It also triggers updating the persistent metadata
relations accordingly.

*/
    void maintainSuccessfulReplication(
            const std::string& relID,
            const std::string& replicaLocationHost,
            const std::string& replicaLocationPort);

/*

1.1.1 \textit{}

This function removes the metadata of a certain relation from the internal data
structures and from the persistent metadata relations.

*/
    void deleteReplicaMetadata(const std::string& relID);

/*
1.1.1 Constructor

*/
private:
    DBServiceManager();

/*
1.1.1 Copy Constructor

*/
    DBServiceManager(const DBServiceManager&)
    {}

/*
1.1.1 Destructor

Deletes existing DBServiceManager instance.

*/
    ~DBServiceManager();

/*

1.1.1 \textit{getNextFreeConnectionID}

This function determines the next free \textit{ConnectionID} which is used as
unique identifier of connections.

*/
    ConnectionID getNextFreeConnectionID();

/*

1.1.1 \textit{getWorkerNodesForReplication}

This function adds the specified number of replicas to the given vector.

*/
    void getWorkerNodesForReplication(std::vector<
                                      ConnectionID>& nodes,
                                      const std::string& host,
                                      const std::string& disk);

/*

1.1.1 \textit{startServersOnWorker}

This function triggers the startup of one \textit{CommunicationServer} and one
\textit{ReplicationServer} on the node that is addressed by the specified
\textit{ConnectionInfo}.

*/
    bool startServersOnWorker(distributed2::ConnectionInfo* connectionInfo);

/*

1.1.1 \textit{getConfigParamFromWorker}

This function retrieves information from the workers that is only stored in
their local configuration files.

*/
    bool getConfigParamFromWorker(std::string& dir,
            distributed2::ConnectionInfo* connectionInfo, const char* section,
            const char* key);

/*

1.1.1 \textit{addToPossibleReplicaLocations}

This function maintains the potential replica locations for a locations
in consideration of the configured fault tolerance mode.

*/
    void addToPossibleReplicaLocations(
            const ConnectionID connectionID,
            const LocationInfo& location,
            std::vector<ConnectionID>& potentialReplicaLocations,
            const std::string& host,
            const std::string& disk);

/*

1.1.1 \textit{restoreConfiguration}

On \textit{DBServiceManager} instantiation, this function restores the
connections from the persistent relation in case it exists. It reopens all
available connections.

*/
    void restoreConfiguration();

/*

1.1.1 \textit{restoreReplicaInformation}

On \textit{DBServiceManager} instantiation, this function restores all replica
information from the persistent relations in case they exist.

*/
    void restoreReplicaInformation();
/*

1.1 Member Definitions

The \textit{DBServiceManager} has some members that store information which
is important for its functionality.

1.1.1 \textit{\_instance}

Pointer to the \textit{DBServiceManager} instance (singleton).

*/
    static DBServiceManager* _instance;

/*

1.1.1 \textit{connections}

This member maps a \textit{ConnectionID} to a pair that contains the
corresponding \textit{LocationInfo} and \textit{ConnectionInfo}.

*/
    std::map<ConnectionID,
                    std::pair<LocationInfo,
                              distributed2::ConnectionInfo*> > connections;

/*

1.1.1 \textit{possibleReplicaLocations}

This member maps a location identifier to a vector of possible replica
locations.

*/

typedef std::map<std::string, std::vector<ConnectionID> > AlternativeLocations;

AlternativeLocations possibleReplicaLocations;

/*

1.1.1 \textit{relations}

This member maps a relation identifier to the corresponding
\textit{RelationInfo} object.

*/
    std::map<std::string, RelationInfo> relations;

/*

1.1.1 \textit{replicaCount}

This member stores the target number of replicas that is read from the
configuration file.

*/
    size_t replicaCount;

    /*

1.1.1 \textit{faultToleranceMode}

This member stores the target number of replicas that is read from the
configuration file.

*/

enum FaultToleranceMode
{
    NONE = 0,
    DISK = 1,
    NODE = 2,
};

FaultToleranceMode mode;


};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_DBSERVICEMANAGER_HPP_ */
