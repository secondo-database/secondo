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

1 Implementation of SmiKeyedFile using the Berkeley-DB

April 2002 Ulrich Telle

September 2002 Ulrich Telle, fixed flag (DB\_DIRTY\_READ) in Berkeley DB calls for system catalog files

April 2003 Ulrich Telle, implemented temporary SmiFiles

May 2008, Victor Almeida created the two sons of the ~SmiKeyedFile~ class, namely ~SmiBtreeFile~ and
~SmiHashFile~, for B-Tree and hash access methods, respectively.

*/


//#define TRACE_ON 1
#undef TRACE_ON
#include "LogMsg.h"

#include <string>
#include <algorithm>
#include <cctype>
#include <cassert>

#include <db_cxx.h>
#include "DbVersion.h"
#include "SecondoSMI.h"
#include "SmiBDB.h"
#include "SmiCodes.h"
#include "Profiles.h"

using namespace std;

/* --- Implementation of class SmiKeyedFile --- */

SmiKeyedFile::SmiKeyedFile( const SmiFile::FileType fileType,
                            const SmiKey::KeyDataType keyType,
                            const bool hasUniqueKeys /* = true */,
                            const bool isTemporary /* = false */ )
  : SmiCachedFile( isTemporary )
{
  this->fileType = fileType;
  keyDataType = keyType;
  uniqueKeys  = hasUniqueKeys;
}

SmiKeyedFile::~SmiKeyedFile()
{
}

SmiKey::KeyDataType
SmiKeyedFile::GetKeyType() const
{
  return keyDataType;
}

bool
SmiKeyedFile::SelectRecord( const SmiKey& key,
                            SmiKeyedFileIterator& iterator,
                            const SmiFile::AccessType accessType
                              /* = SmiFile::ReadOnly */ )
{
  int rc=0;
  Dbc* dbc = 0;
  DbTxn* tid = !impl->isTemporaryFile ?
               SmiEnvironment::instance.impl->usrTxn : 0;

  if ( accessType == SmiFile::Update || !impl->isSystemCatalogFile )
  {
    rc = impl->bdbFile->cursor( tid, &dbc, 0 );
  }
  else
  {
    u_int32_t flags = (!impl->isTemporaryFile) && useTxn ? DB_DIRTY_READ : 0;
    rc = impl->bdbFile->cursor( 0, &dbc, flags );
  }
  if ( rc == 0 )
  {
    iterator.smiFile          = this;
    iterator.opened           = true;
    iterator.impl->bdbCursor  = dbc;
    iterator.solelyDuplicates = true;
    iterator.ignoreDuplicates = false;
    iterator.rangeSearch      = false;
    iterator.firstKey         = key;
    iterator.searchKey        = &iterator.firstKey;
  }

  if (rc != DB_NOTFOUND)
    SmiEnvironment::SetBDBError(rc);
  return (rc == 0);
}

bool
SmiKeyedFile::SelectRecord( const SmiKey& key,
                            SmiRecord& record,
                            const SmiFile::AccessType accessType
                              /* = SmiFile::ReadOnly */ )
{
  int rc = 0;
  Dbt bdbKey;
  bdbKey.set_data( (void*) key.GetAddr() );
  bdbKey.set_size( key.keyLength );
  Dbt data;
  data.set_ulen( 0 );
  data.set_flags( DB_DBT_USERMEM );
  DbTxn* tid = !impl->isTemporaryFile ?
               SmiEnvironment::instance.impl->usrTxn : 0;

  if ( uniqueKeys && accessType == SmiFile::Update )
  {
    u_int32_t flags = (!impl->isTemporaryFile) && useTxn ? DB_RMW : 0;
    rc = impl->bdbFile->get( tid, &bdbKey, &data, flags );
  }
  else if ( !impl->isSystemCatalogFile )
  {
    rc = impl->bdbFile->get( tid, &bdbKey, &data, 0 );
  }
  else
  {
    u_int32_t flags = (!impl->isTemporaryFile) && useTxn ? DB_DIRTY_READ : 0;
    rc = impl->bdbFile->get( 0, &bdbKey, &data, flags );
  }

// VTA - 15.11.2005 - to compile with the new version of Berkeley DB
#if ( DB_VERSION_REQUIRED(4,3))
  if ( rc == DB_BUFFER_SMALL )
#else
  if ( rc == ENOMEM )
#endif
  {
    if ( record.initialized )
    {
      record.Finish();
    }
    record.recordKey         = key;
    record.recordSize        = data.get_size();
    record.writable          = (uniqueKeys && accessType == SmiFile::Update);
    record.smiFile           = this;
    record.impl->bdbFile     = impl->bdbFile;
    record.impl->useCursor   = false;
    record.impl->closeCursor = false;
    record.impl->bdbCursor   = 0;
    record.initialized       = true;
  }
  else
  {
    if (rc != DB_NOTFOUND)
      SmiEnvironment::SetBDBError(rc);
    record.initialized     = false;
  }

  return (record.initialized);
}

bool
SmiKeyedFile::InsertRecord( const SmiKey& key, SmiRecord& record )
{
  int rc = 0;
  char buffer = 0;
  Dbc* dbc = 0;

  Dbt bdbKey( (void*) key.GetAddr(), key.keyLength );
  Dbt data( &buffer, 0 );
  data.set_dlen( 0 );
  DbTxn* tid = !impl->isTemporaryFile ?
               SmiEnvironment::instance.impl->usrTxn : 0;

  if ( uniqueKeys )
  {
    rc = impl->bdbFile->put( tid, &bdbKey, &data, DB_NOOVERWRITE );
  }
  else
  {
    // --- Use of a cursor is required when duplicates are allowed
    rc = impl->bdbFile->cursor( tid, &dbc, 0 );
    SmiEnvironment::SetBDBError( rc );
    if ( rc == 0 )
    {
      rc = dbc->put( &bdbKey, &data, DB_KEYLAST );
    }
  }

  if ( rc == 0 )
  {
    if ( record.initialized )
    {
      record.Finish();
    }
    record.recordKey       = key;
    record.recordSize      = data.get_size();
    record.writable        = true;
    record.smiFile         = this;
    record.impl->bdbFile   = impl->bdbFile;
    if ( uniqueKeys )
    {
      record.impl->useCursor   = false;
      record.impl->closeCursor = false;
      record.impl->bdbCursor   = 0;
    }
    else
    {
      record.impl->useCursor   = true;
      record.impl->closeCursor = true;
      record.impl->bdbCursor   = dbc;
    }
    record.initialized = true;
  }
  else if ( rc == DB_KEYEXIST )
  {
    SmiEnvironment::SetError( E_SMI_FILE_KEYEXIST );
    record.initialized = false;
  }
  else
  {
    SmiEnvironment::SetBDBError( rc );
    record.initialized = false;
  }
  return (record.initialized);
}

bool
SmiKeyedFile::DeleteRecord( const SmiKey& key,
                            const bool all, const SmiRecordId recordId )
{
  if( all )
  {
    int rc = 0;

    Dbt bdbKey( (void *) key.GetAddr(), key.keyLength );
    DbTxn* tid = !impl->isTemporaryFile ?
                 SmiEnvironment::instance.impl->usrTxn : 0;

    rc = impl->bdbFile->del( tid, &bdbKey, 0 );
    SmiEnvironment::SetBDBError( rc );
    return (rc == 0);
  }
  else
  {
    SmiKeyedFileIterator iter;
    if( SelectRecord( key, iter, SmiFile::Update ) )
    {
      SmiRecord record;
      while( iter.Next( record ) )
      {
        SmiSize bytesRead;
        SmiRecordId id;
        SmiSize idSize = sizeof(SmiRecordId);
        bytesRead = record.Read(&id, idSize);
        if( bytesRead == idSize && id == recordId )
          return iter.DeleteCurrent();
      }
    }
    return false;
  }
}

/* --- Implementation of class  --- */

SmiKeyedFileIterator::SmiKeyedFileIterator( bool reportDuplicates )
  : SmiFileIterator(), firstKey(), lastKey()
{
}

SmiKeyedFileIterator::~SmiKeyedFileIterator()
{
}

bool
SmiKeyedFileIterator::Next( SmiKey& key, SmiRecord& record )
{
  bool ok = SmiFileIterator::Next( record );
  if ( ok )
  {
    key = record.recordKey;
    if ( lastKey.GetType() != SmiKey::Unknown )
    {
      ok = !(record.recordKey > lastKey);
      if ( !ok )
      {
        endOfScan = true;
      }
    }
  }
  return (ok);
}

bool
SmiKeyedFileIterator::Next( SmiRecord& record )
{
  bool ok = SmiFileIterator::Next( record );
  if ( ok && lastKey.GetType() != SmiKey::Unknown )
  {
    ok = !(record.recordKey > lastKey);
    if ( !ok )
    {
      endOfScan = true;
    }
  }
  return (ok);
}

/* --- bdbKeyedFile.cpp --- */

