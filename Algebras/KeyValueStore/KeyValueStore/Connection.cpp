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

#include "Connection.h"

#include "RelationAlgebra.h"

extern NestedList* nl;

namespace KVS {

KVSConnection::KVSConnection(string host, string port)
    : connection(0), host(host), port(port) {}

KVSConnection::~KVSConnection() { close(); }

bool KVSConnection::connect() {
  boost::lock_guard<boost::mutex> guard(mtx);
  connection = Socket::Connect(host, port, Socket::SockGlobalDomain);

  if (connection && connection->IsOk()) {
    string init;
    iostream& iosock = connection->GetSocketStream();
    getline(iosock, init);

    if (init.compare("<SecondoKVS/>") == 0) {
      iosock << "<SecondoKVS/>" << endl;
      iosock.flush();
      return true;
    }
  }
  return false;
}

bool KVSConnection::check() {
  bool health = true;
  mtx.lock();
  if (!connection || !connection->IsOk()) {
    health = false;
  }
  mtx.unlock();

  if (!health && !connect()) {
    return false;
  }

  boost::lock_guard<boost::mutex> guard(mtx);

  string ping;

  iostream& iosock = connection->GetSocketStream();
  iosock << "<Ping/>" << endl;
  getline(iosock, ping);

  return (ping.compare("<Ping/>") == 0);
}

void KVSConnection::close() {
  boost::lock_guard<boost::mutex> guard(mtx);
  if (connection) {
    iostream& iosock = connection->GetSocketStream();
    iosock << "<Close/>\n" << endl;
    iosock.flush();
    connection->Close();
    delete connection;
    connection = 0;
  }
}

// add queue parameter
/*void KVSConnection::sendStream(const string& id, const string& streamType,
SyncPseudoQueue<Tuple*>* queue, bool* result) {
  if(connection) {
    iostream& iosock = connection->GetSocketStream();
    iosock<<"<RemoteStream>"<<endl;
    iosock<<id<<endl;
    iosock<<streamType<<endl;

    Tuple* tuple;

    size_t tupleBufferSize = 0;
    char* tupleBuffer = 0;

    size_t tupleBlockSize;
    size_t coreSize = 0;
    size_t extensionSize = 0;
    size_t flobSize = 0;

    int sendTupleCount = 0;

    while( (tuple = queue->next()) != 0) {
      tupleBlockSize = tuple->GetBlockSize(coreSize, extensionSize, flobSize);

      if(tupleBlockSize > tupleBufferSize) {
        delete[] tupleBuffer;
        tupleBuffer = new char[tupleBlockSize];
        tupleBufferSize = tupleBlockSize;
      }

      tuple->WriteToBin(tupleBuffer, coreSize, extensionSize, flobSize);

      iosock.write((char*)&tupleBlockSize, sizeof(tupleBlockSize));
      iosock.write(tupleBuffer, tupleBlockSize);

      sendTupleCount++;
    }

    unsigned int close = 0;
    iosock.write((char*)&close, sizeof(close));

    iosock.flush();

    //get confirmation that all tuple have been received

    int receiveTupleCount = 0;

    iosock.read((char*)&receiveTupleCount, sizeof(receiveTupleCount));

    *result = (sendTupleCount == receiveTupleCount);
  } else {
    *result = false;
  }
}*/

bool KVSConnection::sendStream(const int& id, const string& streamType,
                               const vector<pair<char*, unsigned int> >& data) {
  boost::lock_guard<boost::mutex> guard(mtx);
  if (connection) {
    iostream& iosock = connection->GetSocketStream();
    iosock << "<RemoteStream>" << endl;
    iosock.write((char*)&id, sizeof(id));
    unsigned int len = streamType.length();
    iosock.write((char*)&len, sizeof(len));
    iosock.write((char*)streamType.c_str(), len);

    int sendTupleCount = 0;

    for (unsigned int tupleIdx = 0; tupleIdx < data.size(); ++tupleIdx) {
      iosock.write((char*)&data[tupleIdx].second, sizeof(unsigned int));
      iosock.write(data[tupleIdx].first, data[tupleIdx].second);

      sendTupleCount++;
    }

    unsigned int close = 0;
    iosock.write((char*)&close, sizeof(close));
    iosock.flush();

    // get confirmation that all tuple have been received

    int receiveTupleCount = 0;

    iosock.read((char*)&receiveTupleCount, sizeof(receiveTupleCount));

    return (sendTupleCount == receiveTupleCount);
  } else {
    return false;
  }
}

bool KVSConnection::sendStream(const int& id, const char* data,
                               const unsigned int& dataLen) {
  boost::lock_guard<boost::mutex> guard(mtx);
  if (connection) {
    // cout<<"Sending StreamPart ("<<dataLen<<")"<<endl;
    iostream& iosock = connection->GetSocketStream();
    iosock << "<RemoteStreamPart>" << endl;
    iosock.write((char*)&id, sizeof(id));
    // iosock.write((char*)&dataLen, sizeof(int)); //data should end in 0 so we
    // don#t need the length
    iosock.write(data, dataLen);
    unsigned int close = 0;
    iosock.write((char*)&close, sizeof(close));
    iosock.flush();
    return true;
  } else {
    return false;
  }
}

bool KVSConnection::sendStreamType(const int& id, const string& streamType) {
  boost::lock_guard<boost::mutex> guard(mtx);
  if (connection) {
    iostream& iosock = connection->GetSocketStream();
    iosock << "<RemoteStreamType>" << endl;
    iosock.write((char*)&id, sizeof(id));

    unsigned int len = streamType.length();
    iosock.write((char*)&len, sizeof(len));
    iosock.write(streamType.c_str(), len);
    iosock.flush();
    return true;
  } else {
    return false;
  }
}

bool KVSConnection::endStream(const int& id) {
  boost::lock_guard<boost::mutex> guard(mtx);
  if (connection) {
    iostream& iosock = connection->GetSocketStream();
    iosock << "<EndRemoteStream>" << endl;
    iosock.write((char*)&id, sizeof(id));
    iosock.flush();
    return true;
  } else {
    return false;
  }
}
unsigned int KVSConnection::transferCount(const int& id) {
  boost::lock_guard<boost::mutex> guard(mtx);
  if (connection) {
    iostream& iosock = connection->GetSocketStream();
    iosock << "<RemoteStreamCount>" << endl;
    iosock.write((char*)&id, sizeof(id));
    iosock.flush();

    unsigned int result = 0;
    iosock.read((char*)&result, sizeof(result));
    return result;
  } else {
    return 0;
  }
}

bool KVSConnection::removeStream(const int& id) {
  boost::lock_guard<boost::mutex> guard(mtx);
  if (connection) {
    iostream& iosock = connection->GetSocketStream();
    iosock << "<RemoveRemoteStream>" << endl;
    iosock.write((char*)&id, sizeof(id));
    iosock.flush();

    bool result = false;
    iosock.read((char*)&result, sizeof(result));
    return result;
  } else {
    return false;
  }
}

bool KVSConnection::sendDistributionUpdate(const string& distributionName,
                                           const string& data) {
  boost::lock_guard<boost::mutex> guard(mtx);
  if (connection) {
    iostream& iosock = connection->GetSocketStream();
    iosock << "<SendDistributionUpdate/>" << endl;
    unsigned int len = 0;

    len = distributionName.length();
    iosock.write((char*)&len, sizeof(len));
    iosock.write(distributionName.c_str(), len);

    len = data.length();
    iosock.write((char*)&len, sizeof(len));
    iosock.write(data.c_str(), len);
    iosock.flush();

    bool result = false;
    iosock.read((char*)&result, sizeof(bool));
    return result;
  } else {
    return false;
  }
}

bool KVSConnection::requestDistribution(const string& distributionName,
                                        string& data) {
  boost::lock_guard<boost::mutex> guard(mtx);
  if (connection) {
    iostream& iosock = connection->GetSocketStream();
    iosock << "<RequestDistribution/>" << endl;

    unsigned int len = distributionName.length();
    iosock.write((char*)&len, sizeof(len));
    iosock.write(distributionName.c_str(), len);

    len = 0;
    iosock.read((char*)&len, sizeof(len));

    if (len > 0) {
      char* tempData = new char[len];
      iosock.read(tempData, len);
      data.assign(tempData, len);
      delete[] tempData;
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

unsigned int KVSConnection::requestTransferId() {
  boost::lock_guard<boost::mutex> guard(mtx);
  if (connection) {
    iostream& iosock = connection->GetSocketStream();
    iosock << "<RequestTransferId/>" << endl;

    string response;

    getline(iosock, response);

    if (response.compare("<TransferId>") == 0) {
      unsigned int transferId;
      iosock.read((char*)&transferId, sizeof(transferId));
      return transferId;
    } else {
      return 0;
    }
  } else {
    return 0;
  }
}

string KVSConnection::requestServerList() {
  boost::lock_guard<boost::mutex> guard(mtx);
  if (connection) {
    iostream& iosock = connection->GetSocketStream();
    iosock << "<RequestServerList/>" << endl;

    string response;
    getline(iosock, response);

    if (response.compare("<RequestServerList>") == 0) {
      string serverList;
      getline(iosock, serverList);
      return serverList;
    }
  }
  return "";
}

string KVSConnection::requestSCPPath() {
  boost::lock_guard<boost::mutex> guard(mtx);
  if (connection) {
    iostream& iosock = connection->GetSocketStream();
    iosock << "<RequestSCPPath/>" << endl;

    string response;
    getline(iosock, response);

    if (response.compare("<RequestSCPPath>") == 0) {
      string scpPath;
      getline(iosock, scpPath);
      return scpPath;
    }
  }

  // default
  return "/home/secondo/bin/kvs";
}

int KVSConnection::tryRestructureLock(int id) {
  boost::lock_guard<boost::mutex> guard(mtx);
  if (connection) {
    iostream& iosock = connection->GetSocketStream();
    iosock << "<TryRestructureLock/>" << endl;
    iosock.write((char*)&id, sizeof(id));
    iosock.flush();

    string response;
    getline(iosock, response);

    if (response.compare("<TryRestructureLock>") == 0) {
      int response = -1;
      iosock.read((char*)&response, sizeof(response));
      return response;
    }
  }
  return -1;
}

bool KVSConnection::updateRestructureLock() {
  boost::lock_guard<boost::mutex> guard(mtx);
  if (connection) {
    iostream& iosock = connection->GetSocketStream();
    iosock << "<UpdateRestructureLock/>" << endl;
    return true;
  }
  return false;
}
bool KVSConnection::unlockRestructureLock(int id) {
  boost::lock_guard<boost::mutex> guard(mtx);
  if (connection) {
    iostream& iosock = connection->GetSocketStream();
    iosock << "<UnlockRestructureLock/>" << endl;
    iosock.write((char*)&id, sizeof(id));
    iosock.flush();

    string response;
    getline(iosock, response);

    if (response.compare("<UnlockRestructureLock>") == 0) {
      bool response = false;
      iosock.read((char*)&response, sizeof(bool));
      return response;
    }
  }
  return false;
}

Connection::Connection(const string& host, const int interfacePort,
                       const int kvsPort, const string& config)
    : id(-1),
      host(host),
      interfacePort(interfacePort),
      kvsPort(kvsPort),
      config(config),
      status(Connection::Unknown),
      interfaceConn(0),
      restructureInProgress(false),
      mynl(0) {
  kvsConn = new KVSConnection(host, stringutils::int2str(kvsPort));
}

// constexpr char* Connection::ConnectionStatusNames[] = { "Unknown",
// "Connecting", "Connected", "Failed", "Problem" };

Connection::~Connection() {
  if (interfaceConn) {
    interfaceMutex.lock();
    interfaceConn->Terminate();
    delete interfaceConn;
    interfaceMutex.unlock();
  }

  if (mynl) {
    delete mynl;
  }

  delete kvsConn;
}

bool Connection::check() {
  if (interfaceConn) {
    ListExpr res;
    string cmd = "list databases";
    SecErrInfo err;
    interfaceMutex.lock();
    interfaceConn->Secondo(cmd, res, err);
    interfaceMutex.unlock();
    return err.code == 0;
  } else {
    return false;
  }
}

bool Connection::connectInterface() {
  string errorMsg("");
  if (!interfaceConn) {
    mynl = new NestedList();
    interfaceConn = new SecondoInterfaceCS(true, mynl);
    interfaceConn->setMaxAttempts(4);
    interfaceConn->setTimeout(1);
  }

  if (interfaceConn->Initialize("", "", host,
                                stringutils::int2str(interfacePort), config,
                                errorMsg, true)) {
    return true;
  } else {
    delete interfaceConn;
    interfaceConn = 0;
    delete mynl;
    mynl = 0;

    KOUT << "Connection failed: " << errorMsg << endl;

    return false;
  }
}

bool Connection::initInterface(string localIp, int localInterfacePort,
                               int localKvsPort, string databaseName) {
  string errorMsg("");
  if (!check()) {
    if (!interfaceConn) {
      mynl = new NestedList();
      interfaceConn = new SecondoInterfaceCS(true, mynl);
      interfaceConn->setMaxAttempts(4);
      interfaceConn->setTimeout(1);
    }

    KOUT << "Connecting to Interface:";
    if (!interfaceConn->Initialize("", "", host,
                                   stringutils::int2str(interfacePort), config,
                                   errorMsg, true)) {
      KOUT << "Problem: " << errorMsg << endl;
      return false;
    } else {
      KOUT << "Success!" << endl;
    }
  }

  // init commands
  exec("close database");
  if (!exec("open database " + databaseName)) {
    KOUT << "open database " << databaseName << " failed." << endl;
  }

  if (!exec("query kvsStartApp()", true)) {
    return false;
  }

  if (!exec("query kvsStartClient(" + stringutils::int2str(kvsPort) + ")",
            true)) {
    return false;
  }

  if (!exec("query kvsSetId(" + stringutils::int2str(id) + ")", true)) {
    return false;
  }

  if (!exec("query kvsSetMaster('" + localIp + "'," +
                stringutils::int2str(localInterfacePort) + "," +
                stringutils::int2str(localKvsPort) + ")",
            true)) {
    return false;
  }

  if (!exec("query kvsSyncServerList()", true)) {
    return false;
  }
  return true;
}

void Connection::simpleCommand(string command, int& err, string& result) {
  if (interfaceConn) {
    SecErrInfo serr;
    ListExpr resList;
    interfaceMutex.lock();
    interfaceConn->Secondo(command, resList, serr);
    err = serr.code;
    if (err == 0) {
      result = mynl->ToString(resList);
    } else {
      result = interfaceConn->GetErrorMessage(err);
    }
    interfaceMutex.unlock();
  }
}

void Connection::simpleCommand(string command, int& error, string& errMsg,
                               string& resList) {
  if (interfaceConn) {
    SecErrInfo serr;
    ListExpr myResList = mynl->TheEmptyList();
    interfaceMutex.lock();
    interfaceConn->Secondo(command, myResList, serr);
    error = serr.code;
    if (error != 0) {
      errMsg = interfaceConn->GetErrorMessage(error);
    }
    interfaceMutex.unlock();

    resList = mynl->ToString(myResList);
    mynl->Destroy(myResList);
  }
}

void Connection::simpleCommand(string command, int& error, string& errMsg,
                               ListExpr& resList) {
  if (interfaceConn) {
    SecErrInfo serr;
    ListExpr myResList = mynl->TheEmptyList();
    interfaceMutex.lock();
    interfaceConn->Secondo(command, myResList, serr);
    error = serr.code;
    if (error != 0) {
      errMsg = interfaceConn->GetErrorMessage(error);
    }
    interfaceMutex.unlock();
    // copy resultlist from local nested list to global nested list
    static boost::mutex copylistmutex;
    copylistmutex.lock();
    assert(mynl != nl);
    resList = mynl->CopyList(myResList, nl);
    mynl->Destroy(myResList);
    copylistmutex.unlock();
  }
}

bool Connection::exec(string command) {
  if (interfaceConn) {
    boost::lock_guard<boost::mutex> guard(interfaceMutex);
    SecErrInfo serr;
    ListExpr resList;

    interfaceConn->Secondo(command, resList, serr);
    if (serr.code != 0) {
      KOUT << "Error while executing command (" << command
           << "): " << interfaceConn->GetErrorMessage(serr.code) << endl;
      return false;
    } else {
      return true;
    }
  } else {
    return false;
  }
}

bool Connection::exec(string command, const bool& expected) {
  if (interfaceConn) {
    boost::lock_guard<boost::mutex> guard(interfaceMutex);
    SecErrInfo serr;
    ListExpr resList;

    interfaceConn->Secondo(command, resList, serr);
    if (serr.code != 0) {
      KOUT << "Error while executing command (" << command
           << "): " << interfaceConn->GetErrorMessage(serr.code) << endl;
      return false;
    } else {
      if (mynl->ListLength(resList) == 2 &&
          mynl->ToString(mynl->First(resList)).compare(CcBool::BasicType()) ==
              0) {  // CcBool::checkType(mynl->First(resList))) {
        return mynl->BoolValue(mynl->Second(resList)) == expected;
      } else {
        return false;
      }
    }
  } else {
    return false;
  }
}

bool Connection::exec(string command, const int& expected) {
  if (interfaceConn) {
    boost::lock_guard<boost::mutex> guard(interfaceMutex);
    SecErrInfo serr;
    ListExpr resList;

    interfaceConn->Secondo(command, resList, serr);
    if (serr.code != 0) {
      KOUT << "Error while executing command (" << command
           << "): " << interfaceConn->GetErrorMessage(serr.code) << endl;
      return false;
    } else {
      if (mynl->ListLength(resList) == 2 &&
          mynl->ToString(mynl->First(resList)).compare(CcInt::BasicType()) ==
              0) {  // CcBool::checkType(mynl->First(resList))) {
        return mynl->IntValue(mynl->Second(resList)) == expected;
      } else {
        return false;
      }
    }
  } else {
    return false;
  }
}

bool Connection::relationExists(string relationName) {
  // TODO: use actual exists function that's probably hiding somewhere?
  return exec("query " + relationName + " feed head[0] count", 0);
}
}
