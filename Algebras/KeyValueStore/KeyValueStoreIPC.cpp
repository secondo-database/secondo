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

#include "KeyValueStoreIPC.h"
#include "IPCMessages.h"
#include "IPCProtocol.h"

#include "Application.h"
#include "FileSystem.h"

#ifdef SECONDO_WIN32
#include "windows.h"
#else
#include <unistd.h>
#endif

using namespace std;

namespace KVS {

KeyValueStoreIPC::KeyValueStoreIPC(int appId)
    : localId(-1), appId(appId), connection(0) {}

KeyValueStoreIPC::~KeyValueStoreIPC() {
  if (connection) {
    if (connection->health()) {
      connection->write(IPC_MSG_CLOSECONNECTION);
    }
    delete connection;
  }
}

string KeyValueStoreIPC::getAppName() {
  string name("KeyValueStore");
#ifdef SECONDO_WIN32
  name += ".exe";
#endif
  return name;
}

string KeyValueStoreIPC::getSCPTransferPath() {
  string path(Application::Instance()->GetApplicationPath());

  FileSystem::AppendItem(path, "kvs");

  if (!FileSystem::FileOrFolderExists(path)) {
    FileSystem::CreateFolder(path);
  }

  return path;
}

string KeyValueStoreIPC::getSCPSourcePath() {
  string path(getSCPTransferPath());

  FileSystem::AppendItem(path, "source");

  if (!FileSystem::FileOrFolderExists(path)) {
    FileSystem::CreateFolder(path);
  }

  return path;
}

bool KeyValueStoreIPC::connect() {
  connection = IPCConnection::connect(appId);
  if (connection) {
    cout << "Opened Connection Id:" << connection->connectionId << endl;
  }

  return connection != 0;
}
bool KeyValueStoreIPC::connect(string execPath) {
  // make sure we don't connect twice
  if (connection) {
    if (!connection->health()) {
      delete connection;
    } else {
      return true;
    }
  }

  connection = IPCConnection::connect(appId);

  if (!connection) {
    cout << "Trying to start Application..." << endl;
    ipcServerStart(execPath);
    boost::this_thread::sleep(boost::posix_time::milliseconds(500));
    connection = IPCConnection::connect(appId);
  }

  if (connection) {
    cout << "Opened Connection Id:" << connection->connectionId << endl;
  }

  return connection != 0;
}

bool KeyValueStoreIPC::connected() { return (connection != 0); }

bool KeyValueStoreIPC::cmd(IPCMessage msg) {
  if (connection) {
    connection->write(msg);
    return true;
  } else {
    cout << "Error: No IPC Connection.\n";
    return false;
  }
}

bool KeyValueStoreIPC::getResult(bool* res) {
  IPCMessage responseType;
  connection->read(&responseType);
  if (responseType == IPC_MSG_RESULT) {
    return connection->read(res);
  } else {
    cout << "Error: Wrong IPC response type.\n";
  }
  return false;
}

bool KeyValueStoreIPC::getResult(int* res) {
  IPCMessage responseType;
  connection->read(&responseType);
  if (responseType == IPC_MSG_RESULT) {
    return connection->read(res);
  } else {
    cout << "Error: Wrong IPC response type.\n";
  }
  return false;
}

bool KeyValueStoreIPC::getResult(unsigned int* res) {
  IPCMessage responseType;
  connection->read(&responseType);
  if (responseType == IPC_MSG_RESULT) {
    return connection->read(res);
  } else {
    cout << "Error: Wrong IPC response type.\n";
  }
  return false;
}

bool KeyValueStoreIPC::getResult(string* data) {
  IPCMessage responseType;
  connection->read(&responseType);
  if (responseType == IPC_MSG_RESULT) {
    return connection->read(data);
  } else {
    cout << "Error: Wrong IPC response type.\n";
  }
  return false;
}

bool KeyValueStoreIPC::getResult(set<int>* res) {
  IPCMessage responseType;
  connection->read(&responseType);
  if (responseType == IPC_MSG_RESULT) {
    unsigned int nrResults;
    if (connection->read(&nrResults)) {
      if (nrResults > 0) {
        int tempResult;
        for (unsigned int i = 0; i < nrResults; ++i) {
          connection->read(&tempResult);
          res->insert(tempResult);
        }
        return true;
      }
    }
  } else {
    cout << "Error: Wrong IPC response type.\n";
  }
  return false;
}

bool KeyValueStoreIPC::addConnection(string host, int interfacePort,
                                     int kvsPort, string config) {
  if (connection) {
    connection->write(IPC_MSG_ADDCONNECTION);
    connection->write(&host);
    connection->write(&interfacePort);
    connection->write(&kvsPort);
    connection->write(&config);

    IPCMessage responseType;
    connection->read(&responseType);
    if (responseType == IPC_MSG_RESULT) {
      bool result = false;
      if (connection->read(&result)) {
        return result;
      }
    } else {
      cout << "Error: Wrong IPC response type.\n";
    }
  } else {
    cout << "Error: No IPC Connection.\n";
  }
  return false;
}

bool KeyValueStoreIPC::removeConnection(int index) {
  if (connection) {
    connection->write(IPC_MSG_REMOVECONNECTION);
    connection->write(&index);

    IPCMessage responseType;
    connection->read(&responseType);
    if (responseType == IPC_MSG_RESULT) {
      bool result = false;
      if (connection->read(&result)) {
        return result;
      }
    } else {
      cout << "Error: Wrong IPC response type.\n";
    }
  } else {
    cout << "Error: No IPC Connection.\n";
  }
  return false;
}

bool KeyValueStoreIPC::retryConnection(int index) {
  if (connection) {
    connection->write(IPC_MSG_RETRYCONNECTION);
    connection->write(&index);

    IPCMessage responseType;
    connection->read(&responseType);
    if (responseType == IPC_MSG_RESULT) {
      bool result = false;
      if (connection->read(&result)) {
        return result;
      }
    } else {
      cout << "Error: Wrong IPC response type.\n";
    }
  } else {
    cout << "Error: No IPC Connection.\n";
  }
  return false;
}

bool KeyValueStoreIPC::syncServerList() {
  if (cmd(IPC_MSG_SYNCSERVERLIST)) {
    bool result = false;
    if (getResult(&result)) {
      return result;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

bool KeyValueStoreIPC::updateServerList(string separatedList) {
  if (connection) {
    connection->write(IPC_MSG_UPDATESERVERLIST);
    connection->write(&separatedList);

    IPCMessage responseType;
    connection->read(&responseType);
    if (responseType == IPC_MSG_RESULT) {
      bool result = false;
      if (connection->read(&result)) {
        return result;
      }
    } else {
      cout << "Error: Wrong IPC response type.\n";
    }
  } else {
    cout << "Error: No IPC Connection.\n";
  }
  return false;
}

string KeyValueStoreIPC::getInformationString() {
  if (connection) {
    connection->write(IPC_MSG_SERVERINFORMATIONSTRING);

    IPCMessage responseType;
    connection->read(&responseType);
    if (responseType == IPC_MSG_RESULT) {
      string result;
      if (connection->read(&result)) {
        return result;
      } else {
        cout << "Fehler beim lesen des Ergebnisses." << endl;
      }
    } else {
      cout << "Error: Wrong IPC response type.\n";
    }
  } else {
    cout << "Error: No IPC Connection.\n";
  }
  return "";
}

bool KeyValueStoreIPC::setDatabase(string databaseName) {
  if (connection) {
    connection->write(IPC_MSG_SETDATABASE);
    connection->write(&databaseName);

    IPCMessage responseType;
    connection->read(&responseType);
    if (responseType == IPC_MSG_RESULT) {
      bool result;
      if (connection->read(&result)) {
        return result;
      }
    } else {
      cout << "Error: Wrong IPC response type.\n";
    }
  } else {
    cout << "Error: No IPC Connection.\n";
  }
  return false;
}

string KeyValueStoreIPC::useDatabase(string databaseName) {
  if (connection) {
    connection->write(IPC_MSG_USEDATABASE);
    connection->write(&databaseName);

    IPCMessage responseType;
    connection->read(&responseType);
    if (responseType == IPC_MSG_RESULT) {
      string result;
      if (connection->read(&result)) {
        return result;
      }
    } else {
      cout << "Error: Wrong IPC response type.\n";
    }
  } else {
    cout << "Error: No IPC Connection.\n";
  }
  return "Failed to use database.\n";
}

unsigned int KeyValueStoreIPC::getTransferId() {
  if (connection) {
    connection->write(IPC_MSG_TRANSFERID);

    IPCMessage responseType;
    connection->read(&responseType);
    if (responseType == IPC_MSG_RESULT) {
      unsigned int result;
      if (connection->read(&result)) {
        return result;
      } else {
        return 0;
      }
    } else {
      cout << "Error: Wrong IPC response type.\n";
    }
  } else {
    cout << "Error: No IPC Connection.\n";
  }
  return 0;
}

unsigned int KeyValueStoreIPC::getGlobalTupelId() {
  if (cmd(IPC_MSG_GLOBALTUPELID)) {
    unsigned int globalTupelId = 0;
    if (getResult(&globalTupelId)) {
      return globalTupelId;
    }
  }
  return 0;
}

bool KeyValueStoreIPC::initClients(string localIp, int localInterfacePort,
                                   int localKvsPort) {
  if (cmd(IPC_MSG_INITCLIENTS)) {
    connection->write(&localIp);
    connection->write(&localInterfacePort);
    connection->write(&localKvsPort);

    bool result = false;
    if (getResult(&result)) {
      return result;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

bool KeyValueStoreIPC::startClient(int port) {
  if (!connection || !connection->health()) {
    connect();
  }

  if (connection) {
    connection->write(IPC_MSG_STARTCLIENT);
    connection->write(&port);

    IPCMessage responseType;
    connection->read(&responseType);
    if (responseType == IPC_MSG_RESULT) {
      bool result = false;
      if (connection->read(&result)) {
        return result;
      }
    } else {
      cout << "Error: Wrong IPC response type.\n";
    }
  } else {
    cout << "Error: No IPC Connection.\n";
  }
  return false;
}

bool KeyValueStoreIPC::stopClient(int port) {
  if (connection) {
    connection->write(IPC_MSG_STOPCLIENT);
    connection->write(&port);

    IPCMessage responseType;
    connection->read(&responseType);
    if (responseType == IPC_MSG_RESULT) {
      bool result = false;
      if (connection->read(&result)) {
        return result;
      }
    } else {
      cout << "Error: Wrong IPC response type.\n";
    }
  } else {
    cout << "Error: No IPC Connection.\n";
  }
  return false;
}

bool KeyValueStoreIPC::setId(int id) {
  if (cmd(IPC_MSG_SETID)) {
    connection->write(&id);

    bool result = false;
    if (getResult(&result)) {
      if (result) {
        localId = id;
      }
      return result;
    }
  }
  return false;
}

bool KeyValueStoreIPC::setMaster(string host, int interfacePort, int kvsPort) {
  if (cmd(IPC_MSG_SETMASTER)) {
    connection->write(&host);
    connection->write(&interfacePort);
    connection->write(&kvsPort);

    bool result = false;
    if (getResult(&result)) {
      return result;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

int KeyValueStoreIPC::tryRestructureLock() {
  if (cmd(IPC_MSG_TRYRESTRUCTURELOCK)) {
    int result = -1;
    if (getResult(&result)) {
      return result;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

bool KeyValueStoreIPC::updateRestructureLock() {
  if (cmd(IPC_MSG_UPDATERESTRUCTURELOCK)) {
    bool result = false;
    if (getResult(&result)) {
      return result;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

bool KeyValueStoreIPC::unlockRestructureLock() {
  if (cmd(IPC_MSG_UNLOCKRESTRUCTURELOCK)) {
    bool result = false;
    if (getResult(&result)) {
      return result;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

int KeyValueStoreIPC::getDistributionRef(string name) {
  if (cmd(IPC_MSG_DISTRIBUTIONREF)) {
    connection->write(&name);

    int result = -1;
    if (getResult(&result)) {
      return result;
    }
  }
  return -1;
}

int KeyValueStoreIPC::getDistributionRef(string name, int typeId, string data) {
  if (cmd(IPC_MSG_DISTRIBUTIONREFSET)) {
    connection->write(&name);
    connection->write(&typeId);
    connection->write(&data);

    int result = -1;
    if (getResult(&result)) {
      return result;
    }
  }
  return -1;
}

bool KeyValueStoreIPC::getDistributionData(int refId, string* data) {
  if (cmd(IPC_MSG_DISTRIBUTIONDATA)) {
    connection->write(&refId);

    if (getResult(data)) {
      return true;
    }
  }

  return false;
}

bool KeyValueStoreIPC::execCommand(string command) {
  if (cmd(IPC_MSG_EXECCOMMAND)) {
    connection->write(&command);

    bool result = false;

    if (getResult(&result)) {
      return result;
    } else {
      return false;
    }

  } else {
    return false;
  }
}

bool KeyValueStoreIPC::qtDistAdd(int refId, int nrcoords, double* coords,
                                 set<int>* resultIds) {
  if (cmd(IPC_MSG_ADDDISTRIBUTIONELEM)) {
    connection->write(&refId);
    connection->write(&nrcoords);
    for (int i = 0; i < nrcoords; ++i) {
      connection->write((char*)&coords[i], sizeof(double));
    }
    return getResult(resultIds);
  } else {
    return false;
  }
}

bool KeyValueStoreIPC::qtDistRequest(int refId, int nrcoords, double* coords,
                                     set<int>* resultIds) {
  if (cmd(IPC_MSG_REQUESTDISTRIBUTIONELEM)) {
    connection->write(&refId);
    connection->write(&nrcoords);
    for (int i = 0; i < nrcoords; ++i) {
      connection->write((char*)&coords[i], sizeof(double));
    }
    return getResult(resultIds);
  } else {
    return false;
  }
}

bool KeyValueStoreIPC::qtDistinct(int refId, double x, double y) {
  if (cmd(IPC_MSG_QTDISTINCT)) {
    connection->write(&refId);
    connection->write(&x);
    connection->write(&y);

    bool result = false;
    if (getResult(&result)) {
      return result;
    }
  }
  return false;
}

bool KeyValueStoreIPC::distAddRect(int refId, int nrcoords, double* coords,
                                   set<int>* resultIds, bool requestOnly) {
  if (cmd(IPC_MSG_ADDDISTRIBUTIONRECT)) {
    IPCArray<double> tempArray(coords, nrcoords);

    if (IPCMessageDistAddRect::write(connection, &refId, &requestOnly,
                                     &tempArray)) {
      IPCMessage resultMsg = IPC_MSG_RESULT;
      return IPCMessageIntSetResult::read(connection, &resultMsg, resultIds);
    }
  }
  return false;
}

bool KeyValueStoreIPC::distAddInt(int refId, int value, set<int>* resultIds,
                                  bool requestOnly) {
  if (cmd(IPC_MSG_ADDDISTRIBUTIONINT)) {
    connection->write(&refId);
    connection->write(&requestOnly);
    connection->write(&value);

    return getResult(resultIds);
  } else {
    return false;
  }
}

bool KeyValueStoreIPC::distFilter(int& refId, int& nrcoords, double* coords,
                                  unsigned int& globalId, bool& update) {
  if (cmd(IPC_MSG_FILTERDISTRIBUTION)) {
    IPCArray<double> tempArray(coords, nrcoords);

    if (IPCMessageDistFilter::write(connection, &refId, &update, &globalId,
                                    &tempArray)) {
      IPCMessage message;
      bool result = false;
      if (IPCMessageBoolResult::read(connection, &message, &result)) {
        if (message == IPC_MSG_RESULT) {
          return result;
        } else {
          cout << "Error: Wrong IPC-Message result..." << endl;
        }
      }
    }
  }

  return false;
}

int KeyValueStoreIPC::qtDistPointId(int refId, double interx, double intery) {
  if (connection) {
    connection->write(IPC_MSG_DISTRIBUTIONPOINTID);
    connection->write(&refId);
    connection->write(&interx);
    connection->write(&intery);

    IPCMessage responseType;
    connection->read(&responseType);
    if (responseType == IPC_MSG_RESULT) {
      int resultId = -1;
      if (connection->read(&resultId)) {
        return resultId;
      }
    } else {
      cout << "Error: Wrong IPC response type.\n";
    }
  } else {
    cout << "Error: No IPC Connection.\n";
  }
  return -1;
}

NetworkStreamIPC* KeyValueStoreIPC::getNetworkStream(unsigned int id) {
  if (streams.find(id) == streams.end()) {
    IPCConnection* distributeConn = IPCConnection::connect(0);

    if (distributeConn) {
      cout << "Opened Connection Id (NW):" << distributeConn->connectionId
           << endl;
    }

    NetworkStreamIPC* nstreamIPC = new NetworkStreamIPC(distributeConn, id);

    if (nstreamIPC->init()) {
      streams.insert(make_pair(id, nstreamIPC));
      return nstreamIPC;
    } else {
      delete nstreamIPC;
      return 0;
    }
  } else {
    return streams.find(id)->second;
  }
}

void KeyValueStoreIPC::removeNetworkStream(unsigned int id) {
  map<unsigned int, NetworkStreamIPC*>::iterator item = streams.find(id);
  if (item != streams.end()) {
    delete item->second;
    streams.erase(item);
  }
}

#ifdef SECONDO_WIN32

void KeyValueStoreIPC::ipcServerStart(string execPath) {
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  CreateProcessA(execPath.c_str(), NULL, NULL, NULL, FALSE,
                 NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);
}

#else

void KeyValueStoreIPC::ipcServerStart(string execPath) {
  pid_t pid;

  pid = fork();
  if (pid == 0) {
    execl(execPath.c_str(), getAppName().c_str(), (char*)0);
  }
}

#endif
}
