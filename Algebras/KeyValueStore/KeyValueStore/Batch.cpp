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

#include "Batch.h"

#include "KeyValueStore.h"
#include "DistributionTask.h"

namespace KVS {

Batch::Batch(KeyValueStore* instance, DistributionParameter* distParams,
             bool firstBatch)
    : forceBatch(false),
      tupleCount(0),
      instance(instance),
      sm(&instance->sm),
      distParams(distParams),
      useSCP(false),
      firstBatch(firstBatch) {
  dist = instance->getDistribution(distParams->distributionId);
  initBatch();
}

bool Batch::initBatch() {
  updateTransferList();

  // transferIdx == serverId
  for (unsigned int transferIdx = 0; transferIdx < transferList.size();
       ++transferIdx) {
    if (transferList[transferIdx] != 0) {
      int serverIdx = sm->getConnectionIndex(transferIdx);

      if (serverIdx >= 0) {
        if (!transferList[transferIdx]->init(
                sm->getConnectionIdx(serverIdx)
                    ->kvsConn->requestTransferId())) {  // maybe use threads to
                                                        // speed this up
          return false;
        }
      }
    }
  }

  time(&batchStartTime);
  tupleCount = 0;

  return true;
}

bool Batch::finishBatch() {
  bool result = true;
  // transferIdx == serverId
  for (unsigned int transferIdx = 0; transferIdx < transferList.size();
       transferIdx++) {
    if (transferList[transferIdx] != 0) {
      result = result & finishTransfer(transferList[transferIdx], transferIdx);
      transferList[transferIdx]->cleanUp();
    }
  }

  if (result) {
    initBatch();
  }

  forceBatch = false;
  return result;
}

bool Batch::batchReady() {
  if (batchStartTime == 0) {
    time(&batchStartTime);
  }

  return (tupleCount > 0 &&
          (tupleCount > MAX_BATCH_COUNT ||
           difftime(time(NULL), batchStartTime) > MAX_BATCH_TIME ||
           forceBatch));
}

void Batch::updateTransferList() {
  // id - connection
  vector<Connection*>& serverList = sm->getConnectionList();

  // unsigned int serverId;
  int serverIdx;

  vector<int>::iterator maxId =
      max_element(dist->serverIdOrder.begin(), dist->serverIdOrder.end());
  if (*maxId >= static_cast<int>(transferList.size())) {
    transferList.resize(*maxId + 1, 0);
  }

  for (unsigned int transferIdx = 0; transferIdx < transferList.size();
       ++transferIdx) {
    vector<int>::iterator mappingPos = find(
        dist->serverIdOrder.begin(), dist->serverIdOrder.end(), transferIdx);
    if (mappingPos != dist->serverIdOrder.end()) {
      if (static_cast<int>(transferIdx) != instance->id) {
        // transferIdx == serverId
        serverIdx = sm->getConnectionIndex(transferIdx);

        // TODO: rework this assignment
        if (serverIdx < 0) {
          serverIdx = sm->getAvailableServer();
          if (serverIdx >= 0) {
            serverList[serverIdx]->id = transferIdx;
          }
        }

        if (serverIdx >= 0) {
          if (!serverList[serverIdx]->check() ||
              !serverList[serverIdx]->kvsConn->check()) {
            KOUT << instance->localHost << " : " << instance->localInterfacePort
                 << " : " << instance->localKvsPort << " : "
                 << instance->currentDatabaseName << endl;
            if (!serverList[serverIdx]->initInterface(
                    instance->localHost, instance->localInterfacePort,
                    instance->localKvsPort, instance->currentDatabaseName)) {
              KOUT << "Error: Server could not be initialized.";
            }
          }

          if (useSCP) {
            // scp
            transferList[transferIdx] = new TransferMethodSCP(
                serverList[serverIdx], distParams->streamType);
          } else {
            // tcp
            transferList[transferIdx] = new TransferMethodTCP(
                serverList[serverIdx], distParams->streamType,
                distParams->baseAttributeList, true);
          }
        }
      }
    } else {
      if (transferList[transferIdx] != 0) {
        delete transferList[transferIdx];
        transferList[transferIdx] = 0;
      }
    }
  }
}

bool Batch::finishTransfer(TransferMethod* transfer, int serverId) {
  if (!transfer->endStream() ||
      !transfer->import(distParams->targetRelation,
                        distParams->insertCommand)) {
    // recovery (doesnt work yet... )
    KOUT << "Recovery :(" << endl;
    if (transfer->connection->id != serverId) {
      // has serverId assignment changed?
      Connection* newConn =
          sm->getConnectionIdx(sm->getConnectionIndex(serverId));
      transfer->changeConnection(newConn);
      if (newConn && newConn->check()) {
        transfer->resendUnconfirmed(newConn->kvsConn->requestTransferId());
        return finishTransfer(transfer, serverId);
      }
    }

    if (!transfer->connection->check()) {
      // interface down
      // move to available or neighbor or false
      return recoveryInterface(transfer, serverId);
    } else if (!transfer->connection->kvsConn->check()) {
      // kvs connection down
      if (transfer->connection->exec(
              "query kvsStartClient(" +
              stringutils::int2str(transfer->connection->kvsPort) + ");") &&
          transfer->connection->kvsConn->check()) {
        transfer->resendUnconfirmed(
            transfer->connection->kvsConn->requestTransferId());
        return finishTransfer(transfer, serverId);
      } else {
        // try to restart interface
        return recoveryInterface(transfer, serverId);
      }
      // TODO: check if this makes sence for SCP
      /*} else if(!transfer->dataMismatch()) {
        return true;*/
    } else {
      // unknown move to available or neighbor or false
      return recoveryInterface(transfer, serverId);
    }
  } else {
    KOUT << "Sucessfully transferred " << transfer->tupleCounter
         << " tuples to server " << transfer->connection->id << endl;
    return true;
  }
}

bool Batch::recoveryInterface(TransferMethod* transfer, int serverId) {
  transfer->connection->status = Connection::Problem;
  transfer->connection->id = -1;

  int availableIdx = sm->getAvailableServer();

  if (availableIdx > -1) {
    // try again on different server
    Connection* newConn = sm->getConnectionIdx(availableIdx);

    if (newConn->id != -1) {
      // server already assigned
      return false;
    } else {
      newConn->id = serverId;
      transfer->changeConnection(newConn);

      if (transfer->resendUnconfirmed(newConn->kvsConn->requestTransferId())) {
        // TODO:init backup
        return finishTransfer(transfer, serverId);
      } else {
        return false;
      }
    }
  } else {
    // move to neighbor
    int neighborId = dist->neighbourId(serverId);
    if (neighborId > -1) {
      if (transferList[neighborId] != 0) {
        dist->changeServerId(serverId, neighborId);

        if (neighborId > serverId) {
          // will be processed afterwards
          for (unsigned unconfirmedIdx = 0;
               unconfirmedIdx < transfer->unconfirmed.size();
               ++unconfirmedIdx) {
            transferList[neighborId]->sendTuple(
                transfer->unconfirmed[unconfirmedIdx].first,
                transfer->unconfirmed[unconfirmedIdx].second);
          }
          transfer->unconfirmed.clear();
          // should get deleted automatically

          return true;
        } else {
          int neighborServerIdx = sm->getConnectionIndex(neighborId);

          transfer->changeConnection(sm->getConnectionIdx(neighborServerIdx));
          if (transfer->resendUnconfirmed(
                  transfer->connection->kvsConn->requestTransferId())) {
            return finishTransfer(transfer, serverId);
          } else {
            return false;
          }
        }
      } else {
        return false;
      }
    } else {
      return false;
    }
  }
}
}
