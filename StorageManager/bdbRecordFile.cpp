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

//[_] [\_]

1 Implementation of SmiRecordFile using the Berkeley-DB

April 2002 Ulrich Telle

September 2002 Ulrich Telle, fixed flag (DB[_]DIRTY[_]READ) in Berkeley DB calls for system catalog files

October 2002 Ulrich Telle, fixed bug causing problems with iterators returning record ids (keyDataType was not set appropriately)

April 2003 Ulrich Telle, implemented temporary SmiFiles

*/

using namespace std;

#include <string>
#include <algorithm>
#include <cctype>
#include <errno.h>
#include <sstream>
#include <string>

#include <db_cxx.h>
#include "SecondoSMI.h"
#include "SmiBDB.h"
#include "SmiCodes.h"

extern string lu_2_s(uint32_t value); // defined in bdbFile.cpp

/* --- Implementation of class SmiRecordFile --- */

SmiRecordFile::SmiRecordFile( bool hasFixedLengthRecords,
                              SmiSize recordLength /* = 0 */,
                              const bool isTemporary /* = false */ )
  : SmiFile( isTemporary )
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
  keyDataType = SmiKey::RecNo;
}

SmiRecordFile::~SmiRecordFile()
{
}

bool
SmiRecordFile::SelectRecord( const SmiRecordId recno,
                             SmiRecord& record,
                             const SmiFile::AccessType accessType
                               /* = SmiFile::ReadOnly */ )
{

  int rc = 0;
  Dbt key;
  Dbt data;
  data.set_ulen( 0 );
  data.set_flags( DB_DBT_USERMEM );
  DbTxn* tid = !impl->isTemporaryFile ?
	                SmiEnvironment::instance.impl->usrTxn : 0;

  key.set_data( (void*) &recno );
  key.set_size( sizeof( SmiRecordId ) );
  if ( accessType == SmiFile::Update )
  {
    u_int32_t flags = (!impl->isTemporaryFile) && useTxn ? DB_RMW : 0;
    rc = impl->bdbFile->get( tid, &key, &data, flags );
  }
  else if ( !impl->isSystemCatalogFile )
  {
    rc = impl->bdbFile->get( tid, &key, &data, 0 );
  }
  else
  {
    u_int32_t flags = (!impl->isTemporaryFile) && useTxn ? DB_DIRTY_READ : 0;
    rc = impl->bdbFile->get( 0, &key, &data, flags );
  }

// VTA - 15.11.2005 - to compile with the new version of Berkeley DB
#if (DB_VERSION_MAJOR == 4) && (DB_VERSION_MINOR == 3)
  if ( rc == DB_BUFFER_SMALL )
#else
  if ( rc == ENOMEM )
#endif
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
  }
  else
  {
    if (rc != DB_NOTFOUND && rc != DB_KEYEMPTY)
      SmiEnvironment::SetBDBError(rc);
    record.initialized     = false;
  }

  return (record.initialized);
}

bool
SmiRecordFile::SelectAll( SmiRecordFileIterator& iterator,
                          const SmiFile::AccessType accessType
                                                    /*= SmiFile::ReadOnly */)
{
  Dbc* dbc = 0;

  DbTxn* tid = 0;
  if ( !impl->isTemporaryFile )
    tid = SmiEnvironment::instance.impl->usrTxn;

  int rc = impl->bdbFile->cursor( tid, &dbc, 0 );
  SmiEnvironment::SetBDBError( rc );

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
  }
  return (rc == 0);
}

PrefetchingIterator*
SmiRecordFile::SelectAllPrefetched()
{
  Dbc* dbc = 0;
  DbTxn* tid = !impl->isTemporaryFile ?
	          SmiEnvironment::instance.impl->usrTxn : 0;

  int rc = impl->bdbFile->cursor(tid, &dbc, 0);
  SmiEnvironment::SetBDBError( rc );

  if(rc == 0)
  {
    return
      new PrefetchingIteratorImpl(dbc, SmiKey::RecNo,
                         PrefetchingIteratorImpl::DEFAULT_BUFFER_LENGTH, false);
  }
  return 0;
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

  DbTxn* tid = !impl->isTemporaryFile ?
	          SmiEnvironment::instance.impl->usrTxn : 0;

  rc = impl->bdbFile->put( tid, &key, &data, DB_APPEND );
  SmiEnvironment::SetBDBError( rc );

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
  }
  else
  {
    record.initialized     = false;
  }

  return (record.initialized);
}

bool SmiRecordFile::DeleteRecord( SmiRecordId recno )
{
  int rc = 0;
  Dbt key( &recno, sizeof( recno ) );
  DbTxn* tid = !impl->isTemporaryFile ?
	                SmiEnvironment::instance.impl->usrTxn : 0;

  rc = impl->bdbFile->del( tid, &key, 0 );
  SmiEnvironment::SetBDBError( rc );

  return (rc == 0);
}

SmiStatResultType
  SmiRecordFile::GetFileStatistics(const SMI_STATS_MODE mode)
{ int getStatReturnValue = 0;
  u_int32_t flags = 0;
  DB_BTREE_STAT *sRS = 0;
  SmiStatResultType result;
  // set flags according to ~mode~
  switch(mode){
    case SMI_STATS_LAZY: {
        flags = DB_FAST_STAT;
        break;
      }
    case SMI_STATS_EAGER: {
        flags = 0;
        break;
      }
    default: {
        cout << "Error in SmiRecordFile::GetFileStatistics: Unknown "
             << "SMI_STATS_MODE" << mode << endl;
//         assert( false );
      }
  }
  // call bdb stats method
#if DB_VERSION_MAJOR >= 4
#if DB_VERSION_MINOR >= 3
  getStatReturnValue = impl->bdbFile->stat(0, &sRS, flags);
#endif
#else
  getStatReturnValue = impl->bdbFile->stat(0, &sRS, flags);
#endif
  // check for errors
  if(getStatReturnValue != 0){
    cout << "Error in SmiRecordFile::GetFileStatistics: stat(...) returned != 0"
         << getStatReturnValue << endl;
    string error;
    SmiEnvironment::GetLastErrorCode( error );
    cout << error << endl;
//     assert( false );
    return result;
  }
  // translate result structure to vector<pair<string,string> >
  result.push_back(pair<string,string>("FileName",fileName));
  result.push_back(pair<string,string>("FileType","RecnoFile"));
  result.push_back(pair<string,string>("StatisticsMode",
        (mode == SMI_STATS_LAZY) ? "Lazy" : "Eager" ));
  result.push_back(pair<string,string>("FileTypeVersion",
      lu_2_s(sRS->bt_version)));
  result.push_back(pair<string,string>("NoRecords",lu_2_s(sRS->bt_nkeys)));
  result.push_back(pair<string,string>("NoUndeletedRecords",
      lu_2_s(sRS->bt_ndata)));
  result.push_back(pair<string,string>("PageSize",lu_2_s(sRS->bt_pagesize)));
  result.push_back(pair<string,string>("MinKeyPerPage",lu_2_s(sRS->bt_minkey)));
  result.push_back(pair<string,string>("RecordLength",lu_2_s(sRS->bt_re_len)));
  result.push_back(pair<string,string>("PaddingByte",lu_2_s(sRS->bt_re_pad)));
  result.push_back(pair<string,string>("NoLevels",lu_2_s(sRS->bt_levels)));
  result.push_back(pair<string,string>("NoInternalPages",
      lu_2_s(sRS->bt_int_pg)));
  result.push_back(pair<string,string>("NoLeafPages",lu_2_s(sRS->bt_leaf_pg)));
  result.push_back(pair<string,string>("NoDuplicatePages",
      lu_2_s(sRS->bt_dup_pg)));
  result.push_back(pair<string,string>("NoOverflowPages",
      lu_2_s(sRS->bt_over_pg)));
  result.push_back(pair<string,string>("NoFreeListPages",lu_2_s(sRS->bt_free)));
  result.push_back(pair<string,string>("NoBytesFreeInternalPages",
      lu_2_s(sRS->bt_int_pgfree)));
  result.push_back(pair<string,string>("NoBytesFreeLeafPages",
      lu_2_s(sRS->bt_leaf_pgfree)));
  result.push_back(pair<string,string>("NoBytesFreeDuplicatePages",
      lu_2_s(sRS->bt_dup_pgfree)));
  result.push_back(pair<string,string>("NoBytesFreeOverflowPages",
      lu_2_s(sRS->bt_over_pgfree)));


  free(sRS); // free result structure
  return result;
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

