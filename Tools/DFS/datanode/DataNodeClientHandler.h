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

#ifndef DATANODECLIENTHANDLER_H
#define DATANODECLIENTHANDLER_H

#include "../commlayer/comm.h"
#include "../shared/log.h"
#include "State.h"
#include <sys/stat.h>
#include "DataNode.h"

namespace dfs {
  class DataNodeClientHandler : public dfs::comm::ClientHandler {
  private:
    //0: undefined, single command
    //1: begin of file
    //2: pure content
    int current;

    void _dr(const Str &s) {
      if (!this->logger->canDebug) return;
      this->logger->debug(
        Str("DataNodeClientHandler ").append(id).append(" ").append(s));
    }

  public:

    /**
     * an id of the handler within the factory
     */
    int id;

    /**
     * a logger for the handler
     */
    dfs::log::Logger *logger;

    /**
     * reference to the data node
     */
    const DataNode *dataNode;

    //state of this handler
    //0: free
    //1: returned by factory
    //2: active
    int state;

    DataNodeClientHandler() {
      id = -1;
      state = 0;
      current = 0;
    }

    /**
     * called when the handler is first called by the factory
     */
    void init() {}

    virtual void onStart();

    virtual Str onReceived(Str *s, int *resultFlags);

    virtual void onEnd();

  };
};
#endif /* DATANODECLIENTHANDLER_H */

