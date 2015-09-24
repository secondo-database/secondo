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

#include "TransferMethod.h"
#include "KeyValueStore.h"

#include <algorithm>

namespace KVS {

extern KeyValueStore* kvsInstance;

//
// copied from HadoopParallelAlgebra.cpp which doesn't compile on windows yet
// (may 2015)
//(commands seem to be the same with msys installed)
//
int copyFile(string source, string dest, bool cfn /* = false*/) {
  // bool sRmt = source.find(":") != string::npos ? true : false;
  bool dRmt = dest.find(":") != string::npos ? true : false;

  bool sRmt = false;

  if (!(sRmt ^ dRmt)) {
    // both sides are remote machines
    // or both sides are local machines.
    return -1;
  }

  if (dRmt) {
    string destNode = dest.substr(0, dest.find_first_of(":"));
    dest = dest.substr(dest.find_first_of(":") + 1);
    string sourceFileName = source.substr(source.find_last_of("/") + 1);
    string destFileName = "";
    if (cfn) {
      destFileName = dest.substr(dest.find_last_of("/") + 1);
      dest = dest.substr(0, dest.find_last_of("/") + 1);
    }

    int sourceDepth = 0;
    size_t pos = 0;
    while ((pos = source.find("/", pos)) != string::npos) {
      sourceDepth++;
      pos++;
    }
    sourceDepth = sourceDepth > 0 ? (sourceDepth - 1) : 0;

    stringstream command;
    command << "tar -czf - " << source << " | ssh -oCompression=no " << destNode
            << " \"tar -zxf - -C " << dest << " --strip=" << sourceDepth;
    if (cfn) {
      command << "; mv " << dest << sourceFileName << " " << dest
              << destFileName;
    }
    command << "\"";
    cout << "Command: " << command.str() << "\n";
    return system(command.str().c_str());
  } else {
    string srcNode = source.substr(0, source.find_first_of(":"));
    source = source.substr(source.find_first_of(":") + 1);
    string srcName = source.substr(source.find_last_of("/") + 1);
    string destName = "";
    if (cfn) {
      destName = dest.substr(dest.find_last_of("/") + 1);
    }
    string destPath = dest.substr(0, dest.find_last_of("/") + 1);

    int sourceDepth = 0;
    size_t pos = 0;
    while ((pos = source.find("/", pos)) != string::npos) {
      sourceDepth++;
      pos++;
    }
    sourceDepth = sourceDepth > 0 ? (sourceDepth - 1) : 0;

    stringstream command;
    command << "ssh -oCompression=no " << srcNode << " \"tar -czf - " << source
            << " \" | tar -xzf - -C " << destPath << " --strip=" << sourceDepth;
    if (cfn) {
      command << "; mv " << destPath << srcName << " " << destPath << destName;
    }

    return system(command.str().c_str());
  }
}

TransferMethod::TransferMethod(Connection* connection)
    : transferId(0), tupleCounter(0), connection(connection) {}

TransferMethod::~TransferMethod() {}

void TransferMethod::clearUnconfirmed() {
  for (unsigned int tupleIdx = 0; tupleIdx < unconfirmed.size(); ++tupleIdx) {
    delete unconfirmed[tupleIdx].first;
  }
  unconfirmed.clear();
}

TransferMethodTCP::TransferMethodTCP(Connection* connection, string streamType,
                                     string baseAttributeList,
                                     bool creationCheck)
    : TransferMethod(connection),
      bufferPos(0),
      streamType(streamType),
      baseAttributeList(baseAttributeList),
      creationCheck(creationCheck) {
  // cout<<"Initialized with Type:"<<streamType<<endl;
}

TransferMethodTCP::~TransferMethodTCP() {}

void TransferMethodTCP::changeConnection(Connection* connection) {
  this->connection = connection;
}

bool TransferMethodTCP::init(unsigned int tId) {
  transferId = tId;
  tupleCounter = 0;
  connection->kvsConn->sendStreamType(tId, streamType);

  return true;
}

bool TransferMethodTCP::sendTuple(char* data, unsigned int dataLen) {
  unsigned int tempLen = dataLen + sizeof(unsigned int);

  // KOUT<<"Adding Tuple ("<<dataLen<<")"<<endl;

  if (tempLen > MAX_BUFFER_SIZE - bufferPos) {
    if (bufferPos > 0) {
      connection->kvsConn->sendStream(transferId, buffer, bufferPos);
      bufferPos = 0;
    }

    if (tempLen > MAX_BUFFER_SIZE) {
      char* tempData = new char[dataLen + sizeof(dataLen)];
      memcpy(tempData, (char*)&dataLen, sizeof(dataLen));
      memcpy(tempData + sizeof(dataLen), data, dataLen);
      connection->kvsConn->sendStream(transferId, tempData, tempLen);
      // delete data;
      // data = tempData;
      delete[] tempData;
    } else {
      memcpy(buffer, (char*)&dataLen, sizeof(dataLen));
      memcpy(buffer + sizeof(dataLen), data, dataLen);
      bufferPos = dataLen + sizeof(dataLen);
    }
  } else {
    memcpy(buffer + bufferPos, (char*)&dataLen, sizeof(dataLen));
    bufferPos += sizeof(dataLen);
    memcpy(buffer + bufferPos, data, dataLen);
    bufferPos += dataLen;
  }

  unconfirmed.push_back(make_pair(data, dataLen));
  tupleCounter++;
  return true;
}

bool TransferMethodTCP::endStream() {
  if (bufferPos > 0) {
    connection->kvsConn->sendStream(transferId, buffer, bufferPos);
    bufferPos = 0;
  }
  connection->kvsConn->endStream(transferId);

  int transferredTuples = connection->kvsConn->transferCount(transferId);

  KOUT << "Stream Ended comparring transferredTuples = " << transferredTuples
       << " vs tupleCounter = " << tupleCounter << endl;
  return (transferredTuples == tupleCounter);
}

bool TransferMethodTCP::cleanUp() {
  return connection->kvsConn->removeStream(transferId);
}

bool TransferMethodTCP::import(string targetRelation, string clientCommand) {
  // KOUT<<"calling import"<<endl;
  if (tupleCounter > 0) {
    // KOUT<<"relationExists?"<<endl;
    if (creationCheck && !connection->relationExists(targetRelation)) {
      if (connection->exec("let " + targetRelation + " = kvsRemoteStream(" +
                           stringutils::int2str(transferId) + ") project[" +
                           baseAttributeList + "] consume;")) {
        if (clientCommand.length() > 0) {
          return connection->exec("query " + targetRelation + " feed " +
                                  clientCommand);
        } else {
          return true;
        }
      } else {
        return false;
      }
    }
    // KOUT<<"kvsRemoteStream"<<endl;

    return connection->exec(
        "query kvsRemoteStream(" + stringutils::int2str(transferId) +
        ") project[" + baseAttributeList + "] sort " + targetRelation +
        " feed sort mergediff " + targetRelation + " insert " + clientCommand);
  } else {
    KOUT << "no touples" << endl;
    return true;
  }
}

bool TransferMethodTCP::resendUnconfirmed(unsigned int tId) {
  transferId = tId;
  return connection->kvsConn->sendStream(tId, streamType, unconfirmed);
}

bool TransferMethodTCP::retry() {
  connection->kvsConn->sendStream(transferId, streamType, unconfirmed);
  return endStream();  // && import();
}

// expecting:
// streamType
// sshAddress : USER@IP
// destinationBasePath : scp transfer folder on remote host

TransferMethodSCP::TransferMethodSCP(Connection* connection, string streamType)
    : TransferMethod(connection),
      streamType(streamType),
      localDataPath(""),
      dataFile(0),
      tupleBuffer(0),
      tupleBufferSize(0),
      targetId(connection->id) {
  changeConnection(connection);
}

void TransferMethodSCP::changeConnection(Connection* connection) {
  this->connection = connection;
  targetId = connection->id;
  sshAddress = "secondo@" + connection->host;
  destinationBasePath = connection->kvsConn->requestSCPPath();
}

TransferMethodSCP::~TransferMethodSCP() {
  delete dataFile;
  delete[] tupleBuffer;
}

bool TransferMethodSCP::init(unsigned int tId) {
  transferId = tId;

  localDataPath = kvsInstance->getSCPTransferPath() + PATH_SLASH;
  localDataPath += stringutils::int2str(targetId);
  localDataPath += "_";
  localDataPath += stringutils::int2str(tId);

  dataFile = new ofstream(localDataPath, ios::binary);

  if (dataFile->good()) {
    ofstream typeFile(localDataPath + "_type", ios::binary);

    if (typeFile.good()) {
      size_t typeLen = streamType.length();
      typeFile.write((char*)&typeLen, sizeof(typeLen));
      typeFile.write(streamType.c_str(), typeLen);
      typeFile.close();
      return true;
    }
  }

  return false;
}

bool TransferMethodSCP::sendTuple(char* data, unsigned int dataLen) {
  dataFile->write((char*)&dataLen, sizeof(dataLen));
  dataFile->write(data, dataLen);

  return true;
}

bool TransferMethodSCP::endStream() {
  string tempPath(localDataPath);

// change path so it's compatible with MSYS commands - shouldn't affect unix
#ifdef SECONDO_WIN32
  if (tempPath.find(':', 0) != string::npos) {
    tempPath.erase(tempPath.find(':', 0));
  }
  replace(tempPath.begin(), tempPath.end(), '\\', '/');
  tempPath.insert('/', 0);
#endif

  dataFile->close();

  delete[] tupleBuffer;
  tupleBuffer = 0;
  tupleBufferSize = 0;

  delete dataFile;
  dataFile = 0;

  string targetPath =
      destinationBasePath + "/" + stringutils::int2str(transferId);

  if (copyFile(tempPath + "_type", sshAddress + ":" + targetPath + "_type",
               true)) {
    if (copyFile(tempPath, sshAddress + ":" + targetPath, true)) {
      // delete local copies?

      clearUnconfirmed();
      return true;
    } else {
      KOUT << "Failed to scp copy: " << tempPath << endl;
    }
  } else {
    KOUT << "Failed to scp copy: " << tempPath << "_type" << endl;
  }

  return false;
}

bool TransferMethodSCP::cleanUp() {
  return false;  // connection->kvsConn->removeStream(transferId);
}

bool TransferMethodSCP::import(string targetRelation, string clientCommand) {
  return connection->exec("query kvsRemoteStreamSCP(" +
                          stringutils::int2str(transferId) + ") " +
                          targetRelation + " insert " + clientCommand);
}

bool TransferMethodSCP::resendUnconfirmed(unsigned int tId) {
  transferId = tId;
  // localDataPath ( used by endStream() ) still uses old transfer id, but that
  // should not matter since endStream() uses new transferId for remote host.
  return true;  //;endStream();
}

bool TransferMethodSCP::retry() {
  localDataPath = kvsInstance->getSCPTransferPath() + PATH_SLASH;
  localDataPath += stringutils::int2str(targetId);
  localDataPath += "_";
  localDataPath += stringutils::int2str(transferId);

  string tempPath(localDataPath);

// change path so it's compatible with MSYS commands - shouldn't affect unix
#ifdef SECONDO_WIN32
  if (tempPath.find(':', 0) != string::npos) {
    tempPath.erase(tempPath.find(':', 0));
  }
  replace(tempPath.begin(), tempPath.end(), '\\', '/');
  tempPath.insert('/', 0);
#endif

  string targetPath =
      destinationBasePath + "/" + stringutils::int2str(transferId);

  if (copyFile(tempPath + "_type", sshAddress + ":" + targetPath + "_type",
               true)) {
    if (copyFile(tempPath, sshAddress + ":" + targetPath, true)) {
      // delete local copies?

      clearUnconfirmed();
      return true;
    } else {
      KOUT << "Failed to scp copy: " << tempPath << endl;
    }
  } else {
    KOUT << "Failed to scp copy: " << tempPath << "_type" << endl;
  }

  return false;
}
}
