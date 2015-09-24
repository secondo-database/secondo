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

#ifndef DISTRIBUTIONTASK_H_
#define DISTRIBUTIONTASK_H_

#include "TransferMethod.h"
#include "ipc.h"

#include "Batch.h"

namespace KVS {

class DistributionCriteria;
class ServerManager;
class QuadTreeDistribution;

class DistributionParameter {
 public:
  DistributionParameter(int distributionId, string streamType,
                        string baseAttributeList, string targetRelation,
                        string insertCommand, string deleteCommand)
      : distributionId(distributionId),
        streamType(streamType),
        baseAttributeList(baseAttributeList),
        targetRelation(targetRelation),
        insertCommand(insertCommand),
        deleteCommand(deleteCommand) {}
  DistributionParameter(const DistributionParameter& dp)
      : distributionId(dp.distributionId),
        streamType(dp.streamType),
        baseAttributeList(dp.baseAttributeList),
        targetRelation(dp.targetRelation),
        insertCommand(dp.insertCommand),
        deleteCommand(dp.deleteCommand) {}

  int distributionId;
  string streamType;
  string baseAttributeList;
  string targetRelation;

  string insertCommand;
  string deleteCommand;
};

class DistributionTask {
 public:
  DistributionTask(KeyValueStore* instance, IPCConnection* localConn,
                   const DistributionParameter& dp);
  ~DistributionTask();

  int process(int n);
  bool finishBatch();

  void setContinueCurrentBatch(bool value);

  // Tuple* nextResult();
  void setDistributionCriteria(DistributionCriteria* criteria);

  DistributionParameter distParams;

 private:
  Batch* currentBatch;
  vector<Batch*> finishedBatches;

  void restructure();
  void collectResult(Batch* batch);

  KeyValueStore* instance;
  IPCConnection* conn;

  DistributionCriteria* criteria;

  vector<pair<char*, int> > resultStream;
  unsigned int resultIndex;

  bool provideResult;

  bool continueCurrentBatch;

  string startTime;
  unsigned int tupleCounter;
  unsigned int dataCounter;
  map<int, int> detailedCounter;
};
}

#endif /* DISTRIBUTIONTASK_H_ */
