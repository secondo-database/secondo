/*

1.1 ~RelationInfo~

This object is used to store metadata about a relation for which replicas are
maintained by the ~DBService~.

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
#ifndef ALGEBRAS_DBSERVICE_RELATIONINFO_HPP_
#define ALGEBRAS_DBSERVICE_RELATIONINFO_HPP_

#include <string>
#include <vector>

#include "LocationInfo.hpp"
#include "MetadataObject.hpp"
#include "ReplicaLocations.hpp"

namespace DBService
{

/*

1.1.1 Class Definition

*/

class RelationInfo : public MetadataObject
{
public:

/*

1.1.1.1 Constructor

*/
    RelationInfo(const std::string& dbName,
                 const std::string& relName,
                 const std::string& host,
                 const std::string& port,
                 const std::string& disk);

/*

1.1.1.1 ~getDatabaseName~

This function returns the database name of the associated relation.

*/
    const std::string& getDatabaseName() const;

/*

1.1.1.1 ~getDatabaseName~

This function returns the name of the associated relation.

*/
    const std::string& getRelationName() const;

/*

1.1.1.1 ~addNode~

This function allows adding a node, representing a replica location, to the
relation.

*/
    void addNode(ConnectionID id);

/*

1.1.1.1 ~addNode~

This function allows adding a node, representing a replica location, to the
relation. When adding the node, it has to be specified whether the relation was
already successfully replicated.

*/
    void addNode(ConnectionID id, bool replicated);

/*

1.1.1.1 ~addNodes~

This function allows adding several nodes, representing replica locations, at
once.

*/
    void addNodes(std::vector<ConnectionID>& nodesToAdd);

/*

1.1.1.1 ~nodesBegin~

This function provides an iterator to the begin of the structure that stores
the nodes that were added as replica locations.

*/
    const ReplicaLocations::const_iterator nodesBegin() const;

/*

1.1.1.1 ~nodesEnd~

This function provides an iterator to the end of the structure that stores
the nodes that were added as replica locations.

*/
    const ReplicaLocations::const_iterator nodesEnd() const;

/*

1.1.1.1 ~getOriginalLocation~

This function allows retrieving the original location of the relation associated
with the ~RelationInfo~ object.

*/
    const LocationInfo& getOriginalLocation() const;

/*

1.1.1.1 ~setTransferPortOfOriginalLocation~

This function allows setting the transfer port of the original location.

*/
    void setTransferPortOfOriginalLocation(std::string& newPort);

/*

1.1.1.1 ~toString~

This function returns the relation identifier.

*/
    const std::string toString() const;

/*

1.1.1.1 ~getNodeCount~

This function returns the number of nodes that serve as replica location for the
associated relation.

*/
    const size_t getNodeCount();

/*

1.1.1.1 ~getRandomReplicaLocation~

This function picks a random node and returns it.

*/
    const ConnectionID getRandomReplicaLocation();

/*
1.1.1.1 ~getAllLocations~

Returns the connection ids of all nodes having the
replica flag set to be true.

*/
    void getAllLocations(std::vector<ConnectionID>& result);

/*

1.1.1.1 ~updateReplicationStatus~

This function updates the replication status with regards to a certain replica.

*/
    void updateReplicationStatus(ConnectionID connID, bool replicated);


/*
1.1.1.1 ~getIdentifier~

*/
   inline static std::string getIdentifier(const std::string& database,
                                    const std::string& relation){
      return database + separator + relation;
   }


/*

1.1.1.1 ~databaseName~

Stores the database name of the associated relation.

*/
private:
    const std::string databaseName;

/*

1.1.1.1 ~relationName~

Stores the name of the associated relation.

*/
    const std::string relationName;

/*

1.1.1.1 ~nodes~

Stores the replica locations of the associated relation.

*/
    ReplicaLocations nodes;

/*

1.1.1.1 ~originalLocation~

Stores the original location of the associated relation.

*/
    LocationInfo originalLocation;

};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_RELATIONINFO_HPP_ */
