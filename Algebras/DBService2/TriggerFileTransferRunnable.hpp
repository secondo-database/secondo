/*

1.1 ~TriggerFileTransferRunnable~

This class is used in order to trigger the transfer of a file in a separate
thread.

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
#ifndef ALGEBRAS_DBSERVICE_COMMUNICATIONTriggerFileTransferRunnable_HPP_
#define ALGEBRAS_DBSERVICE_COMMUNICATIONTriggerFileTransferRunnable_HPP_

#include <boost/thread.hpp>

namespace DBService {

/*

1.1.1 Class Definition

*/

class TriggerFileTransferRunnable {

/*

1.1.1.1 Constructor

*/
public:
    TriggerFileTransferRunnable(
            std::string sourceSystemHost,
            int sourceSystemTransferPort,
            std::string dbServiceWorkerHost,
            int dbServiceWorkerCommPort,
            std::string databaseName,
            std::string relationName);

/*

1.1.1.1 Destructor

*/
    ~TriggerFileTransferRunnable();

/*

1.1.1.1 ~run~

This function creates a new thread in which the file transfer can be triggered.

*/
    void run();

/*

1.1.1.1 ~createClient~

This function is passed to the new thread. It creates and runs a
~CommunicationClient~ that connects to the ~CommunicationServer~ on the
~DBService~ worker node in order to trigger the replication.

*/
private:
    void createClient(
            std::string sourceSystemHost,
            int sourceSystemTransferPort,
            std::string dbServiceWorkerHost,
            int dbServiceWorkerCommPort,
            std::string databaseName,
            std::string relationName);

/*

1.1.1.1 ~runner~

Stores a pointer to the created thread.

*/
    boost::thread* runner;

/*

1.1.1.1 ~sourceSystemHost~

Stores the host name of the original system.

*/
    std::string sourceSystemHost;

/*

1.1.1.1 ~sourceSystemTransferPort~

Stores the port on which the ~ReplicationServer~ in the original system listens.

*/
    int sourceSystemTransferPort;

/*

1.1.1.1 ~dbServiceWorkerHost~

Stores the host name of the ~DBService~ worker node.

*/
    std::string dbServiceWorkerHost;

/*

1.1.1.1 ~dbServiceWorkerCommPort~

Stores the port on which the ~CommunicationServer~ on the ~DBService~ worker
node listens.

*/
    int dbServiceWorkerCommPort;

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
    std::string relationName;
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_COMMUNICATIONTriggerFileTransferRunnable_HPP_ */
