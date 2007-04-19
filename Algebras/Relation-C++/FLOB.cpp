/*
----
This file is part of SECONDO.

Copyright (C) 2004-2007, University in Hagen, Faculto of Mathematics and
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[ae] [\"a]
//[ue] [\"u]
//[oe] [\"o]

[1] Implementation File of Module FLOB


1 Includes

*/
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <cassert>

#include "FLOB.h"
#include "FLOBCache.h"
/*

2 Implementation of class FLOB.

2.1 Definition of static members.

*/
const size_t FLOB::SWITCH_THRESHOLD = 1024;
const size_t FLOB::PAGE_SIZE = 4050;

bool FLOB::debug = false;

FLOB::FLOB( size_t sz ) :
type( InMemory )
{
  size = sz;
  Malloc();

  if (debug)
    cerr << "FLOB " << (void*)this << ": Contructed. size = " << size
	 << " inMemory.buffer = " << (void*)fd.inMemory.buffer << endl;
}

/*
3.3 Destructor

Deletes the FLOB instance.

Cleans the memory representation of a FLOB. If it is cached, then a
reference in the cache is removed. If the type is ~Destroyed~ or 
~InDiskLarge~ nothing happens.

*/

FLOB::~FLOB()
{
  if (debug) {
    cerr << "FLOB " << (void*)this 
	 << ": About to destruct. type = " << stateStr(type)
	 << " inMemory.buffer = " << (void*)fd.inMemory.buffer << endl;
  }

  if( type == InMemoryCached ) 
  {
    SecondoSystem::GetFLOBCache()->Release( fd.inMemoryCached.lobFileId,
					    fd.inMemoryCached.lobId );
  }  
  else 
    if( type == InMemoryPagedCached )
    {
      assert( fd.inMemoryPagedCached.buffer != 0 );
      if( fd.inMemoryPagedCached.cached ) 
      { 
	SecondoSystem::GetFLOBCache()->
	  Release( fd.inMemoryPagedCached.lobFileId,
		   fd.inMemoryPagedCached.lobId,
		   fd.inMemoryPagedCached.pageno );
      }  
      else 
      {
	free( fd.inMemoryPagedCached.buffer );
      }  
    }
  else 
     if ( type == InMemory && 
	  fd.inMemory.buffer != 0 ) 
     {
      free( fd.inMemory.buffer );
     }  
}

void FLOB::Clean()
{
  if (debug)
    cerr << "FLOB " << (void*)this
	 << ": About to clean. type = " << stateStr(type)
	 << " inMemory.buffer = " << (void*)fd.inMemory.buffer << endl;
  assert( type != Destroyed );
  if( type == InMemory && size > 0 )
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
  size = 0;
}

/*
3.5 GetPage

*/

void 
FLOB::GetPage( size_t page ) const
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
  
  const char *buffer = 0;
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

void 
FLOB::Get( size_t offset, 
           const char **target, 
	   const bool paged /*= false*/ ) const
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
}

void*
FLOB::Malloc(size_t newSize /*= 0*/) {

  assert( checkState(InMemory) );
  
  const bool resize = (newSize > 0);

  if (!resize) {
    if (size == 0) 
    {
      // initialize the buffer pointer	  
      fd.inMemory.buffer = 0;	  
    }
    else
    {	  
      // allocate a buffer	    
      fd.inMemory.buffer = (char*)malloc( size );
    }
  }
  else // resize
  {   
    if ( newSize != size )  
    {
      // resize the internal buffer	    
      fd.inMemory.buffer = 
        (char *)realloc( fd.inMemory.buffer, newSize );
      size = newSize;
    }  
  }

  return fd.inMemory.buffer; 
}	


/*
 
Function ~ReadFrom~ should only be called in order to initialize the FLOB
directly after it was read from disk, therefore it must be in state
~InMemory~. It copies the data pointed to
by ~src~into its own buffer.

*/

unsigned int
FLOB::ReadFrom(void* src) {

  assert( checkState( InMemory ) );

  Malloc();
  memcpy(fd.inMemory.buffer, src, size);  

  return size;
}

/*
Function ~WriteTo~ will write the data of an ~InMemory~ FLOB to the
given destination pointer.

*/   

unsigned int
FLOB::WriteTo(void* dest) const 
{
  assert( checkState(InMemory) );
  if( size > 0 ) 
  {
    memcpy( dest, fd.inMemory.buffer, size);
  }
  else
  {
    assert( fd.inMemory.buffer == 0);
  }  

  return size;
}


/*
3.10 IsLob

Returns true, if value stored in underlying LOB, otherwise false.

*/

bool 
FLOB::IsLob() const
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
2.4 BringToMemory

*/

const char*
FLOB::BringToMemory() const
{
  assert( type != Destroyed );
  if (debug)
    cerr << "FLOB " << (void*)this 
	 << ": BringToMemory(). type = " << stateStr(type)
	 << " inMemory.buffer = " << (void*)fd.inMemory.buffer << endl;	

  if( type == InDiskLarge )
  {
    const char* buffer = 0;
    bool cached =  
      SecondoSystem::GetFLOBCache()->GetFLOB( fd.inDiskLarge.lobFileId, 
                                              fd.inDiskLarge.lobId, 
                                             -1, size, false, buffer );

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
      fd.inMemory.buffer = const_cast<char*>( buffer );
    }
    return buffer;
  }
  else if( type == InMemory )
    return fd.inMemory.buffer;
  else if( type == InMemoryCached )
    return fd.inMemoryCached.buffer;
  else if( type == InMemoryPagedCached )
  {
    assert( fd.inMemoryPagedCached.buffer != 0 );
    if( fd.inMemoryPagedCached.cached )
      SecondoSystem::GetFLOBCache()->Release( fd.inMemoryPagedCached.lobFileId,
                                              fd.inMemoryPagedCached.lobId,
                                              fd.inMemoryPagedCached.pageno );
    else
      free( fd.inMemoryPagedCached.buffer );

    SmiFileId lobFileId = fd.inMemoryPagedCached.lobFileId;
    SmiRecordId lobId = fd.inMemoryPagedCached.lobId;
    type = InDiskLarge;
    fd.inDiskLarge.lobFileId = lobFileId;
    fd.inDiskLarge.lobId = lobId;
    return BringToMemory();
  }
  
  return 0;
}

/*
2.10 Destroy

*/

void 
FLOB::Destroy()
{
  assert( type != Destroyed );

  if( type == InMemory )
  {
    if( size > 0 && fd.inMemory.buffer != 0 )
    {
      free( fd.inMemory.buffer );
      fd.inMemory.buffer = 0;
    }
    else {
      assert( fd.inMemory.buffer == 0 );
    }      
  }
  else if( type == InMemoryCached )
  {
    SecondoSystem::GetFLOBCache()->Destroy( fd.inMemoryCached.lobFileId, 
                                            fd.inMemoryCached.lobId );
  }
  else if( type == InDiskLarge )
  {
    SecondoSystem::GetFLOBCache()->Destroy( fd.inDiskLarge.lobFileId, 
                                            fd.inDiskLarge.lobId );
  }
  else if( type == InMemoryPagedCached )
  {
    assert( fd.inMemoryPagedCached.buffer != 0 );
    if( fd.inMemoryPagedCached.cached )
      SecondoSystem::GetFLOBCache()->Release( fd.inMemoryPagedCached.lobFileId,
                                              fd.inMemoryPagedCached.lobId,
                                              fd.inMemoryPagedCached.pageno );
    else
      free( fd.inMemoryPagedCached.buffer );

    SecondoSystem::GetFLOBCache()->Destroy( fd.inMemoryPagedCached.lobFileId, 
                                            fd.inMemoryPagedCached.lobId );
  }

  size = 0;
  type = Destroyed;
}

/*
2.10 Resize

*/

void 
FLOB::Resize( size_t newSize )
{
  assert( type != Destroyed );
  assert( newSize > 0 ); // Use Clean
 
  if( type == InMemory )
  {
    Malloc(newSize);	  
  }
  else {
    // This code cannot be reached because the other
    // cases are not implemented yet.
    assert( false );
  }  

  if (debug)
    cerr << "FLOB " << (void*)this << ": resized. size = " << size 
	 << " inMemory.buffer = " << (void*)fd.inMemory.buffer << endl;	
}

/*
2.10 SaveToLob

*/

void 
FLOB::SaveToLob( SmiFileId& lobFileId, SmiRecordId lobId ) const
{
  assert( (type == InMemory || type == InMemoryCached ) && 
            size > SWITCH_THRESHOLD ||
          type == InDiskLarge );

  if( type == InDiskLarge )
  {
    BringToMemory();
    SaveToLob( lobFileId, lobId );
  }
  else if( type == InMemory ) 
  {
    SecondoSystem::GetFLOBCache()->PutFLOB( lobFileId, lobId, 
                                            -1, size, false, 
                                            fd.inMemory.buffer );
    free( fd.inMemory.buffer );
    fd.inMemory.buffer = 0;

    type = InDiskLarge;
    fd.inDiskLarge.lobFileId = lobFileId;
    fd.inDiskLarge.lobId = lobId;
  }
  else if( type == InMemoryCached )
  {
    SecondoSystem::GetFLOBCache()->PutFLOB( lobFileId, lobId, 
                                            -1, size, false, 
                                            fd.inMemoryCached.buffer );
    SecondoSystem::GetFLOBCache()->Release( fd.inMemoryCached.lobFileId, 
                                            fd.inMemoryCached.lobId ); 
    fd.inMemoryCached.buffer = 0;
    type = InDiskLarge;
    fd.inDiskLarge.lobFileId = lobFileId;
    fd.inDiskLarge.lobId = lobId;
  }
}

bool 
FLOB::checkState(const FLOB_Type f) const
{
  FLOB_Type states[] = { f };
  return checkState(states, 1);
}	    

bool 
FLOB::checkState(const FLOB_Type f[], const int n) const
{
  int m = n;	    
  while (m > 0 && (type != f[m-1])) {;      	      
    m--;      
  }       
  if (m == 0)
  { 
    string states = "";
    for (int i=0; i < n; i++)
    {
      states += stateStr(f[i]) + " ";	
    }
    cerr << "Flob " << (const void*)this << ": "
	 << "Assuming one of the states {" << states << "}" 
	 << " but flob has state " << stateStr(type) << endl;
    return false;
  }
  return true;  
}  

const 
string FLOB::stateStr(const FLOB_Type& f) const
{ 
  switch (f)
  { 
   case Destroyed: return "Destroyed"; break;  
   case InMemory: return "InMemory"; break;  
   case InMemoryCached: return "InMemoryCached"; break;  
   case InMemoryPagedCached: return "InMemoryPageCached"; break;  
   case InDiskLarge: return "InDiskLarge"; break;  
   default: return "Unknown state!";
  }    
}

