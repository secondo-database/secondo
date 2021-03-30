/*

1.1 \textit{DBServiceManager}

The \textit{DBServiceManager} is the central component of the
\textit{DBService} system. It is involved by the \textit{CommunicationServer}
on the \textit{DBService} master node in order to coordinate the execution of
all \textit{DBService} functionality.

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
#ifndef ALGEBRAS_DBSERVICE_DBSERVICEMANAGER_HPP_
#define ALGEBRAS_DBSERVICE_DBSERVICEMANAGER_HPP_

#include <memory>
#include <iostream>

#include <boost/shared_ptr.hpp>

#include <loguru.hpp>

#include "Algebras/Distributed2/ConnectionInfo.h"

#include "Algebras/DBService2/CommunicationServer.hpp"
#include "Algebras/DBService2/LocationInfo.hpp"
#include "Algebras/DBService2/RelationInfo.hpp"
#include "Algebras/DBService2/DerivateInfo.hpp"
#include "Algebras/DBService2/NodeManager.hpp"
#include "Algebras/DBService2/RelationManager.hpp"
#include "Algebras/DBService2/FaultToleranceMode.hpp"
#include "Algebras/DBService2/ReplicaPlacementStrategy.hpp"

/*

1.1.1 Type Definitions

*/

namespace DBService
{

/*

1.1.1.1 ~DBServiceLocations~

Maps a ~ConnectionID~ to the corresponding ~LocationInfo~ and ~ConnectionInfo~
objects.
TODO Remove
*/
typedef std::map<ConnectionID,
        std::pair<LocationInfo,
                  distributed2::ConnectionInfo*> > DBServiceLocations;

/*

1.1.1.1 ~DBServiceRelations~

Maps a relation identified to the corresponding ~RelationInfo~ object.
TODO Remove
*/
typedef std::map<std::string, RelationInfo> DBServiceRelations;


/*
1.1.1.1 ~DBServiceDerivates~
TODO Remove
*/
typedef std::map<std::string, DerivateInfo> DBServiceDerivates;


/*

1.1.1 Class Definition

*/

class DBServiceManager
{
public:

/*

1.1.1.1 getInstance

Returns the DBServiceManager instance (singleton).

*/
    static DBServiceManager* getInstance();

/*

1.1.1.1 isActive

Returns whether the DBServiceManager is considered as active.

*/
    static bool isActive();

/*

1.1.1.1 isUsingIncrementalMetadataUpdate

Returns whether the DBServiceManager is updating the metadata incrementally

*/
    static bool isUsingIncrementalMetadataUpdate();

/*

1.1.1.1 useIncrementalMetadataUpdate

Specify whether or not to update metadata incrementally.

 */
    static void useIncrementalMetadataUpdate(bool use);

/*

1.1.1.1 \textit{addNode}

This function adds a node to the connection manager's pool that can be used for
storing relation replicas.

*/
    bool addNode(const std::string host,
                 const int port,
                 std::string config);


/*

1.1.1.1 \textit{addDerivative}

This function adds a derivative to the specified relation and triggers 
the creation of derivative replicas.

*/

    void addDerivative(std::string relationDatabase, std::string relationName, 
        std::string derivativeName, std::string derivativeFunction);

/*

1.1.1.1 \textit{getRelation}

Returns the Relation specified by ~relationDatabase~ and ~relationName~.
Return the ~nullptr~ if none has been found.

*/
std::shared_ptr<DBService::Relation> getRelation(
    std::string relationDatabase, 
    std::string relationName);

/*
1.1.1.1 ~printDerivates~

Debug method. prints all known derivates.

*/
   void printDerivates(std::ostream& out) const;



/*

1.1.1.1 \textit{determineReplicaLocations}

This function determines the replica locations for a certain relation and is
therefore provided with the name of the database and relation as well as all
relevant information on the original location.

Returns false if the placement violated the replication policy, e.g. due to
insufficient number of replicas (nodes qualified for a replica placement).

*/
    bool determineReplicaLocations(
            const std::string& databaseName,
            const std::string& relationName,
            const std::string& host,
            const std::string& port,
            const std::string& disk);


/*

1.1.1.1 \textit{maintainSuccessfulReplication}

This function updates the internal data structures after a successful
replication has been reported. It also triggers updating the persistent metadata
relations accordingly.

*/
    void maintainSuccessfulReplication(
            const std::string& relID,
            const std::string& replicaLocationHost,
            const std::string& replicaLocationPort);

/*

1.1.1.1 ~deleteReplicaMetadata~

This function removes the metadata of a certain relation from the internal data
structures and from the persistent metadata relations.
If the derivbateName is empty, the relation and all depending objects 
will be removed.

*/
    void deleteReplicaMetadata(const std::string& database,
                               const std::string& relation,
                               const std::string& derivateName);


/*

1.1.1.1 ~printMetadata~

This function prints all DBService metadata used for replica provisioning to
the command line.

*/
    void printMetadata(std::ostream& out);

/*

1.1.1.1 ~replicaExists~

This function returns whether a replica exists in ~DBService~ for the specified
database and relation name.

*/
        bool replicaExists(
                const std::string& databaseName,
                const std::string& relationName);


/*
1.1.1.1 ~derivateExists~

This function checks whether a derivate with given id exists.

*/
    bool derivateExists(const std::string& objectId);

/*
1.1.1.1 ~getRandomNodeWithReplica~

Returns the target Node a random replica determined by the
Relation found by the provided ~relationDatabase~ and ~relationName~.

Returns ~nullptr~ if no relation and/or replicas are found.
*/
std::shared_ptr<DBService::Node> getRandomNodeWithReplica(
            std::string relationDatabase, std::string relationName);


/*
1.1.1.1 ~maintainSuccessfulDerivation~

This function is called after a successful creation of a derivate.

*/
        void maintainSuccessfulDerivation(
            const std::string& objectID,
            const std::string& replicaLocationHost,
            const std::string& replicaLocationPort);
/*
1.1.1.1 ~getMessages~~

Get messages from the DBServiceManager.
Currently only returns messages from the ~ReplicaPlacementStrategy~.
*/
std::string getMessages();

/*
1.1.1.1 Constructor

*/
private:
    DBServiceManager();

/*
1.1.1.1 Copy Constructor

*/
    DBServiceManager(const DBServiceManager&)
    {}

/*
1.1.1.1 Destructor

Deletes existing DBServiceManager instance.

*/
    ~DBServiceManager();


/*

1.1.1.1 \textit{getConfigParamFromWorker}

This function retrieves information from the workers that is only stored in
their local configuration files.

*/
    bool getConfigParamFromWorker(std::string& dir,
            distributed2::ConnectionInfo* connectionInfo, const char* section,
            const char* key);


/*

1.1.1.1 \textit{restoreConfiguration}

On \textit{DBServiceManager} instantiation, this function restores the
connections from the persistent relation in case it exists. It reopens all
available connections.

*/
    void restoreConfiguration();

/*

1.1.1.1 \textit{restoreReplicaPlacementStrategyConfig}

Creates a \textit{ReplicaPlacementStrategy} based on the DBService config.

*/

    void restoreReplicaPlacementStrategyConfig();

/*

1.1.1.1 \textit{nodeManager}

The \textit{NodeManager} encapsulates the lifecycle management of nodes. 
It maintains a list of nodes, provides accessor and persistency methods.
This reduces the complexity of the \testit{DBServiceManager}.

*/

    std::unique_ptr<NodeManager> nodeManager;


/*

1.1.1.1 \textit{relationManager}

Similar to the \textit{nodeManager} the \textit{relationManager} is also
a specialization of a \textit{RecordManager}. 
It allows the management of Relations.

*/

    std::unique_ptr<RelationManager> relationManager;

/*

1.1.1.1 \textit{replicaPlacementStrategy}

The \textit{ReplicaPlacementStrategy} encapsulates the decision process
of selecting nodes to place Replicas to. 

*/
    
    std::shared_ptr<ReplicaPlacementStrategy> replicaPlacementStrategy;

/*

1.1.1.1 \textit{database}

The name of the main database of the \textit{DBService} algebra.

*/
std::string database;


/*


1.1.1.1 \textit{\_instance}

Pointer to the \textit{DBServiceManager} instance (singleton).

*/
    static DBServiceManager* _instance;

/*

1.1.1.1 \textit{connections}

This member maps a \textit{ConnectionID} to a pair that contains the
corresponding \textit{LocationInfo} and \textit{ConnectionInfo}.
TODO Remove
*/    
    DBServiceLocations connections;

/*

1.1.1.1 \textit{possibleReplicaLocations}

This member maps a location identifier to a vector of possible replica
locations.

TODO Remove
*/
    typedef std::map<std::string, std::vector<ConnectionID> >
    AlternativeLocations;

    AlternativeLocations possibleReplicaLocations;

/*

1.1.1.1 \textit{relations}

This member maps a relation identifier to the corresponding
\textit{RelationInfo} object.
TODO Remove
*/
    DBServiceRelations relations;

/*
1.1.1.1 ~derivates~

This manager maps a derivate identifier to the corresponding
~DerivateInfo~ object.
TODO Remove
*/
   DBServiceDerivates derivates;




/*

1.1.1.1 \textit{replicaCount}

This member stores the target number of replicas that is read from the
configuration file.

*/
    size_t replicaCount;


FaultToleranceMode mode;

/*

1.1.1.1 ~managerMutex~

Mutex used to coordinate multi-threaded access by different servers.

*/

boost::mutex managerMutex;

/*

1.1.1.1 ~active~

Indicates whether the DBService is active in terms of loaded metadata and open
connections to all workers.

*/

static bool active;

/*

1.1.1.1 ~useIncrementalMetadataUpdate~

Indicates whether the DBService updates the metadata incrementally instead of
deleting and recreating it on each change.

*/

static bool usesIncrementalMetadataUpdate;

};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_DBSERVICEMANAGER_HPP_ */
