/*
----
This file is part of SECONDO.

Copyright (C) 2004-2008, University in Hagen, Faculty of Mathematics and
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

//paragraph [1] Title: [{\Large \bf] [}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[newpage] [\newpage]
//[<] [$<$]
//[>] [$>$]
//[INSET] [$\in$]

[1] Definition of TupleQueueHP

February 2014 Jiamin Lu

[newpage]

1 TupleQueueHP

The ~TupleQueueHP~ is designed as a replacement for the ~TupleBuffer~ structure.
Tuples in the ~TupleBuffer~ cannot be released until the whole query finishes.
This is because when the tuple is cached, it is pinned and it may be used in
several ~tupleBuffer~s.
In this case, all Flobs of cached tuples are also cached within the ~NativeFlobCache~,
and can not be released during the whole query procedure.

This feature causes problem in the ~fetchFlob~ operator, which holds a tupleBuffer
for all tuples that have Flob data need to be fetched from remote DSs.
Since the tuples cannot be released from the buffer although they have been output
to the successive operator, their Flob are also hold the NativeFlobCache all the time,
hence produces unnecessary disk overhead.

Regarding this problem, hereby I created a simple FlobQueue structure as the
replacement for the ~TupleBuffer~.
It is a FIFO structure, when a tuple is returned then its Flob is also released.
It holds a STL vector, named ~mtq~ (memory tuple queue), for the tuples as the memory buffer,
and the overflowed tuples are inserted into a temporal relation structure.
When a tuple is cached into TupleQueueHP, it makes a copy and the origianl one is deleted,
in order to avoid pining the tuple and the Flob.

It does not support random access, all tuples are read from the top of the structure,
i.e. the ~mtq~. When the ~mtq~ is partial cleaned up, then some tuples are read from the
relation and put into the end of the ~mtq~.

In order to traverse all cached tuples, instead of only fetching the top one,
a stl list is used to take place of the queue structure for the ~mtq~


*/

#ifndef TUPLEQUEUEHP_H_
#define TUPLEQUEUEHP_H_

#include "RelationAlgebra.h"
#include <list>

typedef list<Tuple*> TupleList;

class TupleQueueHP;

class TupleQueueHPIterator
{
public:
  TupleQueueHPIterator(TupleQueueHP& queue);

  Tuple* GetNextTuple();

private:

  TupleQueueHP& queue;
  GenericRelationIterator* diskIterator;
  size_t currentTuple;
};


class TupleQueueHP
{
public:

  TupleQueueHP( size_t mamMemmorySize = 16 * 1024 * 1024);
  ~TupleQueueHP();

  void AppendTuple(Tuple *t);
  Tuple* PopTuple(bool& bulkLoaded);


  size_t GetNoTuples() const;
  bool IsEmpty() const;

  void IncFlobReference(const Flob& flob);

  inline TupleQueueHPIterator* MakeScan(){
    return new TupleQueueHPIterator(*this);
  }

  friend class TupleQueueHPIterator;
private:

  const size_t MAX_MEMORY_SIZE;
  TupleList memoryBuffer;
  Relation* diskBuffer;
  bool inMemory;

/*
The transportedTuples is used to cache the tuples read from the diskBuffer, 
and hold inside the memoryBuffer. 
They are released until the memoryBuffer is cleaned.

*/
  vector<Tuple*>* transportedTuples;  
  map<SmiRecordId, size_t> FlobRecIdRefs;

  double totalMemSize;
/*
Size of the cached tuples in the ~mtq~. Including the small Flob.

*/

  bool bulkLoadQueue();
  Tuple* getMemoryCachedTuple(size_t i);
  bool deleteIfAllowed(const Flob& flob);
  void cleanDiskBuffer();
};


#endif /* FLOBQUEUE_H_ */
