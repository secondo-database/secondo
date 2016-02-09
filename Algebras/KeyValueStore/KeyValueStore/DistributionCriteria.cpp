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
#include "KeyValueStoreDebug.h"
#include "DistributionCriteria.h"
#include "Distribution.h"

#include <iomanip>

namespace KVS {

DistributionCriteria::DistributionCriteria(KeyValueStore* instance)
    : insertIntervalStart(0),
      insertsOverall(0),
      dataOverall(0),
      instance(instance) {
  insertIntervalLength = 60;        // in seconds
  insertPercentageThreshold = 0.1;  // expected + threshold

  maxData = 1 * 1024 * 1024;  // ~xMB
}

void DistributionCriteria::evaluateCriteria(Distribution* dist,
                                            int availableServers) {
  ROUT << "\n\n\n" << DebugTime() << "Evaluating Restructure (Transferred "
       << dataOverall << " bytes / " << setprecision(10)
       << dataOverall / (1024.0 * 1024.0) << " MB)" << endl;

  dist->updateWeightVector();

  // init interval
  if (insertIntervalStart == 0) {
    time(&insertIntervalStart);
  }

  // calc average
  double average = 0;
  for (unsigned int serverIdIdx = 0; serverIdIdx < dist->serverIdOrder.size();
       ++serverIdIdx) {
    average += dist->serverWeight[dist->serverIdOrder[serverIdIdx]];
  }
  average = average / (double)dist->serverIdOrder.size();

  // init restructurePotential
  restructurePotential.assign(restructurePotential.size(), false);
  restructurePotential.resize(insertDistribution.size(), false);

  // Tuple Distribution
  // checkTupleInsertPercentage(dist);
  checkTupleDistribution(dist, average);
  checkTupleCapacity(dist);

  // Data Distribution
  // checkMaxData(dist);
  // checkDataDistribution(dist);

  // prepare result lists
  localRestructure.clear();
  split.clear();

  //...
  evaluate(dist, average, availableServers);
}

void DistributionCriteria::reset() {
  dataDistribution.assign(dataDistribution.size(), 0);
}

unsigned int DistributionCriteria::distributedBytes() { return dataOverall; }

void DistributionCriteria::addTuple(unsigned int serverId) {
  if (serverId >= 0) {
    if (serverId >= insertDistribution.size()) {
      insertDistribution.resize(serverId + 1, 0);
    }

    insertDistribution[serverId]++;
    insertsOverall++;
  }
}

void DistributionCriteria::addBytesTransferred(unsigned int serverId, int n) {
  if (serverId >= 0) {
    if (serverId >= dataDistribution.size()) {
      dataDistribution.resize(serverId + 1, 0);
    }

    dataDistribution[serverId] += n;
    dataOverall += n;
  }
}

void DistributionCriteria::checkTupleInsertPercentage(Distribution* dist) {
  // expecting equally distributed workload 1/nrACTIVEservers
  //  double expectedPercentage = 1.0f / (double)dist->serverIdOrder.size();
  //
  //  time_t now = time(NULL);
  //  if(difftime(now, insertIntervalStart) > insertIntervalLength) {
  //    ROUT<<"Checking insert percentage..."<<endl;
  //
  //    for(unsigned int serverId = 0; serverId < insertDistribution.size();
  //    ++serverId) {
  //      //more inserts than expected + threshold
  //      if((double)insertDistribution[serverId]/(double)insertsOverall >
  //      (expectedPercentage + insertPercentageThreshold) &&
  //      dist->serverWeight[serverId] > minSize) {
  //        restructurePotential[serverId] = true;
  //      }
  //    }
  //
  //    insertsOverall = 0;
  //    insertDistribution.assign(insertDistribution.size(),0);
  //
  //    //update
  //    time(&insertIntervalStart);
  //  }
}

void DistributionCriteria::checkTupleDistribution(Distribution* dist,
                                                  const double& average) {
  ROUT << "Checking if server capacity exceeds average by more than 30% ("
       << setprecision(10) << average * 1.3 << " tuples)" << endl;
  for (unsigned int serverIdIdx = 0; serverIdIdx < dist->serverIdOrder.size();
       ++serverIdIdx) {
    int serverId = dist->serverIdOrder[serverIdIdx];
    // check x% above average (30%)
    ROUT << "ID " << serverIdIdx << ": " << dist->serverWeight[serverId]
         << " tuples";
    if (dist->serverWeight[serverId] >= 1.3 * average) {
      ROUT << " -> potential";
      restructurePotential[serverId] = true;
    }
    ROUT << endl;
  }
}

void DistributionCriteria::checkTupleCapacity(Distribution* dist) {
  ROUT << "Checking if server capacity is exceeded" << endl;
  for (unsigned int serverIdIdx = 0; serverIdIdx < dist->serverIdOrder.size();
       ++serverIdIdx) {
    int serverId = dist->serverIdOrder[serverIdIdx];
    int serverIdx = instance->sm.getConnectionIndex(serverId);

    unsigned int capacity =
        instance->sm.getConnectionIdx(serverIdx)->tupleCapacity;

    ROUT << "ID " << serverId << ": " << dist->serverWeight[serverId] << " of "
         << capacity << " tuples";
    if (dist->serverWeight[serverId] >= static_cast<int>(capacity)) {
      ROUT << " -> potential";
      restructurePotential[serverId] = true;
    }
    ROUT << endl;
  }
}

void DistributionCriteria::checkMaxData(Distribution* dist) {
  //  double average = dataOverall / (double)dist->serverIdOrder.size();
  //
  //
  //    ROUT<<"Checking if server exceeds max capacity ("<<maxData<<"
  //    bytes)"<<endl;
  //    for(unsigned int serverId=0; serverId < dataDistribution.size();
  //    ++serverId) {
  //      ROUT<<"ID "<<serverId<<": "<<dataDistribution[serverId]<<" bytes";
  //      if(dataDistribution[serverId] > maxData) {
  //        ROUT<<" -> potential";
  //        restructurePotential[serverId] = true;
  //      }
  //      ROUT<<endl;
  //    }
}

void DistributionCriteria::checkDataDistribution(Distribution* dist) {
  //  if(dist->serverIdOrder.size() > 0) {
  //    double average = dataOverall / (double)dist->serverIdOrder.size();
  //
  //    ROUT<<"Checking if server capacity exceeds average by more than 50%
  //    ("<<setprecision(10)<<average<<" bytes)"<<endl;
  //    for(unsigned int serverIdIdx=0; serverIdIdx <
  //    dist->serverIdOrder.size(); ++serverIdIdx) {
  //      int serverId = dist->serverIdOrder[serverIdIdx];
  //      //check x% above average (50%)
  //      if(dataDistribution[serverId] >= 1.5*average) {
  //        ROUT<<"ID "<<serverId<<": "<<dataDistribution[serverId]<<" bytes ->
  //        potential"<<endl;
  //        restructurePotential[serverId] = true;
  //      }
  //    }
  //  }
}

void DistributionCriteria::evaluate(Distribution* dist, const double& average,
                                    int availableServers) {
  ROUT << "\nDeciding Actions" << endl;
  for (unsigned int serverId = 0; serverId < restructurePotential.size();
       ++serverId) {
    if (restructurePotential[serverId] == true) {
      ROUT << "ID " << serverId << ":";

      // check if local restructure is sufficient
      std::vector<int>::iterator idPos = find(
          dist->serverIdOrder.begin(), dist->serverIdOrder.end(), serverId);

      if (idPos == dist->serverIdOrder.end()) {
        continue;
      }

      double diff = 0;
      bool before = false, after = false;

      int leftNeighborId = -1;

      // left neighbor(before)?
      if (idPos != dist->serverIdOrder.begin()) {
        leftNeighborId = *(idPos - 1);
        ROUT << "Neighbor before weight:" << dist->serverWeight[leftNeighborId]
             << endl;
        if (dist->serverWeight[leftNeighborId] < average) {
          before = true;
          diff += average - dist->serverWeight[leftNeighborId];
          ROUT << "before diff:" << diff << endl;
        }
      }

      int rightNeighborId = -1;
      // right neighbor(after)?
      if (idPos + 1 != dist->serverIdOrder.end()) {
        rightNeighborId = *(idPos + 1);
        ROUT << "Neighbor after weight:" << dist->serverWeight[rightNeighborId]
             << endl;
        // before = mappingIdx+1;
        if (dist->serverWeight[rightNeighborId] < average) {
          after = true;
          diff += average - dist->serverWeight[rightNeighborId];
          ROUT << "after diff:" << diff << endl;
        }
      }

      // above average on
      ROUT << "Netto Problem:" << (dist->serverWeight[serverId] - average)
           << " Neighbor capacity:" << diff << " in No. Tuples";
      if ((dist->serverWeight[serverId] - average) * 0.9 < diff &&
          (before || after)) {
        ROUT << " -> local restructure" << endl;
        //=> local restructure
        int startId;
        int n = 1;

        if (before) {
          startId = leftNeighborId;
          n++;
        } else {
          startId = serverId;
        }

        if (after) {
          n++;
        }

        localRestructure.push_back(make_pair(startId, n));
      } else {
        //=> split
        ROUT << " -> split" << endl;
        // if(split.size()+dist->serverIdOrder.size() < availableServers) {
        // insert sorted by server weight
        if (split.size() == 0) {
          split.push_back(serverId);
        } else {
          for (int insertPos = split.size() - 1; insertPos >= 0; insertPos--) {
            if (dist->serverWeight[split[insertPos]] >=
                dist->serverWeight[serverId]) {
              split.insert(split.begin() + insertPos + 1, serverId);
              break;
            } else if (insertPos == 0) {
              split.insert(split.begin(), serverId);
            }
          }
        }
        //}
      }
    }
  }
}
}
