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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[_] [\_]

January 2012, Thomas Achmann. Initial Implementation

[1] Definition and Implementation of the class ThreadedMemoryCounter

[TOC]

0 Overview 

ThreadedMemoryCounter is a class to count memory allocation
among threads. 
It contains an integer variable, which is set to the maximum
memory available at initialisation.
Each thread can now retrieve a specified amount of memory, if
it is available. If not enough memory is available it will
be halted.
To avoid starvation in the case only one thread requests more
memory than available totally, it is still granted.


1 Preliminaries

1.1 Includes and defines

*/

#ifndef _THREADEDMEMORYCOUNTER_H_
#define _THREADEDMEMORYCOUNTER_H_
#include "zthread/Thread.h"
#include "zthread/Guard.h"
#include "zthread/Mutex.h"

/*
2 class ~ThreadedMemoryCounter~

*/

class ThreadedMemoryCounter
{
public:

/*
2.1 Constructor and destructor

*/

  ThreadedMemoryCounter(long inMemMax) 
  : cond(lock)
  , m_memAvailable(inMemMax)
  , m_memMax(inMemMax) 
  , m_cntAlloc(0) 
  { 
    //m_memMax = m_memAvailable = 0;
    assert(m_memAvailable >= 0);
  }

  virtual ~ThreadedMemoryCounter() 
  {  
    //cout << " A:" << m_memAvailable 
    //   << " M:" << m_memMax 
    //   << " C:" << m_cntAlloc << endl;
    assert (m_memAvailable == m_memMax);
    assert (m_cntAlloc == 0);
  }

/*
2.2 Method ~put[_]back~

Gives back inAmnt of memory to the pool

*/
  void put_back(long inAmnt) 
  {
    ZThread::Guard<ZThread::Mutex> g(lock);
    assert (inAmnt > 0);
     
    m_memAvailable += inAmnt;
    m_cntAlloc --;
    //cout << " B:" << inAmnt
    //   << " A:" << m_memAvailable 
    //   << " M:" << m_memMax 
    //   << " C:" << m_cntAlloc << endl;
    assert(m_memAvailable <= m_memMax);
    assert(m_cntAlloc >= 0);
    cond.signal();
  }

/*
2.3 Method ~request~

Requests inAmnt of Memory from the pool

*/

 void request(long inAmnt) 
  {
    ZThread::Guard<ZThread::Mutex> g(lock);
    while (m_memAvailable - inAmnt < 0 && m_cntAlloc > 0)
      {
        //cout << "TMC Wait" << endl;
        cond.wait();
      }
    m_memAvailable -= inAmnt;
    m_cntAlloc ++;
    //cout << " R:" << inAmnt
    //   << " A:" << m_memAvailable 
    //   << " M:" << m_memMax 
    //   << " C:" << m_cntAlloc << endl;
    return;
  }

/*
2.4 Method ~size~

Returns the available size of the pool.

*/

  long size() {
    ZThread::Guard<ZThread::Mutex> g(lock);
    return m_memAvailable;
  }


/*
2.5 Method ~count~

Returns the number of instances, which has allocated memory.

*/

  long count() {
    ZThread::Guard<ZThread::Mutex> g(lock);
    return m_cntAlloc;
  }

/*
2.6 Method ~max~

Returns the maximum available memory of the pool.

*/

  long max() {
    return m_memMax;
  }
/*
2.8 Private class methods

*/
private:
  // n/a

/*
2.7 Private data members

*/
private:
  ZThread::Mutex lock;
  ZThread::Condition cond;
  long m_memAvailable;
  long m_memMax;
  long m_cntAlloc;
};

/*
2.7 Typedefs

*/
typedef ThreadedMemoryCounter MemCntr;

#endif // _THREADEDMEMORYCOUNTER_H_
