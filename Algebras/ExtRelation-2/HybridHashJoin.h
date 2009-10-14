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

#include "RelationAlgebra.h"
#include "RTuple.h"
#include "Progress.h"
#include "TupleBuffer2.h"

/*
4 Defines

*/

#define HEADLINE_PHISTOGRAM "-------------------- " \
                             "PartitionHistogram -----------------"
/*
Headline for output of trace information.

*/

#define SUBPARTITION_UPDATE 100
/*
The number of processed tuples during sub-partitioning after which the
progress information is updated by a call of the query processors method
CheckProgress().

*/

#define SUBPARTITION_MAX_LEVEL 3
/*
Minimum number of buckets in hash table.

*/

#define HYBRIDHASHJOIN_MINIMUM_BUCKETS      3
/*
Minimum number of buckets in hash table.

*/

#define HYBRIDHASHJOIN_MAXIMUM_BUCKETS      16384
/*
Maximum number of buckets in hash table.

*/

#define HYBRIDHASHJOIN_DEFAULT_BUCKETS      1000
/*
Default number of buckets in hash table if
a value below ~HYBRIDHASHJOIN\_MINIMUM\_BUCKETS~ or higher than
~HYBRIDHASHJOIN\_MAXIMUM\_BUCKETS~ has been specified.

*/

#define HYBRIDHASHJOIN_MINIMUM_PARTITIONS   1
/*
Minimum number of partitions.

*/

#define HYBRIDHASHJOIN_MAXIMUM_PARTITIONS   8192
/*
Maximum number of partitions (~HYBRIDHASHJOIN\_MAXIMUM\_BUCKETS~/2).

*/

#define HYBRIDHASHJOIN_DEFAULT_PARTITIONS   50
/*
Default number of partitions for operator ~hybridhashjoinParam~ if
a value below ~HYBRIDHASHJOIN\_MINIMUM\_PARTITIONS~ or higher than
~HYBRIDHASHJOIN\_MAXIMUM\_PARTITIONS~ has been specified.

*/

#define HYBRIDHASHJOIN_MINIMUM_MEMORY       1024
/*
Minimum operator memory in bytes that may be specified for
operator ~hybridhashjoinParam~.

*/

#define HYBRIDHASHJOIN_MAXIMUM_MEMORY       ( 64 * 1024 * 1024 )
/*
Maximum operator memory in bytes that may be specified for
operator ~hybridhashjoinParam~.

*/

#define HYBRIDHASHJOIN_DEFAULT_MEMORY       ( 16 * 1024 * 1024 )
/*
Default operator memory in bytes for operator ~hybridhashjoinParam~ if
a value below ~HYBRIDHASHJOIN\_MINIMUM\_MEMORY~ or higher than
~HYBRIDHASHJOIN\_MAXIMUM\_MEMORY~ has been specified.

*/

/*
5 Class ~JoinTupleCompareFunction~

Comparison function class for tuples that shall be joined
according to one join attribute.

*/
namespace extrel2
{

class JoinTupleCompareFunction
{
  public:

    JoinTupleCompareFunction(int attrIndexA, int attrIndexB)
    : attrIndexA(attrIndexA)
    , attrIndexB(attrIndexB)
    {}
/*
The constructor. Assigns the attribute indices of the join
attributes ~attrIndexA~ and ~attrIndexB~ to the new instance.

*/

    inline int Compare(Tuple* a, Tuple* b)
    {
      /* tuples with NULL-Values in the join attributes
         are never matched with other tuples. */
      if(!((Attribute*)a->GetAttribute(attrIndexA))->IsDefined())
      {
        return -1;
      }

      if(!((Attribute*)b->GetAttribute(attrIndexB))->IsDefined())
      {
        return 1;
      }

      return ((Attribute*)a->GetAttribute(attrIndexA))->
        Compare((Attribute*)b->GetAttribute(attrIndexB));
    }
/*
Compares the join attributes of tuples ~a~ and ~b~. Returns -1 if
the join attribute of tuple ~a~ is smaller than that of tuple ~b~ or if
the join attribute of ~a~ is not defined. Returns 1 if the join attribute
of ~a~ is greater than that of ~b~ or if the join attribute of ~b~ is not
defined. If the join attributes of both tuples are equal the method
returns 0.

*/

  private:

    int attrIndexA;
/*
Join attribute index for tuples from A.

*/

    int attrIndexB;
/*
Join attribute index for tuples from B.

*/
};

/*
6 Class ~HashFunction~

Class that represents a standard hash join function.
The hash function distributes tuples over the range of the
hash function by using the modulo operator.

*/
class HashFunction
{
  public:

    HashFunction(size_t nBuckets, int attrIndex)
    : nBuckets(nBuckets)
    , attrIndex(attrIndex)
    {}
/*
The constructor. Creates an instance, sets the number of
buckets to ~nBuckets~ and sets the attribute index of the
attribute for which the hash value is calculated.

*/

    HashFunction(const HashFunction& func)
    : nBuckets(func.nBuckets)
    , attrIndex(func.attrIndex)
    {
    }
/*
Copy constructor.

*/

    inline size_t Value(Tuple* t)
    {
      assert(t);
      StandardAttribute* attr;
      attr = static_cast<StandardAttribute*>(t->GetAttribute(attrIndex));
      return ( attr->HashValue() % nBuckets );
    }
/*
Calculate the hash function value for tuple ~t~ using the
hash value of the join attribute with index ~attrIndex~.

*/

    inline int GetAttributeIndex() { return attrIndex; }
/*
Returns the attribute index for which the hash function values
are calculated.

*/

  private:

    size_t nBuckets;
/*
Number of buckets.

*/

    int attrIndex;
/*
Attribute index for which the hash function values are calculated.
are calculated.

*/
};

/*
7 Class ~BucketIterator~

Iterator class which is used to iterate sequentially through all tuples
of a bucket from a hash table.

*/

class Bucket;
/*
Necessary forward declaration for class ~BucketIterator~.

*/

class BucketIterator
{
  public:

    BucketIterator(Bucket& b);
/*
The constructor. Starts a sequential scan for bucket ~b~.

*/

    Tuple* GetNextTuple();
/*
Returns the next tuple in sequential order. If all tuples
have been processed 0 is returned.

*/

  private:

    Bucket& bucket;
/*
Reference to bucket on which the instance iterates.

*/

    vector<Tuple*>::iterator iter;
/*
Iterator for internal bucket tuple buffer.

*/
};

/*
8 Class ~Bucket~

This class represents a bucket of a hash table.

*/

class Bucket
{
  public:

    Bucket(int no) : number(no), totalSize(0) {}
/*
The constructor. Creates an instance and sets the bucket number ~no~.

*/

    ~Bucket()
    {
      Clear();
    }
/*
The destructor.

*/

    inline void Clear()
    {
      for (size_t i = 0; i < tuples.size(); i++)
      {
        Tuple* t = tuples[i];
        t->DeleteIfAllowed();
      }

      tuples.clear();
      totalSize = 0;
    }
/*
Removes all tuples from a bucket. The reference counter
of all tuples is automatically decremented by one by the
destructor call of the ~RTuple~ instances.

*/

    inline void Insert(Tuple* t)
    {
      totalSize += t->GetSize();
      tuples.push_back(t);
    }
/*
Insert a tuple into a bucket. The reference counter
of tuple ~t~ is automatically incremented by one using
a ~RTuple~ instance.

*/

    inline size_t Size() { return totalSize; }
/*
Returns the size of all tuples in a bucket in bytes.

*/

    inline int GetNoTuples() { return (int)tuples.size(); }
/*
Returns the number of tuples in a bucket.

*/

    ostream& Print(ostream& os);
/*
Print the content of a bucket to a stream. This function is
only used for debugging purposes.

*/

    inline BucketIterator* MakeScan() { return new BucketIterator(*this); }
/*
Start a sequential scan of all tuples of a bucket. The method returns a
pointer to a new ~BucketIterator~ instance.

*/

    friend class BucketIterator;
/*
~BucketIterator~ is declared as a friend class, so that
the iterator may access the internal buffer of a ~Bucket~ instance.

*/

  private:

    int number;
/*
Bucket number.

*/

    size_t totalSize;
/*
Total size in bytes of all tuples in a bucket.

*/

    vector<Tuple*> tuples;
/*
Array with tuple references of all tuples in a bucket.

*/
};

/*
9 Class ~HashTable~

Class that represents a hash table for tuples.

*/

class HashTable
{
  public:

    HashTable( size_t nBuckets,
                HashFunction* f,
                JoinTupleCompareFunction* cmp );
/*
The constructor. Creates a hash table with ~nBuckets~ buckets,
hash function ~f~ and a tuple comparison function ~cmp~.

*/

    ~HashTable();
/*
The destructor.

*/

    int ReadFromStream(Word stream, size_t maxSize, bool& finished);
/*
Fills a hash table from stream ~stream~. Returns the number of tuples
read from the stream. If the sizes of all tuples in ~stream~ is lower or
equal than ~maxSize~ bytes the whole stream is consumed and ~finished~ is
set to true. Otherwise the stream is only consumed partially and ~finished~
is set to false.

*/

    void Insert(Tuple* t);
/*
Insert tuple ~t~ into the hash table.

*/

    Tuple* Probe(Tuple* t);
/*
Check if the hash table contains a tuple which is equal to the given tuple ~t~.
A match is found using the ~JoinTupleCompareFunction~ ~cmpFunc~ that has been
specified using the constructor. If a match has been found the method returns
a pointer to the corresponding tuple and internally stores the match location.
The search can be proceeded right after the last match position by another
call of ~Probe~. ~Probe~ then returns the next matching tuple or 0 if the
corresponding bucket has been processed completely. If the first call of
~Probe~ returns 0 then the hash table doesn't contain any matching tuple.

*/

    void Clear();
/*
Removes all tuples from the hash table. The reference counter of all tuples
are decremented by one.

*/

    ostream& Print(ostream& os);
/*
Print the content of a bucket to a stream. This function is
only used for debugging purposes.

*/

    inline size_t GetNoBuckets() { return buckets.size(); }
/*
Returns the number of buckets for a hash table.

*/

    vector<Tuple*> GetTuples(int bucket);
/*
Returns the number of tuples in a hash table.

*/

  private:

    static const bool traceMode = false;
/*
Control flag which enables the tracing mode for this class when
set to true. The

*/

    BucketIterator* iter;
/*
Bucket iterator used to store the location after a successful call
of the ~Probe~ method. The search for matching tuples will be
continued by the next ~Probe~ call at the iterator's location.

*/

    vector<Bucket*> buckets;
/*
Array containing the buckets of the hash tabel.

*/

    HashFunction* hashFunc;
/*
Hash function.

*/

    JoinTupleCompareFunction* cmpFunc;
/*
Comparison function for tuples according to their join attributes.

*/
};

inline ostream& operator<<(ostream& os, HashTable& h)
{
  return h.Print(os);
}

/*
Print the content of a hash table to stream ~os~. This function is
only used for debugging purposes.

*/

/*
10 Template class ~Interval~

This class represents an interval where upper
and lower bound of the interval are included by
the interval.

*/

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

    inline bool IsAt(T n) { return ( low <= n && n <= high ); }
/*
Returns true if value ~n~ lies inside the interval.
Otherwise false is returned.

*/

    inline T GetLength() { return ((high - low) + 1); }
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
11 Class ~PartitionHistogram~

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
First constructor. Creates a histogram for partition with interval ~i~.

*/

    PartitionHistogram(PartitionHistogram& obj, size_t start, size_t end);
/*
Second constructor. Creates a histogram from an existing histogram
copying the histogram entries between 0-based index ~start~ and ~end~.

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

/*
12 Class ~PartitionProgressInfo~

Progress information for a partition.

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

/*
13 Class ~PartitionManagerProgressInfo~

Progress information for a partition manager instance.

*/

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
14 Class ~Partition~

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

    Partition(PInterval i, size_t bufferSize, size_t ioBufferSize);
/*
The constructor. Creates a new partition with interval ~i~
and an internal memory buffer of ~bufferSize~ bytes. For read/write
operations on disk an I/O buffer of ~ioBufferSize~ in bytes is used.

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
15 Class ~PartitionIterator~

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

/*
16 Class ~PartitionCompareLesser~

Class for comparing two partitions according to the lower boundary of
their partition intervals.

*/

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
17 Class ~PartitionManager~

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

    size_t Insert(Tuple* t);
/*
Inserts tuple ~t~ into the partitioning. The corresponding partition
is automatically determined by the partition manager. This method
is used if the partition is not known. The number of the partition is
returned as result.

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

    static void SetIOBufferSize(size_t size) { IO_BUFFER_SIZE = size; }
/*
Sets the I/O buffer size used for read/write operations on disk.

*/

    static size_t GetIOBufferSize() { return IO_BUFFER_SIZE; }
/*
Returns the I/O buffer size used for read/write operations on disk.

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

    static size_t IO_BUFFER_SIZE;
/*
I/O Buffer size in bytes used for read/write operations on disk.

*/
};

inline ostream& operator<<(ostream& os, PartitionManager& pm)
{
  return pm.Print(os);
}
/*
Print the partitioning of a tuple stream to stream ~os~. This function
is only used for debugging purposes.

*/

/*
18 Class ~HybridHashJoinProgressLocalInfo~

All classes and functions have been put into the namespace
~extrel2~ to avoid name conflicts with existing implementations and
to make it later easier to replace existing operators.

This class contains the progress information for the implementation
of the ~hybridhashjoin~ operator. As the operator implementation
needs to provide some more progress information during
partitioning phases this class has been derived from
~ProgressLocalInfo~. The class contains two additional counters
that represent the total and current amount of tuples that must
be processed in intermediate merge phases.

*/

class HybridHashJoinProgressLocalInfo : public ProgressLocalInfo
{
  public:

    HybridHashJoinProgressLocalInfo();
/*
The constructor.

*/

    int CalcProgress( ProgressInfo& p1,
                      ProgressInfo& p2,
                      ProgressInfo* pRes,
                      Supplier s);
/*
Calculates the progress for the hybrid hash join algorithm.

*/

    static const double uHashJoin = 0.023;  //millisecs per probe tuple
    static const double vHashJoin = 0.0067;  //millisecs per tuple right
    static const double wHashJoin = 0.0025;  //millisecs per tuple returned

    static const double t_read = 0.023;  //millisecs per probe tuple
    static const double t_write = 0.0067;  //millisecs per tuple right
    static const double t_probe = 0.023;  //millisecs per tuple returned
    static const double t_hash = 0.0025;  //millisecs per tuple returned
    static const double t_result = 0.0025;  //millisecs per tuple returned

    ostream& Print(ostream& os);
/*
Print to stream ~os~. This function is used
for debugging purposes.

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
};

/*
19 Class ~HybridHashJoinAlgorithm~

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
Updates the progress information. Copies the progres information
collected within the partitions to the HybridHashJoinProgressLocalInfo
instance. Called from value mapping function, whenever a new
REQUESTPROGRESS message is received.

*/

  private:

    void setMemory(size_t maxMemory);
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

    static const size_t MIN_USER_DEF_MEMORY = 1024;
/*
Minimum amount of user defined memory for the operator.

*/

    static const size_t MAX_USER_DEF_MEMORY = ( 64 * 1024 * 1024 );
/*
Maximum amount of user defined memory for the operator.

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

    Tuple* tupleA;
/*
Pointer to last tuple from stream A.

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
    bool bucketProcessed;
/*
Flag that indicates if a bucket of the hash table has been
processed completely for a tuple from stream A.

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
};

/*
20 Class ~HybridHashJoinLocalInfo~

An instance of this class is used in the value mapping function to save
the state of the hybrid hash-join algorithm between multiple message calls.
This class simplifies access to the progress information. A pointer to this
instance of type ~HybridHashJoinLocalInfo~ will be passed to the
~HybridHashJoinAlgorithm~ object ~ptr~.

*/
class HybridHashJoinLocalInfo: public HybridHashJoinProgressLocalInfo
{
  public:

    HybridHashJoinLocalInfo() : HybridHashJoinProgressLocalInfo(), ptr(0) {}
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
