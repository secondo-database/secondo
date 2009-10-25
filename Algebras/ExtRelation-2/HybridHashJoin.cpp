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


ostream& PrintProgressInfo(ostream& os, ProgressInfo& info)
{
  cmsg.info() << "Card: " << info.Card
              << ", Time: " << info.Time
              << ", Progress: " << info.Progress << endl
              << "BTime: " << info.BTime
              << ", BProgress: " << info.BProgress
              << ", Size: " << info.Size
              << ", SizeExt: " << info.SizeExt << endl;
  cmsg.send();

  return os;
}


/*
5 Implementation of class ~Bucket~

*/

ostream& Bucket::Print(ostream& os)
{
  os << "Bucket " << number << " (" << tuples.size() << " tuples)" << endl;

  for(size_t i = 0; i < tuples.size(); i++)
  {
    Tuple* t = tuples[i].tuple;
    os << *t << "(Refs: " << t->GetNumOfRefs() << ")" << endl;
  }

  return os;
}

/*
6 Implementation of class ~BucketIterator~

*/
BucketIterator::BucketIterator(Bucket& b)
: bucket(b)
{
  iter = bucket.tuples.begin();
}

Tuple* BucketIterator::GetNextTuple()
{
  if ( iter != bucket.tuples.end() )
  {
    Tuple* t = (*iter).tuple;
    iter++;
    t->IncReference();
    return t;
  }

  return 0;
}

/*
7 Implementation of class ~HashTable~

*/
HashTable::HashTable( size_t nBuckets,
                      HashFunction* f,
                      JoinTupleCompareFunction* cmp )
: iter(0)
, hashFunc(f)
, cmpFunc(cmp)
{
  for(size_t i = 0; i < nBuckets; i++)
  {
    buckets.push_back( new Bucket(i) );
  }
}

HashTable::~HashTable()
{
  for(size_t i = 0; i < buckets.size(); i++)
  {
    delete buckets[i];
  }
  buckets.clear();

  if ( iter )
  {
    delete iter;
    iter = 0;
  }

  if ( hashFunc )
  {
    delete hashFunc;
    hashFunc = 0;
  }

  if ( cmpFunc )
  {
    delete cmpFunc;
    cmpFunc = 0;
  }
}

void HashTable::Clear()
{
  // reset iterator
  if ( iter )
  {
    delete iter;
    iter = 0;
  }

  // clear buckets
  for(size_t i = 0; i < buckets.size(); i++)
  {
    buckets[i]->Clear();
  }
}

void HashTable::Insert(Tuple* t)
{
  // calculate bucket number
  size_t h = hashFunc->Value(t);

  // insert tuple into bucket
  buckets[h]->Insert(t);
}

int HashTable::ReadFromStream(Word stream, size_t maxSize, bool& finished)
{
  int read = 0;
  size_t bytes = 0;
  Word wTuple(Address(0));

  // Request first tuple
  qp->Request(stream.addr, wTuple);

  while( qp->Received(stream.addr) )
  {
    read++;
    Tuple *t = static_cast<Tuple*>( wTuple.addr );

    bytes += t->GetExtSize();

    // insert tuple into hash table
    this->Insert(t);
    t->DeleteIfAllowed();

    if ( bytes > maxSize )
    {
      finished = false;
      return read;
    }

    qp->Request(stream.addr, wTuple);
  }

  finished = true;

  return read;
}

Tuple* HashTable::Probe(Tuple* t)
{
  Tuple* nextTuple = 0;

  // calculate bucket number
  size_t h = hashFunc->Value(t);

  if ( iter == 0 )
  {
    // start bucket scan
    iter = buckets[h]->MakeScan();

    if ( traceMode )
    {
      cmsg.info() << "Start scanning bucket "
                  << h << ".." << endl;
      cmsg.send();
    }
  }
  else
  {
    if ( traceMode )
    {
      cmsg.info() << "Proceeding scanning bucket "
                  << h << ".." << endl;
      cmsg.send();
    }
  }

  while ( (nextTuple = iter->GetNextTuple() ) != 0 )
  {
    // GetNextTuple() increments reference counter by one, but
    // tuple stays in hash table
    nextTuple->DeleteIfAllowed();

    if ( traceMode )
    {
      cmsg.info() << "Comparing :" << *t << " and " << *nextTuple;
      cmsg.send();
    }

    if ( cmpFunc->Compare(t, nextTuple) == 0 )
    {
      if ( traceMode )
      {
        cmsg.info() << " -> Match!" << endl;
        cmsg.send();
      }
      return nextTuple;
    }

    if ( traceMode )
    {
      cmsg.info() << ".." << endl;
      cmsg.send();
    }
  }

  delete iter;
  iter = 0;

  if ( traceMode )
  {
    cmsg.info() << "End of scan bucket "
                << h << ".." << endl;
    cmsg.send();
  }

  return 0;
}

vector<Tuple*> HashTable::GetTuples(int bucket)
{
  Tuple* t;
  vector<Tuple*> arr;

  BucketIterator* iter = buckets[bucket]->MakeScan();

  while ( ( t = iter->GetNextTuple() ) != 0 )
  {
    arr.push_back(t);
  }

  return arr;
}

ostream& HashTable::Print(ostream& os)
{
  os << "------------- Hash-Table content --------------" << endl;

  for(size_t i = 0; i < buckets.size(); i++)
  {
    buckets[i]->Print(os);
  }

  return os;
}

/*
8 Implementation of class ~PartitionHistogram~

*/

PartitionHistogram::PartitionHistogram(PInterval& intv)
: interval(intv)
, tuples(0)
, totalSize(0)
, totalExtSize(0)
{
  // create entries
  for (size_t j = 0; j < intv.GetLength(); j++)
  {
    data.push_back( PartitionHistogramEntry(intv.GetLow() + j) );
  }
}

PartitionHistogram::PartitionHistogram( PartitionHistogram& obj,
                                        size_t start, size_t end )
: tuples(0)
, totalSize(0)
, totalExtSize(0)
{
  PInterval& intv = obj.GetInterval();

  assert( (end - start) < intv.GetLength() );

  interval = PInterval(intv.GetLow() + start, intv.GetLow() + end);

  // create entries
  for (size_t i = 0, j = start; i < interval.GetLength(); i++, j++)
  {
    data.push_back( PartitionHistogramEntry( obj.data[j] ) );
  }

  // update counters
  for ( size_t i = 0; i < data.size(); i++ )
  {
    tuples += data[i].count;
    totalSize += data[i].totalSize;
    totalExtSize += data[i].totalExtSize;
  }
}

void PartitionHistogram::Insert(Tuple* t, size_t hashFuncValue)
{
  assert(interval.IsAt(hashFuncValue));

  size_t s = t->GetSize();
  size_t sExt = t->GetExtSize();

  tuples++;
  totalSize += s;
  totalExtSize += sExt;

  int hIndex = hashFuncValue - interval.GetLow();

  data[hIndex].count++;
  data[hIndex].totalSize += s;
  data[hIndex].totalExtSize += sExt;

  return;
}

PartitionHistogramEntry& PartitionHistogram::GetHistogramEntry(size_t n)
{
  assert(n < data.size());
  return data[n];
}

/*
9 Implementation of class ~Partition~

*/

Partition::Partition(PInterval i, size_t bufferSize, size_t ioBufferSize)
: interval(i)
, histogram(i)
, subpartitioned(false)
{
  buffer = new TupleBuffer2(bufferSize, ioBufferSize);
}

Partition::~Partition()
{
  if ( buffer )
  {
    delete buffer;
    buffer = 0;
  }
}

PartitionIterator* Partition::MakeScan()
{
  return new PartitionIterator(*this);
}

void Partition::Insert(Tuple* t, size_t hashFuncValue)
{
  // Insert tuple into partition histogram
  histogram.Insert(t, hashFuncValue);

  // Append tuple to buffer
  buffer->AppendTuple(t);
}

ostream& Partition::Print(ostream& os)
{
    os << "[" << this->interval.GetLow() << ", "
       << this->interval.GetHigh() << "] -> "
       << this->interval.GetLength() << " bucket numbers, "
       << this->GetNoTuples() << " tuples, "
       << this->GetTotalSize() << " bytes (Size), "
       << this->GetTotalExtSize() << " bytes (ExtSize)"
       << "InMemory: " << this->buffer->InMemory()
       << endl;

  //this->buffer->Print(os);

  return os;
}

/*
10 Implementation of class ~PartitionManager~

*/

size_t PartitionManager::IO_BUFFER_SIZE = WinUnix::getPageSize();

PartitionManager::PartitionManager( HashFunction* h,
                                    size_t opMem,
                                    size_t nBuckets,
                                    size_t nPartitions,
                                    size_t p0,
                                    PartitionManagerProgressInfo* pInfo )
: iter(0)
, hashFunc(h)
, maxOperatorMemory(opMem)
, checkProgressAfter(50)
, p0(p0)
, tuples(0)
, simSubpartitioning(true)
, progressInfo(pInfo)
{
  // calculate buckets per partition
  size_t step = nBuckets / nPartitions;
  size_t rest = nBuckets % nPartitions;
  size_t low = 0;

  // create partitions
  for(size_t i = 0; i < nPartitions; i++)
  {
    PInterval interval;

    if ( rest != 0 && i >= ( nPartitions - rest ))
    {
      interval = PInterval(low, low + step);
      low += step + 1;
    }
    else
    {
      interval = PInterval(low, low + (step-1));
      low += step;
    }

    size_t bufferSize = 0;

    // set buffer size of partition 0
    if ( i == 0 && p0 != UINT_MAX )
    {
      bufferSize = p0;
    }

    insertPartition(interval, bufferSize, IO_BUFFER_SIZE);
  }
}

PartitionManager::PartitionManager( HashFunction* h,
                                       PartitionManager& pm,
                                       PartitionManagerProgressInfo* pInfo )
: iter(0)
, hashFunc(h)
, maxOperatorMemory(pm.maxOperatorMemory)
, checkProgressAfter(pm.checkProgressAfter)
, p0(0)
, simSubpartitioning(false)
, progressInfo(pInfo)
{
  // create partitions with intervals from pm
  for(size_t i = 0; i < pm.partitions.size(); i++)
  {
    insertPartition(pm.partitions[i]->GetInterval(), 0, IO_BUFFER_SIZE);
  }
}

PartitionManager::~PartitionManager()
{
  for(size_t i = 0; i < partitions.size(); i++)
  {
    delete partitions[i];
  }
  partitions.clear();

  if ( iter )
  {
    delete iter;
    iter = 0;
  }

  if ( hashFunc )
  {
    delete hashFunc;
    hashFunc = 0;
  }

  progressInfo = 0;
}

size_t PartitionManager::Insert(Tuple* t)
{
  // calculate bucket number
  size_t b = hashFunc->Value(t);

  // find partition index
  size_t p = findPartition(b);

  // insert tuple into partition
  this->Insert(t,p,b);

  return p;
}

void PartitionManager::Insert(Tuple* t, size_t p, size_t b)
{
  // insert tuple into partition
  partitions[p]->Insert(t,b);

  tuples++;

  // update progress info if necessary
  if ( progressInfo != 0 )
  {
    progressInfo->partitionProgressInfo[p].tuples++;
    progressInfo->partitionProgressInfo[p].noOfPasses =
      (int)ceil( (double)partitions[p]->GetTotalExtSize()
          / (double)maxOperatorMemory );

    if ( simSubpartitioning == true && ( tuples % SUBPARTITION_UPDATE ) == 0 )
    {
      progressInfo->subTotalTuples =
          calcSubpartitionTupleCount( maxOperatorMemory,
                                      SUBPARTITION_MAX_LEVEL );
    }
  }
}

size_t PartitionManager::PartitionStream(Word stream)
{
  Tuple* t;
  size_t b, last, read = 0;
  size_t p = UINT_MAX;

  while ( ( t = readFromStream(stream) ) )
  {
    read++;

    // calculate bucket number
    b = hashFunc->Value(t);

    // determine partition if necessary
    if ( last != b || p == UINT_MAX )
    {
      p = findPartition(b);
    }

    // insert tuple into partition
    partitions[p]->Insert(t,b);

    if ( progressInfo != 0 )
    {
      progressInfo->partitionProgressInfo[p].tuples++;
      progressInfo->partitionProgressInfo[p].noOfPasses =
        (int)ceil( (double)partitions[p]->GetTotalExtSize()
            / (double)maxOperatorMemory );
    }

    // save last bucket number
    last = b;
  }

  return read;
}

void PartitionManager::Subpartition()
{
  // stop recalculation of sub-partitioning progress information
  simSubpartitioning = false;

  // Subpartition if necessary
  for(size_t i = 0; i < partitions.size(); i++)
  {
    subpartition(i, maxOperatorMemory, SUBPARTITION_MAX_LEVEL, 1);
  }

  // Sort partitions array
  PartitionCompareLesser cmp1;
  sort(partitions.begin(), partitions.end(), cmp1);

  // Sort partitions progress info array
  PartitionProgressInfoCompareLesser cmp2;
  sort( progressInfo->partitionProgressInfo.begin(),
        progressInfo->partitionProgressInfo.end(), cmp2 );
}

size_t PartitionManager::insertPartition ( PInterval intv,
                                            size_t buffer,
                                            size_t io,
                                            int index )
{
  assert(progressInfo != 0);

  size_t result;

  if ( index < 0 )
  {
    partitions.push_back( new Partition(intv, buffer, io) );
    progressInfo->partitionProgressInfo.push_back(PartitionProgressInfo(intv));
    result = partitions.size() - 1;
  }
  else
  {
    assert( index < (int)partitions.size() );
    partitions[index] = new Partition(intv, buffer, io);
    progressInfo->partitionProgressInfo[index] = PartitionProgressInfo(intv);
    result = index;
  }

  // return 0-based partition index
  return result;
}

void PartitionManager::subpartition( size_t n,
                                        size_t maxSize,
                                        int maxRecursion,
                                        int level )
{
  // check partition size
  if ( partitions[n]->GetTotalExtSize() <= maxSize )
  {
    if ( traceMode )
    {
      cmsg.info() << "Partition (" << n
                  << ") is smaller than available memory"
                  << ", no subpartitioning necessary"
                  << endl;
      cmsg.send();
    }
    partitions[n]->SetSubpartitioned();
    return;
  }

  // check if maximum recursion level is reached
  if ( level > maxRecursion )
  {
    if ( traceMode )
    {
      cmsg.info() << "Maximum recursion level ("
                  << maxRecursion << ") reached "
                  << "- subpartitioning stopped!"
                  << endl;
      cmsg.send();
    }
    partitions[n]->SetSubpartitioned();
    return;
  }

  // check if partition contains at least 4 buckets
  if ( partitions[n]->GetInterval().GetLength() < 4 )
  {
    if ( traceMode )
    {
      cmsg.info() << "Partition (" << n
                  << ") contains only "
                  << partitions[n]->GetInterval().GetLength()
                  << " buckets, minimum is 4!"
                  << endl;
      cmsg.send();
    }
    partitions[n]->SetSubpartitioned();
    return;
  }

  // store partition which is split locally
  Partition* p = partitions[n];

  // create two new partitions with half the interval size
  size_t low = p->GetInterval().GetLow();
  size_t high = p->GetInterval().GetHigh();
  size_t m = low + p->GetInterval().GetLength() / 2 - 1;

  size_t bufSize = ( n == 0 && p0 > 0 ) ? p0 : 0;

  PInterval i1(low, m);
  PInterval i2(m+1, high);

  size_t k = insertPartition(i1, bufSize, IO_BUFFER_SIZE, n);
  size_t l = insertPartition(i2, 0, IO_BUFFER_SIZE);

  // scan partition and put tuples in s1 or s2
  Tuple* t;
  size_t counter = 0;
  PartitionIterator* iter = p->MakeScan();

  while( ( t = iter->GetNextTuple() ) != 0 )
  {
    size_t b = hashFunc->Value(t);

    this->Insert(t, i1.IsAt(b) ? k : l, b);
    t->DeleteIfAllowed();

    // update progress information if necessary
    if ( progressInfo != 0 )
    {
      progressInfo->subTuples++;

      // propagate progress message ilevelf necessary
      if ( ( ++counter % SUBPARTITION_UPDATE ) == 0)
      {
        qp->CheckProgress();
      }
    }
  }

  if ( traceMode )
  {
    cmsg.info() << "Partition " << n << " is split into" << endl;
    cmsg.info() << n << ": ";
    p->Print(cmsg.info());
    cmsg.info() << k << ": ";
    partitions[k]->Print(cmsg.info());
    cmsg.info() << l << ": ";
    partitions[l]->Print(cmsg.info());
    cmsg.send();
  }

  // free iterator and partition
  delete iter;
  delete p;

  // recursive subpartitioning
  size_t level1 = level;
  size_t level2 = level;

  subpartition(k, maxSize, maxRecursion, ++level1);
  subpartition(l, maxSize, maxRecursion, ++level2);
}

int PartitionManager::calcSubpartitionTupleCount( size_t maxSize,
                                                  int maxRecursion )
{
  int count = 0;

  // Simulate sub-partitioning
  for(size_t i = 0; i < partitions.size(); i++)
  {
    count += simsubpartition( partitions[i]->GetPartitionHistogram(),
                              maxSize, maxRecursion, 1);
  }

  return count;
}

int PartitionManager::simsubpartition( PartitionHistogram& ph,
                                       size_t maxSize,
                                       int maxRecursion,
                                       int level )
{
  int counter = 0;

  if ( ph.GetTotalExtSize() <= maxSize )
  {
    return counter;
  }

  // check if maximum recursion level is reached
  if ( level > maxRecursion )
  {
    return counter;
  }

  // check if partition contains at least 4 hash function values
  if ( ph.GetInterval().GetLength() < 4 )
  {
    return counter;
  }

  size_t m = ( ph.GetSize() / 2 ) - 1;

  // create two new partition histograms with half the size
  PartitionHistogram ph1(ph, 0, m);
  PartitionHistogram ph2(ph, m+1, ph.GetSize() - 1);

  counter = ph.GetNoTuples();

  size_t level1 = level;
  size_t level2 = level;

  counter += simsubpartition(ph1, maxSize, maxRecursion, ++level1);
  counter += simsubpartition(ph1, maxSize, maxRecursion, ++level2);

  return counter;
}

void PartitionManager::InitPartitions(HashTable* h)
{
  for(size_t i = 0; i < h->GetNoBuckets(); i++)
  {
    vector<Tuple*> arr = h->GetTuples(i);

    for(size_t j = 0; j < arr.size(); j++)
    {
      Tuple* t = arr[j];

      this->Insert(t);
      t->DeleteIfAllowed();
    }
  }

  // free all tuples that were stored on disk
  h->Clear();
}

bool PartitionManager::LoadPartition( int n,
                                      HashTable* h,
                                      size_t maxMemory )
{
  assert(h);
  assert(n < (int)partitions.size());

  Tuple* t;
  size_t usedMemory = 0;

  // Clear hash table
  h->Clear();

  if ( iter == 0 )
  {
    // start new partition scan
    iter = partitions[n]->MakeScan();
  }

  static int counter = 0;

  while( ( t = iter->GetNextTuple() ) != 0 )
  {
    // insert tuple into hash table
    h->Insert(t);
    t->DeleteIfAllowed();

    // update used memory
    usedMemory += t->GetExtSize();

    counter++;

    // update processed tuples of partition
    if ( progressInfo != 0 )
    {
      progressInfo->partitionProgressInfo[n].tuplesProc++;
    }

    if ( usedMemory > maxMemory )
    {
      // memory is filled but partition is not finished
      if ( traceMode )
      {
        cmsg.info() << "LoadPartition(" << n << "): " << counter << " / "
                    << progressInfo->partitionProgressInfo[n].tuples
                    << " tuples"
                    << endl;
        cmsg.send();
      }
      return false;
    }
  }

  if ( traceMode )
  {
    cmsg.info() << "LoadPartition(" << n << "): " << counter << " / "
                << progressInfo->partitionProgressInfo[n].tuples
                << " tuples"
                << endl;
    cmsg.send();
  }

  counter = 0;
  delete iter;
  iter = 0;

  // partition is finished and fits into memory
  return true;
}

ostream& PartitionManager::Print(ostream& os)
{
  os << "-------------------- Partitioning -----------------------" << endl;

  for(size_t i = 0; i < partitions.size(); i++)
  {
    os << "Partition: " << i ;
    partitions[i]->Print(os);
  }

  return os;
}

/*
11 Implementation of class ~HybridHashJoinProgressLocalInfo~

*/

HybridHashJoinProgressLocalInfo::HybridHashJoinProgressLocalInfo()
: ProgressLocalInfo()
, maxOperatorMemory(qp->MemoryAvailableForOperator())
, traceMode(false)
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
    if ( traceMode )
    {
      cmsg.info() << "-----------------------------------------------" << endl;
      cmsg.info() << "calcProgressHybrid()" << endl;
      cmsg.send();
    }
    calcProgressHybrid(p1, p2, pRes, s);
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

void HybridHashJoinProgressLocalInfo::calcProgressHybrid( ProgressInfo& p1,
                                                          ProgressInfo& p2,
                                                          ProgressInfo* pRes,
                                                          Supplier s )
{
  double sel;
  double m = (double)this->returned;
  double k1 = (double)this->readFirst;
  double k2 = (double)this->readSecond;
  size_t M = this->maxOperatorMemory;
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
    size_t card0 = streamA.partitionProgressInfo[0].tuples;

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
    t3 -= card0 * ( t_read + t_write );

    if ( traceMode )
    {
      cmsg.info() << "3: t3 => " << t3 << endl;
      cmsg.send();
    }
  }

  // calculate time for partitioning and processing of stream B
  double t4 = p2.Card * ( t_hash + t_read + t_write )
               - min(streamB.partitionProgressInfo[0].tuples, M_S2)
                 * ( t_read + t_write );
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
    size_t card0 = streamA.partitionProgressInfo[0].tuples;
    size_t proc0 = streamA.partitionProgressInfo[0].tuplesProc;

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

    prog3 -= min(proc0, card0) * ( t_read + t_write );

    if ( traceMode )
    {
      cmsg.info() << "3: prog3 => " << prog3 << endl;
      cmsg.send();
    }
  }

  // calculate current progress of stream B
  double prog4 = streamB.GetTotalProcessedTuples()
                    * ( t_hash + t_read + t_write )
                  - min(streamB.partitionProgressInfo[0].tuples, M_S2)
                    * ( t_read +  t_write );

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
12 Implementation of class ~HybridHashJoinAlgorithm~

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
, firstPassPartition(true)
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
                << "I/O Buffer: \t\t\t" << PartitionManager::GetIOBufferSize()
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
    pmB = new PartitionManager( hashFuncB, MAX_MEMORY,
                                nBuckets, nPartitions,
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
      //cmsg.info() << "Hash Table content:" << endl
                  //<< *hashTable << endl;
      cmsg.send();
    }
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
    PartitionManager::SetIOBufferSize( WinUnix::getPageSize() );
  }
  else
  {
    // set buffer size
    PartitionManager::SetIOBufferSize(bytes);
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

  progress->maxOperatorMemory = MAX_MEMORY;
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
    t->DeleteIfAllowed();
  }

  return;
}

Tuple* HybridHashJoinAlgorithm::partitionA()
{
  while ( tupleA.tuple )
  {
    size_t p;

    if ( ( p = pmA->FindPartition(tupleA.tuple) ) == 0 )
    {
      // Immediately process partition 0
      Tuple* tupleB = hashTable->Probe(tupleA.tuple);

      if ( tupleB )
      {
        // bucket contains match -> build result tuple
        Tuple *result = new Tuple( resultTupleType );
        Concat( tupleA.tuple, tupleB, result );
        progress->returned++;
        return result;
      }
      else // bucket completely processed
      {
        // if partition 0 overflows store tuple on disc
        if ( !finishedPartitionB )
        {
          pmA->Insert(tupleA.tuple);

          //if ( progress->streamA.IsValid() )
          {
            progress->streamA.partitionProgressInfo[0].tuplesProc++;
          }
        }
        else
        {
          //if ( progress->streamA.IsValid() )
          {
            progress->streamA.partitionProgressInfo[0].tuples++;
            progress->streamA.partitionProgressInfo[0].tuplesProc++;
          }
        }

        //if ( progress->streamA.IsValid() )
        {
          PartitionProgressInfo& pinfo =
              progress->streamA.partitionProgressInfo[0];
          pinfo.curPassNo =
              (int)ceil( (double)pinfo.tuplesProc / (double)pinfo.tuples );
        }
      }
    }
    else
    {
      // insert tuple into partition p ( p > 0 )
      pmA->Insert(tupleA.tuple);
    }

    tupleA.setTuple( nextTupleA() );
  }

  // change state
  partitioning = false;

  if ( finishedPartitionB )
  {
    curPartition++;
  }
  else
  {
    firstPassPartition = false;
  }

  // load next partition into memory
  finishedPartitionB = pmB->LoadPartition(curPartition, hashTable, MAX_MEMORY);

  if ( traceMode )
  {
    //cmsg.info() << "Hash table content" << *hashTable << endl;
    cmsg.info() << "Partitioning of stream A.." << endl << *pmA
                << "finishedPartitionB" << finishedPartitionB
                << endl;
    cmsg.send();
  }

  // start scan of corresponding partition A
  iterA = pmA->GetPartition(curPartition)->MakeScan();

  tupleA.setTuple( iterA->GetNextTuple() );

  return processPartitions();
}

Tuple* HybridHashJoinAlgorithm::processPartitions()
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
        return result;
      }

//      if ( progress->streamA.IsValid() )
      {
        PartitionProgressInfo& pinfo =
            progress->streamA.partitionProgressInfo[curPartition];

        pinfo.tuplesProc++;

        pinfo.curPassNo =
            (int)ceil( (double)pinfo.tuplesProc / (double) pinfo.tuples);
      }

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
      iterA = pmA->GetPartition(curPartition)->MakeScan();

      // Read first tuple from A(i)
      tupleA.setTuple( iterA->GetNextTuple() );
    }
  }

  return 0;
}
Tuple* HybridHashJoinAlgorithm::NextResultTuple()
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
13 Implementation of value mapping function of operator ~hybridhashjoin~

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
14 Instantiation of Template Functions

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
