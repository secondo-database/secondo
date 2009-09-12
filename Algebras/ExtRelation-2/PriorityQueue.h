/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics and
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

1 Header File PriorityQueue.h

May 2009. S. Jungnickel. Code provided as header file.

2 Overview

This file contains alternative implementations for the heap
data structure. The goal was to find an implementation that
reduces the number of necessary tuple comparisons compared
to the STL heap implementation (priority\_queue). STL uses
a bottom-up variant for the heap implementation.
The following variants were implemented

  * Standard Heap

  * Bottom-Up Heap

  * Improved Bottom-Up Heap

  * MDR Heap

The final result was that the MDR Heap variant is really
able to reduce the number of necessary tuple comparisons
during a sort operation by about 3-5\%. Unfortunately
the generated overhead per tuple comparison due to the
greater complexity of the algorithms was greater than the
the time which we were able to save using the new heap
implementation. For more information see my master's thesis.

3 Includes

*/

#ifndef SEC_PRIORITY_QUEUE_H
#define SEC_PRIORITY_QUEUE_H

#include <vector>
#include "TupleQueue.h"

#undef PRIORITYQUEUEMDRHEAP_TRACE
#undef PRIORITY_QUEUE_BOTTOM_UP_HEAP2_TRACE

/*
4 Class ~PriorityQueueStandardHeap~

Standard heap implementation within an array without
any improvements.

*/
namespace extrel2
{

template<class T, class CompareFunc> class PriorityQueueStandardHeap{
public:

  PriorityQueueStandardHeap(CompareFunc* func) : func(func) { assert(func); }
/*
The constructor. Assigns the binary compare function.

*/

  void Push(const T& elem)
  {
    // append element to array
    heap.push_back(elem);

    // let the element bubble up to the correct position
    bubbleUp(heap.size()-1);
  }
/*
Insert a new element into the priority queue.

*/

  const T& Top()
  {
    return heap[0];
  }
/*
Get the next element with the highest priority
(the element stays in the queue).

*/

  void Pop()
  {
    size_t n = heap.size();
    heap[0] = heap[n-1];    // exchange last element with root
    heap.resize(n-1);       // make array one element smaller
    siftDown(0, n-2);       // let new root element sift down in heap
    return;
  }
/*
Get the next element with the highest priority and remove it from the queue.

*/

  bool IsEmpty()
  {
    return heap.empty();
  }
/*
Test if the queue contains any elements.

*/

  size_t Size() { return heap.size(); }
/*
Returns the number of elements stored within the heap

*/

private:

  inline void bubbleUp (int n)
  {
    int i = n;
    T x = heap[n];
    while (i >= 1 && (*func)(heap[(i-1)/2], x) )
    {
      // -1 because of 0-based array index
      heap[i] = heap[(i-1)/2];
      i = (i-1)/2;
    }
    heap[i] = x;
  }
/*
Let the element at array position n bubble up until the heap condition is fulfilled.

*/

  inline void siftDown(int i, int n)
  {
    //int dummy = 0;

    // do as long as i has a left son
    while ( (2*i+1) <= n)
    {
      int j = 2*i+1; // +1 because of 0-based array index

      if (j < n && (*func)(heap[j], heap[j+1]) )
      {
        j = j + 1;  // right son is the bigger one
      }

      if ( (*func)(heap[i], heap[j]) )
      {
        T tmp = heap[i];
        heap[i] = heap[j];
        heap[j] = tmp;
        i = j;      // element is still smaller than
                    // the selected child - continue sifting
      }
      else
      {
        i = n;  //Exit loop
      }
    }
  }
/*
Let the element at array position sift down in the heap,
until latest n is reached.

*/

  void swap(int i, int j)
  {
    T tmp = heap[i];
    heap[i] = heap[j];
    heap[j] = tmp;
  }
/*
Swaps two elements in the array.

*/

  // Array
  std::vector<T> heap;

  // Binary compare function
  CompareFunc* func;
};

/*
5 Class ~PriorityQueueBottomUpHeap~

Bottom-up heap implementation within an array without
any further improvements.

*/

template<class T, class CompareFunc> class PriorityQueueBottomUpHeap{
public:

  PriorityQueueBottomUpHeap(CompareFunc* func) : func(func) { assert(func); }
/*
The constructor.

*/

  void Push(const T& elem)
  {
    // append element to array
    heap.push_back(elem);

    // let the element bubble up to the correct position
    bubbleUp(heap.size()-1);
  }
/*
Insert a new element into the priority queue.

*/

  const T& Top()
  {
    return heap[0];
  }
/*
Get the next element with the highest priority (the element stays in the queue).

*/

  void Pop()
  {
    size_t n = heap.size();
    heap[0] = heap[n-1];    // exchange last element with root
    siftDown(0, n-2);       // let new root element sift down in heap
    heap.pop_back();
    return;
  }
/*
Get the next element with the highest priority and remove it from the queue.

*/

  bool IsEmpty()
  {
    return heap.empty();
  }
/*
Test if the queue contains any elements.

*/

  size_t Size() { return heap.size(); }
/*
Returns the number of elements within the heap.

*/

private:

  inline void bubbleUp (int n)
  {
    T x = heap[n];
    // -1 because of 0-based array index
    int parent = (n-1) >> 1; // equal to (n-1)/2

    while (n >= 1 && (*func)(heap[parent], x) )
    {
      heap[n] = heap[parent];
      n = parent;
      parent = (n-1) >> 1; // equal to (n-1)/2
    }
    heap[n] = x;
  }
/*
Let the element at array position n bubble up
until the heap condition is fulfilled.

*/

  inline void siftDown(int i, int n)
  {
    T x = heap[i];

    int leftSon = (i << 1) + 1; // equal to (2*i+1)
    // do as long as i has a left son
    while ( leftSon <= n)
    {
      int j = leftSon; // +1 because of 0-based array index

      if (j < n && (*func)(heap[j], heap[j+1]) )
      {
        j = j + 1;  // right son is the bigger one
      }

      heap[i] = heap[j];
      i = j;      // element is still smaller than
                  // the selected child - continue sifting
      leftSon = (i << 1) + 1; // equal to (2*i+1);
    }
    heap[i] = x;

    bubbleUp(i);
  }
/*
Let the element at array position sift down
in the heap, until latest n is reached.

*/

  // Array
  std::vector<T> heap;

  // Binary compare function
  CompareFunc* func;
};

/*
5 Class ~PriorityQueueBottomUpHeap2~

Bottom-up heap implementation with some further improvements.

*/

template<class T, class CompareFunc>
class PriorityQueueBottomUpHeap2{
public:

  PriorityQueueBottomUpHeap2(CompareFunc* func)
  : func(func) { assert(func); }
/*
The constructor.

*/

  void Push(const T& elem)
  {
    heap.push_back(elem);     // append element to array
    bubbleUp(heap.size()-1);  // let the element bubble up to
                              // the correct position
  }
/*
Insert a new element into the priority queue.

*/

  const T& Top()
  {
    return heap[0];
  }
/*
Get the next element with the highest priority
(the element stays in the queue).

*/

  void Pop()
  {
    size_t n = heap.size();
    //heap[0] = heap[n-1];  // exchange last element with root
    siftDown(n-1, n-2);     // let new root element sift down in heap
    heap.pop_back();
    return;
  }
/*
Get the next element with the highest priority and remove it from the queue.

*/

  bool IsEmpty()
  {
    return heap.empty();
  }
/*
Test if the queue contains any elements.

*/

  size_t Size() { return heap.size(); }
/*
Return the number of elements within the heap.

*/

private:

  inline void bubbleUp (int n)
  {
    T x = heap[n];
    // -1 because of 0-based array index
    int parent = (n-1) >> 1; // equal to (n-1)/2

    while (n >= 1 && (*func)(heap[parent], x) )
    {
      heap[n] = heap[parent];
      n = parent;
      parent = (n-1) >> 1; // equal to (n-1)/2
    }
    heap[n] = x;
  }
/*
Let the element at array position n bubble up
until the heap condition is fulfilled.

*/

  inline void siftDown(int elem, int treeSize)
  {
    int j = leafSearch(0, treeSize);
    j = bottomUpSearch(elem, j);
    interchange(j, elem);
  }
/*
Let the element at array position sift down in the heap,
until latest n is reached.

*/

  inline int leafSearch(int root,int treeSize)
  {
    int i = 0;
    int j = root;
    path[i++] = root;

    int leftSon = (j << 1) + 1;

    while( leftSon < treeSize)
    {
      if ( (*func)(heap[leftSon], heap[leftSon+1]) )
      {
        path[i++] = j = leftSon+1;
      }
      else
      {
        path[i++] = j = leftSon;
      }

      leftSon = (j << 1) + 1;
    }

    if( leftSon == treeSize )
    {
      j = path[i++] = treeSize;
    }

#ifdef PRIORITY_QUEUE_BOTTOM_UP_HEAP2_TRACE
    cmsg.info() << "Leaf found at " << j << endl;
    cmsg.info() << "Path: ";
    for (int m=0; m < i; m++)
      cmsg.info() << path[m] << "-> ";
    cmsg.send();
#endif

    return j;
  }
/*
Search a leaf node

*/

  inline int bottomUpSearch(int i, int j)
  {
    while( j > 0 && (*func)(heap[j], heap[i]) )
    {
      j = (j-1) >> 1;
    }

#ifdef PRIORITY_QUEUE_BOTTOM_UP_HEAP2_TRACE
    cmsg.info() << "Swap position found at " << j << endl;
    cmsg.send();
#endif

    return j;
  }
/*
Bottom-up search.

*/

  inline void interchange(int j, int elem)
  {
    int k;

    for(k=0; path[k] < j; k++)
    {
      heap[path[k]] = heap[path[k+1]];
#ifdef PRIORITY_QUEUE_BOTTOM_UP_HEAP2_TRACE
      cmsg.info() << "Interchange " << path[k] << " <- " << path[k+1] << endl;
#endif
    }

    heap[path[k]] = heap[elem];

#ifdef PRIORITY_QUEUE_BOTTOM_UP_HEAP2_TRACE
    cmsg.info() << "Interchange " << path[k] << " <- " << elem << endl;
    cmsg.send();
#endif
  }
/*
Interchange node along a path.

*/

  // Array
  std::vector<T> heap;

  // Binary compare function
  CompareFunc* func;

  // Path during leaf search
  int path[64];
};

/*
5 Class ~PriorityQueueMDRHeap~

MDR heap implementation

*/

typedef struct InfoElem
{
  int bits:2;
};

template<class T, class CompareFunc> class PriorityQueueMDRHeap{
public:

  PriorityQueueMDRHeap(CompareFunc* func) : func(func) { assert(func); }
/*
The constructor.

*/

  void Push(const T& elem)
  {
    InfoElem e;
    e.bits = UNKNOWN;
    info.push_back(e);
    heap.push_back(elem);    // append element to array
    bubbleUp(heap.size()-1); // let the element bubble up
                             // to the correct position
  }
/*
Insert a new element into the priority queue.

*/

  const T& Top()
  {
    return heap[0];
  }
/*
Get the next element with the highest priority (the element stays in the queue).

*/

  void Pop()
  {
    size_t n = heap.size();
    siftDown(n-1, n-2);       // let new root element sift down in heap
    heap.pop_back();
    info.pop_back();
    return;
  }
/*
Get the next element with the highest priority and remove it from the queue.

*/

  bool IsEmpty()
  {
    return heap.empty();
  }
/*
Test if the queue contains any elements.

*/

  size_t Size() { return heap.size(); }

private:

  inline void bubbleUp (int n)
  {
    T x = heap[n];
    // -1 because of 0-based array index
    int parent = (n-1) >> 1; // equal to (n-1)/2

    while (n >= 1 && (*func)(heap[parent], x) )
    {
      heap[n] = heap[parent];
      n = parent;
      parent = (n-1) >> 1; // equal to (n-1)/2
    }
    heap[n] = x;
  }
/*
Let the element at array position n bubble up
until the heap condition is fulfilled.

*/

  inline void siftDown(int elem, int treeSize)
  {
    int j = leafSearch(0, treeSize);
    j = bottomUpSearch(elem, j);
    interchange(j, elem);
  }
/*
Let the element at array position sift down in the heap,
until latest n is reached.

*/

  inline int leafSearch(int root,int treeSize)
  {
    int i = 0;
    int j = root;
    path[i++] = root;

    int leftSon = (j << 1) + 1;

    while( leftSon < treeSize)
    {
      if ( info[j].bits == LEFT )
      {
        path[i++] = j = leftSon;
      }
      else if ( info[j].bits == RIGHT )
      {
        path[i++] = j = leftSon+1;
      }
      else if ( (*func)(heap[leftSon], heap[leftSon+1]) )
      {
        info[j].bits = RIGHT;
        path[i++] = j = leftSon+1;
      }
      else
      {
        info[j].bits = LEFT;
        path[i++] = j = leftSon;
      }

      leftSon = (j << 1) + 1;
    }

    if( leftSon == treeSize )
    {
      j = path[i++] = treeSize;
    }

#ifdef PRIORITYQUEUEMDRHEAP_TRACE
    cmsg.info() << "Leaf found at " << j << endl;
    cmsg.info() << "Path: ";
    for (int m=0; m < i; m++)
      cmsg.info() << path[m] << "-> ";
    cmsg.send();
#endif

    return j;
  }
/*
Search a leaf node

*/

  inline int bottomUpSearch(int i, int j)
  {
    while( j > 0 && (*func)(heap[j], heap[i]) )
    {
      j = (j-1) >> 1;
    }

#ifdef PRIORITYQUEUEMDRHEAP_TRACE
    cmsg.info() << "Swap position found at " << j << endl;
    cmsg.send();
#endif

    return j;
  }
/*
Bottom-up search

*/

  inline void interchange(int j, int elem)
  {
    int k;

    for(k=0; path[k] < j; k++)
    {
      heap[path[k]] = heap[path[k+1]];
      info[path[k]].bits = UNKNOWN;
#ifdef PRIORITYQUEUEMDRHEAP_TRACE
      cmsg.info() << "Interchange " << path[k] << " <- " << path[k+1] << endl;
#endif
    }

    heap[path[k]] = heap[elem];

#ifdef PRIORITYQUEUEMDRHEAP_TRACE
    cmsg.info() << "Interchange " << path[k] << " <- " << elem << endl;
    cmsg.send();
#endif
  }
/*
Interchange nodes along a path

*/

  enum
  {
    LEFT = 0,
    RIGHT = 1,
    UNKNOWN = 3
  };
/*
Enumeration that identifies which child node of a parent node
is greater.

*/

  typedef std::pair<T,InfoElem> arrayElem;

  // Info elements
  std::vector<InfoElem> info;

  // Array
  std::vector<T> heap;

  // Binary compare function
  CompareFunc* func;

  // Path during leaf search
  int path[64];
};

} // end of namespace extrel2
#endif
