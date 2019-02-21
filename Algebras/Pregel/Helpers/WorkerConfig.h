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
  private:
    void killConnection(){
       if(connection==nullptr){
         return;
       }
       *numRefs = *numRefs -1;
       if(*numRefs == 0){
          connection->killConnection();
          delete connection;
          delete numRefs;
       }
    }
  public:

  std::ostream& print(std::ostream& os) const{
     os << "slot: " << slot;
     os << " endpoint: " << endpoint;
     os << " messageServerPort: " << messageServerPort;
     os << " configFilePath: " << configFilePath;
     os << " connection: " << connection;
     return os;
  }

  int slot;
  RemoteEndpoint endpoint;
  int messageServerPort;
  std::string configFilePath;
  WorkerConnection* connection;
  size_t* numRefs;

  WorkerConfig(int slot, const RemoteEndpoint endpoint, int messageServerPort,
               const std::string configFilePath,
               WorkerConnection *connection) : slot(slot), endpoint(endpoint),
                                               messageServerPort(
                                                messageServerPort),
                                               configFilePath(configFilePath),
                                               connection(connection) {
     numRefs = new size_t;
     *numRefs = 0;
     if(connection) *numRefs = 1;
  }

  bool connect() {
   auto connection = WorkerConnection::createConnection(endpoint.host,
                                                        endpoint.port,
                                                        configFilePath);
   if (connection == nullptr) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't connect to worker.";
    return false;
   }
   killConnection();
   this->connection = connection;
   numRefs = new size_t;
   *numRefs = 1;
   return true;
  }

  WorkerConfig(const WorkerConfig& a): slot(a.slot), endpoint(a.endpoint),
                                       messageServerPort(a.messageServerPort),
                                       configFilePath(a.configFilePath),
                                       connection(a.connection),
                                       numRefs(a.numRefs) {
     *numRefs = *numRefs + 1;
  } 


  virtual ~WorkerConfig() {
      killConnection();
  }
 };


}

std::ostream& operator<<(std::ostream &os, const pregel::WorkerConfig &config);


#endif //SECONDO_WORKERINFO_H
