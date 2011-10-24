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

1 Implementation File HybridHashJoin.cpp

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
#include "GraceHashJoin.h"

/*
3 External linking

*/
extern QueryProcessor* qp;

/*
4 Auxiliary functions

*/
namespace extrel2
{

const double GraceHashJoinProgressLocalInfo::uHashJoin = 0.023;
const double GraceHashJoinProgressLocalInfo::vHashJoin = 0.0067;
const double GraceHashJoinProgressLocalInfo::wHashJoin = 0.0025;
const double GraceHashJoinProgressLocalInfo::t_read = 0.001090;
const double GraceHashJoinProgressLocalInfo::t_write = 0.001090;
const double GraceHashJoinProgressLocalInfo::t_probe = 0.001557;
const double GraceHashJoinProgressLocalInfo::t_hash = 0.004163;
const double GraceHashJoinProgressLocalInfo::t_result = 0.0044;



/*
5 Implementation of class ~GraceHashJoinProgressLocalInfo~

*/

GraceHashJoinProgressLocalInfo::GraceHashJoinProgressLocalInfo( Supplier s )
: ProgressLocalInfo()
, maxOperatorMemory((qp->GetMemorySize(s) * 1024 * 1024))
, tuplesProcessedSinceLastResult(0)
, traceMode(false)
{
}

int GraceHashJoinProgressLocalInfo::CalcProgress( ProgressInfo& p1,
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
    if ( traceMode )
    {
      cmsg.info() << "-----------------------------------------------" << endl;
      cmsg.info() << "calcProgressGrace()" << endl;
      cmsg.send();
    }
    calcProgressGrace(p1, p2, pRes, s);
  }
  else if ( state == 0  )
  {
    if ( traceMode )
    {
      cmsg.info() << "-----------------------------------------------" << endl;
      cmsg.info() << "calcProgressStd()" << endl;
      cmsg.send();
    }
    calcProgressStd(p1, p2, pRes, s);
  }
  else
  {
    return CANCEL;
  }

  return YIELD;
}

void GraceHashJoinProgressLocalInfo::calcProgressStd( ProgressInfo& p1,
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

  if ( traceMode )
  {
    cmsg.info() << "p1" << endl;
    PrintProgressInfo(cmsg.info(), p1);
    cmsg.info() << "p2" << endl;
    PrintProgressInfo(cmsg.info(), p2);
    cmsg.info() << "pRes" << endl;
    PrintProgressInfo(cmsg.info(), *pRes);
    cmsg.send();
  }
}

void GraceHashJoinProgressLocalInfo::calcProgressGrace( ProgressInfo& p1,
                                                        ProgressInfo& p2,
                                                        ProgressInfo* pRes,
                                                        Supplier s )
{
  double sel;
  double m = (double)this->returned;
  double k1 = (double)this->readFirst;
  double k2 = (double)this->readSecond;

  // -------------------------------------------
  // Result cardinality
  // -------------------------------------------

  // calculate estimated selectivity
  if ( m > enoughSuccessesJoin )
  {
    // warm state
    sel = m / ( k1 * streamB.GetTotalProcessedTuples() );

    if ( traceMode )
    {
      cmsg.info() << "WARM state => "
                  << ", m:" << m
                  << ", k1:" << k1
                  << ", streamB.GetTotalProcessedTuples():"
                  << streamB.GetTotalProcessedTuples()
                  << ", sel: "<< sel << endl;
      cmsg.send();
    }
  }
  else
  {
    // cold state
    sel = qp->GetSelectivity(s);

    if ( traceMode )
    {
      cmsg.info() << "COLD state => "
                  << ", m:" << m
                  << ", k1:" << k1
                  << ", k2:" << k2
                  << ", sel: "<< sel << endl;
      cmsg.send();
    }
  }

  // calculate result cardinality
  pRes->Card = p1.Card * p2.Card * sel;

  // -------------------------------------------
  // Total time
  // -------------------------------------------

  // calculate time needed for successors
  double t1 = p1.Time + p2.Time;

  if ( traceMode )
  {
    cmsg.info() << "1: t1 => " << t1 << endl;
    cmsg.send();
  }

  // calculate time for partitioning and processing of stream A
  double t2 = p1.Card * ( t_probe + t_hash + t_read + t_write );

  if ( traceMode )
  {
    cmsg.info() << "2: t2 => " << t2 << endl;
    cmsg.send();
  }

  double t3 = 0;

  if ( streamA.IsValid() )
  {
    for (size_t i = 0; i < streamA.partitionProgressInfo.size(); i++)
    {
      // cardinality of partition from A
      size_t cardA = streamA.partitionProgressInfo[i].tuples;

      // number of passes of the corresponding partition B
      size_t passesB = streamB.partitionProgressInfo[i].noOfPasses;

      if ( passesB > 1 )
      {
        t3 += cardA * ( passesB - 1 ) * ( t_probe + t_read );
      }
    }

    if ( traceMode )
    {
      cmsg.info() << "3: t3 => " << t3 << endl;
      cmsg.send();
    }
  }

  // calculate time for partitioning and processing of stream B
  double t4 = p2.Card * ( t_hash + t_read + t_write );
  if ( traceMode )
  {
    cmsg.info() << "4: t4 => " << t4 << endl;
    cmsg.send();
  }

  // calculate time for sub-partitioning of stream B
  double t5 = streamB.subTotalTuples * ( t_read + t_write );

  if ( traceMode )
  {
    cmsg.info() << "5: t5 => " << t5 << endl;
    cmsg.send();
  }

  // calculate time to create result tuples
  double t6 = pRes->Card * t_result;

  if ( traceMode )
  {
    cmsg.info() << "6: t6 => " << t6 << endl;
    cmsg.send();
  }

  pRes->Time = t1 + t2 + t3 + t4 + t5 + t6;

  if ( traceMode )
  {
    cmsg.info() << "7: pRes->Time => " << pRes->Time << endl;
    cmsg.send();
  }

  // -------------------------------------------
  // Total progress
  // -------------------------------------------

  // calculate current progress of successors
  double prog1 = p1.Progress * p1.Time + p2.Progress * p2.Time;

  if ( traceMode )
  {
    cmsg.info() << "1: prog1 => " << prog1 << endl;
    cmsg.send();
  }

  // calculate current progress of stream A
  double prog2 = k1 * ( t_probe + t_hash + t_read + t_write );

  if ( traceMode )
  {
    cmsg.info() << "2: prog2 => " << prog2 << endl;
    cmsg.send();
  }

  double prog3 = 0;

  if ( streamA.IsValid() )
  {
    for (size_t i = 0; i < streamA.partitionProgressInfo.size(); i++)
    {
      // cardinality of partition from A
      size_t cardA = streamA.partitionProgressInfo[i].tuples;

      // number of passes of the corresponding partition of B
      size_t passesB = streamB.partitionProgressInfo[i].noOfPasses;

      // number of processed tuples of partition from A
      size_t procA = streamA.partitionProgressInfo[i].tuplesProc;

      if ( passesB > 1 && procA > cardA )
      {
        prog3 += ( procA - cardA ) * ( t_probe + t_read );
      }
    }

    if ( traceMode )
    {
      cmsg.info() << "3: prog3 => " << prog3 << endl;
      cmsg.send();
    }
  }

  // calculate current progress of stream B
  double prog4 = streamB.GetTotalProcessedTuples()
                  * ( t_hash + t_read + t_write );

  if ( traceMode )
  {
    cmsg.info() << "4: prog4 => " << prog4 << endl;
    cmsg.send();
  }

  // calculate current progress for sub-partitioning of stream B
  double prog5 = streamB.subTuples * ( t_read + t_write );

  if ( traceMode )
  {
    cmsg.info() << "5: prog5 => " << prog5 << endl;
    cmsg.send();
  }

  // calculate time to create result tuples
  double prog6 = m * t_result;

  if ( traceMode )
  {
    cmsg.info() << "6: prog6 => " << prog6 << endl;
    cmsg.send();
  }

  // calculate total progress
  pRes->Progress = ( prog1 + prog2 + prog3 + prog4 + prog5 + prog6 )
                      / pRes->Time;
  if ( traceMode )
  {
    cmsg.info() << "7: pRes->Progress => "
                << pRes->Progress << endl
                << "8: streamB.subTotalTuples => "
                << streamB.subTotalTuples << endl
                << "9: streamB.subTuples => "
                << streamB.subTuples << endl;
    cmsg.send();
  }

  // -------------------------------------------
  // Blocking time
  // -------------------------------------------

  // calculate blocking time for successors
  pRes->BTime = p1.BTime + p2.BTime;

  // calculate time until stream B is partitioned
  pRes->BTime += p2.Card * ( t_hash + t_read + t_write )
                  + streamB.subTotalTuples * ( t_read + t_write );

  // -------------------------------------------
  // Blocking Progress
  // -------------------------------------------

  // calculate blocking progress for successors
  pRes->BProgress = p1.BProgress * p1.BTime + p2.BProgress * p2.BTime;

  // calculate progress of partitioning of stream B
  pRes->BProgress += k2 * ( t_hash + t_read + t_write )
                     + streamB.subTuples * ( t_read + t_write );

  // calculate blocking progress
  pRes->BProgress /= pRes->BTime;

  if ( traceMode )
  {
    cmsg.info() << "p1" << endl;
    PrintProgressInfo(cmsg.info(), p1);
    cmsg.info() << "p2" << endl;
    PrintProgressInfo(cmsg.info(), p2);
    cmsg.info() << "pRes" << endl;
    PrintProgressInfo(cmsg.info(), *pRes);
    cmsg.send();
  }

  return;
}

ostream& GraceHashJoinProgressLocalInfo::Print(ostream& os)
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
12 Implementation of class ~GraceHashJoinAlgorithm~

*/
GraceHashJoinAlgorithm::GraceHashJoinAlgorithm( Word streamA,
                                            int indexAttrA,
                                            Word streamB,
                                            int indexAttrB,
                                            size_t buckets,
                                            Supplier s,
                                            GraceHashJoinProgressLocalInfo* p,
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
, finishedPartitionB(false)
, iterA(0)
, hashTable(0)
, progress(p)
, traceMode(RTFlag::isActive("ERA:TraceGraceHashJoin"))
, subpartition(!RTFlag::isActive("ERA:GraceHashJoinNoSubpartitioning"))
{
  Word wTuple(Address(0));

  // currently we are in internal mode
  progress->state = 0;

  // Set operator's main memory
  setMemory(maxMemSize, s);

  // Set I/O buffer size for tuple buffers
  setIoBuffer(ioBufferSize);

  // Check number of buckets (must be divisible by two)
  setBuckets(MAX_MEMORY, buckets);

  // Check number of partitions
  setPartitions(partitions);

  if ( traceMode )
  {
    cmsg.info() << "-------------------- GRACE Hash-Join ------------------"
                << endl
                << "Buckets: \t\t\t" << nBuckets << endl
                << "Partitions: \t\t\t" << nPartitions << endl
                << "Memory: \t\t\t" << MAX_MEMORY / 1024 << " KByte" << endl
                << "I/O Buffer: \t\t\t" << PartitionManager::GetIOBufferSize()
                << " Byte" << endl
                << "Join attribute index A: \t" << attrIndexA
                << " (0 based)" << endl
                << "Join attribute index B: \t" << attrIndexB
                << " (0 based)" << endl << endl;
    cmsg.send();

    timer.start();
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

    // create partitions for stream B
    pmB = new PartitionManager( hashFuncB, MAX_MEMORY,
                                nBuckets, nPartitions,
                                0, &progress->streamB);

    // now we are in external mode
    progress->state = 1;

    // load current hash table content into partitions of stream B
    pmB->InitPartitions(hashTable);

    // partition the rest of stream B
    partitionB();

    // sub-partition stream B with maximum recursion level 3
    if ( subpartition )
    {
      pmB->Subpartition();
    }

    // create partitions for stream A according to partitioning of stream B
    pmA = new PartitionManager(hashFuncA, *pmB, &progress->streamA);

    // set current state
    partitioning = true;

    if ( traceMode )
    {
      cmsg.info() << "Partitioning of stream B.."
                  << " (timer: " << timer.diffSecondsReal() << ")"
                  << endl << *pmB;
      cmsg.send();
    }
  } else {
    delete hashFuncA;
    delete hashFuncB;
  }

  // read first tuple from stream A
  tupleA.setTuple( nextTupleA() );
}

/*
It may happen, that the localinfo object will be destroyed
before all internal buffered tuples are delivered stream
upwards, e.g. queries which use a ~head~ operator.
In this case we need to delete also all tuples stored in memory.

*/

GraceHashJoinAlgorithm::~GraceHashJoinAlgorithm()
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

void GraceHashJoinAlgorithm::setIoBuffer(size_t bytes)
{
  if ( bytes == UINT_MAX )
  {
    // set buffer to system's page size
    PartitionManager::SetIOBufferSize( WinUnix::getPageSize() );
  }
  else
  {
    // set buffer size
    PartitionManager::SetIOBufferSize(bytes);
  }
}

void GraceHashJoinAlgorithm::setMemory(size_t maxMemory, Supplier s)
{
  if ( maxMemory == UINT_MAX )
  {
    MAX_MEMORY = (qp->GetMemorySize(s) * 1024 * 1024);
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

  progress->maxOperatorMemory = MAX_MEMORY;
}

void GraceHashJoinAlgorithm::setBuckets(size_t maxMemory, size_t n)
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

void GraceHashJoinAlgorithm::setPartitions(size_t n)
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

Tuple* GraceHashJoinAlgorithm::nextTupleA()
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

Tuple* GraceHashJoinAlgorithm::nextTupleB()
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

void GraceHashJoinAlgorithm::partitionB()
{
  Tuple* t;

  while ( ( t = nextTupleB() ) )
  {
    pmB->Insert(t);
    t->DeleteIfAllowed();
  }

  return;
}

Tuple* GraceHashJoinAlgorithm::partitionA()
{
  while ( tupleA.tuple )
  {
    pmA->Insert(tupleA.tuple);
    tupleA.setTuple( nextTupleA() );
  }

  // change state
  partitioning = false;

  // load first partition from B into memory
  finishedPartitionB = pmB->LoadPartition(curPartition, hashTable, MAX_MEMORY);

  if ( traceMode )
  {
    //cmsg.info() << "Hash table content" << *hashTable << endl;
    cmsg.info() << "Partitioning of stream A.."
                << " (timer: " << timer.diffSecondsReal() << ")"
                << endl << *pmA
                << "finishedPartitionB" << finishedPartitionB
                << endl;
    cmsg.send();
  }

  // start scan of corresponding partition A
  if(iterA){
    delete iterA;
  }
  iterA = pmA->GetPartition(curPartition)->MakeScan();

  tupleA.setTuple( iterA->GetNextTuple() );

  return processPartitions();
}

Tuple* GraceHashJoinAlgorithm::processPartitions()
{
  while ( (int)curPartition < pmB->GetNoPartitions() )
  {
    while ( tupleA.tuple )
    {
      Tuple* tupleB = hashTable->Probe(tupleA.tuple);

      if ( tupleB )
      {
        Tuple *result = new Tuple( resultTupleType );
        Concat(tupleA.tuple, tupleB, result);
        progress->returned++;
        progress->tuplesProcessedSinceLastResult = 0;
        return result;
      }

      PartitionProgressInfo& pinfo =
          progress->streamA.partitionProgressInfo[curPartition];

      pinfo.tuplesProc++;

      pinfo.curPassNo =
          (int)ceil( (double)pinfo.tuplesProc / (double) pinfo.tuples);

      progress->CheckProgressSinceLastResult();

      tupleA.setTuple( iterA->GetNextTuple() );
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
      if(iterA){
        delete iterA;
      }
      iterA = pmA->GetPartition(curPartition)->MakeScan();

      // Read first tuple from A(i)
      tupleA.setTuple( iterA->GetNextTuple() );
    }
  }

  return 0;
}
Tuple* GraceHashJoinAlgorithm::NextResultTuple()
{
  if ( fitsInMemory )
  {
    // Standard in-memory hash-join
    while ( tupleA.tuple )
    {
      Tuple* tupleB = hashTable->Probe(tupleA.tuple);

      if ( tupleB )
      {
        Tuple *result = new Tuple( resultTupleType );
        Concat(tupleA.tuple, tupleB, result);
        progress->returned++;
        progress->tuplesProcessedSinceLastResult = 0;
        return result;
      }
      else
      {
        tupleA.setTuple( nextTupleA() );
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
13 Implementation of value mapping function of operator ~gracehashjoin~

*/

template<bool param>
int GraceHashJoinValueMap( Word* args, Word& result,
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

  GraceHashJoinLocalInfo* li;
  li = static_cast<GraceHashJoinLocalInfo*>( local.addr );

  switch(message)
  {
    case OPEN:
    {
      if (li)
      {
        delete li;
      }

      li = new GraceHashJoinLocalInfo( s );
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

          li->ptr = new GraceHashJoinAlgorithm( args[0], indexAttrA,
                                                args[1], indexAttrB,
                                                nBuckets, s, li,
                                                nPartitions, maxMem,
                                                ioBufferSize );

        }
        else
        {
          int indexAttrA = StdTypes::GetInt( args[5] );
          int indexAttrB = StdTypes::GetInt( args[6] );

          li->ptr = new GraceHashJoinAlgorithm( args[0], indexAttrA,
                                                args[1], indexAttrB,
                                                nBuckets, s, li );
        }

      }

      GraceHashJoinAlgorithm* algo = li->ptr;
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
14 Instantiation of Template Functions

For some reasons the compiler cannot expand these template functions in
the file ~ExtRelation2Algebra.cpp~, thus the value mapping functions
are instantiated here.

*/

template
int GraceHashJoinValueMap<false>( Word* args, Word& result,
                                  int message, Word& local, Supplier s );

template
int GraceHashJoinValueMap<true>( Word* args, Word& result,
                                 int message, Word& local, Supplier s);

} // end of namespace extrel2
