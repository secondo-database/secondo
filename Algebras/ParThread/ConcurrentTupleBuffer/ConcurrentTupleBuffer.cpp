/*
----
This file is part of SECONDO.

Copyright (C) since 2009, University in Hagen, Faculty of Mathematics
and Computer Science, Database Systems for New Applications.

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

#include "ConcurrentTupleBuffer.h"
#include "ConcurrentTupleBufferReader.h"
#include "ConcurrentTupleBufferWriter.h"

using namespace std;

namespace parthread
{

ConcurrentTupleBuffer::ConcurrentTupleBuffer(
    const ConcurrentTupleBufferSettings settings)
    : m_settings(settings), m_availableMemory(settings.TotalBufferSizeInBytes),
      m_blockPool(m_settings), m_lastNotificationBufferSize(0),
      m_tupleBlockMatrix(NULL),
      m_numPartitions(settings.DataPartitioner->NumPartitions()),
      m_numTupleBlocks(0)
{
  assert(settings.TotalBufferSizeInBytes > 0);
  //constrain the memory per block to the maximum of an int, because ints are
  //used for the blocks memory counter
  m_defaultMemoryPerBlock = std::min((settings.TotalBufferSizeInBytes /
                         m_settings.MemoryDistributionFactor), (size_t)INT_MAX);

  //initialize block matrix
  m_tupleBlockMatrix = new TupleBlockQueuePtr[m_numPartitions];
  for (size_t i = 0; i < m_numPartitions; i++)
  {
    m_tupleBlockMatrix[i] = new TupleBlockQueue();
  }
};

ConcurrentTupleBuffer::~ConcurrentTupleBuffer()
{
  for (size_t i = 0; i < m_numPartitions; i++)
  {
    delete m_tupleBlockMatrix[i];
  }
  delete[] m_tupleBlockMatrix;
}

ConcurrentTupleBufferReader *ConcurrentTupleBuffer::GetTupleBufferReader(
    int partitionIndex)
{
  ConcurrentTupleBufferReader *reader = NULL;

  switch (m_settings.DataPartitioner->DistributionType())
  {
  case DistributionTypes::SharedPartitions:
    reader = new ConcurrentTupleBufferSharedReader(this);
    break;
  case DistributionTypes::DedicatedPartition:
    reader = new ConcurrentTupleBufferDedicatedReader(this, partitionIndex);
    break;
  }

  m_access.lock();
  m_readerDirectory.insert(reader);
  m_access.unlock();

  return reader;
}

ConcurrentTupleBufferWriter *ConcurrentTupleBuffer::GetTupleBufferWriter()
{
  ConcurrentTupleBufferWriter *writer = new ConcurrentTupleBufferWriter(
      this, m_settings.DataPartitioner.get());

  m_access.lock();
  m_writerDirectory.insert(writer);
  m_access.unlock();
  return writer;
}

bool ConcurrentTupleBuffer::AllocateTupleBlocks(
    TupleBlockVector &tupleBlockVector)
{
  m_access.lock();
  //an already allocated tuple must be provided to the tuplebuffer
  assert(!tupleBlockVector.IsInitialized());

  int blockVectorMemory = DistributeMemory(
      m_settings.MinMemoryPerTupleVector);
  bool hasMemoryAllocated = blockVectorMemory > 0;

  //get next free blocks to fill into vector
  if (hasMemoryAllocated)
  {
    for (size_t i = 0; i < m_numPartitions; i++)
    {
      tupleBlockVector.SetTupleBlockByPartition(
          m_blockPool.GetMemoryTupleBlock(), i, blockVectorMemory);
    }
  }
  m_access.unlock();

  return hasMemoryAllocated;
}

int ConcurrentTupleBuffer::DistributeMemory(const size_t minMemoryExpected)
{
  size_t blockVectorMemory =
      std::min(m_availableMemory, m_defaultMemoryPerBlock);

  if (blockVectorMemory >= minMemoryExpected)
  {
    //use the distributed tuple block memory
    m_availableMemory -= blockVectorMemory;
  }
  else
  {
    blockVectorMemory = 0;
  }

  return blockVectorMemory;
}

void ConcurrentTupleBuffer::ProvideTupleBlocks(
    TupleBlockVector &tupleBlockVector)
{
  if (!tupleBlockVector.IsInitialized())
  {
    return;
  }

  m_access.lock();
  int memory = tupleBlockVector.Shrink();

  if ((m_availableMemory + memory) > 0)
  {
    //necessary additional memory can be compensated by buffer memory
    m_availableMemory += memory;
    memory = 0;
  }

  //fill blocks into vector
  for (size_t i = 0; i < m_numPartitions; i++)
  {
    TupleBlockPtr tupleBlock = NULL;

    tupleBlockVector.PullTupleBlock(tupleBlock, i);

    if (memory < 0)
    {
      m_availableMemory -= m_settings.MinMemoryPerTupleVector;
      TupleBlockPtr oldMemoryTupleBlock = tupleBlock;
      tupleBlock = m_blockPool.TransformToPersistentTupleBlock(
          tupleBlock, m_settings.MinMemoryPerTupleVector);
      memory += tupleBlock->UsedMemorySize();
      DeallocateTupleBlock(oldMemoryTupleBlock);
    }

    if (tupleBlock->IsEmpty())
    {
      m_blockPool.DestroyTupleBlock(tupleBlock);
    }
    else
    {
      m_numTupleBlocks++;
      m_tupleBlockMatrix[i]->push(tupleBlock);
    }
  }
  m_access.unlock();
}

bool ConcurrentTupleBuffer::ConsumeTupleBlockByPartition(
    TupleBlockPtr &tupleBlock, const size_t partitionIdx)
{
  m_access.lock();
  bool returnValue = false;
  if (!m_tupleBlockMatrix[partitionIdx]->empty())
  {
    tupleBlock = m_tupleBlockMatrix[partitionIdx]->front();
    m_tupleBlockMatrix[partitionIdx]->pop();
    m_numTupleBlocks--;
    returnValue = true;
  }
  else
  {
    if (m_writerDirectory.empty())
    {
      //If no writer is available and the queue is empty,
      //then return an unique block that indicates that no
      //more blocks are expected
      tupleBlock = m_blockPool.GetEndOfDataStreamBlock();
      returnValue = true;
    }
    else
    {
      tupleBlock = NULL;
    }
  }
  m_access.unlock();

  return returnValue;
}

bool ConcurrentTupleBuffer::ConsumeNextTupleBlock(TupleBlockPtr &tupleBlock)
{
  m_access.lock();
  int partitionIndex = -1;
  size_t maxBlockCount = 0;

  for (size_t i = 0; i < m_numPartitions; i++)
  {
    TupleBlockQueuePtr queue = m_tupleBlockMatrix[i];
    size_t size = queue->size();

    if (maxBlockCount <= size)
    {
      partitionIndex = i;
      maxBlockCount = size;
    }
  }
  bool returnValue = ConsumeTupleBlockByPartition(tupleBlock, partitionIndex);

  m_access.unlock();

  return returnValue;
}

void ConcurrentTupleBuffer::DeallocateTupleBlock(TupleBlockPtr &tupleBlock)
{
  //deallocate memory of block
  m_access.lock();
  m_availableMemory += tupleBlock->UsedMemorySize();

  //recycle tuple block or delete it
  m_blockPool.DestroyTupleBlock(tupleBlock);

  m_access.unlock();
  tupleBlock = NULL;
}

void ConcurrentTupleBuffer::RemoveWriter(ConcurrentTupleBufferWriter *writer)
{
  m_access.lock();
  assert(m_writerDirectory.erase(writer));
  m_access.unlock();
}

void ConcurrentTupleBuffer::RemoveReader(ConcurrentTupleBufferReader *reader)
{
  m_access.lock();
  assert(m_readerDirectory.erase(reader) > 0);
  m_access.unlock();
}

} // namespace parthread
