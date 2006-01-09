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
  Clear();
}

char *FLOBCache::Lookup( const FLOBKey key, bool inc )
{
  map< FLOBKey, FLOBCacheElement* >::iterator iter = 
    mapTable.find( key );

  if( iter == mapTable.end() )
    return NULL;

  FLOBCacheElement *e = iter->second;
  lruTable.Promote( e, inc );
  return e->flob;
}

void FLOBCache::Insert( const FLOBKey key, char *flob )
{
  assert( mapTable.find( key ) == mapTable.end() );

  FLOBCacheElement *e = new FLOBCacheElement( key, flob );

  FLOBKey key2;

  while( sizeLeft < key.size && mapTable.size() > 0 && 
         RemoveLast( key2 ) )
    sizeLeft += key2.size;

  if( sizeLeft >= key.size )
  {
    sizeLeft -= key.size;
    mapTable[key] = e;
    lruTable.Insert( e );
  }
  else
    Counter::getRef("RA:FLOBCacheFull")++;
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

    char *flob = iter->second->flob;
    mapTable.erase( iter );
    free( flob );
    return true;
  }
  return false; 
}

char *FLOBCache::GetFLOB( SmiFileId fileId, SmiRecordId lobId, 
                          int size, bool isTempFile )
{
  assert( fileId != 0 && lobId != 0 );

  char *flob;

  FLOBKey key( fileId, lobId, size, isTempFile );
  if( (flob = Lookup( key, true )) == NULL )
    // We need to read
  {
    flob = (char*) malloc( key.size );

    map< SmiFileId, SmiRecordFile* >::iterator iter = 
      files.find( key.fileId );

    SmiRecordFile *file;
    SmiRecord record;
    if( iter == files.end() )
    {
      file = new SmiRecordFile( false, 0, isTempFile );
      file->Open( key.fileId );
      files.insert( make_pair( key.fileId, file ) );
    }
    else
      file = iter->second;
    file->SelectRecord( key.recordId, record );
    record.Read( flob, key.size, 0 );
    Insert( key, flob );
    Counter::getRef("RA:FLOBCacheMisses")++;
  }
  else
    Counter::getRef("RA:FLOBCacheHits")++;

  return flob;
}

void FLOBCache::PutFLOB( SmiFileId& fileId, SmiRecordId& lobId, 
                         int size, bool isTempFile, 
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

  SmiRecord lob;
  if( lobId == 0 )
    assert( file->AppendRecord( lobId, lob ) );
  else
    assert( file->SelectRecord( lobId, lob ) );

  lob.Write( flob, size, 0 );
}

void FLOBCache::Clear()
{
  sizeLeft = maxSize;
  for( map< SmiFileId, SmiRecordFile* >::iterator i = files.begin();
       i != files.end();
       i++ )
  {
    i->second->Close();
    delete i->second;
  }
  files.clear();

  mapTable.clear();
  lruTable.Clear();
}

void FLOBCache::Release( SmiFileId fileId, SmiRecordId lobId )
{
  FLOBKey key( fileId, lobId, 0 );
  DecReference( key );
}

void FLOBCache::Destroy( SmiFileId fileId, SmiRecordId lobId )
{
  FLOBKey key( fileId, lobId, 0 );
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

void FLOBCache::Truncate( SmiFileId fileId, bool isTemp )
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

void FLOBCache::Drop( SmiFileId fileId, bool isTemp )
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
      file = iter->second;
    file->Close();
    file->Drop();
  }
}

void FLOBCache::DecReference( const FLOBKey key )
{
  map< FLOBKey, FLOBCacheElement* >::iterator iter = 
    mapTable.find( key );

  if( iter != mapTable.end() )
  {
    assert( iter->second->refs > 0 );
    iter->second->refs--;
  }
}

