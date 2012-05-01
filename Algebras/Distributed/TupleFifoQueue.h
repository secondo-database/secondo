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
[1] TupleFifoQueue

September 2011 Thomas Achmann

TupleFifoQueue implements a threadsave fifo queue of Tuple pointers.

It is possible to feed the queue by one thread and retrieve the
tuples by an other thread. 

*/

#ifndef _TUPLEFIFOQUEUE_H_
#define _TUPLEFIFOQUEUE_H_

#include <queue>
#include "zthread/Thread.h"
#include "zthread/Guard.h"
#include "zthread/FastMutex.h"

class Tuple;

class TupleFifoQueue
{
public:
/*

1.1 TupleFifoQueue C-/D-tors

*/

  TupleFifoQueue() 
    : m_cond (m_lock) {}

  virtual ~TupleFifoQueue() {}

/*

1.2 put

Adds a Tuple pointer to the queue.

*/
  void put(Tuple* tb) {
    ZThread::Guard<ZThread::FastMutex> g(m_lock);
    m_data.push_back(tb);
    m_cond.signal();
  }

/*

1.3 get

returns a Tuple pointer from the queue.
If queue is empty it waits until a new 
Tuple pointer is added.

*/

  Tuple* get() {
    ZThread::Guard<ZThread::FastMutex> g(m_lock);
    while(m_data.empty())
      m_cond.wait();
    Tuple* returnVal = m_data.front();
    m_data.pop_front();
    return returnVal;
  }

/*

1.4 size

returns the size of the queue.

*/

  unsigned int size() {
    ZThread::Guard<ZThread::FastMutex> g(m_lock);
    return m_data.size();
  }

/*

1.4 empty

returns true, if queue is empty, flase otherwise.

*/

  bool empty() {
    ZThread::Guard<ZThread::FastMutex> g(m_lock);
    return m_data.empty();
  }

private:

  ZThread::FastMutex m_lock;
  ZThread::Condition m_cond;
  std::deque<Tuple* > m_data;
};

typedef TupleFifoQueue* TFQ;

#endif // _TUPLEBUFFERQUEUE_H_
