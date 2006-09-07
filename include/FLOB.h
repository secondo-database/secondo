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

//paragraph [10] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[ae] [\"a]
//[ue] [\"u]
//[oe] [\"o]

[10] Header File of Module FLOB

Stefan Dieker, 03/05/98. FLOB implementation in Secondo with the Shore based SMI.

Markus Spiekermann, 14/05/02. Starting to port the ~TupleManager~ to the Berkeley-DB based SMI.

Mirco G[ue]nster, 31/09/02. First stable version.

Victor Almeida, 24/04/03. Adapting the class to accept standalone 
objects and objects inside tuples transparently.

January 2006, Victor Almeida created the FLOB cache. Some assertions
were removed, since the code is stable.

August 2006, M. Spiekermann. Adding more comprehensive documentation (state-transitions) 
and some auxiliary debugging functions.

1  Overview

FLOB is a shortcut for ~faked larged object~ which is a concept for implementing
datatypes such as regions which may vary strongly in size. The idea of FLOBs has
been studied in [1] (The current implementation differs in some aspects). The
basic idea is to store data of an attribute value depending on a threshold size
either inside the tuple representation or in a separate storage location.  

*/

#ifndef FLOB_H
#define FLOB_H

#include "SecondoSMI.h"
#include "QueryProcessor.h"

/*
A FLOB has one of the following states
   
*/

enum FLOB_Type { Destroyed, InMemory, InMemoryCached, 
                 InMemoryPagedCached, InDiskSmall, InDiskLarge };

/*
The possible state transitions are presented below (Any denotes the set of all possible states and S is a state variable):

-----
 ReInit: S in Any -> InDiskLarge

 GetPage: InDiskLarge -> InMemoryPagedCached 
          InMemoryPagedCached -> InMemoryPagedCached
 
 Get: InDiskLarge -> InMemory, InMemoryCached, InMemoryPagedCached
      InDiskSmall -> InMemory
      InMemory -> InMemory
      InMemoryCached -> InMemoryCached 
      InMemoryPagedCached -> InMemoryPagedCached
 
 Put: InMemory -> InMemory
      
 BringToMemory: InDiskLarge -> InMemory, InMemoryCached
                InDiskSmall -> InMemory
                InMemoryPagedCached -> InDiskLarge -> InMemory, InMemoryCached
                InMemory -> InMemory
                InMemoryCached -> InMemoryCached                

 SaveToExtensionTuple: InMemory -> InDiskSmall
 ReadFromExtensionTuple: InDiskSmall -> InMemory

 SaveToLob: InMemory, InMemoryCached, InDiskLarge -> InDiskLarge
 
 Resize: InMemory -> InMemory 
 
 Clean: S in Any \ {Destroyed} -> InMemory 

 Destroy: S in Any \ {Destroyed} -> Destroyed

 ReuseMemBuffer: S in {InDiskSmall} -> InMemory
----

   
3 Class ~FLOB~

This class implements a FLOB which defines a new large object 
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
FLOB exceeds the threshold size the data will be stored in a separate 
file for LOBs.

*/

    static const size_t PAGE_SIZE;
/*
The page size for paged access.

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
      fd.inMemory.freezed = false;
      
      if (debug)
	cerr << "FLOB " << (void*)this << ": Contructed. size = " << size
	     << " inMemory.buffer = " << (void*)fd.inMemory.buffer << endl;
    }

/*
3.3 Destructor

Deletes the FLOB instance.

*/
    virtual inline ~FLOB()
    {
      if (debug)
	cerr << "FLOB " << (void*)this 
	     << ": About to destruct. type = " << stateStr(type)
	     << " inMemory.buffer = " << (void*)fd.inMemory.buffer << endl;
      if( type == InMemoryCached )
        SecondoSystem::GetFLOBCache()->Release( fd.inMemoryCached.lobFileId,
                                                fd.inMemoryCached.lobId );
      else if( type == InMemoryPagedCached )
      {
        assert( fd.inMemoryPagedCached.buffer != 0 );
        if( fd.inMemoryPagedCached.cached )
          SecondoSystem::GetFLOBCache()->
            Release( fd.inMemoryPagedCached.lobFileId,
                     fd.inMemoryPagedCached.lobId,
                     fd.inMemoryPagedCached.pageno );
        else
          free( fd.inMemoryPagedCached.buffer );
      }
      else if( type == InMemory && 
               fd.inMemory.canDelete && 
	       !fd.inMemory.freezed &&
               fd.inMemory.buffer != 0 )
        free( fd.inMemory.buffer );
    }

/*
3.4 ReInit

Re-initializes the FLOB and changes its state to ~InDiskLarge~.

*/
    inline void ReInit()
    {
      assert( IsLob() );
      SmiFileId lobFileId;
      SmiRecordId lobId;

      if( type == InMemoryCached )
      {
        lobFileId = fd.inMemoryCached.lobFileId;
        lobId = fd.inMemoryCached.lobId;   
        SecondoSystem::GetFLOBCache()->Release( fd.inMemoryCached.lobFileId,
                                                fd.inMemoryCached.lobId ); 
      }
      else if( type == InMemoryPagedCached )
      {
        lobFileId = fd.inMemoryPagedCached.lobFileId;
        lobId = fd.inMemoryPagedCached.lobId;
        assert( fd.inMemoryPagedCached.buffer != 0 );
        if( fd.inMemoryPagedCached.cached )
          SecondoSystem::GetFLOBCache()->
            Release( fd.inMemoryPagedCached.lobFileId,
                     fd.inMemoryPagedCached.lobId,
                     fd.inMemoryPagedCached.pageno );
        else
          free( fd.inMemoryPagedCached.buffer );
      }

      type = InDiskLarge;
      fd.inDiskLarge.lobFileId = lobFileId;
      fd.inDiskLarge.lobId = lobId;
    }

/*
3.4 BringToMemory

Reads a disk LOB to memory, i.e., converts a FLOB in ~InDiskLarge~
state to a ~InMemory~ state.

*/
    virtual const char *BringToMemory() const;

/*
3.5 GetPage

*/
    inline void GetPage( size_t page ) const
    {
      assert( type == InDiskLarge ||
              type == InMemoryPagedCached );

      if( type == InDiskLarge )
      {
        SmiFileId lobFileId = fd.inDiskLarge.lobFileId;
        SmiRecordId lobId = fd.inDiskLarge.lobId;
        type = InMemoryPagedCached;
        fd.inMemoryPagedCached.lobFileId = lobFileId;
        fd.inMemoryPagedCached.lobId = lobId;
      }
      else if( type == InMemoryPagedCached )
      {
        if( fd.inMemoryPagedCached.pageno == page )
          return;

        if( fd.inMemoryPagedCached.buffer != 0 )
        {
          if( fd.inMemoryPagedCached.cached )
            SecondoSystem::GetFLOBCache()->
              Release( fd.inMemoryPagedCached.lobFileId,
                       fd.inMemoryPagedCached.lobId,
                       fd.inMemoryPagedCached.pageno );
          else
            free( fd.inMemoryPagedCached.buffer );
        }
      }  
      
      const char *buffer;
      bool cached =
        SecondoSystem::GetFLOBCache()->
          GetFLOB( fd.inMemoryPagedCached.lobFileId,
                   fd.inMemoryPagedCached.lobId,
                   page,
                   PAGE_SIZE, false, buffer );

      fd.inMemoryPagedCached.buffer = const_cast<char*>(buffer);
      fd.inMemoryPagedCached.pageno = page;
      fd.inMemoryPagedCached.cached = cached;
    }

/*
3.5 Get

Returns a pointer to a memory block of the FLOB data with
offset ~offset~.

*/
    inline void Get( size_t offset, const char **target, 
                     const bool paged = false ) const
    {
      assert( type != Destroyed );

      if( type == InMemory )
        *target = fd.inMemory.buffer + offset;
      else if( type == InMemoryCached )
      {
        assert( paged == false );
        *target = fd.inMemoryCached.buffer + offset;
      }
      else if( type == InMemoryPagedCached )
      {
        assert( paged == true );
        if( offset >= fd.inMemoryPagedCached.pageno * PAGE_SIZE &&
            offset < fd.inMemoryPagedCached.pageno * PAGE_SIZE + PAGE_SIZE ) 
        {
          *target = fd.inMemoryPagedCached.buffer + (offset % PAGE_SIZE);
        }
        else
        {
          GetPage( offset / PAGE_SIZE );
          Get( offset, target, true );
        }
      }
      else if( type == InDiskLarge )
      {
        if( paged )
          GetPage( offset / PAGE_SIZE );
        else
          BringToMemory();

        Get( offset, target, paged );
      }
      else if( type == InDiskSmall )
      {
        BringToMemory();
        Get( offset, target, false );
      }
    }

/*
3.6 Put

Write data from ~source~ into the FLOBs memory.

*/
    inline void Put( size_t offset, size_t length, const void *source )
    {
      assert( type == InMemory );
      assert( fd.inMemory.freezed == false );
      memcpy( fd.inMemory.buffer + offset, source, length );
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
3.8 Clean

Cleans the FLOB, removing it from memory. If it is cached, then a
reference in the cache is removed. Afterwards the state will be ~InMemory~.

*/
    inline void Clean()
    {
      if (debug)
	cerr << "FLOB " << (void*)this 
	     << ": About to clean. type = " << stateStr(type)
	     << " inMemory.buffer = " << (void*)fd.inMemory.buffer << endl;
      assert( type != Destroyed );
      if( type == InMemory && fd.inMemory.canDelete && size > 0 )
        free( fd.inMemory.buffer );
      else if( type == InMemoryCached )
        SecondoSystem::GetFLOBCache()->Release( fd.inMemoryCached.lobFileId,
                                                fd.inMemoryCached.lobId );
      else if( type == InMemoryPagedCached )
      {
        assert( fd.inMemoryPagedCached.buffer != 0 );
        if( fd.inMemoryPagedCached.cached )
          SecondoSystem::GetFLOBCache()->
            Release( fd.inMemoryPagedCached.lobFileId,
                     fd.inMemoryPagedCached.lobId,
                     fd.inMemoryPagedCached.pageno );
        else
          free( fd.inMemoryPagedCached.buffer );
      }

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
3.9 Restrict

Restricts the FLOB to the interval set passed as argument.

*/
    virtual void Restrict( const vector< pair<int, int> >& intervals )
    {}

/*
3.10 SaveToLob

Saves the FLOB to the LOB file. The FLOB must be a LOB. The type is 
set to ~InDiskLarge~.

*/
    virtual void SaveToLob( SmiRecordId& lobFileId, 
                            SmiRecordId lobId = 0 ) const;

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
      assert( checkState( InMemory ) );
      if( type == InMemory && size > 0 )
      {
        if( extensionTuple != 0 )
        {
          memcpy( extensionTuple, fd.inMemory.buffer, size );
          fd.inDiskSmall.buffer = (char*)extensionTuple;
        }
        else
        {
          assert( fd.inMemory.canDelete == false );
          char *buffer = fd.inMemory.buffer;
          fd.inDiskSmall.buffer = buffer;
        }
      }
      else {
        fd.inDiskSmall.buffer = 0;
      }
      //Avoid that the inMemory buffer will be deleted.
      fd.inMemory.freezed = true;

      changeState( type, InDiskSmall );
    }

/*
Flobs of type InDiskSmall will be copied into the extension tuple before the tuple is
written to disk. 
The function below will be called by Attribute::DeleteIfAllowed. It puts flobs which are part
of an attribute instance which could not be deleted since they are still used by other tuples back
from state ~InDiskSmall~ to ~InMemory~. For other states the function will have no effect.

*/
    
   void ReuseMemBuffer();
    
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
      if( type == InDiskLarge || 
          type == InMemoryCached || 
          type == InMemoryPagedCached )
        return true;
      else if( type == InMemory && 
               size > SWITCH_THRESHOLD )
        return true;
      return false;
    }


/*
3.11 GetType

return the FLOB's internal state.

*/
    inline FLOB_Type GetType() const
    {
      return type;
    }

/*
Declariont of a flag for debugging

*/
    static bool debug; 
    
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
	bool freezed;
        bool canDelete;
      } inMemory;

      struct InMemoryCached
      {
        const char *buffer;
        SmiFileId lobFileId;
        SmiRecordId lobId;
      } inMemoryCached;

      struct InMemoryPagedCached
      {
        char *buffer;
        bool cached;
        size_t pageno;
        SmiFileId lobFileId;
        SmiRecordId lobId;
      } inMemoryPagedCached;

      struct InDiskSmall
      { 
        char *buffer;
      } inDiskSmall;

      struct InDiskLarge
      {
        SmiFileId lobFileId;
        SmiRecordId lobId;
      } inDiskLarge;

    } fd;

  
    private:

/*
Auxiliary functions for checking and changing states
   
*/    
    bool checkState(const FLOB_Type f) const;
	    
    bool checkState(const FLOB_Type f[], const int n) const;

    void changeState(FLOB_Type& from, const FLOB_Type to) const;
    
    const string stateStr(const FLOB_Type& f) const;
    
};


#endif

/*
7 References

 [1] S. Dieker, R.H. G[ue]ting, and M. Rodriguez-Luaces, A Tool for Nesting and Clustering Large Objects. Proc. of the 12th Int. Conf. on Scientific and Statistical Database Management (SSDBM 2000), 169-181, July 2000.

*/

