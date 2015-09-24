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

#ifndef KEYVALUESTOREIPC_H_
#define KEYVALUESTOREIPC_H_

#include "ipc.h"
#include "NetworkStreamIPC.h"

namespace KVS {

class KeyValueStoreIPC {
 public:
  KeyValueStoreIPC(int appId);
  ~KeyValueStoreIPC();

  bool connect();
  bool connect(string execPath);
  bool connected();

  bool addConnection(string host, int interfacePort, int kvsPort,
                     string config);
  bool removeConnection(int index);
  bool retryConnection(int index);

  bool updateServerList(string separatedList);
  bool syncServerList();
  string getInformationString();

  bool setDatabase(string databaseName);
  string useDatabase(string databaseName);

  unsigned int getTransferId();

  unsigned int getGlobalTupelId();

  bool initClients(string localIp, int localInterfacePort, int localKvsPort);

  bool startClient(int port);
  bool stopClient(int port);

  bool setId(int id);
  bool setMaster(string host, int interfacePort, int kvsPort);

  NetworkStreamIPC* getNetworkStream(unsigned int id);
  void removeNetworkStream(unsigned int id);

  int tryRestructureLock();
  bool updateRestructureLock();
  bool unlockRestructureLock();

  int getDistributionRef(string name);
  int getDistributionRef(string name, int typeId, string data);

  bool getDistributionData(int refId, string* data);

  bool execCommand(string command);

  bool qtDistAdd(int refId, int nrcoords, double* coords, set<int>* resultIds);
  int qtDistPointId(int refId, double interx, double intery);
  bool qtDistRequest(int refId, int nrcoords, double* coords,
                     set<int>* resultIds);
  bool qtDistinct(int refId, double x, double y);

  bool distAddRect(int refId, int nrcoords, double* coords, set<int>* resultIds,
                   bool requestOnly);
  bool distAddInt(int refId, int value, set<int>* resultIds, bool requestOnly);

  bool distFilter(const int& refId, const int& nrcoords, double* coords,
                  const unsigned int& globalId, const bool& update);

  string getAppName();
  string getSCPTransferPath();
  string getSCPSourcePath();

  int localId;

 private:
  void ipcServerStart(string execPath);

  bool cmd(IPCMessage msg);
  bool getResult(bool* res);
  bool getResult(int* res);
  bool getResult(unsigned int* res);
  bool getResult(string* data);
  bool getResult(set<int>* res);

  map<unsigned int, NetworkStreamIPC*> streams;

  int appId;
  IPCConnection* connection;
};
};

#endif /* KEYVALUESTOREIPC_H_ */
