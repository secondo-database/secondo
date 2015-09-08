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

#ifndef SYNCPSEUDOQUEUE_H_
#define SYNCPSEUDOQUEUE_H_

#include <vector>

#include "boost/thread.hpp"
#include "boost/date_time.hpp"

using namespace std;

namespace KVS {

/* only allows pointer types
 * and sees 0 as end of queue*/
template <typename T>
class SyncPseudoQueue;

template <typename T>
class SyncPseudoQueue<T*> {
 public:
  SyncPseudoQueue() : complete(false), readPos(0), actualSize(0) {}

  ~SyncPseudoQueue() {}

  T* next() {
    boost::unique_lock<boost::mutex> lock(dataMutex);
    while (readPos == data.size()) {
      dataCondition.wait(lock);
    }
    T* result = data[readPos];

    if (data[readPos] != 0) {
      readPos++;
    }

    return result;
  }

  void add(T* item) {
    boost::lock_guard<boost::mutex> guard(dataMutex);
    data.push_back(item);
    dataCondition.notify_all();
    if (item != 0) {
      actualSize++;
    }
  }

  void setComplete() {
    boost::lock_guard<boost::mutex> guard(dataMutex);
    data.push_back(0);
    dataCondition.notify_all();
  }

  unsigned int size() { return actualSize; }

 private:
  bool complete;
  unsigned int readPos;
  unsigned int actualSize;

  vector<T*> data;
  boost::mutex dataMutex;
  boost::condition_variable dataCondition;
};
}

#endif /* SYNCPSEUDOQUEUE_H_ */
