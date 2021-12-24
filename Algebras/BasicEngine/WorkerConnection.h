/*
----
This file is part of SECONDO.

Copyright (C) 2021,
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

*/

#ifndef BASIC_ENGINE_WORKER_H
#define BASIC_ENGINE_WORKER_H

#include <mutex>
#include <boost/log/trivial.hpp>
#include "Algebras/Distributed2/ConnectionInfo.h"

namespace BasicEngine {

/*
1.2 Struct ~RemoteConnectionInfo~

*/
struct RemoteConnectionInfo {
  std::string host;
  std::string port;
  std::string config;
  std::string dbUser;
  std::string dbPass;
  std::string dbPort;
  std::string dbName;
};

class WorkerConnection {

    public:
    WorkerConnection(RemoteConnectionInfo* _connectionInfo) : 
        connectionInfo(_connectionInfo) {

    }

    virtual ~WorkerConnection() {
        if(connectionInfo != nullptr) {
            delete connectionInfo;
            connectionInfo = nullptr;
        }

        if(connection != nullptr) {
            BOOST_LOG_TRIVIAL(debug) 
                << "Releasing connection to " << connection->getHost() 
                << " / " << connection->getPort();
            connection->deleteIfAllowed();
            connection = nullptr;
        }
    }

    RemoteConnectionInfo* getRemoteConnectionInfo() {
        return connectionInfo;
    }

    distributed2::ConnectionInfo* getConnection() {
        return connection;
    }

    friend std::ostream& operator<<(std::ostream &os,
                                    const WorkerConnection &connection) {
                                        
      return os << "(WorkerConnection" << connection.connectionInfo->host << ","
                << connection.connectionInfo->port << ")"
                << "," << connection.connectionInfo->config;
    }

/**
 1.1 Establish the connection to the remote system
 
*/
    bool createConnection();

/**
 1.2 Execute a SECONDO command
 
*/
bool executeSecondoCommand(const std::string &command, const bool checkResult);

/**
 1.2 Execute a simple SECONDO command
 
*/
bool performSimpleSecondoCommand(const std::string &command);


/**
1.3 Get the send path of the connection

*/
std::string getSendPath() {
    return connection -> getSendPath();
}

/**
1.3 Get the send path of the connection

*/
std::string getRequestPath() {
    return connection -> getRequestPath();
}

/**
 1.3 The default timeout value
 
*/
  static const size_t defaultTimeout = 0;

/**
 1.4 the defailt heartbeat value
 
*/
  static const int defaultHeartbeat = 0;

    private:

/**
 2.1 The remote connction info

*/
    RemoteConnectionInfo* connectionInfo = nullptr;

/**
 2.2 The remote connection

*/
    distributed2::ConnectionInfo* connection = nullptr;

/**
 2.3 The connection mutex

*/
    std::mutex connectionMutex;

};

}

#endif