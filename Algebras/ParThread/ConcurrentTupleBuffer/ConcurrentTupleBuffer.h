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

/**************************************************************************
Forward declaration of several classes:

*/
namespace parthread
{

class ConcurrentTupleBufferReader;
class ConcurrentTupleBufferWriter;

class ConcurrentTupleBuffer
{

public: //methods
  ConcurrentTupleBuffer(const ConcurrentTupleBufferSettings settings);

  ~ConcurrentTupleBuffer();

  ConcurrentTupleBufferReader *GetTupleBufferReader(int partitionIdx = 0);

  ConcurrentTupleBufferWriter *GetTupleBufferWriter();

  size_t Size() const
  {
    return m_numTupleBlocks;
  }

  size_t NumberOfPartitions() const
  {
    return m_numPartitions;
  }

  size_t SizeOfPartition(int partitionIndex) const
  {
    m_access.lock();
    assert(partitionIndex < (int)m_numPartitions);
    size_t sizeOfPartition = m_tupleBlockMatrix[partitionIndex]->size();
    m_access.unlock();

    return sizeOfPartition;
  }

private:
  /*blocks until there is a writer connected and tuples are available
    if the tupleBuffer has no producing writers and signals that no new
    writers are expected (TupleBuffer is closed), then the reader 
    consumes the already queued tuples and returns ~false~ after the last
    element was retrieved. 
    */
  bool AllocateTupleBlocks(TupleBlockVector &tupleBlockVector);

  void ProvideTupleBlocks(TupleBlockVector &tupleBlockVector);

  bool ConsumeTupleBlockByPartition(TupleBlockPtr &tupleBlock,
                                    const size_t partitionIdx);

  bool ConsumeNextTupleBlock(TupleBlockPtr &tupleBlock);

  /* threadsave */
  void DeallocateTupleBlock(TupleBlockPtr &tupleBlock);

  /* threadsave */
  void RemoveWriter(ConcurrentTupleBufferWriter *writer);

  /* threadsave */
  void RemoveReader(ConcurrentTupleBufferReader *reader);

  inline const size_t TotalMemorySize() const
  {
    return m_settings.TotalBufferSizeInBytes;
  };

  void BufferSizeChanged();

private: //methods
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
