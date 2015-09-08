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
#include "KeyValueStoreDebug.h"
#include "DistributionCriteria.h"
#include "Distribution.h"

#include <iomanip>

namespace KVS {

DistributionCriteria::DistributionCriteria(unsigned int minTuplePerServer)
    : insertIntervalStart(0),
      minSize(minTuplePerServer),
      insertsOverall(0),
      dataOverall(0) {
  insertIntervalLength = 60;        // in seconds
  insertPercentageThreshold = 0.1;  // expected + threshold

  maxData = 1 * 1024 * 1024;  // ~xMB
}

void DistributionCriteria::evaluateCriteria(Distribution* dist,
                                            int availableServers) {
  ROUT << DebugTime() << "Evaluating Restructure (Transferred " << dataOverall
       << " bytes / " << setprecision(10) << dataOverall / (1024.0 * 1024.0)
       << " MB)" << endl;

  dist->updateWeightVector();

  // init interval
  if (insertIntervalStart == 0) {
    time(&insertIntervalStart);
  }

  // calc average
  double average = 0;
  for (unsigned int serverIdIdx = 0; serverIdIdx < dist->serverIdMapping.size();
       ++serverIdIdx) {
    average += dist->serverWeight[serverIdIdx];
  }
  average = average / (double)dist->serverIdMapping.size();

  // init restructurePotential
  restructurePotential.assign(restructurePotential.size(), false);
  restructurePotential.resize(insertDistribution.size(), false);

  // Tuple Distribution
  // checkTupleInsertPercentage(dist);
  checkTupleDistribution(dist, average);

  // Data Distribution
  checkMaxData(dist);
  checkDataDistribution(dist);

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
  double expectedPercentage = 1.0f / (double)dist->serverIdMapping.size();

  time_t now = time(NULL);
  if (difftime(now, insertIntervalStart) > insertIntervalLength) {
    ROUT << "Checking insert percentage..." << endl;

    for (unsigned int serverIdIdx = 0; serverIdIdx < insertDistribution.size();
         ++serverIdIdx) {
      // more inserts than expected + threshold
      if ((double)insertDistribution[serverIdIdx] / (double)insertsOverall >
              (expectedPercentage + insertPercentageThreshold) &&
          dist->serverWeight[serverIdIdx] > minSize) {
        restructurePotential[serverIdIdx] = true;
      }
    }

    insertsOverall = 0;
    insertDistribution.assign(insertDistribution.size(), 0);

    // update
    time(&insertIntervalStart);
  }
}

void DistributionCriteria::checkTupleDistribution(Distribution* dist,
                                                  const double& average) {
  ROUT << "Checking if server capacity exceeds average by more than 50% ("
       << setprecision(10) << average << " tuples)" << endl;
  for (unsigned int serverIdIdx = 0; serverIdIdx < dist->serverWeight.size();
       ++serverIdIdx) {
    // check x% above average (50%)
    ROUT << "ID " << serverIdIdx << ": " << dist->serverWeight[serverIdIdx]
         << " tuples";
    if (dist->serverWeight[serverIdIdx] >= 1.5 * average &&
        dist->serverWeight[serverIdIdx] > minSize) {
      ROUT << " -> potential";
      restructurePotential[serverIdIdx] = true;
    }
    ROUT << endl;
  }
}

void DistributionCriteria::checkMaxData(Distribution* dist) {
  ROUT << "Checking if server exceeds max capacity (" << maxData << " bytes)"
       << endl;
  for (unsigned int serverIdIdx = 0; serverIdIdx < dataDistribution.size();
       ++serverIdIdx) {
    ROUT << "ID " << serverIdIdx << ": " << dataDistribution[serverIdIdx]
         << " bytes";
    if (dataDistribution[serverIdIdx] > maxData) {
      ROUT << " -> potential";
      restructurePotential[serverIdIdx] = true;
    }
    ROUT << endl;
  }
}

void DistributionCriteria::checkDataDistribution(Distribution* dist) {
  if (dist->serverIdMapping.size() > 0) {
    double average = dataOverall / (double)dist->serverIdMapping.size();

    ROUT << "Checking if server capacity exceeds average by more than 50% ("
         << setprecision(10) << average << " bytes)" << endl;
    for (unsigned int serverIdIdx = 0; serverIdIdx < dist->serverWeight.size();
         ++serverIdIdx) {
      // check x% above average (50%)
      if (dataDistribution[serverIdIdx] >= 1.5 * average) {
        ROUT << "ID " << serverIdIdx << ": " << dataDistribution[serverIdIdx]
             << " bytes -> potential" << endl;
        restructurePotential[serverIdIdx] = true;
      }
    }
  }
}

void DistributionCriteria::evaluate(Distribution* dist, const double& average,
                                    int availableServers) {
  ROUT << "Deciding Actions" << endl;
  for (unsigned int serverIdx = 0; serverIdx < restructurePotential.size();
       ++serverIdx) {
    if (restructurePotential[serverIdx] == true) {
      ROUT << "ID " << serverIdx << ":";

      // check if local restructure is sufficient
      unsigned int mappingIdx;
      for (mappingIdx = 0; mappingIdx < dist->serverIdMapping.size();
           ++mappingIdx) {
        if (dist->serverIdMapping[mappingIdx] == static_cast<int>(serverIdx)) {
          break;
        }
      }

      double diff = 0;
      bool before = false, after = false;

      // left neighbor(before)?
      if (mappingIdx > 0) {
        if (dist->serverWeight[dist->serverIdMapping[mappingIdx - 1]] <
                average &&
            !restructurePotential[dist->serverIdMapping[mappingIdx - 1]]) {
          before = true;
          diff += (average -
                   dist->serverWeight[dist->serverIdMapping[mappingIdx - 1]]);
        }
      }

      // right neighbor(after)?
      if (mappingIdx < (dist->serverIdMapping.size() - 1)) {
        before = mappingIdx + 1;
        if (dist->serverWeight[dist->serverIdMapping[mappingIdx + 1]] <
            average) {
          after = true;
          diff +=
              (average -
                   dist->serverWeight[dist->serverIdMapping[mappingIdx + 1]] &&
               !restructurePotential[dist->serverIdMapping[mappingIdx + 1]]);
        }
      }

      // above average on
      ROUT << "Netto Problem:" << (dist->serverWeight[serverIdx] - average)
           << " Neighbor capacity:" << diff << " in No. Tuples";
      if ((dist->serverWeight[serverIdx] - average) * 0.9 < diff &&
          (before || after)) {
        ROUT << " -> local restructure" << endl;
        //=> local restructure
        int startIdx;
        int n = 1;

        if (before) {
          startIdx = dist->serverIdMapping[mappingIdx - 1];
          n++;
        } else {
          startIdx = serverIdx;
        }

        if (after) {
          n++;
        }

        localRestructure.push_back(make_pair(startIdx, n));
      } else {
        //=> split
        ROUT << " -> split" << endl;
        // if(split.size()+dist->serverIdMapping.size() < availableServers) {
        // insert sorted by server weight
        if (split.size() == 0) {
          split.push_back(serverIdx);
        } else {
          for (int insertPos = split.size() - 1; insertPos >= 0; insertPos--) {
            if (dist->serverWeight[split[insertPos]] >=
                dist->serverWeight[serverIdx]) {
              split.insert(split.begin() + insertPos + 1, serverIdx);
              break;
            } else if (insertPos == 0) {
              split.insert(split.begin(), serverIdx);
            }
          }
        }
        //}
      }
    }
  }
}
}
