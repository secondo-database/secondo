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

1 Implementation File Partition.cpp

June 2009, Sven Jungnickel. Initial version

2 Includes and defines

*/

#include "Partition.h"

/*
3 Implementation of class ~PartitionProgressInfo~

*/
namespace extrel2
{

PartitionInfo::PartitionInfo(size_t n)
: subpartitioned(false)
, tuples(0)
, tupleSizes(0)
, noOfPasses(0)
, subTotalTuples(0)
, subTuples(0)
{
  for(size_t i = 0; i < n; i++)
  {
    histogram.push_back(PartitionHistogramEntry());
  }
}

PartitionInfo::PartitionInfo(PartitionInfo& obj)
{
  if ( this == &obj )
    return;

  tuples = obj.tuples;
  tupleSizes = obj.tupleSizes;
  noOfPasses = obj.noOfPasses;
  subTotalTuples = obj.subTotalTuples;
  subTuples = obj.subTuples;

  for(size_t i = 0; i < obj.histogram.size(); i++)
  {
    histogram.push_back(obj.histogram[i]);
  }
}

ostream& PartitionInfo::Print(ostream& os)
{
  os << "------------ PartitionInfo --------------" << endl
     << "tuples: " << tuples
     << ", tupleSizes: " << tupleSizes
     << ", noOfPasses: " << noOfPasses << endl
     << "subTotalTuples: " << subTotalTuples
     << ", subTuples: " << subTuples << endl;

  return os;
}

/*
4 Implementation of class ~Partition~

*/

Partition::Partition(PInterval i, size_t bufferSize)
: interval(i)
, pinfo(i.GetLength())
{
  buffer = new TupleBuffer(bufferSize);
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

ostream& Partition::Print(ostream& os)
{
    os << "[" << interval.GetLow() << ", "
       << interval.GetHigh() << "] -> "
       << interval.GetLength() << " bucket numbers, "
       << this->GetNoTuples() << " tuples, "
       << this->GetSize() << " bytes"
       << endl;

  return os;
}

/*
5 Implementation of class ~PartitionManager~

*/
PartitionManager::PartitionManager( HashFunction* h,
                                    size_t nBuckets,
                                    size_t nPartitions,
                                    size_t p0 )
: iter(0)
, hashFunc(h)
, p0(p0)
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

    partitions.push_back( new Partition(interval, bufferSize) );
  }
}

PartitionManager::PartitionManager(HashFunction* h, PartitionManager& pm)
: iter(0)
, hashFunc(h)
, p0(0)
{
  // create partitions with intervals from pm
  for(size_t i = 0; i < pm.partitions.size(); i++)
  {
    partitions.push_back( new Partition(pm.partitions[i]->GetInterval(), 0) );
  }
}

PartitionManager::~PartitionManager()
{
  for(size_t i = 0; i < partitions.size(); i++)
  {
    delete partitions[i];
  }

  partitions.clear();
}

void PartitionManager::Insert(Tuple* t)
{
  // calculate bucket number
  size_t b = hashFunc->Value(t);

  // find partition index
  size_t p = findPartition(b);

  // insert tuple into partition
  partitions[p]->Insert(t,b);
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

    // save last bucket number
    last = b;
  }

  return read;
}

void PartitionManager::Subpartition(size_t maxSize, int maxRecursion)
{
  // Subpartition if necessary
  for(size_t i = 0; i < partitions.size(); i++)
  {
    subpartition(i, maxSize, maxRecursion, 1);
  }

  // Sort partitions array
  PartitionCompareLesser cmp;
  sort(partitions.begin(), partitions.end(), cmp);
}

void PartitionManager::subpartition( size_t n,
                                        size_t maxSize,
                                        int maxRecursion,
                                        int level )
{
  // check partition size
  if ( partitions[n]->GetSize() <= maxSize )
  {
    partitions[n]->GetPartitionInfo().subpartitioned = true;
    return;
  }

  // check if maximum recursion level is reached
  if ( level > maxRecursion )
  {
    partitions[n]->GetPartitionInfo().subpartitioned = true;
    return;
  }

  // check if partition contains at least 4 buckets
  if ( partitions[n]->GetInterval().GetLength() < 4 )
  {
    partitions[n]->GetPartitionInfo().subpartitioned = true;
    return;
  }

  // create two new partitions with half the interval size
  size_t low = partitions[n]->GetInterval().GetLow();
  size_t high = partitions[n]->GetInterval().GetHigh();
  size_t m = low + partitions[n]->GetInterval().GetLength() / 2 - 1;

  PInterval i1 = PInterval(low, m);
  PInterval i2 = PInterval(m+1, high);

  Partition* s1 = new Partition(i1, ( n == 0 && p0 > 0 ) ? p0 : 0 );
  Partition* s2 = new Partition(i2, 0);

  if ( traceMode )
  {
    cmsg.info() << "Subpartition of partition " << n << endl;
    cmsg.info() << n << ": ";
    partitions[n]->Print(cmsg.info());
    cmsg.info() << "New partitions " << n << endl;
    cmsg.info() << "s1: ";
    s1->Print(cmsg.info());
    cmsg.info() << "s2: ";
    s2->Print(cmsg.info());
    cmsg.send();
  }

  // scan partition and put tuples in s1 or s2
  PartitionIterator* iter = partitions[n]->MakeScan();

  Tuple* t;
  size_t counter = 0;
  while( ( t = iter->GetNextTuple() ) != 0 )
  {
    size_t h = hashFunc->Value(t);

    if ( i1.IsAt(h) )
    {
      s1->Insert(t,h);
    }
    else
    {
      s2->Insert(t,h);
    }

    // update progress information
    partitions[n]->GetPartitionInfo().subTuples++;

    // propagate progress message if necessary
    if ( ( counter++ % 200 ) == 0)
    {
      qp->CheckProgress();
    }
  }

  // delete empty partitions, store new ones and subpartition
  // if necessary (Note: only one partition can be empty)
  if ( s1->GetNoTuples() == 0 && s2->GetNoTuples() > 0 )
  {
    delete s1;
    delete partitions[n];
    partitions[n] = s2;
    subpartition(n, maxSize, maxRecursion, ++level);
  }
  else if ( s1->GetNoTuples() > 0 && s2->GetNoTuples() == 0 )
  {
    delete s2;
    delete partitions[n];
    partitions[n] = s1;
    subpartition(n, maxSize, maxRecursion, ++level);
  }
  else
  {
    delete partitions[n];
    partitions[n] = s1;
    partitions.push_back(s2);
    size_t level1 = level;
    size_t level2 = level;
    subpartition(n, maxSize, maxRecursion, ++level1);
    subpartition(partitions.size()-1, maxSize, maxRecursion, ++level2);
  }
}

void PartitionManager::InitPartitions(HashTable* h)
{
  size_t p = 0;

  for(size_t i = 0; i < h->GetNoBuckets(); i++)
  {
    if ( !partitions[p]->GetInterval().IsAt(i) )
    {
      p++;
    }

    vector<Tuple*> arr = h->GetTuples(i);

    for(size_t j = 0; j < arr.size(); j++)
    {
      partitions[p]->Insert(arr[j],i);
    }
  }
}

bool PartitionManager::LoadPartition(int n, HashTable* h, size_t maxMemory)
{
  assert(h);
  assert(n < (int)partitions.size());

  Tuple* t;

  // Clear hash table
  h->Clear();

  if ( iter == 0 )
  {
    // start new partition scan
    iter = partitions[n]->MakeScan();
  }

  while( ( t = iter->GetNextTuple() ) != 0 )
  {
    h->Insert(t);
    maxMemory -= t->GetSize();

    if (maxMemory <= 0 )
    {
      // memory is filled but partition is not finished
      return false;
    }
  }

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
