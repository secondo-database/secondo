/*

1 Implementation of SmiKeyedFile using the Oracle-DB 

April 2002 Ulrich Telle

*/

#include <string>
#include <algorithm>
#include <cctype>

#include "SecondoSMI.h"
#include "SmiORA.h"
#include "SmiCodes.h"

using namespace std;
using namespace OCICPP;

/* --- Implementation of class SmiKeyedFile --- */

SmiKeyedFile::SmiKeyedFile( const SmiKey::KeyDataType keyType,
                            const bool hasUniqueKeys = true )
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
                            const SmiFile::AccessType accessType =
                                  SmiFile::ReadOnly )
{
  bool ok = false;
  Connection& con = SmiEnvironment::instance.impl->usrConnection;

  try
  {
    iterator.Finish();
    iterator.firstKey = key;
    string sql = "SELECT RKEY, RDATA FROM " + impl->oraTableName +
                 " WHERE RKEY = :rk";
    if ( accessType == SmiFile::Update )
    {
      sql += " FOR UPDATE";
    }
    con.prepare( sql, iterator.impl->oraCursor );
    if ( !SmiFile::Implementation::BindKeyToCursor(
                   iterator.firstKey.keyType,
                   iterator.firstKey.GetAddr(),
                   iterator.firstKey.keyLength,
                   ":rk", iterator.impl->oraCursor ) )
    {
      iterator.impl->oraCursor.drop();
      return (false);
    }
    iterator.smiFile          = this;
    iterator.opened           = true;
    iterator.restart          = true;
    iterator.writable         = (accessType == SmiFile::Update);
    iterator.solelyDuplicates = true;
    iterator.ignoreDuplicates = false;
    iterator.rangeSearch      = false;
    iterator.searchKey        = &iterator.firstKey;
    SmiEnvironment::SetError( E_SMI_OK );
    ok = true;
  }
  catch ( OraError &err )
  {
    SmiEnvironment::SetError( E_SMI_RECORD_SELECT, err.message );
    iterator.impl->oraCursor.drop();
    ok = false;
  }
  return (ok);
}

bool
SmiKeyedFile::SelectRecord( const SmiKey& key,
                            SmiRecord& record,
                            const SmiFile::AccessType accessType =
                                  SmiFile::ReadOnly )
{
  bool ok = false;
  Connection& con = SmiEnvironment::instance.impl->usrConnection;

  try
  {
    record.Finish();
    string sql = "SELECT RKEY, RDATA, ROWID FROM " + impl->oraTableName +
                 " WHERE RKEY = :rk";
    if ( accessType == SmiFile::Update )
    {
      sql += " FOR UPDATE";
    }
    con.prepare( sql, record.impl->oraCursor );
    if ( !SmiFile::Implementation::BindKeyToCursor(
                   key.keyType, key.GetAddr(), key.keyLength,
                   ":rk", record.impl->oraCursor ) )
    {
      SmiEnvironment::SetError( E_SMI_RECORD_SELECT );
      record.impl->oraCursor.drop();
      return (false);
    }
    record.impl->oraCursor.execute();
    if ( !record.impl->oraCursor.fetch() )
    {
      SmiEnvironment::SetError( E_SMI_RECORD_SELECT );
      record.impl->oraCursor.drop();
      return (false);
    }
    record.impl->oraCursor.getBLOB( "RDATA", record.impl->oraLob );
    record.recordKey         = key;
    record.recordSize        = record.impl->oraLob.getLen();
    record.fixedSize         = false;
    record.writable          = (uniqueKeys && accessType == SmiFile::Update);
    record.smiFile           = this;
    record.impl->closeCursor = true;
    record.initialized       = true;
    SmiEnvironment::SetError( E_SMI_OK );
    ok = true;
  }
  catch ( OraError &err )
  {
    SmiEnvironment::SetError( E_SMI_RECORD_SELECT, err.message );
    record.initialized = false;
    record.impl->oraCursor.drop();
  }
  return (record.initialized);
}

bool
SmiKeyedFile::SelectRange( const SmiKey& fromKey, 
                           const SmiKey& toKey, 
                           SmiKeyedFileIterator& iterator,
                           const SmiFile::AccessType accessType =
                                 SmiFile::ReadOnly,
                           const bool reportDuplicates = false )
{
  bool ok = false;
  Connection& con = SmiEnvironment::instance.impl->usrConnection;

  try
  {
    iterator.Finish();
    iterator.firstKey = fromKey;
    iterator.lastKey  = toKey;
    string sql = "SELECT RKEY, RDATA, ROWID FROM " + impl->oraTableName +
                 " WHERE RKEY >= :rk1 AND RKEY <= :rk2 ORDER BY RKEY";
    if ( accessType == SmiFile::Update )
    {
      sql += " FOR UPDATE";
    }
    con.prepare( sql, iterator.impl->oraCursor );
    if ( !SmiFile::Implementation::BindKeyToCursor(
                   iterator.firstKey.keyType,
                   iterator.firstKey.GetAddr(),
                   iterator.firstKey.keyLength,
                   ":rk1", iterator.impl->oraCursor ) )
    {
      iterator.impl->oraCursor.drop();
      return (false);
    }
    if ( !SmiFile::Implementation::BindKeyToCursor(
                   iterator.lastKey.keyType,
                   iterator.lastKey.GetAddr(),
                   iterator.lastKey.keyLength,
                   ":rk2", iterator.impl->oraCursor ) )
    {
      iterator.impl->oraCursor.drop();
      return (false);
    }
    iterator.smiFile          = this;
    iterator.opened           = true;
    iterator.restart          = true;
    iterator.writable         = (accessType == SmiFile::Update);
    iterator.solelyDuplicates = false;
    iterator.ignoreDuplicates = !reportDuplicates;
    iterator.rangeSearch      = true;
    iterator.searchKey        = &iterator.firstKey;
    SmiEnvironment::SetError( E_SMI_OK );
    ok = true;
  }
  catch ( OraError &err )
  {
    SmiEnvironment::SetError( E_SMI_RECORD_SELECT, err.message );
    iterator.impl->oraCursor.drop();
    ok = false;
  }
  return (ok);
}

bool
SmiKeyedFile::SelectLeftRange( const SmiKey& toKey, 
                               SmiKeyedFileIterator& iterator,
                               const SmiFile::AccessType accessType =
                                     SmiFile::ReadOnly,
                               const bool reportDuplicates = false )
{
  bool ok = false;
  Connection& con = SmiEnvironment::instance.impl->usrConnection;

  try
  {
    iterator.Finish();
    iterator.lastKey = toKey;
    string sql = "SELECT RKEY, RDATA, ROWID FROM " + impl->oraTableName +
                 " WHERE RKEY <= :rk ORDER BY RKEY";
    if ( accessType == SmiFile::Update )
    {
      sql += " FOR UPDATE";
    }
    con.prepare( sql, iterator.impl->oraCursor );
    if ( !SmiFile::Implementation::BindKeyToCursor(
                   iterator.lastKey.keyType,
                   iterator.lastKey.GetAddr(),
                   iterator.lastKey.keyLength,
                   ":rk", iterator.impl->oraCursor ) )
    {
      iterator.impl->oraCursor.drop();
      return (false);
    }
    iterator.smiFile          = this;
    iterator.opened           = true;
    iterator.restart          = true;
    iterator.writable         = (accessType == SmiFile::Update);
    iterator.solelyDuplicates = false;
    iterator.ignoreDuplicates = !reportDuplicates;
    iterator.rangeSearch      = false;
    SmiKey dummy( toKey.mapFunc );
    iterator.firstKey         = dummy;
    iterator.searchKey        = &iterator.firstKey;
    SmiEnvironment::SetError( E_SMI_OK );
    ok = true;
  }
  catch ( OraError &err )
  {
    SmiEnvironment::SetError( E_SMI_RECORD_SELECT, err.message );
    iterator.impl->oraCursor.drop();
    ok = false;
  }
  return (ok);
}

bool
SmiKeyedFile::SelectRightRange( const SmiKey& fromKey, 
                                SmiKeyedFileIterator& iterator,
                                const SmiFile::AccessType accessType =
                                      SmiFile::ReadOnly,
                                const bool reportDuplicates = false )
{
  bool ok = false;
  Connection& con = SmiEnvironment::instance.impl->usrConnection;

  try
  {
    iterator.Finish();
    iterator.firstKey = fromKey;
    string sql = "SELECT RKEY, RDATA, ROWID FROM " + impl->oraTableName +
                 " WHERE RKEY >= :rk ORDER BY RKEY";
    if ( accessType == SmiFile::Update )
    {
      sql += " FOR UPDATE";
    }
    con.prepare( sql, iterator.impl->oraCursor );
    if ( !SmiFile::Implementation::BindKeyToCursor(
                   iterator.firstKey.keyType,
                   iterator.firstKey.GetAddr(),
                   iterator.firstKey.keyLength,
                   ":rk", iterator.impl->oraCursor ) )
    {
      iterator.impl->oraCursor.drop();
      return (false);
    }
    iterator.smiFile          = this;
    iterator.opened           = true;
    iterator.restart          = true;
    iterator.writable         = (accessType == SmiFile::Update);
    iterator.solelyDuplicates = false;
    iterator.ignoreDuplicates = !reportDuplicates;
    iterator.rangeSearch      = true;
    SmiKey dummy( fromKey.mapFunc );
    iterator.lastKey          = dummy;
    iterator.searchKey        = &iterator.firstKey;
    SmiEnvironment::SetError( E_SMI_OK );
    ok = true;
  }
  catch ( OraError &err )
  {
    SmiEnvironment::SetError( E_SMI_RECORD_SELECT, err.message );
    iterator.impl->oraCursor.drop();
    ok = false;
  }
  return (ok);
}

bool
SmiKeyedFile::SelectAll( SmiKeyedFileIterator& iterator,
                         const SmiFile::AccessType accessType =
                               SmiFile::ReadOnly,
                         const bool reportDuplicates = false )
{
  bool ok = false;
  Connection& con = SmiEnvironment::instance.impl->usrConnection;

  try
  {
    iterator.Finish();
    string sql = "SELECT RKEY, RDATA, ROWID FROM " + impl->oraTableName +
                 " ORDER BY RKEY";
    if ( accessType == SmiFile::Update )
    {
      sql += " FOR UPDATE";
    }
    con.prepare( sql, iterator.impl->oraCursor );
    iterator.smiFile          = this;
    iterator.opened           = true;
    iterator.restart          = true;
    iterator.writable         = (accessType == SmiFile::Update);
    iterator.solelyDuplicates = false;
    iterator.ignoreDuplicates = !reportDuplicates;
    iterator.rangeSearch      = false;
    SmiKey dummy;
    iterator.firstKey         = dummy;
    iterator.lastKey          = dummy;
    iterator.searchKey        = &iterator.firstKey;
    SmiEnvironment::SetError( E_SMI_OK );
    ok = true;
  }
  catch ( OraError &err )
  {
    SmiEnvironment::SetError( E_SMI_RECORD_SELECT, err.message );
    iterator.impl->oraCursor.drop();
    ok = false;
  }
  return (ok);
}

bool
SmiKeyedFile::InsertRecord( const SmiKey& key, SmiRecord& record )
{
  bool ok = false;
  Connection& con = SmiEnvironment::instance.impl->usrConnection;

  try
  {
    int rowSeq = impl->GetSeqId( con );
    record.Finish();
    string sql = "INSERT INTO " + impl->oraTableName +
                 " VALUES( :rk, :rs, EMPTY_BLOB() )";
    con.prepare( sql, record.impl->oraCursor );
    if ( !SmiFile::Implementation::BindKeyToCursor(
                   key.keyType, key.GetAddr(), key.keyLength,
                   ":rk", record.impl->oraCursor ) )
    {
      record.impl->oraCursor.drop();
      return (false);
    }
    record.impl->oraCursor.bind( ":rs", rowSeq );
    record.impl->oraCursor.execute();
    sql = "SELECT RKEY, RDATA FROM " + impl->oraTableName +
          " WHERE RKEY = :rk AND RSEQ = :rs FOR UPDATE";
    record.impl->oraCursor.drop();
    con.prepare( sql, record.impl->oraCursor );
    if ( !SmiFile::Implementation::BindKeyToCursor(
                   key.keyType, key.GetAddr(), key.keyLength,
                   ":rk", record.impl->oraCursor ) )
    {
      record.impl->oraCursor.drop();
      return (false);
    }
    record.impl->oraCursor.bind( ":rs", rowSeq );
    record.impl->oraCursor.execute();
    if ( !record.impl->oraCursor.fetch() )
    {
      SmiEnvironment::SetError( E_SMI_RECORD_SELECT );
      record.impl->oraCursor.drop();
      return (false);
    }
    record.impl->oraCursor.getBLOB( "RDATA", record.impl->oraLob );
    record.recordKey         = key;
    record.recordSize        = 0;
    record.fixedSize         = false;
    record.writable          = true;
    record.smiFile           = this;
    record.impl->closeCursor = true;
    record.initialized       = true;
    SmiEnvironment::SetError( E_SMI_OK );
    ok = true;
  }
  catch ( OraError &err )
  {
    if ( err.message.find( "ORA-00001" ) != string::npos )
    {
      SmiEnvironment::SetError( E_SMI_FILE_KEYEXIST );
    }
    else
    {
      SmiEnvironment::SetError( E_SMI_RECORD_INSERT, err.message );
    }
    record.impl->oraCursor.drop();
    ok = false;
  }
  return (ok);
}
  
bool
SmiKeyedFile::DeleteRecord( const SmiKey& key )
{
  bool ok = false;
  Connection& con = SmiEnvironment::instance.impl->usrConnection;

  try
  {
    string sql = "DELETE FROM " + impl->oraTableName + " WHERE RKEY = :rk";
    Cursor csr;
    con.prepare( sql, csr );
    if ( SmiFile::Implementation::BindKeyToCursor(
                  key.keyType, key.GetAddr(), key.keyLength,
                  ":rk", csr ) )
    {
      csr.execute();
      SmiEnvironment::SetError( E_SMI_OK );
      ok = true;
    }
    else
    {
      SmiEnvironment::SetError( E_SMI_RECORD_DELETE );
      ok = false;
    }
  }
  catch ( OraError &err )
  {
    SmiEnvironment::SetError( E_SMI_RECORD_DELETE, err.message );
    ok = false;
  }
  return (ok);
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
  }
  return (ok);
}

bool
SmiKeyedFileIterator::Next( SmiRecord& record )
{
  bool ok = SmiFileIterator::Next( record );
  return (ok);
}
  
/* --- oraKeyedFile.cpp --- */

