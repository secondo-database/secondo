/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[ae] [\"a]
//[ue] [\"u]
//[oe] [\"o]

[1] Implementation File of Module FLOB

Stefan Dieker, 03/05/98

Markus Spiekermann, 14/05/02. Begin of porting to the new 
SecondoSMI.

Mirco G[ue]nster, 31/09/02 End of porting to the new SecondoSMI.

Victor Almeida, 24/04/03. Adapting the class to accept standalone 
objects and objects inside tuples transparently.

January 2006 Victor Almeida created the FLOB cache. Some assertions 
were removed, since the code is stable.

1 Includes

*/
#include "FLOB.h"
#include "FLOBCache.h"
#include "QueryProcessor.h"
#include <iostream>
#include <stdlib.h>
#include <cassert>

extern QueryProcessor *qp;

/*

2 Implementation of class FLOB.

2.1 The threshold size is set to 1K.

*/
const size_t FLOB::SWITCH_THRESHOLD = 1024;

/*

2.2 Constructor.

Create a new InMemory FLOB and initializes it with
size ~sz~.

*/
FLOB::FLOB( size_t sz ) :
type( InMemory )
{
  size = sz;
  if( sz > 0 )
    fd.inMemory.buffer = (char*)malloc( sz );
  else
    fd.inMemory.buffer = 0;
}

/*
2.5 Destructor.

Destroy LOB instance.

*/
FLOB::~FLOB()
{
  assert( type != InMemoryCached );
  // The cached FLOBs are never deleted because they are created
  // with malloc and destroyed with free 

  if( type == InMemory && fd.inMemory.buffer != 0 )
    free( fd.inMemory.buffer );
}

/*
2.4 BringToMemory

Brings a disk lob to memory, i.e., converts a flob in ~InDiskLarge~
state to a ~InMemory~ state.

*/
char *FLOB::BringToMemory()
{
  if( type == InDiskLarge )
  {
    char *buffer = 
      qp->GetFLOBCache()->GetFLOB( fd.inDiskLarge.lobFileId, 
                                   fd.inDiskLarge.lobId, 
                                   size, false );

    SmiFileId fileId = fd.inDiskLarge.lobFileId;
    SmiRecordId lobId = fd.inDiskLarge.lobId;

    type = InMemoryCached;
    fd.inMemoryCached.buffer = buffer;
    fd.inMemoryCached.lobFileId = fileId;
    fd.inMemoryCached.lobId = lobId;
  }
  return fd.inMemoryCached.buffer;
}

/*
2.7 Get

Read by copying

*/
void FLOB::Get( size_t offset, size_t length, void *target )
{
  if( type == InMemory )
    memcpy( target, fd.inMemory.buffer + offset, length );
  else if( type == InMemoryCached )
    memcpy( target, fd.inMemoryCached.buffer + offset, length );
  else if( type == InDiskLarge )
  {
    BringToMemory();
    Get( offset, length, target );
  }
  else
    assert( false );
}

/*
2.8 Put

Write Flob data into source.

*/
void FLOB::Put( size_t offset, size_t length, const void *source)
{
  if( type == InMemory )
    memcpy( fd.inMemory.buffer + offset, source, length );
  else
    assert( false );
}

/*
2.9 Size

Returns the size of a FLOB.

*/
size_t FLOB::Size() const
{
  return size;
}

/*
2.10 Destroy

Destroys the physical representation of the FLOB.

*/
void FLOB::Destroy()
{
  assert( type != Destroyed );

  if( type == InMemory )
  {
    if( size > 0 )
    {
      free( fd.inMemory.buffer );
      fd.inMemory.buffer = 0;
    }
  }
  else if( type == InMemoryCached )
  {
    qp->GetFLOBCache()->Destroy( fd.inMemoryCached.lobFileId, 
                                 fd.inMemoryCached.lobId );
  }
  else if( type == InDiskLarge )
  {
    qp->GetFLOBCache()->Destroy( fd.inDiskLarge.lobFileId, 
                                 fd.inDiskLarge.lobId );
  }
  size = 0;
  type = Destroyed;
}

/*
2.10 Clear

Clears the FLOB.

*/
void FLOB::Clear()
{
  assert( type != Destroyed );
  if( type == InMemory && size > 0 )
    free( fd.inMemory.buffer );

  type = InMemory;
  fd.inMemory.buffer = 0;
  size = 0;
}

/*
2.10 Clean

Cleans the FLOB, removing it from memory. If it is cached, then a 
reference in the cache is removed.

*/
void FLOB::Clean()
{
  assert( type != Destroyed );
  if( type == InMemory && size > 0 )
    free( fd.inMemory.buffer );
  else if( type == InMemoryCached )
    qp->GetFLOBCache()->Release( fd.inMemoryCached.lobFileId,  
                                 fd.inMemoryCached.lobId ); 

  type = InMemory;
  fd.inMemory.buffer = 0;
  size = 0;
}

/*
2.10 Resize

Resizes the FLOB.

*/
void FLOB::Resize( size_t newSize )
{
  assert( type != Destroyed );
  assert( newSize > 0 ); // Use Clear

  if( type == InMemory )
  {
    if( size == 0 )
      fd.inMemory.buffer = (char *) malloc( newSize );
    else
      fd.inMemory.buffer = 
        (char *)realloc( fd.inMemory.buffer, newSize );
  }
  else
    // This code cannot be reached because the other
    // cases are not implemented yet.
    assert( false );

  size = newSize;
}

/*
2.10 SetLobFileId

*/
void FLOB::SetLobFileId( SmiFileId lobFileId )
{
  assert( type == InDiskLarge );
  fd.inDiskLarge.lobFileId = lobFileId;
}

/*
2.10 SaveToLob

*/
void FLOB::SaveToLob( SmiFileId& lobFileId, SmiRecordId lobId )
{
  assert( (type == InMemory || type == InMemoryCached) && 
          size > SWITCH_THRESHOLD );

  if( type == InMemory ) 
  {
    qp->GetFLOBCache()->PutFLOB( lobFileId, lobId, size, false, 
                                 fd.inMemory.buffer );
    free( fd.inMemory.buffer );
    fd.inMemory.buffer = 0;

    type = InDiskLarge;
    fd.inDiskLarge.lobFileId = lobFileId;
    fd.inDiskLarge.lobId = lobId;
  }
  else // type == InMemoryCached
  {
    qp->GetFLOBCache()->Release( fd.inMemoryCached.lobFileId, 
                                 fd.inMemoryCached.lobId ); 
    qp->GetFLOBCache()->PutFLOB( lobFileId, lobId, size, false, 
                                 fd.inMemoryCached.buffer );
    fd.inMemoryCached.buffer = 0;

    type = InDiskLarge;
    fd.inDiskLarge.lobFileId = lobFileId;
    fd.inDiskLarge.lobId = lobId;
  }
}


/*
3.11 SaveToExtensionTuple

Saves the FLOB to a buffer of an extension tuple and sets its type 
to ~InDiskSmall~.

*/
void FLOB::SaveToExtensionTuple( void *extensionTuple )
{
  assert( type == InMemory );

  if( size > 0 )
  {
    if( extensionTuple != 0 )
      Get( 0, size, extensionTuple );

    free( fd.inMemory.buffer );
    fd.inMemory.buffer = 0;
  }

  type = InDiskSmall;
}


/*
3.12 ReadFromExtensionTuple

Reads the FLOB value from an extension tuple. There are two ways of 
reading, one uses a Prefetching Iterator and the other reads 
directly from the SMI Record. The FLOB must be small.

*/
size_t FLOB::ReadFromExtensionTuple( PrefetchingIterator& iter, 
                                     const size_t offset )
{
  assert( type == InDiskSmall );

  size_t result = 0;

  type = InMemory;

  if( size > 0 )
  {
    fd.inMemory.buffer = (char*)malloc( size );
    result = 
      iter.ReadCurrentData( fd.inMemory.buffer, size, offset );
  }
  else
    fd.inMemory.buffer = 0;

  return result;
}

size_t FLOB::ReadFromExtensionTuple( SmiRecord& record, 
                                     const size_t offset )
{
  assert( type == InDiskSmall );

  size_t result = 0;

  type = InMemory;

  if( size > 0 )
  {
    fd.inMemory.buffer = (char*)malloc( size );
    result = record.Read( fd.inMemory.buffer, size, offset );
  }
  else
    fd.inMemory.buffer = 0;

  return result;
}

/*
2.11 IsLob

Returns true, if value stored in underlying LOB, otherwise false.

*/
bool FLOB::IsLob() const
{
  assert( type != Destroyed );

  if( type == InDiskLarge || type == InMemoryCached )
    return true;
  else if( type == InMemory && size > SWITCH_THRESHOLD )
    return true;
  return false;
}

/*
2.12 GetType

*/
FLOB_Type FLOB::GetType() const
{
  return type;
}

