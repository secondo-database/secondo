/*

1 Asynchronous Operation

1.1 ~ServerRunnable~

The ~ServerRunnable~ is used to run a ~CommunicationServer~ or a
~ReplicationServer~ in a separate thread.

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
#ifndef ALGEBRAS_DBSERVICE_SERVERRUNNABLE_HPP_
#define ALGEBRAS_DBSERVICE_SERVERRUNNABLE_HPP_

#include <boost/thread.hpp>

namespace DBService {

/*

1.1.1 Class Definition

*/

class ServerRunnable {

/*

1.1.1.1 Constructor

*/
public:
    explicit ServerRunnable(int serverPort);

/*

1.1.1.1 Destructor

*/
    ~ServerRunnable();

/*

1.1.1.1 ~run~

By calling this function with either the class ~CommunicationServer~ or
~ReplicationServer~ as its template argument, the respective server instance is
created and started in a separate thread.

*/
    template <typename T>
    void run();

/*

1.1.1.1 ~createServer~

This function is passed to the created thread. It contains the logic to create
and run a server.

*/
private:
    template <typename T>
    void createServer(int port);

/*

1.1.1.1 ~runner~

Stores a pointer to the created thread.

*/
    boost::thread* runner;

/*

1.1.1.1 ~port~

Stores the port on which the created server shall listen.

*/
    int port;
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_SERVERRUNNABLE_HPP_ */
