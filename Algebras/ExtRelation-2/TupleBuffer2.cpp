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

1 Implementation File TupleBuffer2.cpp

June 2009, Sven Jungnickel. Initial version.

2 Includes

*/

#include "TupleBuffer2.h"
#include "FileSystem.h"
#include "WinUnix.h"
#include "Profiles.h"
#include "CharTransform.h"

/*
3 Implementation of class ~TupleBuffer2Iterator~

*/
namespace extrel2
{

TupleBuffer2Iterator::TupleBuffer2Iterator
(extrel2::TupleBuffer2& buffer)
: tupleBuffer(buffer)
, iterDiskBuffer(0)
{
  if ( !tupleBuffer.inMemory )
  {
    iterDiskBuffer = tupleBuffer.diskBuffer->MakeScan();
  }

  // Make a copy of the queue content so that
  // we can iterate through in sort order
  int nTuples = tupleBuffer.memoryBuffer.size();
  memoryBufferCopy.resize(nTuples);

  // Iterate through the queue once, remove each tuple
  // from the front (smallest tuple) and append it to
  // the rear of the queue. Put a copy of each tuple
  // pointer into the array
  for (int i = 0; i < nTuples; i++)
  {
    RTuple ref = tupleBuffer.memoryBuffer.front();
    tupleBuffer.memoryBuffer.pop();
    tupleBuffer.memoryBuffer.push(ref);
    memoryBufferCopy[i] = ref;
  }

  iterMemoryBuffer = memoryBufferCopy.begin();
}

TupleBuffer2Iterator::~TupleBuffer2Iterator()
{
  if ( iterDiskBuffer != 0 )
  {
    delete iterDiskBuffer;
  }

  memoryBufferCopy.clear();
}

Tuple* TupleBuffer2Iterator::GetNextTuple()
{
  if ( !tupleBuffer.inMemory && iterDiskBuffer->MoreTuples() == true )
  {
    return iterDiskBuffer->GetNextTuple();
  }
  else
  {
    if ( iterMemoryBuffer != memoryBufferCopy.end() )
    {
      Tuple* t = (*iterMemoryBuffer).tuple;
      t->IncReference();
      iterMemoryBuffer++;
      return t;
    }
    else
    {
      return 0;
    }
  }
}

/*
4 Implementation of class ~TupleBuffer2~

*/

bool extrel2::TupleBuffer2::traceMode = false;
/*
Initialize the trace mode

*/

TupleBuffer2::TupleBuffer2( const size_t maxMemorySize,
                              const size_t ioBufferSize )
: MAX_MEMORY_SIZE(maxMemorySize)
, ioBufferSize(ioBufferSize)
, pathName("")
, diskBuffer(0)
, inMemory(true)
, totalExtSize(0)
, totalSize(0)
{
  if ( traceMode )
  {
    cmsg.info() << "TupleBuffer2 created (BufferSize: "
                << ( maxMemorySize / 1024 ) << " Kbyte, I/O Buffer: "
                << ioBufferSize << " byte)" << endl;
    cmsg.send();
  }
}

TupleBuffer2::TupleBuffer2( const string& pathName,
                              const size_t maxMemorySize,
                              const size_t ioBufferSize )
: MAX_MEMORY_SIZE(maxMemorySize)
, ioBufferSize(ioBufferSize)
, pathName(pathName)
, diskBuffer(0)
, inMemory(true)
, totalExtSize(0)
, totalSize(0)
{
  if ( traceMode )
  {
    cmsg.info() << "TupleBuffer2 created (BufferSize: "
                << ( maxMemorySize / 1024 ) << " Kbyte, I/O Buffer: "
                << ioBufferSize << " byte)" << endl;
    cmsg.send();
  }
}

TupleBuffer2::~TupleBuffer2()
{
  Clear();
}

void TupleBuffer2::Clear()
{
  if ( memoryBuffer.empty() && diskBuffer == 0)
  {
    return;
  }

  while( !memoryBuffer.empty() )
  {
    memoryBuffer.pop();
  }

  if ( diskBuffer )
  {
    delete diskBuffer;
    diskBuffer = 0;
  }

  totalSize = 0;
  totalExtSize = 0;
  inMemory = true;

  if ( traceMode )
  {
    cmsg.info() << "TupleBuffer2 cleared" << endl;
    cmsg.send();
  }
}

void TupleBuffer2::AppendTuple(Tuple* t)
{
  totalSize += t->GetSize();
  totalExtSize += t->GetMemSize();

  if( inMemory )
  {
    if( totalExtSize <= MAX_MEMORY_SIZE )
    {
      // insert new tuple at back of FIFO queue
      memoryBuffer.push(t);
    }
    else
    {
      if ( pathName == "" )
      {
        diskBuffer = new TupleFile( t->GetTupleType(),
                                    ioBufferSize );
      }
      else
      {
        diskBuffer = new TupleFile( t->GetTupleType(),
                                    pathName, ioBufferSize );
      }

      if ( traceMode )
      {
        cmsg.info() << "Created diskBuffer -> "
                    << diskBuffer->GetPathName() << endl;
        cmsg.send();
      }

      // Open temporary tuple file
      diskBuffer->Open();

      if ( !memoryBuffer.empty() )
      {
        // write front tuple of FIFO queue to disk
        diskBuffer->Append(memoryBuffer.front().tuple);

        // discard the front tuple from the queue
        memoryBuffer.pop();

        // insert new tuple at back of FIFO queue
        memoryBuffer.push(t);
      }
      else
      {
        diskBuffer->Append(t);
      }

      // Memory is now completely used
      inMemory = false;
    }
  }
  else
  {
    if ( !memoryBuffer.empty() )
    {
      // write front tuple of FIFO queue to disk
      diskBuffer->Append(memoryBuffer.front().tuple);

      // discard the front tuple from the queue
      memoryBuffer.pop();

      // insert new tuple at back of FIFO queue
      memoryBuffer.push(t);
    }
    else
    {
      diskBuffer->Append(t);
    }
  }

  return;
}

TupleBuffer2Iterator* extrel2::TupleBuffer2::MakeScan()
{
  return new TupleBuffer2Iterator(*this);
}

void TupleBuffer2::CloseDiskBuffer()
{
  if ( diskBuffer )
  {
    diskBuffer->Close();

    if ( traceMode )
    {
      cmsg.info() << "Closing DiskBuffer of TupleBuffer" << endl;
      cmsg.send();
    }
  }
}

ostream& TupleBuffer2::Print(ostream& os)
{
  Tuple* t;

  os << "---------- TupleBuffer2 content ----------" << endl
     << "Tuples in memory: " << this->memoryBuffer.size() << endl
     << "Tuples on disk: "
     << ( this->diskBuffer ? this->diskBuffer->GetNoTuples() : 0 )
     << endl;

  for (size_t i = 0; i < this->memoryBuffer.size(); i++)
  {
    RTuple ref = memoryBuffer.front();
    memoryBuffer.pop();
    memoryBuffer.push(ref);
    os << "Mem: " << *(ref.tuple)
       << " Refs: " << ref.tuple->GetNumOfRefs()
       << " Nr: " << i << endl;
  }

  if ( diskBuffer )
  {
    TupleFileIterator* iter = this->diskBuffer->MakeScan();

    int counter = 0;
    while ( ( t = iter->GetNextTuple() ) != 0 )
    {
      os << "Hdd: " << *t
         << " Refs: " << t->GetNumOfRefs() - 1
         << " Nr: " << counter++ << endl;
      t->DeleteIfAllowed();
    }

    delete iter;
  }

  return os;
}

} // end of namespace extrel2
