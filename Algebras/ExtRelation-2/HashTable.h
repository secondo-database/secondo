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

1 Header File HashTable.h

June 2009, Sven Jungnickel. Initial version

2 Includes and defines

*/

#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include "RelationAlgebra.h"
#include "RTuple.h"

/*
3 Class ~JoinTupleCompareFunction~

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
4 Class ~HashFunction~

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
attribute for which the hash value shall be calculated..

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
5 Class ~BucketIterator~

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

    vector<RTuple>::iterator iter;
/*
Iterator for internal bucket tuple buffer.

*/
};

/*
6 Class ~Bucket~

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
      tupleRefs.clear();
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
      tupleRefs.push_back(RTuple(t));
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

    inline int GetNoTuples() { return (int)tupleRefs.size(); }
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

    vector<RTuple> tupleRefs;
/*
Array with tuple references of all tuples in a bucket.

*/
};

/*
7 Class ~HashTable~

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

/*
8 Global operators

*/

inline ostream& operator<<(ostream& os, HashTable& h)
{
  return h.Print(os);
}

/*
Print the content of a hash table to stream ~os~. This function is
only used for debugging purposes.

*/

} // end of namespace extrel2

#endif /* HASHTABLE_H_ */
