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

#ifndef KEYVALUESTOREDISPATCH_H_
#define KEYVALUESTOREDISPATCH_H_

#include "ipc.h"
#include "KeyValueStore.h"
#include "KeyValueStoreIPCServer.h"
#include "IPCMessages.h"

namespace KVS {

int closeConnection(KeyValueStore* instance, IPCConnection* connection) {
  delete connection;
  return KeyValueStoreIPCServer::REMOVECONNECTION;
}

int addConnection(KeyValueStore* instance, IPCConnection* connection) {
  string host, config;
  int interfacePort, kvsPort;

  if (connection) {
    connection->read(&host);
    connection->read(&interfacePort);
    connection->read(&kvsPort);
    connection->read(&config);

    bool result = instance->addConnection(host, interfacePort, kvsPort, config);
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int removeConnection(KeyValueStore* instance, IPCConnection* connection) {
  int index;

  if (connection) {
    connection->read(&index);

    bool result = instance->removeConnection(index);
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int retryConnection(KeyValueStore* instance, IPCConnection* connection) {
  int index;

  if (connection) {
    connection->read(&index);

    bool result = instance->retryConnection(index);
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int syncServerList(KeyValueStore* instance, IPCConnection* connection) {
  string separatedList;

  if (connection) {
    bool result = instance->syncServerList();
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int updateServerList(KeyValueStore* instance, IPCConnection* connection) {
  string separatedList;

  if (connection) {
    connection->read(&separatedList);

    bool result = instance->updateServerList(separatedList);
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int serverInformationString(KeyValueStore* instance,
                            IPCConnection* connection) {
  if (connection) {
    string result = instance->serverInformationString();
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int setDatabase(KeyValueStore* instance, IPCConnection* connection) {
  string databaseName;

  cout << "Dispatching setDatabase" << endl;

  if (connection) {
    connection->read(&databaseName);

    instance->setDatabase(databaseName);
    bool result = true;
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int useDatabase(KeyValueStore* instance, IPCConnection* connection) {
  string databaseName;

  cout << "Dispatching useDatabase" << endl;

  if (connection) {
    connection->read(&databaseName);

    string result = instance->useDatabase(databaseName);
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

unsigned int transferId(KeyValueStore* instance, IPCConnection* connection) {
  if (connection) {
    unsigned int result = instance->getTransferId();
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

unsigned int globalTupelId(KeyValueStore* instance, IPCConnection* connection) {
  if (connection) {
    unsigned int result = instance->getGlobalTupelId();
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int initClients(KeyValueStore* instance, IPCConnection* connection) {
  string host;
  int interfacePort;
  int kvsPort;

  connection->read(&host);
  connection->read(&interfacePort);
  connection->read(&kvsPort);

  bool result = instance->initClients(host, interfacePort, kvsPort);
  connection->write(IPC_MSG_RESULT);
  connection->write(&result);

  return KeyValueStoreIPCServer::NORESULT;
}

int startClient(KeyValueStore* instance, IPCConnection* connection) {
  int port;

  if (connection) {
    connection->read(&port);

    bool result = instance->startClient(port);
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int stopClient(KeyValueStore* instance, IPCConnection* connection) {
  int port;

  if (connection) {
    connection->read(&port);

    bool result = instance->stopClient(port);
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int setId(KeyValueStore* instance, IPCConnection* connection) {
  int id;

  if (connection) {
    connection->read(&id);

    instance->id = id;
    bool result = true;
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int setMaster(KeyValueStore* instance, IPCConnection* connection) {
  string host;
  int interfacePort;
  int kvsPort;

  if (connection) {
    connection->read(&host);
    connection->read(&interfacePort);
    connection->read(&kvsPort);

    bool result = instance->setMaster(host, interfacePort, kvsPort);
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int tryRestructureLock(KeyValueStore* instance, IPCConnection* connection) {
  if (connection) {
    int result = instance->tryRestructureLock();
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int updateRestructureLock(KeyValueStore* instance, IPCConnection* connection) {
  if (connection) {
    bool result = instance->updateRestructureLock();
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int unlockRestructureLock(KeyValueStore* instance, IPCConnection* connection) {
  if (connection) {
    bool result = instance->unlockRestructureLock();
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int distributionRef(KeyValueStore* instance, IPCConnection* connection) {
  string name;

  if (connection) {
    connection->read(&name);

    int result = instance->getDistributionRef(name);
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int distributionRefSet(KeyValueStore* instance, IPCConnection* connection) {
  string name;
  int typeId;
  string data;

  if (connection) {
    connection->read(&name);
    connection->read(&typeId);
    connection->read(&data);

    int result = instance->getDistributionRef(name, typeId, data);
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int distributionData(KeyValueStore* instance, IPCConnection* connection) {
  int refId;

  if (connection) {
    connection->read(&refId);

    string result("");
    Distribution* dist = instance->getDistribution(refId);
    if (dist) {
      result = dist->toBin();
    }

    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int addDistributionElement(KeyValueStore* instance, IPCConnection* connection) {
  int refId;
  int nrcoords;
  double* coords;
  set<int> resultIds;

  if (connection) {
    connection->read(&refId);
    connection->read(&nrcoords);
    coords = new double[nrcoords];
    for (int i = 0; i < nrcoords; ++i) {
      connection->read((char*)&coords[i], sizeof(double));
    }

    instance->qtDistAdd(refId, nrcoords, coords, &resultIds);

    connection->write(IPC_MSG_RESULT);
    unsigned int nrResults = resultIds.size();
    connection->write(&nrResults);
    for (set<int>::iterator it = resultIds.begin(); it != resultIds.end();
         ++it) {
      int tempRes = *it;
      connection->write(&tempRes);
    }
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int addDistributionRect(KeyValueStore* instance, IPCConnection* connection) {
  int refId;
  bool requestOnly;
  int nrcoords;
  double* coords;
  set<int> resultIds;

  if (connection) {
    assert(connection->read(&refId));
    assert(connection->read(&requestOnly));
    assert(connection->read(&nrcoords));
    coords = new double[nrcoords];

    // TODO:
    assert(nrcoords == 4);

    for (int i = 0; i < nrcoords; ++i) {
      connection->read((char*)&coords[i], sizeof(double));
    }

    instance->distAddRect(refId, nrcoords, coords, &resultIds, requestOnly);

    connection->write(IPC_MSG_RESULT);
    unsigned int nrResults = resultIds.size();
    connection->write(&nrResults);
    for (set<int>::iterator it = resultIds.begin(); it != resultIds.end();
         ++it) {
      int tempRes = *it;
      connection->write(&tempRes);
    }

    if (nrResults == 0) {
      KOUT << "Error: 0 results" << endl;
      for (int i = 0; i < nrcoords; ++i) {
        KOUT << "coords[" << i << "] = " << coords[i] << endl;
      }
      instance->distAddRectDebug(refId, nrcoords, coords, &resultIds,
                                 requestOnly);
    }
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int addDistributionInt(KeyValueStore* instance, IPCConnection* connection) {
  int refId;
  bool requestOnly;
  int value;
  set<int> resultIds;

  if (connection) {
    connection->read(&refId);
    connection->read(&requestOnly);
    connection->read(&value);

    instance->distAddInt(refId, value, &resultIds, requestOnly);

    connection->write(IPC_MSG_RESULT);
    unsigned int nrResults = resultIds.size();
    connection->write(&nrResults);
    for (set<int>::iterator it = resultIds.begin(); it != resultIds.end();
         ++it) {
      int tempRes = *it;
      connection->write(&tempRes);
    }
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int filterDistribution(KeyValueStore* instance, IPCConnection* connection) {
  int refId;
  bool update;
  unsigned int globalId;
  int nrcoords;
  double* coords;

  if (connection) {
    assert(connection->read(&refId));
    assert(connection->read(&update));
    assert(connection->read(&globalId));
    assert(connection->read(&nrcoords));

    // TODO:
    assert(nrcoords == 4);

    coords = new double[nrcoords];
    for (int i = 0; i < nrcoords; ++i) {
      connection->read((char*)&coords[i], sizeof(double));
    }

    bool result =
        instance->distFilter(refId, nrcoords, coords, globalId, update);
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int qtDistinct(KeyValueStore* instance, IPCConnection* connection) {
  int refId;
  double x, y;

  if (connection) {
    connection->read(&refId);
    connection->read(&x);
    connection->read(&y);

    bool result = instance->qtDistinct(refId, x, y);
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int requestDistributionElement(KeyValueStore* instance,
                               IPCConnection* connection) {
  int refId;
  int nrcoords;
  double* coords;
  set<int> resultIds;

  if (connection) {
    connection->read(&refId);
    connection->read(&nrcoords);
    coords = new double[nrcoords];
    for (int i = 0; i < nrcoords; ++i) {
      connection->read((char*)&coords[i], sizeof(double));
    }

    instance->qtDistRequest(refId, nrcoords, coords, &resultIds);

    connection->write(IPC_MSG_RESULT);
    unsigned int nrResults = resultIds.size();
    connection->write(&nrResults);
    for (set<int>::iterator it = resultIds.begin(); it != resultIds.end();
         ++it) {
      int tempRes = *it;
      connection->write(&tempRes);
    }
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int execCommand(KeyValueStore* instance, IPCConnection* connection) {
  string command;

  if (connection) {
    connection->read(&command);

    bool result = instance->execCommand(command);
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int initDistribute(KeyValueStore* instance, IPCConnection* connection) {
  int distributionId;
  string streamType, baseAttributeList, targetRelation, insertCommand,
      deleteCommand;
  bool restructure;

  if (connection) {
    connection->read(&distributionId);
    connection->read(&streamType);
    connection->read(&baseAttributeList);
    connection->read(&targetRelation);
    connection->read(&insertCommand);
    connection->read(&deleteCommand);
    connection->read(&restructure);

    cout << "Creating Task with current db:" << instance->currentDatabaseName
         << endl;
    DistributionTask* task = new DistributionTask(
        instance, connection,
        DistributionParameter(distributionId, streamType, baseAttributeList,
                              targetRelation, insertCommand, deleteCommand));

    instance->dtm.noRestructure = !restructure;
    bool result = instance->dtm.addTask(task);
    if (result) {
      instance->startDistributionThread();
    }

    connection->write(IPC_MSG_RESULT);
    connection->write(&result);

    if (!result) {
      delete task;
    }

    return KeyValueStoreIPCServer::REMOVECONNECTION;
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int initNetworkStream(KeyValueStore* instance, IPCConnection* connection) {
  int streamId;

  if (connection) {
    // KOUT<<"initNetworkStream IN"<<endl;
    connection->read(&streamId);

    NetworkStream* stream = instance->nsb.getNetworkStream(streamId);

    bool result = (stream != 0);
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
    // KOUT<<"initNetworkStream OUT"<<endl;
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int getNetworkStreamType(KeyValueStore* instance, IPCConnection* connection) {
  int streamId;

  if (connection) {
    // KOUT<<"getNetworkStreamType IN"<<endl;
    connection->read(&streamId);

    NetworkStream* stream = instance->nsb.getNetworkStream(streamId);

    string result = stream->getStreamType();
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
    // KOUT<<"getNetworkStreamType OUT"<<endl;
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int requestNetworkStream(KeyValueStore* instance, IPCConnection* connection) {
  int streamId;

  if (connection) {
    // KOUT<<"requestNetworkStream IN"<<endl;
    connection->read(&streamId);

    NetworkStream* stream = instance->nsb.getNetworkStream(streamId);

    bool result = (stream != 0);
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);

    // KOUT<<"requestNetworkStream OUT"<<endl;
    if (result) {
      // boost::thread serveDataThread(&NetworkStream::serveIPC, stream,
      // connection); //don't think branching out is a good idea

      stream->serveIPC(connection);
    }
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

int removeNetworkStream(KeyValueStore* instance, IPCConnection* connection) {
  int streamId;

  if (connection) {
    // cout<<"removeNetworkStream IN"<<endl;
    connection->read(&streamId);

    bool result = instance->nsb.removeStream(streamId);
    connection->write(IPC_MSG_RESULT);
    connection->write(&result);
    // cout<<"removeNetworkStream OUT"<<endl;
  } else {
    KOUT << "Error: Not connected." << endl;
  }

  return KeyValueStoreIPCServer::NORESULT;
}

void buildDispatchMap(
    map<IPCMessage, function<int(KeyValueStore*, IPCConnection*)> >&
        dispatchMap) {
  dispatchMap.insert(make_pair(IPC_MSG_CLOSECONNECTION, closeConnection));

  dispatchMap.insert(make_pair(IPC_MSG_ADDCONNECTION, addConnection));
  dispatchMap.insert(make_pair(IPC_MSG_REMOVECONNECTION, removeConnection));
  dispatchMap.insert(make_pair(IPC_MSG_RETRYCONNECTION, retryConnection));
  dispatchMap.insert(make_pair(IPC_MSG_SYNCSERVERLIST, syncServerList));
  dispatchMap.insert(make_pair(IPC_MSG_UPDATESERVERLIST, updateServerList));
  dispatchMap.insert(
      make_pair(IPC_MSG_SERVERINFORMATIONSTRING, serverInformationString));
  dispatchMap.insert(make_pair(IPC_MSG_SETDATABASE, setDatabase));
  dispatchMap.insert(make_pair(IPC_MSG_USEDATABASE, useDatabase));
  dispatchMap.insert(make_pair(IPC_MSG_TRANSFERID, transferId));
  dispatchMap.insert(make_pair(IPC_MSG_GLOBALTUPELID, globalTupelId));
  dispatchMap.insert(make_pair(IPC_MSG_INITCLIENTS, initClients));
  dispatchMap.insert(make_pair(IPC_MSG_STARTCLIENT, startClient));
  dispatchMap.insert(make_pair(IPC_MSG_STOPCLIENT, stopClient));
  dispatchMap.insert(make_pair(IPC_MSG_SETID, setId));
  dispatchMap.insert(make_pair(IPC_MSG_SETMASTER, setMaster));

  dispatchMap.insert(make_pair(IPC_MSG_TRYRESTRUCTURELOCK, tryRestructureLock));
  dispatchMap.insert(
      make_pair(IPC_MSG_UPDATERESTRUCTURELOCK, updateRestructureLock));
  dispatchMap.insert(
      make_pair(IPC_MSG_UNLOCKRESTRUCTURELOCK, unlockRestructureLock));

  dispatchMap.insert(make_pair(IPC_MSG_DISTRIBUTIONREF, distributionRef));
  dispatchMap.insert(make_pair(IPC_MSG_DISTRIBUTIONREFSET, distributionRefSet));
  dispatchMap.insert(
      make_pair(IPC_MSG_ADDDISTRIBUTIONELEM, addDistributionElement));
  dispatchMap.insert(
      make_pair(IPC_MSG_ADDDISTRIBUTIONRECT, addDistributionRect));
  dispatchMap.insert(make_pair(IPC_MSG_ADDDISTRIBUTIONINT, addDistributionInt));
  dispatchMap.insert(make_pair(IPC_MSG_FILTERDISTRIBUTION, filterDistribution));
  dispatchMap.insert(make_pair(IPC_MSG_DISTRIBUTIONDATA, distributionData));

  dispatchMap.insert(make_pair(IPC_MSG_QTDISTINCT, qtDistinct));

  dispatchMap.insert(make_pair(IPC_MSG_EXECCOMMAND, execCommand));

  dispatchMap.insert(make_pair(IPC_MSG_INITDISTRIBUTE, initDistribute));
  dispatchMap.insert(make_pair(IPC_MSG_INITNETWORKSTREAM, initNetworkStream));
  dispatchMap.insert(
      make_pair(IPC_MSG_REQUESTSTREAMTYPE, getNetworkStreamType));
  dispatchMap.insert(make_pair(IPC_MSG_REQUESTSTREAM, requestNetworkStream));
  dispatchMap.insert(make_pair(IPC_MSG_REMOVESTREAM, removeNetworkStream));
}
}
#endif /* KEYVALUESTOREDISPATCH_H_ */
