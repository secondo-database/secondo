/*

1 Implementation of SmiFile using the Berkeley-DB 

April 2002 Ulrich Telle

September 2002 Ulrich Telle, mssing fileId in Open method fixed

*/

using namespace std;

#include <string>
#include <algorithm>
#include <cctype>
#include <cstring>

#include <db_cxx.h>
#include "SecondoSMI.h"
#include "SmiBDB.h"
#include "SmiCodes.h"
#include "Profiles.h"

static int  BdbCompareInteger( Db* dbp, const Dbt* key1, const Dbt* key2 );
static int  BdbCompareFloat( Db* dbp, const Dbt* key1, const Dbt* key2 );
static void BdbInitCatalogEntry( SmiCatalogEntry& entry );

/* --- Implementation of class SmiFile --- */

SmiFile::Implementation::Implementation()
{
  bdbHandle = SmiEnvironment::Implementation::AllocateDbHandle();
  bdbFile   = SmiEnvironment::Implementation::GetDbHandle( bdbHandle );
/*
The constructor cannot allocate a Berkeley DB handle by itself since handles
must stay open until the enclosing transaction has been terminated, that is
the handle must be capable to survive this SmiFile instance.

*/
  isSystemCatalogFile = false;
}

SmiFile::Implementation::~Implementation()
{
  SmiEnvironment::Implementation::FreeDbHandle( bdbHandle );
/*
The destructor flags the handle as not used any more. After the termination
of the enclosing transaction the handle will be closed.

*/
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

  return (ok);
}

bool
SmiFile::Create( const string& context /* = "Default" */ )
{
  int rc = 0;

  if ( CheckName( context ) )
  {
    fileId = SmiEnvironment::Implementation::GetFileId();
    if ( fileId != 0 )
    {
      string bdbName =
        SmiEnvironment::Implementation::ConstructFileName( fileId );

      // --- Find out the appropriate Berkeley DB file type
      // --- and set required flags or options if necessary

      DBTYPE bdbType;
      switch ( fileType )
      {
        case Keyed:
          bdbType = DB_BTREE;
          if ( !uniqueKeys )
          {
            impl->bdbFile->set_flags( DB_DUP );
          }
          if ( keyDataType == SmiKey::Integer )
          {
            impl->bdbFile->set_bt_compare( BdbCompareInteger );
          }
          else if ( keyDataType == SmiKey::Float )
          {
            impl->bdbFile->set_bt_compare( BdbCompareFloat );
          }
          break;
        case FixedLength:
          bdbType = DB_QUEUE;
          impl->bdbFile->set_re_len( fixedRecordLength );
          break;
        case VariableLength:
        default:
          bdbType = DB_RECNO;
          break;
      }

      // --- Set Berkeley DB page size

      u_int32_t pagesize = 
        SmiProfile::GetParameter( context, "PageSize", 0,
                                  SmiEnvironment::configFile );
      if ( pagesize > 0 )
      {
        impl->bdbFile->set_pagesize( pagesize );
      }

      // --- Open Berkeley DB file

      rc = impl->bdbFile->open( bdbName.c_str(), 0, bdbType, DB_CREATE, 0 );
      if ( rc == 0 )
      {
        SmiDropFilesEntry entry;
        entry.fileId = fileId;
        entry.dropOnCommit = false;
        SmiEnvironment::instance.impl->bdbFilesToDrop.push( entry );
        SmiEnvironment::SetError( E_SMI_OK );
        opened      = true;
        fileName    = "";
        fileContext = context;
        impl->isSystemCatalogFile = (fileContext == "SecondoCatalog");
      }
      else
      {
        SmiEnvironment::SetError( E_SMI_FILE_CREATE, rc );
      }
    }
    else
    {
      rc = E_SMI_FILE_NOFILEID;
      SmiEnvironment::SetError( E_SMI_FILE_NOFILEID );
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
SmiFile::Open( const string& name, const string& context /* = "Default" */ )
{
  int rc = 0;
  bool existing = false;

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
        SmiEnvironment::instance.impl->bdbFilesToCatalog.find( newName );
      if ( it != SmiEnvironment::instance.impl->bdbFilesToCatalog.end() &&
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
      string bdbName =
        SmiEnvironment::Implementation::ConstructFileName( fileId );

      // --- Find out the appropriate Berkeley DB file type
      // --- and set required flags or options if necessary

      DBTYPE bdbType;
      switch ( fileType )
      {
        case Keyed:
          bdbType = DB_BTREE;
          if ( !uniqueKeys )
          {
            impl->bdbFile->set_flags( DB_DUP );
          }
          if ( keyDataType == SmiKey::Integer )
          {
            impl->bdbFile->set_bt_compare( BdbCompareInteger );
          }
          else if ( keyDataType == SmiKey::Float )
          {
            impl->bdbFile->set_bt_compare( BdbCompareFloat );
          }
          break;
        case FixedLength:
          bdbType = DB_QUEUE;
          impl->bdbFile->set_re_len( fixedRecordLength );
          break;
        case VariableLength:
        default:
          bdbType = DB_RECNO;
          break;
      }

      // --- Set Berkeley DB page size

      u_int32_t pagesize = 
        SmiProfile::GetParameter( context, "PageSize", 0,
                                  SmiEnvironment::configFile );
      if ( pagesize > 0 )
      {
        impl->bdbFile->set_pagesize( pagesize );
      }

      // --- Open Berkeley DB file

      rc = impl->bdbFile->open( bdbName.c_str(), 0, bdbType, DB_CREATE | DB_DIRTY_READ, 0 );
      if ( rc == 0 )
      {
        if ( !existing )
        {
          // --- Register newly created file, since it has to be dropped
          // --- when the enclosing transaction is aborted
          SmiDropFilesEntry dropEntry;
          dropEntry.fileId = fileId;
          dropEntry.dropOnCommit = false;
          SmiEnvironment::instance.impl->bdbFilesToDrop.push( dropEntry );
          SmiCatalogFilesEntry catalogEntry;
          BdbInitCatalogEntry( catalogEntry.entry );
          catalogEntry.entry.fileId = fileId;
          newName.copy( catalogEntry.entry.fileName, 2*SMI_MAX_NAMELEN+1 );
          catalogEntry.entry.isKeyed = fileType == Keyed;
          catalogEntry.entry.isFixed = fileType == FixedLength;
          catalogEntry.updateOnCommit = true;
          SmiEnvironment::instance.impl->bdbFilesToCatalog[newName] = catalogEntry;
        }
        SmiEnvironment::SetError( E_SMI_OK );
        opened      = true;
        fileName    = name;
        fileContext = context;
        impl->isSystemCatalogFile = (fileContext == "SecondoCatalog");
      }
      else
      {
        SmiEnvironment::SetError( E_SMI_FILE_CREATE, rc );
      }
    }
    else
    {
      rc = E_SMI_FILE_NOFILEID;
      SmiEnvironment::SetError( E_SMI_FILE_NOFILEID );
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
SmiFile::Open( const SmiFileId fileId, const string& context /* = "Default" */ )
{
  int rc = 0;

  if ( CheckName( context ) )
  {
    SmiCatalogEntry entry;
    if ( SmiEnvironment::Implementation::LookUpCatalog( fileId, entry ) )
    {
      // --- File found in permanent file catalog
      char* point = strchr( entry.fileName, '.' );
      *point = '\0';
      fileContext = entry.fileName;
      fileName = ++point;
      this->fileId = fileId;
    }
    else
    {
      fileContext = context;
      fileName    = "";
    }
    if ( fileId != 0 && fileContext == context )
    {
      string bdbName =
        SmiEnvironment::Implementation::ConstructFileName( fileId );

      // --- Find out the appropriate Berkeley DB file type
      // --- and set required flags or options if necessary

      DBTYPE bdbType;
      switch ( fileType )
      {
        case Keyed:
          bdbType = DB_BTREE;
          if ( !uniqueKeys )
          {
            impl->bdbFile->set_flags( DB_DUP );
          }
          if ( keyDataType == SmiKey::Integer )
          {
            impl->bdbFile->set_bt_compare( BdbCompareInteger );
          }
          else if ( keyDataType == SmiKey::Float )
          {
            impl->bdbFile->set_bt_compare( BdbCompareFloat );
          }
          break;
        case FixedLength:
          bdbType = DB_QUEUE;
          impl->bdbFile->set_re_len( fixedRecordLength );
          break;
        case VariableLength:
        default:
          bdbType = DB_RECNO;
          break;
      }

      // --- Set Berkeley DB page size

      u_int32_t pagesize = 
        SmiProfile::GetParameter( context, "PageSize", 0,
                                  SmiEnvironment::configFile );
      if ( pagesize > 0 )
      {
        impl->bdbFile->set_pagesize( pagesize );
      }

      // --- Open Berkeley DB file

      rc = impl->bdbFile->open( bdbName.c_str(), 0, bdbType, 0, 0 );
      if ( rc == 0 )
      {
        SmiEnvironment::SetError( E_SMI_OK );
        opened = true;
        impl->isSystemCatalogFile = (fileContext == "SecondoCatalog");
      }
      else
      {
        SmiEnvironment::SetError( E_SMI_FILE_OPEN, rc );
      }
    }
    else
    {
      rc = (fileId == 0) ? E_SMI_FILE_NOFILEID : E_SMI_FILE_BADCONTEXT;
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
  int rc = 0;

  if ( opened )
  {
    // --- The current Berkeley DB handle is freed, but a new one is
    // --- allocated for possible reuse of this SmiFile instance
    opened = false;
    SmiEnvironment::Implementation::FreeDbHandle( impl->bdbHandle );
    impl->bdbHandle = SmiEnvironment::Implementation::AllocateDbHandle();
    impl->bdbFile   = SmiEnvironment::Implementation::GetDbHandle( impl->bdbHandle );
    impl->isSystemCatalogFile = false;
    SmiEnvironment::SetError( E_SMI_OK );
  }
  else
  {
    SmiEnvironment::SetError( E_SMI_OK );
  }

  return (rc == 0);
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
    SmiEnvironment::instance.impl->bdbFilesToDrop.push( dropEntry );
    SmiCatalogFilesEntry catalogEntry;
    if ( fileName.length() > 0 )
    {
      string newName = fileContext + '.' + fileName;
      BdbInitCatalogEntry( catalogEntry.entry );
      catalogEntry.entry.fileId = fileId;
      newName.copy( catalogEntry.entry.fileName, 2*SMI_MAX_NAMELEN+1 );
      catalogEntry.entry.isKeyed = fileType == Keyed;
      catalogEntry.entry.isFixed = fileType == FixedLength;
      catalogEntry.updateOnCommit = false;
      SmiEnvironment::instance.impl->bdbFilesToCatalog[newName] = catalogEntry;
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
  return (fileId);
}

bool
SmiFile::IsOpen()
{
  return (opened);
}

// --- Key comparison function for integer keys

static int BdbCompareInteger( Db* dbp, const Dbt* key1, const Dbt* key2 )
{
  int ret;
  long d1, d2;
  memcpy( &d1, key1->get_data(), sizeof(long) );
  memcpy( &d2, key2->get_data(), sizeof(long) );

  if ( d1 < d2 )
  {
    ret = -1;
  }
  else if ( d1 > d2 )
  {
    ret = 1;
  }
  else
  {
    ret = 0;
  }
  return (ret);
}

// --- Key comparison function for floating point keys

static int BdbCompareFloat( Db* dbp, const Dbt* key1, const Dbt* key2 )
{
  int ret;
  double d1, d2;
  memcpy( &d1, key1->get_data(), sizeof(double) );
  memcpy( &d2, key2->get_data(), sizeof(double) );

  if ( d1 < d2 )
  {
    ret = -1;
  }
  else if ( d1 > d2 )
  {
    ret = 1;
  }
  else
  {
    ret = 0;
  }
  return (ret);
}

// --- Initialize Catalog Entry ---

static void BdbInitCatalogEntry( SmiCatalogEntry& entry )
{
  memset( entry.fileName, 0 , (2*SMI_MAX_NAMELEN+2) );
}

// --- Implementation of class SmiFileIterator ---

SmiFileIterator::Implementation::Implementation()
  : bdbCursor( 0 )
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
  static char keyData[SMI_MAX_KEYLEN];
  bool ok = false;
  if ( opened )
  {
    Dbt key( keyData, SMI_MAX_KEYLEN );
    key.set_data( keyData );

    // --- Initialize key if required

    if ( restart && (rangeSearch || solelyDuplicates) )
    {
      memcpy( keyData, searchKey->GetAddr(), searchKey->keyLength );
      key.set_size( searchKey->keyLength );
    }
    key.set_ulen( SMI_MAX_KEYLEN );
    key.set_flags( DB_DBT_USERMEM );

    // --- Initialize data buffer

    char buffer;
    Dbt data( &buffer, 0 );
    data.set_ulen( 0 );
    data.set_flags( DB_DBT_USERMEM | DB_DBT_PARTIAL );

    // --- Find out the appropriate cursor position

    u_int32_t flags;
    if ( restart && solelyDuplicates ) flags = DB_SET;
    else if ( restart && rangeSearch ) flags = DB_SET_RANGE;
    else if ( restart )                flags = DB_FIRST;
    else if ( solelyDuplicates )       flags = DB_NEXT_DUP;
    else if ( ignoreDuplicates )       flags = DB_NEXT_NODUP;
    else                               flags = DB_NEXT;

    // --- Position the cursor to the requested record
    // --- without reading any data, then find out the
    // --- size of the record

    restart   = false;
    endOfScan = false;
    int rc = impl->bdbCursor->get( &key, &data, flags );
    if ( rc == 0 )
    {
      flags = DB_CURRENT;
      data.set_ulen( 0 );
      data.set_flags( DB_DBT_USERMEM );
      rc = impl->bdbCursor->get( &key, &data, flags );
    }

    // --- Initialize record handle if record is available

    if ( rc == ENOMEM )
    {
      if ( record.initialized )
      {
        record.Finish();
      }
      record.recordKey.SetKey( smiFile->keyDataType,
                               keyData, key.get_size() );
      record.recordSize        = data.get_size();
      record.writable          = writable;
      record.smiFile           = smiFile;
      record.impl->bdbFile     = smiFile->impl->bdbFile;
      record.impl->useCursor   = true;
      record.impl->closeCursor = false;
      record.impl->bdbCursor   = impl->bdbCursor;
      record.initialized       = true;
      SmiEnvironment::SetError( E_SMI_OK );
      ok = true;
    }
    else if ( rc == DB_NOTFOUND )
    {
      endOfScan = true;
      SmiEnvironment::SetError( E_SMI_CURSOR_ENDOFSCAN );
    }
    else
    {
      SmiEnvironment::SetError( E_SMI_CURSOR_NEXT, rc );
    }
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
  if ( opened )
  {
    int rc = impl->bdbCursor->del( 0 );
    if ( rc == 0 )
    {
      SmiEnvironment::SetError( E_SMI_OK );
    }
    else
    {
      SmiEnvironment::SetError( E_SMI_CURSOR_DELETE, rc );
    }
    ok = (rc == 0);
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
    int rc = impl->bdbCursor->close();
    if ( rc == 0 )
    {
      SmiEnvironment::SetError( E_SMI_OK );
    }
    else
    {
      SmiEnvironment::SetError( E_SMI_CURSOR_FINISH, rc );
    }
    ok = (rc == 0);
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

/* --- bdbFile.cpp --- */

