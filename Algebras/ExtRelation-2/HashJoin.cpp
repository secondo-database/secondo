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
#include "HashJoin.h"

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

BucketIterator* Bucket::MakeScan()
{
  return new BucketIterator(*this);
}

/*
6 Implementation of class ~BucketIterator~

*/
BucketIterator::BucketIterator(Bucket& b)
: bucket(b)
{
  iter = bucket.tuples.begin();
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

    bytes += t->GetMemSize();

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

vector<Tuple*> HashTable::GetTuples(int bucket)
{
  Tuple* t;
  vector<Tuple*> arr;

  BucketIterator* iter = buckets[bucket]->MakeScan();

  while ( ( t = iter->GetNextTuple() ) != 0 )
  {
    t->IncReference();
    arr.push_back(t);
  }
  delete iter;
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

  // determine number of partitions (constant value is needed here
  // otherwise, subpartitioned partitions will be subpartioned again
  // with maximum recursion level)
  const size_t n = partitions.size();

  // Subpartition if necessary
  for(size_t i = 0; i < n; i++)
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

      // propagate progress message if necessary
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
  counter += simsubpartition(ph2, maxSize, maxRecursion, ++level2);

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
    usedMemory += t->GetMemSize();

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

} // end of namespace extrel2
