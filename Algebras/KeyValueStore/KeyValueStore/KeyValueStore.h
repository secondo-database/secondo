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

#ifndef KEYVALUESTORE_H_
#define KEYVALUESTORE_H_

#include "KeyValueStoreDebug.h"
#include "SecondoIncludes.h"

#include "ServerManager.h"
#include "DistributionTaskManager.h"

#include "NetworkStreamBuffer.h"

#include "Connection.h"
#include "QuadTreeDistributionType.h"

#include "boost/thread.hpp"
#include "boost/date_time.hpp"

#include <set>

namespace KVS {

//
// Counterpart to KeyValueStoreIPC but also does other things.
// Listen and responds to client connections by KvsConnection.
//
class KeyValueStore {
 public:
  KeyValueStore(string appPath);
  ~KeyValueStore();

  string getAppBasePath();
  string getRestructurePath();
  string getRestructureFilePath(string suffix);

  bool initClients(string localIp, int localInterfacePort, int localKvsPort);
  bool startClient(int port);
  bool stopClient(int port);
  void listenThread(int port, Socket* gate);
  void connectionThread(Socket* client);

  bool addConnection(string host, int interfacePort, int kvsPort,
                     string config);
  bool removeConnection(unsigned int index);
  bool syncServerList();
  bool updateServerList(string separatedList);
  bool retryConnection(unsigned int index);
  string serverInformationString();
  void setDatabase(string databaseName);
  string useDatabase(string databaseName);

  int tryRestructureLock();
  bool updateRestructureLock();
  bool unlockRestructureLock();

  int getDistributionRef(string name);
  int getDistributionRef(string name, int typeId, string data);
  Distribution* getDistribution(int id);
  string getDistributionName(int id);

  bool execCommand(string command);

  bool qtDistAdd(int refId, int nrcoords, double* coords, set<int>* resultIds);
  bool qtDistRequest(int refId, int nrcoords, double* coords,
                     set<int>* resultIds);

  bool distAddRect(int refId, int nrcoords, double* coords, set<int>* resultIds,
                   bool requestOnly);
  bool distAddRectDebug(int refId, int nrcoords, double* coords,
                        set<int>* resultIds, bool requestOnly);
  bool distAddInt(int refId, int value, set<int>* resultIds, bool requestOnly);

  bool distFilter(const int& refId, const int& nrcoords, double* coords,
                  const unsigned int& globalId, const bool& update);

  bool qtDistinct(const int& refId, const double& x, const double& y);

  void lockRemove();
  void unlockRemove();

  bool setMaster(string host, int interfacePort, int kvsPort);

  string getSCPTransferPath();
  string getSCPSourcePath();
  unsigned int getTransferId();

  unsigned int getGlobalTupelId();

  // should be executed AFTER task was added
  void startDistributionThread();

  ServerManager sm;
  DistributionTaskManager dtm;
  NetworkStreamBuffer nsb;

  void listenStatus();

  string localHost;
  int localInterfacePort;
  int localKvsPort;

  string currentDatabaseName;

  int id;
  Connection* masterConn;
  unsigned int batchSize;

 private:
  int addDistribution(Distribution* dist, const string& name);

  string appPath;

  unsigned int transferId;
  boost::mutex transferIdMutex;

  bool distributionThreadRunning;
  boost::thread* distributionThread;

  // port -> {thread, socket}
  boost::mutex listenThreadsMutex;
  map<int, pair<boost::thread*, Socket*> > listenThreads;

  int distributionId;
  map<int, Distribution*> distributionsMap;
  map<int, string> distributionsIdNameMap;
  map<string, int> distributionsNameIdMap;

  unsigned int globalTupelId;
  boost::mutex globalTupelIdMutex;
};
}

#endif /* KEYVALUESTORE_H_ */
