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

#ifndef BATCH_H_
#define BATCH_H_

#include "TransferMethod.h"
#include "Distribution.h"

#include <ctime>
#include <string>
#include <vector>

using namespace std;

namespace KVS {

class KeyValueStore;
class ServerManager;
class QuadTreeDistribution;
class DistributionParameter;

//
// Represents a part of the tuple stream created by kvsDistribute
//
class Batch {
 public:
  Batch(KeyValueStore* instance, DistributionParameter* distParams,
        bool firstBatch = false);
  ~Batch();

  bool initBatch();
  bool finishBatch();

  bool batchReady();

  bool forceBatch;

  vector<TransferMethod*> transferList;
  int tupleCount;

 private:
  void updateTransferList();

  bool finishTransfer(TransferMethod* transfer, int serverId);
  bool recoveryInterface(TransferMethod* transfer, int serverId);

  KeyValueStore* instance;
  ServerManager* sm;
  Distribution* dist;

  DistributionParameter* distParams;

  time_t batchStartTime;              // to calculate insert frequency
  const double MAX_BATCH_TIME = 180;  // in seconds
  const int MAX_BATCH_COUNT = 1000;

  bool useSCP;
  bool firstBatch;
};
}

#endif /* BATCH_H_ */
