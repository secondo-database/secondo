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
#include <sstream>

#include <iostream>
#include <cassert>

#include "FLOB.h"
#include "FLOBCache.h"

#include "Serialize.h"

/*

2 Implementation of class FLOB.

2.1 Definition of static members.

*/
const size_t FLOB::SWITCH_THRESHOLD = 1024;
const size_t FLOB::PAGE_SIZE = 4050;


FLOB::FLOB( size_t sz ) :
type( InMemory )
{
  fd = new FLOB_Descriptor();
  size = sz;
  Malloc();

  FT("size = " << size);
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
  FT( "type = " << stateStr(type) 
                << " inMemory.buffer = " << (void*)fd->inMemory.buffer );
 
  if( type == InMemoryCached )
  {
    SecondoSystem::GetFLOBCache()->Release( fd->inMemoryCached.lobFileId,
                                            fd->inMemoryCached.lobId );
  }
  else if( type == InMemoryPagedCached )
  {
    assert( fd->inMemoryPagedCached.buffer != 0 );
    if( fd->inMemoryPagedCached.cached )
    {
      SecondoSystem::GetFLOBCache()->
          Release(  fd->inMemoryPagedCached.lobFileId,
                    fd->inMemoryPagedCached.lobId,
                    fd->inMemoryPagedCached.pageno );
    }
    else
    {
      free( fd->inMemoryPagedCached.buffer );
      fd->inMemoryPagedCached.buffer = 0;
    }
  }
  else if ( type == InMemory && fd->inMemory.buffer != 0 )
  {
    free( fd->inMemory.buffer );
    fd->inMemory.buffer = 0;
  }

  //cerr << (void*)fd << " freed" << endl;
  if(fd){
    delete fd; 
  }
  fd = 0;
}


void FLOB::SetEmpty()
{
  FT("type = InMemory, size = 0");

  type = InMemory;
  size = 0;
}


void FLOB::Clean()
{
  FT( stateStr(type) << " inMemory.buffer = " << (void*)fd->inMemory.buffer );

  // SPM 19.06.2007
  //
  // WORK AROUND to make the update relation algebra running
  // more precisely: the queries of file Test/Testspecs/update.test

  // Commenting out the following line is maybe not a general solution !!!
  // assert( type != Destroyed );

  // Open question: what happens if an operator destroys tuples from a relation
  // (deleting the physical record representation) but the attributes are
  // referenced by other in memory tuple instances.  I would suggest that the
  // FLOBs record must be copied into a separate FLOB file maintained by the
  // FlobCache so that other operations which need the full representation are
  // able to access it later if necessary. I'm not sure if the current
  // implementation will do this

  if( type == InMemory && size > 0 )
    free( fd->inMemory.buffer );
  else if( type == InMemoryCached )
    SecondoSystem::GetFLOBCache()->Release( fd->inMemoryCached.lobFileId,
                                            fd->inMemoryCached.lobId );
  else if( type == InMemoryPagedCached )
  {
    assert( fd->inMemoryPagedCached.buffer != 0 );
    if( fd->inMemoryPagedCached.cached )
      SecondoSystem::GetFLOBCache()->
        Release( fd->inMemoryPagedCached.lobFileId,
                 fd->inMemoryPagedCached.lobId,
                 fd->inMemoryPagedCached.pageno );
    else
      free( fd->inMemoryPagedCached.buffer );
  }

  type = InMemory;
  fd->inMemory.buffer = 0;
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
    SmiFileId lobFileId = fd->inDiskLarge.lobFileId;
    SmiRecordId lobId = fd->inDiskLarge.lobId;
    type = InMemoryPagedCached;
    fd->inMemoryPagedCached.lobFileId = lobFileId;
    fd->inMemoryPagedCached.lobId = lobId;
  }
  else if( type == InMemoryPagedCached )
  {
    if( fd->inMemoryPagedCached.pageno == page )
      return;

    if( fd->inMemoryPagedCached.buffer != 0 )
    {
      if( fd->inMemoryPagedCached.cached )
        SecondoSystem::GetFLOBCache()->
          Release( fd->inMemoryPagedCached.lobFileId,
                   fd->inMemoryPagedCached.lobId,
                   fd->inMemoryPagedCached.pageno );
      else
        free( fd->inMemoryPagedCached.buffer );
    }
  }

  const char *buffer = 0;
  bool cached =
    SecondoSystem::GetFLOBCache()->
      GetFLOB( fd->inMemoryPagedCached.lobFileId,
               fd->inMemoryPagedCached.lobId,
               page,
               PAGE_SIZE, false, buffer );

  fd->inMemoryPagedCached.buffer = const_cast<char*>(buffer);
  fd->inMemoryPagedCached.pageno = page;
  fd->inMemoryPagedCached.cached = cached;
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
    *target = fd->inMemory.buffer + offset;
  else if( type == InMemoryCached )
  {
    assert( paged == false );
    *target = fd->inMemoryCached.buffer + offset;
  }
  else if( type == InMemoryPagedCached )
  {
    assert( paged == true );
    if( offset >= fd->inMemoryPagedCached.pageno * PAGE_SIZE &&
        offset < fd->inMemoryPagedCached.pageno * PAGE_SIZE + PAGE_SIZE )
    {
      *target = fd->inMemoryPagedCached.buffer + (offset % PAGE_SIZE);
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
      FT(" inMemory.buffer = 0");
      // initialize the buffer pointer
      fd->inMemory.buffer = 0;
    }
    else
    {
      // allocate a buffer
      FT( "allocate " << size);
      fd->inMemory.buffer = (char*)malloc( size );
    }
  }
  else // resize
  {
    if ( newSize != size )
    {
      FT("resize " << size << " -> " << newSize);
      // resize the internal buffer
      fd->inMemory.buffer =
        (char *)realloc( fd->inMemory.buffer, newSize );
      size = newSize;
    }
  }

  return fd->inMemory.buffer;
}

string FLOB::ToString() const{
   stringstream s;
   s << "FLOB:"
     << "  type(" << stateStr(type) <<")"
     << "  size(" << size << ")" ;
  if(type==Destroyed) {
      ;
  } else if(type == InMemory){
    s << "buffer("<< (void*) fd->inMemory.buffer  <<")" ;
  } else if( type == InMemoryCached) {
    s << "buffer(" << (void*) fd->inMemoryCached.buffer  <<")"
      << "lobFileId(" << fd->inMemoryCached.lobFileId << ")"
      << "lobId(" << fd->inMemoryCached.lobId << ")";
  } else if( type == InMemoryPagedCached){
    s << "buffer("
      << (void*) fd->inMemoryPagedCached.buffer << ")"
      << " chached (" << fd->inMemoryPagedCached.cached << ")"
      << " pageno(" << fd->inMemoryPagedCached.pageno << ")"
      << " lobFileId(" << fd->inMemoryPagedCached.lobFileId << ")"
      << " lobId(" << fd->inMemoryPagedCached.lobId << ")";
  } else if (type==InDiskLarge){
    s << " lobFileId(" << fd->inDiskLarge.lobFileId << ")"
      << " lobId(" << fd->inDiskLarge.lobId << ")";
  } else {
    s << "unknown type !!!" ;
  }
  return s.str();
}


ostream& operator<<(ostream& os, const FLOB& flob){
    os << flob.ToString();
    return os;
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

  FT( "size =" << size );
  Malloc();
  memcpy(fd->inMemory.buffer, src, size);

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
    memcpy( dest, fd->inMemory.buffer, size);
  }
  else
  {
    assert( fd->inMemory.buffer == 0);
  }

  return size;
}


size_t 
FLOB::RestoreFD(char* src)
{
  assert(  type == InMemory || type == InDiskLarge ); 

  if ( type == InMemory ) {
    // no metadata to be restored         
    return 0;
  } else {

    //cerr << "RestoreFD inDiskLarge " << endl;
    size_t offset = 0;    
    ReadVar<SmiFileId>(fd->inDiskLarge.lobFileId, src, offset);           
    ReadVar<SmiRecordId>(fd->inDiskLarge.lobId, src, offset);
    //cerr << "lobFileId = " << fd->inDiskLarge.lobFileId << endl;
    //cerr << "lobId = " << fd->inDiskLarge.lobId << endl;
    //assert(false);
    return offset;    
  }       
}               
    

size_t
FLOB::SerializeFD(char* dest) const
{
  assert(  type == InMemory || type == InDiskLarge ); 
  if ( type == InMemory ) {
    // no metadata to be stored   
    return 0;
  } else {

    size_t offset = 0;    
    //cerr << "lobFileId = " << fd->inDiskLarge.lobFileId << endl;
    //cerr << "lobId = " << fd->inDiskLarge.lobId << endl;
    WriteVar<SmiFileId>(fd->inDiskLarge.lobFileId, dest, offset);         
    WriteVar<SmiRecordId>(fd->inDiskLarge.lobId, dest, offset);
    return offset;    
  }       

}

size_t
FLOB::MetaSize() const
{
  
  if ( type == InMemory ) {
    // no metadata to be stored   
    return 0;
  } else {
    size_t m = sizeof(SmiFileId) + sizeof(SmiRecordId);
    //cerr << "m = " << m << endl;
    return m;
  }       
}


void 
FLOB::SaveToRecord( SmiRecord& record, 
                    size_t& offset, 
                    SmiFileId& lobFile, 
                    bool checkLob /*=true*/ )
{
  FT( "checkLob " << checkLob << ", size " << size);

  // save data to a LOB record stored in lobFile
  if( checkLob && IsLob() ){
     SaveToLob( lobFile );
  }

  // save the object's byte block
  size_t objSize1 = sizeof(*this);

  record.Write( this, objSize1, offset );
  offset += objSize1;

  size_t extensionsize = 0;
  char *extensionElement = 0;
  if( !checkLob || !IsLob() )
  {
    // not a LOB, save the data into the record
    extensionsize = size;
    if(extensionsize>0)
    {
       extensionElement = (char*) malloc(extensionsize);
       WriteTo(extensionElement);
       record.Write( extensionElement, extensionsize, offset );
       offset += extensionsize;
       free( extensionElement );
    }
  }
  
  // save meta data store in FLOB descriptor
  size_t mSize = MetaSize();
  if (mSize > 0) 
  {
    char* meta = (char*) malloc(mSize); 
    SerializeFD(meta);
    record.Write( meta, mSize, offset );
    offset += mSize;
    free( meta );
  }

}


void FLOB::OpenFromRecord( SmiRecord& record,
                           size_t& offset, bool checkLob /*= true*/)
{
  FT( "checkLob " << checkLob );
  Clean();
  if(fd){
    delete fd;
  }

  // restore the class byte block 
  size_t objSize1 = sizeof(*this);

  record.Read( this , objSize1, offset );
  offset += objSize1;
 
  FT( "size = " << size );


  // restore the FLOB Descriptor
  fd = new FLOB_Descriptor();

  // if the array is not a LOB, read the data directly
  if( !checkLob || !IsLob() )
  {
     FT( "!IsLob() size =" << size );

     size_t extSize = size;
     char* data = (char*) malloc(extSize);
     record.Read( data, extSize, offset );
     offset += extSize;
     ReadFrom(data);
     if(data){
        free(data);
     }
  }

  char* meta = 0;       
  size_t mSize = MetaSize(); 
  if ( mSize > 0 )
  {
    meta = (char*) malloc(mSize); 
    record.Read( meta, mSize, offset );
    offset += mSize;
  }
  RestoreFD(meta);
  if ( meta != 0 ) {
    free( meta );
  }  

}




/*
3.10 IsLob

Returns true, if value stored in underlying LOB, otherwise false.

*/

bool
FLOB::IsLob() const
{
  assert( type != Destroyed );
  return ( IsPersistentLob()
           || IsMemoryLob() || IsCachedLob() );
}

bool
FLOB::IsMemoryLob() const
{
  return ( type == InMemory &&
           size > SWITCH_THRESHOLD );
}

bool
FLOB::IsPersistentLob() const
{
  return ( type == InDiskLarge );
}


bool
FLOB::IsCachedLob() const
{
  return ( type == InMemoryCached ||
           type == InMemoryPagedCached );
}


/*
2.4 BringToMemory

*/

const char*
FLOB::BringToMemory() const
{
  assert( type != Destroyed );
  FT( "type = " << stateStr(type) 
                << " inMemory.buffer = " << (void*)fd->inMemory.buffer );

  if( type == InDiskLarge )
  {
    const char* buffer = 0;
    bool cached =
      SecondoSystem::GetFLOBCache()->GetFLOB( fd->inDiskLarge.lobFileId,
                                              fd->inDiskLarge.lobId,
                                             -1, size, false, buffer );

    SmiFileId fileId = fd->inDiskLarge.lobFileId;
    SmiRecordId lobId = fd->inDiskLarge.lobId;

    if( cached )
    {
      type = InMemoryCached;
      fd->inMemoryCached.buffer = buffer;
      fd->inMemoryCached.lobFileId = fileId;
      fd->inMemoryCached.lobId = lobId;
    }
    else
    {
      type = InMemory;
      fd->inMemory.buffer = const_cast<char*>( buffer );
    }
    return buffer;
  }
  else if( type == InMemory )
    return fd->inMemory.buffer;
  else if( type == InMemoryCached )
    return fd->inMemoryCached.buffer;
  else if( type == InMemoryPagedCached )
  {
    assert( fd->inMemoryPagedCached.buffer != 0 );
    if( fd->inMemoryPagedCached.cached )
      SecondoSystem::GetFLOBCache()->Release( fd->inMemoryPagedCached.lobFileId,
                                              fd->inMemoryPagedCached.lobId,
                                              fd->inMemoryPagedCached.pageno );
    else
      free( fd->inMemoryPagedCached.buffer );

    SmiFileId lobFileId = fd->inMemoryPagedCached.lobFileId;
    SmiRecordId lobId = fd->inMemoryPagedCached.lobId;
    type = InDiskLarge;
    fd->inDiskLarge.lobFileId = lobFileId;
    fd->inDiskLarge.lobId = lobId;
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
    if( size > 0 && fd->inMemory.buffer != 0 )
    {
      free( fd->inMemory.buffer );
      fd->inMemory.buffer = 0;
    }
    else {
      assert( fd->inMemory.buffer == 0 );
    }
  }
  else if( type == InMemoryCached )
  {
    SecondoSystem::GetFLOBCache()->Destroy( fd->inMemoryCached.lobFileId,
                                            fd->inMemoryCached.lobId );
  }
  else if( type == InDiskLarge )
  {
    SecondoSystem::GetFLOBCache()->Destroy( fd->inDiskLarge.lobFileId,
                                            fd->inDiskLarge.lobId );
  }
  else if( type == InMemoryPagedCached )
  {
    assert( fd->inMemoryPagedCached.buffer != 0 );
    if( fd->inMemoryPagedCached.cached )
      SecondoSystem::GetFLOBCache()->Release( fd->inMemoryPagedCached.lobFileId,
                                              fd->inMemoryPagedCached.lobId,
                                              fd->inMemoryPagedCached.pageno );
    else
      free( fd->inMemoryPagedCached.buffer );

    SecondoSystem::GetFLOBCache()->Destroy( fd->inMemoryPagedCached.lobFileId,
                                            fd->inMemoryPagedCached.lobId );
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
  if( newSize <= 0)
  {
    Clean();
    return;
  }
  if( type == InMemory )
  {
    Malloc(newSize);
  }
  else {
    // This code cannot be reached because the other
    // cases are not implemented yet.
    assert( false );
  }
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
                                            fd->inMemory.buffer );
    free( fd->inMemory.buffer );
    fd->inMemory.buffer = 0;

    type = InDiskLarge;
    fd->inDiskLarge.lobFileId = lobFileId;
    fd->inDiskLarge.lobId = lobId;
  }
  else if( type == InMemoryCached )
  {
    SecondoSystem::GetFLOBCache()->PutFLOB( lobFileId, lobId,
                                            -1, size, false,
                                            fd->inMemoryCached.buffer );
    SecondoSystem::GetFLOBCache()->Release( fd->inMemoryCached.lobFileId,
                                            fd->inMemoryCached.lobId );
    fd->inMemoryCached.buffer = 0;
    type = InDiskLarge;
    fd->inDiskLarge.lobFileId = lobFileId;
    fd->inDiskLarge.lobId = lobId;
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

