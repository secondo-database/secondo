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

1 Implementation of SmiBtreeFile using the Berkeley-DB

April 2002 Ulrich Telle

September 2002 Ulrich Telle, fixed flag (DB\_DIRTY\_READ) in Berkeley DB calls for system catalog files

April 2003 Ulrich Telle, implemented temporary SmiFiles

May 2008, Victor Almeida created the two sons of the ~SmiKeyedFile~ class, namely ~SmiBtreeFile~ and
~SmiHashFile~, for B-Tree and hash access methods, respectively.

*/

using namespace std;

//#define TRACE_ON 1
#undef TRACE_ON
#include "LogMsg.h"

#include <string>
#include <algorithm>
#include <cctype>
#include <cassert>

#include <db_cxx.h>
#include "SecondoSMI.h"
#include "SmiBDB.h"
#include "SmiCodes.h"
#include "Profiles.h"

extern string lu_2_s(uint32_t value); // defined in bdbFile.cpp

/* --- Implementation of class SmiBtreeFile --- */

SmiBtreeFile::SmiBtreeFile( const SmiKey::KeyDataType keyType,
                            const bool hasUniqueKeys /* = true */,
                            const bool isTemporary /* = false */ )
  : SmiKeyedFile( KeyedBtree, keyType, hasUniqueKeys, isTemporary )
{
  useTxn = SmiEnvironment::useTransactions;
}

SmiBtreeFile::~SmiBtreeFile()
{
}

bool
SmiBtreeFile::SelectRange( const SmiKey& fromKey,
                           const SmiKey& toKey,
                           SmiKeyedFileIterator& iterator,
                           const SmiFile::AccessType accessType
                             /* = SmiFile::ReadOnly */,
                           const bool reportDuplicates /* = false */ )
{
  int rc = 0;
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
    iterator.solelyDuplicates = false;
    iterator.ignoreDuplicates = !reportDuplicates;
    iterator.rangeSearch      = true;
    iterator.firstKey         = fromKey;
    iterator.lastKey          = toKey;
    iterator.searchKey        = &iterator.firstKey;
  }

  if (rc != DB_NOTFOUND)
    SmiEnvironment::SetBDBError( rc );

  return (rc == 0);
}

PrefetchingIterator*
SmiBtreeFile::SelectRangePrefetched(const SmiKey& fromKey, const SmiKey& toKey)
{
  int rc = 0;
  Dbc* dbc = 0;
  DbTxn* tid = !impl->isTemporaryFile ?
	                SmiEnvironment::instance.impl->usrTxn : 0;

  rc = impl->bdbFile->cursor(tid, &dbc, 0);
  if(rc == 0)
  {
    return new PrefetchingIteratorImpl(dbc, keyDataType,
      (const char*)fromKey.GetAddr(), fromKey.keyLength,
      (const char*)toKey.GetAddr(), toKey.keyLength,
      PrefetchingIteratorImpl::DEFAULT_BUFFER_LENGTH);
  }
  else
  {
    SmiEnvironment::SetBDBError(rc);
    return 0;
  }
}

bool
SmiBtreeFile::SelectLeftRange( const SmiKey& toKey,
                               SmiKeyedFileIterator& iterator,
                               const SmiFile::AccessType accessType
                                 /* = SmiFile::ReadOnly */,
                               const bool reportDuplicates /* = false */ )
{
  int rc = 0;
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
    iterator.solelyDuplicates = false;
    iterator.ignoreDuplicates = !reportDuplicates;
    iterator.rangeSearch      = false;
    iterator.firstKey         = SmiKey();
    iterator.lastKey          = toKey;
    iterator.searchKey        = &iterator.firstKey;
  }
  else
  {
    SmiEnvironment::SetBDBError( rc );
  }
  return (rc == 0);
}

PrefetchingIterator*
SmiBtreeFile::SelectLeftRangePrefetched(const SmiKey& toKey)
{
  int rc = 0;
  Dbc* dbc = 0;
  DbTxn* tid = !impl->isTemporaryFile ?
	                SmiEnvironment::instance.impl->usrTxn : 0;
  rc = impl->bdbFile->cursor(tid, &dbc, 0);
  if(rc == 0)
  {
    return new PrefetchingIteratorImpl(dbc, keyDataType,
      0, 0, (const char*)toKey.GetAddr(), toKey.keyLength,
      PrefetchingIteratorImpl::DEFAULT_BUFFER_LENGTH);
  }
  else
  {
    SmiEnvironment::SetBDBError(rc);
    return 0;
  }
}


bool
SmiBtreeFile::SelectRightRange( const SmiKey& fromKey,
                                SmiKeyedFileIterator& iterator,
                                const SmiFile::AccessType accessType
                                  /* = SmiFile::ReadOnly */,
                                const bool reportDuplicates /* = false */ )
{
  int rc = 0;
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
    iterator.solelyDuplicates = false;
    iterator.ignoreDuplicates = !reportDuplicates;
    iterator.rangeSearch      = true;
    iterator.firstKey         = fromKey;
    iterator.lastKey          = SmiKey();
    iterator.searchKey        = &iterator.firstKey;
  }
  else
  {
    SmiEnvironment::SetBDBError( rc );
  }
  return (rc == 0);
}

PrefetchingIterator*
SmiBtreeFile::SelectRightRangePrefetched(const SmiKey& fromKey)
{
  int rc = 0;
  Dbc* dbc = 0;
  DbTxn* tid = !impl->isTemporaryFile ?
	                SmiEnvironment::instance.impl->usrTxn : 0;

  rc = impl->bdbFile->cursor(tid, &dbc, 0);
  if(rc == 0)
  {
    return new PrefetchingIteratorImpl(dbc, keyDataType,
      (const char*)fromKey.GetAddr(), fromKey.keyLength, 0, 0,
      PrefetchingIteratorImpl::DEFAULT_BUFFER_LENGTH);
  }
  else
  {
    SmiEnvironment::SetBDBError(rc);
    return 0;
  }
}

bool
SmiBtreeFile::SelectAll( SmiKeyedFileIterator& iterator,
                         const SmiFile::AccessType accessType
                           /* = SmiFile::ReadOnly */,
                         const bool reportDuplicates /* = false */ )
{
  TRACE_ENTER

  int rc = 0;
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
    iterator.solelyDuplicates = false;
    iterator.ignoreDuplicates = !reportDuplicates;
    iterator.rangeSearch      = false;

    SmiKey dummy;
    iterator.firstKey         = dummy;
    iterator.lastKey          = dummy;
    iterator.searchKey        = &iterator.firstKey;
  }
  else
  {
    SmiEnvironment::SetBDBError( rc );
  }

  SHOW(iterator.opened);
  TRACE_LEAVE
  return (rc == 0);
}

SmiStatResultType
  SmiBtreeFile::GetFileStatistics(const SMI_STATS_MODE mode)
{
  int getStatReturnValue = 0;
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
        cout << "Error in SmiBtreeFile::GetFileStatistics: Unknown "
             << "SMI_STATS_MODE" << mode << endl;
//         assert( false );
        return result;
      }
  }
  // call bdb stats method
#if DB_VERSION_MAJOR >= 4
#if DB_VERSION_MINOR >= 3
  getStatReturnValue = impl->bdbFile->stat(0, &sRS, flags);
#endif
#else
  getStatReturnValue = impl->bdbFile->stat( &sRS, flags);
#endif
  // check for errors
  if(getStatReturnValue != 0){
    cout << "Error in SmiBtreeFile::GetFileStatistics: stat(...) returned != 0"
         << getStatReturnValue << endl;
    string error;
    SmiEnvironment::GetLastErrorCode( error );
    cout << error << endl;
//     assert( false );
    return result;
  }
  // translate result structure to vector<pair<string,string> >
  result.push_back(pair<string,string>("FileName",fileName));
  result.push_back(pair<string,string>("FileType","BtreeFile"));
  result.push_back(pair<string,string>("StatisticsMode",
            (mode == SMI_STATS_LAZY) ? "Lazy" : "Eager" ));
  result.push_back(pair<string,string>("FileTypeVersion",
            lu_2_s(sRS->bt_version)));
  result.push_back(pair<string,string>("NoUniqueKeys",lu_2_s(sRS->bt_nkeys)));
  result.push_back(pair<string,string>("NoEntries",lu_2_s(sRS->bt_ndata)));
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

PrefetchingIterator* SmiBtreeFile::SelectAllPrefetched()
{
  int rc = 0;
  Dbc* dbc = 0;
  DbTxn* tid = !impl->isTemporaryFile ?
	                 SmiEnvironment::instance.impl->usrTxn : 0;

  rc = impl->bdbFile->cursor(tid, &dbc, 0);
  if(rc == 0)
  {
    return new PrefetchingIteratorImpl(dbc, keyDataType,
      PrefetchingIteratorImpl::DEFAULT_BUFFER_LENGTH, true);
  }
  else
  {
    SmiEnvironment::SetBDBError(rc);
    return 0;
  }
}
/* --- bdbBtreeFile.cpp --- */

