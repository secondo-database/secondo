/*

1 Implementation of SmiFile using the Oracle-DB 

April 2002 Ulrich Telle

*/

#include <string>
#include <algorithm>
#include <cctype>
#include <sstream>
using namespace std;

#include "SecondoSMI.h"
#include "SmiORA.h"
#include "SmiCodes.h"
#include "Profiles.h"
using namespace OCICPP;

/* --- Implementation of class SmiFile --- */

SmiFile::Implementation::Implementation()
{
}

SmiFile::Implementation::~Implementation()
{
}

bool
SmiFile::Implementation::BindKeyToCursor( const SmiKey::KeyDataType keyType,
                                          const void* keyAddr,
                                          const int keyLength,
                                          const string& bindname,
                                          Cursor& csr )
{
  bool ok = true;
  switch (keyType)
  { 
    case SmiKey::RecNo:
    case SmiKey::Integer:
    {
      csr.bind( bindname, *((int*) keyAddr) );
      break;
    }
    case SmiKey::Float:
    {
      csr.bind( bindname, *((double*) keyAddr) );
      break;
    }
    case SmiKey::String:
    case SmiKey::Composite:
    {
      csr.bind( bindname, (char*) keyAddr, keyLength+1 );
      break;
    }
    default:
    {
      ok = false;
      break;
    }
  }
  return (ok);
}

int
SmiFile::Implementation::GetSeqId( Connection& con )
{
  int newSeqId = 0;

  try
  {
    string sql = "SELECT " + oraSeqName + ".NEXTVAL FROM DUAL";
    Cursor csr;
    con.execQuery( sql, csr );
    if ( csr.fetch() )
    {
      newSeqId = csr.getInt( 0 );
    }
    SmiEnvironment::SetError( E_SMI_OK );
  }
  catch ( OraError &err )
  {
    SmiEnvironment::SetError( E_SMI_FILE_NOFILEID, err.message );
  }

  return (newSeqId);
}

SmiFile::SmiFile()
  : opened( false ), fileContext( "" ), fileName( "" ), fileId( 0 ),
    fixedRecordLength( 0 ), uniqueKeys( true ), keyDataType( SmiKey::Unknown )
{
  impl = new Implementation();
}

SmiFile::~SmiFile()
{
  delete impl;
}

bool
SmiFile::CheckName( const string& name )
{
  static string alpha( "abcdefghijklmnopqrstuvwxyz" );
  static string alnum( alpha + "0123456789_" );
  string temp = name;
  bool ok = false;

  if ( temp.length() > 0 )
  {
    transform( temp.begin(), temp.end(), temp.begin(), tolower );
    string::size_type pos = temp.find_first_not_of( alnum );
    ok = pos == string::npos &&
         name[0] != '_' &&
         name.length() < SMI_MAX_NAMELEN;
  }

  return ok;
}

bool
SmiFile::Create( const string& context /* = "Default" */ )
{
  bool ok = false;
  Connection& con = SmiEnvironment::instance.impl->sysConnection;

  if ( CheckName( context ) )
  {
    fileId = SmiEnvironment::Implementation::GetFileId();
    if ( fileId != 0 )
    {
      impl->oraTableName =
        SmiEnvironment::Implementation::ConstructTableName( fileId );
      impl->oraSeqName =
        SmiEnvironment::Implementation::ConstructSeqName( fileId );
      impl->oraIndexName =
        SmiEnvironment::Implementation::ConstructIndexName( fileId );
      try
      {
        string sql = "CREATE TABLE " + impl->oraTableName + " ( RKEY";
        if ( fileType == Keyed )
        {
          if ( keyDataType == SmiKey::Integer )
          {
            sql += " NUMBER(10) NOT NULL";
          }
          else if ( keyDataType == SmiKey::Float )
          {
            sql += " NUMBER NOT NULL";
          }
          else
          {
            ostringstream osKeyLen;
            osKeyLen << SMI_MAX_KEYLEN;
            sql += " VARCHAR2(" + osKeyLen.str() + ") NOT NULL";
          }
          if ( uniqueKeys )
          {
            sql += " PRIMARY KEY";
          }
          sql += ", RSEQ NUMBER(10) NOT NULL";
        }
        else // SmiRecordFile
        {
          sql += " NUMBER(10) NOT NULL PRIMARY KEY";
        }
        sql += ", RDATA BLOB ) ";
        sql += SmiProfile::GetParameter( context, "OraTableSpace", "",
                                         SmiEnvironment::configFile );
        con.execUpdate( sql );
        if ( fileType == Keyed )
        {
          if ( !uniqueKeys )
          {
            sql = "CREATE INDEX " + impl->oraIndexName +
                  " ON " + impl->oraTableName + " (RKEY ASC) ";
            sql += SmiProfile::GetParameter( context, "OraIndexSpace", "",
                                             SmiEnvironment::configFile );
            con.execUpdate( sql );
          }
        }
        sql = "CREATE SEQUENCE " + impl->oraSeqName +
              " START WITH 1 INCREMENT BY 1 NOCACHE";
        con.execUpdate( sql );
        SmiDropFilesEntry entry;
        entry.fileId = fileId;
        entry.dropOnCommit = false;
        SmiEnvironment::instance.impl->oraFilesToDrop.push( entry );
        SmiEnvironment::SetError( E_SMI_OK );
        fileContext = context;
        fileName    = "";
        opened = true;
        ok = true;
      }
      catch ( OraError &err )
      {
        SmiEnvironment::SetError( E_SMI_FILE_CREATE, err.message );
      }
    }
    else
    {
      SmiEnvironment::SetError( E_SMI_FILE_NOFILEID );
    }
  }
  else
  {
    SmiEnvironment::SetError( E_SMI_FILE_INVALIDNAME );
  }
  return (ok);
}

bool
SmiFile::Open( const string& name, const string& context /* = "Default" */ )
{
  bool ok = false;
  bool existing = false;
  Connection& con = SmiEnvironment::instance.impl->sysConnection;

  if ( CheckName( context ) && CheckName( name ) )
  {
    SmiCatalogEntry entry;
    string newName = context + '.' + name;
    if ( SmiEnvironment::Implementation::LookUpCatalog( newName, entry ) )
    {
      // --- File found in permanent file catalog
      fileId = entry.fileId;
      existing = true;
    }
    else
    {
      // --- Check whether a file with the given name was created
      // --- earlier within the enclosing transaction
      map<string,SmiCatalogFilesEntry>::iterator it = 
        SmiEnvironment::instance.impl->oraFilesToCatalog.find( newName );
      if ( it != SmiEnvironment::instance.impl->oraFilesToCatalog.end() &&
           it->second.updateOnCommit )
      {
        fileId = it->second.entry.fileId;
        existing = true;
      }
      else
      {
        fileId = SmiEnvironment::Implementation::GetFileId();
      }
    }

    if ( fileId != 0 )
    {
      impl->oraTableName =
        SmiEnvironment::Implementation::ConstructTableName( fileId );
      impl->oraSeqName =
        SmiEnvironment::Implementation::ConstructSeqName( fileId );
      impl->oraIndexName =
        SmiEnvironment::Implementation::ConstructIndexName( fileId );
      if ( !existing )
      {
        try
        {
          string sql = "CREATE TABLE " + impl->oraTableName + " ( RKEY";
          if ( fileType == Keyed )
          {
            if ( keyDataType == SmiKey::Integer )
            {
              sql += " NUMBER(10) NOT NULL";
            }
            else if ( keyDataType == SmiKey::Float )
            {
              sql += " NUMBER NOT NULL";
            }
            else
            {
              ostringstream osKeyLen;
              osKeyLen << SMI_MAX_KEYLEN;
              sql += " VARCHAR2(" + osKeyLen.str() + ") NOT NULL";
            }
            if ( uniqueKeys )
            {
              sql += " PRIMARY KEY";
            }
            sql += ", RSEQ NUMBER(10) NOT NULL";
          }
          else // SmiRecordFile
          {
            sql += " NUMBER(10) NOT NULL PRIMARY KEY";
          }
          sql += ", RDATA BLOB ) ";
          sql += SmiProfile::GetParameter( context, "OraTableSpace", "",
                                           SmiEnvironment::configFile );
          con.execUpdate( sql );
          if ( fileType == Keyed )
          {
            if ( !uniqueKeys )
            {
              sql = "CREATE INDEX " + impl->oraIndexName +
                    " ON " + impl->oraTableName + " (RKEY ASC) ";
              sql += SmiProfile::GetParameter( context, "OraIndexSpace", "",
                                               SmiEnvironment::configFile );
              con.execUpdate( sql );
            }
          }
          sql = "CREATE SEQUENCE " + impl->oraSeqName +
                " START WITH 1 INCREMENT BY 1 NOCACHE";
          con.execUpdate( sql );
          // --- Register newly created file, since it has to be dropped
          // --- when the enclosing transaction is aborted
          SmiDropFilesEntry dropEntry;
          dropEntry.fileId = fileId;
          dropEntry.dropOnCommit = false;
          SmiEnvironment::instance.impl->oraFilesToDrop.push( dropEntry );
          SmiCatalogFilesEntry catalogEntry;
          catalogEntry.entry.fileId   = fileId;
          catalogEntry.entry.fileName = newName;
          catalogEntry.entry.isKeyed  = (fileType == Keyed);
          catalogEntry.entry.isFixed  = (fileType == FixedLength);
          catalogEntry.updateOnCommit = true;
          SmiEnvironment::instance.impl->oraFilesToCatalog[newName] = catalogEntry;
          SmiEnvironment::SetError( E_SMI_OK );
          fileContext = context;
          fileName    = name;
          opened      = true;
          ok = true;
        }
        catch ( OraError &err )
        {
          SmiEnvironment::SetError( E_SMI_FILE_CREATE, err.message );
        }
      }
      else
      {
        SmiEnvironment::SetError( E_SMI_OK );
        fileContext = context;
        fileName    = name;
        opened      = true;
        ok = true;
      }
    }
    else
    {
      SmiEnvironment::SetError( E_SMI_FILE_NOFILEID );
    }
  }
  else
  {
    SmiEnvironment::SetError( E_SMI_FILE_INVALIDNAME );
  }
  return (ok);
}

bool
SmiFile::Open( const SmiFileId fileId, const string& context /* = "Default" */ )
{
  int rc = 0;

  if ( CheckName( context ) )
  {
    SmiCatalogEntry entry;
    if ( SmiEnvironment::Implementation::LookUpCatalog( fileId, entry ) )
    {
      // --- File found in permanent file catalog
      string::size_type point = entry.fileName.find( "." );
      fileContext = entry.fileName.substr( 0, point-1 );
      fileName = entry.fileName.substr( point+1 );
    }
    else
    {
      fileContext = context;
      fileName    = "";
    }
    if ( fileId != 0 && fileContext == context )
    {
      impl->oraTableName =
        SmiEnvironment::Implementation::ConstructTableName( fileId );
      impl->oraSeqName =
        SmiEnvironment::Implementation::ConstructSeqName( fileId );
      impl->oraIndexName =
        SmiEnvironment::Implementation::ConstructIndexName( fileId );
      SmiEnvironment::SetError( E_SMI_OK );
      opened = true;
    }
    else
    {
      if ( fileId == 0 )
        rc = E_SMI_FILE_NOFILEID;
      else
        rc = E_SMI_FILE_BADCONTEXT;
      SmiEnvironment::SetError( rc );
    }
  }
  else
  {
    rc = E_SMI_FILE_INVALIDNAME;
    SmiEnvironment::SetError( E_SMI_FILE_INVALIDNAME );
  }

  return (rc == 0);
}

bool
SmiFile::Close()
{
  opened = false;
  SmiEnvironment::SetError( E_SMI_OK );
  return (true);
}

bool
SmiFile::Drop()
{
  bool ok = Close();
  if ( ok )
  {
    // --- Register SmiFile for real dropping after 
    // --- successfully committing the enclosing transaction
    SmiDropFilesEntry dropEntry;
    dropEntry.fileId = fileId;
    dropEntry.dropOnCommit = true;
    SmiEnvironment::instance.impl->oraFilesToDrop.push( dropEntry );
    SmiCatalogFilesEntry catalogEntry;
    if ( fileName.length() > 0 )
    {
      string newName = fileContext + '.' + fileName;
      catalogEntry.entry.fileId   = fileId;
      catalogEntry.entry.fileName = newName;
      catalogEntry.entry.isKeyed  = fileType == Keyed;
      catalogEntry.entry.isFixed  = fileType == FixedLength;
      catalogEntry.updateOnCommit = false;
      SmiEnvironment::instance.impl->oraFilesToCatalog[newName] = catalogEntry;
    }
    SmiEnvironment::SetError( E_SMI_OK );
  }
  return (ok);
}

string
SmiFile::GetContext()
{
  return (fileContext);
}

string
SmiFile::GetName()
{
  return (fileName);
}

SmiFileId
SmiFile::GetFileId()
{
  return fileId;
}

bool
SmiFile::IsOpen()
{
  return opened;
}

// --- Implementation of class SmiFileIterator ---

SmiFileIterator::Implementation::Implementation()
  : oraCursor()
{
}

SmiFileIterator::Implementation::~Implementation()
{
}

SmiFileIterator::SmiFileIterator()
  : solelyDuplicates( false ), ignoreDuplicates( false ),
    smiFile( 0 ), endOfScan( true ), opened( false), writable( false ),
    restart( true ), rangeSearch( false ), searchKey( 0 )
{
  impl = new Implementation();
}

SmiFileIterator::~SmiFileIterator()
{
  Finish();
  delete impl;
}

bool
SmiFileIterator::Next( SmiRecord& record )
{
//  static char keyData[SMI_MAX_KEYLEN];
  bool ok = false;
  if ( opened )
  {
    try
    {
      record.Finish();
      if ( restart )
      {
        impl->oraCursor.execute();
        restart   = false;
        endOfScan = false;
      }
      if ( impl->oraCursor.fetch() )
      {
        impl->oraCursor.getBLOB( "RDATA", record.impl->oraLob );
        switch (smiFile->keyDataType)
        {
          case SmiKey::RecNo:
          {
            record.recordKey.SetKey( (SmiRecordId) impl->oraCursor.getInt( "RKEY" ) );
            break;
          }
          case SmiKey::Integer:
          {
            record.recordKey.SetKey( (long) impl->oraCursor.getInt( "RKEY" ) );
            break;
          }
          case SmiKey::Float:
          {
            record.recordKey.SetKey( impl->oraCursor.getDouble( "RKEY" ) );
            break;
          }
          case SmiKey::String:
          {
            record.recordKey.SetKey( impl->oraCursor.getStr( "RKEY" ) );
            break;
          }
          case SmiKey::Composite:
          {
            string tempKey = impl->oraCursor.getStr( "RKEY" );
            record.recordKey.SetKey( SmiKey::Composite,
                                     (void*) tempKey.data(),
                                     (SmiSize) tempKey.length() );
            break;
          }
          default:
          {
            ok = false;
            break;
          }
        }
        record.recordSize        = record.impl->oraLob.getLen();
        record.writable          = writable;
        record.smiFile           = smiFile;
        record.impl->closeCursor = false;
        record.initialized       = true;
        SmiEnvironment::SetError( E_SMI_OK );
        ok = true;
      }
      else
      {
        endOfScan = true;
        SmiEnvironment::SetError( E_SMI_CURSOR_ENDOFSCAN );
        ok = false;
      }
    }
    catch ( OraError &err )
    {
      SmiEnvironment::SetError( E_SMI_CURSOR_NEXT, err.message );
      ok = false;
    }
//      memcpy( keyData, searchKey->GetAddr(), searchKey->keyLength );
//      key.set_size( searchKey->keyLength );
  }
  else
  {
    SmiEnvironment::SetError( E_SMI_CURSOR_NOTOPEN );
  }
  return (ok);
}

bool
SmiFileIterator::DeleteCurrent()
{
  bool ok;
  if ( opened && !restart )
  {
    Connection& con = SmiEnvironment::instance.impl->usrConnection;
    try
    {
      RowID rowid;
      impl->oraCursor.getRowID( "ROWID", rowid );
      string sql = "DELETE FROM " + smiFile->impl->oraTableName +
                   " WHERE ROWID = :rid";
      Cursor csr;
      con.prepare( sql, csr );
      csr.bind( ":rid", rowid );
      csr.execute();
      SmiEnvironment::SetError( E_SMI_OK );
      ok = true;
    }
    catch ( OraError &err )
    {
      SmiEnvironment::SetError( E_SMI_RECORD_DELETE, err.message );
      ok = false;
    }
  }
  else
  {
    ok = false;
    SmiEnvironment::SetError( E_SMI_CURSOR_NOTOPEN );
  }
  return (ok);
}

bool
SmiFileIterator::EndOfScan()
{
  SmiEnvironment::SetError( E_SMI_OK );
  return (endOfScan);
}

bool
SmiFileIterator::Finish()
{
  bool ok;
  if ( opened )
  {
    opened    = false;
    writable  = false;
    endOfScan = true;
    restart   = true;
    smiFile   = 0;
    solelyDuplicates = false;
    ignoreDuplicates = false;
    try
    {
      impl->oraCursor.drop();
      SmiEnvironment::SetError( E_SMI_OK );
      ok = true;
    }
    catch ( OraError &err )
    {
      SmiEnvironment::SetError( E_SMI_CURSOR_FINISH, err.message );
      ok = false;
    }
  }
  else
  {
    ok = false;
    SmiEnvironment::SetError( E_SMI_CURSOR_NOTOPEN );
  }
  return (ok);
}

bool
SmiFileIterator::Restart()
{
  bool ok;
  if ( opened )
  {
    restart   = true;
    endOfScan = false;
    ok        = true;
    SmiEnvironment::SetError( E_SMI_OK );
  }
  else
  {
    ok = false;
    SmiEnvironment::SetError( E_SMI_CURSOR_NOTOPEN );
  }
  return (ok);
}

/* --- oraFile.cpp --- */

