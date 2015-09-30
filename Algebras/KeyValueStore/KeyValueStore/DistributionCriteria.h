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

#ifndef RESTRICTURECRITERIA_H_
#define RESTRICTURECRITERIA_H_

#include <vector>
#include <ctime>

using namespace std;

namespace KVS {

class KeyValueStore;
class Distribution;

//
// Checks if redistributions are needed and decides
// which servers will be involved and if split or redistribute occurs
//
// Used in DistributionTaskManager
//
// Contains lots of stuff thats currently not used
//
class DistributionCriteria {
 public:
  DistributionCriteria(KeyValueStore* instance);
  void evaluateCriteria(Distribution* dist, int availableServers);
  void addTuple(unsigned int serverId);
  void addBytesTransferred(unsigned int serverId, int n);

  vector<pair<int, int> > localRestructure;  // startIdx, n
  vector<unsigned int> split;

  void reset();

  unsigned int distributedBytes();

 private:
  void checkTupleDistribution(Distribution* dist, const double& average);
  void checkTupleCapacity(Distribution* dist);

  void evaluate(Distribution* dist, const double& average,
                int availableServers);

  void checkTupleInsertPercentage(Distribution* dist);
  void checkMaxData(Distribution* dist);
  void checkDataDistribution(Distribution* dist);

  time_t insertIntervalStart;        // to calculate insert frequency
  double insertIntervalLength;       // in seconds
  double insertPercentageThreshold;  // expected + threshold

  int insertsOverall;
  vector<unsigned int> insertDistribution;

  unsigned int maxData;  // universal for all servers in Bytes

  unsigned int dataOverall;
  vector<unsigned int> dataDistribution;

  vector<bool> restructurePotential;

  KeyValueStore* instance;
};
}

#endif /* RESTRICTURECRITERIA_H_ */
