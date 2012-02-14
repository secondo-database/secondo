/*
----
This file is part of SECONDO.

Copyright (C) 2009, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[_] [\_]
//[x] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]
//[>] [\ensuremath{>}]
//[<] [\ensuremath{<}]


1 Header File HybridHashJoin.h

June 2009, Sven Jungnickel. Initial version

2 Overview

This file contains the declaration of all classes and functions that
implement the new hash-join operator ~hybridhashjoin~.

3 Includes

*/

#ifndef HYBRIDHASHJOIN_H_
#define HYBRIDHASHJOIN_H_

#include "HashJoin.h"
#include "StopWatch.h"

/*
4 Class ~HybridHashJoinProgressLocalInfo~

All classes and functions have been put into the namespace
~extrel2~ to avoid name conflicts with existing implementations and
to make it later easier to replace existing operators.

This class contains the progress information for the implementation
of the ~hybridhashjoin~ operator. As the operator implementation
needs to provide some more progress information during
partitioning phases this class has been derived from
~ProgressLocalInfo~.

*/

namespace extrel2
{
class HybridHashJoinProgressLocalInfo : public ProgressLocalInfo
{
  public:

    HybridHashJoinProgressLocalInfo( Supplier s );
/*
The constructor.

*/

    inline void CheckProgressSinceLastResult()
    {
      if ( ++(tuplesProcessedSinceLastResult) % 1000 )
      {
        qp->CheckProgress();
      }
    }
/*
Initiate a progress message if a certain amount of tuples
has been processed since the last result tuple.

*/

    int CalcProgress( ProgressInfo& p1,
                      ProgressInfo& p2,
                      ProgressInfo* pRes,
                      Supplier s);
/*
Calculates the progress for the hybrid hash join algorithm.

The following constants were assumed from operator ~hashjoin~.

*/

    static const double uHashJoin;
/*
Milliseconds per probe tuple.

*/
    static const double vHashJoin;
/*
Milliseconds per tuple right.

*/

    static const double wHashJoin;
/*
Milliseconds per tuple returned.

The following constants were determined experimentally. See file
ConstantsHybridHashJoin.txt for details.

*/

    static const double t_read;
/*
Milliseconds per tuple read from disc.

*/

    static const double t_write;
/*
Milliseconds per tuple written to disc.

*/

    static const double t_probe;
/*
Milliseconds per probe tuple.

*/

    static const double t_hash;
/*
Milliseconds per tuple hashed.

*/

    static const double t_result;
/*
Milliseconds per tuple returned.

*/

    ostream& Print(ostream& os);
/*
Print to stream ~os~. This function is used
for debugging purposes.

*/

    size_t maxOperatorMemory;
/*
Maximum available operator memory in bytes.

*/

    size_t tuplesProcessedSinceLastResult;
/*
Number of tuples processed since the last result tuple
has been produced.

*/

    PartitionManagerProgressInfo streamA;
/*
Progress information of partitions from stream A.

*/

    PartitionManagerProgressInfo streamB;
/*
Progress information of partitions from stream B.

*/

  private:

    void calcProgressStd( ProgressInfo& p1,
                            ProgressInfo& p2,
                            ProgressInfo* pRes,
                            Supplier s );
/*
Calculates the progress for the standard hash-join algorithm.

*/

    void calcProgressHybrid( ProgressInfo& p1,
                               ProgressInfo& p2,
                               ProgressInfo* pRes,
                               Supplier s );
/*
Calculates the progress for the hybrid hash-join algorithm making use
of the partition information in ~pinfoA~ and ~pinfoB~.

*/


    bool traceMode;
/*
Flag which decides if tracing information is generated on standard output

*/

};

/*
5 Class ~HybridHashJoinAlgorithm~

*/

class HybridHashJoinAlgorithm
{
  public:

    HybridHashJoinAlgorithm( Word wstreamA,
                             int indexAttrA,
                             Word wstreamB,
                             int indexAttrB,
                             size_t buckets,
                             Supplier s,
                             HybridHashJoinProgressLocalInfo* p,
                             size_t partitions = UINT_MAX,
                             size_t maxMemSize = UINT_MAX,
                             size_t ioBufferSize = UINT_MAX );
/*
The constructor. Consumes all tuples of the tuple stream
~stream~ immediately into

*/

    ~HybridHashJoinAlgorithm();
/*
The destructor. Frees all resources of the algorithm.

*/

    Tuple* NextResultTuple();
/*
Returns the pointer of the next result tuple in sort order. If all tuples
have been processed the method returns 0.

*/

    void UpdateProgressInfo();
/*
Updates the progress information. Copies the progress information
collected within the partitions to the HybridHashJoinProgressLocalInfo
instance. Called from value mapping function, whenever a new
REQUESTPROGRESS message is received.

*/

  private:

    void setMemory(size_t maxMemory, Supplier s);
/*
Sets the usable main memory for the operator in bytes. If ~maxMemory~
has value ~UINT\_MAX~ the usable main memory is requested from the
query processor.

*/

    void setIoBuffer(size_t maxMemory);
/*
Sets the I/O buffer to ~maxMemory~ bytes. If ~maxMemory~ has value
~UINT\_MAX~ the default I/O buffer size is used (system's page size).

*/

    void setBuckets(size_t maxMemory, size_t n);
/*
Sets the number of buckets to ~n~. The method verifies that ~n~
does not exceed the upper bound of maxMemory/1024 and the lower bound of
~MIN\_BUCKETS~. Also the number of buckets is rounded to the next divisible
of two.

*/

    void setPartitions(size_t n);
/*
Sets the number of partitions to ~n~. The method verifies that ~n~
does not exceed the upper bound of nBuckets/2 and the lower bound of
~MIN\_PARTITIONS~. If ~n~ is set to ~UINT\_MAX~ the method calculates
a default value for the number of partitions from the number of buckets
nBucket. ~setPartitions~ must be called after ~setBuckets~ has been called.

*/

    Tuple* nextTupleA();
/*
Returns the next tuple from stream A. If the stream has been processed
completely 0 is returned.

*/

    Tuple* nextTupleB();
/*
Returns the next tuple from stream B. If the stream has been processed
completely 0 is returned.

*/

    Tuple* partitionA();
/*
Partitions stream A and probes tuples that fall into partition 0 against
the hash table in memory (partition 0 of stream B). The method returns
the pointer to the next result tuple of partition 0. If 0 is returned
stream A has been partitioned completely.

*/

    void partitionB();
/*
Partitions stream B completely.

*/

    Tuple* processPartitions();
/*
Processes partitions 1 to N-1 and returns the next result tuple. If 0
is returned all partitions have been processed.

*/

    static const size_t MIN_BUCKETS = 4;
/*
Minimum number of buckets.

*/

    static const size_t MIN_PARTITIONS = 2;
/*
Minimum number of partitions.

*/
    static const size_t MAX_PARTITIONS = 256;
/*
Maximum number of partitions.

*/

    static const size_t MIN_USER_DEF_MEMORY = 1024;
/*
Minimum amount of user defined memory for the operator.

*/


    Word streamA;
/*
Address of stream A.

*/

    Word streamB;
/*
Address of stream B.

*/

    int attrIndexA;
/*
Attribute index of the join attribute for stream A.

*/

    int attrIndexB;
/*
Attribute index of the join attribute for stream B.

*/

    size_t MAX_MEMORY;
/*
Maximum memory available for the sort operation [bytes]

*/

    size_t usedMemory;
/*
Used memory in bytes

*/

    TupleType* resultTupleType;
/*
The tuple type for result tuples.

*/

    TupleType* tupleTypeA;
/*
The tuple type of stream A

*/

    TupleType* tupleTypeB;
/*
The tuple type of stream A

*/

    size_t nBuckets;
/*
The number of buckets for a hash table. The minimum number of buckets is 3.
The maximum number of buckets is limited by the available operator memory
Op$_{max}$. The maximum number of tuples is Op$_{max}$ / 1024.

*/

    size_t nPartitions;
/*
The number of partitions. The minimum number of partitions is 2.
The maximum number of partitions is limited by ~nBuckets~ / 2.

*/

    PartitionManager* pmA;
/*
Partition manager for stream A. Contains the partitioning of stream A.

*/

    PartitionManager* pmB;
/*
Partition manager for stream B. Contains the partitioning of stream B.

*/

    RTuple tupleA;
/*
Reference to last tuple from stream A.

*/

    bool fitsInMemory;
/*
Flag which indicates if stream B fits completely into the operator's
main memory. If set to true the operator works like a standard hash join
operator. If this flag is false the operator performs a hybrid hash join
operation.

*/

    bool partitioning;
/*
Flag which indicates if stream A is still partitioned. If set to true
the partitioning of stream A has not yet been finished.

*/

    size_t curPartition;
/*
Number of the current processed partition.

*/
    bool firstPassPartition;
/*
Flag that indicates if the we are in the first pass of partition
a scan for stream B. Flag is used to decide whether tuples of
partition 0 from stream A have to be stored on disc.

*/

    bool finishedPartitionB;
/*
Flag that indicates that stream B has been processed completely.

*/

    PartitionIterator* iterA;
/*
Pointer to partition iterator. Stores the current position of a
partition during different REQUEST messages.

*/

    HashTable* hashTable;
/*
Pointer to in-memory hash table.

*/

    HybridHashJoinProgressLocalInfo* progress;
/*
Pointer to operator's progress information.

*/

    bool traceMode;
/*
Flag which decides if tracing information is generated. To enable tracing
set the flag ~RTF::ERA:TraceHybridHashJoin~ in SecondoConfig.ini.

*/

    bool subpartition;
/*
Flag which decides if partitions are sub-partitioned if they don't fit
into the operator's main memory. Per default this flag is set to true.
If the flag ~RTF::ERA:HybridHasjJoinNoSubpartitioning~ in SecondoConfig.ini
is set to true this flag will be set to false.

*/

    StopWatch timer;
/*
Timer used for tracing.

*/
};

/*
6 Class ~HybridHashJoinLocalInfo~

An instance of this class is used in the value mapping function to save
the state of the hybrid hash-join algorithm between multiple message calls.
This class simplifies access to the progress information. A pointer to this
instance of type ~HybridHashJoinLocalInfo~ will be passed to the
~HybridHashJoinAlgorithm~ object ~ptr~.

*/
class HybridHashJoinLocalInfo: public HybridHashJoinProgressLocalInfo
{
  public:

    HybridHashJoinLocalInfo( Supplier s) : 
      HybridHashJoinProgressLocalInfo( s ), ptr(0) {}
/*
The constructor. Construct an empty instance.

*/

    ~HybridHashJoinLocalInfo()
    {
      if (ptr)
      {
        delete ptr;
        ptr = 0;
      }
    }
/*
The destructor. Frees the hybrid hash-join algorithm instance.

*/

    HybridHashJoinAlgorithm * ptr;
/*
Pointer to hybrid hash-join algorithm

*/
};

} // end of namespace extrel2

#endif /* HYBRIDHASHJOIN_H_ */
