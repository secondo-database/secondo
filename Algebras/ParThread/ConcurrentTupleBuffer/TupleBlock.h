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

1 Header File: TupleBlock

September 2019, Fischer Thomas

1.1 Imports and protoypes

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


/*
1.2 ConcurrentTupleBufferSettings

This struct summarize a couple of settings used in the tuple buffer. Most of
them are read from secondos configuration file.

*/

struct ConcurrentTupleBufferSettings
{
    size_t InitialNumberOfTuplesPerBlock;
/*
Initial tuple block size. This setting is just used to initialize the vector 
of the block, the block can increase in size during execution. The writer 
controlls how many tuples are written to a single block, 

*/

    IDataPartitionerPtr DataPartitioner;
/*
The partitioning method to separate tuples to different queues

*/

    size_t TotalBufferSizeInBytes;
/*
The size in bytes available for the complete tuple buffer

*/

    size_t MemoryDistributionFactor;
/*
Division factor used to distribute the available memory to the tuple blocks.

*/

    size_t MinMemoryPerTupleVector;
/*
The minimum memory which is necessary to create a new set of tuple blocks.

*/

    size_t TupleBlocksToRecycle;
/*
The already consumed tuple blocks can be reused to avoid overhead through 
creation of memory tuple blocks. This variable set the maximum number of tuple 
blocks to be recycled. 

*/
};


/*
1.3 TupleBlockType and the ITupleBlock interface

It's possible to store different types of tuple blocks in the ConcurrentTupleBuffer.
They all share the ~ITupleBlock~-interface to store and get tuples from a block. 

A note on threadsavety: tuple blocks are intended to be used by one thread at a time.
Each writer and reader of the tuple buffer is assigned to one execution context entity
and processes the tuple blocks in sequential order. Therefore the methods of ~ITupleBlock~
are not threadsave.
 
*/
enum class TupleBlockType
{
    MemoryTupleBlock = 1,
/*
MemoryTupleBlocks store references of the tuple in memory.

*/

    PersistentTupleBlock,
/*
PersistentTupleBlocks swaps memory to a relation file.

*/

    EndOfDataStreamBlock,
/*
EndOfDataStreamBlock does not store tuples, it is just a dummy to indicate that
the tuple stream is finished and there is no data to be expected from the 
tuple buffer anymore.

*/

    TupleBlockTypeSize = EndOfDataStreamBlock
};

class ITupleBlock
{

public: //methods
    virtual ~ITupleBlock(){};

    virtual void Initialize() = 0;
/*
Removes all existing tuple references and resets the datastructure to 
initial state without deallocating memory

*/

    virtual void Finalize() = 0;
/*
Finalize state indicates that no tuples can be added to the block anymore.

*/

    virtual TupleBlockType Type() const = 0;
/*
Returns the implementation type using this interface.

*/


    virtual int UsedMemorySize() const = 0;
/*
Returns the current used memory in bytes as sum of all tuple sizes.

*/

    virtual size_t Count() const = 0;
/*
Returns the number of stored tuples in this block

*/

    virtual bool IsFinalized() const = 0;
/*
Indicates if the block is finalized and can't store more tuples.

*/

    virtual bool IsEmpty() const = 0;
/*
Returns ~true~ if the block contains no tuples.

*/

    virtual void Push(Tuple *tuple) = 0;
/*
Adds a tuple at the end of the internal queue.

*/

    virtual bool TryPull(Tuple *&tuple) = 0;
/*
Returns ~true~ if the block is not empty and has a tuple block to return to the
out-parameter. 

*/
};

/*
1.4 Typedefs related to pointer and arrays of tuple blocks

*/

typedef ITupleBlock *TupleBlockPtr;

typedef TupleBlockPtr *TupleBlockVectorPtr;

typedef std::queue<TupleBlockPtr> TupleBlockQueue;
typedef TupleBlockQueue *TupleBlockQueuePtr;


/*
1.5 MemoryTupleBlock

Stores tuple references to a internal vector. The initialNoOfTuples parameter
of the constructor sets the initial size of the vector.

*/
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

    void Initialize() override
    {
        m_lastReadIdx = 0;
        m_lastWriteIdx = 0;
        m_currentMemorySize = 0;
        m_finalized = false;
    }

    void Finalize() override
    {
        m_finalized = true;
    }

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

/*
1.6 PersistentTupleBlock

Stores the tuples to a relation file. The parameter ~maxDiskBufferSize~ passes 
the size of the reserved memory used for buffering the data before writing it to 
the file. This is also the value returned by the method ~UsedMemorySize~.

The temporary file is removed in the destructor, so a recycling of the block is not
possible.

*/
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


/*
1.7 EndOfDataStreamBlock

The ~EndOfDataStreamBlock~ is just a dummy block indicating that the end of the
data stream is reached and no more data can be fetched from the ~ConcurrentDataBuffer~.

The tuple block is finalized and empty by default. Therfore it's not possible to 
push and pull tuples from this block. 

*/
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

/*
1.8 TupleBlockPool

The ~TupleBlockPool~ is responsible to create and destroy tuple blocks. It acts
as factory for the different types of tuple blocks and decides if a tuple block
can be destroyed (~DestroyTupleBlock~). 

The method ~TransformToPersistentTupleBlock~ allows a memory tuple block to be
swapped to a persistent tuple block in case no more memory is available.  

All method (except constructor and desctructor) are threadsave.

*/
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
            //this block type is an unique static object
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


/*
1.9 TupleBlockVector

The ~TupleBlockVector~ collects a tuple block for each partition of the 
~ConcurrentTupleBuffer~. If a writer successfully allocates memory, it gets a 
~TupleBlockVector~ with empty tuple blocks and a maximum size of the allocated 
memory. This memory is shared between all tuple blocks.

If the allocated memory of this ~TupleBlockVector~ is running out, it's still 
possible to add new tuples. The missing allocating memory is compensated when
returning the tuple blocks to the ~ConcurrentTupleBuffer~.

*/
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
/*
Sets the allocated memory to the size of used memory for the tuples stored in 
the blocks of this vector. This method is called by the ~ConcurrentTupleBuffer~.
The return value can be posititve or negative. In the first case it increases 
the tuple buffers pool of free memory, in the other case the buffer must reallocate 
memory to store the block vector or swap the blocks to persistent tuple blocks. 

*/

    bool IsEmpty() const
    {
        return UsedMemorySize() == 0;
    }

    int FreeMemorySize() const
    {
        return m_allocatedMemorySize - m_currentUsedMemorySize;
    }
/*
Returns the current free size of bytes of the reserved memory
the value can be negative

*/

    int UsedMemorySize() const
    {
        return m_currentUsedMemorySize;
    }

    int AllocatedMemory() const
    {
        return m_allocatedMemorySize;
    }

/*
~UsedMemorySize~ returns the summarized memory of all tuples in this block. 
This can exceed the allocated memory, which is set when calling 
~SetTupleBlockByPartition~.

*/

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
/*
These methods are used to fill the vector with records and to access the
different tuple blocks.

*/

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
/*
Adds new empty tuple blocks to the vector and sets the allocated memory.

*/

private: //member
    size_t m_numPartitions;
    int m_allocatedMemorySize;
    int m_currentUsedMemorySize;

    TupleBlockVectorPtr m_tupleBlockArray;
};

} // namespace parthread
#endif //SECONDO_PARTHREAD_TUPLE_BLOCK_H
