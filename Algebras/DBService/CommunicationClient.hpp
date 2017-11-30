/*

1.1 \textit{CommunicationClient}

The \textit{CommunicationClient} is the counterpart of the
\textit{CommunicationServer}. Whenever communication associated with the
 \textit{DBService's} functionality takes place the \textit{CommunicationClient}
is the initiator. The \textit{CommunicationClient} is deducted from the more
generic class \textit{Client} of the \textit{Distributed2Algebra} which had
to be extracted from the \textit{FileTransferClient} for this purpose.

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
#ifndef ALGEBRAS_DBSERVICE_CommunicationClient_HPP_
#define ALGEBRAS_DBSERVICE_CommunicationClient_HPP_

#include <vector>

#include "Algebras/Distributed2/Client.h"

#include "Algebras/DBService/LocationInfo.hpp"
#include "Algebras/DBService/TraceWriter.hpp"

namespace DBService {

/*

1.1.1 Class Definition

*/

class CommunicationClient: public distributed2::Client
{

public:
/*

1.1.1.1 Constructor

*/
    CommunicationClient(
            std::string& server,
            int port,
            Socket* socket);

/*

1.1.1.2 Destructor

*/
    ~CommunicationClient();

/*

1.1.1.2 \textit{triggerReplication}

This function is meant to be called by on \textit{CommunicationClient} that
resides on the original node of a relation that needs to be replicated.
It establishes a connection to the \textit{CommunicationServer} on the
\textit{DBService master node} and provides all relevant information about the
relation that shall be replicated, such as database name, relation name and
original location.

*/
    bool triggerReplication(
            const std::string& databaseName,
            const std::string& relationName);

/*

1.1.1.2 ~giveStartingSignalForReplication~

This function is meant to be called by on \textit{CommunicationClient} that
resides on the original node of a relation that needs to be replicated.
It establishes a connection to the \textit{CommunicationServer} on the
\textit{DBService master node} and gives the starting signal for the
replication, so that the ~DBService~ worker nodes can be notified and request
file transfer.

*/
    bool giveStartingSignalForReplication(
            const std::string& databaseName,
            const std::string& relationName);

/*
1.1.1.2 \textit{getReplicaLocation}

This function establishes a connection to the \textit{CommunicationServer} on
the DBService master node in order to request the replica location for a
certain location. It passes the database name and relation name and receives
the host and the transfer port of the node that holds the replica.

*/
    bool getReplicaLocation(
            const std::string& databaseName,
            const std::string& relationName,
            const std::vector<std::string>& otherObjects,
            std::string& host,
            std::string& transferPort,
            std::string& commPort);

/*
1.1.1.2 \textit{triggerFileTransfer}

The file transfer between the nodes was implemented following the approach that
is already used by the \textit{Distributed2Algebra}. The
\textit{ReplicationServer} and \textit{ReplicationClient} of the
\textit{DBServiceAlgebra} are deducted from the \textit{FileTransferServer} and
the \textit{FileTransferClient} of the \textit{Distributed2Algebra}.
Therefore, the \textit{ReplicationServer} can only send files and the
\textit{ReplicationClient} can only receive files. This means if we want to
ship a replica from the original location to a \textit{DBService} worker node,
the \textit{DBService} worker node needs to instantiate a
\textit{ReplicationClient} that connects to the \textit{ReplicationServer} that
is running on the original node and request the file transfer.
The \textit{triggerFileTransfer} function is responsible for notifying the
respective \textit{DBService} worker and providing it with the host and port
of the responsible \textit{ReplicationServer}.

*/
    int triggerFileTransfer(
            const std::string& transferServerHost,
            const std::string& transferServerPort,
            const std::string& databaseName,
            const std::string& relationName);

/*
1.1.1.2 \textit{reportSuccessfulReplication}

Once a relation was successfully replicated, the \textit{DBService} worker node
has to notify the \textit{DBService} master so that the corresponding metadata
can be updated. Therefore a \textit{CommunicationClient} needs to be
instantiated that connects to the \textit{CommunicationServer} on the DBService
master and executes the function \textit{reportSuccessfulReplication}.

*/
    bool reportSuccessfulReplication(
            const std::string& databaseName,
            const std::string& relationName);

/*
1.1.1.3 \textit{reportSuccessfulDerivation}

Once ain object was successfully derived from a relation, 
the \textit{DBService} worker node has to notify the 
\textit{DBService} master so that the corresponding metadata
can be updated. Therefore a \textit{CommunicationClient} needs to be
instantiated that connects to the \textit{CommunicationServer} on the DBService
master and executes the function \textit{reportSuccessfulDerivation}.

*/
    bool reportSuccessfulDerivation(const std::string& objectId);

/*
1.1.1.2 \textit{requestReplicaDeletion}

If a relation is deleted at its original location, it does not make sense to
keep the corresponding replicas. Therefore, using the function
\textit{requestReplicaDeletion}, of a \textit{CommunicationClient} on
the original node, one can request the deletion of the replicas by contacting
the \textit{CommunicationServer} on the DBService master.

*/
    bool requestReplicaDeletion(
            const std::string& databaseName,
            const std::string& relationName,
            const std::string& derivateName);


/*
1.1.1.2 \textit{triggerReplicaDeletion}

Once the deletion of a replica has been requested, the \textit{DBService}
master will determine the replica locations and instantiate
\textit{CommunicationClients} to establish connections to each of them in order
to trigger the deletion of the replicas by calling function
\textit{triggerReplicaDeletion}. If derivateName is empty, the relation will 
be removed.


*/
    bool triggerReplicaDeletion(
            const std::string& databaseName,
            const std::string& relationName,
            const std::string& derivateName);

/*
1.1.1.2 ~pingDBService~

This function allows checking the ~DBService~ availability.

*/
    bool pingDBService();

/*
1.1.1.2 ~getRelType~

This function allows checking whether a relation exists in the DBService and
provides the corresponding tuple type in this case.

*/

    bool getRelType(const std::string& relID, std::string& nestedListAsString);

/*
1.1.1.1 ~getDerivedType~

This operator retrieves the type of a derived object from connected server.

*/

bool getDerivedType( const std::string& relID, 
                     const std::string& derivedName, 
                     std::string& nestedListAsString);

/*
1.1.1.3 ~triggerDerivation~

This function triggers an creation of a new object using a relation as well as
a function definition.

*/
    bool triggerDerivation( const std::string& databaseName,
                            const std::string& targetName,
                            const std::string& relName,
                            const std::string& fundef);


/*
1.1.1.4 ~createDerivation~

This function sends the command for creating a derivated object from a 
relation to the connected server   

*/
    bool createDerivation( const std::string& databaseName,
                            const std::string& targetName,
                            const std::string& relName,
                            const std::string& fundef);


/*
1.1.1.2 \textit{getLocationParameter}

This function simplifies reading information from the "Environment" section of
a SECONDO configuration file.

*/
private:
    void getLocationParameter(std::string& location, const char* key);

/*

1.1.1.2 \textit{start}

This function starts the \textit{CommunicationClient} by connecting to the
specified server.

*/
    int start();

/*

1.1.1.2 \textit{connectionTargetIsDBServiceMaster}

This function checks whether the \textit{CommunicationServer} to which we would
like to connect is actually the one on the \textit{DBService} master node.

*/
    bool connectionTargetIsDBServiceMaster();

/*

1.1.1.1 \textit{traceWriter}

As the \textit{DBService} acts in a highly distributed environment, it
is important to provide comprehensive tracing in order to be able to understand
the behaviour of the individual components which is important for potential
error analysis. Therefore, each communication client holds its own instance of
a \textit{TraceWriter} object which documents all important events in a trace
file.

*/
    std::unique_ptr<TraceWriter> traceWriter;
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_CommunicationClient_HPP_ */
