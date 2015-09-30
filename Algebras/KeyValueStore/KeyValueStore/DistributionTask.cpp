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
#include "DistributionTask.h"
#include "IPCMessages.h"
#include "DistributionCriteria.h"
#include "QuadTreeDistribution.h"
#include "KeyValueStore.h"

namespace KVS {

DistributionTask::DistributionTask(KeyValueStore* instance,
                                   IPCConnection* localConn,
                                   const DistributionParameter& dp)
    : distParams(dp),
      instance(instance),
      conn(localConn),
      criteria(0),
      resultIndex(0),
      provideResult(false),
      continueCurrentBatch(false),
      startTime(DebugTime()),
      tupleCounter(0),
      dataCounter(0) {
  currentBatch = new Batch(instance, &distParams, true);
}

DistributionTask::~DistributionTask() {
  for (unsigned int resultIdx = 0; resultIdx < resultStream.size();
       ++resultIdx) {
    delete[] resultStream[resultIdx].first;
  }

  KOUT << "\n\n\nStart:" << startTime << endl;
  KOUT << "Ende:" << DebugTime() << endl;
  KOUT << "Relation:" << distParams.targetRelation << endl;
  KOUT << "Tupel:" << tupleCounter << endl;
  map<int, int>::iterator item;
  for (item = detailedCounter.begin(); item != detailedCounter.end(); item++) {
    KOUT << item->first << ": " << item->second << endl;
  }
  KOUT << "Bytes:" << dataCounter << " MB:" << setprecision(10)
       << dataCounter / (1024.0 * 1024.0) << endl;
}

void DistributionTask::setDistributionCriteria(DistributionCriteria* criteria) {
  this->criteria = criteria;
}

void DistributionTask::setContinueCurrentBatch(bool value) {
  continueCurrentBatch = value;
}

int DistributionTask::process(int n) {
  IPCMessage msgType;
  int serverId;
  unsigned int tupleLen;

  bool prepareResult = false;
  int resultCode = 0;  // 0 all good, 1 error
  string resultMessage("Distribution successfully completed");

  int processedTuples = 0;
  for (int i = 0; i < n; ++i) {
    if (conn->avail()) {
      conn->read(&msgType);
      if (msgType == IPC_MSG_SENDTUPLE) {
        processedTuples++;
        conn->read(&serverId);
        tupleLen = 0;
        conn->read(&tupleLen);

        char* tupleBuffer = new char[tupleLen];
        conn->read(tupleBuffer, tupleLen);

        if (currentBatch->transferList[serverId] == 0) {
          resultStream.push_back(make_pair(tupleBuffer, tupleLen));
        } else {
          if (currentBatch->transferList[serverId]->sendTuple(tupleBuffer,
                                                              tupleLen)) {
            tupleCounter++;
            detailedCounter[serverId]++;
            dataCounter += tupleLen;
            currentBatch->tupleCount++;

            criteria->addTuple(serverId);
            criteria->addBytesTransferred(serverId, tupleLen);
          } else {
            resultCode = 1;
            resultMessage.assign("Could not send tuple.");
            prepareResult = true;
          }
        }

      } else if (msgType == IPC_MSG_ENDDISTRIBUTE) {
        KOUT << "Ending Distribution" << endl;
        prepareResult = true;
      } else if (msgType == IPC_MSG_CLOSEDISTRIBUTE) {
        KOUT << "Closing Distribution" << endl;
        return 1;
      } else {
        // unkown message? quit?
        resultCode = 1;
        resultMessage.assign("Received unknown IPC Message.");
        prepareResult = true;
      }
    } else if (provideResult) {
      if (resultIndex < resultStream.size()) {
        conn->write(&resultStream[resultIndex].second);
        conn->write(resultStream[resultIndex].first,
                    resultStream[resultIndex].second);
        resultIndex++;
      } else {
        int close = 0;
        conn->write(&close);
        KOUT << "Close written..." << endl;
        provideResult = false;
      }
    } else {
      break;
    }
  }

  if (currentBatch->batchReady() || prepareResult) {
    KOUT << "Finishing Batch" << endl;

    if (!currentBatch->finishBatch()) {
      resultCode = 1;
      resultMessage.assign("Error while finishing batch.");
      prepareResult = true;
    } else {
      KOUT << "Batch finished..." << endl;
      delete currentBatch;
      currentBatch = new Batch(instance, &distParams);
    }
  }

  if (prepareResult) {
    collectResult(currentBatch);
    provideResult = true;

    conn->write(IPC_MSG_RESULT);
    conn->write(&resultCode);
    conn->write(&resultMessage);
  }

  return 0;
}

bool DistributionTask::finishBatch() {
  bool result = true;

  if (!currentBatch->finishBatch()) {
    // TODO:consequence ?
    result = false;
  }

  collectResult(currentBatch);

  delete currentBatch;
  currentBatch = new Batch(instance, &distParams);

  return result;
}

void DistributionTask::collectResult(Batch* batch) {
  for (unsigned int transferIdx = 0; transferIdx < batch->transferList.size();
       ++transferIdx) {
    if (batch->transferList[transferIdx] != 0) {
      resultStream.insert(resultStream.end(),
                          batch->transferList[transferIdx]->unconfirmed.begin(),
                          batch->transferList[transferIdx]->unconfirmed.end());
      batch->transferList[transferIdx]->unconfirmed.clear();
    }
  }
}
}
