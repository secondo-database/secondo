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

#ifndef NETWORKSTREAMBUFFER_H_
#define NETWORKSTREAMBUFFER_H_

#include <map>

#include "ipc.h"
#include "SyncPseudoQueue.h"

#include "boost/thread.hpp"
#include "boost/date_time.hpp"

class Tuple;

namespace KVS {

class NetworkStream {
 public:
  NetworkStream(unsigned int id);
  ~NetworkStream();

  void setStreamType(const string& sType);
  string getStreamType();

  void serveIPC(IPCConnection* conn);

  SyncPseudoQueue<pair<unsigned int, char*>*> tupleQueue;

 private:
  unsigned int id;

  string streamType;
  boost::mutex streamTypeMutex;
  boost::condition_variable streamTypeCondition;
  bool streamTypeSet;
};

class NetworkStreamBuffer {
 public:
  NetworkStreamBuffer() : dataSourceStream(0) {}
  ~NetworkStreamBuffer() {}

  NetworkStream* createStream(unsigned int id);

  bool addNetworkStream(unsigned int id, NetworkStream* stream);
  NetworkStream* getNetworkStream(unsigned int id);

  bool setDataSourceStream(NetworkStream* stream);
  NetworkStream* getDataSourceStream();

  bool removeStream(unsigned int id);

 private:
  map<unsigned int, NetworkStream*> streams;
  boost::mutex streamsMutex;
  boost::condition_variable streamsCondition;

  NetworkStream* dataSourceStream;
  boost::mutex dataSourceMutex;
  boost::condition_variable dataSourceCondition;
};
}

#endif /* NETWORKSTREAMBUFFER_H_ */
