
/*
----
This file is part of SECONDO.

Copyright (C) 2012, University in Hagen, Department of Computer Science,
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
//[&] [\&]
//[x] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]
//[>] [\ensuremath{>}]
//[<] [\ensuremath{<}]
//[ast] [\ensuremath{\ast}]

*/

/*
[1] Class DServerShuffleSender Definition

\begin{center}
April 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

The class ~DServerShuffleSender~ sends the relation data on 
the workers to the workers of the new darray, where it will be received

1 Preliminaries

1.1 Defines

*/
#ifndef H_DSERVERSHUFFLESENDER_H
#define H_DSERVERSHUFFLESENDER_H

/*
1.3 Includes

*/
#include "zthread/Runnable.h"
#include "ThreadedMemoryCntr.h"
#include "TupleFifoQueue.h"

/*
2 Class ~DServerThreadRunner~

  * derives from class ~ZThread::Runnable~

*/
class DServerShuffleSender : public ZThread::Runnable
{
public:
/*
2.1 Default Constructor

  * Automatically created

*/
/*
2.2 Constructor

  * const string[&] inDestHost - the host name of the worker, where 
the new darray index data is stored

  * const string[&] inToPort - the port number (as string) of the worker,where 
the new darray index data is stored

  * MemCntr[ast] inMemCntr - a pointer to a datastructure, where the used 
memory is counted

*/
  DServerShuffleSender(const std::string& inDestHost,
                       const std::string& inToPort,
                       MemCntr* inMemCntr) 
    : ZThread::Runnable()
    , m_destHost(inDestHost)
    , m_toPort(inToPort)
    , m_runit(true)
    , m_memCntr(inMemCntr) {}

/*
2.5 Modifying Tuplequeue

2.5.1 Method ~void AppendTuple~

appends a tuple to the tuple queue

  * Tuple[ast] - the tuple to be appended

*/
  void AppendTuple(Tuple* t)
  {
    m_tfq.put(t);
  }

/*
2.5.2 Method ~void done~

terminates the tuple queue

*/

  void done()
  {
    m_runit = false;
    m_tfq.put(NULL); // dummy to wake up waiting threads
  }

/*
2.6 Running

2.6.1 Method ~void run~

*/

  void run();

/*
2.8 Private Section

*/
private:

/*
2.8.1 Private Methods

*/

// n/a

/*
2.8.1 Private Members

*/
  TupleFifoQueue m_tfq;
  std::string m_destHost;
  std::string m_toPort;
  bool m_runit;
  MemCntr* m_memCntr;

/*
2.9 End of Class 

*/
};
#endif // H_DSERVERSHUFFLESENDER_H
