/*

1 Implementation of SmiEnvironment using the Berkeley-DB 

April 2002 Ulrich Telle

*/

#include <string>
#include <algorithm>
#include <cctype>
#include <vector>
#include <queue>
#include <map>
#include <sstream>
#include <iomanip>

#include <db_cxx.h>
#include "SecondoSMI.h"
#include "SmiBDB.h"
#include "SmiCodes.h"
#include "Profiles.h"
#include "FileSystem.h"

using namespace std;

/* --- Prototypes of internal functions --- */

static int getfilename( Db* dbp, const Dbt* pkey, const Dbt* pdata, Dbt* skey );

/* --- Implementation of class SmiEnvironment --- */

SmiEnvironment SmiEnvironment::instance;
SmiError       SmiEnvironment::lastError = E_SMI_OK;
string         SmiEnvironment::lastMessage;
bool           SmiEnvironment::smiStarted = false;
bool           SmiEnvironment::singleUserMode = false;
bool           SmiEnvironment::useTransactions = false;
string         SmiEnvironment::configFile;
string         SmiEnvironment::uid;
bool           SmiEnvironment::dbOpened = false;
string         SmiEnvironment::database;
string         SmiEnvironment::registrar;
SmiEnvironment::SmiType SmiEnvironment::smiType = SmiEnvironment::SmiBerkeleyDB;

SmiEnvironment::Implementation::Implementation()
  : bdbHome( "" ), usrTxn( 0 ), txnStarted( false ),
    bdbDatabases( 0 ), bdbSeq( 0 ), bdbCatalog( 0 ), bdbCatalogIndex( 0 )
{
  bdbEnv = new DbEnv( DB_CXX_NO_EXCEPTIONS );
  dbHandles.reserve( DEFAULT_DBHANDLE_ALLOCATION_COUNT );
  SmiDbHandleEntry dummy = { 0, false, 0 };
  dbHandles.push_back( dummy );
  firstFreeDbHandle = 0;
}

SmiEnvironment::Implementation::~Implementation()
{
  delete bdbEnv;
}

DbHandleIndex
SmiEnvironment::Implementation::AllocateDbHandle()
{
  DbHandleIndex idx = instance.impl->firstFreeDbHandle;
  
  if ( idx == 0 )
  {
    // --- Allocate new handle entry
    if ( instance.impl->dbHandles.size() >=
         instance.impl->dbHandles.capacity() )
    {
      instance.impl->dbHandles.reserve( instance.impl->dbHandles.size() + DEFAULT_DBHANDLE_ALLOCATION_COUNT );
    }
    SmiDbHandleEntry dummy = { 0, false, 0 };
    instance.impl->dbHandles.push_back( dummy );
    idx = instance.impl->dbHandles.size() - 1;
  }
  else
  {
    // --- Reuse free handle entry
    instance.impl->firstFreeDbHandle = instance.impl->dbHandles[idx].nextFree;
  }

  instance.impl->dbHandles[idx].handle   = 
    new Db( instance.impl->bdbEnv, DB_CXX_NO_EXCEPTIONS );
  instance.impl->dbHandles[idx].inUse    = true;
  instance.impl->dbHandles[idx].nextFree = 0;

  return (idx);
}

Db*
SmiEnvironment::Implementation::GetDbHandle( DbHandleIndex idx )
{
  return (instance.impl->dbHandles[idx].handle);
}

void
SmiEnvironment::Implementation::FreeDbHandle( DbHandleIndex idx )
{
  instance.impl->dbHandles[idx].inUse = false;
}

void
SmiEnvironment::Implementation::CloseDbHandles()
{
  DbHandleIndex idx;
  for ( idx = 1; idx < instance.impl->dbHandles.size(); idx++ )
  {
    if ( !instance.impl->dbHandles[idx].inUse &&
          instance.impl->dbHandles[idx].handle != 0 )
    {
      instance.impl->dbHandles[idx].handle->close( 0 );
      delete instance.impl->dbHandles[idx].handle;
      instance.impl->dbHandles[idx].handle = 0;
      instance.impl->dbHandles[idx].nextFree = instance.impl->firstFreeDbHandle;
      instance.impl->firstFreeDbHandle = idx;
    }
  }
}

SmiFileId 
SmiEnvironment::Implementation::GetFileId()
{
  SmiFileId newFileId = 0;

  if ( !dbOpened )
  {
    SetError( E_SMI_DB_NOTOPEN );
    return (newFileId);
  }

  int       rc = 0;
  DbEnv*    dbenv = instance.impl->bdbEnv;
  Db*       dbseq = instance.impl->bdbSeq;

  if ( dbseq )
  {
    DbTxn* tid = 0;
    db_recno_t seqno = SMI_SEQUENCE_FILEID;
    Dbt key( &seqno, sizeof(seqno) );
    Dbt data;
    
    data.set_flags( DB_DBT_USERMEM );
    data.set_data( &newFileId );
    data.set_ulen( sizeof( newFileId ) );

    if ( useTransactions )
    {
      rc = dbenv->txn_begin( 0, &tid, 0 );
    }

    if ( rc == 0 )
    {
      rc = dbseq->get( tid, &key, &data, DB_RMW );
      if ( rc == DB_NOTFOUND  || rc == DB_KEYEMPTY )
      {
        newFileId = 1;
        data.set_size( sizeof( newFileId ) );
        rc = 0;
      }
      else if ( rc == 0 )
      {
        newFileId++;
      }
      if ( rc == 0 )
      {
        if ( (rc = dbseq->put( tid, &key, &data, 0 )) == 0 )
        {
          if ( useTransactions )
          {
            rc = tid->commit( 0 );
          }
        }
        else
        {
          if ( useTransactions )
          {
            tid->abort();
          }
        }
        if ( rc != 0 )
        {
          newFileId = 0;
        }
      }
    }
    else if ( tid != 0 )
    {
      tid->abort();
    }
  }

  return (newFileId);
}

bool
SmiEnvironment::Implementation::LookUpCatalog( const string& fileName,
                                               SmiCatalogEntry& entry )
{
  if ( !dbOpened )
  {
    SetError( E_SMI_DB_NOTOPEN );
    return (false);
  }

  int    rc = 0;
  DbEnv* dbenv = instance.impl->bdbEnv;
  Db*    dbidx = instance.impl->bdbCatalogIndex;

  if ( dbidx )
  {
    DbTxn* tid = 0;
    Dbt key( (void*) fileName.c_str(), fileName.length() );
    Dbt data;

    data.set_flags( DB_DBT_USERMEM );
    data.set_data( &entry );
    data.set_ulen( sizeof( entry ) );

    if ( useTransactions )
    {
      rc = dbenv->txn_begin( 0, &tid, 0 );
    }

    if ( rc == 0 )
    {
      rc = dbidx->get( tid, &key, &data, 0 );
      if ( rc == 0 || rc == DB_NOTFOUND || rc == DB_KEYEMPTY )
      {
        if ( useTransactions )
        {
          tid->commit( 0 );
        }
        if ( rc == 0 )
        {
          SetError( E_SMI_OK );
        }
        else
        {
          SetError( E_SMI_CATALOG_NOTFOUND );
        }
      }
      else
      {
        if ( useTransactions )
        {
          tid->abort();
        }
        SetError( E_SMI_CATALOG_LOOKUP, rc );
      }
    }
    else
    {
      if ( tid != 0 )
      {
        tid->abort();
      }
      SetError( E_SMI_CATALOG_LOOKUP, rc );
    }
  }
  else
  {
    SetError( E_SMI_CATALOG_LOOKUP );
    rc = E_SMI_CATALOG_LOOKUP;
  }

  return (rc == 0);
}

bool
SmiEnvironment::Implementation::LookUpCatalog( const SmiFileId fileId,
                                               SmiCatalogEntry& entry )
{
  if ( !dbOpened )
  {
    SetError( E_SMI_DB_NOTOPEN );
    return (false);
  }

  int    rc = 0;
  DbEnv* dbenv = instance.impl->bdbEnv;
  Db*    dbctl = instance.impl->bdbCatalog;

  if ( dbctl )
  {
    DbTxn* tid = 0;
    Dbt key( (void*) &fileId, sizeof( fileId ) );
    Dbt data;

    data.set_flags( DB_DBT_USERMEM );
    data.set_data( &entry );
    data.set_ulen( sizeof( entry ) );

    if ( useTransactions )
    {
      rc = dbenv->txn_begin( 0, &tid, 0 );
    }

    if ( rc == 0 )
    {
      rc = dbctl->get( tid, &key, &data, 0 );
      if ( rc == 0 || rc == DB_NOTFOUND || rc == DB_KEYEMPTY )
      {
        if ( useTransactions )
        {
          tid->commit( 0 );
        }
        if ( rc == 0 )
        {
          SetError( E_SMI_OK );
        }
        else
        {
          SetError( E_SMI_CATALOG_NOTFOUND );
        }
      }
      else
      {
        if ( useTransactions )
        {
          tid->abort();
        }
        SetError( E_SMI_CATALOG_LOOKUP, rc );
      }
    }
    else
    {
      if ( tid != 0 )
      {
        tid->abort();
      }
      SetError( E_SMI_CATALOG_LOOKUP, rc );
    }
  }
  else
  {
    SetError( E_SMI_CATALOG_LOOKUP );
    rc = E_SMI_CATALOG_LOOKUP;
  }

  return (rc == 0);
}

bool
SmiEnvironment::Implementation::InsertIntoCatalog( const SmiCatalogEntry& entry, DbTxn* tid )
{
  if ( !dbOpened )
  {
    SetError( E_SMI_DB_NOTOPEN );
    return (false);
  }

  int    rc = 0;
  Db*    dbctl = instance.impl->bdbCatalog;

  if ( dbctl )
  {
    Dbt key( (void*) &entry.fileId, sizeof( entry.fileId ) );
    Dbt data( (void*) &entry, sizeof( entry ) );

    rc = dbctl->put( tid, &key, &data, DB_NOOVERWRITE );
    if ( rc == 0 || rc == DB_KEYEXIST )
    {
      if ( rc == 0 )
      {
        SetError( E_SMI_OK );
      }
      else
      {
        SetError( E_SMI_CATALOG_KEYEXIST );
      }
    }
    else
    {
      SetError( E_SMI_CATALOG_INSERT, rc );
    }
  }
  else
  {
    SetError( E_SMI_CATALOG_INSERT );
    rc = E_SMI_CATALOG_INSERT;
  }

  return (rc == 0);
}

bool
SmiEnvironment::Implementation::DeleteFromCatalog( const string& fileName, DbTxn* tid )
{
  if ( !dbOpened )
  {
    SetError( E_SMI_DB_NOTOPEN );
    return (false);
  }

  int    rc = 0;
  Db*    dbidx = instance.impl->bdbCatalogIndex;

  if ( dbidx )
  {
    Dbt key( (void*) fileName.c_str(), fileName.length() );

    rc = dbidx->del( tid, &key, 0 );
    if ( rc == 0 || rc == DB_NOTFOUND )
    {
      if ( rc == 0 )
      {
        SetError( E_SMI_OK );
      }
      else
      {
        SetError( E_SMI_CATALOG_NOTFOUND );
      }
      rc = 0;
    }
    else
    {
      SetError( E_SMI_CATALOG_DELETE, rc );
    }
  }
  else
  {
    SetError( E_SMI_CATALOG_DELETE );
    rc = E_SMI_CATALOG_DELETE;
  }

  return (rc == 0);
}

bool
SmiEnvironment::Implementation::UpdateCatalog( bool onCommit )
{
  if ( !dbOpened )
  {
    SetError( E_SMI_DB_NOTOPEN );
    return (false);
  }

  bool   ok = true;
  int    rc = 0;
  DbEnv* dbenv = instance.impl->bdbEnv;
  DbTxn* ptid  = instance.impl->usrTxn;
  DbTxn* tid = 0;

  if ( useTransactions )
  {
    rc = dbenv->txn_begin( ptid, &tid, 0 );
  }

  if ( rc == 0 )
  {
    map<string,SmiCatalogFilesEntry>::iterator it = 
      instance.impl->bdbFilesToCatalog.begin();
    for ( ; ok && it != instance.impl->bdbFilesToCatalog.end(); ++it )
    {
      if ( it->second.updateOnCommit )
      {
        ok = InsertIntoCatalog( it->second.entry, tid );
      }
      else
      {
        ok = DeleteFromCatalog( it->first.c_str(), tid );
      }
    }
    instance.impl->bdbFilesToCatalog.clear();
    if ( useTransactions )
    {
      if ( ok )
      {
        tid->commit( 0 );
      }
      else
      {
        tid->abort();
      }
    }
  }
  else
  {
    if ( tid != 0 )
    {
      tid->abort();
    }
    SetError( E_SMI_DB_UPDATE_CATALOG, rc );
  }
  return (ok);
}

bool
SmiEnvironment::Implementation::EraseFiles( bool onCommit )
{
  if ( !dbOpened )
  {
    SetError( E_SMI_DB_NOTOPEN );
    return (false);
  }

  int    rc = 0;
  DbEnv* dbenv = instance.impl->bdbEnv;
  SmiDropFilesEntry entry;

  while ( !instance.impl->bdbFilesToDrop.empty() )
  {
    entry = instance.impl->bdbFilesToDrop.front();
    if ( ( onCommit &&  entry.dropOnCommit) ||
         (!onCommit && !entry.dropOnCommit) )
    {
      Db* dbp = new Db( dbenv, DB_CXX_NO_EXCEPTIONS );
      int ret;
      if ( (ret = dbp->remove( ConstructFileName ( entry.fileId ).c_str(), 0, 0 )) != 0 )
      {
        rc = ret;
      }
      delete dbp;
    }
    instance.impl->bdbFilesToDrop.pop();
  }
  return (rc == 0);
}

bool
SmiEnvironment::ListDatabases( string& dbname )
{
  bool ok    = false;
  int  rc    = 0;
  Db*  dbctl = instance.impl->bdbDatabases;
  dbname     = "";

  if ( dbctl )
  {
    u_int32_t flags;
    if ( !instance.impl->listStarted )
    {
      rc = dbctl->cursor( 0, &instance.impl->listCursor, 0 );
      if ( rc != 0 )
      {
        SetError( E_SMI_DB_NOTFOUND, rc );
        return (false);
      }
      instance.impl->listStarted = true;
      flags = DB_FIRST;
    }
    else
    {
      flags = DB_NEXT;
    }

    Dbt key;
    static char keyData[SMI_MAX_DBNAMELEN+1];
    key.set_data( keyData );
    key.set_ulen( SMI_MAX_DBNAMELEN );
    key.set_flags( DB_DBT_USERMEM );

    Dbt data;
    static char ch;    
    data.set_data( &ch );
    data.set_ulen( 1 );
    data.set_flags( DB_DBT_USERMEM );

    rc = instance.impl->listCursor->get( &key, &data, flags );
    if ( rc == 0 )
    {
      keyData[key.get_size()] = 0;
      dbname = keyData;
      SetError( E_SMI_OK );
      ok = true;
    }
    else
    {
      instance.impl->listCursor->close();
      instance.impl->listStarted = false;
      SetError( E_SMI_DB_NOTFOUND, rc );
    }
  }
  else
  {
    SetError( E_SMI_DB_NOTFOUND );
  }
  return (ok);
}

string
SmiEnvironment::Implementation::ConstructFileName( SmiFileId fileId )
{
  ostringstream os;
//  os << database << PATH_SLASH << "d" << setw(10) << setfill('_') << fileId << ".sdb";
  os << database << PATH_SLASH << "d" << fileId << ".sdb";
  return (os.str());
}

bool
SmiEnvironment::Implementation::LookUpDatabase( const string& dbname )
{
  bool   ok = false;   
  int    rc = 0;
  Db*    dbctl = instance.impl->bdbDatabases;

  if ( dbctl )
  {
    Dbt key( (void*) dbname.c_str(), dbname.length() );
    Dbt data;

    char ch;    
    data.set_flags( DB_DBT_USERMEM );
    data.set_data( &ch );
    data.set_ulen( 1 );

    rc = dbctl->get( 0, &key, &data, 0 );
    ok = (rc == 0);
  }
  return (ok);
}

bool
SmiEnvironment::Implementation::InsertDatabase( const string& dbname )
{
  bool   ok = false;   
  int    rc = 0;
  Db*    dbctl = instance.impl->bdbDatabases;

  if ( dbctl )
  {
    Dbt key( (void*) dbname.c_str(), dbname.length() );
    char ch;
    Dbt data( (void*) &ch, 0 );

    rc = dbctl->put( 0, &key, &data, DB_NOOVERWRITE );
    ok = (rc == 0);
  }
  return (ok);
}

bool
SmiEnvironment::Implementation::DeleteDatabase( const string& dbname )
{
  bool   ok = false;
  int    rc = 0;
  Db*    dbctl = instance.impl->bdbDatabases;

  if ( dbctl )
  {
    Dbt key( (void*) dbname.c_str(), dbname.length() );

    rc = dbctl->del( 0, &key, 0 );
    ok = (rc == 0);
  }
  return (ok);
}

SmiEnvironment::SmiEnvironment()
{
  impl = new Implementation();
}

SmiEnvironment::~SmiEnvironment()
{
  delete impl;
}

void
SmiEnvironment::SetError( const SmiError smiErr, const int sysErr = 0 )
{
  lastError = smiErr;
  if ( sysErr != 0 )
  {
    lastMessage = string("SecondoSMI: ") + DbEnv::strerror( sysErr );
  }
  else
  {
    lastMessage = "";
  }
}

bool
SmiEnvironment::StartUp( const RunMode mode, const string& parmFile,
                         ostream& errStream)
{
  if ( smiStarted )
  {
    return (true);
  }

  int rc = 0;
  DbEnv* dbenv = instance.impl->bdbEnv;

  configFile = parmFile;

  // --- Set the name of the registrar for registering and locking databases

  registrar = SmiProfile::GetParameter( "Environment", "RegistrarName", "SECONDO_REGISTRAR", parmFile ); 

  // --- Set output stream for error messages from Berkeley DB
  //     and the prefix string for these messages

  dbenv->set_error_stream( &errStream );
  dbenv->set_errpfx( "SecondoSMI" );

  // --- Set cache size

  u_int32_t cachesize = 
    SmiProfile::GetParameter( "BerkeleyDB", "CacheSize", CACHE_SIZE_STD, parmFile );
  if ( cachesize < CACHE_SIZE_STD )
  {
    cachesize = CACHE_SIZE_STD;
  }
  else if ( cachesize > CACHE_SIZE_MAX )
  {
    cachesize = CACHE_SIZE_MAX;
  }
  rc = dbenv->set_cachesize( 0, cachesize * 1024, 0 );

  // --- Set locking configuration

  if ( rc == 0 )
  {
    u_int32_t lockValue;
    lockValue = SmiProfile::GetParameter( "BerkeleyDB", "MaxLockers", 0, parmFile.c_str() );
    if ( lockValue > 0 )
    {
      rc = dbenv->set_lk_max_lockers( lockValue );
    }
    lockValue = SmiProfile::GetParameter( "BerkeleyDB", "MaxLocks", 0, parmFile.c_str() );
    if ( lockValue > 0 )
    {
      rc = dbenv->set_lk_max_locks( lockValue );
    }
    lockValue = SmiProfile::GetParameter( "BerkeleyDB", "MaxLockObjects", 0, parmFile.c_str() );
    if ( lockValue > 0 )
    {
      rc = dbenv->set_lk_max_objects( lockValue );
    }
    rc = dbenv->set_lk_detect( DB_LOCK_DEFAULT );
  }

  // --- Set log directory, if requested

  if ( rc == 0 )
  {
    string logDir = SmiProfile::GetParameter( "BerkeleyDB", "LogDir", "", parmFile.c_str() );
    if ( logDir.length() > 0 )
    {
      rc = dbenv->set_lg_dir( logDir.c_str() );
    }
  }

  // --- Open Berkeley DB environment

  if ( rc == 0 )
  {
    instance.impl->bdbHome = SmiProfile::GetParameter( "BerkeleyDB", "SecondoHome", "", parmFile.c_str() );
    u_int32_t flags = 0;
    switch ( mode )
    {
      case SmiEnvironment::SingleUserSimple:
        singleUserMode  = true;
        useTransactions = false;
        flags = DB_PRIVATE  | DB_CREATE | DB_INIT_MPOOL;
/*
creates a private environment for the calling process and enables
automatic recovery during startup. If the environment does not exist,
it is created. Transactions, logging and locking are disabled.
Using this mode is not recommended except for read-only databases.

*/
        break;
      case SmiEnvironment::SingleUser:
        singleUserMode  = true;
        useTransactions = true;
        flags = DB_PRIVATE  | DB_CREATE     | DB_RECOVER  |
                DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN;
/*
creates a private environment for the calling process and enables
automatic recovery during startup. If the environment does not exist,
it is created. Transactions and logging are enabled, locking is disabled.

*/
        break;
      case SmiEnvironment::MultiUserMaster:
        singleUserMode  = false;
        useTransactions = true;
        flags = DB_CREATE   | DB_RECOVER    | DB_INIT_LOCK |
                DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN;
/*
enables automatic recovery during startup. If the environment does not exist,
it is created. Transactions, logging and locking are enabled.

*/
        break;
      case SmiEnvironment::MultiUser:
      default:
        singleUserMode  = false;
        useTransactions = true;
        flags = DB_CREATE     | DB_INIT_LOCK | DB_INIT_LOG |
                DB_INIT_MPOOL | DB_INIT_TXN;
/*
If the environment does not exist, it is created.
Transactions, logging and locking are enabled.

*/
        break;
    }
    rc = dbenv->open( instance.impl->bdbHome.c_str(), flags, 0 );

    // --- Open Database Catalog

    Db* dbctlg = new Db( dbenv, DB_CXX_NO_EXCEPTIONS );
    rc = dbctlg->open( "databases", 0, DB_BTREE, DB_CREATE, 0 );
    if ( rc == 0 )
    {
      instance.impl->bdbDatabases = dbctlg;
    }
    else
    {
      dbctlg->close( 0 );
      delete dbctlg;
      instance.impl->bdbDatabases = 0;
    }
  }

  // --- Check error condition

  if ( rc == 0 )
  {
    smiStarted = true;
    SetError( E_SMI_OK );
  }
  else
  {
    smiStarted = false;
    SetError( E_SMI_STARTUP, rc );
  }

  return (rc == 0);
}

bool
SmiEnvironment::ShutDown()
{
  if ( !smiStarted )
  {
    return (true);
  }

  int rc = 0;
  DbEnv* dbenv  = instance.impl->bdbEnv;
  Db*    dbctlg = instance.impl->bdbDatabases;

  // --- Close current database, if opened

  if ( dbOpened )
  {
    CloseDatabase();
  }

  // --- Close Berkeley DB environment

  if ( dbctlg )
  {
    dbctlg->close( 0 );
    delete dbctlg;
    instance.impl->bdbDatabases = 0;
  }

  rc = dbenv->close( 0 );
  smiStarted = false;

  // --- Check error condition

  if ( rc == 0 )
  {
    SetError( E_SMI_OK );
  }
  else
  {
    SetError( E_SMI_SHUTDOWN, rc );
  }

  return (rc == 0);
}

bool
SmiEnvironment::CreateDatabase( const string& dbname )
{
  bool ok = false;

  if ( dbOpened )
  {
    SetError( E_SMI_DB_NOTCLOSED );
    return (false);
  }

  if ( SetDatabaseName( dbname ) )
  {
    if ( !SmiEnvironment::Implementation::LookUpDatabase( database ) )
    {
      // --- Create directory for the new database
      string oldHome = FileSystem::GetCurrentFolder();
      FileSystem::SetCurrentFolder( instance.impl->bdbHome );
      FileSystem::CreateFolder( database );

      // --- Initialize Database
      if ( InitializeDatabase() )
      {
        // --- Insert into Catalog of Databases
        if ( SmiEnvironment::Implementation::InsertDatabase( database ) )
        {
          RegisterDatabase( database );
          dbOpened = ok = true;
          SetError( E_SMI_OK );
        }        
        else
        {
          SetError( E_SMI_DB_CREATE );
        }
      }
      else
      {
        SetError( E_SMI_DB_CREATE );
//        FileSystem::DeleteFileOrFolder( database );
      }
      FileSystem::SetCurrentFolder( oldHome );
    }
    else
    {
      SetError( E_SMI_DB_EXISTING );
    }
  }
  else
  {
    SetError( E_SMI_DB_INVALIDNAME );
  }
  return (ok);
}

bool
SmiEnvironment::OpenDatabase( const string& dbname )
{
  bool ok = false;

  if ( dbOpened )
  {
    SetError( E_SMI_DB_NOTCLOSED );
    return (false);
  }

  SetDatabaseName( dbname );
  if ( SmiEnvironment::Implementation::LookUpDatabase( database ) )
  {
    if ( InitializeDatabase() )
    {
      RegisterDatabase( database );
      dbOpened = ok = true;
      SetError( E_SMI_OK );
    }
    else
    {
      SetError( E_SMI_DB_OPEN );
    }
  }
  else
  {
    SetError( E_SMI_DB_NOTEXISTING );
  }

  return (ok);
}

bool
SmiEnvironment::CloseDatabase()
{
  if ( !dbOpened )
  {
    SetError( E_SMI_DB_NOTOPEN );
    return (false);
  }

  int    rc = 0;
  Db*    dbseq = instance.impl->bdbSeq;
  Db*    dbctl = instance.impl->bdbCatalog;
  Db*    dbidx = instance.impl->bdbCatalogIndex;

  // --- Implicitly commit/abort transaction???

  if ( instance.impl->txnStarted )
  {
    CommitTransaction();
//    AbortTransaction();
  }
  else
  {
    SmiEnvironment::Implementation::CloseDbHandles();
  }

  // --- Close Berkeley DB sequences

  if ( dbseq )
  {
    dbseq->close( 0 );
    delete dbseq;
    instance.impl->bdbSeq = 0;
  }

  // --- Close Berkely DB filecatalog and associated index

  if ( dbctl )
  {
    dbctl->close( 0 );
    delete dbctl;
    instance.impl->bdbCatalog = 0;
  }

  if ( dbidx )
  {
    dbidx->close( 0 );
    delete dbidx;
    instance.impl->bdbCatalogIndex = 0;
  }

  UnregisterDatabase( database );
  dbOpened = false;

  if ( rc == 0)
  {
    SetError( E_SMI_OK );
  }
  else
  {
    SetError( E_SMI_DB_CLOSE, rc );
  }
  return (rc == 0);
}

bool
SmiEnvironment::EraseDatabase( const string& dbname )
{
  bool ok = false;

  if ( !dbOpened )
  {
    SetDatabaseName( dbname );
    if ( SmiEnvironment::Implementation::LookUpDatabase( database ) )
    {
      if ( LockDatabase( database ) )
      {
        DbEnv* dbenv = instance.impl->bdbEnv;
        int ret1 = 0, ret2 = 0;
//        ret1 = dbenv->txn_checkpoint( 0, 0, DB_FORCE );
//        ret2 = dbenv->txn_checkpoint( 0, 0, DB_FORCE );
        ok = (ret1 == 0 && ret2 == 0);
        if ( ok )
        {
          SmiEnvironment::Implementation::DeleteDatabase( database );
          string oldHome = FileSystem::GetCurrentFolder();
          FileSystem::SetCurrentFolder( instance.impl->bdbHome );
          FilenameList fnl;
          if ( FileSystem::FileSearch( database, fnl, 0, 3 ) )
          {
            vector<string>::const_iterator iter = fnl.begin();
            while ( iter != fnl.end() )
            {
              Db*    dbp   = new Db( dbenv, DB_CXX_NO_EXCEPTIONS );
              int ret;
              ret = dbp->remove( (*iter).c_str(), 0, 0 );
              delete dbp;
              iter++;
            }
          }
//          ok = FileSystem::EraseFolder( database );
/*
Since the Berkeley DB is not able to recreate folders on recovery, the
folder is not erased. When making backups of the Secondo database empty
folders should be removed.

*/
          FileSystem::SetCurrentFolder( oldHome );
        }
        UnlockDatabase( database );
        if ( ok )
        {
          SetError( E_SMI_OK );
        }
        else
        {
          SetError( E_SMI_DB_ERASE );
        }
      }
      else
      {
        SetError( E_SMI_DB_NOTLOCKED );
      }
    }
    else
    {
      SetError( E_SMI_DB_NOTEXISTING );
    }
  }
  else
  {
    SetError( E_SMI_DB_NOTCLOSED );
  }
  return (ok);
}

bool
SmiEnvironment::BeginTransaction()
{
  int rc = 0;
  if ( !instance.impl->txnStarted )
  {
    DbEnv* dbenv = instance.impl->bdbEnv;
    if ( useTransactions )
    {
      rc = dbenv->txn_begin( 0, &instance.impl->usrTxn, 0 );
    }
    if ( rc == 0 )
    {
      instance.impl->txnStarted = true;
      SetError( E_SMI_OK );
    }
    else
    {
      SetError( E_SMI_TXN_BEGIN, rc );
    }
  }
  else
  {
    rc = E_SMI_TXN_RUNNING;
    SetError( E_SMI_TXN_RUNNING );
  }
  return (rc == 0);
}

bool
SmiEnvironment::CommitTransaction()
{
  int rc = 0;
  if ( instance.impl->txnStarted )
  {
    // --- Close _all_ open cursors first!!!

    SmiEnvironment::Implementation::UpdateCatalog( true );
    if ( useTransactions )
    {
      rc = instance.impl->usrTxn->commit( 0 );
    }
    if ( rc == 0 )
    {
      SmiEnvironment::Implementation::CloseDbHandles();
      SmiEnvironment::Implementation::EraseFiles( true );
      SetError( E_SMI_OK );
    }
    else
    {
      SmiEnvironment::Implementation::CloseDbHandles();
      SmiEnvironment::Implementation::EraseFiles( false );
      SetError( E_SMI_TXN_COMMIT, rc );
    }
    instance.impl->txnStarted = false;
    instance.impl->usrTxn = 0;
  }
  else
  {
    rc = E_SMI_TXN_NOTRUNNING;
    SetError( E_SMI_TXN_NOTRUNNING );
  }
  return (rc == 0);
}

bool
SmiEnvironment::AbortTransaction()
{
  int rc = 0;
  if ( instance.impl->txnStarted )
  {
    // --- Close _all_ open cursors first!!!

    SmiEnvironment::Implementation::UpdateCatalog( false );
    if ( useTransactions )
    {
      rc = instance.impl->usrTxn->abort();
    }
    SmiEnvironment::Implementation::CloseDbHandles();
    SmiEnvironment::Implementation::EraseFiles( false );
    if ( rc == 0 )
    {
      SetError( E_SMI_OK );
    }
    else
    {
      SetError( E_SMI_TXN_ABORT, rc );
    }
    instance.impl->txnStarted = false;
    instance.impl->usrTxn = 0;
  }
  else
  {
    rc = E_SMI_TXN_NOTRUNNING;
    SetError( E_SMI_TXN_NOTRUNNING );
  }
  return (rc == 0);
}

bool
SmiEnvironment::InitializeDatabase()
{
  int rc;
  string fileName;
  DbEnv* dbenv = instance.impl->bdbEnv;
  Db*    dbctl = 0;
  Db*    dbidx = 0;

  // --- Create Sequence, if it does not exist

  Db* dbseq = new Db( dbenv, DB_CXX_NO_EXCEPTIONS );
  dbseq->set_re_len( sizeof( SmiFileId ) );
  fileName = database+PATH_SLASH+"sequences";
  rc = dbseq->open( fileName.c_str(), 0, DB_QUEUE, DB_CREATE, 0 );
  if ( rc == 0 )
  {
    instance.impl->bdbSeq = dbseq;
  }
  else
  {
    instance.impl->bdbSeq = 0;
  }

  // --- Create system catalog, if it does not exist

  if ( rc == 0 )
  {
    dbctl = new Db( dbenv, DB_CXX_NO_EXCEPTIONS );
    dbidx = new Db( dbenv, DB_CXX_NO_EXCEPTIONS );

    fileName = database+PATH_SLASH+"filecatalog";
    rc = dbctl->open( fileName.c_str(), 0, DB_BTREE, DB_CREATE, 0 );
    if ( rc == 0 )
    {
      instance.impl->bdbCatalog = dbctl;
    }
    else
    {
      instance.impl->bdbCatalog = 0;
    }

    fileName = database+PATH_SLASH+"fileindex";
    rc = dbidx->open( fileName.c_str(), 0, DB_BTREE, DB_CREATE, 0 );
    if ( rc == 0 )
    {
      instance.impl->bdbCatalogIndex = dbidx;
    }
    else
    {
      instance.impl->bdbCatalogIndex = 0;
    }

    // --- Associate the secondary key with the primary key
    if ( rc == 0 )
    {
      rc = dbctl->associate( dbidx, getfilename, 0 );
    }
  }

  // --- Start user transaction

  if ( rc != 0 )
  {
    // --- In case of an error delete the created files

    if ( dbseq )
    {
      dbseq->close( 0 );
      delete dbseq;
      instance.impl->bdbSeq = 0;
      Db*    dbp   = new Db( dbenv, DB_CXX_NO_EXCEPTIONS );
      fileName = database+PATH_SLASH+"sequences";
      dbp->remove( fileName.c_str(), 0, 0 );
      delete dbp;
    }
    if ( dbctl )
    {
      dbctl->close( 0 );
      delete dbctl;
      instance.impl->bdbCatalog = 0;
      Db*    dbp   = new Db( dbenv, DB_CXX_NO_EXCEPTIONS );
      fileName = database+PATH_SLASH+"filecatalog";
      dbp->remove( fileName.c_str(), 0, 0 );
      delete dbp;
    }
    if ( dbidx )
    {
      dbidx->close( 0 );
      delete dbidx;
      instance.impl->bdbCatalogIndex = 0;
      Db*    dbp   = new Db( dbenv, DB_CXX_NO_EXCEPTIONS );
      fileName = database+PATH_SLASH+"fileindex";
      dbp->remove( fileName.c_str(), 0, 0 );
      delete dbp;
    }
  }

  return (rc == 0);
}

/* --- Definition of internal procedures --- */

static int
getfilename( Db* dbp, const Dbt* pkey, const Dbt* pdata, Dbt* skey )
{
  SmiCatalogEntry* entry = (SmiCatalogEntry*) pdata->get_data();
  skey->set_data( entry->fileName );
  skey->set_size( strlen( entry->fileName ) );
  skey->set_ulen( 0 );
  skey->set_dlen( 0 );
  skey->set_doff( 0 );
  skey->set_flags( 0 );
  return (0);
}

/* --- bdbEnvironment.cpp --- */

