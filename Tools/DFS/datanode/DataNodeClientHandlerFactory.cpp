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

#include "DataNodeClientHandlerFactory.h"

using namespace dfs;

DataNodeClientHandlerFactory::DataNodeClientHandlerFactory(
  const DataNode *dataNode, int maxHandlerCount, dfs::log::Logger *l) {
  this->maxHandlerCount = maxHandlerCount;
  this->logger = l;
  // this->nodeState = nodeState; // self assignment: makes no sense
  this->dataNode = dataNode;
  this->freeHandlerId = 0;
  this->maxHandlerCount = 8;
  handlers = new DataNodeClientHandler[maxHandlerCount];
}

DataNodeClientHandlerFactory::~DataNodeClientHandlerFactory() {
  delete[] handlers;
  handlers = 0;
}

dfs::comm::ClientHandler *DataNodeClientHandlerFactory::createHandler() {


  for (int i = 0; i < maxHandlerCount; i++) {
    DataNodeClientHandler *h = &handlers[i];

    if (h->id == -1) {
      h->dataNode = dataNode;
      h->id = freeHandlerId++;
      h->logger = logger;
      h->init();
    }

    if (h->state == 0) {
      debug(Str(
        "DataNodeHandlerFactory.createHandler:"
          "found and return handler with id ").append(
        h->id));
      h->state = 1;
      return h;
    }
  }
  //fixme verbessern, aeussere schleife
  throw "no free client handlers found";
}

