/*

1 Implementation of SmiRecordFile using the Oracle-DB

April 2002 Ulrich Telle

*/

using namespace std;
using namespace OCICPP;

#include <string>
#include <algorithm>
#include <cctype>
#include <errno.h>

#include "SecondoSMI.h"
#include "SmiORA.h"
#include "SmiCodes.h"

/* --- Implementation of class SmiRecordFile --- */

SmiRecordFile::SmiRecordFile( bool hasFixedLengthRecords,
                              SmiSize recordLength /* = 0 */ )
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
                             const SmiFile::AccessType accessType
                               /* = SmiFile::ReadOnly */ )
{
  Connection& con = SmiEnvironment::instance.impl->usrConnection;
  try
  {
    record.Finish();
    string sql = "SELECT RKEY, RDATA FROM " + impl->oraTableName +
                 " WHERE RKEY = :rk";
    if ( accessType == SmiFile::Update )
    {
      sql += " FOR UPDATE";
    }
    con.prepare( sql, record.impl->oraCursor );
    record.impl->oraCursor.bind( ":rk", (int) recno );
    record.impl->oraCursor.execute();
    if ( !record.impl->oraCursor.fetch() )
    {
      SmiEnvironment::SetError( E_SMI_RECORD_SELECT );
      record.impl->oraCursor.drop();
      return (false);
    }
    record.impl->oraCursor.getBLOB( "RDATA", record.impl->oraLob );

    record.recordKey.SetKey( recno );
    record.recordSize        = record.impl->oraLob.getLen();
    record.fixedSize         = (fileType == SmiFile::FixedLength);
    record.writable          = (accessType == SmiFile::Update);
    record.smiFile           = this;
    record.impl->closeCursor = true;
    record.initialized       = true;
    SmiEnvironment::SetError( E_SMI_OK );
  }
  catch ( OraError &err )
  {
    record.impl->oraCursor.drop();
    SmiEnvironment::SetError( E_SMI_RECORD_SELECT, err.message );
    record.initialized     = false;
  }

  return (record.initialized);
}

bool
SmiRecordFile::SelectAll( SmiRecordFileIterator& iterator,
                          const SmiFile::AccessType accessType
                            /* = SmiFile::ReadOnly */ )
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
    iterator.ignoreDuplicates = false;
    iterator.rangeSearch      = false;
    SmiEnvironment::SetError( E_SMI_OK );
    ok = true;
  }
  catch ( OraError &err )
  {
    SmiEnvironment::SetError( E_SMI_RECORD_SELECTALL, err.message );
    iterator.impl->oraCursor.drop();
    ok = false;
  }
  return (ok);
}

bool
SmiRecordFile::AppendRecord( SmiRecordId& recno, SmiRecord& record )
{
  Connection& con = SmiEnvironment::instance.impl->usrConnection;

  try
  {
    record.Finish();
    recno = (SmiRecordId) impl->GetSeqId( con );
    string sql = "INSERT INTO " + impl->oraTableName +
                 " VALUES( :rk, EMPTY_BLOB() )";
    con.prepare( sql, record.impl->oraCursor );
    record.impl->oraCursor.bind( ":rk", (int) recno );
    record.impl->oraCursor.execute();
    sql = "SELECT RKEY, RDATA FROM " + impl->oraTableName +
          " WHERE RKEY = :rk FOR UPDATE";
    record.impl->oraCursor.drop();
    con.prepare( sql, record.impl->oraCursor );
    record.impl->oraCursor.bind( ":rk", (int) recno );
    record.impl->oraCursor.execute();
    if ( !record.impl->oraCursor.fetch() )
    {
      SmiEnvironment::SetError( E_SMI_RECORD_SELECT );
      record.impl->oraCursor.drop();
      return (false);
    }
    record.impl->oraCursor.getBLOB( "RDATA", record.impl->oraLob );
    record.recordKey.SetKey( (SmiRecordId) recno );
    record.recordSize        = 0;
    record.writable          = true;
    record.fixedSize         = (fileType == SmiFile::FixedLength);
    record.smiFile           = this;
    record.impl->closeCursor = true;
    record.initialized       = true;
    SmiEnvironment::SetError( E_SMI_OK );
  }
  catch ( OraError &err )
  {
    if ( err.message.find( "ORA-00001" ) != string::npos )
    {
      SmiEnvironment::SetError( E_SMI_FILE_KEYEXIST );
    }
    else
    {
      SmiEnvironment::SetError( E_SMI_RECORD_APPEND, err.message );
    }
    record.impl->oraCursor.drop();
  }
  return (record.initialized);
}

bool SmiRecordFile::DeleteRecord( SmiRecordId recno )
{
  bool ok = false;
  Connection* con = &SmiEnvironment::instance.impl->usrConnection;

  try
  {
    string sql = "DELETE FROM " + impl->oraTableName + " WHERE RKEY = :rk";
    Cursor csr;
    con->prepare( sql, csr );
    csr.bind( ":rk", (int) recno );
    csr.execute();
    SmiEnvironment::SetError( E_SMI_OK );
    ok = true;
  }
  catch ( OraError &err )
  {
    SmiEnvironment::SetError( E_SMI_RECORD_DELETE, err.message );
    ok = false;
  }
  return (ok);
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

/* --- oraRecordFile.cpp --- */

