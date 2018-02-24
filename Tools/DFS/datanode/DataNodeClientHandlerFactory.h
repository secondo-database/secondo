/*
----
This file is part of SECONDO.
Realizing a simple distributed filesystem for master thesis of stephan scheide

Copyright (C) 2015,
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

#ifndef DFS_DATANODECLIENTHANDLERFACTORY_H
#define DFS_DATANODECLIENTHANDLERFACTORY_H

#include "../commlayer/comm.h"
#include "DataNode.h"
#include "DataNodeClientHandler.h"

namespace dfs {

  class DataNodeClientHandlerFactory : public dfs::comm::ClientHandlerFactory {
  private:

    int freeHandlerId;
    int maxHandlerCount;

    const DataNode *dataNode;
    DataNodeClientHandler *handlers;
    dfs::log::Logger *logger;
    State *nodeState;

    void debug(const Str &s) {
      logger->debug(s);
    }

  public:

    DataNodeClientHandlerFactory(const DataNode *dataNode, int maxHandlerCount,
                                 dfs::log::Logger *l);

    ~DataNodeClientHandlerFactory();

    virtual dfs::comm::ClientHandler *createHandler();
  };
};

#endif //DFS_DATANODECLIENTHANDLERFACTORY_H
