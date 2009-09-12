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

1 Implementation File TupleBuffer.cpp

June 2009, Sven Jungnickel. Initial version.

2 Includes

*/

#include "TupleBuffer.h"
#include "FileSystem.h"
#include "WinUnix.h"
#include "Profiles.h"
#include "CharTransform.h"

/*
3 Implementation of class ~TupleBufferIterator~

*/

extrel2::TupleBufferIterator::TupleBufferIterator(extrel2::TupleBuffer& buffer)
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
    Tuple* t = tupleBuffer.memoryBuffer.front();
    tupleBuffer.memoryBuffer.pop();
    tupleBuffer.memoryBuffer.push(t);
    memoryBufferCopy[i] = t;
  }

  iterMemoryBuffer = memoryBufferCopy.begin();
}

extrel2::TupleBufferIterator::~TupleBufferIterator()
{
  if ( iterDiskBuffer != 0 )
  {
    delete iterDiskBuffer;
  }

  memoryBufferCopy.clear();
}

Tuple* extrel2::TupleBufferIterator::GetNextTuple()
{
  if ( !tupleBuffer.inMemory && iterDiskBuffer->MoreTuples() == true )
  {
    return iterDiskBuffer->GetNextTuple();
  }
  else
  {
    if ( iterMemoryBuffer != memoryBufferCopy.end() )
    {
      Tuple* t = *iterMemoryBuffer;
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
4 Implementation of class ~TupleBuffer~

*/

//size_t extrel2::TupleBuffer::IO_BUFFER_SIZE =
//  WinUnix::getPageSize();

size_t
extrel2::TupleBuffer::IO_BUFFER_SIZE =
  SmiProfile::GetParameter( "TupleBuffer",
                            "IOBuffer",
                            WinUnix::getPageSize(),
                            expandVar("$(SECONDO_CONFIG)") );
/*
Set the I/O Buffer size by default to the page size of the
operating system.

*/

bool extrel2::TupleBuffer::traceMode = false;
/*
Initialize the trace mode

*/

extrel2::TupleBuffer::TupleBuffer(const size_t maxMemorySize)
: MAX_MEMORY_SIZE(maxMemorySize)
, pathName("")
, diskBuffer(0)
, inMemory(true)
, totalMemSize(0)
, totalExtSize(0)
, totalSize(0)
{
  if ( traceMode )
  {
    cmsg.info() << "TupleBuffer created (BufferSize: "
                << ( maxMemorySize / 1024 ) << " Kbyte)" << endl;
    cmsg.send();
  }
}

extrel2::TupleBuffer::TupleBuffer( const string& pathName,
                                      const size_t maxMemorySize )
: MAX_MEMORY_SIZE(maxMemorySize)
, pathName(pathName)
, diskBuffer(0)
, inMemory(true)
, totalMemSize(0)
, totalExtSize(0)
, totalSize(0)
{
  if ( traceMode )
  {
    cmsg.info() << "TupleBuffer created (File: " << pathName
                << ", BufferSize: " << ( maxMemorySize / 1024 )
                << " Kbyte)" << endl;
    cmsg.send();
  }
}

extrel2::TupleBuffer::~TupleBuffer()
{
  Clear();
}

void extrel2::TupleBuffer::Clear()
{
  while( !memoryBuffer.empty() )
  {
    Tuple* t = memoryBuffer.front();
    memoryBuffer.pop();
    t->DeleteIfAllowed();
  }

  if ( diskBuffer )
  {
    delete diskBuffer;
  }

  totalMemSize = 0;
  totalSize = 0;
  totalExtSize = 0;
  inMemory = true;

  if ( traceMode )
  {
    cmsg.info() << "TupleBuffer cleared" << endl;
    cmsg.send();
  }
}

void extrel2::TupleBuffer::AppendTuple(Tuple* t)
{
  if( inMemory )
  {
    if( totalMemSize + t->GetMemSize() <= MAX_MEMORY_SIZE )
    {
      // insert new tuple at back of FIFO queue
      intoQueue(t);
      updateSizes(t);
    }
    else
    {
      if ( pathName == "" )
      {
        diskBuffer = new TupleFile( t->GetTupleType(),
                                    IO_BUFFER_SIZE );
      }
      else
      {
        diskBuffer = new TupleFile( t->GetTupleType(),
                                    pathName, IO_BUFFER_SIZE);
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
        updateSizes(t);

        // write front tuple of FIFO queue to disk
        oneToDisk();

        // insert new tuple at back of FIFO queue
        intoQueue(t);
      }
      else
      {
        updateSizes(t);
        diskBuffer->Append(t);
        t->DeleteIfAllowed();
      }

      // Memory is now completely used
      inMemory = false;
    }
  }
  else
  {
    if ( !memoryBuffer.empty() )
    {
      updateSizes(t);

      // write front tuple of FIFO queue to disk
      oneToDisk();

      // insert new tuple at back of FIFO queue
      intoQueue(t);
    }
    else
    {

      updateSizes(t);
      diskBuffer->Append(t);
      t->DeleteIfAllowed();
    }
  }
}

extrel2::TupleBufferIterator* extrel2::TupleBuffer::MakeScan()
{
  return new extrel2::TupleBufferIterator(*this);
}

void extrel2::TupleBuffer::CloseDiskBuffer()
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

