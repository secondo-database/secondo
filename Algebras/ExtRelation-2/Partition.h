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

1 Header File Partition.h

June 2009, Sven Jungnickel. Initial version

2 Includes and defines

*/

#ifndef EXTREl2_PARTITION_H_
#define EXTREl2_PARTITION_H_

#include "RelationAlgebra.h"
#include "TupleBuffer2.h"
#include "HybridHashJoinHashTable.h"

#define HEADLINE_PHISTOGRAM "-------------------- " \
                             "PartitionHistogram -----------------"

#define SUBPARTITION_UPDATE 100
#define SUBPARTITION_MAX_LEVEL 3

/*
3 Template class ~Interval~

This class represents an interval where upper
and lower bound of the interval are included by
the interval.

*/
namespace extrel2
{
template<typename T> class Interval
{
  public:

    Interval()
    : low(0)
    , high(0)
    {
    }
/*
First constructor. Creates an empty instance.

*/

    Interval(T low, T high)
    : low(low)
    , high(high)
    {
      assert(low <= high);
    }
/*
Second constructor. Creates a new instance and sets
lower bound to ~low~ and upper bound to ~high~.

*/

    Interval(const Interval& i)
    : low(i.low)
    , high(i.high)
    {
    }
/*
Copy constructor.

*/

    Interval& operator=(const Interval& obj)
    {
      if ( this == &obj )
        return *this;

      this->low = obj.low;
      this->high = obj.high;

      return *this;
    }
/*
Assignment operator.

*/

    inline bool IsAt(T n)
    {
      return ( low <= n && n <= high );
    }
/*
Returns true if value ~n~ lies inside the interval.
Otherwise false is returned.

*/

    inline T GetLength()
    {
      return ((high - low) + 1);
    }
/*
Get length of interval.

*/

    inline T GetLow() { return low; }
/*
Get lower bound of interval.

*/

    inline T GetHigh() { return high; }
/*
Get upper bound of interval.

*/

  private:

    T low;
/*
Lower bound of interval.

*/

    T high;
/*
Upper bound of interval.

*/
};

typedef Interval<size_t> PInterval;
/*
Type definition of a partition interval.

*/

/*
4 Class ~PartitionInfo~

Statistical information for a partition.

*/

typedef struct PartitionHistogramEntry
{
  PartitionHistogramEntry()
  : value(0)
  , count(0)
  , totalSize(0)
  , totalExtSize(0)
  {}
/*
The first constructor. Creates an empty instance.

*/

  PartitionHistogramEntry( size_t value,
                           size_t count,
                           size_t totalSize,
                           size_t totalExtSize )
  : value(0)
  , count(count)
  , totalSize(totalSize)
  , totalExtSize(totalExtSize)
  {}
/*
The second constructor. Creates an instance with the specified values.

*/

  PartitionHistogramEntry(const PartitionHistogramEntry& rhs)
  {
    value = rhs.value;
    count = rhs.count;
    totalSize = rhs.totalSize;
    totalExtSize = rhs.totalExtSize;
  }
/*
Copy constructor.

*/

  size_t value;
/*
Hash value

*/

  size_t count;
/*
Number of values within a partition.

*/

  size_t totalSize;
/*
Total tuple size for a partition including LOBs.

*/

  size_t totalExtSize;
/*
Size of all core and extension parts for a partition.

*/

};
/*
Type definition of a partition histogram entry.

*/

class PartitionHistogram
{
  public:

    PartitionHistogram(PInterval& i);
/*
First constructor. Creates a histogram for partition interval i.

*/

    PartitionHistogram(PartitionHistogram& obj, size_t start, size_t end);
/*
Second constructor. Creates a histogram from an existing histogram
copying the histogram entries between 0-based index ~start~ and ~end~

*/

    void Insert(Tuple* t, size_t hashFuncValue);
/*
Insert tuple ~t~ with hash value ~h~ into histogram.

*/

    PInterval& GetInterval() { return interval; }
/*
Returns the number of tuple in a partition histogram.

*/

    size_t GetSize() { return interval.GetLength(); }
/*
Returns the size of a partition histogram.

*/

    PartitionHistogramEntry& GetHistogramEntry(size_t n);
/*
Returns the histogram entry with index ~n~.

*/

    int GetNoTuples() { return tuples; }
/*
Returns the number of tuple in a partition histogram.

*/

    size_t GetTotalSize() { return totalSize; }
/*
Returns the total size of all tuples in a partition histogram
including LOBs.

*/

    size_t GetTotalExtSize() { return totalExtSize; }
/*
Returns the core and extension part size of all tuples in a
partition histogram.

*/

    ostream& Print(ostream& os)
    {
      cmsg.info() << HEADLINE_PHISTOGRAM << endl;

      for(size_t i = 0; i < data.size(); i++)
      {
        cmsg.info() << "Value: " << data[i].value
                    << ", Tuples: " << data[i].count
                    << ", totalSize: " << data[i].totalSize
                    << ", totalExtSize: " << data[i].totalExtSize
                    << endl;
      }

      cmsg.send();

      return os;
    }
/*
Print partition histogram content to a stream (for debugging purposes).

*/
  private:

    PInterval interval;
/*
Partition interval

*/

    std::vector<PartitionHistogramEntry> data;
/*
Histogram data.

*/

    int tuples;
/*
Total number of tuples within the histogram.

*/

    size_t totalSize;
/*
Size of all tuples in the histogram including FLOBs and LOBs.

*/

    size_t totalExtSize;
/*
Size of all tuples in the histogram including only FLOBs (no LOBs).

*/

};

/*
Type definition of a partition histogram.

*/

class PartitionProgressInfo
{
  public:

    PartitionProgressInfo()
    : tuples(0)
    , tuplesProc(0)
    , noOfPasses(0)
    {
    }
/*
The constructor. Creates an empty instance.

*/

    PartitionProgressInfo(const PartitionProgressInfo& obj)
    {
      if ( this == &obj )
        return;

      this->tuples = obj.tuples;
      this->tuplesProc = obj.tuplesProc;
      this->noOfPasses = obj.noOfPasses;
    }
/*
Copy constructor.

*/

    ostream& Print(ostream& os)
    {
      os << "Tuples: " << tuples
         << ", TuplesProc: " << tuplesProc
         << ", noOfPasses: " << noOfPasses
         << endl;

      return os;
    }
/*
Print to stream ~os~. This function is used
for debugging purposes.

*/

    size_t tuples;
/*
Number of tuples of a partition.

*/

    size_t tuplesProc;
/*
Current number of tuples processed during join operation.

*/

    int noOfPasses;
/*
Number of passes necessary to process a partition. This field
is only used for partitions from stream B.


*/
};

class PartitionManagerProgressInfo
{
  public:

    PartitionManagerProgressInfo()
    : subTotalTuples(0)
    , subTuples(0)
    {
    }
/*
The constructor. Creates an empty instance.

*/

    bool IsValid() { return !partitionProgressInfo.empty(); }
/*
Returns ~true~ if progress information is available and valid.
Otherwise ~false~ is returned.

*/

    ostream& Print(ostream& os)
    {
      os << "PartitionManagerProgressInfo" << endl
         << "subTotalTuples: " << subTotalTuples
         << ", subTuples: " << subTuples
         << endl;

      for (size_t i = 0; i < partitionProgressInfo.size(); i++)
      {
        os << "Partition: " << i << " - ";
        partitionProgressInfo[i].Print(os);
      }

      return os;
    }

/*
Print to stream ~os~. This function is used
for debugging purposes.

*/

    vector<PartitionProgressInfo> partitionProgressInfo;
/*
Vector with progress information for each partition

*/

    size_t subTotalTuples;
/*
Total number of tuples to process during sub-partitioning.

*/

    size_t subTuples;
/*
Number of tuples already processed during sub-partitioning.

*/

};

/*
Global operator << for ~PartitionInfo~.

*/

/*
5 Class ~Partition~

This class represents a partition of the hybrid hash join algorithm.
Each partition
Instances of this class are used for temporary storage of tuples
that fall into the same hash function value interval.

*/
class PartitionIterator;
/*
Necessary forward declaration for class ~Partition~.

*/

class Partition
{
  public:

    Partition(PInterval i, size_t bufferSize);
/*
The constructor. Creates a new partition with interval ~i~
and with an internal memory buffer of ~bufferSize~ bytes.

*/

    ~Partition();
/*
The destructor. Free the partition's resources.

*/

    inline int GetNoTuples()
    {
      return buffer->GetNoTuples();
    }
/*
Returns the number of tuples for a partition.

*/

    inline size_t GetTotalSize()
    {
      return buffer->GetTotalSize();
    }
/*
Returns the partition size in bytes including LOBs.

*/

    inline size_t GetTotalExtSize()
    {
      return buffer->GetTotalExtSize();
    }
/*
Returns the partition size in bytes including FLOBs.

*/

    inline bool Overflows()
    {
      return ( this->GetTotalExtSize() > qp->MemoryAvailableForOperator() );
    }
/*
Returns true if the partition's size exceeds ~maxMemorySize~.

*/

    void Insert(Tuple* t, size_t hashFuncValue);
/*
Insert a tuple into a partition.

*/

    inline PInterval& GetInterval()
    {
      return interval;
    }
/*
Returns the interval for a partition.

*/

    PartitionIterator* MakeScan();
/*
Starts a sequential scan of the partition's tuples. The method
returns a new ~PartitionIterator~ instance that can be used
to read the tuples in sequential order.

*/

    inline void SetSubpartitioned() { subpartitioned = true; }
/*
Mark a partition as sub-partitioned.

*/

    inline PartitionHistogram& GetPartitionHistogram()
    {
      return histogram;
    }
/*
Returns the progress information for a partition.

*/

    ostream& Print(ostream& os);
/*
Print the partition info to stream ~os~. This function is used
for debugging purposes.

*/

    friend class PartitionIterator;
/*
~PartitionIterator~ is declared as a friend class, so that
the iterator may access the internal buffer.

*/

  private:

    PInterval interval;
/*
Interval of hash function values.

*/

    TupleBuffer2* buffer;
/*
Tuple buffer for temporary storage in-memory and on disk.

*/

    PartitionHistogram histogram;
/*
Statistical partition information for a partition. Used for
debugging and progress estimation.

*/

    bool subpartitioned;
/*
Flag which indicates whether a partition has already gone
through the sub-partitioning algorithm or not.

*/
};

/*
6 Class ~PartitionIterator~

Iterator class used for a sequential scan of a partition's
tuples.

*/
class PartitionIterator
{
  public:

    PartitionIterator(Partition& p)
    {
      iter = p.buffer->MakeScan();
    }
/*
The constructor. Starts a sequential scan of partition ~p~.

*/

    ~PartitionIterator()
    {
      delete iter;
      iter = 0;
    }
/*
The destructor.

*/

    inline Tuple* GetNextTuple()
    {
      return iter->GetNextTuple();
    }
/*
Returns the next tuple of a partition in sequential order. If all
tuples have been processed 0 is returned.

*/

  private:

    TupleBuffer2Iterator* iter;
/*
Iterator for the internal buffer of a partition.

*/
};

class PartitionCompareLesser :
  binary_function<Partition*, Partition*, bool>
{
  public:

    inline bool operator()(Partition* a, Partition* b)
    {
      return ( a->GetInterval().GetLow() <
                b->GetInterval().GetLow() );
    }
};

/*
7 Class ~PartitionManager~

Class which represents the partitioning of a complete stream.

*/
class PartitionManager
{
  public:

    PartitionManager( HashFunction* h,
                       size_t buckets,
                       size_t partitions,
                       size_t p0 = UINT_MAX,
                       PartitionManagerProgressInfo* pInfo = NULL );

/*
Creates an equal spaced partitioning with ~partitions~ partitions. Each
partition holds about ~buckets~/~partitions~ hash function values.
The interval ranges for the partitions are calculated equally spaced.
Parameter ~p0~ specifies the memory buffer size for partition 0. If ~p0~
is set to UINT\_MAX no tuples will be buffered in memory, like it is
default for all other partitions. If 0 < ~p0~ < UINT\_MAX the specified
buffer size ~p0~ in bytes will be used.

*/

    PartitionManager( HashFunction* h,
                       PartitionManager& pm,
                       PartitionManagerProgressInfo* pInfo = NULL );

    ~PartitionManager();
/*
The destructor. Deletes all partitions.

*/

    size_t PartitionStream(Word stream);
/*
Partitions the stream ~stream~ completely and returns the number
of processed tuples.

*/

    void Insert(Tuple* t);
/*
Inserts tuple ~t~ into the partitioning. The corresponding partition
is automatically determined by the partition manager. This method
is used if the partition is not known.

*/

    size_t FindPartition(Tuple* t)
    {
      size_t b = hashFunc->Value(t);
      return findPartition(b);
    }
/*
Return the partition number for tuple ~t~. If the partition
cannot be found ~UINT\_MAX~ is returned.

*/

    Partition* GetPartition(int n)
    {
      assert( n < this->GetNoPartitions() );
      return partitions[n];
    }
/*
Return a pointer to partition ~n~.

*/

    void Subpartition();
/*
This methods checks if the size of all partitions is lower or equal to
~maxSize~. If any partitions exceeds ~maxSize~ the partition is split
into sub-partitions until each sub-partition has the correct size.
The maximum level of recursion is limited by ~maxRecursion~. If a
partition doesn't fit into memory after ~maxRecursion~ recursion levels
the partition has to be processed using the standard external hash join
algorithm.

*/

    void InitPartitions(HashTable* h);
/*
Loads the buckets of hash table ~h~ into the corresponding partitions.
This method is used to load the content of the first memory charge
into the partitions, when switching from internal standard hash-join
to external hybrid hash-join.

*/

    bool LoadPartition(int n, HashTable* h, size_t maxMemory);
/*
Loads partition ~n~ into the hash table ~h~. Method returns true if the
partition fits into memory. If the partition size exceeds the available
main memory ~maxMemory~ false is returned.

*/

    inline int GetNoPartitions()
    {
      return (int)partitions.size();
    }
/*
Returns the number of partitions.

*/

    ostream& Print(ostream& os);
/*
Print the partitioning to stream ~os~. This function is used
for debugging purposes.

*/

  private:

    void subpartition( size_t n, size_t maxSize,
                       int maxRecursion, int level );
/*
Sub-partition partition ~n~ into partitions that are maximal
~maxSize~ bytes big using ~maxRecursion~ levels. ~level~
is the current recursion level.

*/

    int simsubpartition( PartitionHistogram& ph, size_t maxSize,
                         int maxRecursion, int level );
/*
Simulate sub-partitioning for a partition with partition histogram ~ph~
into partitions that are maximal ~maxSize~ bytes big using
~maxRecursion~ levels. ~level~ is the current recursion level.

*/

    int calcSubpartitionTupleCount(size_t maxSize, int maxRecursion);
/*
Returns the number of tuples that must be processed during sub-partitioning
when ~maxSize~ memory is available and a maximum recursion level of
~maxRecursion~ is allowed.

*/
    inline Tuple* readFromStream(Word stream)
    {
      Word wTuple(Address(0));

      qp->Request(stream.addr, wTuple);

      if ( qp->Received(stream.addr) )
      {
        return static_cast<Tuple*>( wTuple.addr );
      }

      return 0;
    }
/*
Read the next tuple from stream ~stream~. If there are
no more tuples in the stream 0 is returned.

*/

    inline size_t findPartition(size_t bucket)
    {
      size_t l = 0, x;
      size_t r = partitions.size() - 1;

      while ( r >= l )
      {
        x = ( l + r ) / 2;

        if ( bucket < partitions[x]->GetInterval().GetLow() )
        {
          r = x - 1;
        }
        else
        {
          l = x + 1;
        }

        if ( partitions[x]->GetInterval().IsAt(bucket) )
        {
          return x;
        }
      }

      return UINT_MAX;
    }
/*
Find the partition number of bucket ~bucket~. A binary search is
performed to find the correct partition interval. The method
returns the partition number of the partition that holds an interval
containing ~bucket~

*/

    PartitionIterator* iter;
/*
Partition iterator which is used when a partition cannot
be loaded into memory at once.

*/

    HashFunction* hashFunc;
/*
Hash function which is used to calculate the bucket number.

*/

    vector<Partition*> partitions;
/*
Array of all partitions.

*/

    size_t p0;
/*
Buffer size for partition 0 in bytes.

*/

    size_t tuples;
/*
Tuple counter. Necessary for recalculating the number of tuples
which must be processed during subpartitioning.

*/

    bool subpartitioned;
/*
Flag which indicates if sub-partitioning of stream A has already been
performed. If set to true sub-partitioning of stream B ha been finished.

*/

    PartitionManagerProgressInfo* progressInfo;
/*
Pointer to progress information. If ~progressInfo~ is set to NULL
no progress information will be collected.

*/

    static const bool traceMode = true;
/*
Flag to enable trace mode.

*/
};

/*
8 Global operators

*/

inline ostream& operator<<(ostream& os, PartitionManager& pm)
{
  return pm.Print(os);
}
/*
Print the partitioning of a tuple stream to stream ~os~. This function
is only used for debugging purposes.

*/

} // end of namespace extrel2

#endif /* EXTREl2_PARTITION_H_ */
