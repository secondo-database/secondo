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

#include "NetworkStreamBuffer.h"

namespace KVS {

NetworkStream::NetworkStream(unsigned int id)
    : id(id), streamType(""), streamTypeSet(false) {}

NetworkStream::~NetworkStream() {
  pair<unsigned int, char*>* tuple;

  while ((tuple = tupleQueue.next()) != 0) {
    delete[] tuple->second;
    delete tuple;
  }
}

void NetworkStream::setStreamType(const string& sType) {
  boost::lock_guard<boost::mutex> guard(streamTypeMutex);
  streamType = sType;
  streamTypeSet = true;
  streamTypeCondition.notify_all();
}

string NetworkStream::getStreamType() {
  boost::unique_lock<boost::mutex> lock(streamTypeMutex);
  while (!streamTypeSet) {
    streamTypeCondition.wait(lock);
  }
  return streamType;
}

void NetworkStream::serveIPC(IPCConnection* conn) {
  pair<unsigned int, char*>* tuple;

  while ((tuple = tupleQueue.next()) != 0) {
    conn->write(&tuple->first);
    conn->write(tuple->second, tuple->first);
    delete[] tuple->second;
    delete tuple;
  }

  unsigned int close = 0;
  conn->write(&close);

  delete conn;
}

bool NetworkStreamBuffer::addNetworkStream(unsigned int id,
                                           NetworkStream* stream) {
  boost::lock_guard<boost::mutex> guard(streamsMutex);
  if (streams.find(id) == streams.end()) {
    streams.insert(make_pair(id, stream));

    streamsCondition.notify_all();
    return true;
  } else {
    return false;
  }
}

NetworkStream* NetworkStreamBuffer::createStream(unsigned int id) {
  boost::unique_lock<boost::mutex> lock(streamsMutex);

  if (streams.find(id) == streams.end()) {
    NetworkStream* temp = new NetworkStream(id);
    streams.insert(make_pair(id, temp));
    return temp;
  } else {
    return streams.find(id)->second;
  }
}

NetworkStream* NetworkStreamBuffer::getNetworkStream(unsigned int id) {
  boost::unique_lock<boost::mutex> lock(streamsMutex);
  while (streams.find(id) == streams.end()) {
    streamsCondition.wait(lock);
  }

  return streams.find(id)->second;
}

bool NetworkStreamBuffer::setDataSourceStream(NetworkStream* stream) {
  boost::lock_guard<boost::mutex> guard(dataSourceMutex);
  if (dataSourceStream != 0) {
    delete dataSourceStream;
  }
  dataSourceStream = stream;
  dataSourceCondition.notify_all();
  return true;
}

NetworkStream* NetworkStreamBuffer::getDataSourceStream() {
  boost::unique_lock<boost::mutex> lock(dataSourceMutex);
  while (dataSourceStream == 0) {
    dataSourceCondition.wait(lock);
  }

  return dataSourceStream;
}

bool NetworkStreamBuffer::removeStream(unsigned int id) {
  boost::unique_lock<boost::mutex> lock(streamsMutex);
  while (streams.find(id) == streams.end()) {
    streamsCondition.wait(lock);
  }

  map<unsigned int, NetworkStream*>::iterator streamItem = streams.find(id);

  if (streamItem != streams.end()) {
    delete streamItem->second;
    streams.erase(streamItem);
    return true;
  } else {
    return false;
  }
}
}
