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

#ifndef SERVERMANAGER_H_
#define SERVERMANAGER_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cstdio>

#include "Connection.h"

#include "SecondoInterface.h"
#include "SecondoInterfaceCS.h"

#include "boost/thread.hpp"
#include "boost/date_time.hpp"


namespace KVS {

class KeyValueStore;

class ServerManager {
 public:
  ServerManager(KeyValueStore* instance);
  ~ServerManager();
  bool initialize();

  bool addConnection(Connection* conn);
  bool removeConnection(Connection* conn);
  bool requestRemove(Connection* conn);

  bool connect(Connection* conn);

  vector<Connection*>& getConnectionList();

  vector<Connection*>& getRemoveList();

  unsigned int connectionSize();
  Connection* getConnectionIdx(int index);

  int getConnectionIndex(int id);

  int getAvailableServer();

  bool updateServerList(string separatedList);
  string getServerListString();

 private:
  void startMonitorThread();

  bool killConnectionThread(Connection* conn);
  void killAllThreads();

  // monitor while connecting
  bool monitorRunning;
  boost::thread* monitorThread;

  boost::mutex activeThreadsMutex;
  map<Connection*, boost::thread*> activeThreads;

  vector<Connection*> connectionList;

  vector<Connection*> removeList;

  string logPath;
  string configPath;

  basic_streambuf<char>* backupCout;

  KeyValueStore* instance;
};
}

#endif /* SERVERMANAGER_H_ */
