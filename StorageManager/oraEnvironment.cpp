/*

1 Implementation of SmiEnvironment using the Oracle-DB 

April 2002 Ulrich Telle

*/

using namespace std;
using namespace OCICPP;

#include <string>
#include <algorithm>
#include <cctype>
#include <vector>
#include <queue>
#include <map>
#include <sstream>
#include <iomanip>

#include "SecondoSMI.h"
#include "SmiORA.h"
#include "SmiCodes.h"
#include "Profiles.h"

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
SmiEnvironment::SmiType SmiEnvironment::smiType = SmiEnvironment::SmiOracleDB;

SmiEnvironment::Implementation::Implementation()
  : usrConnection(), sysConnection(),
    msgStream( &cerr ), txnStarted( false ),
    listStarted( false ), listCursor()
{
}

SmiEnvironment::Implementation::~Implementation()
{
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

  Connection& con = SmiEnvironment::instance.impl->usrConnection;
  try
  {
    string sql = "SELECT " + database + "_SEQUENCES.NEXTVAL FROM DUAL";
    Cursor csr;
    con.execQuery( sql, csr );
    if ( csr.fetch() )
    {
      newFileId = (SmiFileId) csr.getInt( 0 );
    }
    SetError( E_SMI_OK );
  }
  catch ( OraError &err )
  {
    SmiEnvironment::SetError( E_SMI_FILE_NOFILEID, err.message );
  }

  return (newFileId);
}

bool
SmiEnvironment::Implementation::LookUpCatalog( const string& fileName,
                                               SmiCatalogEntry& entry )
{
  bool found = false;
  if ( !dbOpened )
  {
    SetError( E_SMI_DB_NOTOPEN );
    return (false);
  }

  Connection& con = SmiEnvironment::instance.impl->usrConnection;
  try
  {
    string sql = "SELECT FILEID, FILENAME, ISKEYED, ISFIXED FROM " +
                 database + "_TABLES WHERE FILENAME='" +
                 fileName + "'";
    Cursor csr;
    con.execQuery( sql, csr );
    if ( csr.fetch() )
    {
      entry.fileId   = (SmiFileId) csr.getInt( "FILEID" );
      entry.fileName = csr.getStr( "FILENAME" );
      entry.isKeyed  = (csr.getStr( "ISKEYED" ) == "Y");
      entry.isFixed  = (csr.getStr( "ISFIXED" ) == "Y");
      SetError( E_SMI_OK );
      found = true;
    }
    else
    {
      SetError( E_SMI_CATALOG_NOTFOUND );
    }
  }
  catch ( OraError &err )
  {
    SmiEnvironment::SetError( E_SMI_CATALOG_LOOKUP, err.message );
  }
  return (found);
}

bool
SmiEnvironment::Implementation::LookUpCatalog( const SmiFileId fileId,
                                               SmiCatalogEntry& entry )
{
  bool found = false;
  if ( !dbOpened )
  {
    SetError( E_SMI_DB_NOTOPEN );
    return (false);
  }

  Connection& con = SmiEnvironment::instance.impl->usrConnection;
  try
  {
    ostringstream osFileId;
    osFileId << fileId;
    string sql = "SELECT FILEID, FILENAME, ISKEYED, ISFIXED FROM " +
                 database + "_TABLES WHERE FILEID=" + osFileId.str();
    Cursor csr;
    con.execQuery( sql, csr );
    if ( csr.fetch() )
    {
      entry.fileId   = (SmiFileId) csr.getInt( "FILEID" );
      entry.fileName = csr.getStr( "FILENAME" );
      entry.isKeyed  = (csr.getStr( "ISKEYED" ) == "Y");
      entry.isFixed  = (csr.getStr( "ISFIXED" ) == "Y");
      SetError( E_SMI_OK );
      found = true;
    }
    else
    {
      SetError( E_SMI_CATALOG_NOTFOUND );
    }
  }
  catch ( OraError &err )
  {
    SmiEnvironment::SetError( E_SMI_CATALOG_LOOKUP, err.message );
  }
  return (found);
}

bool
SmiEnvironment::Implementation::InsertIntoCatalog( const SmiCatalogEntry& entry )
{
  bool ok = false;
  if ( !dbOpened )
  {
    SetError( E_SMI_DB_NOTOPEN );
    return (false);
  }

  SmiCatalogEntry lookup;
  if ( !LookUpCatalog( entry.fileId, lookup ) )
  {
    Connection& con = SmiEnvironment::instance.impl->usrConnection;
    try
    {
      ostringstream osFileId;
      osFileId << entry.fileId;
      string sql = "INSERT INTO " + database +
                   "_TABLES (FILEID,FILENAME,ISKEYED,ISFIXED) VALUES (" +
                   osFileId.str() + ",'" + entry.fileName + "'," +
                   (entry.isKeyed ? "'Y'" : "'N'") + "," +
                   (entry.isFixed ? "'Y'" : "'N'") + ")";
      con.execUpdate( sql );
      SetError( E_SMI_OK );
      ok = true;
    }
    catch ( OraError &err )
    {
      SmiEnvironment::SetError( E_SMI_CATALOG_INSERT, err.message );
    }
  }
  else
  {
    SetError( E_SMI_CATALOG_KEYEXIST );
  }
  return (ok);
}

bool
SmiEnvironment::Implementation::DeleteFromCatalog( const string& fileName )
{
  bool ok = false;
  if ( !dbOpened )
  {
    SetError( E_SMI_DB_NOTOPEN );
    return (false);
  }

  SmiCatalogEntry lookup;
  if ( LookUpCatalog( fileName, lookup ) )
  {
    Connection& con = SmiEnvironment::instance.impl->usrConnection;
    try
    {
      string sql = "DELETE FROM " + database +
                   "_TABLES WHERE FILENAME = '" + fileName + "'";
      con.execUpdate( sql );
      SetError( E_SMI_OK );
      ok = true;
    }
    catch ( OraError &err )
    {
      SmiEnvironment::SetError( E_SMI_CATALOG_DELETE, err.message );
    }
  }
  else
  {
    SetError( E_SMI_CATALOG_NOTFOUND );
  }
  return (ok);
}

bool
SmiEnvironment::Implementation::UpdateCatalog( bool onCommit )
{
  if ( !dbOpened )
  {
    SetError( E_SMI_DB_NOTOPEN );
    return false;
  }

  bool   ok = true;
  map<string,SmiCatalogFilesEntry>::iterator it = 
    instance.impl->oraFilesToCatalog.begin();
  for ( ; ok && it != instance.impl->oraFilesToCatalog.end(); ++it )
  {
    if ( it->second.updateOnCommit )
    {
      ok = ok && InsertIntoCatalog( it->second.entry );
    }
    else
    {
      ok = ok && DeleteFromCatalog( it->first /*.c_str()*/ );
    }
  }
  instance.impl->oraFilesToCatalog.clear();
  return (ok);
}

bool
SmiEnvironment::Implementation::EraseFiles( bool onCommit )
{
  bool ok = true;
  if ( !dbOpened )
  {
    SetError( E_SMI_DB_NOTOPEN );
    return (false);
  }

  SmiDropFilesEntry entry;
  Connection& con = SmiEnvironment::instance.impl->sysConnection;

  while ( !instance.impl->oraFilesToDrop.empty() )
  {
    entry = instance.impl->oraFilesToDrop.front();
    if ( ( onCommit &&  entry.dropOnCommit) ||
         (!onCommit && !entry.dropOnCommit) )
    {
      try
      {
        string sql = "DROP TABLE " + ConstructTableName( entry.fileId );
        con.execUpdate( sql );
        sql = "DROP SEQUENCE " + ConstructSeqName( entry.fileId );
        con.execUpdate( sql );
        sql = "DROP INDEX " + ConstructIndexName( entry.fileId );
        con.execUpdate( sql );
      }
      catch ( OraError &err )
      {
        SmiEnvironment::SetError( E_SMI_CATALOG_DELETE, err.message );
        ok = false;
      }
    }
    instance.impl->oraFilesToDrop.pop();
  }
  return (ok);
}

string
SmiEnvironment::Implementation::ConstructTableName( SmiFileId fileId )
{
  ostringstream os;
  os << database << "_TAB" << fileId;
  return (os.str());
}

string
SmiEnvironment::Implementation::ConstructSeqName( SmiFileId fileId )
{
  ostringstream os;
  os << database << "_SEQ" << fileId;
  return (os.str());
}

string
SmiEnvironment::Implementation::ConstructIndexName( SmiFileId fileId )
{
  ostringstream os;
  os << database << "_IDX" << fileId;
  return (os.str());
}

bool
SmiEnvironment::Implementation::LookUpDatabase( const string& dbname )
{
  bool found = false;
  Connection& con = SmiEnvironment::instance.impl->usrConnection;
  try
  {
    string sql = string( "SELECT COUNT(*) FROM USER_TABLES " ) +
                 "WHERE TABLE_NAME LIKE '" + dbname + "_TAB%'";
    Cursor csr;
    con.execQuery( sql, csr );
    if ( csr.fetch() )
    {
      SetError( E_SMI_OK );
      found = (csr.getInt( 0 ) > 0);
    }
    else
    {
      SetError( E_SMI_DB_NOTFOUND );
    }
  }
  catch ( OraError &err )
  {
    SmiEnvironment::SetError( E_SMI_DB_LOOKUP, err.message );
  }
  return (found);
}

bool
SmiEnvironment::Implementation::DeleteDatabase( const string& dbname )
{
  bool   ok = false;
  Connection& con = SmiEnvironment::instance.impl->sysConnection;
  try
  {
    string sql = "SELECT TABLE_NAME FROM USER_TABLES WHERE TABLE_NAME LIKE '" +
                 dbname + "_TAB%'";
    Cursor csr;
    con.execQuery( sql, csr );
    while ( csr.fetch() )
    {
      try
      {
        string sql = "DROP TABLE " + csr.getStr( 0 );
        con.execUpdate( sql );
      }
      catch ( OraError &err )
      {
      }
    }
    csr.drop();
    sql = "SELECT SEQUENCE_NAME FROM USER_SEQUENCES WHERE SEQUENCE_NAME LIKE '" +
          dbname + "_SEQ%'";
    con.execQuery( sql, csr );
    while ( csr.fetch() )
    {
      try
      {
        string sql = "DROP SEQUENCE " + csr.getStr( 0 );
        con.execUpdate( sql );
      }
      catch ( OraError &err )
      {
      }
    }
    csr.drop();
    sql = "SELECT INDEX_NAME FROM USER_INDEXES WHERE INDEX_NAME LIKE '" +
          dbname + "_IDX%'";
    con.execQuery( sql, csr );
    while ( csr.fetch() )
    {
      try
      {
        string sql = "DROP INDEX " + csr.getStr( 0 );
        con.execUpdate( sql );
      }
      catch ( OraError &err )
      {
      }
    }
    ok = true;
  }
  catch ( OraError &err )
  {
    SetError( E_SMI_DB_ERASE, err.message );
  }
  return (ok);
}

SmiEnvironment::SmiEnvironment()
{
  db::init();
  impl = new Implementation();
}

SmiEnvironment::~SmiEnvironment()
{
  delete impl;
}

void
SmiEnvironment::SetError( const SmiError smiErr, const int sysErr /* = 0 */ )
{
  lastError = smiErr;
  if ( sysErr != 0 )
  {
    ostringstream os;
    os << sysErr;
    lastMessage = string("SecondoSMI: System error ") + os.str();
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

  SmiEnvironment::instance.impl->msgStream  = &errStream;
  configFile = parmFile;

//  if ( mode != SmiEnvironment::MultiUserMaster &&
//       mode != SmiEnvironment::MultiUser )
//  {
//    SetError( E_SMI_STARTUP, "Single user mode not supported by Oracle SMI." );
//    return (false);
//  }
//  singleUserMode  = false;
  singleUserMode  = (mode == SmiEnvironment::SingleUser ||
                     mode == SmiEnvironment::SingleUserSimple);
/*
Although the Oracle based Secondo version does not support SingleUser mode,
the flag is set to suppress communication to the Secondo registrar.
This makes testing possible without having the registrar running.

*/
  useTransactions = true;
/*
Set to *true* in any running mode.

*/

  // --- Set the name of the registrar for registering and locking databases

  registrar = SmiProfile::GetParameter( "Environment", "RegistrarName", "SECONDO_REGISTRAR", parmFile ); 

  // --- Oracle connection information

  string tnsStr  = SmiProfile::GetParameter( "OracleDB", "ConnectString", "", parmFile );
  string userStr = SmiProfile::GetParameter( "OracleDB", "SecondoUser", "", parmFile );
  string pswdStr = SmiProfile::GetParameter( "OracleDB", "SecondoPswd", "", parmFile );

  if ( tnsStr == "" || userStr == "" || pswdStr == "" )
  {
    SetError( E_SMI_STARTUP, "Oracle connect parameters missing." );
    return (false);
  }

  Connection& conUsr = SmiEnvironment::instance.impl->usrConnection;
  Connection& conSys = SmiEnvironment::instance.impl->sysConnection;

  bool usrConnected = false;
  bool sysConnected = false;
  try
  {
    db::connect( tnsStr, userStr, pswdStr, conUsr );
    usrConnected = true;
    db::connect( tnsStr, userStr, pswdStr, conSys );
    sysConnected = true;
    conUsr.execUpdate( "ALTER SESSION SET NLS_NUMERIC_CHARACTERS='.,'" );
    conSys.execUpdate( "ALTER SESSION SET NLS_NUMERIC_CHARACTERS='.,'" );
/*
To set the Oracle session parameter NLS\_NUMERIC\_CHARACTERS is necessary
to transfer floating point numbers correctly between client and server,
because C++ defaults always to a decimal point.

*/
//    conUsr.execUpdate( "ALTER SESSION SET NLS_LANGUAGE=AMERICAN NLS_TERRITORY=AMERICA" );
//    conUsr.execUpdate( "ALTER SESSION SET NLS_LANGUAGE=AMERICAN NLS_TERRITORY=AMERICA" );
/*
These session parameters are an alternative to the parameter
NLS\_NUMERIC\_CHARACTERS. Additionally they force English as the language
for error messages.

*/
    smiStarted = true;
    SetError( E_SMI_OK );
  }
  catch ( OraError &err )
  {
    if ( usrConnected )
    {
      try
      {
        conUsr.drop();
      }
      catch ( OraError &err )
      {
      }
    }
    if ( sysConnected )
    {
      try
      {
        conSys.drop();
      }
      catch ( OraError &err )
      {
      }
    }
    SetError( E_SMI_STARTUP, err.message );
  }

  return (smiStarted);
}

bool
SmiEnvironment::ShutDown()
{
  if ( !smiStarted )
  {
    return true;
  }

  // --- Close current database, if opened

  if ( dbOpened )
  {
    CloseDatabase();
  }

  // --- Close Oracle DB environment

  Connection& conUsr = SmiEnvironment::instance.impl->usrConnection;
  bool usrClosed = false;
  try
  {
    conUsr.drop();
    usrClosed = true;
  }
  catch ( OraError &err )
  {
    SetError( E_SMI_SHUTDOWN, err.message );
  }

  Connection& conSys = SmiEnvironment::instance.impl->sysConnection;
  bool sysClosed = false;
  try
  {
    conSys.drop();
    sysClosed = true;
  }
  catch ( OraError &err )
  {
    SetError( E_SMI_SHUTDOWN, err.message );
  }

  smiStarted = false;
  return (usrClosed && sysClosed);
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
      // --- Initialize Database
      if ( InitializeDatabase() )
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
    return false;
  }

  SetDatabaseName( dbname );
  if ( SmiEnvironment::Implementation::LookUpDatabase( database ) )
  {
    RegisterDatabase( database );
    dbOpened = ok = true;
    SetError( E_SMI_OK );
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

  // --- Implicitly commit/abort transaction???

  if ( instance.impl->txnStarted )
  {
    CommitTransaction();
//    AbortTransaction();
  }

  UnregisterDatabase( database );
  dbOpened = false;
  SetError( E_SMI_OK );
  return (true);
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
        SmiEnvironment::Implementation::DeleteDatabase( database );
        UnlockDatabase( database );
        SetError( E_SMI_OK );
        ok = true;
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
SmiEnvironment::ListDatabases( string& dbname )
{
  bool ok = false;
  dbname = "";
  try
  {
    Cursor& csr = SmiEnvironment::instance.impl->listCursor;
    if ( !SmiEnvironment::instance.impl->listStarted )
    {
      Connection& con = SmiEnvironment::instance.impl->usrConnection;
      string sql = string( "SELECT TABLE_NAME FROM USER_TABLES" ) +
                   " WHERE TABLE_NAME LIKE '%_TABLES' ORDER BY TABLE_NAME";
      csr.drop();
      con.execQuery( sql, csr );
      SmiEnvironment::instance.impl->listStarted = true;
    }
    if ( csr.fetch() )
    {
      dbname = csr.getStr( 0 );
      dbname = dbname.substr( 0, dbname.length() - 7 );
      ok = true;
    }
    else
    {
      csr.drop();
      SmiEnvironment::instance.impl->listStarted = false;
    }
    SetError( E_SMI_OK );
  }
  catch ( OraError &err )
  {
    SmiEnvironment::SetError( E_SMI_DB_NOTFOUND, err.message );
  }
  return (ok);
}

bool
SmiEnvironment::BeginTransaction()
{
  bool ok = false;
  if ( !instance.impl->txnStarted )
  {
    Connection& conUsr = SmiEnvironment::instance.impl->usrConnection;
    try
    {
      conUsr.transStart();
      SetError( E_SMI_OK );
      instance.impl->txnStarted = true;
      ok = true;
    }
    catch ( OraError &err )
    {
      SetError( E_SMI_TXN_BEGIN, err.message );
    }
  }
  else
  {
    SetError( E_SMI_TXN_RUNNING );
  }
  return (ok);
}

bool
SmiEnvironment::CommitTransaction()
{
  bool ok = false;
  if ( instance.impl->txnStarted )
  {
    // --- Close _all_ open cursors first!!!

    SmiEnvironment::Implementation::UpdateCatalog( true );
    Connection& conUsr = SmiEnvironment::instance.impl->usrConnection;
    try
    {
      conUsr.transCommit();
      SetError( E_SMI_OK );
      ok = true;
    }
    catch ( OraError &err )
    {
      SetError( E_SMI_TXN_COMMIT, err.message );
    }
    SmiEnvironment::Implementation::EraseFiles( ok );
    instance.impl->txnStarted = false;
  }
  else
  {
    SetError( E_SMI_TXN_NOTRUNNING );
  }
  return (ok);
}

bool
SmiEnvironment::AbortTransaction()
{
  bool ok = false;
  if ( instance.impl->txnStarted )
  {
    // --- Close _all_ open cursors first!!!

    SmiEnvironment::Implementation::UpdateCatalog( false );
    Connection& conUsr = SmiEnvironment::instance.impl->usrConnection;
    try
    {
      conUsr.transRollback();
      SetError( E_SMI_OK );
      ok = true;
    }
    catch ( OraError &err )
    {
      SetError( E_SMI_TXN_COMMIT, err.message );
    }
    SmiEnvironment::Implementation::EraseFiles( false );
    instance.impl->txnStarted = false;
  }
  else
  {
    SetError( E_SMI_TXN_NOTRUNNING );
  }
  return (ok);
}

bool
SmiEnvironment::InitializeDatabase()
{
  bool ok = false;
  bool seqCreated = false;
  Connection& con = SmiEnvironment::instance.impl->sysConnection;
  try
  {
    string sql = "CREATE SEQUENCE " + database +
                "_SEQUENCES START WITH 1 INCREMENT BY 1 NOCACHE";
    con.execUpdate( sql );
    seqCreated = true;
    ostringstream osNameLen;
    osNameLen << (2*SMI_MAX_NAMELEN+1);
    sql = "CREATE TABLE " + database + "_TABLES (" +
          "FILEID NUMBER(10) NOT NULL PRIMARY KEY, " +
          "FILENAME VARCHAR2(" + osNameLen.str() + ") NOT NULL UNIQUE, " +
          "ISKEYED VARCHAR2(1) DEFAULT 'N', " +
          "ISFIXED VARCHAR2(1) DEFAULT 'N' )"; 
    con.execUpdate( sql );
    SetError( E_SMI_OK );
    ok = true;
  }
  catch ( OraError &err )
  {
    if ( seqCreated )
    {
      try
      {
        string sql = "DROP SEQUENCE " + database + "_SEQUENCES";
        con.execUpdate( sql );
      }
      catch ( OraError &err )
      {
      }
    }
    SetError( E_SMI_DB_CREATE, err.message );
  }
  return (ok);
}

/* --- oraEnvironment.cpp --- */

