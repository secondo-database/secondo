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

1 Implementation of SmiFile using the Berkeley-DB 

April 2002 Ulrich Telle

September 2002 Ulrich Telle, missing fileId in Open method fixed

February 2003 Ulrich Telle, adjusted for Berkeley DB version 4.1.25

April 2003 Ulrich Telle, implemented temporary SmiFiles

August 2004 M. Spiekermann. New private function ~CheckDbHandles~ introduced.
Since files are closed only when an enclosing transaction is finished, reallocation
of DbHandles is only done when necessary, e.g. when an instance op SmiFile should be
reused after ~close~ and ~create~ and ~open~ is called again.

January 2005 M.Spiekermann. Changes in the Implementation of the PrefetchingIterator.
Since Berkeley DB 4.2.52 does not support any longer the bulk retrieval macros with
parameters of class ~Dbt~, a reference to the C-API struct ~DBT~ will be passed now.
This code also compiles with version 4.1.25 of Berkeley-DB. 

*/

using namespace std;

#include <string.h>
#include <string>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <sstream>
#include <cassert>

#include <db_cxx.h>
#include "SecondoSMI.h"
#include "SmiBDB.h"
#include "SmiCodes.h"
#include "Profiles.h"
#include "CharTransform.h"
#include "Counter.h"
#include "WinUnix.h"

static int  BdbCompareInteger( Db* dbp, const Dbt* key1, const Dbt* key2 );
static int  BdbCompareFloat( Db* dbp, const Dbt* key1, const Dbt* key2 );
static void BdbInitCatalogEntry( SmiCatalogEntry& entry );


/* --- Implementation of class SmiFile --- */


SmiFile::Implementation::Implementation()
{
  bdbHandle = SmiEnvironment::Implementation::AllocateDbHandle();
  bdbFile   = SmiEnvironment::Implementation::GetDbHandle( bdbHandle );
	noHandle = false;
/*
The constructor cannot allocate a Berkeley DB handle by itself since handles
must stay open until the enclosing transaction has been terminated, that is
the handle must be capable to survive this SmiFile instance.

*/
  isSystemCatalogFile = false;
  isTemporaryFile = false;
}

SmiFile::Implementation::Implementation( bool isTemp )
{
  bdbHandle = 0;
  bdbFile   = new Db( SmiEnvironment::Implementation::GetTempEnvironment(), 
                      DB_CXX_NO_EXCEPTIONS );
	noHandle = false;
/*
The constructor cannot allocate a Berkeley DB handle by itself since handles
must stay open until the enclosing transaction has been terminated, that is
the handle must be capable to survive this SmiFile instance.

*/
  isSystemCatalogFile = false;
  isTemporaryFile = true;
}

SmiFile::Implementation::~Implementation()
{
  if ( !isTemporaryFile )
  {
    SmiEnvironment::Implementation::FreeDbHandle( bdbHandle );
/*
The destructor flags the handle as not used any more. After the termination
of the enclosing transaction the handle will be closed.

*/
  }
  else
  {
    bdbFile->close( 0 );
  }
}



void
SmiFile::Implementation::CheckDbHandles() {

  static long& ctr = Counter::getRef("SmiFile:Realloc-DBHandles");
  if ( !isTemporaryFile && noHandle ) { // reallocate a DbHandle if necessary
	
    ctr++;
    bdbHandle = SmiEnvironment::Implementation::AllocateDbHandle();
    bdbFile   = SmiEnvironment::Implementation::GetDbHandle( bdbHandle );
		noHandle = false;
  }		
}	



SmiFile::SmiFile( const bool isTemporary )
  : opened( false ), fileContext( "" ), fileName( "" ), fileId( 0 ),
    fixedRecordLength( 0 ), uniqueKeys( true ), keyDataType( SmiKey::Unknown )
{
  if ( !isTemporary )
  {
    impl = new Implementation();
  }
  else
  {
    impl = new Implementation( true );
  }
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
    transform( temp.begin(), temp.end(), temp.begin(), ToLowerProperFunction );
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
  static long& ctrCreate = Counter::getRef("SmiFile::Create");
  static long& ctrOpen = Counter::getRef("SmiFile::Open");
  int rc = 0;
  impl->CheckDbHandles();

  if ( CheckName( context ) )
  {
    fileId = SmiEnvironment::Implementation::GetFileId( 
                                                    impl->isTemporaryFile );
    if ( fileId != 0 )
    {
      string bdbName =
        SmiEnvironment::Implementation::ConstructFileName( 
                                                      fileId, 
                                                      impl->isTemporaryFile );

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
	cout << "Setting page size for SmiFile to " 
             << pagesize << " !" << endl;
      }

      // --- Open Berkeley DB file

      u_int32_t commitFlag = SmiEnvironment::Implementation::AutoCommitFlag;
      u_int32_t flags = (!impl->isTemporaryFile) ? 
                           DB_CREATE | DB_DIRTY_READ | commitFlag : DB_CREATE;
      rc = impl->bdbFile->open( 0, bdbName.c_str(), 0, bdbType, flags, 0 );
      if ( rc == 0 )
      {
        ctrCreate++;
        ctrOpen++;
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
        SmiEnvironment::SetBDBError( rc );
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
  assert ( !opened );
	
  static long& ctr = Counter::getRef("SmiFile::Open");
  int rc = 0;
  bool existing = false;
  impl->CheckDbHandles();
	
  if ( impl->isTemporaryFile )
  {
    rc = E_SMI_FILE_ISTEMP;
    SmiEnvironment::SetError( E_SMI_FILE_ISTEMP );
  }
  else if ( CheckName( context ) && CheckName( name ) )
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

      u_int32_t commitFlag = SmiEnvironment::Implementation::AutoCommitFlag;
      rc = impl->bdbFile->open( 0, bdbName.c_str(), 
                                0, bdbType, 
                                DB_CREATE | DB_DIRTY_READ | commitFlag, 0 );
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
          SmiEnvironment::instance.impl->bdbFilesToCatalog[newName] 
                                                           = catalogEntry;
        }
        ctr++;
        SmiEnvironment::SetError( E_SMI_OK );
        opened      = true;
        fileName    = name;
        fileContext = context;
        impl->isSystemCatalogFile = (fileContext == "SecondoCatalog");
      }
      else
      {
        SmiEnvironment::SetBDBError( rc );
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
SmiFile::Open( const SmiFileId fileid, const string& context /* = "Default" */ )
{
  assert ( !opened );
	
  static long& ctr = Counter::getRef("SmiFile::Open");
  int rc = 0;
  impl->CheckDbHandles();
	
  if ( CheckName( context ) )
  {
    SmiCatalogEntry entry;
    if ( impl->isTemporaryFile )
    {
      fileContext = context;
      fileName    = "";
    }
    else if ( SmiEnvironment::Implementation::LookUpCatalog( fileid, entry ) )
    {
      // --- File found in permanent file catalog
      char* point = strchr( entry.fileName, '.' );
      *point = '\0';
      fileContext = entry.fileName;
      fileName = ++point;
    }
    else
    {
      fileContext = context;
      fileName    = "";
    }

    if ( fileid != 0 && fileContext == context )
    {
      string bdbName =
        SmiEnvironment::Implementation::ConstructFileName( 
                                                      fileid, 
                                                      impl->isTemporaryFile );

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

      u_int32_t commitFlag = SmiEnvironment::Implementation::AutoCommitFlag;
      u_int32_t flags = (!impl->isTemporaryFile) ? 
                            DB_DIRTY_READ | commitFlag : 0;

      rc = impl->bdbFile->open( 0, bdbName.c_str(), 0, bdbType, flags, 0 );
      if ( rc == 0 )
      {
        SmiEnvironment::SetError( E_SMI_OK );
        opened = true;
        impl->isSystemCatalogFile = (fileContext == "SecondoCatalog");
      }
      else
      {
        SmiEnvironment::SetBDBError( rc );
      }
    }
    else
    {
      rc = (fileid == 0) ? E_SMI_FILE_NOFILEID : E_SMI_FILE_BADCONTEXT;
      SmiEnvironment::SetError( rc );
    }
  }
  else
  {
    rc = E_SMI_FILE_INVALIDNAME;
    SmiEnvironment::SetError( E_SMI_FILE_INVALIDNAME );
  }
 
  if( rc == 0 )
  {
    ctr++;
    fileId = fileid;
    return true;
  }
  return false;
}

bool
SmiFile::Close()
{
  static long& ctr = Counter::getRef("SmiFile::Close");
  int rc = 0;

  if ( opened )
  {
    ctr++;
    // --- The current Berkeley DB handle is freed, but a new one is
    // --- allocated for possible reuse of this SmiFile instance
    opened = false;
    if ( !impl->isTemporaryFile )
    {
      SmiEnvironment::Implementation::FreeDbHandle( impl->bdbHandle );
			impl->noHandle = true;
    }
    else
    {
      impl->bdbFile->close( 0 );
      impl->bdbFile = new Db( SmiEnvironment::instance.impl->tmpEnv, 
                              DB_CXX_NO_EXCEPTIONS );
    }
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
  if ( ok && !impl->isTemporaryFile )
  {
    // --- Register SmiFile for real dropping after 
    // --- successfully committing the enclosing transaction
    SmiDropFilesEntry dropEntry;
    dropEntry.fileId = fileId;
    dropEntry.dropOnCommit = true;
    SmiEnvironment::instance.impl->bdbFilesToDrop.push( dropEntry );
    
    /* FIXME : commented out because the persistent version 
               does not work otherwise    
    if ( fileName.length() > 0 )
    {
      SmiCatalogFilesEntry catalogEntry;
      string newName = fileContext + '.' + fileName;
      BdbInitCatalogEntry( catalogEntry.entry );
      catalogEntry.entry.fileId = fileId;
      newName.copy( catalogEntry.entry.fileName, 2*SMI_MAX_NAMELEN+1 );
      catalogEntry.entry.isKeyed = fileType == Keyed;
      catalogEntry.entry.isFixed = fileType == FixedLength;
      catalogEntry.updateOnCommit = false;
      SmiEnvironment::instance.impl->bdbFilesToCatalog[newName] = catalogEntry;
    }*/
    
    SmiEnvironment::SetError( E_SMI_OK );
  }
  return (ok);
}

bool 
SmiFile::Truncate()
{
  DbTxn* tid = !impl->isTemporaryFile ? 
                  SmiEnvironment::instance.impl->usrTxn : 0;
  u_int32_t countp;
  return impl->bdbFile->truncate( tid, &countp, 0 ) == 0;
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
  static long& ctr = Counter::getRef("SmiFileIterator::Next");
  ctr++;
   
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
      SmiEnvironment::SetBDBError( rc );
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
      SmiEnvironment::SetBDBError( rc );
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
      SmiEnvironment::SetBDBError( rc );
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

PrefetchingIterator::~PrefetchingIterator()
{
}

void PrefetchingIterator::CurrentKey(SmiKey& smiKey)
{
  void* addr;
  SmiSize length;
  
  GetKeyAddressAndLength(&addr, length);
  smiKey.SetKey(keyType, addr, length);
}
/*
Get a new bulk of tuples. If that is not possible due to 
too little memory, the state changes to partial retrieval. 

*/
bool PrefetchingIteratorImpl::NewPrefetch()
{  
  //cerr << "PrefIter = " << (void*)this 
  //     << ": NewPrefetch(), state = " << state << endl;
  if(state == INITIAL && (searchType == RANGE || searchType == RIGHTRANGE))
  {
    memcpy(keyBuffer, leftBoundary, leftBoundaryLength);
    keyDbt.set_size(leftBoundaryLength);
    errorCode = dbc->get(&keyDbt, &buffer, DB_SET_RANGE | DB_MULTIPLE_KEY);
  }
  else
  {
    errorCode = dbc->get(&keyDbt, &buffer, DB_NEXT | DB_MULTIPLE_KEY);
  }
  
  if(errorCode != 0)
  {
// VTA - 15.11.2005 - to compile with the new version of Berkeley DB
#if (DB_VERSION_MAJOR == 4) && (DB_VERSION_MINOR == 3)
    if ( errorCode == DB_BUFFER_SMALL )
#else
    if ( errorCode == ENOMEM )
#endif
    {
      Dbt buf;
      const size_t cBufLength = 10;
      char cBuf[cBufLength];
    
      buf.set_data(cBuf);
      buf.set_ulen(cBufLength);
      buf.set_dlen(cBufLength);
      buf.set_doff(0);
      buf.set_flags(DB_DBT_PARTIAL | DB_DBT_USERMEM);
      
      if(state == INITIAL && (searchType == RANGE || searchType == RIGHTRANGE))
      {
        memcpy(keyBuffer, leftBoundary, leftBoundaryLength);
        keyDbt.set_size(leftBoundaryLength);
        errorCode = dbc->get(&keyDbt, &buf, DB_SET_RANGE);
      }
      else
      {
        errorCode = dbc->get(&keyDbt, &buf, DB_NEXT);
      }
      
      if(errorCode == 0)
      {
        state = PARTIAL_RETRIEVAL;
        cerr << "PrefetchingIterator - Warning: state==PARTIAL_RETRIEVAL"
             << endl;
				
        if(!isBTreeIterator)
	      {
          recordNumber = *((db_recno_t*)keyBuffer);
	      };
	      SmiEnvironment::SetError(E_SMI_OK);
        return true;  
      }
    }
   
    // return code DB_NOTFOUND indicates an end of scan!
    if(errorCode != DB_NOTFOUND)
    {
      SmiEnvironment::SetBDBError(errorCode);    
    }
    
    state = BROKEN;
    //cerr << "PrefetchingIterator - Warning: state==BROKEN" << endl;
    return false;
  }
   

  DB_MULTIPLE_INIT( p, buffer.get_DBT() );
  state = BULK_RETRIEVAL; 

  return true;
}

bool PrefetchingIteratorImpl::RightBoundaryExceeded()
{
  size_t cmpLength = 0;
  int rc = 0;
  void* key = 0;
  size_t keyLength = 0;
  
  long keyLong = 0;
  long boundaryLong = 0;
  
  double keyDouble = 0;
  double boundaryDouble = 0;

  if(searchType == ALL || searchType == RIGHTRANGE)
  {
    return false;
  }

  assert(rightBoundary != 0);

  switch(state)
  {
    case BULK_RETRIEVAL:
      key = retKey;
      keyLength = retKeyLength;
      break;

    case PARTIAL_RETRIEVAL:
      key = keyDbt.get_data();
      keyLength = keyDbt.get_size();
      break;

    case BROKEN:
      return true;

    default:
      assert(false);
  }  
  
  /* This is analogous to SmiKey::operator> */
  switch(keyType)
  {
    case SmiKey::Integer:
      assert(keyLength == sizeof(long));
      assert(rightBoundaryLength == sizeof(long));
      memcpy(&keyLong, key, keyLength);
      memcpy(&boundaryLong, rightBoundary, keyLength);
      if(keyLong > boundaryLong)
      {
        // end of scan
        return true;
      }
      else
      {
        return false;
      };
      
    case SmiKey::Float:
      assert(keyLength == sizeof(double));
      assert(rightBoundaryLength == sizeof(double));
      memcpy(&keyDouble, key, keyLength);
      memcpy(&boundaryDouble, rightBoundary, keyLength);
      if(keyDouble > boundaryDouble)
      {
        SmiEnvironment::SetError(E_SMI_CURSOR_ENDOFSCAN);
        return true;
      }
      else
      {
	return false;
      };
  
    case SmiKey::String:
    case SmiKey::Composite:
    case SmiKey::Unknown:
      cmpLength = 
        keyLength > rightBoundaryLength ?
          rightBoundaryLength:
          keyLength;
      rc = memcmp(key, rightBoundary, cmpLength);
      if(rc > 0 || (rc == 0 && keyLength > rightBoundaryLength))
      {
        // the first cmpLength bytes of key are greater than rightBoundary
	// or they are equal but the keyLength is greater
        return true;
      }
      else
      {
	return false;
      };
      
    default:
      assert(false);
      return false;
  }
}

void PrefetchingIteratorImpl::Init
  (Dbc* dbc, const size_t bufferLength, bool isBTreeIterator)
{
  bufferPtr = new char[bufferLength];
  assert(bufferPtr != 0);

  searchType = ALL;
  
  buffer.set_data(bufferPtr);
  buffer.set_ulen(bufferLength);
  buffer.set_flags(DB_DBT_USERMEM);
  
  keyDbt.set_data(keyBuffer);
  keyDbt.set_ulen(SMI_MAX_KEYLEN);
  keyDbt.set_flags(DB_DBT_USERMEM);
  
  this->dbc = dbc;
  this->isBTreeIterator = isBTreeIterator;
  state = INITIAL;
}

void 
PrefetchingIteratorImpl::GetKeyAddressAndLength
  (void** addr, SmiSize& length)
{
  if(isBTreeIterator)
  {
    switch(state)
    {
      case BULK_RETRIEVAL:
        *addr = retKey;
        length = retKeyLength;
        break;

      case PARTIAL_RETRIEVAL:
        *addr = keyDbt.get_data();
        length = keyDbt.get_size();
        break;

      case INITIAL:
      case BROKEN:
        assert(false);
    }
  }
  else
  {
    *addr = &recordNumber;
    length = sizeof(SmiRecordId);
  }
}

PrefetchingIteratorImpl::PrefetchingIteratorImpl
  (Dbc* dbc, SmiKey::KeyDataType keyType, 
  const size_t bufferLength, bool isBTreeIterator)
{
  Init(dbc, bufferLength, isBTreeIterator);
  
  this->keyType = keyType;
}

PrefetchingIteratorImpl::PrefetchingIteratorImpl
  (Dbc* dbc, SmiKey::KeyDataType keyType, const char* leftBoundary, 
  size_t leftBoundaryLength, const char* rightBoundary, 
  size_t rightBoundaryLength, const size_t bufferLength)
{
  assert(leftBoundaryLength >= 0);
  assert(leftBoundaryLength <= SMI_MAX_KEYLEN);
  assert(rightBoundaryLength >= 0);
  assert(rightBoundaryLength <= SMI_MAX_KEYLEN);

  Init(dbc, bufferLength, true);
  
  if(leftBoundary == 0)
  {
    if(rightBoundary == 0)
    {
      searchType = ALL;
    }
    else
    {
      searchType = LEFTRANGE;
    }
  }
  else
  {
    if(rightBoundary == 0)
    {
      searchType = RIGHTRANGE;
    }
    else
    {
      searchType = RANGE;
    }
  }
  
  if(leftBoundary != 0)
  {
    memcpy(this->leftBoundary, leftBoundary, leftBoundaryLength);
  }
  
  if(rightBoundary != 0)
  {
    memcpy(this->rightBoundary, rightBoundary, rightBoundaryLength);
  }
  
  this->leftBoundaryLength = leftBoundaryLength;
  this->rightBoundaryLength = rightBoundaryLength;
  
  this->keyType = keyType;
}

PrefetchingIteratorImpl::~PrefetchingIteratorImpl()
{
  char* bufferPtr;
  
  bufferPtr = (char*)buffer.get_data();
  delete[] bufferPtr;

  assert(dbc->close() != DB_LOCK_DEADLOCK);
}

bool PrefetchingIteratorImpl::Next()
{
  if(state == INITIAL || state == PARTIAL_RETRIEVAL)
  {
    if(!NewPrefetch())
    {
      return false;
    }
  }

  if(state == PARTIAL_RETRIEVAL || state == BROKEN)
  {
    return state == PARTIAL_RETRIEVAL && !RightBoundaryExceeded();
  }
  
  if(isBTreeIterator)
  { 
    DB_MULTIPLE_KEY_NEXT(p, buffer.get_DBT(), retKey, 
      retKeyLength, retData, retDataLength);
  }
  else
  {
    DB_MULTIPLE_RECNO_NEXT(p, buffer.get_DBT(), 
                           recordNumber, retData, retDataLength);
  }
     
  if(p == 0)
  {
    /* The pointer p is managed by Berkeley DB. ~p == 0~ implies that
       no tuples could be bulk-retrieved. */
    if(!NewPrefetch())
    {
      return false;
    }
    
    if(state == PARTIAL_RETRIEVAL || state == BROKEN)
    {
      return state == PARTIAL_RETRIEVAL  && !RightBoundaryExceeded();
    }

    if(isBTreeIterator)
    {   
      DB_MULTIPLE_KEY_NEXT(p, buffer.get_DBT(), retKey, 
        retKeyLength, retData, retDataLength);
    }
    else
    {
      DB_MULTIPLE_RECNO_NEXT(p, buffer.get_DBT(), 
                             recordNumber, retData, retDataLength);
    }
    
    if(p != 0  && !RightBoundaryExceeded())
    {
      return true;
    }
    else
    {
      if (p != 0) // out of range error
        SmiEnvironment::SetError(E_SMI_PREFETCH_RANGE);
      // end of scan
      return false; 
    }
  }
  else
  {
    if(RightBoundaryExceeded())
    {
      //end of scan 
      return false;
    }
    else
    {
      return true;
    }
  }
}

SmiSize PrefetchingIteratorImpl::BulkCopy(void* data, size_t dataLength, 
  void* userBuffer, SmiSize nBytes, SmiSize offset)
{
  char* src = (char*)data;
  SmiSize nBytesCopied;

  if(offset >= dataLength)
  {
    return 0;
  }
  else
  {
    nBytesCopied = 
     offset + nBytes > dataLength ? 
       dataLength - offset :
       nBytes;

    memcpy(userBuffer, src + offset, nBytesCopied);
  }
  return nBytesCopied;
}

SmiSize PrefetchingIteratorImpl::ReadCurrentData
  (void* userBuffer, SmiSize nBytes, SmiSize offset)
{

  static long& ctr = Counter::getRef("Prefetch::ReadCurrent:Calls");
  static long& byteCtr = Counter::getRef("Prefetch::ReadCurrent:Bytes");
  static long& pageCtr = Counter::getRef("Prefetch::ReadCurrent:Pages");
  static const int pageSize = WinUnix::getPageSize();
  SmiSize bytes = 0;
  ctr++;
  
  Dbt buf;

  switch(state)
  {
    case BULK_RETRIEVAL:
      bytes = BulkCopy(retData, retDataLength, userBuffer, nBytes, offset);
      byteCtr += bytes;
      pageCtr = byteCtr / pageSize; 
      return bytes;
      
    case PARTIAL_RETRIEVAL:
      buf.set_data(userBuffer);
      buf.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);
      buf.set_dlen(nBytes);
      buf.set_doff(offset);
      buf.set_ulen(nBytes);
      errorCode = dbc->get(&keyDbt, &buf, DB_CURRENT);
      bytes = buf.get_size();
      byteCtr += bytes;
      return bytes;
    
    case INITIAL:
      assert(false);
      return 0;

    case BROKEN: 
      return 0;
  }
  assert(false);
  return 0;
}

SmiSize PrefetchingIteratorImpl::ReadCurrentKey
  (void* userBuffer, SmiSize nBytes, SmiSize offset)
{
  assert(isBTreeIterator);

  switch(state)
  {
    case BULK_RETRIEVAL:
      return BulkCopy(retKey, retKeyLength, userBuffer, nBytes, offset);
      
    case PARTIAL_RETRIEVAL:
      assert(keyDbt.get_size() <= keyDbt.get_ulen());
      return BulkCopy(keyDbt.get_data(), keyDbt.get_size(), 
        userBuffer, nBytes, offset);
	
    case INITIAL:
      assert(false);
      return 0;
      
    case BROKEN: 
      return 0;
  }
  assert(false);
  return 0;
}

void 
PrefetchingIteratorImpl::ReadCurrentRecordNumber(SmiRecordId& recordNumber)
{
  assert(!isBTreeIterator);
  recordNumber = (SmiRecordId)this->recordNumber;
}

int PrefetchingIteratorImpl::ErrorCode()
{
  return errorCode;
}

/* --- bdbFile.cpp --- */
