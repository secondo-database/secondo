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

1 Implementation File Sort.cpp

June 2009, Sven Jungnickel. Initial version

2 Includes and defines

*/

#include <algorithm>
#include <cmath>
#include "stdlib.h"

#include "LogMsg.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "RTuple.h"
#include "HybridHashJoin.h"

/*
3 External linking

*/
extern QueryProcessor* qp;

/*
4 Auxiliary functions

*/
namespace extrel2
{

double log2(double n)
{
  return ( log(n) / log(2.0) );
}

/*
5 Implementation of class ~HybridHashJoinProgressLocalInfo~

*/

HybridHashJoinProgressLocalInfo::HybridHashJoinProgressLocalInfo()
: ProgressLocalInfo()
{
}

int HybridHashJoinProgressLocalInfo::CalcProgress( ProgressInfo& p1,
                                                   ProgressInfo& p2,
                                                   ProgressInfo* pRes,
                                                   Supplier s )
{
  // calculate tuple size of join
  this->SetJoinSizes(p1, p2);

  // copy sizes to result
  pRes->CopySizes(this);

  if ( this->state == 1 )
  {
    calcProgressHybrid(p1, p2, pRes, s);
  }
  else if ( state == 0  )
  {
    calcProgressStd(p1, p2, pRes, s);
  }
  else
  {
    return CANCEL;
  }

  return YIELD;
}

void HybridHashJoinProgressLocalInfo::calcProgressStd( ProgressInfo& p1,
                                                       ProgressInfo& p2,
                                                       ProgressInfo* pRes,
                                                       Supplier s )
{
  double sel;
  double m = (double)this->returned;
  double k1 = (double)this->readFirst;
  double k2 = (double)this->readSecond;

  // Calculate estimated selectivity
  if ( m > enoughSuccessesJoin )
  {
    // warm state
    sel = m / ( k1 * k2 );
  }
  else
  {
    // cold state
    sel = qp->GetSelectivity(s);
  }

  // calculate result cardinality
  pRes->Card = p1.Card * p2.Card * sel;

  // calculate total time
  pRes->Time = p1.Time + p2.Time
                + p2.Card * vHashJoin     // reading stream B into hash table
                + p1.Card * uHashJoin     // probing stream A against hash table
                + pRes->Card * wHashJoin; // output of result tuples

  // calculate total progress
  pRes->Progress = ( p1.Progress * p1.Time + p2.Progress * p2.Time
                      + this->readSecond * vHashJoin
                      + this->readFirst * uHashJoin
                      + this->returned * wHashJoin ) / pRes->Time;

  // calculate time until first result tuple
  pRes->BTime = p1.BTime + p2.BTime
                + p2.Card * vHashJoin; // reading stream B into hash table

  // calculate blocking progress
  pRes->BProgress = ( p1.BProgress * p1.BTime + p2.BProgress * p2.BTime
                      + this->readSecond * vHashJoin ) / pRes->BTime;

}

void HybridHashJoinProgressLocalInfo::calcProgressHybrid( ProgressInfo& p1,
                                                          ProgressInfo& p2,
                                                          ProgressInfo* pRes,
                                                          Supplier s )
{
  double sel;
  double m = (double)this->returned;
  double k1 = (double)this->readFirst;
  double k2 = (double)this->readSecond;
  size_t M = qp->MemoryAvailableForOperator();
  size_t S2 = (size_t)p2.SizeExt;

  // calculate the maximum amount of tuples of stream B that fit into
  // the operator's main memory
  size_t M_S2 = (size_t)floor((double)M / (double)S2);

  // -------------------------------------------
  // Result cardinality
  // -------------------------------------------

  // calculate estimated selectivity
  if ( m > enoughSuccessesJoin )
  {
    // warm state
    sel = m / ( k1 * k2 );
  }
  else
  {
    // cold state
    sel = qp->GetSelectivity(s);
  }

  // calculate result cardinality
  pRes->Card = p1.Card * p2.Card * sel;

  // -------------------------------------------
  // Total time
  // -------------------------------------------

  // calculate time needed for successors
  pRes->Time = p1.Time + p2.Time;

  // calculate time for partitioning and processing of stream A
  if ( streamA.IsValid() )
  {
    for (size_t i = 0; i < streamA.partitionProgressInfo.size(); i++)
    {
      pRes->Time += streamA.partitionProgressInfo[i].tuples
                    * ( streamA.partitionProgressInfo[i].noOfPasses
                        * ( t_probe + t_read ) + t_write );
    }
    pRes->Time -= streamA.partitionProgressInfo[0].tuples
                  * ( t_read + t_write );
  }

  // calculate time for partitioning and processing of stream B
  pRes->Time += p2.Card * ( t_hash + t_read + t_write )
                - min(streamB.partitionProgressInfo[0].tuples, M_S2)
                  * ( t_read + t_write );

  // calculate time for sub-partitioning of stream B
  pRes->Time += streamB.subTotalTuples * ( t_read + t_write );

  // calculate time to create result tuples
  pRes->Time += pRes->Card * t_result;

  // -------------------------------------------
  // Total progress
  // -------------------------------------------

  // calculate current progress of successors
  pRes->Progress = p1.Progress * p1.Time + p2.Progress * p2.Time;

  // calculate current progress of stream A
  if ( streamA.IsValid() )
  {
    for (size_t i = 0; i < streamA.partitionProgressInfo.size(); i++)
    {
      pRes->Progress += streamA.partitionProgressInfo[i].tuplesProc
                        * ( t_probe + t_read )
                        + min( streamA.partitionProgressInfo[i].tuples,
                               streamA.partitionProgressInfo[i].tuplesProc )
                               * t_write ;
    }
    pRes->Progress -= min( streamA.partitionProgressInfo[0].tuples,
                           streamA.partitionProgressInfo[0].tuplesProc )
                      * ( t_read + t_write );
  }

  // calculate current progress of stream B
  pRes->Progress += k2 * ( t_hash + t_read + t_write )
                    - min(streamB.partitionProgressInfo[0].tuples, M_S2)
                    * ( t_read +  t_write );

  // calculate current progress for sub-partitioning of stream B
  pRes->Progress += streamB.subTuples * ( t_read + t_write );

  // calculate time to create result tuples
  pRes->Progress += m * t_result;

  // calculate total progress
  pRes->Progress /= pRes->Time;

  // -------------------------------------------
  // Blocking time
  // -------------------------------------------

  // calculate blocking time for successors
  pRes->BTime = p1.BTime + p2.BTime;

  // calculate time until stream B is partitioned
  pRes->BTime += p2.Card * ( t_hash + t_read + t_write )
                  - min(streamB.partitionProgressInfo[0].tuples, M_S2)
                    * ( t_read +  t_write )
                  + streamB.subTotalTuples * ( t_read + t_write );

  // -------------------------------------------
  // Blocking Progress
  // -------------------------------------------

  // calculate blocking progress for successors
  pRes->BProgress = p1.BProgress * p1.BTime + p2.BProgress * p2.BTime;

  // calculate progress of partitioning of stream B
  pRes->BProgress += k2 * ( t_hash + t_read + t_write )
                     - min(streamB.partitionProgressInfo[0].tuplesProc, M_S2)
                       * ( t_read +  t_write )
                     + streamB.subTuples * ( t_read + t_write );

  // calculate blocking progress
  pRes->BProgress /= pRes->BTime;

  cmsg.info() << "pRes->Card: " << pRes->Card << endl
              << "pRes->Time: " << pRes->Time << endl
              << "pRes->Progress: " << pRes->Progress << endl
              << "pRes->BTime: " << pRes->BTime << endl
              << "pRes->BProgress: " << pRes->BProgress << endl;
  cmsg.send();

  return;
}

ostream& HybridHashJoinProgressLocalInfo::Print(ostream& os)
{
  os << "---------- Progress Information -----------"
     << endl
     << "k1: " << this->readFirst
     << ", k2: " << this->readSecond
     << ", m: " << this->returned << endl;

  if ( state == 1)
  {
    if ( streamA.IsValid() )
    {
      os << "ProgressInformation - Stream A" << endl;
      streamA.Print(os);
    }

    if ( streamB.IsValid() )
    {
      os << "ProgressInformation - Stream B" << endl;
      streamB.Print(os);
    }
  }

  return os;
}

/*
6 Implementation of class ~HybridHashJoinAlgorithm~

*/
HybridHashJoinAlgorithm::HybridHashJoinAlgorithm( Word streamA,
                                            int indexAttrA,
                                            Word streamB,
                                            int indexAttrB,
                                            size_t buckets,
                                            Supplier s,
                                            HybridHashJoinProgressLocalInfo* p,
                                            size_t partitions,
                                            size_t maxMemSize,
                                            size_t ioBufferSize )
: streamA(streamA)
, streamB(streamB)
, attrIndexA(indexAttrA-1)
, attrIndexB(indexAttrB-1)
, MAX_MEMORY(0)
, usedMemory(0)
, resultTupleType(0)
, tupleTypeA(0)
, tupleTypeB(0)
, nBuckets(0)
, nPartitions(0)
, pmA(0)
, pmB(0)
, tupleA(0)
, fitsInMemory(false)
, partitioning(false)
, curPartition(0)
, bucketProcessed(true)
, finishedPartitionB(false)
, iterA(0)
, hashTable(0)
, progress(p)
, traceMode(RTFlag::isActive("ERA:TraceHybridHashJoin"))
{
  Word wTuple(Address(0));

  // currently we are in internal mode
  progress->state = 0;

  // Set operator's main memory
  setMemory(maxMemSize);

  // Set I/O buffer size for tuple buffers
  setIoBuffer(ioBufferSize);

  // Check number of buckets (must be divisible by two)
  setBuckets(MAX_MEMORY, buckets);

  // Check number of partitions
  setPartitions(partitions);

  if ( traceMode )
  {
    cmsg.info() << "-------------------- Hybrid Hash-Join ------------------"
                << endl
                << "Buckets: \t\t\t" << nBuckets << endl
                << "Partitions: \t\t\t" << nPartitions << endl
                << "Memory: \t\t\t" << MAX_MEMORY / 1024 << " KByte" << endl
                << "I/O Buffer: \t\t\t" << TupleBuffer2::GetIoBufferSize()
                << " Byte" << endl
                << "Join attribute index A: \t" << attrIndexA
                << " (0 based)" << endl
                << "Join attribute index B: \t" << attrIndexB
                << " (0 based)" << endl << endl;
    cmsg.send();
  }

  // create hash function instances
  HashFunction* hashFuncA =
    new HashFunction(this->nBuckets, this->attrIndexA);

  HashFunction* hashFuncB =
    new HashFunction(this->nBuckets, this->attrIndexB);

  // create tuple comparison function instance
  JoinTupleCompareFunction* cmp =
    new JoinTupleCompareFunction( this->attrIndexA,
                                  this->attrIndexB);

  // create result type
  ListExpr resultType =
    SecondoSystem::GetCatalog()->NumericType( qp->GetType(s) );
  resultTupleType =  new TupleType( nl->Second( resultType ) );

  // create hash table
  hashTable = new HashTable( this->nBuckets,
                              new HashFunction(*hashFuncB),
                              cmp);

  // Read tuples from stream B until memory is full or stream B is finished
  progress->readSecond +=
    hashTable->ReadFromStream(streamB, MAX_MEMORY, fitsInMemory);

  if ( !fitsInMemory )
  {
    if ( traceMode )
    {
      cmsg.info() << "Switching to external hash-join algorithm!" << endl;
      cmsg.send();
    }

    // create partitions for stream B with partition 0 buffered
    pmB = new PartitionManager( hashFuncB, nBuckets, nPartitions,
                                MAX_MEMORY, &progress->streamB);

    // now we are in external mode
    progress->state = 1;

    // load current hash table content into partitions of stream B
    pmB->InitPartitions(hashTable);

    // partition the rest of stream B
    partitionB();

    // sub-partition stream B with maximum recursion level 3
    pmB->Subpartition();

    // create partitions for stream A according to partitioning of stream B
    pmA = new PartitionManager(hashFuncA, *pmB, &progress->streamA);

    // load partition 0 into hash table
    finishedPartitionB = pmB->LoadPartition(0, hashTable, MAX_MEMORY);
    curPartition = 0;

    // set current state
    partitioning = true;

    if ( traceMode )
    {
      cmsg.info() << "Partitioning of stream B.."
                  << endl << *pmB;
      cmsg.info() << "Hash Table content:" << endl
                  << *hashTable << endl;
      cmsg.send();
    }
  }

  // read first tuple from stream A
  tupleA = nextTupleA();
}

/*
It may happen, that the localinfo object will be destroyed
before all internal buffered tuples are delivered stream
upwards, e.g. queries which use a ~head~ operator.
In this case we need to delete also all tuples stored in memory.

*/

HybridHashJoinAlgorithm::~HybridHashJoinAlgorithm()
{
  if ( traceMode )
  {
    float sel = (float)progress->returned
        / ( (float)progress->readFirst * (float)progress->readSecond );
    cmsg.info() << "C1: " << progress->readFirst << endl
                << "C2: " << progress->readSecond << endl
                << "m: " << progress->returned << endl
                << "Selectivity: " << sel << endl;
    cmsg.send();
  }


  if ( hashTable )
  {
    delete hashTable;
    hashTable = 0;
  }

  if ( iterA )
  {
    delete iterA;
    iterA = 0;
  }

  if ( resultTupleType )
  {
    resultTupleType->DeleteIfAllowed();
    resultTupleType = 0;
  }

  if ( tupleTypeA )
  {
    tupleTypeA->DeleteIfAllowed();
    tupleTypeA = 0;
  }

  if ( tupleTypeB )
  {
    tupleTypeB->DeleteIfAllowed();
    tupleTypeB = 0;
  }

  if ( pmA )
  {
    delete pmA;
    pmA = 0;
  }

  if ( pmB )
  {
    delete pmB;
    pmB = 0;
  }
}

void HybridHashJoinAlgorithm::setIoBuffer(size_t bytes)
{
  if ( bytes == UINT_MAX )
  {
    // set buffer to system's page size
    TupleBuffer2::SetIoBufferSize( WinUnix::getPageSize() );
  }
  else
  {
    // set buffer size
    TupleBuffer2::SetIoBufferSize(bytes);
  }
}

void HybridHashJoinAlgorithm::setMemory(size_t maxMemory)
{
  if ( maxMemory == UINT_MAX )
  {
    MAX_MEMORY = qp->MemoryAvailableForOperator();
  }
  else if ( maxMemory < MIN_USER_DEF_MEMORY )
  {
    MAX_MEMORY = MIN_USER_DEF_MEMORY;
  }
  else if ( maxMemory > MAX_USER_DEF_MEMORY )
  {
    MAX_MEMORY = MAX_USER_DEF_MEMORY;
  }
  else
  {
    MAX_MEMORY = maxMemory;
  }
}

void HybridHashJoinAlgorithm::setBuckets(size_t maxMemory, size_t n)
{
  // calculate maximum number of buckets
  size_t maxBuckets = maxMemory / 1024;

  // make bucket number divisible by two
  n = n + n % 2;

  // check upper limit
  n = ( n > maxBuckets ) ? maxBuckets : n;

  // check lower limit
  n = ( n < MIN_BUCKETS ) ? MIN_BUCKETS : n;

  nBuckets = n;
}

void HybridHashJoinAlgorithm::setPartitions(size_t n)
{
  assert(nBuckets >= MIN_BUCKETS);

  // calculate maximum number of partitions -> nBuckets/2
  size_t maxPartitions = nBuckets / 2;

  // check if we should use default number of partitions
  if ( n == UINT_MAX )
  {
    // default value is the number of inner nodes
    // in a binary tree with nBucket leafs and height h
    // on level h/2
    n = 1 << (int)( log2(nBuckets) / 2.0 );
  }

  // check upper limit
  n = n > maxPartitions ? maxPartitions : n;

  // check lower limit
  n = n < MIN_PARTITIONS ? MIN_PARTITIONS : n;

  nPartitions = n;
}

Tuple* HybridHashJoinAlgorithm::nextTupleA()
{
  Word wTuple(Address(0));

  qp->Request(streamA.addr, wTuple);

  if ( qp->Received(streamA.addr) )
  {
    progress->readFirst++;
    Tuple* t = static_cast<Tuple*>( wTuple.addr );

    if ( tupleTypeA == 0 )
    {
      tupleTypeA = t->GetTupleType();
      tupleTypeA->IncReference();
    }

    return t;
  }

  return NULL;
}

Tuple* HybridHashJoinAlgorithm::nextTupleB()
{
  Word wTuple(Address(0));

  qp->Request(streamB.addr, wTuple);

  if ( qp->Received(streamB.addr) )
  {
    progress->readSecond++;
    Tuple* t = static_cast<Tuple*>( wTuple.addr );

    if ( tupleTypeB == 0 )
    {
      tupleTypeB = t->GetTupleType();
      tupleTypeB->IncReference();
    }

    return t;
  }

  return NULL;
}

void HybridHashJoinAlgorithm::partitionB()
{
  Tuple* t;

  while ( ( t = nextTupleB() ) )
  {
    pmB->Insert(t);
  }

  return;
}

Tuple* HybridHashJoinAlgorithm::partitionA()
{
  while ( tupleA )
  {
    size_t p;

    if ( ( p = pmA->FindPartition(tupleA) ) == 0 )
    {
      Tuple* tupleB = hashTable->Probe(tupleA);

      if ( progress->streamA.IsValid() && bucketProcessed == true )
      {
        progress->streamA.partitionProgressInfo[0].tuples++;
      }

      if ( tupleB )
      {
        // bucket contains match -> build result tuple
        Tuple *result = new Tuple( resultTupleType );
        Concat( tupleA, tupleB, result );
        progress->returned++;
        bucketProcessed = false;
        return result;
      }
      else // bucket completely processed
      {
        bucketProcessed = true;

        if ( progress->streamA.IsValid() )
        {
          progress->streamA.partitionProgressInfo[0].tuplesProc++;
        }

        // if partition 0 overflows store tuple
        if ( !finishedPartitionB )
        {
          tupleA->IncReference();
          pmA->Insert(tupleA);
        }
      }
    }
    else
    {
      // insert tuple into partition p
      tupleA->IncReference();
      pmA->Insert(tupleA);
    }

    tupleA->DeleteIfAllowed();
    tupleA = nextTupleA();
  }

  // change state
  partitioning = false;

  if ( finishedPartitionB )
  {
    curPartition++;
  }

  // load next partition into memory
  finishedPartitionB = pmB->LoadPartition(curPartition, hashTable, MAX_MEMORY);

  if ( traceMode )
  {
    cmsg.info() << "Hash table content" << *hashTable << endl;
    cmsg.info() << "Partitioning of stream A.." << endl << *pmA;
    cmsg.send();
  }

  // start scan of corresponding partition A
  iterA = pmA->GetPartition(curPartition)->MakeScan();

  tupleA = iterA->GetNextTuple();

  return processPartitions();
}

Tuple* HybridHashJoinAlgorithm::processPartitions()
{
  while ( (int)curPartition < pmB->GetNoPartitions() )
  {
    while ( tupleA )
    {
      Tuple* tupleB = hashTable->Probe(tupleA);

      if ( tupleB )
      {
        Tuple *result = new Tuple( resultTupleType );
        Concat(tupleA, tupleB, result);
        progress->returned++;
        return result;
      }

      if ( progress->streamA.IsValid() )
      {
        progress->streamA.partitionProgressInfo[curPartition].tuplesProc++;
      }

      tupleA->DeleteIfAllowed();
      tupleA = iterA->GetNextTuple();
    }

    // Proceed to next partition if B(i) is finished
    if ( finishedPartitionB )
    {
      curPartition++;
    }

    if ( (int)curPartition < pmB->GetNoPartitions() )
    {
      // Load next part of partition B(i) into memory
      finishedPartitionB =
        pmB->LoadPartition(curPartition, hashTable, MAX_MEMORY);

      // Start scan of corresponding partition A(i)
      iterA = pmA->GetPartition(curPartition)->MakeScan();

      // Read first tuple from A(i)
      tupleA->DeleteIfAllowed();
      tupleA = iterA->GetNextTuple();
    }
  }

  return 0;
}
Tuple* HybridHashJoinAlgorithm::NextResultTuple()
{
  if ( fitsInMemory )
  {
    // Standard in-memory hash-join
    while ( tupleA )
    {
      Tuple* tupleB = hashTable->Probe(tupleA);

      if ( tupleB )
      {
        Tuple *result = new Tuple( resultTupleType );
        Concat(tupleA, tupleB, result);
        progress->returned++;
        return result;
      }
      else
      {
        tupleA->DeleteIfAllowed();
        tupleA = nextTupleA();
      }
    }

    return 0;
  }
  else if ( partitioning )
  {
    return partitionA();
  }
  else
  {
    return processPartitions();
  }

  return 0;
}

/*
7 Implementation of value mapping function of operator ~hybridhashjoin~

*/

template<bool param>
int HybridHashJoinValueMap( Word* args, Word& result,
                            int message, Word& local, Supplier s)
{
  // if ( param = false )
  // args[0] : stream A
  // args[1] : stream B
  // args[2] : attribute name of join attribute for stream A
  // args[3] : attribute name join attribute for stream B
  // args[4] : number of buckets
  // args[5] : attribute index of join attribute for stream A
  // args[6] : attribute index of join attribute for stream B

  // if ( param = true )
  // args[0] : stream A
  // args[1] : stream B
  // args[2] : attribute name of join attribute for stream A
  // args[3] : attribute name join attribute for stream B
  // args[4] : number of buckets
  // args[5] : number of partitions (only if param is true)
  // args[6] : usable main memory in bytes (only if param is true)
  // args[7] : I/O buffer size in bytes (only if param is true)
  // args[8] : attribute index of join attribute for stream A
  // args[9] : attribute index of join attribute for stream B

  HybridHashJoinLocalInfo* li;
  li = static_cast<HybridHashJoinLocalInfo*>( local.addr );

  switch(message)
  {
    case OPEN:
    {
      if (li)
      {
        delete li;
      }

      li = new HybridHashJoinLocalInfo();
      local.addr = li;

      // at this point the local value is well defined
      // afterwards progress request calls are
      // allowed.

      li->ptr = 0;

      qp->Open(args[0].addr);
      qp->Open(args[1].addr);

      return 0;
    }

    case REQUEST:
    {
      if ( li->ptr == 0 )
      {
        int nBuckets = StdTypes::GetInt( args[4] );

        if ( param )
        {
          int nPartitions = StdTypes::GetInt( args[5] );
          int maxMem = StdTypes::GetInt( args[6] );
          int ioBufferSize = StdTypes::GetInt( args[7] );
          int indexAttrA = StdTypes::GetInt( args[8] );
          int indexAttrB = StdTypes::GetInt( args[9] );

          li->ptr = new HybridHashJoinAlgorithm( args[0],
                                                 indexAttrA,
                                                 args[1],
                                                 indexAttrB,
                                                 nBuckets,
                                                 s,
                                                 li,
                                                 nPartitions,
                                                 maxMem,
                                                 ioBufferSize );

        }
        else
        {
          int indexAttrA = StdTypes::GetInt( args[5] );
          int indexAttrB = StdTypes::GetInt( args[6] );

          li->ptr = new HybridHashJoinAlgorithm( args[0],
                                                 indexAttrA,
                                                 args[1],
                                                 indexAttrB,
                                                 nBuckets,
                                                 s,
                                                 li );
        }

      }

      HybridHashJoinAlgorithm* algo = li->ptr;
      result.setAddr( algo->NextResultTuple() );
      return result.addr != 0 ? YIELD : CANCEL;
    }

    case CLOSE:
    {
      qp->Close(args[0].addr);
      qp->Close(args[1].addr);

      // Note: object deletion is handled in OPEN and CLOSEPROGRESS
      return 0;
    }


    case CLOSEPROGRESS:
    {
      if (li)
      {
        delete li;
        local.addr = 0;
      }
      return 0;
    }

    case REQUESTPROGRESS:
    {
      ProgressInfo p1, p2;
      ProgressInfo* pRes = static_cast<ProgressInfo*>( result.addr );

      if( !li )
      {
         return CANCEL;
      }
      else
      {
        if ( qp->RequestProgress(args[0].addr, &p1) &&
             qp->RequestProgress(args[1].addr, &p2) )
        {
          return li->CalcProgress(p1, p2, pRes, s);
        }
        else
        {
          return CANCEL;
        }
      }
    }
  }
  return 0;
}

/*
8 Instantiation of Template Functions

For some reasons the compiler cannot expand these template functions in
the file ~ExtRelation2Algebra.cpp~, thus the value mapping functions
are instantiated here.

*/

template
int HybridHashJoinValueMap<false>( Word* args, Word& result,
                                    int message, Word& local, Supplier s );

template
int HybridHashJoinValueMap<true>( Word* args, Word& result,
                                   int message, Word& local, Supplier s);

} // end of namespace extrel2
