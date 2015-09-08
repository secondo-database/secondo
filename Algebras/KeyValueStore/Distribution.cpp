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

#include "Distribution.h"
#include "QuadTreeDistributionType.h"

#include <sstream>

namespace KVS {

Distribution* Distribution::getInstance(const string& data) {
  stringstream strData(data);

  int type;
  strData.read((char*)&type, sizeof(type));

  Distribution* dist = 0;

  if (type == TYPE_QUADTREE) {
    dist = new QuadTreeDistribution();
  } else {
    return 0;
  }

  if (dist->fromBin(data)) {
    return dist;
  } else {
    delete dist;
    return 0;
  }
}

const bool Distribution::checkType(const ListExpr type) {
  return QuadTreeDistributionType::checkType(type);
}

void Distribution::addUpdateListener(DistributionUpdateListener* listener) {
  updateListener.push_back(listener);
}

void Distribution::removeUpdateListener(DistributionUpdateListener* listener) {
  for (unsigned int listenerIdx = 0; listenerIdx < updateListener.size();
       ++listenerIdx) {
    if (updateListener[listenerIdx] == listener) {
      updateListener.erase(updateListener.begin() + listenerIdx);
      return;
    }
  }
}

void Distribution::distributionUpdated() {
  for (unsigned int listenerIdx = 0; listenerIdx < updateListener.size();
       ++listenerIdx) {
    updateListener[listenerIdx]->distributionUpdated();
  }
}
bool Distribution::needsSynchronisation() { return needsSync; }
}
