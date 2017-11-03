/*

1.1 ~CreateDerivateRunnable~

This class is used to sends a command for creating a derivate to a server.

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
#ifndef CREATEDERIVATERUNNABLE_HPP 
#define CREATEDERIVATERUNNABLE_HPP 

#include <boost/thread.hpp>

/*

1.1.1 Class Definition

*/

namespace DBService {

class CreateDerivateRunnable {

/*

1.1.1.1 Constructor

*/
public:
    CreateDerivateRunnable(
            std::string dbServiceWorkerHost,
            int dbServiceWorkerCommPort,
            const std::string& database,
            const std::string& targetName,
            const std::string& relname,
            const std::string& fundef);

/*

1.1.1.1 Destructor

*/
    ~CreateDerivateRunnable();

/*

1.1.1.1 ~run~

This function creates a separate thread in which the creation of a derivate
is send to a server.

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
            std::string database,
            std::string targetName,
            std::string relname,
            std::string fundef);


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

    std::string dbname;
    std::string targetname;
    std::string relname;
    std::string fundef;
};

} /* namespace DBService */

#endif 
