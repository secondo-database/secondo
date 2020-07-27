/*

1.1 ~ReplicationClientRunnable~

This class is used in order to run a ~ReplicationClient~ in a separate thread.

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
#ifndef ALGEBRAS_DBSERVICE_REPLICATIONCLIENTRUNNABLE_HPP_
#define ALGEBRAS_DBSERVICE_REPLICATIONCLIENTRUNNABLE_HPP_

#include <string>

#include <boost/thread.hpp>

namespace DBService {

/*

1.1.1 Class Definition

*/

class ReplicationClientRunnable {
public:

/*

1.1.1.1 Constructor

*/
    ReplicationClientRunnable(std::string targetHost,
                int targetTransferPort,
                std::string databaseName,
                std::string relationName);

/*

1.1.1.1 Destructor

*/
        ~ReplicationClientRunnable();

/*

1.1.1.1 ~run~

This function is executed in order to create a separate thread in which the
~ReplicationClient~ will be run.

*/
        void run();

/*

1.1.1.1 ~create~

This function is passed to the new thread and creates a ~ReplicationClient~
which connects to the specified host and requests the transfer of a file
containing the specified relation.

*/
    private:
        void create(
                std::string& targetHost,
                int targetTransferPort,
                std::string& databaseName,
                std::string& relationName);

/*

1.1.1.1 ~targetHost~

Stores the name of the host on which the ~ReplicationServer~ is running.

*/
        std::string targetHost;

/*

1.1.1.1 ~targetTransferPort~

Stores the number of the port on which the ~ReplicationServer~ is listening.

*/
        int targetTransferPort;

/*

1.1.1.1 ~databaseName~

Stores the name of the database in which the relation resides.

*/
        std::string databaseName;

/*

1.1.1.1 ~relationName~

Stores the name of the relation.

*/
        std::string relationName;

/*

1.1.1.1 ~runner~

Stores a pointer to the created thread.

*/
        boost::thread* runner;
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_REPLICATIONCLIENTRUNNABLE_HPP_ */
