/*

1 Implementation of SmiRecordFile using the Berkeley-DB 

April 2002 Ulrich Telle

*/

#include <string>
#include <algorithm>
#include <cctype>
#include <errno.h>

#include <db_cxx.h>
#include "SecondoSMI.h"
#include "SmiBDB.h"
#include "SmiCodes.h"

using namespace std;

/* --- Implementation of class SmiRecordFile --- */

SmiRecordFile::SmiRecordFile( bool hasFixedLengthRecords,
                              SmiSize recordLength = 0 )
{
  opened = false;
  if ( hasFixedLengthRecords )
  {
    fileType          = FixedLength;
    fixedRecordLength = recordLength;
    uniqueKeys        = true;
  }
  else
  {
    fileType          = VariableLength;
    fixedRecordLength = 0;
    uniqueKeys        = true;
  }
}
                 
SmiRecordFile::~SmiRecordFile()
{
}

bool
SmiRecordFile::SelectRecord( const SmiRecordId recno, 
                             SmiRecord& record,
                             const SmiFile::AccessType accessType =
                                   SmiFile::ReadOnly )
{
  int rc = 0;
  Dbt key;
  Dbt data;
  data.set_ulen( 0 );
  data.set_flags( DB_DBT_USERMEM );
  DbTxn* tid = SmiEnvironment::instance.impl->usrTxn;
  key.set_data( (void*) &recno );
  key.set_size( sizeof( SmiRecordId ) );
  if ( accessType == SmiFile::Update )
  {
    rc = impl->bdbFile->get( tid, &key, &data, DB_RMW );
  }
  else if ( !impl->isSystemCatalogFile )
  {
    rc = impl->bdbFile->get( tid, &key, &data, 0 );
  }
  else
  {
    rc = impl->bdbFile->get( 0, &key, &data, 0 );
  }
  if ( rc == ENOMEM )
  {
    if ( record.initialized )
    {
      record.Finish();
    }
    record.recordKey.SetKey( recno );
    record.recordSize      = data.get_size();
    record.fixedSize       = (fileType == SmiFile::FixedLength);
    record.writable        = (accessType == SmiFile::Update);
    record.smiFile         = this;
    record.impl->bdbFile   = impl->bdbFile;
    record.impl->useCursor = false;
    record.impl->bdbCursor = 0;
    record.initialized     = true;
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
SmiRecordFile::SelectAll( SmiRecordFileIterator& iterator,
                          const SmiFile::AccessType accessType =
                                SmiFile::ReadOnly )
{
  int rc;
  DbTxn* tid = SmiEnvironment::instance.impl->usrTxn;
  Dbc* dbc;
  rc = impl->bdbFile->cursor( tid, &dbc, 0 );
  if ( rc == 0 )
  {
    iterator.smiFile          = this;
    iterator.opened           = true;
    iterator.writable         = (accessType == SmiFile::Update);
    iterator.restart          = true;
    iterator.impl->bdbCursor  = dbc;
    iterator.solelyDuplicates = false;
    iterator.ignoreDuplicates = false;
    iterator.rangeSearch      = false;
    SmiEnvironment::SetError( E_SMI_OK );
  }
  else
  {
    SmiEnvironment::SetError( E_SMI_RECORD_SELECTALL, rc );
  }
  return (rc == 0);
}

bool
SmiRecordFile::AppendRecord( SmiRecordId& recno, SmiRecord& record )
{
  int rc = 0;
  char buffer = 0;
  Dbt key( &recno, sizeof( SmiRecordId ));
  Dbt data( &buffer, 0 );
  data.set_flags( DB_DBT_PARTIAL );
  data.set_dlen( 0 );
  DbTxn* tid = SmiEnvironment::instance.impl->usrTxn;
  rc = impl->bdbFile->put( tid, &key, &data, DB_APPEND );
  if ( rc == 0 )
  {
    if ( record.initialized )
    {
      record.Finish();
    }
    recno = *((SmiRecordId*) key.get_data());
    record.recordKey.SetKey( (SmiRecordId) recno );
    record.recordSize      = data.get_size();
    record.fixedSize       = (fileType == SmiFile::FixedLength);
    record.writable        = true;
    record.smiFile         = this;
    record.impl->bdbFile   = impl->bdbFile;
    record.impl->useCursor = false;
    record.impl->bdbCursor = 0;
    record.initialized     = true;
    SmiEnvironment::SetError( E_SMI_OK );
  }
  else
  {
    SmiEnvironment::SetError( E_SMI_RECORD_APPEND, rc );
    record.initialized     = false;
  }

  return (record.initialized);
}

bool SmiRecordFile::DeleteRecord( SmiRecordId recno )
{
  int rc = 0;
  Dbt key( &recno, sizeof( recno ) );
  DbTxn* tid = SmiEnvironment::instance.impl->usrTxn;
  
  rc = impl->bdbFile->del( tid, &key, 0 );
  if ( rc == 0 )
    SmiEnvironment::SetError( E_SMI_OK );
  else
    SmiEnvironment::SetError( E_SMI_RECORD_DELETE, rc );
  
  return (rc == 0);  
}

/* --- Implementation of class SmiRecordFileIterator --- */

SmiRecordFileIterator::SmiRecordFileIterator()
{
}

SmiRecordFileIterator::~SmiRecordFileIterator()
{
}

bool SmiRecordFileIterator::Next( SmiRecordId& recno, SmiRecord& record )
{
  bool ok = SmiFileIterator::Next( record );
  if ( ok )
  {
    ok = record.recordKey.GetKey( recno );
  }
  return ok;
}

bool SmiRecordFileIterator::Next( SmiRecord& record )
{
  return (SmiFileIterator::Next( record ));
}

/* --- bdbRecordFile.cpp --- */

