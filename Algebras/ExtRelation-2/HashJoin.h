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


1 Header File HashJoin.h

June 2009, Sven Jungnickel. Initial version

2 Overview

This file contains the declaration of all classes and functions that
are necessary for the implementation the hash-join operators
~gracehashjoin~ and ~hybridhashjoin~.

3 Includes

*/

#ifndef HASHJOIN_H_
#define HASHJOIN_H_


#include <limits.h>
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

#define HASHJOIN_MINIMUM_BUCKETS      3
/*
Minimum number of buckets in hash table.

*/

#define HASHJOIN_MAXIMUM_BUCKETS      16384
/*
Maximum number of buckets in hash table.

*/

#define HASHJOIN_DEFAULT_BUCKETS      1000
/*
Default number of buckets in hash table if
a value below ~HASHJOIN\_MINIMUM\_BUCKETS~ or higher than
~HASHJOIN\_MAXIMUM\_BUCKETS~ has been specified.

*/

#define HASHJOIN_MINIMUM_PARTITIONS   1
/*
Minimum number of partitions.

*/

#define HASHJOIN_MAXIMUM_PARTITIONS   8192
/*
Maximum number of partitions (~HASHJOIN\_MAXIMUM\_BUCKETS~/2).

*/

#define HASHJOIN_DEFAULT_PARTITIONS   50
/*
Default number of partitions for operator ~gracehashjoinParam~ and
~hybridhashjoinParam~ if a value below ~HASHJOIN\_MINIMUM\_PARTITIONS~
or higher than ~HASHJOIN\_MAXIMUM\_PARTITIONS~ has been specified.

*/

#define HASHJOIN_MINIMUM_MEMORY       1024
/*
Minimum operator memory in bytes that may be specified for
operator ~gracehashjoinParam~ and ~hybridhashjoinParam~.

*/

#define HASHJOIN_MAXIMUM_MEMORY       ( 64 * 1024 * 1024 )
/*
Maximum operator memory in bytes that may be specified for
operator ~gracehashjoinParam~ and ~hybridhashjoinParam~.

*/

#define HASHJOIN_DEFAULT_MEMORY       ( 16 * 1024 * 1024 )
/*
Default operator memory in bytes for operator ~gracehashjoin~ and
~hybridhashjoinParam~ if a value below ~HASHJOIN\_MINIMUM\_MEMORY~
or higher than ~HASHJOIN\_MAXIMUM\_MEMORY~ has been specified.

*/


namespace extrel2
{

/*
5 Auxiliary functions

Logarithm base 2

*/

double log2(double n);

/*
Print progress information

*/

ostream& PrintProgressInfo(ostream& os, ProgressInfo& info);

/*
5 Class ~JoinTupleCompareFunction~

Comparison function class for tuples that shall be joined
according to one join attribute.

*/

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
      Attribute* attr;
      attr = static_cast<Attribute*>(t->GetAttribute(attrIndex));
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
7 Class ~Bucket~

This class represents a bucket of a hash table.

*/

class BucketIterator;
/*
Necessary forward declaration for class ~BucketIterator~.

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

    BucketIterator* MakeScan();
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

    vector<RTuple> tuples;
/*
Array with tuple references of all tuples in a bucket.

*/
};

/*
8 Class ~BucketIterator~

Iterator class which is used to iterate sequentially through all tuples
of a bucket from a hash table.

*/

class BucketIterator
{
  public:

    BucketIterator(Bucket& b);
/*
The constructor. Starts a sequential scan for bucket ~b~.

*/

    inline Tuple* GetNextTuple()
    {
      if ( iter != bucket.tuples.end() )
      {
        Tuple* t = (*iter).tuple;
        iter++;
        return t;
      }

      return 0;
    }
/*
Returns the next tuple in sequential order. If all tuples
have been processed 0 is returned.

*/

  private:

    Bucket& bucket;
/*
Reference to bucket on which the instance iterates.

*/

    vector<RTuple>::iterator iter;
/*
Iterator for internal bucket tuple buffer.

*/
};

/*
9 Class ~HashTable~

Class that represents a hash table for tuples.

*/

class HashTable
{
  public:

    HashTable( const size_t nBuckets,
               const HashFunction& f,
               const HashFunction& probeHash,
               const JoinTupleCompareFunction& cmp );
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

    inline Tuple* Probe(Tuple* t)
    {
      Tuple* nextTuple = 0;

      if ( iter == 0 )
      {
        // calculate bucket number
        size_t h = probeFunc.Value(t);

        // start bucket scan
        iter = buckets[h]->MakeScan();
      }

      while ( (nextTuple = iter->GetNextTuple() ) != 0 )
      {
        if ( cmpFunc.Compare(t, nextTuple) == 0 )
        {
          return nextTuple;
        }
      }

      delete iter;
      iter = 0;

      return 0;
    }
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
Array containing the buckets of the hash table.

*/

    HashFunction hashFunc;
/*
Hash function.

*/
    HashFunction probeFunc;


    JoinTupleCompareFunction cmpFunc;
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

struct PartitionHistogramEntry
{
  PartitionHistogramEntry(size_t value)
  : value(value)
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

    PartitionHistogram(PInterval& intv);
/*
First constructor. Creates a histogram for partition with interval ~intv~.

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
      cmsg.info() << HEADLINE_PHISTOGRAM << endl
                  << "Interval: [" << interval.GetLow() << ","
                  << interval.GetHigh() << "]"
                  << ", tuples: " << tuples
                  << ", totalSize: " << totalSize
                  << ", totalExtSize: " << totalExtSize
                  << endl;

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

    PartitionProgressInfo(PInterval& intv)
    : interval(intv)
    , tuples(0)
    , tuplesProc(0)
    , noOfPasses(0)
    , curPassNo(1)
    {
    }
/*
The constructor. Creates an empty instance.

*/

    PartitionProgressInfo(const PartitionProgressInfo& obj)
    {
      if ( this == &obj )
        return;

      this->interval = obj.interval;
      this->tuples = obj.tuples;
      this->tuplesProc = obj.tuplesProc;
      this->noOfPasses = obj.noOfPasses;
      this->curPassNo = obj.curPassNo;
    }
/*
Copy constructor.

*/

    ostream& Print(ostream& os)
    {
      os << "Interval [" << interval.GetLow()
         << ", " << interval.GetHigh() << "]"
         << ", tuples: " << tuples
         << ", TuplesProc: " << tuplesProc
         << ", noOfPasses: " << noOfPasses
         << ", curPassNo: " << curPassNo
         << endl;

      return os;
    }
/*
Print to stream ~os~. This function is used
for debugging purposes.

*/

    PInterval interval;
/*
Partition interval. Necessary for sorting the progress information
in the same way as the partitions are sorted.

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

    int curPassNo;
/*
Current pass number. This field is only used for partitions from stream A.

*/
};

/*
13 Class ~PartitionCompareLesser~

Class for comparing two partitions according to the lower boundary of
their partition intervals.

*/

class PartitionProgressInfoCompareLesser :
  binary_function<PartitionProgressInfo, PartitionProgressInfo, bool>
{
  public:

    inline bool operator()(PartitionProgressInfo a, PartitionProgressInfo b)
    {
      return ( a.interval.GetLow() < b.interval.GetLow() );
    }
};


/*
14 Class ~PartitionManagerProgressInfo~

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

    size_t GetTotalProcessedTuples()
    {
      size_t result = 0;

      for (size_t i = 0; i < partitionProgressInfo.size(); i++)
      {
        result += partitionProgressInfo[i].tuplesProc;
      }

      return result;
    }
/*
Returns the total number of processed tuples of all partitions..

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
15 Class ~Partition~

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

    inline bool Overflows( Supplier s)
    {
      return ( this->GetTotalExtSize() > (qp->GetMemorySize(s) * 1024 * 1024) );
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
16 Class ~PartitionIterator~

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
17 Class ~PartitionCompareLesser~

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
18 Class ~PartitionManager~

Class which represents the partitioning of a complete stream.

*/
class PartitionManager
{
  public:

    PartitionManager( HashFunction* h,
                       size_t opMem,
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

    void Insert(Tuple* t, size_t p, size_t b);
/*
Inserts tuple ~t~ into partition ~p~ and bucket ~b~.
This method is used if the partition is known.

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

    size_t insertPartition( PInterval intv,
                             size_t buffer,
                             size_t io,
                             int index = -1 );
/*
Insert a new partition with partition interval ~intv~, internal
buffer size of ~buffer~ bytes and an I/O buffer size of ~io~ bytes.
If ~index~ is smaller than 0 the partition is appended to the end of the
partition array. If a value greater than 0 is specified the
corresponding array entry will be overwritten. Method returns
the 0-based partition index of the new partition

*/

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

    void printPartitionHistograms()
    {
      for(size_t i = 0; i < partitions.size(); i++)
      {
        cmsg.info() << "Partition => " << i << endl;
        partitions[i]->GetPartitionHistogram().Print(cmsg.info());
        cmsg.send();
      }
    }
/*
Prints the partition histograms of all partitions. Used for debugging
purposes only.

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

    size_t maxOperatorMemory;
/*
Maximum available memory for operator in bytes. This information
is necessary for subpartitioning in order to decide whether
a partition needs to be subpartitioned.

*/

    int checkProgressAfter;
/*
During subpartitioning after ~checkProgressAfter~ tuples
have been processed a progress message will be propagated by the
query processor (but only if enough time since the last progress
message has passed by, the query processor will insure this)

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

    bool simSubpartitioning;
/*
Flag which indicates if progress information for sub-partitioning
shall be generated.

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

} // end of namespace extrel2

#endif /* HASHJOIN_H_ */
