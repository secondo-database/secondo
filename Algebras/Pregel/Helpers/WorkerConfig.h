/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header File of the class ~PregelAlgebra~

November 2018, J. Mende


[TOC]

1 Overview

This header file defines the class WorkerConfig

2 Defines and includes

*/

#ifndef SECONDO_WORKERINFO_H
#define SECONDO_WORKERINFO_H

#include <ostream>
#include "RemoteEndpoint.h"
#include "../typedefs.h"
#include <boost/log/trivial.hpp>

namespace pregel {
 struct WorkerConfig {
  friend std::ostream &
  operator<<(std::ostream &os, const WorkerConfig &config) {
   os << "slot: " << config.slot << " endpoint: " << config.endpoint
      << " messageServerPort: "
      << config.messageServerPort << " configFilePath: "
      << config.configFilePath << " connection: "
      << config.connection;
   return os;
  }

  int slot;
  RemoteEndpoint endpoint;
  int messageServerPort;
  std::string configFilePath;
  WorkerConnection *connection;

  WorkerConfig(int slot, const RemoteEndpoint endpoint, int messageServerPort,
               const std::string configFilePath,
               WorkerConnection *connection) : slot(slot), endpoint(endpoint),
                                               messageServerPort(
                                                messageServerPort),
                                               configFilePath(configFilePath),
                                               connection(connection) {}

  bool connect() {
   auto connection = WorkerConnection::createConnection(endpoint.host,
                                                        endpoint.port,
                                                        configFilePath);
   if (connection == nullptr) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't connect to worker.";
    return false;
   }
   this->connection = connection;
   return true;
  }

  virtual ~WorkerConfig() {
   if (connection == nullptr) {
    connection->killConnection();
    delete connection;
   }
  }
 };
}


#endif //SECONDO_WORKERINFO_H
