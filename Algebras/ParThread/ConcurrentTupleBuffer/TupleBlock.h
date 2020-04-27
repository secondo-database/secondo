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

1 Header File: TupleBlock

September 2019, Fischer Thomas

1.1 Overview

1.2 Imports

*/

#ifndef SECONDO_PARTHREAD_TUPLE_BLOCK_H
#define SECONDO_PARTHREAD_TUPLE_BLOCK_H

#include "../../Relation-C++/RelationAlgebra.h"
#include "ThreadsafeQueue.h"

#include "DataPartitioner.h"
#include <mutex>

namespace parthread
{

typedef Tuple *TuplePtr;

class ConcurrentTupleBuffer;

struct ConcurrentTupleBufferSettings
{
    //Initial tuple block size
    size_t InitialNumberOfTuplesPerBlock;

    // Number of tuple blocks per partition a queue can store before
    // a lock-based allocation takes place
    size_t InitialQueueCapacity;

    //The partitioning method to separate tuples to different queues
    IDataPartitionerPtr DataPartitioner;

    //If the internal memory runs out of space, tuples are swapped to
    //a file per tuple block of the given size
    size_t PersistentBufferSizePerBlock;

    //The size in bytes of the complete tuple buffer
    size_t TotalBufferSizeInBytes;

    //Devision factor used to distribute the available memory to the
    //tuple blocks.
    size_t MemoryDistributionFactor;

    //the minimum memory which is necessary to create a new set of tuple blocks
    size_t MinMemoryPerTupleVector;

    //tuple blocks to recycle
    size_t TupleBlocksToRecycle;
};

enum class TupleBlockType
{
    MemoryTupleBlock = 1,
    PersistentTupleBlock,
    EndOfDataStreamBlock,
    TupleBlockTypeSize = EndOfDataStreamBlock
};

class ITupleBlock
{

public: //methods
    virtual ~ITupleBlock(){};

    //removes all tuple references and resets the datastructure to initial state
    //without deallocating memory
    virtual void Initialize() = 0;

    //shrink memory and set finalized state
    virtual void Finalize() = 0;

    virtual TupleBlockType Type() const = 0;

    //returns the current free size of bytes of the reserved memory
    virtual int UsedMemorySize() const = 0;

    virtual size_t Count() const = 0;

    virtual bool IsFinalized() const = 0;

    virtual bool IsEmpty() const = 0;

    virtual void Push(Tuple *tuple) = 0;

    virtual bool TryPull(Tuple *&tuple) = 0;
};

//pointer to a single tuple block
typedef ITupleBlock *TupleBlockPtr;

//pointer to a vector/array of blocks
typedef TupleBlockPtr *TupleBlockVectorPtr;

//typedef boost::lockfree::queue<TupleBlockPtr> TupleBlockQueue;
typedef std::queue<TupleBlockPtr> TupleBlockQueue;
typedef TupleBlockQueue *TupleBlockQueuePtr;

class MemoryTupleBlock : public ITupleBlock
{

public: //methods
    MemoryTupleBlock(const size_t initialNoOfTuples)
        : m_lastReadIdx(0), m_lastWriteIdx(0), 
          m_initialNoOfTuples(initialNoOfTuples),
          m_currentMemorySize(0), m_finalized(false)
    {
        assert(initialNoOfTuples > 0);
        m_tupleArray.resize(initialNoOfTuples);
    };

    ~MemoryTupleBlock() override{};

    TupleBlockType Type() const override
    {
        return TupleBlockType::MemoryTupleBlock;
    }

    //removes all tuple references (if any exists) and resets the 
    //datastructure to initial state without deallocating memory
    void Initialize() override
    {
        m_lastReadIdx = 0;
        m_lastWriteIdx = 0;
        m_currentMemorySize = 0;
        m_finalized = false;
    }

    //Finalize state indicates that no tuples can be added
    //to the block anymore.
    void Finalize() override
    {
        m_finalized = true;
    }

    //returns the current free size of bytes of the reserved memory
    int UsedMemorySize() const override
    {
        return m_currentMemorySize;
    }

    size_t Count() const override
    {
        return m_tupleArray.size();
    }

    bool IsFinalized() const override
    {
        return m_finalized;
    }

    bool IsEmpty() const override
    {
        return m_tupleArray.empty() || m_lastReadIdx >= m_lastWriteIdx;
    }

    void Push(Tuple *tuple) override
    {
        assert(!m_finalized);

        //controlled reallocation of the vector array
        if (m_tupleArray.size() == m_tupleArray.capacity())
        {
            m_tupleArray.resize(m_tupleArray.size() + m_initialNoOfTuples);
        }

        m_tupleArray[m_lastWriteIdx] = tuple;
        m_lastWriteIdx++;
        m_currentMemorySize += tuple->GetMemSize();
    }

    bool TryPull(Tuple *&tuple) override
    {
        Finalize();
        if (IsEmpty())
        {
            tuple = NULL;
            return false;
        }
        tuple = m_tupleArray[m_lastReadIdx];
        m_lastReadIdx++;
        return true;
    }

private: //member
    size_t m_lastReadIdx;
    size_t m_lastWriteIdx;
    size_t m_initialNoOfTuples;
    size_t m_currentMemorySize;
    bool m_finalized;
    std::vector<TuplePtr> m_tupleArray;
};

class PersistentTupleBlock : public ITupleBlock
{

public: //methods
    PersistentTupleBlock(const size_t maxDiskBufferSize)
        : m_maxDiskBufferSize(maxDiskBufferSize), m_fileIterator(NULL),
          m_finalized(false), m_diskBuffer(NULL){};

    ~PersistentTupleBlock()
    {
        if (m_diskBuffer != NULL)
        {
            m_diskBuffer->Close();
            delete m_diskBuffer;
        }
        if (m_fileIterator != NULL)
        {
            delete m_fileIterator;
            m_fileIterator = NULL;
        }
    }

    TupleBlockType Type() const override
    {
        return TupleBlockType::PersistentTupleBlock;
    }

    void Initialize() override
    {
        if (m_diskBuffer != NULL)
        {
            m_diskBuffer->Close();
            delete m_diskBuffer;
            m_diskBuffer = NULL;
        }
        if (m_fileIterator != NULL)
        {
            delete m_fileIterator;
            m_fileIterator = NULL;
        }

        m_finalized = false;
    }

    //shrink memory and set finalized state
    void Finalize() override
    {
        m_finalized = true;
    }

    //returns the current free size of bytes of the reserved memory
    int UsedMemorySize() const override
    {
        return m_maxDiskBufferSize;
    }

    size_t Count() const override
    {
        if (m_diskBuffer != NULL)
        {
            return m_diskBuffer->GetNoTuples();
        }
        return 0;
    }

    bool IsFinalized() const override
    {
        return m_finalized;
    }

    bool IsEmpty() const override
    {
        return Count() == 0;
    }

    void Push(Tuple *tuple) override
    {
        assert(!m_finalized);
        if (m_diskBuffer == NULL)
        {
            m_diskBuffer = new TupleFile(tuple->GetTupleType(), 
                                         m_maxDiskBufferSize);
        }

        m_diskBuffer->Append(tuple);
    }

    bool TryPull(Tuple *&tuple) override
    {
        if (!IsEmpty())
        {
            if (m_fileIterator == NULL)
            {
                m_fileIterator = m_diskBuffer->MakeScan();
            }

            if (m_fileIterator->MoreTuples())
            {
                tuple = m_fileIterator->GetNextTuple();
                return true;
            }
        }

        tuple = NULL;
        return false;
    }

private: //member
    int m_maxDiskBufferSize;
    TupleFileIterator *m_fileIterator;
    bool m_finalized;
    TupleFile *m_diskBuffer;
};

class EndOfDataStreamBlock : public ITupleBlock
{

public: //methods
    EndOfDataStreamBlock(){};

    ~EndOfDataStreamBlock() override{};

    TupleBlockType Type() const override
    {
        return TupleBlockType::EndOfDataStreamBlock;
    }

    //removes all tuple references and resets the datastructure
    //to initial state without deallocating memory
    void Initialize() override
    {
    }

    //shrink memory and set finalized state
    void Finalize() override
    {
    }

    //returns the current free size of bytes of the reserved memory
    int UsedMemorySize() const override
    {
        return 0;
    }

    size_t Count() const override
    {
        return 0;
    }

    bool IsFinalized() const override
    {
        return true;
    }

    bool IsEmpty() const override
    {
        return true;
    }

    void Push(Tuple *tuple) override
    {
    }

    bool TryPull(Tuple *&tuple) override
    {
        tuple = NULL;
        return false;
    }
};

//not threadsave
class TupleBlockPool
{
public: //types
public: //methods
    TupleBlockPool(const ConcurrentTupleBufferSettings &settings)
        : m_settings(settings), 
          m_recyclingQueue(settings.TupleBlocksToRecycle), 
          m_endOfDataStreamBlock()
    {
    }

    ~TupleBlockPool()
    {
        TupleBlockPtr newTupleBlock = NULL;
        while (m_recyclingQueue.TryPop(newTupleBlock))
        {
            DestroyTupleBlock(newTupleBlock, false);
        }
    }

    TupleBlockPtr TransformToPersistentTupleBlock(TupleBlockPtr tupleBlock, 
                                                  const size_t fileBuffer)
    {
        TupleBlockPtr persistentBlock = new PersistentTupleBlock(fileBuffer);
        persistentBlock->Initialize();

        Tuple *tuple;
        while (tupleBlock->TryPull(tuple))
        {
            persistentBlock->Push(tuple);
        }

        persistentBlock->Finalize();

        return persistentBlock;
    }

    TupleBlockPtr GetEndOfDataStreamBlock()
    {
        return &m_endOfDataStreamBlock;
    }

    //threadsave
    TupleBlockPtr GetMemoryTupleBlock()
    {
        TupleBlockPtr newTupleBlock = NULL;

        if (m_recyclingQueue.TryPop(newTupleBlock))
        {
            return newTupleBlock;
        }
        else
        {
            return new MemoryTupleBlock(
                m_settings.InitialNumberOfTuplesPerBlock);
        }
    }

    //threadsave
    void DestroyTupleBlock(const TupleBlockPtr tupleBlock, bool recycle = true)
    {
        assert(tupleBlock != NULL);

        switch (tupleBlock->Type())
        {
        case TupleBlockType::EndOfDataStreamBlock:
            //this block type is a unique static object
            return;
            break;
        case TupleBlockType::MemoryTupleBlock:
            //if the recycling queue is running out capacity, 
            //then delete the block
            if (recycle &&
                m_recyclingQueue.TryPush(tupleBlock))
            {
                return;
            }
            delete tupleBlock;
            break;
        default:
            delete tupleBlock;
            break;
        }
    }

private: //member
    const ConcurrentTupleBufferSettings &m_settings;
    size_t m_maxRecycledTupleBlocks;
    ThreadsafeQueue<TupleBlockPtr> m_recyclingQueue;
    EndOfDataStreamBlock m_endOfDataStreamBlock;
};

class TupleBlockVector
{
public: //methods
    TupleBlockVector(const size_t numPartitions)
        : m_numPartitions(numPartitions), m_allocatedMemorySize(0),
          m_currentUsedMemorySize(0)
    {
        m_tupleBlockArray = new TupleBlockPtr[numPartitions];

        for (size_t i = 0; i < numPartitions; i++)
        {
            m_tupleBlockArray[i] = NULL;
        }
    };

    ~TupleBlockVector()
    {
        delete[] m_tupleBlockArray;
    }

    bool IsInitialized() const
    {
        return m_allocatedMemorySize != 0;
    }

    int Shrink()
    {
        int freeMemory = FreeMemorySize();
        m_allocatedMemorySize = m_currentUsedMemorySize;
        return freeMemory;
    }

    bool IsEmpty() const
    {
        return UsedMemorySize() == 0;
    }

    //returns the current free size of bytes of the reserved memory
    //the value can be negative
    int FreeMemorySize() const
    {
        return m_allocatedMemorySize - m_currentUsedMemorySize;
    }

    //the summarized memory of all tuples in this block. Can exceed the
    //allocated memory
    int UsedMemorySize() const
    {
        return m_currentUsedMemorySize;
    }

    //the initial allocated memory
    int AllocatedMemory() const
    {
        return m_allocatedMemorySize;
    }

    const TupleBlockPtr BlockAtPartition(const size_t partitionIdx) const
    {
        assert(partitionIdx < m_numPartitions);

        TupleBlockPtr block = m_tupleBlockArray[partitionIdx];

        return block;
    }

    void AddTupleToPartition(Tuple *tuple, const size_t partitionIdx)
    {
        assert(partitionIdx < m_numPartitions);
        assert(m_tupleBlockArray[partitionIdx] != NULL);

        m_tupleBlockArray[partitionIdx]->Push(tuple);
        m_currentUsedMemorySize += tuple->GetMemSize();
    }

    void PullTupleBlock(TupleBlockPtr &tupleBlock, const size_t partitionIdx)
    {
        assert(partitionIdx < m_numPartitions);
        assert(tupleBlock == NULL);

        tupleBlock = m_tupleBlockArray[partitionIdx];
        m_tupleBlockArray[partitionIdx] = NULL;

        m_currentUsedMemorySize -= tupleBlock->UsedMemorySize();
        m_allocatedMemorySize = std::max(m_allocatedMemorySize - 
                                         tupleBlock->UsedMemorySize(), 0);

        tupleBlock->Finalize();
    }

    void SetTupleBlockByPartition(TupleBlockPtr tupleBlock, 
                                  const size_t partitionIdx, 
                                  const int memory)
    {
        assert(partitionIdx < m_numPartitions);
        assert(tupleBlock != NULL);

        m_tupleBlockArray[partitionIdx] = tupleBlock;
        m_tupleBlockArray[partitionIdx]->Initialize();

        m_allocatedMemorySize = memory;
    }

private: //member
    size_t m_numPartitions;
    int m_allocatedMemorySize;
    int m_currentUsedMemorySize;

    TupleBlockVectorPtr m_tupleBlockArray;
};

} // namespace parthread
#endif //SECONDO_PARTHREAD_TUPLE_BLOCK_H
