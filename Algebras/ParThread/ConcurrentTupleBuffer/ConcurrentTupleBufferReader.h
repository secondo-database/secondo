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

1 Header File: ConcurrentTupleBufferReader

September 2019, Fischer Thomas

1.1 Overview

1.2 Imports

*/

#ifndef SECONDO_PARTHREAD_CONCURRENT_TUPLE_BUFFER_READER_H
#define SECONDO_PARTHREAD_CONCURRENT_TUPLE_BUFFER_READER_H

#include "ConcurrentTupleBuffer.h"

namespace parthread
{

class ConcurrentTupleBufferReader
{
public:
  /* Connects a new ~ConcurrentTupleBufferReader~ to the ~TupleBuffer~ and 
    allocates resources */
  ConcurrentTupleBufferReader(ConcurrentTupleBuffer *buffer)
      : m_buffer(buffer), m_currentTupleBlock(NULL), m_numReadTuples(0)
  {
  }

  virtual ~ConcurrentTupleBufferReader()
  {
    Close();
  };

  bool IsEndOfDataReached()
  {
    if (ReadNextTupleBlockIfNecessary())
    {
      return m_currentTupleBlock->Type() ==
             TupleBlockType::EndOfDataStreamBlock;
    }
    return false;
  }

  bool TryReadTuple(Tuple *&tuple)
  {
    //fetch next tuple from block
    if (ReadNextTupleBlockIfNecessary() &&
        m_currentTupleBlock->TryPull(tuple))
    {
      //if the tuple block is empty return it to the buffer
      if (m_currentTupleBlock->IsEmpty())
      {
        m_buffer->DeallocateTupleBlock(m_currentTupleBlock);
      }

      m_numReadTuples++;
      return true;
    }

    return false;
  }

  size_t NumReadTuples()
  {
    return m_numReadTuples;
  }

protected: //methods
  /*
  returns true if a tuple block is available, otherwise false 
  */
  virtual bool ReadNextTupleBlockIfNecessary() = 0;

  /* Disconnects the ~ConcurrentTupleBufferReader~ from the ~TupleBuffer~ 
     and frees resources for other connected writers */
  void Close()
  {
    if (m_buffer != NULL)
    {
      if (m_currentTupleBlock != NULL)
      {
        m_buffer->DeallocateTupleBlock(m_currentTupleBlock);
      }

      m_buffer->RemoveReader(this);
      m_buffer = NULL;
    }
  }

  bool ConsumeTupleBlockByPartition(TupleBlockPtr &tupleBlock, 
                                    const size_t partitionIdx)
  {
    return m_buffer->ConsumeTupleBlockByPartition(tupleBlock, partitionIdx);
  }

  bool ConsumeNextTupleBlock(TupleBlockPtr &tupleBlock)
  {
    return m_buffer->ConsumeNextTupleBlock(tupleBlock);
  }

  size_t NumPartitionsInBuffer()
  {
    return m_buffer->m_numPartitions;
  }

protected: //member
  ConcurrentTupleBuffer *m_buffer;
  TupleBlockPtr m_currentTupleBlock;
  size_t m_numReadTuples;
};

class ConcurrentTupleBufferSharedReader : public ConcurrentTupleBufferReader
{
public: //methods
  ConcurrentTupleBufferSharedReader(ConcurrentTupleBuffer *buffer)
      : ConcurrentTupleBufferReader(buffer)
  {
  }

protected: //methods
  virtual bool ReadNextTupleBlockIfNecessary() override
  {
    if (m_currentTupleBlock == NULL)
    {
      return ConsumeNextTupleBlock(m_currentTupleBlock);
    }
    return true;
  }
};

class ConcurrentTupleBufferDedicatedReader : public ConcurrentTupleBufferReader
{
public: //methods
  ConcurrentTupleBufferDedicatedReader(ConcurrentTupleBuffer *buffer, 
                                       size_t partitionIdx)
      : ConcurrentTupleBufferReader(buffer), m_partitionIdx(partitionIdx)
  {
  }

protected: //methods
  virtual bool ReadNextTupleBlockIfNecessary() override
  {
    if (m_currentTupleBlock == NULL)
    {
      return ConsumeTupleBlockByPartition(m_currentTupleBlock, m_partitionIdx);
    }
    return true;
  }

private: //member
  int m_partitionIdx;
};

} // namespace parthread
#endif //SECONDO_PARTHREAD_CONCURRENT_TUPLE_BUFFER_READER_H
