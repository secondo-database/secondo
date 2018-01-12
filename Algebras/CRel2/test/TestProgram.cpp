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

December 07, 2017

Author: Nicolas Napp

\tableofcontents

1 Test Program

This program is part of the author's BSc thesis. It was written for two purposes: the first purpose is to help verify the results of two core implementations of the ~cache-conscious partitioned hash-join operator~, which was developed as part of the thesis. One of the core implementations partitions a ~binary relation~ and the other one performs the actual hash-join.

As the partitions must be ~sorted~ on the lower ~radixBits~ of the field ~hashvalue~, the result can be verified easily by a simple scan of the partitioned relation. The results of the hash-join, on the other hand, are verified by comparing them to results of two different implementations of join-algorithms: the ~partitioned nested loop join~ and the ~simple nested loop join~. Results are accepted if all three join-implementations concur on the number of tuples, which have been joined.

The second purpose is to gauge the performance of these two core implementations and find optimal parameters, i.e. the optimal number of partitions (or ~radixBits~) and the optimal number of ~passes~ used to calculate the partitions. The performance is evaluated on the basis of the number of cache-misses and the runtime.

This program is run independently of SECONDO.

1.1 Usage

The program requires an installation of the performance application programming interface (PAPI,  see ~http://icl.cs.utk.edu/papi/~). For compilation you need to provide a pathname to PAPI's header file and library. For example, in my case I ran:

~g++ -std=c++14 -O3 -I /usr/local/include/ -o TestProgram.o~

~TestProgram.cpp /usr/local/lib/libpapi.a~

1.1 Imports

*/

#include "BinaryTuple.h"
#include "CRel2Debug.h"

#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <papi.h>
#include <random>
#include <string>
#include <sys/time.h>
#include <vector>

using namespace CRel2Algebra;

/*
1.1 Preparations

a) Choose the size of the binary tuple:

\begin{itemize}
\item binaryTupleSmall (4 Byte),
\item binaryTupleMedium (8 Byte), or 
\item binaryTupleLarge (16 Byte).
\end{itemize}

*/

//typedef binaryTupleSmall binaryTuple;
typedef binaryTupleMedium binaryTuple;
//typedef binaryTupleLarge binaryTuple;

/*
b) Choose exactly one of three test-cases. You can run tests on either the partitioning or the hash-join phase alone or on both together:

*/

#define TESTALL
//#define TESTPARTITIONING
//#define TESTHASHJOIN

/*
c) Choose the range of the tests:

~expTuplesBegin~ and ~expTuplesEnd~ determine the range of tuples to be analyzed and ~expTuplesStep~ sets the step in form of a factor:

$number-of-tuples = 2^{expTuplesBegin}, 2^{expTuplesBegin + expTuplesStep}, . . ., 2^{expTuplesEnd}$

*/

const uint64_t expTuplesBegin = 17;
const uint64_t expTuplesEnd = 26;
const uint64_t expTuplesStep = 3;

/*
~maxPasses~ sets the number of passes to be examined.

*/

const uint64_t maxPasses = 4;

/*
~HINT~: If you are interested in ~performance~ tests only, then you may want to consider commenting out the function calls ~partNestedLoopJoin()~ and ~simpleNestedLoopJoin()~ in part ~main~, as the program will run much faster for large numbers of tuples. The subsequent comparison of the results will have to be commented out as well.

1.1 Support Functions

A function for PAPI error messages is defined.

*/

void handle_error(int retval)
{
  printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
  exit(1);
}

/*
This function displays the binary notation of an unsigned 64 bit integer value.

*/

void displayBinaryNotation(uint64_t num)
{
  int bit = 8 * sizeof(num); // equals 64
  unsigned long mask = 1ULL << (bit - 1);

  for (int i = (bit - 1); i >= 0; --i)
  {
    if ((i != (bit - 1)) && !((i+1) % 8)) std::cout << '.';
    // bitwise "and"
    // result > 0 (true) if num has bit set at the same position as mask
    (num & mask) ? std::cout << 1 : std::cout << 0;

    // right shift
    mask >>= 1;
  }
}

/*
This function returns the difference between two points in time in seconds.

*/

double difftimeval(struct timeval &t1, struct timeval &t2)
{
  return (double) (t2.tv_sec - t1.tv_sec)
      + (double) (t2.tv_usec - t1.tv_usec) / 1000000;
}

/*
1.1 Function: Partitioning the ~Binary Relation~

The function ~partitionBAT()~ contains one of the two core implementations of the ~cache-conscious partitioned hash-join operator~, which are analyzed here. Although its code can be run independently of SECONDO's Crel-Algebra, the actual algorithm is the same.

It partitions the tuples of a ~binary relation~ by sorting them on the lower ~radixBits~ of the field ~hashvalue~ in ~passes~ number of iterations. The sorting is performed using a hashing algorithm.

The function receives references to a ~binary relation~, i.e. an array of binary tuples aligned in memory, the number of tuples it contains, the number of ~passes~, and the number of ~radixBits~.
The final number of partitions depends on the number of ~radixBits~:

\hspace{1.00cm}$number-of-partitions = 2^{radixBits}$

~Precondition~: The ~binary tuples~ must be initialized with valid values. In particular, the value for ~tupleref~ must not be zero.

~Postcondition~: The input array is partitioned - or sorted - on the lower ~radixBits~ of the field ~hashvalue~ in an ascending order.

*/

template<typename binaryTuple>
void partitionBAT(binaryTuple* const &bat, const uint64_t &numTuples,
    const size_t &passes, const size_t &bits)
{

  const size_t cacheLineSize = 64; // Cache line size in Bytes.
  const uint64_t minMem = 1;

  if (passes == 0 || bits == 0)
  {
    return; // Nothing to do...
  }

  Assert((cacheLineSize % sizeof(binaryTuple) == 0),
    std::runtime_error( "Cache line size not a multiple of binaryTuple size."))

/*
Determine the size of temporary memory for hashing.
The size of each bucket is a multiple of a cache-line.

*/

//#define MINBUCKETCACHELINE // sets the minimum bucket size to a cache line.
  uint64_t maxBuckets =
      (bits % passes) ? 1ULL << (bits / passes + 1) : 1ULL << (bits / passes);

#ifdef MINBUCKETCACHELINE
  size_t cacheLinesPerBucket = numTuples * sizeof(binaryTuple) / maxBuckets
      / cacheLineSize + 1; // always >= 1, thus bucket >= 1 x cacheLine

  // #tupels fitting in temporary memory allocated for hashing.
  uint64_t numTuplesMem = cacheLinesPerBucket * maxBuckets * cacheLineSize
      / sizeof(binaryTuple);
#else
  size_t tuplesPerBucket = numTuples / maxBuckets + 2;
  // always >=2, 1 accounts for decimals cut off, 1 needed for reference to 
  // overflow bucket.

  // #tupels fitting in temporary memory allocated for hashing.
  uint64_t numTuplesMem = tuplesPerBucket * maxBuckets;
#endif

  // Assert that newly allocated memory is larger than the minimum memory.
  while (numTuplesMem * sizeof(binaryTuple) < minMem)
  {
    numTuplesMem *= 2;
  }

/*
Temporary memory is allocated for hashing and aligned to cache-lines.
This also aligns all buckets as the size of a bucket is a multiple of a
cache-line.
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
Loop over all ~passes~.
The tuples are partitioned in several ~passes~ using the lower ~radixBits~ of the field ~hashvalue~.

*/

  // Current number of bits used in this pass.
  size_t currentBits = bits / passes;
  // Distribute the remaining bits after ~incBits~ passes.
  size_t incBits = passes - (bits % passes);
  // Number of bits used to bit-shift the ~hashvalue~ while hashing.
  size_t hashShift = bits;
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
~Hashing~: distribute all tuples in the current partition over the buckets.

*/

      uint64_t partition = part << partShift;

      while ((tupleCounter < numTuples)
          && ((bat[tupleCounter].hashvalue & partMask) == partition))
      {

        uint64_t bucket =
            ((bat[tupleCounter].hashvalue >> hashShift) & hashMask)
                % numBuckets;
        if ((nextSlot[bucket] + 1 - tmpBAT) % bucketSize == 0) // overflow
        {
          nextSlot[bucket]->tupleref = (nextOverflowBucket - tmpBAT)
              / bucketSize;
          nextSlot[bucket] = nextOverflowBucket;
          nextOverflowBucket += bucketSize;
        }
        nextSlot[bucket]->tupleref = bat[tupleCounter].tupleref;
        nextSlot[bucket]->hashvalue = bat[tupleCounter].hashvalue;

        ++nextSlot[bucket];
        ++tupleCounter;
      } // End while-loop over all tuples in current partition.

/*
~Cleanup~: copy the partitioned tuples back to the ~binary relation~ and clear the buckets ~simultaneously~ in order to avoid further cache-misses.

*/

      uint64_t tuplePos = beginCurrentPartition;
      for (uint64_t bucket = 0; bucket < numBuckets; ++bucket)
      {
        binaryTuple* nextTuple = tmpBAT + bucket * bucketSize;
        while (nextTuple->tupleref)
        {
          if (((nextTuple + 1 - tmpBAT) % bucketSize) == 0)
          {
            binaryTuple* tmp = nextTuple;
            nextTuple = tmpBAT + nextTuple->tupleref * bucketSize;
            tmp->tupleref = 0;
          }
          bat[tuplePos].tupleref = nextTuple->tupleref;
          bat[tuplePos].hashvalue = nextTuple->hashvalue;

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
}  // End partitionBAT

/*
1.1 Function: Partitioned Hash-Join

The following code performs a partitioned hash-join on two partitioned ~binary relations~. It contains a class ~MemBlock~, which manages blocks of memory for the hash-join. The actual hash-join is implemented in a function called ~partHashjoin()~. This piece of code is the second of the two core implementations of the ~cache-conscious partitioned hash-join operator~, which are analyzed here. Although it can be run independently of SECONDO's CRel-Algebra, the actual algorithm is the same as the one in the operator.

1.1.1 Memory Management

The class ~MemBlock~ is used to manage blocks of memory. A memory block can hold several buckets, which makes allocating and deallocating it more efficient.

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
  binaryTuple* beginBAT;        // beginn memory block
  binaryTuple* beginAlignedBAT;  // beginn aligned memory block

}; // End class MemBlock

/*
1.1.1 Partitioned Hash-Join

~Description~: The function ~partHashjoin()~ receives two binary relations, which are both partitioned on the lower ~radixBits~, and calculates their hash-join on the field ~hashvalue~. It iterates over all partitions and joins partition i of R and partition i of S.

~Precondition~: The ~binary Tuples~ must be initialized with valid values. In particular the value for ~tupleref~ must not be zero.

~Postcondition~: The relation ~batResult~ contains the join of the two argument relations. However, this function is used for testing the hash-join, only. Therefore, if the array overflows, then the following result tuples are stored again at the beginning of the array thus overwriting previous results. The field ~tupleCounterRes~, however, stores the correct number of tuples joined together.

There are two goals: the first is to test if the implementation returns a ~correct~ result by counting the number of tuples, which are joined together, and the second is to evaluate the ~performance~ of the implementation by counting the number of cache-misses and by measuring its runtime.

*/

template<typename binaryTuple>
void partHashjoin(binaryTuple* const & batResult, binaryTuple* const & batR,
    binaryTuple* const & batS, const uint64_t& numTuplesR,
    const uint64_t& numTuplesS, const size_t& radixBits,
    uint64_t& tupleCounterRes)
{

  // Caps amount of memory used.
  const uint64_t maxNumBuckets = 64ULL * 1024ULL * 1024ULL;
  const uint64_t cacheLineSize = 64;

  uint64_t numBuckets = numTuplesS / (1ULL << radixBits);
  // # Tuples fitting into one cache-line
  uint64_t bucketSize = cacheLineSize / sizeof(binaryTuple);
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

  tupleCounterRes = 0;

  // #tupels fitting in temporary memory allocated for hashing.
  uint64_t numTuplesMem = bucketSize * numBuckets;

  //Array storing the next free slot for each bucket.
  binaryTuple** nextSlot = new binaryTuple*[numBuckets];
  uint16_t* bucketsPos = new uint16_t[numBuckets];

  uint64_t tupleCounterS = 0;
  uint64_t tupleCounterR = 0;
  uint64_t partMask = (1ULL << radixBits) - 1;
  uint64_t currentPartition = 0;

/*
Keep iterating until either R or S or both have no more tuples left.

*/

  while ((tupleCounterS < numTuplesS) && (tupleCounterR < numTuplesR))
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

    std::vector<MemBlock<binaryTuple>*> mem;
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
Hash current partition of ~batS~...

*/

    while ((tupleCounterS < numTuplesS)
        && ((batS[tupleCounterS].hashvalue & partMask) == currentPartition))
    {
      uint64_t bucket = (batS[tupleCounterS].hashvalue >> radixBits)
          % numBuckets;

      if (bucketsPos[bucket] == bucketSize) // bucket overflown
      {
        if (nextOverflowBucket == numBuckets) // current memory block overflown
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

/*
Finished hashing of ~batS~. Now hash current partition of ~batR~ and find all matches with ~batS~.

*/

    while ((tupleCounterR < numTuplesR)
        && ((batR[tupleCounterR].hashvalue & partMask) == currentPartition))
    {
      // Find bucket.
      uint64_t bucket = (batR[tupleCounterR].hashvalue >> radixBits)
          % numBuckets;
      // Find beginning of bucket.
      binaryTuple* nextTuple = mem[0]->getAlignedBAT() + bucket * bucketSize;
      uint16_t bucketPos = 1;
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
Store tuple references for R and S in ~batResult~. As the relation has a size of up to ~n x m~
the position is calculated $tupleCounterRes \% numTuplesS$, thus overwriting results at some point.
This is intended as we are interested in the amount of time it takes to ~write~ the results,
not the results themselves.

The field ~tupleCounterRes~, however, stores the correct number of tuples joined together.

*/

          batResult[tupleCounterRes % numTuplesS].tupleref =
              batR[tupleCounterR].tupleref;
          batResult[tupleCounterRes % numTuplesS].hashvalue =
              nextTuple->tupleref;

          ++tupleCounterRes;
        }

        ++bucketPos;
        ++nextTuple;
      } // End while-loop (searching for matches)

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

  delete[] nextSlot;
  delete[] bucketsPos;

} /* End partHashjoin */

/*
1.1 Function: Partitioned Nested Loop Join

The function ~partNestedLoopJoin()~ is a ~control~ implementation of a join-algorithm, which is used to verify the results of the join.

~Description~: The function ~partNestedLoopJoin()~ receives two binary relations, which are both partitioned on the lower ~radixBits~ number of bits, and calculates their hash-join on the field ~hashvalue~. It iterates over all partitions and joins partition i of R and partition i of S by performing a simple nested loop join.

~Precondition~: The ~binary Tuples~ must be initialized with valid values. In particular the value for ~tupleref~ must not be zero.

~Postcondition~: The relation ~batResult~ contains the join of the two argument relations. However, this function is used to count the number of ~hits~, only. Therefore, if the array overflows, then the following result tuples are stored again at the beginning of the array thus overwriting previous results.

The field ~tupleCounterRes~, however, stores the correct number of tuples joined together.

*/

template<typename binaryTuple>
void partNestedLoopJoin(
    binaryTuple* const & batResult,
    binaryTuple* const & batR, binaryTuple* const & batS,
    const uint64_t& numTuplesR, const uint64_t& numTuplesS, const size_t& bits,
    uint64_t& tupleCounterRes)
{

  tupleCounterRes = 0;

  uint64_t tupleCounterS = 0;
  uint64_t tupleCounterR = 0;
  uint64_t mask = (1ULL << bits) - 1;
  uint64_t currentPartition = 0;

/*
Iterate over all tuples in R and S.

*/

  while ((tupleCounterR < numTuplesR) && (tupleCounterS < numTuplesS))
  {

/*
If R has no tuples in the current partition, skip to the next one...

*/

    if ((batR[tupleCounterR].hashvalue & mask) != currentPartition)
    {
      while ((tupleCounterS < numTuplesS)
          && ((batS[tupleCounterS].hashvalue & mask) == currentPartition))
      {
        ++tupleCounterS;
      }
      ++currentPartition;
      continue;
    }

/*
If S has no tuples in the current partition, skip to the next one...

*/

    if ((batS[tupleCounterS].hashvalue & mask) != currentPartition)
    {
      while ((tupleCounterR < numTuplesR)
          && ((batR[tupleCounterR].hashvalue & mask) == currentPartition))
      {
        ++tupleCounterR;
      }
      ++currentPartition;
      continue;
    }

/*
Both, R and S, have tuples in the current partition, thus find all matches...

*/

    uint64_t beginPartS = tupleCounterS;
    while ((tupleCounterR < numTuplesR)
        && ((batR[tupleCounterR].hashvalue & mask) == currentPartition))
    {
      tupleCounterS = beginPartS;
      while ((tupleCounterS < numTuplesS)
          && ((batS[tupleCounterS].hashvalue & mask) == currentPartition))
      {
        if (batR[tupleCounterR].hashvalue == batS[tupleCounterS].hashvalue)
        {
          batResult[tupleCounterRes % numTuplesS].tupleref =
              batR[tupleCounterR].tupleref;
          batResult[tupleCounterRes % numTuplesS].hashvalue =
              batS[tupleCounterS].tupleref;
          ++tupleCounterRes;
        }
        ++tupleCounterS;
      }
      ++tupleCounterR;
    }
    ++currentPartition;
  }

} /* End partNestedLoopJoin */

/*
1.1 Function: Simple Nested Loop Join

The function ~simpleNestedLoopJoin()~ is a ~control~ implementation of a join-algorithm, which is used to verify the results of the join.

~Description~: The function ~simpleNestedLoopJoin()~ receives two binary relations and calculates the hash-join on the field ~hashvalue~. It iterates over all tuples and finds for each tuple in R all matches in S.

~Postcondition~: The relation ~batResult~ contains the join of the two argument relations. However, this function is used to count the number of ~hits~, only. Therefore, if the array overflows, then the following result tuples are stored again at the beginning of the array thus overwriting previous results.

The field ~tupleCounterRes~, however, stores the correct number of tuples joined together.

*/

template<typename binaryTuple>
void simpleNestedLoopJoin(
    binaryTuple* const & batResult,
    binaryTuple* const & batR, binaryTuple* const & batS,
    const uint64_t& numTuplesR, const uint64_t& numTuplesS, const size_t& bits,
    uint64_t& tupleCounterRes)
{

  tupleCounterRes = 0;

  for (uint64_t i = 0; i < numTuplesR; ++i)
  {
    for (uint64_t j = 0; j < numTuplesS; ++j)
    {
      if (batR[i].hashvalue == batS[j].hashvalue)
      {
        batResult[tupleCounterRes % numTuplesS].tupleref = batR[i].tupleref;
        batResult[tupleCounterRes % numTuplesS].hashvalue = batS[j].tupleref;
        ++tupleCounterRes;
      }
    }
  }
} /* End TestSimpleNestedLoopJoin */

/*
1.1 Main

*/

int main()
{

/*
Depending on the test-case a filename is defined for the test results.

*/

#ifdef TESTALL
  #undef TESTPARTITIONING
  #undef TESTHASHJOIN
  std::cout << "running TESTALL..." << std::endl;
  const std::string outFilename = "performanceAll.dat";

#elif defined TESTPARTITIONING
  #undef TESTALL
  #undef TESTHASHJOIN
  std::cout << "running TESTPARTITIONING..." << std::endl;
  const std::string outFilename = "performanceRadixCluster.dat";

#elif defined TESTHASHJOIN
  #undef TESTALL
  #undef TESTPARTITIONING
  std::cout << "running TESTHASHJOIN..." << std::endl;
  const std::string outFilename = "performanceHashJoin.dat";

#else
  std::cout << "no test case chosen! Aborting..." << std::endl;
#endif

/*
PAPI (performance application programming interface) event sets are defined. Total cache misses for L1, L2, and L3 caches and total cache accesses for L3 cache will be counted.

*/

  // Define PAPI events.
  const int numEvents = 4;
  long long int values[numEvents];
  int events[numEvents] = {PAPI_L1_TCM, PAPI_L2_TCM, PAPI_L3_TCA, PAPI_L3_TCM};
  //{ PAPI_L3_TCA, PAPI_L3_TCM, PAPI_TLB_DM };

/*
Results will be written to file for later analysis with gnuplot.

*/

  // Open output file for writing...
  std::ofstream outfile(outFilename);
  if (!outfile.good())
  {
    throw std::runtime_error(
        "Error! " + outFilename + " cannot be opened for writing...");
  }

  outfile << "#exp bits passes elapsedtime "
   << "PAPI_L1_TCM PAPI_L2_TCM PAPI_L3_TCA PAPI_L3_TCM PAPI_L3_TCM/PAPI_L3_TCA"
   << " num_hits\n";

  std::cout << "\nexp bits passes elapsedtime "
   << "PAPI_L1_TCM PAPI_L2_TCM PAPI_L3_TCA PAPI_L3_TCM PAPI_L3_TCM/PAPI_L3_TCA"
   << " num_hits\n";


/*
1.1.1 Test Range

Three parameters are analyzed: the number of tuples of the argument relations (~numTuples~), the number of ~passes~, and the number of ~radixBits~. The number of tuples is given by the exponent ~exp~, with $numTuples = 2^{exp}$. Both argument relations always have the same number of tuples.

*/

  for (size_t exp = expTuplesBegin; exp <= expTuplesEnd; exp += expTuplesStep)
  {

    for (size_t passes = 1; passes <= maxPasses; ++passes)
    {

      for (size_t bits = 0; bits <= exp; ++bits)
      {
        uint64_t numTuples = 1ULL << exp;
        // range set to numTuples => hit rate of approx. "1"
        uint64_t range = numTuples;

/*
While testing the performance only one counter is needed. However, all three are needed for testing.

*/

        // Tuple counters for different the join implementations.
        uint64_t numTuplesHashJoin = 0;
        uint64_t numTuplesPartNestedLoopJoin = 0;
        uint64_t numTuplesSimpleNestedLoopJoin = 0;

        struct timeval t1, t2;
        double elapsedtime;

        //Obtain a seed for the random number engine
        std::random_device rd;
        //Standard mersenne_twister_engine seeded with rd()
        std::mt19937 gen(rd());
        std::uniform_int_distribution<unsigned long long> dis(1, range);

/*
1.1.1 Partitioning Phase

The following code is run only, if the partitioning is analyzed.

*/
#ifdef TESTPARTITIONING

/*
Create a binary relation.

*/

        binaryTuple* bat = new binaryTuple[numTuples];

        for (uint64_t i = 0; i < numTuples; ++i)
        {
          /* Using dis to transform the random unsigned int generated
           by gen into an unsigned long long in [1, range] */

          // Required: "tupleref != 0"; needed for cleanup in partition phase.
          bat[i].tupleref = i + 1;
          bat[i].hashvalue = dis(gen);
        }

/*
Start / stop counters to gauge hardware events (cache misses and wall clock time), which occur while partitioning the newly created binary relation.

*/

        // Start counting hardware events
        if (PAPI_start_counters(events, numEvents) != PAPI_OK)
          handle_error(1);

        // Start time measurement
        gettimeofday(&t1, NULL);

        partitionBAT<binaryTuple>(bat, numTuples, passes, bits);

        // Stop time measurement
        gettimeofday(&t2, NULL);

        // Stop counting hardware events
        if (PAPI_stop_counters(values, numEvents) != PAPI_OK)
                  handle_error(1);

        elapsedtime = difftimeval(t1, t2);

/*
If in testing mode, the newly created partitions are checked for errors. The partitions must be ordered by the ~radixBits~, i.e. the lower bits.

*/

        // Test partitions for errors.
        uint64_t mask = (1ULL << bits) - 1;
        uint64_t last = bat[0].hashvalue & mask;
        for (uint64_t i = 1; i < numTuples; ++i)
        {
          // testing for errors...
          if ((bat[i].hashvalue & mask) < last)
          {
            throw std::runtime_error("partitioning incorrect...");
          }
          last = bat[i].hashvalue & mask;
        }
        delete[] bat;

#else

/*
1.1.1 Hash-Join Phase

This code is run only, if the hash-join is analyzed.

First the binary relations are created.

*/
        binaryTuple* batR = new binaryTuple[numTuples];
        binaryTuple* batS = new binaryTuple[numTuples];
        binaryTuple* batResult = new binaryTuple[numTuples];

        for (uint64_t i = 0; i < numTuples; ++i)
        {
          /* Using dis to transform the random unsigned int generated
           by gen into an unsigned long long in [1, range] */

          // Required: "tupleref != 0"; needed for cleanup in partition phase.
          batR[i].tupleref = i + 1;
          batR[i].hashvalue = dis(gen);
          batS[i].tupleref = i + 1;
          batS[i].hashvalue = dis(gen);
        }

/*
Depending on the test case the hardware performance counters and the clock are started before or after the binary relations have been partitioned.

*/

#ifdef TESTALL

        // Start counting hardware events
        if (PAPI_start_counters(events, numEvents) != PAPI_OK)
          handle_error(1);

        // Start time measurement
        gettimeofday(&t1, NULL);

#endif

        partitionBAT<binaryTuple>(batR, numTuples, passes, bits);
        partitionBAT<binaryTuple>(batS, numTuples, passes, bits);

#ifdef TESTHASHJOIN

        // Start counting hardware events
        if (PAPI_start_counters(events, numEvents) != PAPI_OK)
          handle_error(1);

        // Start time measurement
        gettimeofday(&t1, NULL);

#endif

/*
Performance tests are run on ~partHashJoin()~ only, as this implementation is used in the operator.

*/
        partHashjoin(batResult, batR, batS, numTuples, numTuples, bits,
            numTuplesHashJoin);

        // Stop time measurement
        gettimeofday(&t2, NULL);

        // Stop counting hardware events
        if (PAPI_stop_counters(values, numEvents) != PAPI_OK)
          handle_error(1);

        elapsedtime = difftimeval(t1, t2);

/*
Check that all join implementations calculate the same result. May be commented out for performance tests, as this will run much faster for large numbers of tuples. The subsequent comparison of the results will have to be commented out as well.

*/

        partNestedLoopJoin(batResult, batR, batS, numTuples, numTuples,
            bits, numTuplesPartNestedLoopJoin);
        simpleNestedLoopJoin(batResult, batR, batS, numTuples, numTuples, bits,
            numTuplesSimpleNestedLoopJoin);

        if ((numTuplesHashJoin != numTuplesPartNestedLoopJoin)
            || (numTuplesHashJoin != numTuplesSimpleNestedLoopJoin))
           {
             throw std::runtime_error("join incorrect...");
           }

        delete[] batR;
        delete[] batS;
        delete[] batResult;

#endif

        // Display results on screen.
        printf("\n%4lu", exp);
        printf("\t%4lu", bits);
        printf("\t%4lu", passes);
        printf("\t%3.6f", elapsedtime);
        for (int i = 0; i < numEvents; ++i)
        {
          printf("\t%10lld", values[i]);
        }
        printf("\t%3.6f", (double) values[3] / values[2]);
        printf("\t%7lu", numTuplesHashJoin);

/*
1.1.1 Writing Results to Disk

Results are written to disk for later analysis, e.g. with gnuplot.

*/

        // Write results to disk
        outfile << exp << ' ' << bits << ' ' << passes << ' ' << elapsedtime;
        for (int i = 0; i < numEvents; ++i)
        {
          outfile << ' ' << values[i]; // L1_TCM, L2_TCM, L3_TCA, L3_TCM
        }
        outfile << ' ' << (double) values[3] / values[2]; // L3: TCM / TCA
        outfile << ' ' << numTuplesHashJoin << std::endl;

      } // End for-loop bits

    } // End for-loop passes

  } // End for-loop numTuples

  outfile.close();
  std::cout << '\n';

}
