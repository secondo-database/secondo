/*
----
This file is part of SECONDO.

Copyright (C) 2016,
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


//[$][\$]
//[_][\_]

*/
#ifndef ALGEBRAS_DBSERVICE_MULTICLIENTSERVER_HPP_
#define ALGEBRAS_DBSERVICE_MULTICLIENTSERVER_HPP_

#include <iostream>
#include <queue>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#include "SocketIO.h"

#include "Algebras/Distributed2/Server.h"

class Socket;

namespace DBService {

class MultiClientServer : public distributed2::Server {
public:
    explicit MultiClientServer(int port);
    virtual ~MultiClientServer();
    int start();
protected:
    virtual int communicate(std::iostream& io) = 0;
    bool handleCommunicationThread();
private:
    std::queue<Socket*> socketBuffer;
    boost::mutex queueGuard;
    boost::condition_variable queueIndicator;
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_MULTICLIENTSERVER_HPP_ */
