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

#ifndef DISTRIBUTION_H_
#define DISTRIBUTION_H_

#include "StandardTypes.h"

#include <string>
#include <vector>

#include "boost/thread.hpp"
#include "boost/date_time.hpp"

using namespace std;

namespace KVS {

//
// Keep clients up to date on state of distribution
//
class DistributionUpdateListener {
 public:
  virtual ~DistributionUpdateListener() {}
  virtual void distributionUpdated() = 0;
};

//
// Distribution base type to support different types of distributions
// currently only based on quadtree needs
//
class Distribution {
 public:
  enum DistributionType { TYPE_NONE, TYPE_QUADTREE, TYPE_CONSISTENT };

  Distribution(const int& type) : type(type), needsSync(false){};
  Distribution(const int& type, const vector<int>& serverIdOrder,
               const map<int, int>& serverWeight)
      : type(type),
        serverIdOrder(serverIdOrder),
        serverWeight(serverWeight),
        needsSync(false) {}

  virtual ~Distribution() {}

  static Distribution* getInstance(const string& data);

  static const bool checkType(const ListExpr type);

  void addUpdateListener(DistributionUpdateListener* listener);
  void removeUpdateListener(DistributionUpdateListener* listener);
  void distributionUpdated();
  bool needsSynchronisation();

  virtual string toBin() = 0;
  virtual bool fromBin(const string& data) = 0;

  virtual string serverIdAssignment(string attributeName,
                                    string distributionName,
                                    bool requestOnly) = 0;

  virtual int split(int serverId) = 0;
  virtual void redistribute(int serverId,
                            int n) = 0;  // n: number of servers after serverId
                                         // (referencing serverIdOrder order)

  virtual int neighbourId(int id) = 0;
  virtual void changeServerId(int oldid, int newid) = 0;

  virtual void resetWeight() = 0;
  virtual void updateWeightVector() = 0;
  virtual void addWeight(Distribution* dist, const int& id) = 0;

  virtual void resetMaxGlobalIds() = 0;
  virtual void addMaxGlobalIds(Distribution* dist, const int& id) = 0;

  virtual bool filter(int nrcoords, double* coords,
                      const unsigned int& globalId, bool update) = 0;

  virtual void add(int value, set<int>* resultIds) = 0;  // int
  virtual void add(int nrcoords, double* coords,
                   set<int>* resultIds) = 0;  // rect
  virtual void addDebug(int nrcoords, double* coords,
                        set<int>* resultIds) = 0;  // rect

  virtual void request(int value, set<int>* resultIds) = 0;  // int
  virtual void request(int nrcoords, double* coords,
                       set<int>* resultIds) = 0;  // rect
  virtual void requestDebug(int nrcoords, double* coords,
                            set<int>* resultIds) = 0;  // rect

  int type;

  vector<int> serverIdOrder;
  map<int, int> serverWeight;

  boost::mutex syncMutex;

 protected:
  std::vector<DistributionUpdateListener*> updateListener;

  bool needsSync;
};
}

#endif /* DISTRIBUTION_H_ */
