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

#include "DataNode.h"
#include "../shared/io.h"
#include "../shared/maschine.h"
#include "DataNodeClientHandlerFactory.h"

using namespace dfs;

DataNode::DataNode() {
  pNodeState = new State();
}

DataNode::~DataNode() {
  delete pNodeState;
  pNodeState = 0;
}

int DataNode::run() {

  debug("DataNode.run");

  maschineId = dfs::Maschine::volatileId(Str("d").append(config.port));
  debug(Str("maschinenID ist ").append(maschineId));

  DataNodeClientHandlerFactory fac(this, config.maxClients, pLogger);
  debug("DataNode.run created handlerFactory");

  dfs::comm::Endpoint ep(Str("dataNode").append(config.port));
  ep.bufsize = 1024 * 1024;
  ep.port = config.port;
  ep.handlerMode = 1;
  ep.handlerFactory = &fac;
  ep.setLogger(pLogger);
  debug("DataNode.run starting listener");

  cout << "Datenknoten auf Port " << config.port << " betriebsbereit." << endl
       << "ID ist " << maschineId << endl;
  ep.listen();
  cout << "Datenknoten - Serverbetrieb beendet" << endl;
  return 0;
}

Str DataNode::mapToDir(const Str &r) const {
  return dfs::io::file::combinePath(config.dataDir, r);
}

Str DataNode::getMaschineId() const {
  return maschineId;
}
