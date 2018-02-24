/*
----
This file is part of SECONDO.
Realizing a simple distributed filesystem for master thesis of stephan scheide

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


//[$][\$]

*/
#include "DataNodeIndex.h"
#include "../shared/numberUtils.h"

using namespace dfs;
using namespace std;

typedef std::map<Str, DataNodeEntry>::iterator IT;

DataNodeIndex::DataNodeIndex() {
}

DataNodeIndex::~DataNodeIndex() {
}

void DataNodeIndex::add(const Str &uri) {
  DataNodeEntry v;
  v.uri = URI::fromString(uri);
  v.usage = 0;
  index[uri] = v;
}

void DataNodeIndex::addRaw(const DataNodeEntry &entry) {
  IT pos = index.begin();
  index.insert(pos, std::pair<Str, DataNodeEntry>(entry.uri.toString(), entry));
}

int DataNodeIndex::count() const {
  return index.size();
}

void DataNodeIndex::remove(const Str &uri) {
  index.erase(uri);
}

bool DataNodeIndex::hasNode(const Str &uri) {
  return index.count(uri) > 0;
}

vector<DataNodeEntry> DataNodeIndex::need(int amount) {

  const int amountOfKnownDataNodes = count();
  int s = amountOfKnownDataNodes < amount ? amountOfKnownDataNodes : amount;

  //get possible indices
  int *indices = numberUtils::findUniqueRandomInts(0,
                                                   amountOfKnownDataNodes - 1,
                                                   s);

  //get list of keys from index
  vector<Str> keys;
  for (IT it = index.begin(); it != index.end(); it++) {
    keys.push_back(it->first);
  }

  vector<DataNodeEntry> result;

  for (int i = 0; i < s; i++) {
    int vectorIndex = indices[i];
    Str key = keys.at(vectorIndex);
    result.push_back(index.at(key));
  }
  delete[] indices;
  return result;
}

vector<URI> DataNodeIndex::allURIs() {
  vector<URI> uris;
  for (IT it = index.begin(); it != index.end(); it++) {
    uris.push_back(it->second.uri);
  }
  return uris;
}
