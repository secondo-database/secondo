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


September 2015 - Nico Kaufmann: Connection class started as copy from
Distributed2Algebra

*/

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cstdio>
#include <string>

#include "SocketIO.h"
#include "SecondoInterface.h"
#include "SecondoInterfaceCS.h"

#include "KeyValueStoreDebug.h"

#include "RestructureLock.h"

#include "boost/thread.hpp"
#include "boost/date_time.hpp"

namespace KVS {

class KVSConnection {
 public:
  KVSConnection(string host, string port);
  ~KVSConnection();

  bool connect();
  bool check();
  void close();

  // add queue parameter
  // void sendStream(const string& id, const string& streamType,
  // SyncPseudoQueue<Tuple*>* queue, bool* result);

  bool sendStream(const int& id, const string& streamType,
                  const vector<pair<char*, unsigned int> >& data);
  bool sendStream(const int& id, const char* data, const unsigned int& dataLen);
  bool sendStreamType(const int& id, const string& streamType);

  bool endStream(const int& id);
  unsigned int transferCount(const int& id);
  bool removeStream(const int& id);

  bool sendDistributionUpdate(const string& distributionName,
                              const string& data);
  bool requestDistribution(const string& distributionName, string& data);

  unsigned int requestTransferId();
  string requestServerList();
  string requestSCPPath();

  int tryRestructureLock(int id);
  bool updateRestructureLock();
  bool unlockRestructureLock(int id);

  Socket* connection;

  string host;
  string port;

  boost::mutex mtx;
};

class Connection {
 public:
  Connection(const string& host, const int interfacePort, const int kvsPort,
             const string& config);
  ~Connection();

  bool connectInterface();
  bool check();
  bool initInterface(string localIp, int localInterfacePort, int localKvsPort,
                     string databaseName);
  bool setId(int id);

  bool exec(string command);
  bool exec(string command, const bool& expected);
  bool exec(string command, const int& expected);

  bool relationExists(string relationName);
  bool updateDistributionObject(string distributionName);

  void simpleCommand(string command, int& err, string& result);
  void simpleCommand(string command, int& error, string& errMsg,
                     string& resList);
  void simpleCommand(string command, int& error, string& errMsg,
                     ListExpr& resList);

  int id;

  string host;
  int interfacePort;
  int kvsPort;
  string config;
  unsigned int tupleCapacity;

  int status;
  SecondoInterfaceCS* interfaceConn;
  KVSConnection* kvsConn;

  enum ConnectionStatus { Unknown, Connecting, Connected, Failed, Problem };

  RestructureLock rLock;
  bool restructureInProgress;

 private:
  NestedList* mynl;
  boost::mutex interfaceMutex;
};

/*namespace ConnectionEnum {
  static const char* ConnectionStatusNames[5] = { "Unknown", "Connecting",
"Connected", "Failed", "Problem" };
}*/
}

#endif /* CONNECTION_H_ */
