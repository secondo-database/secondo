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

[1] Header File of Module FLOB

Stefan Dieker, 03/05/98

Markus Spiekermann, 14/05/02. Begin of porting to the new SecondoSMI.

Mirco G[ue]nster, 31/09/02 End of porting to the new SecondoSMI.

Victor Almeida, 24/04/03. Adapting the class to accept standalone 
objects and objects inside tuples transparently.

January 2006 Victor Almeida created the FLOB cache. Some assertions
were removed, since the code is stable.

1 Defines

*/

#ifndef FLOB_H
#define FLOB_H

/*

2 Includes

*/
#include "SecondoSMI.h"
#include "QueryProcessor.h"

enum FLOB_Type {Destroyed, InMemory, InMemoryCached, 
                InDiskSmall, InDiskLarge};

extern QueryProcessor *qp;

/*
3 class FLOB

This class implements a FLOB which define a new large object 
abstraction that offers most access methods of the original one.

It defines a threshold size namely SWITCH\_THRESHOLD as a static 
attribute. This value defines a limit between "small" and "large" 
values referring its size.

A small FLOB is stored in a main memory structure whereas a large 
FLOB is stored into a seperate SmiRecord in an SmiFile.

*/
class FLOB
{

  public:

    static const size_t SWITCH_THRESHOLD;
/*
This is the treshold size for a FLOB. Whenever the size of this 
FLOB exceeds the threshold size the data will stored in a separate 
file for LOBs.

*/

    inline FLOB() {}
/*
This constructor should not be used.

3.1 Constructor

Create a new FLOB from scratch.

*/
    inline FLOB( size_t sz ):
    type( InMemory )
    {
      size = sz;
      if( sz > 0 )
        fd.inMemory.buffer = (char*)malloc( sz );
      else
        fd.inMemory.buffer = 0;
      fd.inMemory.canDelete = true;
    }

/*
3.3 Destructor

Deletes the FLOB instance.

*/
    inline ~FLOB()
    {
      assert( type != InMemoryCached );
      // The cached FLOBs are never deleted because they are created
      // with malloc and destroyed with free
      if( type == InMemory && fd.inMemory.canDelete && fd.inMemory.buffer != 0 )
        free( fd.inMemory.buffer );
    }

/*
3.4 ReInit

Re-initializes the FLOB.

*/
    inline void ReInit()
    {
      if( type == InMemoryCached )
      {
        assert( IsLob() );
        SmiFileId lobFileId = fd.inMemoryCached.lobFileId;
        SmiRecordId lobId = fd.inMemoryCached.lobId;    
        type = InDiskLarge;
        fd.inDiskLarge.lobFileId = lobFileId;
        fd.inDiskLarge.lobId = lobId;
      }
    }
/*
3.4 BringToMemory

Brings a disk lob to memory, i.e., converts a flob in ~InDiskLarge~
state to a ~InMemory~ state.

*/
    const char *BringToMemory() const;

/*
3.5 Get

Read 

*/
    inline void Get( size_t offset, const char **target ) const
    {
      if( type == InMemory )
        *target = fd.inMemory.buffer + offset;
      else if( type == InMemoryCached )
        *target = fd.inMemoryCached.buffer + offset;
      else if( type == InDiskLarge )
      {
        BringToMemory();
        Get( offset, target );
      }
      else 
        assert( false );
    }

/*
3.6 Put

Write Flob data into source.

*/
    inline void Put( size_t offset, size_t length, const void *source )
    {
      if( type == InMemory )
        memcpy( fd.inMemory.buffer + offset, source, length );
      else
        assert( false );
    }

/*
3.7 Size

Returns the size of a FLOB.

*/
    inline size_t Size() const
    {
      return size;
    }

/*
3.8 Resize

Resizes the FLOB.

*/
    void Resize( size_t size );

/*
3.8 Clear

Clears the FLOB.

*/
    inline void Clear()
    {
      assert( type != Destroyed );
      if( type == InMemory && fd.inMemory.canDelete && size > 0 )
        free( fd.inMemory.buffer );

      type = InMemory;
      fd.inMemory.buffer = 0;
      fd.inMemory.canDelete = true;
      size = 0;
    }

/*
3.8 Clean

Cleans the FLOB, removing it from memory. If it is cached, then a
reference in the cache is removed.

*/
    inline void Clean()
    {
      assert( type != Destroyed );
      if( type == InMemory && fd.inMemory.canDelete && size > 0 )
        free( fd.inMemory.buffer );
      else if( type == InMemoryCached )
        qp->GetFLOBCache()->Release( fd.inMemoryCached.lobFileId,
                                     fd.inMemoryCached.lobId );

      type = InMemory;
      fd.inMemory.buffer = 0;
      fd.inMemory.canDelete = true;
      size = 0;
    }

/*
3.8 Destroy

Destroys the physical representation of the FLOB.

*/
    void Destroy();

/*
3.10 SaveToLob

Saves the FLOB to the LOB file. The FLOB must be a LOB. The type is 
set to ~InDiskLarge~.

*/
    void SaveToLob( SmiRecordId& lobFileId, SmiRecordId lobId = 0 ) const;

/*
3.10 SetLobFile

Sets the LOB file. The FLOB must be a LOB.

*/
    inline void SetLobFileId( SmiFileId lobFileId )
    {
      assert( type == InDiskLarge );
      fd.inDiskLarge.lobFileId = lobFileId;
    }


/*
3.11 SaveToExtensionTuple

Saves the FLOB to a buffer of an extension tuple and sets its type 
to ~InDiskSmall~.

*/
    inline void SaveToExtensionTuple( void *extensionTuple ) const
    {
      assert( type == InMemory || type == InDiskSmall );
      if( type == InMemory && size > 0 )
      {
        if( extensionTuple != 0 )
          memcpy( extensionTuple, fd.inMemory.buffer, size );
        if( fd.inMemory.canDelete && size > 0 )
          free( fd.inMemory.buffer );
        fd.inMemory.buffer = 0;
        fd.inMemory.canDelete = true;
      }
      type = InDiskSmall;
    }

/*
3.12 ReadFromExtensionTuple

Reads the FLOB value from an extension tuple. There are two ways of 
reading, one uses a Prefetching Iterator and the other reads 
directly from the SMI Record.

The FLOB must be small.

*/
    inline void ReadFromExtensionTuple( char *extensionPtr )
    {
      assert( type == InDiskSmall );
      type = InMemory;
      if( size > 0 )
      {
        fd.inMemory.buffer = extensionPtr;
        fd.inMemory.canDelete = false;
      }
      else
      {
        fd.inMemory.buffer = 0;
        fd.inMemory.canDelete = true;
      }
    }

/*
3.10 IsLob

Returns true, if value stored in underlying LOB, otherwise false.

*/
    inline bool IsLob() const
    {
      assert( type != Destroyed );
      if( type == InDiskLarge || type == InMemoryCached )
        return true;
      else if( type == InMemory && size > SWITCH_THRESHOLD )
        return true;
      return false;
    }


/*
3.11 GetType

*/
    inline FLOB_Type GetType() const
    {
      return type;
    }


  protected:

/*
3.12 Attributes

*/
    mutable FLOB_Type type;
    size_t size;

    mutable 
    union FLOB_Descriptor
    {
      struct InMemory
      {
        char *buffer;
        bool canDelete;
      } inMemory;

      struct InMemoryCached
      {
        const char *buffer;
        SmiFileId lobFileId;
        SmiRecordId lobId;
      } inMemoryCached;

      struct InDiskLarge
      {
        SmiFileId lobFileId;
        SmiRecordId lobId;
      } inDiskLarge;

    } fd;
};

#endif
