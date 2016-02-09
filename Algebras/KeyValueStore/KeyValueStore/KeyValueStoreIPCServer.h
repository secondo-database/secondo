/*
----
This file is part of SECONDO.

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

*/
#ifndef KEYVALUESTOREIPCSERVER_H_
#define KEYVALUESTOREIPCSERVER_H_

#include "ipc.h"
#include "KeyValueStore.h"

#include <map>
#include <functional>
#include <fstream>

namespace KVS {

//
// IPC-Server. This is what algebras connect to when calling kvsStartApp()
//=> Contains main-loop of KeyValueStore application.
//
class KeyValueStoreIPCServer {
 public:
  KeyValueStoreIPCServer(std::string appPath, int appId, bool useConsole);
  ~KeyValueStoreIPCServer();

  bool checkExclusive();
  int run();

  int dispatch(IPCConnection* conn);

  enum DispatchResults { NORESULT, REMOVECONNECTION };

  KVS::KeyValueStore* getKVSInstance() { return &kvs; }

 private:
  int appId;
  std::string appPath;

  std::map<IPCMessage, std::function<int(KVS::KeyValueStore*, IPCConnection*)> >
      dispatchMap;

  void* appMutexHandle;

  KVS::KeyValueStore kvs;

  IPCMessage lastMessage;

  std::ofstream* kvsLogfile;
  std::ofstream* restructureLogfile;
};
}

#endif /* KEYVALUESTOREIPCSERVER_H_ */
