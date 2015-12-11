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

//
// Interface to application.
// Almost all commands are dispatched to Key-Value-Store application.
//

class KeyValueStoreIPC {
 public:
  KeyValueStoreIPC(int appId);
  ~KeyValueStoreIPC();

  bool connect();
  bool connect(std::string execPath);
  bool connected();

  bool addConnection(std::string host, int interfacePort, int kvsPort,
                     std::string config);
  bool removeConnection(int index);
  bool retryConnection(int index);

  bool updateServerList(std::string separatedList);
  bool syncServerList();
  std::string getInformationString();

  bool setDatabase(std::string databaseName);
  std::string useDatabase(std::string databaseName);

  unsigned int getTransferId();

  unsigned int getGlobalTupelId();

  bool initClients(std::string localIp, int localInterfacePort, 
                   int localKvsPort);

  bool startClient(int port);
  bool stopClient(int port);

  bool setId(int id);
  bool setMaster(std::string host, int interfacePort, int kvsPort);

  NetworkStreamIPC* getNetworkStream(unsigned int id);
  void removeNetworkStream(unsigned int id);

  int tryRestructureLock();
  bool updateRestructureLock();
  bool unlockRestructureLock();

  int getDistributionRef(std::string name);
  int getDistributionRef(std::string name, int typeId, std::string data);

  bool getDistributionData(int refId, std::string* data);

  bool execCommand(std::string command);

  bool qtDistAdd(int refId, int nrcoords, double* coords, 
                 std::set<int>* resultIds);
  int qtDistPointId(int refId, double interx, double intery);
  bool qtDistRequest(int refId, int nrcoords, double* coords,
                     std::set<int>* resultIds);
  bool qtDistinct(int refId, double x, double y);

  bool distAddRect(int refId, int nrcoords, double* coords, 
                   std::set<int>* resultIds,
                   bool requestOnly);
  bool distAddInt(int refId, int value, std::set<int>* resultIds, 
                   bool requestOnly);

  bool distFilter(int& refId, int& nrcoords, double* coords,
                  unsigned int& globalId, bool& update);

  std::string getAppName();
  std::string getSCPTransferPath();
  std::string getSCPSourcePath();

  int localId;

 private:
  void ipcServerStart(std::string execPath);

  bool cmd(IPCMessage msg);
  bool getResult(bool* res);
  bool getResult(int* res);
  bool getResult(unsigned int* res);
  bool getResult(std::string* data);
  bool getResult(std::set<int>* res);

  std::map<unsigned int, NetworkStreamIPC*> streams;

  int appId;
  IPCConnection* connection;
};
};

#endif /* KEYVALUESTOREIPC_H_ */
