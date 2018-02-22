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
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "RTuple.h"

/*
3 Class ~TupleQueueEntry~

This class is used to store a reference to a tuple
and the array index of the sorted run. During a merge
phase the sort algorithm consumes the tuples from different
runs. If a tuple is consumed the merge queue has to be
refilled with the next tuple from the same run. Therefore
the algorithm needs to know the array index of the
last result tuple.

The class has been re-implemented here because I wouldn't
make too much changes within the normal Relation Algebra. So I
implemented my own version with some static counters
for tracing.

*/

namespace extrel2
{

  class TupleQueueEntry
  {
    public:

    TupleQueueEntry()
    : ref(0)
    , pos(0)
    {
      createCounter++;
    }
/*
First constructor. Creates an empty instance and
increments the create counter by one.

*/

    TupleQueueEntry(Tuple* t, int pos = 0)
    : ref(t)
    , pos(pos)
    {
      createCounter++;
    }
/*
Second constructor. Assigns ~newTuple~ with its array index ~newPos~
to this instance and increments the create counter by one.

*/

    ~TupleQueueEntry()
    {
      ref = 0;
      pos = 0;
    }
/*
The destructor.

*/

    inline TupleQueueEntry(const TupleQueueEntry& obj)
    : ref(obj.ref)
    , pos(obj.pos)
    {
      copyCounter++;
    }
/*
Copy constructor. Increments the copy constructor counter by one.

*/

    inline TupleQueueEntry& operator=(const TupleQueueEntry& obj)
    {
      if ( this == &obj )
        return *this;

      ref = obj.ref;
      pos = obj.pos;

      assignCounter++;

      return *this;
    }
/*
Assignment operator. Increments the assignment counter by one.

*/

    inline Tuple* GetTuple() const { return ref; }
/*
Returns the pointer to the referenced tuple.

*/

    inline int GetPosition() const { return pos; }
/*
Returns the position of the referenced tuple.

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

    private:

    Tuple* ref;
/*
Pointer to referenced tuple.

*/
    int pos;
/*
Array index of the sorted run from which the tuple comes.

*/

    static size_t createCounter;
/*
Static class counter which counts the number of created instances.

*/

    static size_t copyCounter;
/*
Static class counter which counts the calls of the copy constructor.

*/
    static size_t assignCounter;
/*
Static class counter which counts calls of the assignment operator.

*/
  };

/*
4 Class ~TupleCompare~

Derived functional STL object used to compare two tuple queue entries.

*/

  class TupleQueueCompare:
    public std::binary_function<TupleQueueEntry*, TupleQueueEntry*, bool>
  {
    public:

    TupleQueueCompare(const SortOrderSpecification& spec, int attributes)
    : spec(spec)
    , lexCmp()
    , specCmp(spec)
    , attributes(attributes)
    {
      state = analyseSpec(spec);
    }
/*
The constructor. Constructs an instance and with a specified sort order
decsription ~spec~. The number of attributes ~attributes~ must be
specified. so that the compare object is able to decide whether a sort
order specification is lexicographical ordered or not. The background
is that the implementation of ~LexicographicalTupleSmaller~ is a little
bit faster than ~TupleCompareBy~. As the type mapping functions of
all sort operators now always create a sort order description, I have
decided that the tuple queue should decide on its own which compare
operator it uses. This is done by method ~analyseSpec~.


*/

    ~TupleQueueCompare() { }
/*
The destructor.

*/

    inline bool operator()(TupleQueueEntry* x, TupleQueueEntry* y)
    {
      comparisonCounter++;

      if ( state > 0 )
      {
        return !lexCmp(x->GetTuple(), y->GetTuple());
      }
      else if ( state < 0 )
      {
        return lexCmp(x->GetTuple(), y->GetTuple());
      }
      else
      {
        return !(specCmp(x->GetTuple(), y->GetTuple()));
      }
    }
/*
Comparison operator for tuple queue entries. Returns true if
tuple queue entry ~x~ is smaller than entry ~y~.

*/

    inline bool operator()(Tuple* x, Tuple* y)
    {
      comparisonCounter++;

      if ( state > 0 )
      {
        return !lexCmp(x,y);
      }
      else if ( state < 0 )
      {
        return lexCmp(x,y);
      }
      else
      {
        return!(specCmp(x,y));
      }
    }
/*
Comparison operator for tuples. This method is used by method
~Compare~ of class TupleQueue only, to provide a method for
tuple comparison. Returns true if tuple ~x~ is smaller than
tuple ~y~.

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

    private:

    int analyseSpec(const SortOrderSpecification& spec);
/*
Analysis if the sort order specification is lexicographical in
ascending/descending order. Returns -1 if sort order is
lexicographical in descending order, +1 if sort order is
lexicographical in ascending order and 0 otherwise.

*/

    const SortOrderSpecification& spec;
/*
Sort order specification.

*/

    LexicographicalTupleSmaller lexCmp;
/*
Comparison object for lexicographical comparison.

*/

    TupleCompareBy specCmp;
/*
Comparison object for comparison according to a sort order specification.

*/

    int attributes;
/*
Maximum number of sort attributes.

*/

    int state;
/*
State. Decides which comparison operator is used.

  * -1 lexicographical comparison in descending order

  * 0 comparison according to sort order specification

  * +1 lexicographical comparison in ascending order

*/

    static size_t comparisonCounter;
/*
Static class counter which counts the number of comparison operations.

*/

    static bool traceMode;
/*
Flag that enables tracing if set to ~true~.

*/
};


/*
5 Class ~TupleQueue~

This class implements a priority queue for tuples (minimum heap).
Internally the tuples are stored using tuple references implemented
by the ~RTuple~ class. Additionally the class provides an
interface to insert and retrieve tuples directly via pointer.
Object comparison is handled by a functional STL object of type
~TupleCompare~. ~TupleCompare~ compares tuple references.

*/

  class TupleQueue
  {
    public:

    TupleQueue(const SortOrderSpecification& spec, int attributes)
    : totalByteSize(0)
    {
      pComparer = new TupleQueueCompare(spec, attributes);
    }
/*
The constructor. Constructs a minimum heap with a compare
object of type ~TupleCompare~.

*/

    ~TupleQueue()
    {
      while(Size()>0){
        Pop();
      }
      if ( pComparer )
      {
        delete pComparer;
        pComparer = 0;
      }

      totalByteSize = 0;
    }
/*
The destructor. Deletes the priority queue and the
functional STL compare object.

*/

    std::vector<TupleQueueEntry*>& GetContainer() { return container; }
/*
Clears the content of the tuple queue.

*/


    inline TupleQueueEntry* Top() { return container.front(); }
/*
Returns the minimum tuple of the heap.

*/

    inline size_t Size() { return container.size(); }
/*
Returns the number of tuples in the heap.

*/

    inline bool Empty() { return container.empty(); }
/*
Returns true if the heap is empty. Otherwise false is returned.

*/

    inline void Push(Tuple* t, int pos = 0)
    {
      t->IncReference();
      totalByteSize += t->GetSize();
      container.push_back(new TupleQueueEntry(t,pos));
      push_heap (container.begin(),container.end(), *pComparer);
    }
/*
Inserts the tuple ~t~ into the heap.

*/

    inline void Pop()
    {
      pop_heap (container.begin(),container.end(), *pComparer);
      totalByteSize -= container.back()->GetTuple()->GetSize();
      TupleQueueEntry* victim = container.back();
      victim->GetTuple()->DeleteIfAllowed();
      container.pop_back();
      delete victim;
    }
/*
Removes the minimum tuple from the heap.

*/

    inline bool Compare(Tuple* x, Tuple* y)
    {
      return (*pComparer)(x,y);
    }

    inline size_t GetTotalSize() { return totalByteSize; }
/*
Returns the total size of all tuples in the heap.

*/

    protected:

    std::vector<TupleQueueEntry*> container;
/*
Container for tuples.

*/

    TupleQueueCompare* pComparer;
/*
Functional STL object for tuple comparison in descending order.

*/

    size_t totalByteSize;
/*
Total tuple sizes in bytes.

*/
  };

}

#endif /* TUPLEQUEUE_H_ */
