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

1 Header File: ConcurrentTupleBufferReader

September 2019, Fischer Thomas

1.1 Overview

The abstract class ~ConcurrentTupleBufferReader~ allows the access to tuples 
temporary stored in the related ~ConcurrentTupleBuffer~. It encapsulates the 
tuple blocks retrieved from the buffer and supports an iterator interface to 
fetch single tuples. 

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

/*
1.3 Initalization and destruction

*/
    ConcurrentTupleBufferReader(ConcurrentTupleBuffer *buffer)
        : m_buffer(buffer), m_currentTupleBlock(NULL), m_numReadTuples(0)
    {
    }

    virtual ~ConcurrentTupleBufferReader()
    {
      Close();
    };

/* 
The constructor connects a new ~ConcurrentTupleBufferReader~ to the 
~TupleBuffer~. The disconnection occurs on destruction of the reader.

*/

    bool IsEndOfDataReached()
    {
      if (ReadNextTupleBlockIfNecessary())
      {
        return m_currentTupleBlock->Type() ==
               TupleBlockType::EndOfDataStreamBlock;
      }
      return false;
    }

/* 
This method indicates if the end of the data stream is reached. For this
purpose it checks the current loaded tuple block if it is a ~EndOfDataStreamBlock~

*/

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
/* 
~TryReadTuple~ allows access to a single tuple out of a tuple block. If all 
records of the current tuple block are read, the block is passed to the associated 
~ConcurrentTupleBuffer~ to be deleted. The next call of this method will 
request a new block from the buffer. If no block is available it returns ~false~
to notify the calling code.

*/

    size_t NumReadTuples()
    {
      return m_numReadTuples;
    }

    virtual std::string ToString() = 0;

  protected: //methods

    virtual bool ReadNextTupleBlockIfNecessary() = 0;

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


/*
1.3 ConcurrentTupleBufferDedicatedReader and ConcurrentTupleBufferSharedReader

There are two implementations of the reader. Both implementations differ just 
in the way they request new tuple blocks from the ~ConcurrentTupleBuffer~ . 

The ~ConcurrentTupleBufferSharedReader~ access all queues of the tuple buffer 
in a round robin fashion. The ~ConcurrentTupleBufferDedicatedReader~ only gets 
the tuple blocks from a fixed queue determined at construction of the reader. 
This kind of reader is used for hash-partitioning. 

*/

  class ConcurrentTupleBufferSharedReader : public ConcurrentTupleBufferReader
  {
  public: //methods
    ConcurrentTupleBufferSharedReader(ConcurrentTupleBuffer *buffer)
        : ConcurrentTupleBufferReader(buffer)
    {
    }

    virtual std::string ToString() override
    {
      return "ConcurrentTupleBufferSharedReader";
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

  class ConcurrentTupleBufferDedicatedReader
      : public ConcurrentTupleBufferReader
  {
  public: //methods
    ConcurrentTupleBufferDedicatedReader(ConcurrentTupleBuffer *buffer,
                                         size_t partitionIdx)
        : ConcurrentTupleBufferReader(buffer), m_partitionIdx(partitionIdx)
    {
    }

    virtual std::string ToString() override
    {
      std::stringstream message;
      message << "ConcurrentTupleBufferDedicatedReader, Partitionindex ";
      message << m_partitionIdx;
      return message.str();
    }

  protected: //methods
    virtual bool ReadNextTupleBlockIfNecessary() override
    {
      if (m_currentTupleBlock == NULL)
      {
        return ConsumeTupleBlockByPartition(m_currentTupleBlock,
                                            m_partitionIdx);
      }
      return true;
    }

  private: //member
    int m_partitionIdx;
  };

} // namespace parthread
#endif //SECONDO_PARTHREAD_CONCURRENT_TUPLE_BUFFER_READER_H
