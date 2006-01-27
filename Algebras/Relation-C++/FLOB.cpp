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
#include <iostream>
#include <stdlib.h>
#include <cassert>

/*

2 Implementation of class FLOB.

2.1 The threshold size is set to 1K.

*/
const size_t FLOB::SWITCH_THRESHOLD = 1024;

/*
2.4 BringToMemory

Brings a disk lob to memory, i.e., converts a flob in ~InDiskLarge~
state to a ~InMemory~ state.

*/
char *FLOB::BringToMemory() const
{
  if( type == InDiskLarge )
  {
    char *buffer;
    bool cached =  
      qp->GetFLOBCache()->GetFLOB( fd.inDiskLarge.lobFileId, 
                                   fd.inDiskLarge.lobId, 
                                   size, false, buffer );

    SmiFileId fileId = fd.inDiskLarge.lobFileId;
    SmiRecordId lobId = fd.inDiskLarge.lobId;

    if( cached )
    {
      type = InMemoryCached;
      fd.inMemoryCached.buffer = buffer;
      fd.inMemoryCached.lobFileId = fileId;
      fd.inMemoryCached.lobId = lobId;
    }
    else
    {
      type = InMemory;
      fd.inMemory.buffer = buffer;
      fd.inMemory.canDelete = true;
    }
    return buffer;
  }
  else if( type == InMemory )
    return fd.inMemory.buffer;
  else if( type == InMemoryCached )
    return fd.inMemoryCached.buffer;

  return 0;
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
    if( size > 0 && fd.inMemory.canDelete )
    {
      free( fd.inMemory.buffer );
      fd.inMemory.buffer = 0;
      fd.inMemory.canDelete = true;
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
2.10 Resize

Resizes the FLOB.

*/
void FLOB::Resize( size_t newSize )
{
  assert( type != Destroyed );
  assert( newSize > 0 ); // Use Clear

  if( type == InMemory )
  {
    assert( fd.inMemory.canDelete );

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
2.10 SaveToLob

*/
void FLOB::SaveToLob( SmiFileId& lobFileId, SmiRecordId lobId ) const
{
  assert( (type == InMemory || type == InMemoryCached) && 
          size > SWITCH_THRESHOLD );

  if( type == InMemory ) 
  {
    qp->GetFLOBCache()->PutFLOB( lobFileId, lobId, 
                                 size, false, 
                                 fd.inMemory.buffer );
    assert( fd.inMemory.canDelete );
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
    qp->GetFLOBCache()->PutFLOB( lobFileId, lobId, 
                                 size, false, 
                                 fd.inMemoryCached.buffer );
    fd.inMemoryCached.buffer = 0;
    type = InDiskLarge;
    fd.inDiskLarge.lobFileId = lobFileId;
    fd.inDiskLarge.lobId = lobId;
  }
}

