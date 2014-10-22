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


#undef TRACE_ON
#include "Trace.h"
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

using namespace std;

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

bool
SmiBtreeFile::KeyRange( const SmiKey& key,
                        SmiKeyRange& range ) const
{
    int rc = 0;

    Dbt bdb_key( (void *) key.GetAddr(), key.keyLength );
    DbTxn* tid = !impl->isTemporaryFile ?
                 SmiEnvironment::instance.impl->usrTxn : 0;


    DB_KEY_RANGE bdb_kr;
    rc = impl->bdbFile->key_range( tid, &bdb_key, &bdb_kr, 0 );
    SmiEnvironment::SetBDBError( rc );

    range.less = bdb_kr.less; 
    range.equal = bdb_kr.equal;
    range.greater = bdb_kr.greater; 

    return (rc == 0);
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

