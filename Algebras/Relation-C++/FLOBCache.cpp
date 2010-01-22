/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics & Computer Science,
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

[1] Implementation File of Module FLOB Cache

Victor Almeida, 08/12/2005.

1 Includes

*/

#include <cassert>
#include <exception>
#include <sstream>


#include "Counter.h"
#include "FLOB.h"
#include "FLOBCache.h"
#include "Trace.h"


using namespace std;

// #define __NEW_FLOB__



ostream& operator<<(ostream& os, const FLOBKey& fk) { return fk.out(os); }


/*
2 Implementation of class LRUTable

*/
LRUTable::~LRUTable()
{
  Clear();
}

void LRUTable::Insert( FLOBCacheElement *e )
{
  if( first == NULL )
  {
    assert( last == NULL );
    first = last = e;
    return;
  }
  else
  {
    e->next = first;
    first->prev = e;
    first = e;
  }
}

void LRUTable::Promote( FLOBCacheElement *e, bool inc )
{
  Remove( e );
  e->next = e->prev = NULL;
  if( inc )
    e->refs++;
  Insert( e );
}

void LRUTable::Remove( FLOBCacheElement *e )
{
  FLOBCacheElement *next = e->next,
                   *prev = e->prev;

  if( prev != NULL )
    prev->next = next;
  else
    first = next;

  if( next != NULL )
    next->prev = prev;
  else
    last = prev;
}

bool LRUTable::RemoveLast( FLOBKey& key )
{
  FLOBCacheElement *e = last;

  while( e != NULL && e->refs > 0 )
    e = e->prev;

  if( e == NULL )
    return false;

  key = e->key;
  Remove( e );

  return true;
}

void LRUTable::Clear()
{
  FLOBCacheElement *e = first;
  while( e != NULL )
  {
    FLOBCacheElement *aux = e;
    e = e->next;
    if( aux->refs != 0 ) {
      stringstream err;
      err  <<   "Key ("
           << aux->key.fileId << ", "
           << aux->key.recordId << ", "
           << aux->key.pageno << ") still has "
           << aux->refs << " references while cleaning the cache.";
      throw FLOBCache_error(err.str());	    
    }  
    free( aux->flob );
    delete aux;
  }
  first = last = NULL;
}

/*
2 Implementation of class FLOBCache

*/
FLOBCache::~FLOBCache()
{
  trace.enter(__FUNCTION__);
  Clean();
}

char *FLOBCache::Lookup( const FLOBKey key, bool inc )
{
  trace.enter(__FUNCTION__);

  FLOBTable::iterator iter = mapTable.find( key );

  if( iter == mapTable.end() )
  {
    trace.add() << "Entry (" << key << ") not found";
    trace.flush(5);
    return NULL;
  }

  FLOBCacheElement *e = iter->second;
  lruTable.Promote( e, inc );

   trace.add() << "Entry (" << key << ") found, now with " 
	       << e->refs << " references";
   trace.flush(5);

   return e->flob;
}

bool FLOBCache::Insert( const FLOBKey key, char *flob )
{
  trace.enter(__FUNCTION__);
  trace.show(VAL(key));

  assert( mapTable.find( key ) == mapTable.end() );

  FLOBKey key2;
  while( sizeLeft < key.size && mapTable.size() > 0 &&
         RemoveLast( key2 ) )
    sizeLeft += key2.size;

  if( sizeLeft >= key.size )
  {
    FLOBCacheElement *e = new FLOBCacheElement( key, flob );
    sizeLeft -= key.size;
    mapTable[key] = e;
    lruTable.Insert( e );

    trace.add() << "Entry inserted into the cache with "
                << e->refs << " references";
    trace.flush(5);

    return true;
  }

  Counter::getRef("RA:FLOBCacheFull")++;

  trace.out("Cache full of entries with at least 1 reference", 5);
  
  return false;
}

void FLOBCache::Remove( const FLOBKey key )
{
  trace.enter(__FUNCTION__);
  FLOBTable::iterator iter = mapTable.find( key );

  if( iter != mapTable.end() )
  {
    FLOBCacheElement *e = iter->second;
    mapTable.erase( iter );
    lruTable.Remove( e );
    free( e->flob );
    delete e;
  }
}

bool FLOBCache::RemoveLast( FLOBKey& key )
{
  trace.enter(__FUNCTION__);
  if( lruTable.RemoveLast( key ) )
  {
    FLOBTable::iterator iter = mapTable.find( key );

    free( iter->second->flob );
    delete iter->second;
    mapTable.erase( iter );
    return true;
  }
  return false;
}

bool FLOBCache::GetFLOB( SmiFileId fileId, SmiRecordId lobId,
                         long pageno, size_t size, bool isTempFile,
                         const char *&flob )
{
  trace.enter(__FUNCTION__);
  assert( fileId != 0 && lobId != 0 );

  FLOBKey key( fileId, lobId, pageno, size, isTempFile );
  if( (flob = Lookup( key, true )) == NULL )
    // We need to read
  {
    char *buf = (char*) malloc( key.size );
    flob = buf;

    SmiRecordFile *file = LookUpFile(key.fileId, isTempFile);

    SmiRecord record;
    file->SelectRecord( key.recordId, record );

    if( pageno == -1 )
    {
      unsigned int RecordRead = record.Read( buf, key.size, 0 );
      assert( RecordRead == (unsigned int)key.size );
    }
    else
    {
      unsigned int RecordRead = record.Read( buf,
                                     FLOB::PAGE_SIZE,
                                     pageno * FLOB::PAGE_SIZE );
      assert( RecordRead == FLOB::PAGE_SIZE );
    }

    if( !Insert( key, buf ) )
      return false;

    Counter::getRef("RA:FLOBCacheMisses")++;
  }
  else
    Counter::getRef("RA:FLOBCacheHits")++;

  return true;
}


SmiRecordFile*
FLOBCache::CreateFile( bool tmp ) 
{
    SmiRecordFile* rf = new SmiRecordFile( false, 0, tmp);
    
    if( !rf->Create() )
    {
      string error;
      SmiEnvironment::GetLastErrorCode( error );
      cout << error << endl;
      assert( false );
    }

    SmiFileId fileId = rf->GetFileId();
    traceFile(fileId, tmp);
    files.insert( make_pair( fileId, rf ) );

    return rf;
}	


void FLOBCache::PutFLOB( SmiFileId& fileId, SmiRecordId& lobId,
                         long pageno, size_t size, bool isTempFile,
                         const char *flob )
{
  trace.enter(__FUNCTION__);

  SmiRecordFile* file = 0;
  if( fileId == 0 ) // assign a lob file id
  {
    assert( lobId == 0 );

    if (lobFile == 0) { 
      lobFile = CreateFile(isTempFile);
      lobFileId = lobFile->GetFileId();
    }
    file = lobFile;    
    fileId = lobFileId;
  }

  file = LookUpFile(fileId, isTempFile);

  SmiRecord record;
  bool append = (lobId == 0);


  if( pageno == -1 ) // ignore page number
  {
    if( append )
    {
      unsigned int RecordAppended = file->AppendRecord( lobId, record );
      assert( RecordAppended );
    }
    else
    {
      unsigned int RecordSelected =
          file->SelectRecord( lobId, record, SmiFile::Update );
      assert( RecordSelected );
    }
    record.Write( flob, size, 0 );
  }
  else // use page information
  {
    if( append )
    {
      assert( pageno == 0 );
      unsigned int RecordAppended = file->AppendRecord( lobId, record );
      assert( RecordAppended );
    }
    else
    {
      unsigned int RecordSelected =
          file->SelectRecord( lobId, record, SmiFile::Update );
      assert( RecordSelected );
    }

    assert( size <= FLOB::PAGE_SIZE );
    record.Write( flob, size, pageno * FLOB::PAGE_SIZE );
  }
}

void FLOBCache::Clear()
{
  trace.enter(__FUNCTION__);
  sizeLeft = maxSize;
  mapTable.clear();
  lruTable.Clear();
}

void FLOBCache::Clean()
{
  trace.enter(__FUNCTION__);
  Clear();
  for( FileTable::iterator i = files.begin();
       i != files.end();
       i++ )
  {
    i->second->Close();
    delete i->second;
  }
  files.clear();
  lobFile = 0;
  lobFileId = 0;
}

void FLOBCache::Release( SmiFileId fileId, SmiRecordId lobId, long pageno )
{
  trace.enter(__FUNCTION__);
  FLOBKey key( fileId, lobId, pageno, 0 );
  DecReference( key );
}

void FLOBCache::Destroy( SmiFileId fileId, SmiRecordId lobId )
{
  FLOBKey key( fileId, lobId, -1, 0 );
  FLOBTable::iterator iter = mapTable.find( key );

  if( iter != mapTable.end() )
  {
    FLOBCacheElement *e = iter->second;
    assert( e->refs == 1 );
    mapTable.erase( iter );
    lruTable.Remove( e );
    free( e->flob );
    delete e;
  }
  files[fileId]->DeleteRecord( lobId );
}

void FLOBCache::Truncate( SmiFileId fileId, bool isTemp, bool paged )
{
  trace.enter(__FUNCTION__);
  if( fileId != 0 )
  {
    SmiRecordFile* file = LookUpFile(fileId, isTemp);
    file->Truncate();
  }
}

void FLOBCache::Drop( SmiFileId fileId, bool isTemp, bool paged )
{
  trace.enter(__FUNCTION__);

  if( fileId != 0 )
  {
    trace.show(VAL(fileId));	  
    SmiRecordFile* file = LookUpFile(fileId, isTemp);
    trace.show(VAL(file->GetFileId()));	  
    file->Close();
    trace.show(VAL(file->GetFileId()));	  
    file->Drop();
    RemoveFile( fileId);
    delete file;
  }
}


SmiRecordFile* 
FLOBCache::LookUpFile( SmiFileId id, bool tmp ) {
   
   trace.enter(__FUNCTION__);
   trace.show( VAL(id) );

   SmiRecordFile* file = 0;
   FileTable::iterator iter = files.find( id );

   if( iter == files.end() ) // file not in the file table, try to open
   {
     file = new SmiRecordFile( false, 0, tmp );
     bool ok = file->Open(id); 
     assert(ok);
     traceFile(id, tmp, "open");
     files.insert( make_pair( id, file ) );
   }	   
   else
   {
     file = iter->second;
   }
   
   return file;
}	

void
FLOBCache::RemoveFile( SmiFileId id ) {
  
  trace.enter(__FUNCTION__);

  FileTable::iterator iter = files.find( id );
  assert( iter != files.end() );
  files.erase(iter);
}     	



void FLOBCache::DecReference( const FLOBKey key )
{
  trace.enter(__FUNCTION__);
  FLOBTable::iterator iter = mapTable.find( key );

  assert( iter != mapTable.end() );
  assert( iter->second->refs > 0 );
  iter->second->refs--;

  trace.add() << "Reference decreased (" << key << ") to "
              << iter->second->refs << endl;
  trace.flush(5);
}

