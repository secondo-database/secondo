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

#include "KeyValueStore.h"

#include "StringUtils.h"
#include "FileSystem.h"
#include "Application.h"

#include "QuadTreeDistributionType.h"

#include "KeyValueStoreDebug.h"

namespace KVS {

KeyValueStore::KeyValueStore(string appPath)
    : sm(this),
      dtm(this),
      id(-1),
      masterConn(0),
      batchSize(100),
      appPath(appPath),
      transferId(0),
      distributionThreadRunning(false),
      distributionThread(0),
      distributionId(0),
      globalTupelId(0) {
  localKvsPort = 0;
  localInterfacePort = 0;
}

KeyValueStore::~KeyValueStore() {
  // stop listen threads
  map<int, pair<boost::thread*, Socket*> >::iterator it;
  for (it = listenThreads.begin(); it != listenThreads.end(); ++it) {
    Socket* gate = it->second.second;

    if (gate != 0 && gate->IsOk()) {
      gate->CancelAccept();
      gate->Close();  // 2nd time? problem?
    }

    boost::thread* listenThread = it->second.first;

    listenThread->join();
    delete listenThread;
    delete gate;
  }

  listenThreads.clear();

  if (distributionThread) {
    distributionThreadRunning = false;
    distributionThread->join();
    delete distributionThread;
  }
}

string KeyValueStore::getAppBasePath() {
  return appPath.substr(0, appPath.find_last_of(PATH_SLASH));
}

string KeyValueStore::getRestructurePath() {
  string resultPath(appPath.substr(0, appPath.find_last_of("/\\") + 1) +
                    "kvsRestructure");

  if (!FileSystem::FileOrFolderExists(resultPath)) {
    FileSystem::CreateFolder(resultPath);
  }
  return resultPath;
}

string KeyValueStore::getRestructureFilePath(string suffix) {
  time_t now = time(NULL);
  struct tm* timeparts = localtime(&now);

  char buffer[32];
  strftime(buffer, 32, "%Y%m%d_%H%M%S ", timeparts);

  string resultBasePath(getRestructurePath() + PATH_SLASH + string(buffer));

  int incr = 0;
  while (FileSystem::FileOrFolderExists(resultBasePath +
                                        stringutils::int2str(incr) + suffix)) {
    incr++;
  }

  return resultBasePath + stringutils::int2str(incr) + suffix;
}

bool KeyValueStore::setMaster(string host, int interfacePort, int kvsPort) {
  if (masterConn != 0) {
    if (masterConn->kvsConn->check()) {
      if (host.compare(masterConn->host) == 0 &&
          interfacePort == masterConn->interfacePort &&
          kvsPort == masterConn->kvsPort) {
        // already connected
        return true;
      }
    }

    delete masterConn;
  }

  KOUT << "Setting master..." << endl;

  localHost = host;
  localInterfacePort = interfacePort;
  localKvsPort = kvsPort;

  masterConn =
      new Connection(host, interfacePort, kvsPort, "SecondoConfig.ini");

  return masterConn->kvsConn->check();  // this should establish a connection
}

string KeyValueStore::getSCPTransferPath() {
  string path(getAppBasePath());

  FileSystem::AppendItem(path, "kvs");

  if (!FileSystem::FileOrFolderExists(path)) {
    FileSystem::CreateFolder(path);
  }

  return path;
}

string KeyValueStore::getSCPSourcePath() {
  string path(getSCPTransferPath());

  FileSystem::AppendItem(path, "source");

  if (!FileSystem::FileOrFolderExists(path)) {
    FileSystem::CreateFolder(path);
  }

  return path;
}

unsigned int KeyValueStore::getTransferId() {
  boost::lock_guard<boost::mutex> guard(transferIdMutex);

  transferId++;

  return transferId;
}

unsigned int KeyValueStore::getGlobalTupelId() {
  boost::lock_guard<boost::mutex> guard(globalTupelIdMutex);
  globalTupelId++;
  return globalTupelId;
}

void KeyValueStore::startDistributionThread() {
  dtm.exitMutex.lock();

  if (!distributionThreadRunning) {
    if (distributionThread != 0) {
      distributionThread->join();
      delete distributionThread;
    }

    distributionThreadRunning = true;
    distributionThread = new boost::thread(&DistributionTaskManager::exec, &dtm,
                                           &distributionThreadRunning);
  }

  dtm.exitMutex.unlock();
}

bool KeyValueStore::addConnection(string host, int interfacePort, int kvsPort,
                                  string config) {
  return sm.addConnection(new Connection(host, interfacePort, kvsPort, config));
}

bool KeyValueStore::removeConnection(unsigned int index) {
  vector<Connection*> connList(sm.getConnectionList());

  if (index >= 0 && index < connList.size()) {
    return sm.removeConnection(connList[index]);
  } else {
    return false;
  }
}

bool KeyValueStore::retryConnection(unsigned int index) {
  vector<Connection*> connList(sm.getConnectionList());

  if (index >= 0 && index < connList.size()) {
    return sm.connect(connList[index]);
  } else {
    return false;
  }
}

bool KeyValueStore::syncServerList() {
  if (masterConn != 0 && masterConn->kvsConn->check()) {
    string serverList = masterConn->kvsConn->requestServerList();
    cout << "Requested Serverlist:" << serverList << endl;
    return sm.updateServerList(serverList);
  } else {
    KOUT << "Failed to connect to Master-Server";
    return false;
  }
}

bool KeyValueStore::updateServerList(string separatedList) {
  return sm.updateServerList(separatedList);
}

string KeyValueStore::serverInformationString() {
  const char* ConnectionStatusNames[5] = {"Unknown", "Connecting", "Connected",
                                          "Failed", "Problem"};
  vector<Connection*> connList(sm.getConnectionList());

  stringstream output;

  // header
  output << "\nServer List:\n";
  output << " ID  " << left << setw(21) << "IP"
         << "  " << left << setw(20) << "ALIAS"
         << "STATUS\n";

  string alias("");
  string status("");

  for (unsigned int i = 0; i < connList.size(); i++) {
    string ipStr = connList[i]->host + string(":") +
                   stringutils::int2str(connList[i]->interfacePort);
    output << " " << left << setw(3) << i << " " << left << setw(21) << ipStr
           << "  " << left << setw(20) << alias
           << ConnectionStatusNames[connList[i]->status]
           << "\n";  //<<" e.code:"<<err.code<<" e.msg:"<<err.msg<<
  }

  output << "\n";

  return output.str();
}

void KeyValueStore::setDatabase(string databaseName) {
  currentDatabaseName = databaseName;
  KOUT << "Setting current db:" << currentDatabaseName << endl;
}

string KeyValueStore::useDatabase(string databaseName) {
  vector<Connection*> connList(sm.getConnectionList());

  KOUT << "Changing client databases..." << endl;

  // data
  int errResult;
  string strResult;

  stringstream output;

  currentDatabaseName = databaseName;

  cout << "Setting current db:" << currentDatabaseName << endl;

  string openCmd("open database ");
  openCmd += databaseName;
  openCmd += ";";

  for (unsigned int i = 0; i < connList.size(); i++) {
    output << i << " :trying close command...\n" << endl;
    connList[i]->simpleCommand("close database;", errResult, strResult);

    output << i << " :trying: " << openCmd << endl;

    connList[i]->simpleCommand(openCmd, errResult, strResult);
    output << i << " : result = " << strResult << " errCode = " << errResult
           << endl;
  }

  return output.str();
}

int KeyValueStore::tryRestructureLock() {
  if (masterConn) {
    return masterConn->kvsConn->tryRestructureLock(id);
  } else {
    KOUT << "Error: Master-Server unknown.<<endl";
    return -1;
  }
}

bool KeyValueStore::updateRestructureLock() {
  if (masterConn) {
    return masterConn->kvsConn->updateRestructureLock();
  } else {
    KOUT << "Error: Master-Server unknown.<<endl";
    return -1;
  }
}

bool KeyValueStore::unlockRestructureLock() {
  if (masterConn) {
    return masterConn->kvsConn->unlockRestructureLock(id);
  } else {
    KOUT << "Error: Master-Server unknown.<<endl";
    return -1;
  }
}

int KeyValueStore::getDistributionRef(string name) {
  map<string, int>::iterator item = distributionsNameIdMap.find(name);

  if (item != distributionsNameIdMap.end()) {
    return item->second;
  } else {
    return -1;
  }
}

int KeyValueStore::getDistributionRef(string name, int typeId, string data) {
  map<string, int>::iterator item = distributionsNameIdMap.find(name);
  int result = -1;

  if (item != distributionsNameIdMap.end()) {
    return item->second;
  } else {
    Distribution* tempDist = Distribution::getInstance(data);
    if (tempDist != 0) {
      result = addDistribution(tempDist, name);
    }
  }

  return result;
}

int KeyValueStore::addDistribution(Distribution* dist, const string& name) {
  int result = -1;
  distributionsMap.insert(make_pair(distributionId, dist));
  distributionsNameIdMap.insert(make_pair(name, distributionId));
  distributionsIdNameMap.insert(make_pair(distributionId, name));
  result = distributionId;
  distributionId++;

  return result;
}

bool KeyValueStore::qtDistAdd(int refId, int nrcoords, double* coords,
                              set<int>* resultIds) {
  map<int, Distribution*>::iterator item = distributionsMap.find(refId);

  if (item != distributionsMap.end()) {
    QuadTreeDistribution* qtd = (QuadTreeDistribution*)item->second;
    boost::lock_guard<boost::mutex> guard(qtd->syncMutex);

    if (qtd->root == 0) {
      qtd->root = new QuadNode(coords[0], coords[1], qtd->initialWidth,
                               qtd->initialHeight);
    }

    qtd->expand(coords);
    qtd->insert(qtd->root, coords, resultIds);
    return true;
  } else {
    return false;
  }
}

bool KeyValueStore::qtDistRequest(int refId, int nrcoords, double* coords,
                                  set<int>* resultIds) {
  map<int, Distribution*>::iterator item = distributionsMap.find(refId);

  if (item != distributionsMap.end()) {
    QuadTreeDistribution* qtd = (QuadTreeDistribution*)item->second;
    boost::lock_guard<boost::mutex> guard(qtd->syncMutex);
    if (qtd->root != 0) {
      qtd->retrieveIds(qtd->root, coords, resultIds);
    }

    if (resultIds->size() == 0) {
      resultIds->insert(-1);
    }

    return true;
  } else {
    return false;
  }
}

bool KeyValueStore::distAddRect(int refId, int nrcoords, double* coords,
                                set<int>* resultIds, bool requestOnly) {
  map<int, Distribution*>::iterator item = distributionsMap.find(refId);

  if (item != distributionsMap.end()) {
    Distribution* dist = item->second;
    boost::lock_guard<boost::mutex> guard(dist->syncMutex);

    if (requestOnly) {
      dist->request(nrcoords, coords, resultIds);
    } else {
      dist->add(nrcoords, coords, resultIds);
    }

    return true;
  } else {
    return false;
  }
}

bool KeyValueStore::distAddRectDebug(int refId, int nrcoords, double* coords,
                                     set<int>* resultIds, bool requestOnly) {
  map<int, Distribution*>::iterator item = distributionsMap.find(refId);

  cout << "calling distAddRectDebug" << endl;

  if (item != distributionsMap.end()) {
    Distribution* dist = item->second;
    boost::lock_guard<boost::mutex> guard(dist->syncMutex);

    string debugPath = getRestructureFilePath("error");
    SaveDebugFile(debugPath, dist);

    if (requestOnly) {
      cout << "calling request" << endl;
      dist->requestDebug(nrcoords, coords, resultIds);
    } else {
      cout << "calling add" << endl;
      dist->addDebug(nrcoords, coords, resultIds);
    }

    return true;
  } else {
    return false;
  }
}

bool KeyValueStore::distAddInt(int refId, int value, set<int>* resultIds,
                               bool requestOnly) {
  map<int, Distribution*>::iterator item = distributionsMap.find(refId);

  if (item != distributionsMap.end()) {
    Distribution* dist = (Distribution*)item->second;
    boost::lock_guard<boost::mutex> guard(dist->syncMutex);

    if (requestOnly) {
      // dist->request(value, resultIds);
    } else {
      // dist->add(value, resultIds);
    }

    if (resultIds->size() == 0) {
      resultIds->insert(-1);
    }

    return true;
  } else {
    return false;
  }
}

bool KeyValueStore::distFilter(const int& refId, const int& nrcoords,
                               double* coords, const unsigned int& globalId,
                               const bool& update) {
  map<int, Distribution*>::iterator item = distributionsMap.find(refId);

  if (item != distributionsMap.end()) {
    Distribution* dist = (Distribution*)item->second;
    boost::lock_guard<boost::mutex> guard(dist->syncMutex);

    return dist->filter(nrcoords, coords, globalId, update);
  } else {
    return false;
  }
}

bool KeyValueStore::qtDistinct(const int& refId, const double& x,
                               const double& y) {
  map<int, Distribution*>::iterator item = distributionsMap.find(refId);

  if (item != distributionsMap.end()) {
    Distribution* dist = (Distribution*)item->second;
    boost::lock_guard<boost::mutex> guard(dist->syncMutex);

    if (dist->type == Distribution::TYPE_QUADTREE) {
      QuadTreeDistribution* qtd = (QuadTreeDistribution*)dist;

      return (id == qtd->pointId(x, y));
    } else {
      KOUT << "Error: qtDistinct wrong type" << endl;
    }
  }
  return false;
}

bool KeyValueStore::initClients(string localIp, int localInterfacePort,
                                int localKvsPort) {
  vector<Connection*>& connectionList = sm.getConnectionList();
  Connection* currentConn = 0;

  KOUT << "Initializing:" << endl;

  bool result = true;

  for (unsigned int connIdx = 0; connIdx < connectionList.size(); ++connIdx) {
    currentConn = connectionList[connIdx];

    KOUT << connIdx << " : " << currentConn->host << ":"
         << currentConn->interfacePort << ":" << endl;

    this->localHost = localIp;
    this->localInterfacePort = localInterfacePort;
    this->localKvsPort = localKvsPort;

    if (currentConn->initInterface(localIp, localInterfacePort, localKvsPort,
                                   currentDatabaseName)) {
      KOUT << "=> successfully initialized!";
    } else {
      result = false;
      KOUT << "=> failed to initialize!";
    }
    KOUT << endl << endl;
  }
  return result;
}

Distribution* KeyValueStore::getDistribution(int id) {
  map<int, Distribution*>::iterator item = distributionsMap.find(id);

  if (item != distributionsMap.end()) {
    return item->second;
  } else {
    return 0;
  }
}

string KeyValueStore::getDistributionName(int id) {
  map<int, string>::iterator item = distributionsIdNameMap.find(id);

  if (item != distributionsIdNameMap.end()) {
    return item->second;
  } else {
    return string("");
  }
}

bool KeyValueStore::execCommand(string command) {
  vector<Connection*>& serverList = sm.getConnectionList();

  KOUT << "Executing: " << command << endl << endl;

  for (unsigned int serverIdx = 0; serverIdx < serverList.size(); ++serverIdx) {
    int error = 0;
    string result;
    KOUT << "Server " << serverIdx << ":" << endl;
    serverList[serverIdx]->simpleCommand(command, error, result);
    if (error != 0) {
      KOUT << "Command failed:" << result << endl;
    } else {
      KOUT << result << endl;
    }
  }

  return true;
}

bool KeyValueStore::startClient(int port) {
  if (port > 1024 && port < 65535) {
    boost::lock_guard<boost::mutex> guard(listenThreadsMutex);

    if (listenThreads.find(port) == listenThreads.end()) {
      Socket* gate =
          Socket::CreateGlobal("localhost", stringutils::int2str(port));

      if (gate && gate->IsOk()) {
        KOUT << left << setw(5) << port << ": Created gate." << endl;
        listenThreads.insert(make_pair(
            port, make_pair(new boost::thread(&KeyValueStore::listenThread,
                                              this, port, gate),
                            gate)));
        return true;
      }
    } else {
      KOUT << left << setw(5) << port << ": Client already started." << endl;
      return true;
    }

    KOUT << left << setw(5) << port << ": startClient failed." << endl;
  }
  return false;
}

bool KeyValueStore::stopClient(int port) {
  map<int, pair<boost::thread*, Socket*> >::iterator item =
      listenThreads.find(port);

  if (item == listenThreads.end()) {
    return false;
  } else {
    item->second.second->CancelAccept();
    item->second.first->join();

    delete item->second.second;
    delete item->second.first;

    listenThreads.erase(item);
    return true;
  }
}

void KeyValueStore::listenThread(int port, Socket* gate) {
  while (true) {
    KOUT << left << setw(5) << port
         << ": Waiting for client connection... (Accept)\n";
    Socket* client = gate->Accept();

    if (client && client->IsOk()) {
      KOUT << left << setw(5) << port << ": Client connected...\n";
      boost::thread(&KeyValueStore::connectionThread, this, client);
    } else {
      KOUT << left << setw(5) << port << ": Problem accepting - closing gate\n";
      gate->Close();
      return;
    }
  }
}

void KeyValueStore::connectionThread(Socket* client) {
  iostream& iosock = client->GetSocketStream();

  iosock.exceptions(ios_base::failbit | ios_base::badbit | ios_base::eofbit);

  try {
    bool closeConn = false;

    string cmd;
    // greeting
    iosock << "<SecondoKVS/>" << endl;

    getline(iosock, cmd);
    if (cmd.compare("<SecondoKVS/>") == 0) {
      while (!iosock.fail() && !closeConn) {
        // commands
        getline(iosock, cmd);

        if (cmd.compare("<RemoteStream>") == 0) {
          int id;
          iosock.read((char*)&id, sizeof(id));

          NetworkStream* stream = new NetworkStream(id);
          if (nsb.addNetworkStream(id, stream)) {
            int typeLen;
            iosock.read((char*)&typeLen, sizeof(typeLen));
            char* tempTypeBuffer = new char[typeLen];
            iosock.read(tempTypeBuffer, typeLen);

            string streamType = tempTypeBuffer;
            delete[] tempTypeBuffer;

            stream->setStreamType(streamType);

            unsigned int tupleSize = 0;
            char* tupleBuffer = 0;

            int receiveTupleCount = 0;

            do {
              iosock.read((char*)&tupleSize, sizeof(tupleSize));

              if (tupleSize > 0) {
                tupleBuffer = new char[tupleSize];

                iosock.read(tupleBuffer, tupleSize);
                // process tuple

                stream->tupleQueue.add(
                    new pair<unsigned int, char*>(tupleSize, tupleBuffer));
                receiveTupleCount++;
              } else {
                stream->tupleQueue.setComplete();
              }

            } while (tupleSize > 0);

            // confirm received tuple count
            iosock.write((char*)&receiveTupleCount, sizeof(receiveTupleCount));
            iosock.flush();
          } else {
            KOUT << "Error: Stream id already exists\n";
          }
        } else if (cmd.compare("<RemoteStreamPart>") == 0) {
          int id;
          iosock.read((char*)&id, sizeof(id));

          NetworkStream* stream = nsb.createStream(id);
          unsigned int tupleSize = 0;
          char* tupleBuffer = 0;

          do {
            iosock.read((char*)&tupleSize, sizeof(tupleSize));

            if (tupleSize > 0) {
              tupleBuffer = new char[tupleSize];

              iosock.read(tupleBuffer, tupleSize);
              // process tuple

              stream->tupleQueue.add(
                  new pair<unsigned int, char*>(tupleSize, tupleBuffer));
            }
          } while (tupleSize > 0);

        } else if (cmd.compare("<RemoteStreamType>") == 0) {
          int id;
          iosock.read((char*)&id, sizeof(id));

          NetworkStream* stream = nsb.createStream(id);
          unsigned int typeLen;
          iosock.read((char*)&typeLen, sizeof(typeLen));
          char* tempTypeBuffer = new char[typeLen];
          iosock.read(tempTypeBuffer, typeLen);

          string streamType(tempTypeBuffer, typeLen);
          delete[] tempTypeBuffer;

          stream->setStreamType(streamType);
        } else if (cmd.compare("<EndRemoteStream>") == 0) {
          int id;
          iosock.read((char*)&id, sizeof(id));

          NetworkStream* stream = nsb.createStream(id);
          stream->tupleQueue.setComplete();
        } else if (cmd.compare("<RemoveRemoteStream>") == 0) {
          int id;
          iosock.read((char*)&id, sizeof(id));

          bool result = nsb.removeStream(id);
          iosock.write((char*)&result, sizeof(result));
          iosock.flush();
        } else if (cmd.compare("<RemoteStreamCount>") == 0) {
          int id;
          iosock.read((char*)&id, sizeof(id));

          NetworkStream* stream = nsb.createStream(id);

          unsigned int count = stream->tupleQueue.size();
          iosock.write((char*)&count, sizeof(count));
          iosock.flush();
        } else if (cmd.compare("<SourceStream>") == 0) {
          NetworkStream* stream = new NetworkStream(0);
          if (nsb.setDataSourceStream(stream)) {
            string streamType;
            getline(iosock, streamType);

            stream->setStreamType(streamType);

            unsigned int tupleSize = 0;
            char* tupleBuffer = 0;

            int receiveTupleCount = 0;

            do {
              iosock.read((char*)&tupleSize, sizeof(tupleSize));

              if (tupleSize > 0) {
                tupleBuffer = new char[tupleSize];

                iosock.read(tupleBuffer, tupleSize);
                // process tuple

                stream->tupleQueue.add(
                    new pair<unsigned int, char*>(tupleSize, tupleBuffer));
                receiveTupleCount++;
              } else {
                stream->tupleQueue.setComplete();
              }

            } while (tupleSize > 0);

            // confirm received tuple count
            iosock.write((char*)&receiveTupleCount, sizeof(receiveTupleCount));
            iosock.flush();

          } else {
            KOUT << "Error: Can't set source stream.\n";
          }
        } else if (cmd.compare("<Ping/>") == 0) {
          iosock << "<Ping/>" << endl;
        } else if (cmd.compare("<TryRestructureLock/>") == 0) {
          iosock << "<TryRestructureLock>" << endl;
          int result = -1;
          int serverId = 0;
          iosock.read((char*)&serverId, sizeof(serverId));

          int serverIdx = sm.getConnectionIndex(serverId);

          if (serverIdx > -1) {
            Connection* conn = sm.getConnectionIdx(serverIdx);
            if (conn->rLock.tryLockClient()) {
              result = 1;
            } else {
              result = 0;
            }
          }

          iosock.write((char*)&result, sizeof(result));
          iosock.flush();
        } else if (cmd.compare("<UpdateRestructureLock/>") == 0) {
          int serverId = 0;
          iosock.read((char*)&serverId, sizeof(serverId));
          int serverIdx = sm.getConnectionIndex(serverId);

          if (serverIdx > -1) {
            Connection* conn = sm.getConnectionIdx(serverIdx);
            conn->rLock.updateLock();
          }
        } else if (cmd.compare("<UnlockRestructureLock/>") == 0) {
          iosock << "<UnlockRestructureLock>" << endl;
          bool result = false;
          int serverId = 0;
          iosock.read((char*)&serverId, sizeof(serverId));

          int serverIdx = sm.getConnectionIndex(serverId);

          if (serverIdx > -1) {
            Connection* conn = sm.getConnectionIdx(serverIdx);
            result = conn->rLock.unlockClient();
          }

          iosock.write((char*)&result, sizeof(result));
          iosock.flush();
        } else if (cmd.compare("<RequestTransferId/>") == 0) {
          unsigned int tempId = getTransferId();
          iosock << "<TransferId>" << endl;
          iosock.write((char*)&tempId, sizeof(tempId));
          iosock.flush();
        } else if (cmd.compare("<SendDistributionUpdate/>") == 0) {
          bool result = false;
          int len = 0;

          iosock.read((char*)&len, sizeof(len));
          char* tempNameBuffer = new char[len];
          iosock.read(tempNameBuffer, len);
          string distNameBuffer(tempNameBuffer, len);
          delete[] tempNameBuffer;

          iosock.read((char*)&len, sizeof(len));
          char* tempDataBuffer = new char[len];
          iosock.read(tempDataBuffer, len);
          string distDataBuffer(tempDataBuffer, len);
          delete[] tempDataBuffer;

          int distId = getDistributionRef(distNameBuffer);
          if (distId > -1) {
            Distribution* tempDist = getDistribution(distId);
            result = tempDist->fromBin(distDataBuffer);
          } else {
            Distribution* tempDist = Distribution::getInstance(distDataBuffer);
            if (tempDist) {
              addDistribution(tempDist, distNameBuffer);
              result = true;
            }
          }

          iosock.write((char*)&result, sizeof(bool));
          iosock.flush();
        } else if (cmd.compare("<RequestDistribution/>") == 0) {
          cout << "Answering distribution Request..." << endl;
          unsigned int len = 0;

          iosock.read((char*)&len, sizeof(len));
          char* tempNameBuffer = new char[len];
          iosock.read(tempNameBuffer, len);
          string distNameBuffer(tempNameBuffer, len);
          delete[] tempNameBuffer;

          int distId = getDistributionRef(distNameBuffer);
          if (distId > -1) {
            Distribution* tempDist = getDistribution(distId);
            string distDataBuffer = tempDist->toBin();

            len = distDataBuffer.size();
            iosock.write((char*)&len, sizeof(len));
            iosock.write(distDataBuffer.c_str(), len);
          } else {
            // error send 0 len
            len = 0;
            iosock.write((char*)&len, sizeof(len));
          }

          iosock.flush();
          cout << "Finished Answering distribution Request..." << endl;
        } else if (cmd.compare("<RequestSCPPath/>") == 0) {
          iosock << "<RequestSCPPath>" << endl;
          iosock << getSCPTransferPath() << endl;
          iosock.flush();
        } else if (cmd.compare("<RequestServerList/>") == 0) {
          iosock << "<RequestServerList>" << endl;
          iosock << sm.getServerListString() << endl;
          iosock.flush();
        } else if (cmd.compare("<Close/>") == 0) {
          closeConn = true;
          break;
        } else {
        }
      }
    } else {
      iosock << "<Reject/>";
    }
  } catch (ios_base::failure& e) {
    KOUT << client->GetSocketAddress() << " : ios_base::failure exception.\n";
    if (!client->IsOk()) {
      KOUT << client->GetSocketAddress()
           << " : Socket Error: " << client->GetErrorText() << endl;
    }
  }
}

void KeyValueStore::listenStatus() {
  map<int, pair<boost::thread*, Socket*> >::iterator it;
  for (it = listenThreads.begin(); it != listenThreads.end(); ++it) {
    KOUT << "Listening on Port " << it->first << ". Socket Ok? "
         << it->second.second->IsOk() << "\n";
  }
}
}
