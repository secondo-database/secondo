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

[1] Implementation File of Module FLOB Cache

Victor Almeida, 08/12/2005. 

1 Includes

*/
#include "Counter.h"
#include "FLOB.h"
#include "FLOBCache.h"
#include <cassert>

using namespace std;

// #define __NEW_FLOB__

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
    if( aux->refs > 0 )
      cout << "FLOBCache ERROR: Key (" 
           << aux->key.fileId << ", " 
           << aux->key.recordId << ", "  
           << aux->key.pageno << ") still has " 
           << aux->refs << " references while cleaning the cache." 
           << endl;
    assert( aux->refs == 0 );
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
  Clean();
}

char *FLOBCache::Lookup( const FLOBKey key, bool inc )
{
//  cout << "Lookup" << endl;

  map< FLOBKey, FLOBCacheElement* >::iterator iter = 
    mapTable.find( key );

  if( iter == mapTable.end() )
  {
//    cout << "Entry (" << key.fileId << ", " 
//         << key.recordId << ", " << key.pageno << ") not found" << endl;
    return NULL;
  }

  FLOBCacheElement *e = iter->second;
  lruTable.Promote( e, inc );

//  cout << "Entry (" << key.fileId << ", "
//       << key.recordId << ", " << key.pageno << ") found, now with " 
//       << e->refs << " references" << endl;

  return e->flob;
}

bool FLOBCache::Insert( const FLOBKey key, char *flob )
{
//  cout << "FLOBCache::Insert (" << key.fileId << ", "
//       << key.recordId << ", " << key.pageno << ")" << endl;

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

//    cout << "Entry inserted into the cache with " 
//         << e->refs << " references" << endl;

    return true;
  }

  Counter::getRef("RA:FLOBCacheFull")++;

//  cout << "Cache full of entries with at least 1 reference" << endl;

  return false;
}

void FLOBCache::Remove( const FLOBKey key )
{
  map< FLOBKey, FLOBCacheElement* >::iterator iter = 
    mapTable.find( key );

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
  if( lruTable.RemoveLast( key ) )
  {
    map< FLOBKey, FLOBCacheElement* >::iterator iter = 
      mapTable.find( key );

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
  assert( fileId != 0 && lobId != 0 );

  FLOBKey key( fileId, lobId, pageno, size, isTempFile );
  if( (flob = Lookup( key, true )) == NULL )
    // We need to read
  {
    char *buf = (char*) malloc( key.size );
    flob = buf;

    map< SmiFileId, SmiRecordFile* >::iterator iter = 
      files.find( key.fileId );

    SmiRecordFile *file;
    if( iter == files.end() )
    {
      file = new SmiRecordFile( false, 0, isTempFile );
      file->Open( key.fileId );
      files.insert( make_pair( key.fileId, file ) );
    }
    else
      file = iter->second;

    SmiRecord record;
    file->SelectRecord( key.recordId, record );

    if( pageno == -1 )
      assert( record.Read( buf, key.size, 0 ) == (unsigned int)key.size );
    else
      assert( record.Read( buf, 
                           FLOB::PAGE_SIZE, 
                           pageno * FLOB::PAGE_SIZE ) == FLOB::PAGE_SIZE );

    if( !Insert( key, buf ) )
      return false;

    Counter::getRef("RA:FLOBCacheMisses")++;
  }
  else
    Counter::getRef("RA:FLOBCacheHits")++;

  return true;
}

void FLOBCache::PutFLOB( SmiFileId& fileId, SmiRecordId& lobId, 
                         long pageno, size_t size, bool isTempFile, 
                         const char *flob )
{
  SmiRecordFile *file;
  if( fileId == 0 )
  {
    assert( lobId == 0 );
    file = new SmiRecordFile( false, 0, isTempFile );

    if( !file->Create() )
    {
      string error;
      SmiEnvironment::GetLastErrorCode( error );
      cout << error << endl;
      assert( false );
    }
    fileId = file->GetFileId();
    files.insert( make_pair( fileId, file ) );
  }
  else
  {
    map< SmiFileId, SmiRecordFile* >::iterator iter = 
      files.find( fileId );

    if( iter == files.end() )
    {
      file = new SmiRecordFile( false, 0, isTempFile );
      file->Open( fileId );
      files.insert( make_pair( fileId, file ) );
    }
    else
      file = iter->second;
  }

  SmiRecord record;
  bool append = lobId == 0;

  if( pageno == -1 )
  {
    if( append )
      assert( file->AppendRecord( lobId, record ) );
    else
      assert( file->SelectRecord( lobId, record, SmiFile::Update ) );

    record.Write( flob, size, 0 );
  }
  else
  {
    if( append )
    {
      assert( pageno == 0 );
      assert( file->AppendRecord( lobId, record ) );
    }
    else
      assert( file->SelectRecord( lobId, record, SmiFile::Update ) );

    assert( size <= FLOB::PAGE_SIZE );
    record.Write( flob, size, pageno * FLOB::PAGE_SIZE );
  }
}

void FLOBCache::Clear()
{
  sizeLeft = maxSize;
  mapTable.clear();
  lruTable.Clear();
}

void FLOBCache::Clean()
{
  Clear();
  for( map< SmiFileId, SmiRecordFile* >::iterator i = files.begin();
       i != files.end();
       i++ )
  {
    i->second->Close();
    delete i->second;
  }
  files.clear();
}

void FLOBCache::Release( SmiFileId fileId, SmiRecordId lobId, long pageno )
{
//  cout << "FLOBCache::Release" << endl;
  FLOBKey key( fileId, lobId, pageno, 0 );
  DecReference( key );
}

void FLOBCache::Destroy( SmiFileId fileId, SmiRecordId lobId )
{
  FLOBKey key( fileId, lobId, -1, 0 );
  map< FLOBKey, FLOBCacheElement* >::iterator iter = 
    mapTable.find( key );

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
  if( fileId != 0 )
  {
    SmiRecordFile *file;
    map< SmiFileId, SmiRecordFile* >::iterator iter = 
      files.find( fileId );

    if( iter == files.end() )
    {
      file = new SmiRecordFile( false, 0, isTemp );
      file->Open( fileId );
    }
    else
      file = iter->second;
    file->Truncate();
  }
}

void FLOBCache::Drop( SmiFileId fileId, bool isTemp, bool paged )
{
  if( fileId != 0 )
  {
    SmiRecordFile *file;
    map< SmiFileId, SmiRecordFile* >::iterator iter = 
      files.find( fileId );

    if( iter == files.end() )
    {
      file = new SmiRecordFile( false, 0, isTemp );
      assert( file->Open( fileId ) );
    }
    else
    {
      file = iter->second;
      files.erase( iter );
    }

    file->Close();
    file->Drop();
    delete file;
  }
}

void FLOBCache::DecReference( const FLOBKey key )
{
  map< FLOBKey, FLOBCacheElement* >::iterator iter = 
    mapTable.find( key );

  assert( iter != mapTable.end() );
  assert( iter->second->refs > 0 );
  iter->second->refs--;

//  cout << "Reference decreased (" << key.fileId << ", "
//       << key.recordId << ", " << key.pageno << ") to " 
//       << iter->second->refs << endl;
}

