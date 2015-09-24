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

#ifndef DISTRIBUTIONTASKMANAGER_H_
#define DISTRIBUTIONTASKMANAGER_H_

#include "DistributionCriteria.h"
#include "DistributionTask.h"
#include "Distribution.h"

#include "boost/thread.hpp"
#include "boost/date_time.hpp"

namespace KVS {

class KeyValueStore;

class DistributionTaskManager : public DistributionUpdateListener {
 public:
  DistributionTaskManager(KeyValueStore* instance);
  ~DistributionTaskManager();

  void exec(bool* run);

  void processDistributions(bool* run, unsigned int n);

  bool addTask(DistributionTask* task);
  bool removeTask(unsigned int index);

  void distributionUpdated();

  void syncDistributions(bool* run);
  void syncDistributionsThread(Connection* server,
                               vector<DistributionParameter> distParams,
                               Distribution* dist, string distName,
                               string distData);

  void syncDataState();
  void syncDataStateThread(Connection* server,
                           vector<DistributionParameter> distParams,
                           string distName, string distData);

  void restructure(bool* run);
  void restructurePhaseTwo(vector<Connection*> involvedServers);

  void restructureRedistribute(Connection* server,
                               vector<DistributionParameter> distParams);
  void restructureCleanData(Connection* server,
                            vector<DistributionParameter> distParams);

  bool noRestructure;
  boost::mutex exitMutex;

 private:
  void resetRedistribtionProgress();

  boost::thread* restructureProcess;

  KeyValueStore* instance;

  int currentDistributionId;
  bool syncDistribution;

  time_t lastRestructure;
  double RESTRUCTURE_INTERVAL;  // in seconds

  DistributionCriteria criteria;

  vector<DistributionTask*> tasks;
  vector<DistributionTask*> removedTasks;

  vector<DistributionTask*> newTasks;
  boost::mutex newTasksMutex;
};
}

#endif /* DISTRIBUTIONTASKMANAGER_H_ */
