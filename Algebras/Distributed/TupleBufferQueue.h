/*
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
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

/*
[1] TupleBufferQueue

September 2011 Thomas Achmann

TupleBufferQueue implements a threadsave queue of TupleBuffer pointers.

It is used to limit the number of concurrent relations on the server while
transferring data from the workers (e.g dsummarize command). 

*/

#ifndef _TUPLEBUFFERQUEUE_H_
#define _TUPLEBUFFERQUEUE_H_

#include "StandardTypes.h"
#include "zthread/Thread.h"
#include "zthread/Guard.h"
#include "zthread/Mutex.h"

class TupleBufferQueue
{
public:
/*

1.1 TupleBufferQueue C-/D-tors

*/

  TupleBufferQueue() : cond(lock) {}
  virtual ~TupleBufferQueue() {  }

/*

1.2 put

Adds a TupleBufferPointer to the queue.

*/
  void put(TupleBuffer* tb) {
    ZThread::Guard<ZThread::Mutex> g(lock);
    data.push_back(tb);
    cond.signal();
  }

/*

1.3 get

returns a TupleBufferPointer from the queue.
If queue is empty it waits until a new 
TupleBufferPointer is added.

*/

  TupleBuffer* get() {
    ZThread::Guard<ZThread::Mutex> g(lock);
    while(data.empty())
      cond.wait();
    TupleBuffer* returnVal = data.front();
    data.pop_front();
    return returnVal;
  }

/*

1.4 size

returns the size of the queue.

*/

  unsigned int size() {
    ZThread::Guard<ZThread::Mutex> g(lock);
    return data.size();
  }

/*

1.4 empty

returns true, if queue is empty, flase otherwise.

*/

  bool empty() {
    ZThread::Guard<ZThread::Mutex> g(lock);
    return data.empty();
  }

private:
  ZThread::Mutex lock;
  ZThread::Condition cond;
  std::deque<TupleBuffer *> data;
};

typedef TupleBufferQueue* TBQueue;

#endif // _TUPLEBUFFERQUEUE_H_
