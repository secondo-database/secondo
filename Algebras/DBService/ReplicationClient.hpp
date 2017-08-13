/*

1.1 ~ReplicationClient~

The ~ReplicationClient~ connects to a ~ReplicationServer~ and requests the
~DBService~ worker node, in order to create a replica, or from an arbitrary
node in the original system, in order to request a replica.

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
#ifndef ALGEBRAS_DBSERVICE_REPLICATIONCLIENT_HPP_
#define ALGEBRAS_DBSERVICE_REPLICATIONCLIENT_HPP_

#include "Algebras/Distributed2/FileTransferClient.h"

#include "Algebras/DBService/LocationInfo.hpp"
#include "Algebras/DBService/TraceWriter.hpp"

namespace DBService {

/*

1.1.1 Class Definition

*/

class ReplicationClient: public distributed2::FileTransferClient
{
public:

/*

1.1.1.1 Constructor

*/
    ReplicationClient(
            std::string& server,
            int port,
            const std::string& fileNameDBS,
            const std::string& fileNameOrigin,
            std::string& databaseName,
            std::string& relationName);

/*

1.1.1.1 Destructor

*/
    ~ReplicationClient();

/*

1.1.1.1 ~start~

This function starts the ~ReplicationClient~ by opening the socket connection.

*/
    int start();

/*

1.1.1.1 ~receiveReplica~

This function is called in order to request the transmission of a replica
from the original node.

*/
    int receiveReplica();

/*

1.1.1.1 ~requestReplica~

This function is called in order to request the transmission of a replica from
the ~DBService~ worker node.

*/
    int requestReplica(
            const std::string& functionAsNestedListString,
            std::string& fileName);

/*

1.1.1.1 ~reportSuccessfulReplication~

By calling this function, the ~DBService~ master is notified about the
successful replication.

*/
    void reportSuccessfulReplication();

/*

1.1.1.1 ~receiveFileFromServer~

This function is called in order to start the file transfer.

*/
private:
    bool receiveFileFromServer();

/*

1.1.1.1 ~fileNameDBS~

Stores the name of the file that contains the replica on the local side.

*/
    const std::string fileNameDBS;

/*

1.1.1.1 ~fileNameOrigin~

Stores the name of the file that contains the replica on the remote side.

*/
    const std::string fileNameOrigin;

/*

1.1.1.1 ~databaseName~

Stores the database name of the relation.

*/
    std::string databaseName;

/*

1.1.1.1 ~relationName~

Stores the name of the relation.

*/
    std::string relationName;

/*

1.1.1.1 ~traceWriter~

Stores a pointer to the ~TraceWriter~ that is needed to write trace files.

*/
    std::unique_ptr<TraceWriter> traceWriter;
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_REPLICATIONCLIENT_HPP_ */
