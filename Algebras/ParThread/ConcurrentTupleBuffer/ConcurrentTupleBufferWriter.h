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

1 Header File: ConcurrentTupleBufferWriter

September 2019, Fischer Thomas

1.1 Overview

The ~ConcurrentTupleBufferWriter~ is an interface for the ~ConcurrentTupleBuffer~
to write tuples into tuple blocks. 

1.2 Imports

*/

#ifndef SECONDO_PARTHREAD_CONCURRENT_TUPLE_BUFFER_WRITER_H
#define SECONDO_PARTHREAD_CONCURRENT_TUPLE_BUFFER_WRITER_H

#include "ConcurrentTupleBuffer.h"

namespace parthread
{

class ConcurrentTupleBufferWriter
{
public:

/*
1.3 Initalization and destruction

*/

  ConcurrentTupleBufferWriter(ConcurrentTupleBuffer *buffer,
                              DataPartitioner *partitioner)
      : m_buffer(buffer), m_tupleBlockVector(buffer->m_numPartitions),
        m_partitioner(partitioner->Copy()), m_numWrittenTuples(0),
        m_hasMemoryAllocated(false)
  {
  }

  virtual ~ConcurrentTupleBufferWriter()
  {
    if (m_buffer != NULL)
    {
      Flush();
      m_buffer->RemoveWriter(this);
      m_buffer = NULL;
    }

    if (m_partitioner != NULL)
    {
      delete m_partitioner;
      m_partitioner = NULL;
    }
  };

/* 
The constructor connects a new ~ConcurrentTupleBufferWriter~ to the 
~TupleBuffer~. Additionally a copy of the partitioner needs to be passed to the 
constructor. The disconnection from the buffer and the deletion of the partitioner
occurs on destruction of the writer.

1.4 Methods

*/


  bool Allocate()
  {
    if (!m_hasMemoryAllocated)
    {
      m_hasMemoryAllocated = m_buffer->AllocateTupleBlocks(m_tupleBlockVector);
    }

    return m_hasMemoryAllocated;
  }

/* 
Allocates memory from the buffer to store new tuples in the internal tuple blocks.
The method returns ~false~ if there is already memory allocated or it's not possible
for to get an empty vector of tuple blocks from the buffer.

*/

  bool HasFreeMemory()
  {
    return m_tupleBlockVector.FreeMemorySize() > 0;
  }

/* 
Returns ~true~ if theres is memory left to store additional buffers, otherwise ~false~. 

*/


  virtual void WriteTuple(Tuple *tuple)
  {
    assert(m_hasMemoryAllocated);

    size_t partitionIdx = m_partitioner->DistributeValue(tuple);

    m_tupleBlockVector.AddTupleToPartition(tuple, partitionIdx);
    m_numWrittenTuples++;
  }
/* 
Write a single tuple to the vector of tuple blocks. The internal ~DataPartitioner~
is responsible to choose the tuple block. The writer must allocate a new tuple
vector before using this method.

*/

  void Flush()
  {
    if (m_hasMemoryAllocated)
    {
      m_buffer->ProvideTupleBlocks(m_tupleBlockVector);
    }
    m_hasMemoryAllocated = false;
  }
/* 
Returns the tuple block back to the tuple buffer.

*/

  size_t NumWrittenTuples()
  {
    return m_numWrittenTuples;
  }
/* 
Counts all tuples processed by this writer independent of the number of allocated
blocks or used tasks. For debug pruposes.

*/

private: //methods
private: //member
  ConcurrentTupleBuffer *m_buffer;
  TupleBlockVector m_tupleBlockVector;
  DataPartitioner *m_partitioner;
  size_t m_numWrittenTuples;
  bool m_hasMemoryAllocated;
};

} // namespace parthread
#endif //SECONDO_PARTHREAD_CONCURRENT_TUPLE_BUFFER_WRITER_H
