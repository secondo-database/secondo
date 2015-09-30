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

#include "ServerManager.h"

#include "KeyValueStore.h"

#include <tuple>
#include <algorithm>

#include "StringUtils.h"

namespace KVS {

void connectionThread(Connection* conn) {
  string errorMsg("");
  conn->status = Connection::Connecting;

  if (conn->interfaceConn->Initialize("", "", conn->host,
                                      stringutils::int2str(conn->interfacePort),
                                      conn->config, errorMsg, true)) {
    conn->status = Connection::Connected;
  } else {
    conn->status = Connection::Failed;
  }
}

void monitorConnectionsThread(map<Connection*, boost::thread*>* activeThreads,
                              boost::mutex* activeThreadsMutex,
                              bool* monitorRunning) {
  Connection* conn;

  while (*monitorRunning) {
    activeThreadsMutex->lock();

    for (auto it = activeThreads->begin(); it != activeThreads->end(); ++it) {
      conn = it->first;
      if (conn->status == Connection::Connected ||
          conn->status == Connection::Failed) {
        it->second->join();
        activeThreads->erase(it);

        if (activeThreads->size() == 0) {
          *monitorRunning = false;
        }
        break;
      }
    }

    activeThreadsMutex->unlock();
    boost::this_thread::sleep(boost::posix_time::milliseconds(200));
  }
}

ServerManager::ServerManager(KeyValueStore* instance)
    : monitorRunning(false),
      monitorThread(0),
      backupCout(0),
      instance(instance) {}

ServerManager::~ServerManager() {
  killAllThreads();

  for (unsigned int i = 0; i < connectionList.size(); i++) {
    delete connectionList[i];
  }

  if (backupCout != 0) {
    cout.rdbuf(backupCout);
    cerr.rdbuf(backupCout);
  }
}

bool ServerManager::initialize() { return true; }

bool ServerManager::addConnection(Connection* conn) {
  connectionList.push_back(conn);
  return true;
}

bool ServerManager::connect(Connection* conn) {
  boost::lock_guard<boost::mutex> guard(activeThreadsMutex);

  if (activeThreads.find(conn) == activeThreads.end()) {
    boost::thread* connectThread = new boost::thread(connectionThread, conn);
    activeThreads.insert(make_pair(conn, connectThread));

    if (!monitorRunning) {
      startMonitorThread();
    }

    return true;
  } else {
    return false;
  }
}

void ServerManager::startMonitorThread() {
  monitorRunning = true;
  if (monitorThread == 0) {
    monitorThread = new boost::thread(monitorConnectionsThread, &activeThreads,
                                      &activeThreadsMutex, &monitorRunning);
  }
}

bool ServerManager::removeConnection(Connection* conn) {
  auto item = find(connectionList.begin(), connectionList.end(), conn);

  if (item != connectionList.end()) {
    killConnectionThread(conn);
    delete conn;
    connectionList.erase(item);
    return true;
  } else {
    return false;
  }
}

bool ServerManager::requestRemove(Connection* conn) {
  auto item = find(connectionList.begin(), connectionList.end(), conn);
  if (item != connectionList.end() &&
      find(removeList.begin(), removeList.end(), conn) == removeList.end()) {
    removeList.push_back(conn);
    return true;
  } else {
    return false;
  }
}

bool ServerManager::updateServerList(string separatedList) {
  std::istringstream inputStream(separatedList);
  std::string idStr, host, interfacePortStr, kvsPortStr, config,
      tupleCapacityStr;
  int id, interfacePort, kvsPort;
  int tupleCapacity = 0;

  bool check;

  while (std::getline(inputStream, idStr, ';') &&
         std::getline(inputStream, host, ';') &&
         std::getline(inputStream, interfacePortStr, ';') &&
         std::getline(inputStream, kvsPortStr, ';') &&
         std::getline(inputStream, config, ';') &&
         std::getline(inputStream, tupleCapacityStr, ';')) {
    id = stringutils::str2int<int>(idStr, check);
    interfacePort = stringutils::str2int<int>(interfacePortStr, check);
    kvsPort = stringutils::str2int<int>(kvsPortStr, check);
    tupleCapacity = stringutils::str2int<unsigned int>(tupleCapacityStr, check);

    // if(id == -1 || id != instance->id) {
    // check if host exists
    int hostIdx = -1;
    for (unsigned int serverIdx = 0; serverIdx < connectionList.size();
         ++serverIdx) {
      if (connectionList[serverIdx]->host.compare(host) == 0 &&
          connectionList[serverIdx]->interfacePort == interfacePort) {
        hostIdx = serverIdx;
        break;
      }
    }

    if (hostIdx >= 0) {
      connectionList[hostIdx]->id = id;
      connectionList[hostIdx]->kvsPort = kvsPort;
      connectionList[hostIdx]->config = config;
      connectionList[hostIdx]->tupleCapacity = tupleCapacity;

      if (connectionList[hostIdx]->kvsPort != kvsPort) {
        connectionList[hostIdx]->kvsConn->close();
        connectionList[hostIdx]->kvsConn->check();
      }

      if (connectionList[hostIdx]->config.compare(config) != 0) {
        // TODO: restart
        // connectionList[hostIdx]->c
      }
    } else {
      // add
      Connection* conn = new Connection(host, interfacePort, kvsPort, config);
      conn->id = id;
      conn->tupleCapacity = tupleCapacity;

      addConnection(conn);
    }
    //}
  }

  return true;
}

string ServerManager::getServerListString() {
  stringstream resultStr;
  for (unsigned int serverIdx = 0; serverIdx < connectionList.size();
       ++serverIdx) {
    resultStr << connectionList[serverIdx]->id << ";"
              << connectionList[serverIdx]->host << ";"
              << connectionList[serverIdx]->interfacePort << ";"
              << connectionList[serverIdx]->kvsPort << ";"
              << connectionList[serverIdx]->config << ";"
              << connectionList[serverIdx]->tupleCapacity << ";";
  }
  return resultStr.str();
}

bool ServerManager::killConnectionThread(Connection* conn) {
  boost::lock_guard<boost::mutex> guard(activeThreadsMutex);

  auto item = activeThreads.find(conn);
  if (item != activeThreads.end()) {
    //! terminate
    // TerminateThread(item->second->native_handle());

    activeThreads.erase(item);
    return true;
  } else {
    return false;
  }
}

void ServerManager::killAllThreads() {
  // monitor
  if (monitorRunning && monitorThread != 0) {
    monitorRunning = false;
    monitorThread->join();
    delete monitorThread;
    monitorThread = 0;
  }

  // connection threads
  boost::lock_guard<boost::mutex> guard(activeThreadsMutex);

  for (auto it = activeThreads.begin(); it != activeThreads.end(); ++it) {
    // pthread_cancel(*(it->second));
  }
  activeThreads.clear();
}

unsigned int ServerManager::connectionSize() { return connectionList.size(); }

Connection* ServerManager::getConnectionIdx(int index) {
  return connectionList[index];
}

int ServerManager::getConnectionIndex(int id) {
  for (unsigned int serverIdx = 0; serverIdx < connectionList.size();
       ++serverIdx) {
    if (connectionList[serverIdx]->id == id) {
      return static_cast<int>(serverIdx);
    }
  }

  return -1;
}

int ServerManager::getAvailableServer() {
  for (unsigned int serverIdx = 0; serverIdx < connectionList.size();
       ++serverIdx) {
    if (connectionList[serverIdx]->id == -1 &&
        connectionList[serverIdx]->status != Connection::Problem) {
      return serverIdx;
    }
  }
  return -1;
}

vector<Connection*>& ServerManager::getConnectionList() {
  return connectionList;
}

vector<Connection*>& ServerManager::getRemoveList() { return removeList; }
}
