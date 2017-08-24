/*

1.1 \textit{CommunicationServer}

The \textit{CommunicationServer} is the counterpart of the
\textit{CommunicationClient}. Whenever communication associated with the
 \textit{DBService's} functionality takes place,
 the \textit{CommunicationServer} is contacted by a
 \textit{CommunicationClient}.
 The \textit{CommunicationServer} is deducted from the more generic class
 \textit{Server} of the \textit{Distributed2Algebra} which had to be extracted
 from the \textit{FileTransferServer} for this purpose. The second superclass
 of the \textit{CommunicationServer} is \textit{MultiClientServer}, as we only
 want to have one \textit{CommunicationServer} per node which is able to handle
 multiple client requests.

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
#ifndef ALGEBRAS_DBSERVICE_CommunicationServer_HPP_
#define ALGEBRAS_DBSERVICE_CommunicationServer_HPP_

#include "Algebras/DBService/MultiClientServer.hpp"

class Socket;

namespace DBService
{

/*

1.1.1 Class Definition

*/

class CommunicationServer: public MultiClientServer
{
/*

1.1.1.1 Constructor

*/
public:
    explicit CommunicationServer(int port);
/*

1.1.1.1 Destructor

*/
    virtual ~CommunicationServer();

/*

1.1.1.1 \textit{start}

This function is called to start the ~CommunicationServer~

*/

int start();

/*

1.1.1.1 \textit{communicate}

This function is called as soon as an incoming connection from a
\textit{CommunicationClient} is detected. Based on the used keywords, it decides
which function shall be executed.

*/

protected:
    int communicate(std::iostream& io);
/*

1.1.1.1 \textit{handleTriggerReplicationRequest}

This function is executed when a \textit{CommunicationClient} on the original
node of a relation requests its replication. The \textit{DBServiceManager} on
the \textit{DBService} master node is contacted in order to determine suitable
replica locations and if a sufficient number of worker nodes is available, the
replication is triggered.

*/
    bool handleStartingSignalRequest(
            std::iostream& io,
            const boost::thread::id tid);

/*

1.1.1.1 ~handleStartingSignalRequest~

This function is executed when a \textit{CommunicationClient} on the original
node of a relation gives the starting signal for the replication.
The replication is triggered by notifying the ~DBService~ workers

*/
    bool handleTriggerReplicationRequest(
            std::iostream& io,
            const boost::thread::id tid);

/*

1.1.1.1 \textit{handleTriggerFileTransferRequest}

This function is executed when a \textit{CommunicationClient} residing on the
 \textit{DBService} master node requests triggering the file transfer between
 the \textit{DBService} worker node and the original node.
 A \textit{ReplicationClient} is instantiated that connects to the
 \textit{ReplicationServer} on the original node in order to request the file
 transfer.

*/
    bool handleTriggerFileTransferRequest(
            std::iostream& io,
            const boost::thread::id tid);

/*

1.1.1.1 \textit{handleProvideReplicaLocationRequest}

This function retrieves one of the replica locations from the
\textit{DBServiceManager} on the \textit{DBService} master node and provides
it to the connected \textit{CommunicationClient}.

*/
    bool handleProvideReplicaLocationRequest(
            std::iostream& io,
            const boost::thread::id tid);

/*

1.1.1.1 \textit{reportSuccessfulReplication}

This function notifies the \textit{DBServiceManager} on the \textit{DBService}
master node so that the successful replication can be maintained in the
corresponding mapping table.

*/
    bool reportSuccessfulReplication(
            std::iostream& io,
            const boost::thread::id tid);

/*

1.1.1.1 \textit{handleRequestReplicaDeletion}

This function retrieves all replica locations from the
\textit{DBServiceManager} on the \textit{DBService} master node and initializes
one \textit{CommunicationClient} for each worker node which triggers the
deletion of the replica.

*/
    bool handleRequestReplicaDeletion(
            std::iostream& io,
            const boost::thread::id tid);

/*

1.1.1.1 \textit{handleTriggerReplicaDeletion}

This function triggers the deletion of a certain replica on the
\textit{DBService} worker node where the \textit{CommunicationServer} is
running.

*/
    bool handleTriggerReplicaDeletion(
            std::iostream& io,
            const boost::thread::id tid);

/*

1.1.1.1 \textit{handlePing}

This function triggers reacts on the ping of a client by sending a ping
back.

*/
    bool handlePing(
            std::iostream& io,
            const boost::thread::id tid);

/*

1.1.1.1 ~handleRelTypeRequest~

This function provides the connected client with the type of a relation for
which a replica is stored in the ~DBService~.

*/
        bool handleRelTypeRequest(
                std::iostream& io,
                const boost::thread::id tid);

/*

1.1.1.1 \textit{lookupMinimumReplicaCount}

This function retrieves the minimum number of replicas from the configuration
file and stores it in the corresponding member variable.

*/
private:
    void lookupMinimumReplicaCount();
/*

1.1.1.1 \textit{minimumReplicaCount}

One configuration parameter of the \textit{DBService} is the number of replicas
that shall be available for each relation. As it does not make sense to read
this information from the configuration file again every time a relation shall
be replicated, each \textit{CommunicationServer} looks it up once during
initialization. However, it is actually only used on the \textit{DBService}
master node.

*/
    int minimumReplicaCount;
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_CommunicationServer_HPP_ */
