/*
---- 
This file is part of SECONDO.

Copyright (C) 2019, University in Hagen, Department of Computer Science, 
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

//paragraph    [10]    title:           [{\Large \bf ] [}]
//paragraph    [21]    table1column:    [\begin{quote}\begin{tabular}{l}]     [\end{tabular}\end{quote}]
//paragraph    [22]    table2columns:   [\begin{quote}\begin{tabular}{ll}]    [\end{tabular}\end{quote}]
//paragraph    [23]    table3columns:   [\begin{quote}\begin{tabular}{lll}]   [\end{tabular}\end{quote}]
//paragraph    [24]    table4columns:   [\begin{quote}\begin{tabular}{llll}]  [\end{tabular}\end{quote}]
//[--------]    [\hline]
//characters    [1]    verbatim:   [$]    [$]
//characters    [2]    formula:    [$]    [$]
//characters    [3]    capital:    [\textsc{]    [}]
//characters    [4]    teletype:   [\texttt{]    [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]
//[Contents] [\tableofcontents]

1 Header File: ConcurrentTupleBuffer

September 2019, Fischer Thomas

1.1 Overview

The ~ConcurrentTupleBuffer~ temporary stores tuple blocks produced by execution
contexts before they are consumed by an adjacent context. Access to write and 
read methods is only possible through ~ConcurrentTupleBufferReader~ and 
~ConcurrentTupleBufferWriter~.

1.2 Imports

*/

#ifndef SECONDO_PARTHREAD_CONCURRENT_TUPLE_BUFFER_H
#define SECONDO_PARTHREAD_CONCURRENT_TUPLE_BUFFER_H

#include "TupleBlock.h"
#include "DataPartitioner.h"
#include "Algebras/ExtRelation-2/TupleBuffer2.h"

#include "boost/thread/shared_mutex.hpp"
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <set>
#include <atomic>

namespace parthread
{

class ConcurrentTupleBufferReader;
class ConcurrentTupleBufferWriter;

class ConcurrentTupleBuffer
{

public: //methods
/*
1.3 Initalization and destruction

*/
  ConcurrentTupleBuffer(const ConcurrentTupleBufferSettings settings);

  ~ConcurrentTupleBuffer();
/*
The ~ConcurrentTupleBuffer~ initializes an array of queues depending on the 
number of partitions. Each queue can temporary store tuple blocks of different
types sharing the ~ITupleBlock~ interface.


1.4 Methods

*/

  ConcurrentTupleBufferReader *GetTupleBufferReader(int partitionIdx = 0);

  ConcurrentTupleBufferWriter *GetTupleBufferWriter();
/*
These methods are used to create writers and readers to get access to the stored
tuples or to add new tuples to the buffer. Every reader/writer has an reference to
the created buffer and can access methods to allocate or provide tuple blocks.

*/

  size_t Size() const
  {
    return m_numTupleBlocks;
  }

  size_t SizeOfPartition(int partitionIndex) const
  {
    m_access.lock();
    assert(partitionIndex < (int)m_numPartitions);
    size_t sizeOfPartition = m_tupleBlockMatrix[partitionIndex]->size();
    m_access.unlock();

    return sizeOfPartition;
  }
/*
~Size~ and ~SizeOfPartition~ provide the number of stored tuples by partition or
for the complete block. The buffer allows concurrent access, so it's possible that
the returned size can change in the meantime if other threads access the puffer too.

*/

  size_t NumberOfPartitions() const
  {
    return m_numPartitions;
  }


private: //methods

  bool AllocateTupleBlocks(TupleBlockVector &tupleBlockVector);
/*
Reserves memory for the allocated tuple block vector. This method is called by
writers.

*/

  void ProvideTupleBlocks(TupleBlockVector &tupleBlockVector);
/*
Adds filled tuple blocks of the given ~TupleBlockVector~ to the queues of the 
buffer. This method is called by writers.

*/

  bool ConsumeTupleBlockByPartition(TupleBlockPtr &tupleBlock,
                                    const size_t partitionIdx);
/*
Gets the next tuple block available for the partition at ~partitionIdx~. If no
tuple is available the method returns ~false~. This method is called by
readers.

*/


  bool ConsumeNextTupleBlock(TupleBlockPtr &tupleBlock);
/*
Similar to ~ConsumeTupleBlockByPartition~ but gets the tuple block from the 
internal queue with the highest number of tuple blocks. This method is called by
readers.

*/

  void DeallocateTupleBlock(TupleBlockPtr &tupleBlock);
/*
Consumed tuple blocks needs to be deallocated and the reserved memory returns to
the tuple puffers pool. This method is called by readers.

*/

  void RemoveWriter(ConcurrentTupleBufferWriter *writer);

  void RemoveReader(ConcurrentTupleBufferReader *reader);
/*
The writer and reader are responsible to call the remove-methods and quit the 
connection to the tuple buffer.

*/

  int DistributeMemory(const size_t minMemoryExpected);

  void SwapExistingMemoryBlocks(int minMemoryToSwap,
                                TupleBlockVector &tupleBlockVector);

private: //member
  ConcurrentTupleBufferSettings m_settings;
  size_t m_availableMemory;
  size_t m_defaultMemoryPerBlock;
  TupleBlockPool m_blockPool;

  std::set<ConcurrentTupleBufferReader *> m_readerDirectory;
  std::set<ConcurrentTupleBufferWriter *> m_writerDirectory;

  long m_lastNotificationBufferSize;

  TupleBlockQueuePtr *m_tupleBlockMatrix;
  const size_t m_numPartitions;
  int m_numTupleBlocks;

  mutable std::recursive_mutex m_access;
  std::condition_variable m_tupleBlockAvailable;

  friend class ConcurrentTupleBufferReader;
  friend class ConcurrentTupleBufferWriter;
};

typedef std::shared_ptr<ConcurrentTupleBuffer> ConcurrentTupleBufferPtr;

} // namespace parthread
#endif //SECONDO_PARTHREAD_CONCURRENT_TUPLE_BUFFER_H
