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

#ifndef RESTRUCTURELOCK_H_
#define RESTRUCTURELOCK_H_

#include <ctime>

#include "boost/thread.hpp"
#include "boost/date_time.hpp"

namespace KVS {

class RestructureLock {
 public:
  RestructureLock() : restructuring(false), retrieveCounter(0), lastUpdate(0) {}
  ~RestructureLock() {}

  bool tryLockClient() {
    boost::lock_guard<boost::mutex> guard(mtx);

    if (!restructuring) {
      retrieveCounter++;
      time(&lastUpdate);
      return true;
    } else {
      return false;
    }
  }

  bool unlockClient() {
    boost::lock_guard<boost::mutex> guard(mtx);
    if (retrieveCounter > 0) {
      retrieveCounter--;
      return true;
    } else {
      return false;
    }
  }

  void updateLock() {
    // TODO: ?
    boost::lock_guard<boost::mutex> guard(mtx);
    time(&lastUpdate);
  }

  bool startRestructuring() {
    boost::lock_guard<boost::mutex> guard(mtx);
    restructuring = true;
    return true;
  }

  bool lock() {
    mtx.lock();

    restructuring = true;

    while (retrieveCounter > 0 &&
           difftime(time(NULL), lastUpdate) < TIMEOUTINTERVAL) {
      mtx.unlock();
      boost::this_thread::sleep(boost::posix_time::milliseconds(100));
      mtx.lock();
    }

    mtx.unlock();

    return true;
  }

  bool unlock() {
    boost::lock_guard<boost::mutex> guard(mtx);
    restructuring = false;
    retrieveCounter = 0;
    return true;
  }

  bool restructuring;

 private:
  unsigned int retrieveCounter;  // clients currently retrieving

  time_t lastUpdate;                  // clients alive?
  const double TIMEOUTINTERVAL = 30;  // in seconds

  boost::mutex mtx;
};
}

#endif /* RESTRUCTURELOCK_H_ */
