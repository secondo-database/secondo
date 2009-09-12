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

1 Header File TupleBuffer.h

June 2009, Sven Jungnickel. Initial version.

2 Overview

Class ~extrel2::TupleBuffer~ is a replacement for the class
~TupleBuffer~. As the former ~TupleBuffer~ has been implemented
as a subclass of ~GenericRelation~, which also allows random access,
we have decided to implement the ~TupleBuffer~ class as a separate
new class. The main differences are

  * Allows only sequential access

  * If the internal buffer overflows not the whole buffer is flushed
to disk, but only the amount of tuples that caused the overflow. Thus
tuples are moved gradually to disk following the FIFO principle.

  * Tuples moved to disk are stored in a temporary ~TupleFile~ (see
RelationAlgebra.h for details)

3 Defines, includes, and constants

*/
#ifndef TUPLEBUFFER_H_
#define TUPLEBUFFER_H_

#include <queue>
#include "RelationAlgebra.h"

using namespace std;

/*
In order to avoid name conflicts and to simplify replacement
~TupleBuffer~ has been placed in the namespace ~extrel2~.

*/
namespace extrel2
{
  class TupleBuffer;
/*
Necessary forward declaration for class ~TupleBufferIterator~.

*/

/*
4 Class ~TupleBufferIterator~

~TupleBufferIterator~ is used to scan a ~TupleBuffer~ instance
sequentially.

*/
  class TupleBufferIterator
  {
  public:
    TupleBufferIterator(TupleBuffer& buffer);
/*
The constructor. Initializes a sequential scan of ~buffer~.
A scan always starts with the tuples located on disk and then
proceeds through the in-memory tuples.

*/

    ~TupleBufferIterator();
/*
The destructor.

*/

    Tuple* GetNextTuple();
/*
Returns a pointer to the next tuple from the ~TupleBuffer~.
If all tuples have been processed 0 is returned.

*/

  private:

    TupleBuffer& tupleBuffer;
/*
Constant reference to ~TupleBuffer~ instance.

*/

    TupleFileIterator* iterDiskBuffer;
/*
~TupleFileIterator~ for disk buffer.

*/

    vector<Tuple*>::iterator iterMemoryBuffer;
/*
Iterator for in-memory array ~memoryBufferCopy~.

*/

    vector<Tuple*> memoryBufferCopy;
/*
Copy of the FIFO queue content of the ~TupleBuffer~
instance. The copy is made when the Iterator is
constructed. The copy is necessary because the
used queue implementation by ~TupleBuffer~ is based
on a STL queue, which does not support iteration.

*/
  };

/*
5 Class ~TupleBuffer~

*/
  class TupleBuffer
  {
  public:

    TupleBuffer( const size_t maxMemorySize = 16 * 1024 * 1024 );
/*
First constructor. Constructs an instance with ~maxMemorySize~ Bytes of
internal memory. The filename for the ~diskBuffer~ will be generated
automatically.

*/

    TupleBuffer( const string& pathName,
                  const size_t maxMemorySize = 16 * 1024 * 1024 );
/*
Second constructor. Constructs an instance with a specific filename
~pathName~ for the external buffer and ~maxMemorySize~ Bytes of
internal memory.

*/

    ~TupleBuffer();
/*
The destructor.

*/

    void Clear();
/*
Cleans the internal and external buffers.

*/

    void CloseDiskBuffer();
/*
Closes the external buffer of type ~TupleFile~ if open.

*/

    inline int GetNoTuples() const
    {
      if ( diskBuffer != 0 )
      {
        return ( (int)memoryBuffer.size() + diskBuffer->GetNoTuples() );
      }
      else
      {
        return (int)memoryBuffer.size();
      }
    }
/*
Returns the number of tuples within the internal and external buffers.

*/

    inline size_t GetTotalExtSize() const { return totalExtSize; };
/*
Returns the total size of the extension part of all tuples
within the internal and external buffers.

*/

    inline size_t GetTotalSize() const { return totalSize; };
/*
Returns the total size of all tuples within the internal and
external buffers.

*/

    void   AppendTuple( Tuple *t );
/*
Append tuple ~t~ to the buffer.

*/

    TupleBufferIterator* MakeScan();
/*
Start a sequential scan of the tuples in the buffer.

*/

    bool IsEmpty() const { return inMemory ? memoryBuffer.empty() : false; }
/*
Returns true if the buffer is empty, otherwise false.

*/

    inline size_t FreeBytes() const
      { return inMemory ? ( MAX_MEMORY_SIZE - totalSize ) : 0; }
/*
Returns the number of free bytes in the internal buffer.

*/

    inline bool InMemory() const { return inMemory; }
/*
Returns true if all tuples are in the internal memory buffer.

*/

    static size_t GetIoBufferSize() { return IO_BUFFER_SIZE; }
/*
Returns the size of the used I/O Buffer in bytes.
The I/O Buffer size is by default set to the
page size of the system.

*/

    static void SetIoBufferSize(size_t s) { IO_BUFFER_SIZE = s; }
/*
Sets the size of the used I/O Buffer in bytes.

*/

    friend class TupleBufferIterator;
/*
~TupleBufferIterator~ is declared as friend class, so that it
gains access to the internal and external buffers.

*/

  protected:

/*
Takes the front tuple from the internal memory queue and
write it to the disk buffer.

*/
    inline void oneToDisk()
    {
      // Take the front tuple from the queue and append
      // it to the disk buffer
      Tuple* t = memoryBuffer.front();
      diskBuffer->Append(t);

      // discard the front tuple from the queue
      memoryBuffer.pop();
      t->DeleteIfAllowed();
    }

/*
Puts tuple ~t~ into the internal memory queue.

*/
    inline void intoQueue(Tuple* t)
    {
      memoryBuffer.push(t);
      t->IncReference();
    }

/*
Update the size internal statistics.

*/
    inline void updateSizes(Tuple* t)
    {
      totalMemSize += t->GetMemSize();
      totalSize += t->GetSize();
      totalExtSize += t->GetExtSize();
    }

  private:

  const size_t MAX_MEMORY_SIZE;
/*
Maximum size in bytes of the internal memory buffer.

*/

  static size_t IO_BUFFER_SIZE;
/*
I/O Buffer size for read/write operations. By default
this value is set to the page size of the operating system.

*/

  const string pathName;
/*
Filename for the external buffer of type ~TupleFile~.

*/

  queue<Tuple*> memoryBuffer;
/*
Internal memory buffer (FIFO)

*/

  TupleFile* diskBuffer;
/*
External buffer

*/

  TupleType *tupleType;
/*
~TupleType~ for the buffer.

*/

  bool inMemory;
/*
Flag which is true if all tuples are kept in internal memory.
The flag is set to false when the first tuple is moved to disk.

*/

  static bool traceMode;
/*
Flag to control the trace mode. If set to true log messages
will be produced on the standard output.

*/

  size_t totalMemSize;
/*
Used internal memory in bytes.

*/

  size_t totalExtSize;
  size_t totalSize;
/*
Internal tuple size statistics

*/
  };
}

#endif /* TUPLEBUFFER_H_ */
