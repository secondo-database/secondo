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

1 Header File TupleBuffer2.h

June 2009, Sven Jungnickel. Initial version.

2 Overview

Class ~TupleBuffer2~ is a replacement for the class
~TupleBuffer~ in RelationAlgebra. As the former ~TupleBuffer~
has been implemented as a subclass of ~GenericRelation~, which
also allows random access, we have decided to implement the
~TupleBuffer2~ class as a separate new class. The main
differences are

  * Allows only sequential access

  * If the internal buffer overflows not the whole buffer is flushed
to disk, but only the amount of tuples that caused the overflow. Thus
tuples are moved gradually to disk following the FIFO principle.

  * Tuples moved to disk are stored in a temporary ~TupleFile~ (see
RelationAlgebra.h for details)

3 Defines, includes, and constants

*/
#ifndef TUPLEBUFFER2_H_
#define TUPLEBUFFER2_H_

#include <queue>
#include "RelationAlgebra.h"
#include "RTuple.h"

using namespace std;

/*
In order to avoid name conflicts and to simplify replacement
~TupleBuffer2~ has been placed in the namespace ~extrel2~.

*/
namespace extrel2
{
  class TupleBuffer2;
/*
Necessary forward declaration for class ~TupleBuffer2Iterator~.

*/

/*
4 Class ~TupleBuffer2Iterator~

~TupleBuffer2Iterator~ is used to scan a ~TupleBuffer2~ instance
sequentially.

*/
  class TupleBuffer2Iterator
  {
  public:
    TupleBuffer2Iterator(TupleBuffer2& buffer);
/*
The constructor. Initializes a sequential scan of ~buffer~.
A scan always starts with the tuples located on disk and then
proceeds through the in-memory tuples.

*/

    ~TupleBuffer2Iterator();
/*
The destructor.

*/

    Tuple* GetNextTuple();
/*
Returns a pointer to the next tuple from the ~TupleBuffer2~ instance.
If all tuples have been processed 0 is returned.

*/

  private:

    TupleBuffer2& tupleBuffer;
/*
Reference to ~TupleBuffer2~ instance.

*/

    TupleFileIterator* iterDiskBuffer;
/*
~TupleFileIterator~ for disk buffer.

*/

    vector<RTuple>::iterator iterMemoryBuffer;
/*
Iterator for in-memory array ~memoryBufferCopy~.

*/

    vector<RTuple> memoryBufferCopy;
/*
Copy of the FIFO queue content of the ~TupleBuffer2~
instance. The copy is made when the Iterator is
constructed. The copy is necessary because the
used queue implementation by ~TupleBuffer2~ is based
on a STL queue, which does not support iteration.

*/
  };

/*
5 Class ~TupleBuffer2~

*/
  class TupleBuffer2
  {
  public:

    TupleBuffer2( const size_t maxMemorySize = 16 * 1024 * 1024,
                  const size_t ioBufferSize = WinUnix::getPageSize() );
/*
First constructor. Constructs an instance with ~maxMemorySize~ bytes of
internal memory. The I/O buffer size for read/write operations on
disk is set to ~ioBufferSize~. The filename for the ~diskBuffer~ will
be generated automatically.

*/

    TupleBuffer2( const string& pathName,
                  const size_t maxMemorySize = 16 * 1024 * 1024,
                  const size_t ioBufferSize = WinUnix::getPageSize() );
/*
Second constructor. Constructs an instance with a specific filename
~pathName~ for the external buffer and ~maxMemorySize~ bytes of
internal memory. The I/O buffer size for read/write operations on
disk is set to ~ioBufferSize~.

*/

    ~TupleBuffer2();
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

    void AppendTuple( Tuple *t );
/*
Append tuple ~t~ to the buffer.

*/

    TupleBuffer2Iterator* MakeScan();
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

    size_t GetIoBufferSize() { return ioBufferSize; }
/*
Returns the size of the used I/O Buffer in bytes. The I/O Buffer size
is by default set to the page size of the system.

*/

    ostream& Print(ostream& os);
/*
Print the tuple buffer to stream ~os~. This function is used
for debugging purposes.

*/

    friend class TupleBuffer2Iterator;
/*
~TupleBuffer2Iterator~ is declared as friend class, so that it
gains access to the internal and external buffers.

*/

  private:

    const size_t MAX_MEMORY_SIZE;
/*
Maximum size in bytes of the internal memory buffer.

*/

    const size_t ioBufferSize;
/*
I/O Buffer size for read/write operations on disk in bytes. By default
this value is set to the page size of the operating system.

*/

    const string pathName;
/*
Filename for the external buffer of type ~TupleFile~.

*/

    queue<RTuple> memoryBuffer;
/*
Internal memory buffer (FIFO)

*/

    TupleFile* diskBuffer;
/*
External buffer

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

    size_t totalExtSize;
/*
Total size of the extension part of all tuples within the internal
and external buffers.

*/

    size_t totalSize;
/*
Total size of all tuples within the internal and external buffers.

*/
  };
} // end of namespace extrel2

#endif /* TUPLEBUFFER2_H_ */
