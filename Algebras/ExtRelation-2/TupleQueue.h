/*
----
This file is part of SECONDO.

Copyright (C) 2009, University in Hagen, Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

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

1 Header File TupleQueue.h

May 2009, Sven Jungnickel. Initial version.

2 Defines, includes, and constants

*/

#ifndef TUPLEQUEUE_H_
#define TUPLEQUEUE_H_

#include <queue>
#include "RelationAlgebra.h"
#include "RTuple.h"

/*
3 Class ~TupleAndRelPos~

This class is used during the merge phase of the sort
algorithm. An instance stores a reference to a tuple
and the array index of the sorted run. During a merge
phase the sort algorithm consumes the tuples from different
runs. If a tuple is consumed the merge queue has to be
refilled with the next tuple from the same run. Therefore
the algorithm needs to know the array index of the
last result tuple.

The class has been re-implemented here because I wouldn't
make too much changes within the normal Relation Algebra. So I
implemented my own version with some static counters
for benchmarking. To avoid name conflicts it has been placed
into namespace ~extrel2~.

*/

namespace extrel2
{
  class TupleAndRelPos
  {
    public:

    TupleAndRelPos()
    : ref(0)
    , pos(0)
    {
      createCounter++;
    }
/*
First constructor. Creates an empty instance and
increments the create counter by one.

*/

    ~TupleAndRelPos()
    {}
/*
The destructor.

*/

    TupleAndRelPos(Tuple* newTuple, int newPos = 0)
    : ref( newTuple )
    , pos(newPos)
    {
      createCounter++;
    }
/*
Second constructor. Assigns ~newTuple~ with its array index ~newPos~
to this instance and increments the create counter by one.

*/

    inline TupleAndRelPos(const TupleAndRelPos& rhs)
    : ref(rhs.ref)
    , pos(rhs.pos)
    {
      copyCounter++;
    }
/*
Copy constructor. Increments the copy constructor counter by one.

*/

    inline TupleAndRelPos& operator=(const TupleAndRelPos& rhs)
    {
      if ( this == &rhs )
        return *this;

      ref = rhs.ref;
      pos = rhs.pos;
      assignCounter++;
      return *this;
    }
/*
Assignment operator. Increments the assignment counter by one.

*/

    inline Tuple* tuple() const { return ref; }
/*
Returns the pointer to the referenced tuple.

*/

    static void ResetCounters()
    {
      createCounter = 0;
      copyCounter = 0;
      assignCounter = 0;
    }
/*
Resets all static counter to 0.

*/

    static size_t GetCreateCounter()
    {
      return createCounter;
    }
/*
Returns the value of the create counter.

*/

    static size_t GetCopyCounter()
    {
      return copyCounter;
    }
/*
Returns the value of the copy constructor counter.

*/

    static size_t GetAssignCounter()
    {
      return assignCounter;
    }
/*
Returns the value of the assignment counter.

*/

    static void ResetCreateCounter()
    {
      createCounter = 0;
    }
/*
Resets the create counter to 0.

*/

    static void ResetCopyCounter()
    {
      copyCounter = 0;
    }
/*
Resets the copy constructor counter to 0.

*/

    static void ResetAssignCounter()
    {
      assignCounter = 0;
    }
/*
Resets the assignment counter to 0.

*/

    Tuple* ref;
/*
Pointer to referenced tuple

*/
    int pos;
/*
Array index of the sorted run from which the tuple comes

*/

    static size_t createCounter;
/*
Static class counter which counts the number of created instances

*/

    static size_t copyCounter;
/*
Static class counter which counts the calls of the copy constructor

*/
    static size_t assignCounter;
/*
Static class counter which counts calls of the assignment operator

*/
  };

/*
4 Class ~TupleCompare~

Derived functional STL object used to compare two tuple references
~RTuple~ using a tuple compare object.

*/

  class TupleCompare: public binary_function<RTuple, RTuple, bool >
  {
    public:

    TupleCompare(TupleCompareBy* cmp) { this->cmp = cmp; }
/*
The constructor. Constructs an instance and assigns ~cmp~ as its
compare object for tuple comparisons

*/

    ~TupleCompare() { cmp = 0; }
/*
The destructor.

*/

    static size_t GetComparisonCounter()
    {
      return comparisonCounter;
    }
/*
Returns the value of the comparison counter.

*/

    static void ResetComparisonCounter()
    {
      comparisonCounter = 0;
    }
/*
Resets the comparison counter to 0.

*/

    protected:
    TupleCompareBy* cmp;
/*
Pointer to compare object for tuple comparisons

*/
    static size_t comparisonCounter;
/*
Static class counter which counts the number of comparison operations

*/
};

/*
5 Class ~TupleCompareAsc~

Derived functional STL object used to compare two tuple references
~RTuple~ using a tuple compare object. This class makes it possible to
build a minimum Heap of ~RTuple~ objects using the STL
template class priority\_queue.

*/

  class TupleCompareAsc : public TupleCompare
  {
    public:

    TupleCompareAsc(TupleCompareBy* cmp) : TupleCompare(cmp) {}
/*
The constructor. Constructs an instance and assigns ~cmp~ as its
compare object for tuple comparisons

*/

    inline bool operator()(RTuple x, RTuple y)
    {
      comparisonCounter++;
      return (*cmp)(x.tuple, y.tuple);
    }
/*
Comparison operator which returns true if the referenced tuple ~x~
and ~y~ are in ascending sort order. Otherwise false is returned and
~x~ and ~y~ are in descending sort order.

*/
};

/*
6 Class ~TupleCompareDesc~

Derived functional STL object used to compare two tuple references
~RTuple~ using a tuple compare object. This class makes it possible to
build a minimum Heap of ~RTuple~ objects using the STL
template class priority\_queue.

*/

  class TupleCompareDesc : public TupleCompare
  {
    public:

      TupleCompareDesc(TupleCompareBy* cmp) : TupleCompare(cmp) {}
/*
The constructor. Constructs an instance and assigns ~cmp~ as its
compare object for tuple comparisons

*/

    inline bool operator()(RTuple x, RTuple y)
    {
      comparisonCounter++;
      return !((*cmp)(x.tuple, y.tuple));
    }
/*
Comparison operator which returns true if the referenced tuple ~x~
and ~y~ are in ascending sort order. Otherwise false is returned and
~x~ and ~y~ are in descending sort order.

*/
};


/*
7 Class ~TupleAndRelPosCompare~

Derived functional STL object used to compare two instances of type
~TupleAndRelPos~ using a tuple compare object. This class makes it
possible to build a minimum Heap of ~TupleAndRelPos~ objects using
the STL template class priority\_queue.

*/

  class TupleAndRelPosCompare :
    public binary_function<TupleAndRelPos, TupleAndRelPos, bool >
  {
    public:

      TupleAndRelPosCompare(TupleCompareBy* cmp) { this->cmp = cmp; }
/*
The constructor. Constructs an instance and assigns ~cmp~ as its
compare object for tuple comparisons

*/

    ~TupleAndRelPosCompare() { cmp = 0; }
/*
The destructor.

*/

    protected:
    TupleCompareBy* cmp;
/*
Pointer to compare object for tuple comparisons

*/
  };

/*
8 Class ~TupleAndRelPosCompareAsc~

Derived functional STL object used to compare two instances of type
~TupleAndRelPos~ using a tuple compare object. This class makes it
possible to build a minimum Heap of ~TupleAndRelPos~ objects using
the STL template class priority\_queue.

*/

  class TupleAndRelPosCompareAsc : public TupleAndRelPosCompare
  {
    public:

      TupleAndRelPosCompareAsc(TupleCompareBy* cmp)
      : TupleAndRelPosCompare(cmp) {}
/*
The constructor. Constructs an instance and assigns ~cmp~ as its
compare object for tuple comparisons

*/

    inline bool operator()(TupleAndRelPos& x, TupleAndRelPos& y)
    {
      return (*cmp)(x.tuple(), y.tuple());
    }
/*
Comparison operator which returns true if the referenced tuple ~x~
and ~y~ are in ascending sort order. Otherwise false is returned and
~x~ and ~y~ are in descending sort order.

*/
};

/*
9 Class ~TupleAndRelPosCompareDesc~

Derived functional STL object used to compare two instances of type
~TupleAndRelPos~ using a tuple compare object. This class makes it
possible to build a minimum Heap of ~TupleAndRelPos~ objects using
the STL template class priority\_queue.

*/

  class TupleAndRelPosCompareDesc : public TupleAndRelPosCompare
  {
    public:

      TupleAndRelPosCompareDesc(TupleCompareBy* cmp)
      : TupleAndRelPosCompare(cmp) {}
/*
The constructor. Constructs an instance and assigns ~cmp~ as its
compare object for tuple comparisons

*/

    inline bool operator()(TupleAndRelPos& x, TupleAndRelPos& y)
    {
      return !((*cmp)(x.tuple(), y.tuple()));
    }
/*
Comparison operator which returns true if the referenced tuple ~x~
and ~y~ are in ascending sort order. Otherwise false is returned and
~x~ and ~y~ are in descending sort order.

*/
};

/*
6 Class ~Queue~

Template class that implements a STL priority\_queue of
type ~T~ using a specific functional STL object of type
~CompareObj~.

*/

  template<typename CompareObj, typename T>
  class Queue
  {
    public:

    Queue(TupleCompareBy* cmp)
    {
      pComparer = new CompareObj(cmp);
      pQueue = new Heap(*pComparer);
    }
/*
The constructor. Constructs a heap with a compare
object of type ~CompareObj~.

*/

    virtual ~Queue()
    {
      delete pQueue;
      pQueue = 0;

      delete pComparer;
      pComparer = 0;
    }
/*
Virtual destructor. Deletes the priority queue and the
functional STL compare object.

*/

    virtual T& Top() { return (T&)pQueue->top(); }
/*
Returns the top element of the heap.

*/

    virtual void Push(T& t) { pQueue->push(t); }
/*
Puts a new element into the heap. Sorting will be done
by usage of the ~CompareObj~.

*/

    virtual size_t Size() { return pQueue->size(); }
/*
Returns the number of elements in the heap.

*/

    virtual bool Empty() { return pQueue->empty(); }
/*
Returns true if the heap is empty. Otherwise false is returned.

*/

    virtual void Pop() { pQueue->pop(); };
/*
Removes the top element from the heap.

*/

    protected:

    typedef std::priority_queue<T, std::vector<T>, CompareObj> Heap;
/*
Type definition of a STL priority\_queue that uses a functional STL
object of type ~CompareObj~ for element comparison.

*/

    CompareObj* pComparer;
/*
Functional object instance for element comparison.

*/

    Heap* pQueue;
/*
Pointer to heap instance.

*/

  };

/*
7 Class ~TupleQueue~

This class implements a priority queue for tuples (minimum heap).
Internally the tuples are stored using tuple references implemented
by the ~RTuple~ class. Additionally the class provides an
interface to insert and retrieve tuples directly via pointer.
Object comparison is handled by a functional STL object of type
~TupleCompare~. ~TupleCompare~ compares tuple references.

The class hasn't been derived from ~Queue~ because internally
it makes sense to store tuples as references of type ~RTuple~.
In the class interface we provide methods to insert retrieve
tuples using pointers instead. As overloading of the ~Top()~
method with different return types is not possible the class
was implemented without inheritance.

*/

  class TupleQueue
  {
    public:

    TupleQueue(TupleCompareBy* cmp)
    : totalByteSize(0)
    {
      pComparer = new TupleCompareDesc(cmp);
      pQueue = new MinHeap(*pComparer);
    }
/*
The constructor. Constructs a minimum heap with a compare
object of type ~TupleCompare~.

*/

    ~TupleQueue()
    {
      delete pQueue;
      pQueue = 0;

      delete pComparer;
      pComparer = 0;
    }
/*
The destructor. Deletes the priority queue and the
functional STL compare object.

*/

    inline Tuple* Top() { return pQueue->top().tuple; }
/*
Returns the minimum tuple of the heap.

*/

    inline size_t Size() { return pQueue->size(); }
/*
Returns the number of tuples in the heap.

*/

    inline bool Empty() { return pQueue->empty(); }
/*
Returns true if the heap is empty. Otherwise false is returned.

*/

    inline void Push(Tuple* t)
    {
      pQueue->push(RTuple(t));
      totalByteSize += t->GetSize();
    }
/*
Inserts the tuple ~t~ into the heap.

*/

    inline void Pop()
    {
      totalByteSize -= pQueue->top().tuple->GetSize();
      pQueue->pop();
    }
/*
Removes the minimum tuple from the heap.

*/

    inline size_t GetTotalSize() { return totalByteSize; }
/*
Returns the total size of all tuples in the heap.

*/

    protected:

    typedef std::priority_queue< RTuple,
                                 std::vector<RTuple>,
                                 TupleCompareDesc > MinHeap;
/*
Type definition of minimum heap.

*/

    TupleCompareDesc* pComparer;
/*
Functional STL object for comparison

*/

    MinHeap* pQueue;
/*
Minimum heap

*/

    size_t totalByteSize;
/*
Total tuple sizes in bytes

*/
  };

/*
8 Class ~TupleAndRelPosQueue~

This class implements a priority queue for ~TupleAndRelPos~ objects.
Object comparison is handled by a functional STL object of type
~TupleAndRelPosGreaterCompare~.

*/

  class TupleAndRelPosQueue :
    public Queue<TupleAndRelPosCompareDesc, TupleAndRelPos>
  {
    public:

    TupleAndRelPosQueue(TupleCompareBy* cmp);
/*
The constructor. Constructs a ~TupleAndRelPosQueue~ instance
and assign the tuple compare object ~cmp~ to the
~TupleAndRelPosGreaterCompare~ instance.

*/

    virtual void Push(TupleAndRelPos& t);
/*
Puts the element ~t~ into the heap.

*/

    virtual void Pop();
/*
Removes the minimum element from the heap. The minimum
element is the one which contains the reference to the
minimum tuple.

*/

    inline size_t GetTotalSize() { return totalByteSize; }
/*
Returns the total size of all tuples in the heap.

*/

    protected:
    size_t totalByteSize;
/*
Total tuple sizes in bytes

*/
  };
}

#endif /* TUPLEQUEUE_H_ */
