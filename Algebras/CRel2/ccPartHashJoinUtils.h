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

December 07, 2017

Author: Nicolas Napp

\tableofcontents

1 Header File: ccPartHashJoinUtils.h

This file provides the implementation of the template class ~HashJoinState~, which is used by the ~ccPartHashJoin~ operator in file ~ccPartHashJoin.cpp~. It encapsulates the actual ~partitioned hash-join~ of two sets of argument tuple blocks. Its public interface comprises three functions:

\begin{itemize}
\item ~nextTBlock()~,
\item ~newBatR()~, and 
\item ~newBatS()~.
\end{itemize}

The function ~nextTBlock()~ performs the actual hash-join and returns the next result tuple block. ~newBatR()~ and ~newBatS()~ force an instance of this class to re-create and partition the ~binary relation~ for R and S, respectively.

The class ~HashJoinState~ inherits from the ~non-template~ class ~HashJoinStateBase~, which simplifies using it. The field ~hashJoinState~ in class ~LocalInfo~ (see file ~ccPartHashJoin.cpp~) is of the super type ~HashJoinStateBase~ and can thus store a reference to an instance of class ~HashJoinState~ with any template parameter, i.e. with binary tuples of any size.

A class ~MemBlock~ is used to manage blocks of memory for the hash-join. A memory block can hold several buckets, which makes allocating and deallocating it more efficient.

There is also a support function ~createPartBAT()~ for extracting the relevant data from the tuple blocks and for storing them in ~temporary binary relations~, which are then partitioned on the lower ~radixBits~ of the field ~hashvalue~. A binary relation consists of an array of ~binary tuples~ storing the extracted hash-values of the join-attributes and the references to the actual tuples.

The function ~nextTBlock()~ of class ~HashJoinState~ hash-joins both binary relations on the basis of the ~hash values~ of the join attributes. When a matching binary tuple pair has been found, the actual tuples are retrieved and the join attributes are compared directly. If the match can be confirmed, then the tuples are concatenated and appended to the result relation.

Once a result tuple block has reached its specified size in MiB, the function ~nextTBlock()~ interrupts and returns the result tuple block immediately. On the next call it resumes the hash-join exactly where it has left off.

*/
#ifndef CCPARTHASHJOINUTILS_H_
#define CCPARTHASHJOINUTILS_H_

/*
1.1 Imports

*/

#include <vector>
#include "BinaryTuple.h"
#include "Algebras/CRel/TBlock.h"
#include "CRel2Debug.h"
//#include <boost/interprocess/mapped_region.hpp>

#ifdef CREL2DEBUG
#include "CRel2Utils.h"
#endif
#define  LOOP_FUSION

using namespace CRelAlgebra;
using std::vector;

namespace CRel2Algebra {

/*
1.1 Function ~createPartBAT()~

This function receives a set of tuple blocks and creates a binary relation in the form of an array of binary tuples. Each binary tuple stores the ~hash-value~ of the join-attribute and a reference to the actual tuple. The binary relation is then partitioned - or sorted - on the lower ~radixBits~ of its field ~hashvalue~ in ~passes~ number of iterations using a hash-algorithm.

*/

template<typename binaryTuple>
binaryTuple* createPartBAT(const vector<TBlock*> &tBlockVector,
    const uint64_t joinIndex, const uint64_t &numTuples,
    const size_t& bitsTupleRef, uint64_t &radixBits, uint64_t &passes)
{

/*
New memory is allocated for the temporary binary relation.

*/
  // Allocate new memory for BAT. May throw bad_alloc exception
  binaryTuple* BAT = new binaryTuple[numTuples];

/*
1.1.1 Loop Fusion

~Loop-Fusion~ is a technique to reduce the number of times a loop is executed. Without loop-fusion the ~binary relation~ is first build by iterating over the join-attribute arrays of all tuple blocks. Later we iterate over the binary relation in order to hash its values. This requires two iterations. If ~loop-fusion~ is defined, then building the binary relation is omitted. The binary relation will be build later in the ~cleanup~ stage during the partitioning phase saving one iteration.

If ~loop fusion~ is defined, then the following code for building the binary relation will be ignored. The binary relation will be built later during the first pass of the partitioning phase.

*/

#ifndef LOOP_FUSION
  uint64_t tupleCounter = 0;
  // TBlock count starts at 1. Reference of 0 needed for partitioning.
  uint64_t tblockNum = 1;

  // Iterate over all loaded tuples and build BAT.
  while (tblockNum <= tBlockVector.size())
  {
    TBlockIterator tBlockIterator = tBlockVector[tblockNum - 1]->GetIterator();

    uint64_t row = 0;

    while (tBlockIterator.IsValid())
    {
      const TBlockEntry &tuple = tBlockIterator.Get();

      // Store TBlock and Tuple numbers as reference.
      BAT[tupleCounter].tupleref = (tblockNum << bitsTupleRef) | row;

      // Store hash value.
      BAT[tupleCounter].hashvalue = tuple[joinIndex].GetHash();

      ++tupleCounter;
      ++row;
      tBlockIterator.MoveToNext();
    }

    ++tblockNum;
  }

  Assert(tupleCounter == numTuples,
      std::runtime_error("tupleCounter != numTuples."));
#endif

/*
1.1.1 Partitioning Phase

This phase partitions the ~binary relation~ on the lower ~radixBits~ in ~passes~ number of iterations.

*/

  if (passes == 0 || radixBits == 0)
  {
    return BAT; // Nothing to do...
  }

  const size_t cacheLineSize = 64; // Cache line size in Bytes.

  Assert((cacheLineSize % sizeof(binaryTuple) == 0),
      std::runtime_error(
          "Cache line size not a multiple of binaryTuple size."))

/*
Determine the size of the temporary memory for hashing. The size of each bucket is a multiple of a cache-line.

*/

  uint64_t maxBuckets =
      (radixBits % passes) ?
          1ULL << (radixBits / passes + 1) : 1ULL << (radixBits / passes);

  size_t tuplesPerBucket = numTuples / maxBuckets + 2;
  // always >=2, 1 accounts for decimals cut off, 
  // 1 needed for reference to overflow bucket.

  // #tupels fitting in temporary memory allocated for hashing.
  uint64_t numTuplesMem = tuplesPerBucket * maxBuckets;

/*
Temporary memory is allocated for hashing and aligned to cache-lines. This also aligns all buckets as the size of a bucket is a multiple of a cache-line.
The memory block is chosen large enough to accommodate overflow containers.

*/

  size_t extraMem = cacheLineSize / sizeof(binaryTuple);

  // Allocate temporary memory. May throw bad_alloc exception
  binaryTuple* tmpBAT0 = new binaryTuple[2 * numTuplesMem + extraMem];
  binaryTuple* tmpBAT = tmpBAT0;

  // Aligning: find beginning of a cache-line.
  for (size_t i = 0; i < extraMem; ++i)
  {
    if (((long) tmpBAT % cacheLineSize))
    {
      ++tmpBAT;
    } else
    {
      break;
    }
  }

/*
Loop over all ~passes~. The tuples are partitioned in several ~passes~ on the lower ~radixBits~ of the field ~hashvalue~.

*/

  // Current number of bits used in this pass.
  size_t currentBits = radixBits / passes;
  // Distribute the remaining bits after ~incBits~ passes.
  size_t incBits = passes - (radixBits % passes);
  // Number of bits used to bit-shift the ~hashvalue~ while hashing.
  size_t hashShift = radixBits;
  //Array storing the next free slot for each bucket.
  binaryTuple** nextSlot = new binaryTuple*[maxBuckets];
  // Number of current partitions
  uint64_t numPart = 1;
  // Needed to determine the boundary of a partition.
  size_t partShift = 0;


  for (size_t pass = 0; pass < passes; ++pass)
  {
    if (pass == incBits)
    {
      ++currentBits; // Distribute remaining bits.
    }
    if (currentBits == 0)
    {
      continue; // One bucket only...
    }

    uint64_t numBuckets = 1ULL << currentBits;
    uint64_t bucketSize = numTuplesMem / numBuckets;
    uint64_t hashMask = numBuckets - 1;
    hashShift -= currentBits;

    uint64_t partMask = (numPart - 1) << partShift;
    uint64_t tupleCounter = 0;

/*
Loop over all current partitions.
Initially all tuples are in one large partition. Each pass increases the number of partitions by a factor of $2^{currentBits}$.

*/

    for (uint64_t part = 0; part < numPart; ++part)
    {

      uint64_t beginCurrentPartition;

      if (tupleCounter < numTuples) // Avoid overflow of ~bat~ array.
      {
        beginCurrentPartition = tupleCounter;
      } else
      {
        break; // nothing to do...
      }

      // Initialize nextSlot[] for all current buckets.
      for (size_t i = 0; i < numBuckets; ++i)
      {
        nextSlot[i] = tmpBAT + i * bucketSize;
      }

      // Set first overflow bucket.
      binaryTuple* nextOverflowBucket = tmpBAT + numTuplesMem;

/*
~Hashing~: distribute all tuples in current partition over buckets.

If ~loop-fusion~ is defined, then, during the first pass, the hash-values are read straight from the tuple blocks and hashed over the buckets. There, they are stored together with their tuple reference. The binary relation will be filled later during the ~cleanup~ stage.

*/

#ifdef LOOP_FUSION
    if (pass == 0)
    {
      // TBlock count starts at 1. Reference of 0 needed for partitioning.
      uint64_t tblockNum = 1;

      while (tblockNum <= tBlockVector.size())
      {
        TBlockIterator tBlockIterator = 
          tBlockVector[tblockNum - 1]->GetIterator();

        uint64_t row = 0;

        while (tBlockIterator.IsValid())
        {
          const TBlockEntry &tuple = tBlockIterator.Get();
          uint64_t hashvalue = tuple[joinIndex].GetHash();

          uint64_t bucket = ((hashvalue >> hashShift) & hashMask) % numBuckets;
          if ((nextSlot[bucket] + 1 - tmpBAT) % bucketSize == 0) // overflow
          {
            nextSlot[bucket]->tupleref = (nextOverflowBucket - tmpBAT)
            / bucketSize;
            nextSlot[bucket] = nextOverflowBucket;
            nextOverflowBucket += bucketSize;
          }

/*
The tuple reference is calculated and stored in a bucket together with the hash-value. The tuple reference consists of the tuple block number in the higher bits and the row number in the lower bits. This is done with a ~bit shift~ and a logical ~or~.

*/

            nextSlot[bucket]->tupleref = (tblockNum << bitsTupleRef) | row;
            nextSlot[bucket]->hashvalue = hashvalue;

            ++nextSlot[bucket];
            ++tupleCounter;
            ++row;
            tBlockIterator.MoveToNext();
          }

          ++tblockNum;
        }

        Assert(tupleCounter == numTuples,
            std::runtime_error("tupleCounter != numTuples."));

      } else
#endif

/*
In later passes the binary tuples are read straight from the binary relation.

*/

      {
        uint64_t partition = part << partShift;

        while ((tupleCounter < numTuples)
            && ((BAT[tupleCounter].hashvalue & partMask) == partition))
        {
          uint64_t bucket = ((BAT[tupleCounter].hashvalue >> hashShift)
              & hashMask) % numBuckets;
          if ((nextSlot[bucket] + 1 - tmpBAT) % bucketSize == 0) // overflow
          {
            nextSlot[bucket]->tupleref = (nextOverflowBucket - tmpBAT)
                / bucketSize;
            nextSlot[bucket] = nextOverflowBucket;
            nextOverflowBucket += bucketSize;
          }
          nextSlot[bucket]->tupleref = BAT[tupleCounter].tupleref;
          nextSlot[bucket]->hashvalue = BAT[tupleCounter].hashvalue;

          ++nextSlot[bucket];
          ++tupleCounter;
        } // End while-loop over all tuples in current partition.
      }

/*
~Cleanup~: copy partitioned tuples back to the ~binary relation~ and clear buckets simultaneously in order to avoid further cache-misses.

*/

      uint64_t tuplePos = beginCurrentPartition;
      for (uint64_t bucket = 0; bucket < numBuckets; ++bucket)
      {
        binaryTuple* nextTuple = tmpBAT + bucket * bucketSize;
        while (nextTuple->tupleref) // Loop until ~tupleref~ is 0.
        {
          if (((nextTuple + 1 - tmpBAT) % bucketSize) == 0)
          {
            binaryTuple* tmp = nextTuple;
            nextTuple = tmpBAT + nextTuple->tupleref * bucketSize;
            tmp->tupleref = 0;
          }
          BAT[tuplePos].tupleref = nextTuple->tupleref;
          BAT[tuplePos].hashvalue = nextTuple->hashvalue;

          nextTuple->tupleref = 0;
          nextTuple->hashvalue = 0;

          ++nextTuple;
          ++tuplePos;
        }

      }  // End for-loop (cleanup)

    } // End for-loop over all current partitions

    Assert((tupleCounter == numTuples),
        std::runtime_error("Error! Not all tuples have been hashed..."))

    // Set number of partitions for next pass.
    numPart = numPart << currentBits;
    // Hash bit shift of current pass equals partition bit shift for next pass.
    partShift = hashShift;

  } // End for-loop over passes

  delete[] nextSlot;
  delete[] tmpBAT0;

  return BAT;
}


/*
1.1 Class ~MemBlock~

Class ~MemBlock~ is used to manage blocks of memory.
A memory block can hold several hash buckets, which makes allocating and deallocating it
more efficient.

*/

template<typename binaryTuple>
class MemBlock
{
public:
  MemBlock(const uint64_t & numTuples)
  {

    const size_t cacheLineSize = 64;
    size_t extraMem = cacheLineSize / sizeof(binaryTuple);

    // Allocate temporary memory. May throw bad_alloc exception
    beginBAT = new binaryTuple[numTuples + extraMem];
    beginAlignedBAT = beginBAT;

    // Aligning: find beginning of a cache-line.
    for (size_t i = 0; i < extraMem; ++i)
    {
      if (((long) beginAlignedBAT % cacheLineSize))
      {
        ++beginAlignedBAT;
      } else
      {
        break;
      }
    }
  }

  ~MemBlock()
  {
    delete[] beginBAT;
  }

  binaryTuple* getAlignedBAT()
  {
    return beginAlignedBAT;
  }

private:
  binaryTuple* beginBAT;        // beginning of memory block
  binaryTuple* beginAlignedBAT; // beginning of aligned memory block

}; // End class MemBlock

/*
1.1 Class ~HashJoinStateBase~

The class ~HashJoinStateBase~ is the super class of ~HashJoinState~ and simplifies using it.

The field ~hashJoinState~ in class ~LocalInfo~ (see file ~ccPartHashJoin.cpp~) is of the super type ~HashJoinStateBase~ and can thus store a reference to an instance of class ~HashJoinState~ with any template parameter, i.e. with binary tuples of any size.

*/

class HashJoinStateBase
{
public:
  virtual ~HashJoinStateBase()
  {
  }

  virtual void newBatR()
  {
  }

  virtual void newBatS()
  {
  }

  virtual bool nextTBlock(TBlock* const resultTBlock)
  {
    return true;
  }
};

/*
1.1 Class ~HashJoinState~

The template class ~HashJoinState~ encapsulates the actual ~partitioned hash-join~ of two sets of argument tuple blocks. Its public interface comprises three functions: ~nextTBlock~, ~newBatR~, and ~newBatS~.

*/

template<typename binaryTuple>
class HashJoinState : public HashJoinStateBase
{

/*
1.1.1 Constructor

The constructor initializes all fields, determines the number of ~passes~ and ~radixBits~, creates and partitions the ~binary relations~ of R and S, and sets the number and size of hash buckets.

*/

public:
  HashJoinState(
      const vector<TBlock*> &tBlockVectorR_,
      const vector<TBlock*> &tBlockVectorS_,
      const uint64_t &joinIndexR_,
      const uint64_t &joinIndexS_,
      const uint64_t& numTuplesR_,
      const uint64_t& numTuplesS_,
      const uint64_t bitsTupleRef_,
      const uint64_t& resultTBlockSize_) :

      tBlockVectorR ( tBlockVectorR_ ),
      tBlockVectorS ( tBlockVectorS_ ),
      joinIndexR ( joinIndexR_ ),
      joinIndexS ( joinIndexS_ ),
      numTuplesR ( numTuplesR_ ),
      numTuplesS ( numTuplesS_ ),
      bitsTupleRef ( bitsTupleRef_),
      resultTBlockSize ( resultTBlockSize_ ),
      numColumnsR ( tBlockVectorR[0]->GetColumnCount() ),
      numColumnsS ( tBlockVectorS[0]->GetColumnCount() ),
      newTuple ( new AttrArrayEntry[numColumnsR  + numColumnsS] ),
      rowMask ( (1ULL << bitsTupleRef) - 1 )
  {

/*
Determine the number of ~radixBits~ and ~passes~.
The optimal values were determined for my computer hardware and an expected range of argument tuples as part of my BSc thesis. A different hardware may require different values for ~radixBits~ and ~passes~.

*/

    passes = 1;

    radixBits = 0;
    while (true)
    {
      if ((1ULL << ++radixBits) > numTuplesS) // S is hashed.
        break;
    }
    if (radixBits > 8)
    {
      radixBits -= 8;
    } else
    {
      radixBits = 1;
    }


    partMask = (1ULL << radixBits) - 1;

    // Create and partition binary Relation.
    batR = createPartBAT<binaryTuple>
      (tBlockVectorR, joinIndexR, numTuplesR, bitsTupleRef, radixBits, passes);
    batS = createPartBAT<binaryTuple>
      (tBlockVectorS, joinIndexS, numTuplesS, bitsTupleRef, radixBits, passes);

    numBuckets = numTuplesS / (1ULL << radixBits);
    bucketSize = cacheLineSize / sizeof(binaryTuple);

    if (numBuckets < 1) // We need at least one bucket.
    {
      numBuckets = 1;
    } else if (numBuckets > maxNumBuckets) // Cap amount of memory used.
    {
      numBuckets = maxNumBuckets;
    }

    // We need at least two tuples to fit into one bucket.
    if (bucketSize < 2)
    {
      bucketSize = 2;
    }

    // #tupels fitting in temporary memory allocated for hashing.
    numTuplesMem = bucketSize * numBuckets;

    //Array storing the next free slot for each bucket.
    nextSlot = new binaryTuple*[numBuckets];
    bucketsPos = new uint16_t[numBuckets];

    tupleCounterS = 0;
    tupleCounterR = 0;
    currentPartition = 0;

    bucketPos = 1;
    nextTuple = nullptr;
    resume = false;

  }

/*
1.1.1 Destructor

The destructor clears all temporary memory, such as allocated memory blocks for hashing, the binary relations, and other data structures, which were needed for the hashing.

*/

  ~HashJoinState()
  {
    for (MemBlock<binaryTuple>* memBlock : mem)
    {
      delete memBlock;
    }
    mem.clear();
    // std::vector<MemBlock<binaryTuple>*>().swap(mem);

    delete[] batR;
    delete[] batS;

    delete[] nextSlot;
    delete[] bucketsPos;
    delete[] newTuple;
  }

/*
1.1.1 Function ~newBatR()~

This function deletes an old ~binary relation~ for R and creates a new partitioned ~binary relation~. This is needed when the old tuple blocks of R have been replaced with new ones.

*/

  void newBatR()
  {
    delete[] batR;
    batR = createPartBAT<binaryTuple>
      (tBlockVectorR, joinIndexR, numTuplesR, bitsTupleRef, radixBits, passes);
  }

/*
1.1.1 Function ~newBatS()~

This function deletes an old ~binary relation~ for S and creates a new partitioned ~binary relation~. This is needed when the old tuple blocks of S have been replaced with new ones.

*/

  void newBatS()
  {
    delete[] batS;
    batS = createPartBAT<binaryTuple>
      (tBlockVectorS, joinIndexS, numTuplesS, bitsTupleRef, radixBits, passes);
  }


/*
1.1.1 Function ~nextTBlock()~

This function takes a result tuple block as a parameter and appends to it all tuples which are joined together. As soon as the result tuple block has reached its specified size, it interrupts and returns the complete result tuple block. On the next call it resumes exactly where it has left off.

*/

  bool nextTBlock(TBlock* const resultTBlock)
  {

    while (resume
        || ((tupleCounterS < numTuplesS) && (tupleCounterR < numTuplesR)))
    {

      if (!resume)
      {
/*
If R has no tuples in the current partition, skip to the next one...

*/

        if ((batR[tupleCounterR].hashvalue & partMask) != currentPartition)
        {
          while ((tupleCounterS < numTuplesS)
           && ((batS[tupleCounterS].hashvalue & partMask) == currentPartition))
          {
            ++tupleCounterS;
          }
          ++currentPartition;
          continue;
        }


/*
If S has no tuples in the current partition, skip to the next one...

*/

        if ((batS[tupleCounterS].hashvalue & partMask) != currentPartition)
        {
          while ((tupleCounterR < numTuplesR)
           && ((batR[tupleCounterR].hashvalue & partMask) == currentPartition))
          {
            ++tupleCounterR;
          }
          ++currentPartition;
          continue;
        }

/*
Both, R and S, have tuples in the current partition, thus find all matches...

*/

        // Allocate first memory block.
        mem.push_back(new MemBlock<binaryTuple>(numTuplesMem));

        // ~nextSlot[]~ points to the next free slot of the respective bucket.
        binaryTuple* firstBucket = mem[0]->getAlignedBAT();
        for (size_t i = 0; i < numBuckets; ++i)
        {
          nextSlot[i] = firstBucket + i * bucketSize;
          bucketsPos[i] = 1;
        }

        // Defines the next overflow bucket.
        uint64_t nextOverflowMemBlock = 0;
        uint64_t nextOverflowBucket = numBuckets;

/*
Hash current partition of ~binary relation S~

*/

        while ((tupleCounterS < numTuplesS)
            && ((batS[tupleCounterS].hashvalue & partMask) == currentPartition))
        {
          uint64_t bucket = 
            (batS[tupleCounterS].hashvalue >> radixBits) % numBuckets;

          // bucket overflown
          if (bucketsPos[bucket] == bucketSize)
          {
            // current memory block overflown
            if (nextOverflowBucket == numBuckets) 
            {
              mem.push_back(new MemBlock<binaryTuple>(numTuplesMem));
              ++nextOverflowMemBlock;
              nextOverflowBucket = 0;
            }
            nextSlot[bucket]->tupleref = nextOverflowMemBlock;
            nextSlot[bucket]->hashvalue = nextOverflowBucket;

            nextSlot[bucket] = mem[nextOverflowMemBlock]->getAlignedBAT()
                + nextOverflowBucket * bucketSize;

            bucketsPos[bucket] = 1;
            ++nextOverflowBucket;
          }

          nextSlot[bucket]->tupleref = batS[tupleCounterS].tupleref;
          nextSlot[bucket]->hashvalue = batS[tupleCounterS].hashvalue;

          ++nextSlot[bucket];
          ++bucketsPos[bucket];

          ++tupleCounterS;
        }
      } // End if(!resume)...

/*
Now hash current partition of ~binary relation R~ and find all matches with ~binary relation S~.

*/

      while ((tupleCounterR < numTuplesR)
          && ((batR[tupleCounterR].hashvalue & partMask) == currentPartition))
      {
        if (!resume)
        {
          // Find bucket.
          uint64_t bucket = 
            (batR[tupleCounterR].hashvalue >> radixBits) % numBuckets;
          // Find beginning of bucket.
          nextTuple = mem[0]->getAlignedBAT()
              + bucket * bucketSize;
          bucketPos = 1;
        } else
        {
          resume = false;
        }
        // Search bucket for likely matches.
        while (nextTuple->tupleref)
        {
          if (bucketPos == bucketSize) // bucket overflown
          {
            nextTuple = mem[nextTuple->tupleref]->getAlignedBAT()
                + nextTuple->hashvalue * bucketSize;
            bucketPos = 1;
          }
          if (batR[tupleCounterR].hashvalue == nextTuple->hashvalue)
          {

/*
Matching hash values from ~binary relation R~ and ~binary relation S~ have been found. Now extract ~tuple block~ and ~row~ numbers and find the actual tuples (tuple blocks are encoded beginning at 1, thus tuple block 0 has number 1).

*/

            const TBlockEntry &tupleR = TBlockEntry(
              tBlockVectorR[(batR[tupleCounterR].tupleref >> bitsTupleRef) - 1],
              batR[tupleCounterR].tupleref & rowMask);

            const TBlockEntry &tupleS = TBlockEntry(
              tBlockVectorS[(nextTuple->tupleref >> bitsTupleRef) - 1],
              nextTuple->tupleref & rowMask);

/*
If their join attributes match, then build a new result tuple and append it to the result tuple block.

*/

            if (tupleR[joinIndexR] == tupleS[joinIndexS])
            {
              for (uint64_t i = 0; i < numColumnsR; ++i)
              {
                newTuple[i] = tupleR[i];
              }

              for (uint64_t i = 0; i < numColumnsS; ++i)
              {
                newTuple[numColumnsR + i] = tupleS[i];
              }

              resultTBlock->Append(newTuple);

/*
If the result tuple block has reached its target size, then return to class ~LocalInfo~. There, the result tuple block is passed on in the tuple block stream. When the next tuple block is requested, the the hashing will be resumed.

*/

              if (resultTBlock->GetSize() > resultTBlockSize)
              {
                ++bucketPos;
                ++nextTuple;
                resume = true;
                return false; // Join of ~batR~ and ~batS~ not yet finished.
              }
            }
          }

          ++bucketPos;
          ++nextTuple;

        } // End while-loop (searching for matches for R)

        ++tupleCounterR;
      } // End while-loop (hashing of ~batR~)

/*
~Cleanup~: free memory.

*/

      for (MemBlock<binaryTuple>* memBlock : mem)
      {
        delete memBlock;
      }
      mem.clear();
      // std::vector<MemBlock<binaryTuple>*>().swap(mem);

      ++currentPartition;
    } // End while-loop over all partitions

    // Reset all counters for next use.
    tupleCounterS = 0;
    tupleCounterR = 0;
    currentPartition = 0;

    bucketPos = 1;
    nextTuple = nullptr;
    resume = false;

    return true; // Hash-Join of ~batR~ and ~batS~ finished.

  } /* End nextTBlock */

private:
  const size_t maxNumBuckets = 4LL * 1024LL * 1024LL; // Caps mem used.
  const size_t cacheLineSize = 64; // Cache line size in Bytes.

  const vector<TBlock*> &tBlockVectorR;
  const vector<TBlock*> &tBlockVectorS;
  const uint64_t &joinIndexR;
  const uint64_t &joinIndexS;
  binaryTuple* batR;
  binaryTuple* batS;
  const uint64_t& numTuplesR;
  const uint64_t& numTuplesS;
  const size_t bitsTupleRef;
  const uint64_t& resultTBlockSize;

  const size_t numColumnsR;
  const size_t numColumnsS;
  AttrArrayEntry* const newTuple; // AttrArrayEntry* const newTuple;
  const uint64_t rowMask;
  uint64_t partMask;
  uint64_t radixBits;
  uint64_t passes;
  size_t numBuckets;
  size_t bucketSize;
  uint64_t numTuplesMem;
  std::vector<MemBlock<binaryTuple>*> mem;
  binaryTuple** nextSlot; //Array storing the next free slot for each bucket.
  uint16_t* bucketsPos;
  uint64_t tupleCounterS;
  uint64_t tupleCounterR;
  uint64_t currentPartition;
  uint16_t bucketPos;
  binaryTuple* nextTuple;
  bool resume;

};
/* End class ccHashJoinPartitions */

} /* namespace CRel2Algebra */

#endif /* CCPARTHASHJOINUTILS_H_ */
