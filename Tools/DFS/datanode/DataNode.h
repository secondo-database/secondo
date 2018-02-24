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

#ifndef DATANODE_H
#define DATANODE_H

#include "../commlayer/comm.h"
#include "../shared/log.h"
#include "State.h"
#include "../shared/str.h"
#include "DataNodeConfig.h"

namespace dfs {

  class DataNode {
  private:
    Str maschineId;
    State *pNodeState;

    void debug(const Str &s) { this->pLogger->debug(s); }

  public:

    /**
     * configuration of data node
     * cannot be changed at runtime
     */
    DataNodeConfig config;

    /**
     * logger of the data node
     */
    log::Logger *pLogger;

    /**
     * returns access to state
     * @return
     */
    State *state() const { return pNodeState; }

    /**
     * maps relative path to data dir
     * @param r
     * @return
     */
    Str mapToDir(const Str &r) const;

    /**
     * returns id of maschine
     * @return
     */
    Str getMaschineId() const;

    /**
     * returns data node
     * @return
     */
    int run();

    DataNode();

    virtual ~DataNode();
  };
};


#endif /* DATANODE_H */

