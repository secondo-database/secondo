/*

1 Implementation of SmiKeyedFile using the Berkeley-DB 

April 2002 Ulrich Telle

*/

using namespace std;

#include <string>
#include <algorithm>
#include <cctype>

#include <db_cxx.h>
#include "SecondoSMI.h"
#include "SmiBDB.h"
#include "SmiCodes.h"
#include "Profiles.h"

/* --- Implementation of class SmiKeyedFile --- */

SmiKeyedFile::SmiKeyedFile( const SmiKey::KeyDataType keyType,
                            const bool hasUniqueKeys /* = true */ )
{
  fileType    = Keyed;
  keyDataType = keyType;
  uniqueKeys  = hasUniqueKeys;
}
  
SmiKeyedFile::~SmiKeyedFile()
{
}
  
bool
SmiKeyedFile::SelectRecord( const SmiKey& key,
                            SmiKeyedFileIterator& iterator,
                            const SmiFile::AccessType accessType
                              /* = SmiFile::ReadOnly */ )
{
  int rc;
  DbTxn* tid = SmiEnvironment::instance.impl->usrTxn;
  Dbc* dbc;
  if ( accessType == SmiFile::Update || !impl->isSystemCatalogFile )
  {
    rc = impl->bdbFile->cursor( tid, &dbc, 0 );
  }
  else
  {
    rc = impl->bdbFile->cursor( 0, &dbc, 0 );
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
    SmiEnvironment::SetError( E_SMI_OK );
  }
  else
  {
    SmiEnvironment::SetError( E_SMI_RECORD_SELECT, rc );
  }
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
  DbTxn* tid = SmiEnvironment::instance.impl->usrTxn;
  if ( uniqueKeys && accessType == SmiFile::Update )
  {
    rc = impl->bdbFile->get( tid, &bdbKey, &data, DB_RMW );
  }
  else if ( !impl->isSystemCatalogFile )
  {
    rc = impl->bdbFile->get( tid, &bdbKey, &data, 0 );
  }
  else
  {
    rc = impl->bdbFile->get( 0, &bdbKey, &data, 0 );
  }
  if ( rc == ENOMEM )
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
    SmiEnvironment::SetError( E_SMI_OK );
  }
  else
  {
    SmiEnvironment::SetError( E_SMI_RECORD_SELECT, rc );
    record.initialized     = false;
  }

  return (record.initialized);
}

bool
SmiKeyedFile::SelectRange( const SmiKey& fromKey, 
                           const SmiKey& toKey, 
                           SmiKeyedFileIterator& iterator,
                           const SmiFile::AccessType accessType
                             /* = SmiFile::ReadOnly */,
                           const bool reportDuplicates /* = false */ )
{
  int rc;
  DbTxn* tid = SmiEnvironment::instance.impl->usrTxn;
  Dbc* dbc;
  if ( accessType == SmiFile::Update || !impl->isSystemCatalogFile )
  {
    rc = impl->bdbFile->cursor( tid, &dbc, 0 );
  }
  else
  {
    rc = impl->bdbFile->cursor( 0, &dbc, 0 );
  }
  if ( rc == 0 )
  {
    iterator.smiFile          = this;
    iterator.opened           = true;
    iterator.impl->bdbCursor  = dbc;
    iterator.solelyDuplicates = false;
    iterator.ignoreDuplicates = !reportDuplicates;
    iterator.rangeSearch      = true;
    iterator.firstKey         = fromKey;
    iterator.lastKey          = toKey;
    iterator.searchKey        = &iterator.firstKey;
    SmiEnvironment::SetError( E_SMI_OK );
  }
  else
  {
    SmiEnvironment::SetError( E_SMI_RECORD_SELECT, rc );
  }
  return (rc == 0);
}

bool
SmiKeyedFile::SelectLeftRange( const SmiKey& toKey, 
                               SmiKeyedFileIterator& iterator,
                               const SmiFile::AccessType accessType
                                 /* = SmiFile::ReadOnly */,
                               const bool reportDuplicates /* = false */ )
{
  int rc;
  DbTxn* tid = SmiEnvironment::instance.impl->usrTxn;
  Dbc* dbc;
  if ( accessType == SmiFile::Update || !impl->isSystemCatalogFile )
  {
    rc = impl->bdbFile->cursor( tid, &dbc, 0 );
  }
  else
  {
    rc = impl->bdbFile->cursor( 0, &dbc, 0 );
  }
  if ( rc == 0 )
  {
    iterator.smiFile          = this;
    iterator.opened           = true;
    iterator.impl->bdbCursor  = dbc;
    iterator.solelyDuplicates = false;
    iterator.ignoreDuplicates = !reportDuplicates;
    iterator.rangeSearch      = false;
    SmiKey dummy( toKey.mapFunc );
    iterator.firstKey         = dummy;
    iterator.lastKey          = toKey;
    iterator.searchKey        = &iterator.firstKey;
    SmiEnvironment::SetError( E_SMI_OK );
  }
  else
  {
    SmiEnvironment::SetError( E_SMI_RECORD_SELECT, rc );
  }
  return (rc == 0);
}

bool
SmiKeyedFile::SelectRightRange( const SmiKey& fromKey, 
                                SmiKeyedFileIterator& iterator,
                                const SmiFile::AccessType accessType
                                  /* = SmiFile::ReadOnly */,
                                const bool reportDuplicates /* = false */ )
{
  int rc;
  DbTxn* tid = SmiEnvironment::instance.impl->usrTxn;
  Dbc* dbc;
  if ( accessType == SmiFile::Update || !impl->isSystemCatalogFile )
  {
    rc = impl->bdbFile->cursor( tid, &dbc, 0 );
  }
  else
  {
    rc = impl->bdbFile->cursor( 0, &dbc, 0 );
  }
  if ( rc == 0 )
  {
    iterator.smiFile          = this;
    iterator.opened           = true;
    iterator.impl->bdbCursor  = dbc;
    iterator.solelyDuplicates = false;
    iterator.ignoreDuplicates = !reportDuplicates;
    iterator.rangeSearch      = true;
    SmiKey dummy( fromKey.mapFunc );
    iterator.firstKey         = fromKey;
    iterator.lastKey          = dummy;
    iterator.searchKey        = &iterator.firstKey;
    SmiEnvironment::SetError( E_SMI_OK );
  }
  else
  {
    SmiEnvironment::SetError( E_SMI_RECORD_SELECT, rc );
  }
  return (rc == 0);
}

bool
SmiKeyedFile::SelectAll( SmiKeyedFileIterator& iterator,
                         const SmiFile::AccessType accessType
                           /* = SmiFile::ReadOnly */,
                         const bool reportDuplicates /* = false */ )
{
  int rc;
  DbTxn* tid = SmiEnvironment::instance.impl->usrTxn;
  Dbc* dbc;
  if ( accessType == SmiFile::Update || !impl->isSystemCatalogFile )
  {
    rc = impl->bdbFile->cursor( tid, &dbc, 0 );
  }
  else
  {
    rc = impl->bdbFile->cursor( 0, &dbc, 0 );
  }
  if ( rc == 0 )
  {
    iterator.smiFile          = this;
    iterator.opened           = true;
    iterator.impl->bdbCursor  = dbc;
    iterator.solelyDuplicates = false;
    iterator.ignoreDuplicates = !reportDuplicates;
    iterator.rangeSearch      = false;
    SmiKey dummy;
    iterator.firstKey         = dummy;
    iterator.lastKey          = dummy;
    iterator.searchKey        = &iterator.firstKey;
    SmiEnvironment::SetError( E_SMI_OK );
  }
  else
  {
    SmiEnvironment::SetError( E_SMI_RECORD_SELECT, rc );
  }
  return (rc == 0);
}

bool
SmiKeyedFile::InsertRecord( const SmiKey& key, SmiRecord& record )
{
  int rc = 0;
  char buffer = 0;
  Dbt bdbKey( (void*) key.GetAddr(), key.keyLength );
  Dbt data( &buffer, 0 );
  data.set_dlen( 0 );
  DbTxn* tid = SmiEnvironment::instance.impl->usrTxn;
  Dbc* dbc;

  if ( uniqueKeys )
  {
    rc = impl->bdbFile->put( tid, &bdbKey, &data, DB_NOOVERWRITE );
  }
  else
  {
    // --- Use of a cursor is required when duplicates are allowed
    rc = impl->bdbFile->cursor( tid, &dbc, 0 );
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
    SmiEnvironment::SetError( E_SMI_OK );
  }
  else if ( rc == DB_KEYEXIST )
  {
    SmiEnvironment::SetError( E_SMI_FILE_KEYEXIST );
    record.initialized = false;
  }
  else
  {
    SmiEnvironment::SetError( E_SMI_RECORD_INSERT, rc );
    record.initialized = false;
  }
  return (record.initialized);
}
  
bool
SmiKeyedFile::DeleteRecord( const SmiKey& key )
{
  int rc = 0;
  Dbt bdbKey( (void *) key.GetAddr(), key.keyLength );
  DbTxn* tid = SmiEnvironment::instance.impl->usrTxn;
  
  rc = impl->bdbFile->del( tid, &bdbKey, 0 );
  if ( rc == 0 )
  {
    SmiEnvironment::SetError( E_SMI_OK );
  }
  else
  {
    SmiEnvironment::SetError( E_SMI_RECORD_DELETE, rc );
  }
  
  return (rc == 0);  
}

/* --- Implementation of class  --- */

SmiKeyedFileIterator::SmiKeyedFileIterator( bool reportDuplicates = false )
  : firstKey(), lastKey()
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
    MapKeyFunc oldFunc = key.mapFunc;
    key = record.recordKey;
    key.mapFunc = oldFunc;
    if ( lastKey.GetType() != SmiKey::Unknown )
    {
      ok = !(record.recordKey > lastKey);
      if ( !ok )
      {
        endOfScan = true;
        SmiEnvironment::SetError( E_SMI_CURSOR_ENDOFSCAN );
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
      SmiEnvironment::SetError( E_SMI_CURSOR_ENDOFSCAN );
    }
  }
  return (ok);
}
  
/* --- bdbKeyedFile.cpp --- */

