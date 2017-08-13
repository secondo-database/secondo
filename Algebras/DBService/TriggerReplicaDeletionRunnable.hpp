/*

1.1 ~TriggerReplicaDeletionRunnable~

This class is used to trigger the deletion of a replica in a separate thread.

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
#ifndef ALGEBRAS_DBSERVICE_COMMUNICATIONTriggerReplicaDeletionRunnable_HPP_
#define ALGEBRAS_DBSERVICE_COMMUNICATIONTriggerReplicaDeletionRunnable_HPP_

#include <boost/thread.hpp>

/*

1.1.1 Class Definition

*/

namespace DBService {

class TriggerReplicaDeletionRunnable {

/*

1.1.1.1 Constructor

*/
public:
    TriggerReplicaDeletionRunnable(
            std::string dbServiceWorkerHost,
            int dbServiceWorkerCommPort,
            std::string relID);

/*

1.1.1.1 Destructor

*/
    ~TriggerReplicaDeletionRunnable();

/*

1.1.1.1 ~run~

This function creates a separate thread in which the deletion of a replica can
be triggered.

*/
    void run();

/*

1.1.1.1 ~createClient~

This function is passed to the new thread. It creates and runs a
~CommunicationClient~ which contacts the ~CommunicationServer~ running on the
~DBService~ worker node that holds the replica that shall be deleted.

*/
private:
    void createClient(
            std::string dbServiceWorkerHost,
            int dbServiceWorkerCommPort,
            std::string relID);

/*

1.1.1.1 ~runner~

Stores a pointer to the created thread.

*/
    boost::thread* runner;

/*

1.1.1.1 ~dbServiceWorkerHost~

Stores the host name of the ~DBService~ worker that holds the replica.

*/
    std::string dbServiceWorkerHost;

/*

1.1.1.1 ~dbServiceWorkerCommPort~

Stores the communication port of the ~DBService~ worker that holds the replica.

*/
    int dbServiceWorkerCommPort;

/*

1.1.1.1 ~relID~

Stores the identifier of the relation that shall be deleted.

*/
    std::string relID;
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_COMMUNICATIONTriggerReplicaDeletionRunnable_HPP_ */
