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

1 Implementation File ExtRelation2AlgebraTest.cpp

June 2009, Sven Jungnickel. Initial version.

2 Overview

This file contains the implementation of different test operators
which were used to test and benchmark the algebra.

3 Includes

*/
#include "LogMsg.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "CPUTimeMeasurer.h"
#include "StandardTypes.h"
#include "SecondoInterface.h"
#include "WinUnix.h"
#include "FileSystem.h"
#include "TupleBuffer2.h"
#include "TupleQueue.h"
#include "Counter.h"
#include "Symbols.h"

/*
4 Defines

*/

#define HEADLINE_TUPLECOMP "--------------------------- " \
                            "tuplecomp -----------------------------"

#define HEADLINE_HEAPSTL "--------------------------- "\
                          "heapstl -----------------------------"

#define HEADLINE_HEAPSTD "--------------------------- "\
                          "heapstd -----------------------------"

#define HEADLINE_HEAPBUP "--------------------------- "\
                          "heapbup -----------------------------"

#define HEADLINE_HEAPBUP2 "--------------------------- "\
                          "heapbup2 -----------------------------"

#define HEADLINE_HEAPMDR "--------------------------- "\
                          "heapmdr -----------------------------"


/*
5 External linking

*/
extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

/*
6 Utility functions

*/
extrel2::TupleQueueCompare* createCompareObject(Tuple* t)
{
  SortOrderSpecification spec;

  assert(t);

  int nAttrCount = t->GetNoAttributes();

  for(int i = 1; i <= nAttrCount; i++)
  {
    spec.push_back(std::pair<int, bool>(i, true));
  };

  return new extrel2::TupleQueueCompare(spec,nAttrCount);
}

/*
7 Operator ~tuplefiletest~

7.1 LocalInfo class for operator ~tuplefiletest~

*/
namespace extrel2
{

class TupleFileTestLocalInfo
{
public:

  TupleFileTestLocalInfo()
  : file(0)
  , iter(0)
  {}
/*
The constructor. Constructs an empty object.

*/

  ~TupleFileTestLocalInfo()
  {
    if ( iter )
    {
      delete iter;
    }

    if ( file )
    {
      delete file;
    }
  }
/*
The destructor. Deletes the tuple file and iterator instances
if they exist.

*/

  TupleFile* file;
/*
Pointer to ~TupleFile~ instance

*/

  TupleFileIterator* iter;
/*
Pointer to ~TupleFileIterator~ instance

*/
};

/*

7.2 Value mapping function of operator ~tuplefile~

The argument vector ~args~ contains in the first slot ~args[0]~ the
stream. The value mapping function creates a tuple file when the
OPEN message is received and consumes all tuples onto disk. With
the first REQUEST message a sequential tuple file scan is started.
The operator is finished when all tuples from the tuple file
have been read.

*/
int TupleFileValueMap( Word* args, Word& result,
                        int message, Word& local, Supplier s )
{
  // args[0] : stream
  // args[1] : size of I/O buffer in bytes

  switch(message)
  {
    case OPEN:
    {
      Word wTuple(Address(0));
      TupleFileTestLocalInfo* li = new TupleFileTestLocalInfo();

      // open stream and request first tuple
      qp->Open(args[0].addr);
      qp->Request(args[0].addr, wTuple);

      // Set I/O buffer size in bytes
      int args1 = StdTypes::GetInt( args[1] );
      size_t pageSize = WinUnix::getPageSize();
      size_t ioBufferSize = ( args1 == -1 ) ? pageSize : (size_t)args1;

      while( qp->Received(args[0].addr) )
      {
        Tuple* t = static_cast<Tuple*>( wTuple.addr );

        // create tuple file
        if ( li->file == 0 )
        {
          li->file = new TupleFile(t->GetTupleType(), ioBufferSize);
          li->file->Open();
        }

        // append tuple to tuple file on disk
        li->file->Append(t);
        t->DeleteIfAllowed();

        qp->Request(args[0].addr, wTuple);
      }

      // close tuple file
      li->file->Close();

      // store pointer to local info
      local.addr = li;

      return 0;
    }
    case REQUEST:
    {
      TupleFileTestLocalInfo* li =
        static_cast<TupleFileTestLocalInfo*>( local.addr );

      // create iterator if not yet done
      if ( li->iter == 0 )
      {
        li->iter = li->file->MakeScan();
      }

      // get next tuple from disk
      Tuple* t = li->iter->GetNextTuple();

      // store tuple address in result
      result.setAddr(t);

      return result.addr != 0 ? YIELD : CANCEL;
    }

    case CLOSE:
    {
      if( local.addr )
      {
        // close stream
        qp->Close(args[0].addr);

        TupleFileTestLocalInfo* li =
          static_cast<TupleFileTestLocalInfo*>( local.addr );

        delete li;
        local.addr = 0;
      }
      return 0;
    }
  }
  return 0;
}

/*
8 Operator ~tuplebuffer~

8.1 LocalInfo class for operator ~tuplebuffer~

*/

class TupleBufferTestLocalInfo
{
public:
  TupleBufferTestLocalInfo()
  : buffer(0)
  , iter(0)
  {}

  ~TupleBufferTestLocalInfo()
  {
    if ( iter )
    {
      delete iter;
    }

    if ( buffer )
    {
      delete buffer;
    }
  }

  TupleBuffer* buffer;
  GenericRelationIterator* iter;
};

/*

8.2 Value mapping function of operator ~tuplebuffer~

The argument vector ~args~ contains in the first slot ~args[0]~
the stream. The value mapping function creates a tuple buffer of
16 MB size when the OPEN message is received and consumes
all tuples into this buffer. With the first REQUEST message
a sequential buffer scan is started. The operator is finished
when all tuples from the buffer have been read.

*/

int TupleBufferValueMap( Word* args, Word& result,
                           int message, Word& local, Supplier s )
{
  switch(message)
  {
    case OPEN:
    {
      Word wTuple(Address(0));
      TupleBufferTestLocalInfo* li = new TupleBufferTestLocalInfo();

      // open stream and request first tuple
      qp->Open(args[0].addr);
      qp->Request(args[0].addr, wTuple);

      // Read buffer size in KBytes from second argument
      size_t bufferSize = StdTypes::GetInt( args[1] );

      while( qp->Received(args[0].addr) )
      {
        Tuple* t = static_cast<Tuple*>( wTuple.addr );

        // create tuple file
        if ( li->buffer == 0 )
        {
          li->buffer = new TupleBuffer(bufferSize*1024);
        }

        // append tuple to buffer
        li->buffer->AppendTuple(t);
        t->DeleteIfAllowed();

        qp->Request(args[0].addr, wTuple);
      }

      // store pointer to local info
      local.addr = li;

      return 0;
    }
    case REQUEST:
    {
      TupleBufferTestLocalInfo* li =
        static_cast<TupleBufferTestLocalInfo*>( local.addr );

      // create iterator if not yet done
      if ( li->iter == 0 )
      {
        li->iter = li->buffer->MakeScan();
      }

      // get next tuple from disk
      Tuple* t = li->iter->GetNextTuple();

      // store tuple address in result
      result.setAddr(t);

      return result.addr != 0 ? YIELD : CANCEL;
    }

    case CLOSE:
    {
      if( local.addr )
      {
        // close stream
        qp->Close(args[0].addr);

        TupleBufferTestLocalInfo* li =
          static_cast<TupleBufferTestLocalInfo*>( local.addr );

        delete li;
        local.addr = 0;
      }
      return 0;
    }
  }
  return 0;
}

/*
9 Operator ~tuplebuffer2~

9.1 LocalInfo class for operator ~tuplebuffer2~

*/

class TupleBuffer2TestLocalInfo
{
public:
  TupleBuffer2TestLocalInfo()
  : buffer(0)
  , iter(0)
  {}

  ~TupleBuffer2TestLocalInfo()
  {
    if ( iter )
    {
      delete iter;
    }

    if ( buffer )
    {
      delete buffer;
    }
  }

  extrel2::TupleBuffer2* buffer;
  extrel2::TupleBuffer2Iterator* iter;
};

/*

9.2 Value mapping function of operator ~tuplebuffer2~

The argument vector ~args~ contains in the first slot ~args[0]~
the stream. The value mapping function creates a tuple buffer of
16 MB size when the OPEN message is received and consumes
all tuples into this buffer. With the first REQUEST message
a sequential buffer scan is started. The operator is finished
when all tuples from the buffer have been read.

*/

int TupleBuffer2ValueMap( Word* args, Word& result,
                            int message, Word& local, Supplier s )
{
  switch(message)
  {
    case OPEN:
    {
      Word wTuple(Address(0));
      TupleBuffer2TestLocalInfo* li = new TupleBuffer2TestLocalInfo();

      // open stream and request first tuple
      qp->Open(args[0].addr);
      qp->Request(args[0].addr, wTuple);

      // Read buffer size in KBytes from second argument
      size_t bufferSize = StdTypes::GetInt( args[1] );

      // Set I/O buffer size in bytes
      int args2 = StdTypes::GetInt( args[2] );
      size_t pageSize = WinUnix::getPageSize();
      size_t ioBufferSize = ( args2 == -1 ) ? pageSize : (size_t)args2;

      while( qp->Received(args[0].addr) )
      {
        Tuple* t = static_cast<Tuple*>( wTuple.addr );

        // create tuple file
        if ( li->buffer == 0 )
        {
          li->buffer = new extrel2::TupleBuffer2(bufferSize*1024, ioBufferSize);
        }

        // append tuple to buffer
        li->buffer->AppendTuple(t);
        t->DeleteIfAllowed();

        qp->Request(args[0].addr, wTuple);
      }

      // close tuple file
      li->buffer->CloseDiskBuffer();

      // store pointer to local info
      local.addr = li;

      return 0;
    }
    case REQUEST:
    {
      TupleBuffer2TestLocalInfo* li =
        static_cast<TupleBuffer2TestLocalInfo*>( local.addr );

      // create iterator if not yet done
      if ( li->iter == 0 )
      {
        li->iter = li->buffer->MakeScan();
      }

      // get next tuple from disk
      Tuple* t = li->iter->GetNextTuple();

      // store tuple address in result
      result.setAddr(t);

      return result.addr != 0 ? YIELD : CANCEL;
    }

    case CLOSE:
    {
      if( local.addr )
      {
        // close stream
        qp->Close(args[0].addr);

        TupleBuffer2TestLocalInfo* li =
          static_cast<TupleBuffer2TestLocalInfo*>( local.addr );

        delete li;
        local.addr = 0;
      }
      return 0;
    }
  }
  return 0;
}

} // end of namespace extrel2
