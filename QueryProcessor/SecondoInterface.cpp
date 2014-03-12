/*
----
This file is part of SECONDO.

Copyright (C) 2004-2008, University in Hagen, Faculty of Mathematics and Computer Science,
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

1 The Implementation-Module SecondoInterface

September 1996 Claudia Freundorfer

December 23, 1996 RHG Added error code information.

January 17, 1998 RHG Connected Secondo Parser (= command level 1 available).

This module implements the module ~SecondoInterface~ by using the
modules ~NestedList~, ~SecondoCatalog~, ~QueryProcessor~ and
~StorageManager~.

May 15, 1998 RHG Added a command ~model value-expression~ which is
analogous to ~query value-expression~ but computes the result model for
a given query rather than the result value.

November 18, 1998 Stefan User commands ``abort transaction'' and
``commit transaction'' are implemented by calling SMI\_Abort() and
SMI\_Commit(), respectively.

April 2002 Ulrich Telle Port to C++

August 2002 Ulrich Telle Set the current algebra level for SecondoSystem.

September 2002 Ulrich Telle Close database after creation.

November 7, 2002 RHG Implemented the ~let~ command.

December 2002 M. Spiekermann Changes in Secondo(...) and NumTypeExpr(...).

February 3, 2003 RHG Added a ~list counters~ command.

April 29 2003 Hoffmann Added save and restore commands for single objects.

April 29, 2003 M. Spiekermann bug fix in LookUpTypeExpr(...).

April 30 2003 Hoffmann Changes syntax for the restore objects command.

September 2003 Hoffmann Extended section List-Commands for Secondo-Commands
~list algebras~ and ~list algebra <algebra name>~.

October 2003 M. Spiekermann made the command echo (printing out the command in
NL format) configurable.  This is useful for server configuration, since the
output of big lists consumes more time than processing the command.

May 2004, M. Spiekermann. Support of derived objects (for further Information
see DerivedObj.h) introduced.  A new command derive similar to let can be used
by the user to create objects which are derived from other objects via a more
or less complex value expression. The information about dependencies is stored
in two system tables (relation objects). The save database command omits to
save list expressions for those objects.  After restoring all saved objects the
derived objects are rebuild in the restore database command.

August 2004, M. Spiekermann. The complex nesting of function ~Secondo~ has been reduced.

Sept 2004, M. Spiekermann. A bug in the error handling of restore databases has been fixed.

Dec 2004, M. Spiekermann. The new command ~set~ was implemented to support
interactive changes of runtime parameters.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

February 2006, M. Spiekermann. A proper handling of errors when commit or abort
transaction fails after command execution was implemented. Further, the scope of
variables in function Secondo was limited to a minimum, e.g. the declarations
were moved nearer to the usage. This gives more encapsulation and is easier to
understand and maintain.

April 2006, M. Spiekermann. Implementation of system tables SEC\_COUNTERS and SEC\_COMMANDS.

August 2006, M. Spiekermann. Bug fix for error messages of create or delete
database.  The error codes of the SMI module are now integrated into the error
reporting of the secondo interface. However, currently only a few SMI error
codes are mapped to strings.

September 2006, M. Spiekermann. System tables excluded into a separate file
called SystemTables.h.

April 2007, M. Spiekermann. Fixed bug concerning transaction management. Started
transactions of errorneous queries were not aborted.

\tableofcontents

*/

using namespace std;

#include <iostream>
#include <fstream>
#include <unistd.h>

//#define TRACE_ON 1
#undef TRACE_ON
#include "LogMsg.h"

#include "SecondoInterface.h"
#include "SecondoSystem.h"
#include "SecondoCatalog.h"
#include "SecondoConfig.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "Profiles.h"
#include "FileSystem.h"

#include "RelationAlgebra.h"
#include "StandardTypes.h"

#include "SecondoSMI.h"
#include "SecParser.h"

#include "StopWatch.h"
#include "Counter.h"
#include "DerivedObj.h"
#include "NList.h"

#include "CharTransform.h"
#include "version.h"

#include "DbVersion.h"

#include "ProgressView.h"
#include "Progress.h"
#include "AlmostEqual.h"
#include "WinUnix.h"

#ifdef SECONDO_ANDROID
#include "android/log.h"
#endif

extern bool USE_AUTO_BUFFER;

/*
The System Tables

Below there are implementations of system tables. More information
about them can be found in SystemInfoRel.h

*/

#include "SystemTables.h"
SystemTables* SystemTables::instance = 0;


// generic implementation of the << operator
ostream& operator<<(ostream& os, const InfoTuple& si)
{
  return si.print(os);
}

CmdTimesRel* cmdTimesRel = 0;
CmdCtrRel* cmdCtrRel = 0;
CacheInfoRel* cacheInfoRel = 0;
FileInfoRel* fileInfoRel = 0;
DerivedObjRel* devObjRel = 0;
TypeInfoRel* typeInfoRel = 0;
OperatorInfoRel* operatorInfoRel = 0;
OperatorUsageRel* operatorUsageRel = 0;


extern AlgebraListEntry& GetAlgebraEntry( const int j );

static SecondoSystem* ss = 0;

Symbols sym;


/**************************************************************************

1 Implementation of class SecondoInterface

*/


/*

1.1 Constructor

*/

SecondoInterface::SecondoInterface(bool isServer/* =false*/)
{
  Init();

  serverInstance = isServer;
}


/*

1.2 Destructor

*/

SecondoInterface::~SecondoInterface()
{
  if ( initialized )
  {
    Terminate();
  }
}



/*

1.3 Initialize method

*/

bool
SecondoInterface::Initialize( const string& user, const string& pswd,
                              const string& host, const string& port,
                              string& parmFile, string& errorMsg,
                              const bool multiUser
                              )
{
  bool ok = false;
#ifdef SECONDO_ANDROID
        __android_log_print(ANDROID_LOG_INFO,
	"FU", "Initializing the Secondo on Android Interface ...");
#else

  cout << endl << "Initializing the SECONDO Interface ..." << endl;
#endif

  stringstream version;
  version << "Version: " << SECONDO_VERSION_MAJOR << "."
                         << SECONDO_VERSION_MINOR << "."
			 << SECONDO_VERSION_REVISION << endl;

#ifndef SECONDO_ANDROID
  cout << version.str() << endl;
#else
        __android_log_print(ANDROID_LOG_INFO,
	"FU", "Secondo Version %s\n",version.str().c_str());
#endif



  stringstream dbversion;
  dbversion  << "Berkeley DB version: "
             << DB_VERSION_MAJOR
             << "."
             << DB_VERSION_MINOR;

#ifndef SECONDO_ANDROID
  cout << dbversion.str() << endl  << endl;
#else
        __android_log_print(ANDROID_LOG_INFO,
		"FU", "%s\n",dbversion.str().c_str());
#endif


  // initialize runtime flags
  InitRTFlags(parmFile);

  string value, foundValue;
  if ( SmiProfile::GetParameter( "Environment",
                                 "SecondoHome", "", parmFile ) == "" )
  {
#ifndef SECONDO_ANDROID
    cout << "Error: Secondo home directory not specified." << endl;
#else
        __android_log_print(ANDROID_LOG_ERROR,
	"FU", "Secondo Home directory not specified");
#endif

    errorMsg += "Secondo home directory not specified\n";
  }
  else
  {
#ifdef SECONDO_ANDROID
        __android_log_print(ANDROID_LOG_INFO,"FU", "Secondo Home directory ok");
#endif

    ok = true;
  }

  // check username and password if enabled
  if(RTFlag::isActive("SI:UsePasswd")){
     // get the name of the file containing the passwords
     string passwd = SmiProfile::GetParameter( "Environment",
                                               "PASSWD_FILE",
                                               "passwd",
                                                parmFile );
     cout << "try to open file '" << passwd << "'" << endl;
     ifstream pwd(passwd.c_str());
     if(!pwd){
       cmsg.error() << "password file not found" << endl;
       cmsg.send();
       errorMsg += "password file not found\n";
       ok = false;
     }
     string line;
     bool userok = false;

    while(!pwd.eof()&& !userok){
       pwd >> line;
       string::size_type posOfSpace = line.find(':');
       if (posOfSpace != string::npos) {
         string u = line.substr(0,posOfSpace);
         string p = line.substr(posOfSpace+1);
         if(u==user){
            userok = true;
            if(p==pswd){
              cmsg.info() << "accept user " << u << endl;
              cmsg.send();
            } else {
              cmsg.error() << "password for user " << u << " is wrong" << endl;
              errorMsg += "wrong password for user " + u + "\n";
              cmsg.send();
              ok = false;
            }
         }
       }
     }
     if(!userok){
       cmsg.error() << "user " << user << " is not valid " << endl;
       cmsg.send();
       errorMsg += "user " + user + " is not known\n";
       ok =  false;
     }
     pwd.close();
  }

  string progressConstantsFileDefault="ProgressConstants.csv";
#ifndef SECONDO_ANDROID 
  string sbd = getenv("SECONDO_BUILD_DIR");
  if(sbd.length()>0){
     progressConstantsFileDefault = sbd + CFile::pathSep + "bin"
                                    + CFile::pathSep
                                    + progressConstantsFileDefault;
  }
#else
	string sbd = parmFile.substr(0,parmFile.find_last_of("\\/"));
        //string sbd ="/data/data/de.fernunihagen.dna.Secondo4Android";
        progressConstantsFileDefault = sbd 
		+ CFile::pathSep + progressConstantsFileDefault;
#endif

  string progressConstantsFile = SmiProfile::GetParameter(
                                      "ProgressEstimation",
                                      "PROGRESS_CONSTANTS_FILE",
                                      progressConstantsFileDefault,
                                      parmFile);
  trim(progressConstantsFile);
  expandVarRef(progressConstantsFile);

  bool progressConstantsOk = 
            ProgressConstants::readConstants(progressConstantsFile);

  if(!progressConstantsOk){
     cmsg.error() << "Problem in reading progress constants" << endl
                  << "File " << progressConstantsFile << " missing or corrupt"
                  << endl;
     cmsg.send();

     WinUnix::sleep(10);
  }


  // create directory tmp below current dir
#ifndef SECONDO_ANDROID 
 string tempDir("tmp");
#else
	string tempDirPath = parmFile.substr(0,
		parmFile.find_last_of("\\/"));	
        string tempDir(tempDirPath+"tmp");
#endif

  if (! FileSystem::FileOrFolderExists(tempDir) )
  {
    if (! FileSystem::CreateFolder(tempDir) )
    {
      cmsg.error() << "Could not create directory " << tempDir << endl;
      cmsg.send();
      ok = false;
      errorMsg += "Could not create directory " + tempDir +"\n";
#ifdef SECONDO_ANDROID
        __android_log_print(ANDROID_LOG_INFO,"FU", 
		"Could not Create Directory %s",tempDir.c_str());
#endif

    }
  }

  if ( ok )
  {
    // --- Check storage management interface
    SmiEnvironment::RunMode mode =  SmiEnvironment::SingleUser;
    if ( RTFlag::isActive("SMI:NoTransactions") ) {

       mode = SmiEnvironment::SingleUserSimple;
       cout << "  SmiEnvironment mode  = SingleUserSimple"
	       " (transaction submodule disabled)" << endl;

    } else { // Transactions and logging are used

       mode = (multiUser) ? SmiEnvironment::MultiUser
                          : SmiEnvironment::SingleUser;
    }

    if ( SmiEnvironment::StartUp( mode, parmFile, cout ) )
    {
      SmiEnvironment::SetUser( user ); // TODO: Check for valid user/pswd
      ok = true;
      if(RTFlag::isActive("SMI:AutoRemoveLogs")){
         SmiEnvironment::SetAutoRemoveLogs(true);
      } else {
         SmiEnvironment::SetAutoRemoveLogs(false);
      }
    }
    else
    {
      cerr << "Fatal Error: Could not start up SMI! "
           << "Check configuration parameters." << endl;
      errorMsg += "Fatal Error: Could not start up SMI!,"
                  " Check configuration parameters.\n" ;
      return false;
    }
  }

  // set list memory
  long nodeMem = 2048;
  nodeMem =
    SmiProfile::GetParameter("QueryProcessor", "NodeMem",
                             nodeMem, parmFile);

  long stringMem = 1024;
  stringMem =
    SmiProfile::GetParameter("QueryProcessor", "StringMem",
                             stringMem, parmFile);

  long textMem = 1024;
  textMem =
    SmiProfile::GetParameter("QueryProcessor", "TextMem",
                             textMem, parmFile);


  size_t native_flobcache_maxSize = 0;
  size_t native_flobcache_slotSize = 0;
  size_t native_flobcache_avgSize = 0;

  size_t persistent_flobcache_maxSize = 0;
  size_t persistent_flobcache_slotSize = 0;
  size_t persistent_flobcache_avgSize = 0;

  native_flobcache_maxSize =
     SmiProfile::GetParameter("FlobCache", "Native_MaxSize", 0, parmFile);
  native_flobcache_slotSize =
     SmiProfile::GetParameter("FlobCache", "Native_SlotSize", 0, parmFile);
  native_flobcache_avgSize =
     SmiProfile::GetParameter("FlobCache", "Native_AvgSize", 0, parmFile);


  if(native_flobcache_maxSize >0 && native_flobcache_slotSize >0 &&
     native_flobcache_avgSize > 0){
    if(native_flobcache_slotSize > native_flobcache_maxSize ||
        native_flobcache_avgSize > native_flobcache_slotSize ){
      cout << "conflicting values for native flob cache found";
    } else {
       Flob::SetNativeCache(native_flobcache_maxSize,
                            native_flobcache_slotSize,
                            native_flobcache_avgSize);
    }

  }


  persistent_flobcache_maxSize =
     SmiProfile::GetParameter("FlobCache", "Persistent_MaxSize", 0, parmFile);
  persistent_flobcache_slotSize =
     SmiProfile::GetParameter("FlobCache", "Persistent_SlotSize", 0, parmFile);
  persistent_flobcache_avgSize =
     SmiProfile::GetParameter("FlobCache", "Persistent_AvgSize", 0, parmFile);


  if(persistent_flobcache_maxSize >0 && persistent_flobcache_slotSize >0 &&
     persistent_flobcache_avgSize > 0){
    if(persistent_flobcache_slotSize > persistent_flobcache_maxSize ||
        persistent_flobcache_avgSize > persistent_flobcache_slotSize ){
      cout << "conflicting values for persistent flob cache found";
    } else {
       Flob::SetPersistentCache(persistent_flobcache_maxSize,
                            persistent_flobcache_slotSize,
                            persistent_flobcache_avgSize);
    }

  }

  if (ok)
  {
    cout << "Initializing the SECONDO System ... " << endl;
    bool rc = SecondoSystem::CreateInstance( &GetAlgebraEntry );
    assert(rc);
    ss = SecondoSystem::GetInstance();

    nl = SecondoSystem::GetNestedList();
    nl->setMem(nodeMem, stringMem, textMem);
    nl->initializeListMemory();
    NList::setNLRef(nl);

    cmsg.info() << "Kernels List Memory:" << endl
              << "  NodeMem = " << nodeMem
	      << " / slots = " << nl->nodeEntries << endl
              << "  StringMem = " << stringMem
	      << " / slots = " << nl->stringEntries << endl
              << "  TextMem = " << textMem
	      << " / slots = " << nl->textEntries << endl
	      << endl;
    cmsg.send();

    al = SecondoSystem::GetAppNestedList();
    ok = SecondoSystem::StartUp();
  }

  // Print type sizes and check requirements for AlmostEqual implementations:
  ok = (ok && AlmostEqual_CheckTypeSizes());

  if(ok){
    // set the maximum memory which may be allocated by operators
    QueryProcessor& qp = *SecondoSystem::GetQueryProcessor();


    long keyVal =
      SmiProfile::GetParameter("QueryProcessor", "MaxMemPerOperator",
                               16 * 1024, parmFile);   // in kB
    qp.SetMaxMemPerOperator(keyVal*1024);   // in bytes
    cmsg.info() << "Memory usage per operator limited by "
                << keyVal << "kb" << endl;


    size_t globalMem =
      SmiProfile::GetParameter("QueryProcessor", "GlobalMemory",
                               512, parmFile);     // in MB
    qp.SetGlobalMemory(globalMem);   // in MB
    cmsg.info() << "Global memory limited by "
                << globalMem << " MB" << endl;

    keyVal =
      SmiProfile::GetParameter("System", "FLOBCacheSize",
                               16*1024, parmFile);
    //SecondoSystem::InitializeFLOBCache( keyVal*1024 );

    cmsg.info() << "FLOB Cache size "
                << keyVal << "kb" << endl;
    cmsg.send();

    SystemTables& st = SystemTables::getInstance();

    // create sytem tables
    cmdTimesRel = new CmdTimesRel("SEC2COMMANDS");
    cmdCtrRel = new CmdCtrRel("SEC2COUNTERS");
    cacheInfoRel = new CacheInfoRel("SEC2CACHEINFO");
    fileInfoRel = new FileInfoRel("SEC2FILEINFO");
    typeInfoRel = new TypeInfoRel("SEC2TYPEINFO");
    operatorInfoRel = new OperatorInfoRel("SEC2OPERATORINFO");
    operatorUsageRel = new OperatorUsageRel("SEC2OPERATORUSAGE");

    // The next table is currently only a dummy. This is necessary
    // that the catalog recognizes it as a system table. In the future
    // I try to change the implementation of class ~DerivedObj~ in order
    // to make it to a subclass of SystemInfoRel
    devObjRel = new DerivedObjRel("SEC_DERIVED_OBJ");

    st.insert(cmdCtrRel);
    st.insert(cmdTimesRel);
    st.insert(cacheInfoRel);
    st.insert(fileInfoRel);
    st.insert(typeInfoRel);
    st.insert(operatorInfoRel);
    st.insert(operatorUsageRel);
    st.insert(devObjRel);

    // add size information into typeInfoRel
    SecondoCatalog& ctlg = *SecondoSystem::GetCatalog();
    ctlg.Initialize(typeInfoRel);
    ctlg.Initialize(operatorInfoRel);
  } else {
    errorMsg += "Problem in SecondoSystem::StartUp()\n";
  }

  initialized = ok;
  return (ok);
}


/*

1.4 Terminate method

*/


void
SecondoInterface::Terminate()
{
  static bool traceHandles =
	        RTFlag::isActive("SMI:traceHandles") ? true : false;
  const string bullet(" - ");
  if ( initialized )
  {

    if (traceHandles) {
      cmsg.info() << "Terminating the secondo interface instance ..." << endl;
      cmsg.send();
    }
    // --- Abort open transaction, if there is an open transaction
    if ( activeTransaction )
    {
      SecondoSystem::AbortTransaction();
      activeTransaction = false;
    }

    if ( derivedObjPtr != 0 ) { // The destructor closes a relation object
      if (traceHandles) {
        cmsg.info() << "Closing system tables ..." << endl;
        cmsg.send();
      }
      delete derivedObjPtr;
      derivedObjPtr = 0;
    }

    assert ( ss != 0 );
    delete ss;
    ss = 0;

    initialized = false;
    activeTransaction = false;
    nl = 0;
    al = 0;
    server = 0;
  }
  else
  {
    cmsg.error() << bullet << "Error: Secondo interface already terminated."
                 << endl;
  }
  cmsg.info() << endl;
  cmsg.send();
}


/*

1.5 The Secondo method. The Interface for applications.

*/



void
SecondoInterface::Secondo( const string& commandText,
                           const ListExpr commandLE,
                           const int commandLevel,
                           const bool commandAsText,
                           const bool resultAsText,
                           ListExpr& resultList,
                           int& errorCode,
                           int& errorPos,
                           string& errorMessage,
                           const string& resultFileName /*="SecondoResult"*/,
                           const bool isApplicationLevelCommand/* = true */ )
{
/*

~Secondo~ reads a command and executes it; it possibly returns a result.
The command is one of a set of SECONDO commands.

Error Codes: see definition module.

If ~errorCode~ 0 is returned, the command was executed without error.

To avoid redundant maintainance of error messages in the C++ code and in the
Java-GUI, the errorcode will not longer be used and code -1 will be returned
in case of an error. All error messages should be stored in the ErrorReporter
as a formatted string with linebreaks and this string will be returned in
return parameter ~errorMessage~.

Messages send to ~cerr~ or ~cout~ should be avoided, since they can not be
transferred to the client. If necessary please use the ~cmsg~ object explained
in the file "include/LogMsg.h"

Please create new functions for implementation of new commands, since this
function contains already many lines of code. Hence, some commands have been
moved to separate functions. This kind of functions should be named Command\_<name>.

*/

 #ifdef SECONDO_ANDROID
        __android_log_print(ANDROID_LOG_INFO,"FU", "Starting Secondo(...)");
        __android_log_print(ANDROID_LOG_INFO,"FU", 
		"Command: %s", commandText.c_str());

#endif 


  //assert( SmiEnvironment::GetNumOfErrors() == 0 );
  SHOW(activeTransaction)

  // reset errors
  SmiEnvironment::ResetSmiErrors();
  cmsg.resetErrors();
  errorMessage = "";
  errorCode    = 0;
  errorPos     = 0;

  resultList   = nl->TheEmptyList();

  NestedList* nl = SecondoSystem::GetNestedList();
  NestedList* al = SecondoSystem::GetAppNestedList();
  NList::setNLRef(nl);

  // copy command list to internal NL memory
  ListExpr commandLE2 = nl->TheEmptyList();
  if (!commandAsText && commandLE){
    if(!isApplicationLevelCommand){// use own NL storage
//       commandLE2 = commandLE; // XRIS original code
      commandLE2 = nl->CopyList(commandLE, nl); //  XRIS new code
    } else { // copy from application's NL storage
      commandLE2 = al->CopyList(commandLE, nl);
    }
  }

  // The error list which may be extended by some catalog commands
  ListExpr errorList    = nl->GetErrorList();
  ListExpr errorInfo    = errorList;

  // the next variable stores the command as a nested list data structure
  ListExpr list = nl->TheEmptyList();

  switch (commandLevel)
  {
    case 0:  // executable, list form
    {
      if ( commandAsText )
      {
        if ( !nl->ReadFromString( commandText, list ) )
        {
          errorCode = ERR_SYNTAX_ERROR;  // syntax error in command/expression
        }
      }
      else
      {
        list = commandLE2;
      }
      break;
    }
    case 1:  // executable, text form
    {
      SecParser sp;            // translates SECONDO syntax into nested list
      USE_AUTO_BUFFER = RTFlag::isActive("SI:AUTO_BUFFER");
      string listCommand = ""; // buffer for command in list form

      if ( sp.Text2List( commandText, listCommand ) != 0 )
      {
        // conversion into a textual nested list failed
        errorCode = ERR_SYNTAX_ERROR;
      }
      else if ( !nl->ReadFromString( listCommand, list ) )
      {
        errorCode = ERR_SYNTAX_ERROR;
      }
      break;
    }
    default:
    {
      errorCode = ERR_CMD_LEVEL_NOT_YET_IMPL;  // Command level not implemented
      break;
    }
  } // end of switch

  if (!RTFlag::isActive("SI:NoCommandEcho"))
  {
     nl->WriteListExpr(list, cerr);
     cerr << endl;
  }

  if ( errorCode != 0 )
  {
     //cerr << "aborting command execution ..." << endl;
     //abort command execution
     constructErrMsg(errorCode, errorMessage);
     Flob::killAllNativeFlobs();
     return;
  }

  // Initialize Counters for the creation and deletion of objects
  bool relStatistics = RTFlag::isActive("SI:RelStatistics");
  Tuple::InitCounters( relStatistics );
  Attribute::InitCounters( relStatistics );

  bool stdStatistics = RTFlag::isActive("STD:Statistics");
  StdTypes::InitCounters( stdStatistics );


  //*** START COMMAND RECOGNITION ***//

  // measure the time used for executing the command.

  printQueryAnalysis = RTFlag::isActive("SI:PrintQueryAnalysis");

  cmdTime.start();
  cmdReal = 0;
  cmdCPU = 0;
  commitReal = 0;
  queryReal = 0;
  queryCPU = 0;
  outObjReal = 0;
  copyReal = 0;

  int length = nl->ListLength( list );
  if ( length > 1 )
  {
    ListExpr first = nl->First( list );

    // use simpler progromming interface "NList.h"
    NList nlist(list);
    NList nfirst(first);

    // strings needed for catalog commands
    string filename = "", dbName = "", objName = "", typeName = "";

    bool fc=true; // stores if call to FinishCommand was successful

    // local references of important objects
    QueryProcessor& qp = *SecondoSystem::GetQueryProcessor();
    SecondoCatalog& ctlg = *SecondoSystem::GetCatalog();
    SecondoSystem& sys = *SecondoSystem::GetInstance();
    AlgebraManager& am = *SecondoSystem::GetAlgebraManager();

    // write textual command to progress protocol file
    if ( !qp.progressView ) qp.progressView = new ProgressView();
    qp.progressView->WriteCommandToProtocol( commandText );


    // --- Transaction handling

    if ( nl->IsEqual( nl->Second( list ), "transaction" ) && (length == 2) )
    {
      if ( nl->IsEqual( first, "begin" ) )
      {
        if ( !activeTransaction )
        {
          if ( SecondoSystem::BeginTransaction() )
          {
            activeTransaction = true;
          }
          else
          {
            errorCode = ERR_BEGIN_TRANSACTION_FAILED;
          }
        }
        else // nested transactions are not supported
        {
          errorCode = ERR_TRANSACTION_ACTIVE;
        }
      }
      else if ( nl->IsEqual( first, "commit" ) )
      {
        if ( activeTransaction )
        {
          if ( !SecondoSystem::CommitTransaction() )
          {
            errorCode = ERR_COMMIT_OR_ABORT_FAILED;
          }
          activeTransaction = false;
        }
        else
        {
          errorCode = ERR_NO_TRANSACTION_ACTIVE;
        }
      }
      else if ( nl->IsEqual( first, "abort" ) )
      {
        if ( activeTransaction )
        {
          if ( !SecondoSystem::AbortTransaction() )
          {
            errorCode = ERR_COMMIT_OR_ABORT_FAILED;
          }
          activeTransaction = false;
        }
        else
        {
          errorCode = ERR_NO_TRANSACTION_ACTIVE;
        }
      }
      else
      {
        errorCode = ERR_CMD_NOT_RECOGNIZED;  // Command not recognized.
      }
    }

    // --- Database commands

    else if ( nl->IsEqual( nl->Second( list ), "database" ) )
    {
      if ( nl->IsEqual( first, "create" ) && (length == 3) &&
           nl->IsAtom( nl->Third( list ) ) &&
          (nl->AtomType( nl->Third( list ) ) == SymbolType) )
      {
        if ( sys.IsDatabaseOpen() )
        {
          errorCode = ERR_DATABASE_OPEN;  // a database is open
        }
        else
        {
          dbName = nl->SymbolValue( nl->Third( list ) );
          if ( sys.CreateDatabase( dbName ) )
          {
            sys.CloseDatabase();
          }
          else
          {
            errorCode = ERR_CREATE_DATABASE;   // create db failed
          }
        }
      }
      else if ( nl->IsEqual( first, "delete" ) &&
               (length == 3) && nl->IsAtom( nl->Third( list ) ) &&
               (nl->AtomType( nl->Third( list ) ) == SymbolType) )
      {
        if ( sys.IsDatabaseOpen() )
        {
          errorCode = ERR_DATABASE_OPEN;  // a database is open
        }
        else
        {
          dbName = nl->SymbolValue( nl->Third( list ) );
          if ( !sys.DestroyDatabase( dbName ) )
          {
            // identifier not a known database name
            errorCode = ERR_DELETE_DATABASE;
          }
        }
      }
      else if ( nl->IsEqual( first, "open" ) &&
               (length == 3) && nl->IsAtom( nl->Third( list ) ) &&
               (nl->AtomType( nl->Third( list ) ) == SymbolType) )
      {
        if ( sys.IsDatabaseOpen() )
        {
          errorCode = ERR_DATABASE_OPEN;  // a database is open
        }
        else
        {
          dbName = nl->SymbolValue( nl->Third( list ) );
	  errorCode = sys.OpenDatabase( dbName );
          if ( errorCode == ERR_NO_ERROR )
          {
            // create new instance if necessary
            derivedObjPtr = new DerivedObj();
          }
        }
      }
      else if ( nl->IsEqual( first, "close" ) )
      {
        if ( !sys.IsDatabaseOpen() )
        {
          errorCode = ERR_NO_DATABASE_OPEN;  // no database open
        }
        else
        {
          if ( activeTransaction )
          {
            SecondoSystem::CommitTransaction();
            activeTransaction = false;
          }
          sys.CloseDatabase();

          delete derivedObjPtr;
          derivedObjPtr = 0;
        }
        Flob::clearCaches();
      }
      else if ( nfirst.isEqual("save") &&
                nlist.hasLength(4) &&
                nlist.elem(3).isEqual("to") &&
                ( nlist.isSymbol(4) || nlist.isText(4)) )
      {
        if ( !sys.IsDatabaseOpen() )
        {
          errorCode = ERR_NO_DATABASE_OPEN;  // no database open
        }
        else
        {
          StartCommand();
          filename = nlist.elem(4).str();

          if ( !sys.SaveDatabase( expandVar(trim(filename)), *derivedObjPtr ) )
          {
            // Problem in writing to file
            errorCode = ERR_PROBLEM_IN_WRITING_TO_FILE;
          }
          fc = FinishCommand( errorCode, errorMessage );
        }
      }
      else if ( nfirst.isEqual("restore") &&
                nlist.hasLength(5) &&
                nlist.isSymbol(3) &&
                nlist.elem(4).isEqual("from") &&
                ( nlist.isSymbol(5) || nlist.isText(5) )  )
      {
        if ( sys.IsDatabaseOpen() )
        {
          errorCode = ERR_DATABASE_OPEN;  // database open
        }
        else
        {
          dbName = nlist.third().str();
          filename = nlist.fifth().str();
          errorCode = sys.RestoreDatabase( dbName,
                                           expandVar(trim(filename)),
                                           errorInfo                  );
        }

        switch ( errorCode ) // error handling
        {
          case ERR_NO_ERROR:
          case ERR_DATABASE_OPEN:
          case ERR_IDENT_UNKNOWN_DB_NAME:
          case ERR_PROBLEM_IN_READING_FILE:
          case E_SMI_DB_INVALIDNAME:
          {
            break;
          }
          case ERR_IN_DEFINITIONS_FILE:
          {
            resultList = errorList;
          }
          case ERR_DB_NAME_NEQ_IDENT:
          case ERR_IN_LIST_STRUCTURE_IN_FILE:

          default:
          {
            cmsg.info()
              << "Error during restore detected. Trying to create "
	            << "derived objects ..." << endl;
            cmsg.send();

            delete derivedObjPtr;
            derivedObjPtr = new DerivedObj();
            derivedObjPtr->rebuildObjs();

	    cmsg.info()
	      << "About to close the database "
              << dbName << " ... ";
            cmsg.send();

            if ( !sys.CloseDatabase() )
            {
              cmsg.info() << " [ERROR]!" << endl;
            }
            else
            {
              cmsg.info() << " [OK]!" << endl;
            }
            cmsg.info() << endl;
            cmsg.send();
            break;
          }
        }

        if ( !errorCode )
        { // rebuild derived objects if present
          delete derivedObjPtr;
          derivedObjPtr = new DerivedObj();
          derivedObjPtr->rebuildObjs();
        }
      }
      else
      {
        errorCode = ERR_CMD_NOT_RECOGNIZED;  // Command not recognized.
      }

    }

    // --- Writing command for objects

    else if ( nfirst.isEqual("save") &&
              nlist.hasLength(4) &&
              nlist.elem(3).isEqual("to") &&
              nlist.isSymbol(2) &&
              (nlist.isText(4) || nlist.isSymbol(4)) )
    {
      filename = nlist.elem(4).str();
      string objectname = nlist.elem(2).str();

      if ( !sys.IsDatabaseOpen() )
      {
        errorCode = ERR_NO_DATABASE_OPEN;  // no database open
      }
      else
      {
        if ( !sys.IsDatabaseObject( objectname ) )
        {
          // object is not known in the database
          errorCode = ERR_IDENT_UNKNOWN_DB_OBJECT;
        }
        else
        {
          StartCommand();
          if ( !sys.SaveObject( objectname, expandVar(trim(filename)) ) )
          {
            // Problem in writing to file
            errorCode = ERR_PROBLEM_IN_WRITING_TO_FILE;
          }
          fc = FinishCommand( errorCode, errorMessage );
        }
      }
    }

    // --- Reading command for objects

    else if ( nfirst.isEqual("restore") &&
              nlist.hasLength(4) &&
              nlist.isSymbol(2) &&
              nlist.third().isEqual("from") &&
              ( nlist.isSymbol(4) || nlist.isText(4) ) )
    {
      if ( !sys.IsDatabaseOpen() )
      {
        errorCode = ERR_NO_DATABASE_OPEN;  // no database open
      }
      else
      {
        objName = nlist.second().str();
        filename = nlist.fourth().str();

        if ( sys.IsDatabaseObject( objName ) )
        {
          // object is already known in the database
          errorCode = ERR_IDENT_ALREADY_KNOWN_IN_DB;
        }
        else
        {
          int message = sys.RestoreObjectFromFile( objName,
                                               expandVar(trim(filename)),
                                               errorInfo                   );
          // the command above should return these values directly!
          switch (message)
          {
            case 0:
              break;
            case 1:
              // object name in file different from identifier
              errorCode = ERR_OBJ_NAME_IN_FILE_NEQ_IDENT;
              break;
            case 2:
              // problem in reading the file
              errorCode = ERR_PROBLEM_IN_READING_FILE;
              break;
            case 3:
              // error in list structure in file
              errorCode = ERR_IN_LIST_STRUCTURE_IN_FILE;
              break;
            case 4:
              // error in object definition in file
              errorCode = ERR_IN_DEFINITIONS_FILE;
              resultList = errorList;
              break;
            default:
              errorCode = ERR_CMD_NOT_RECOGNIZED;   // Should never occur
              break;
          }
        }
      }
    }

    // --- List commands

    else if ( nl->IsEqual( first, "list" ) )
    {
      if ( nl->IsEqual( nl->Second( list ), "algebras" ) && (length == 2) )
      {
        resultList = nl->TwoElemList(nl->SymbolAtom("inquiry"),
                                     nl->TwoElemList(nl->SymbolAtom("algebras"),
                                                     am.ListAlgebras() ));
      }
      else if ( nl->IsEqual( nl->Second( list ), "algebra" )
                && (length == 3)
                &&  nl->IsAtom(nl->Third(list))
                &&  nl->AtomType( nl->Second( list ) ) == SymbolType )
      {
        if ( am.GetAlgebraId( nl->SymbolValue( nl->Third(list) )))
        {
          int aid = am.GetAlgebraId( nl->SymbolValue( nl->Third(list) ));

          ListExpr constOp =  nl->TwoElemList( ctlg.ListTypeConstructors( aid ),
                                               ctlg.ListOperators( aid ));

          resultList =
            nl->TwoElemList( nl->SymbolAtom("inquiry"),
                             nl->TwoElemList( nl->SymbolAtom("algebra"),
                                              nl->TwoElemList( nl->Third(list),
                                                               constOp )));
        }
        else errorCode = ERR_ALGEBRA_UNKNOWN;
      }
      else if ( nl->IsEqual( nl->Second( list ), "type" )
                && (length == 3)
                &&  nl->IsEqual( nl->Third( list ), "constructors" ) )
      {
        resultList = nl->TwoElemList(
                           nl->SymbolAtom("inquiry"),
                           nl->TwoElemList( nl->SymbolAtom("constructors"),
                                            ctlg.ListTypeConstructors() ));
      }
      else if ( nl->IsEqual( nl->Second(list), "operators" ) )
      {
        resultList = nl->TwoElemList(
                           nl->SymbolAtom("inquiry"),
                           nl->TwoElemList( nl->SymbolAtom("operators"),
                                            ctlg.ListOperators() ));
      }
      else if ( nl->IsEqual( nl->Second( list ), "databases" ) )
      {
        resultList = nl->TwoElemList(
                           nl->SymbolAtom("inquiry"),
                           nl->TwoElemList( nl->SymbolAtom("databases"),
                                            sys.ListDatabaseNames() ));
      }
      else if ( nl->IsEqual( nl->Second( list ), "types") )
      {
        if ( !sys.IsDatabaseOpen() )
        {
          errorCode = ERR_NO_DATABASE_OPEN;  // no database open
        }
        else
        {
          StartCommand();
          resultList = nl->TwoElemList( nl->SymbolAtom("inquiry"),
          nl->TwoElemList( nl->SymbolAtom("types"),
                           ctlg.ListTypes() ));
          fc = FinishCommand( errorCode, errorMessage );
        }
      }
      else if ( nl->IsEqual( nl->Second( list ), "objects" ) )
      {
        if ( !sys.IsDatabaseOpen() )
        {
          errorCode = ERR_NO_DATABASE_OPEN;  // no database open
        }
        else
        {
          StartCommand();
          resultList = nl->TwoElemList( nl->SymbolAtom("inquiry"),
          nl->TwoElemList( nl->SymbolAtom("objects"),
                           ctlg.ListObjects() ));
          fc = FinishCommand( errorCode, errorMessage );
        }
      }
      else if ( nl->IsEqual( nl->Second( list ), "counters" ) )
      {
        if ( !sys.IsDatabaseOpen() )
        {
          errorCode = ERR_NO_DATABASE_OPEN;  // no database open
        }
        else
        {
          StartCommand();
          resultList = qp.GetCounters();
          fc = FinishCommand( errorCode, errorMessage );
        }
      }
      else
      {
        errorCode = ERR_CMD_NOT_RECOGNIZED;  // Command not recognized.
      }
    }

    // --- Type definition

    else if ( nl->IsEqual( first, "type" ) &&
             (length == 4) && nl->IsAtom( nl->Second( list ) ) &&
             (nl->AtomType( nl->Second( list )) == SymbolType) &&
              nl->IsEqual( nl->Third( list ), "=" ) )
    {
      if ( sys.IsDatabaseOpen() )
      {
        StartCommand();
        typeName = nl->SymbolValue( nl->Second( list ) );
        ListExpr typeExpr = nl->Fourth( list );
        ListExpr typeExpr2 = ctlg.ExpandedType( typeExpr );

        if ( ctlg.KindCorrect( typeExpr2, errorInfo ) )
        {
          if ( !ctlg.InsertType( typeName, typeExpr2 ) )
          {
            errorCode = ERR_IDENT_USED;  // identifier already used
          }
        }
        else
        {
          errorCode = ERR_NO_TYPE_DEFINED;     // Wrong type expression
          resultList = errorList;
        }
        fc = FinishCommand( errorCode, errorMessage );
      }
      else
      {
        errorCode = ERR_NO_DATABASE_OPEN;       // no database open
      }
    }

    // delete type

    else if ( nl->IsEqual( first, "delete" ) )
    {
      if ( (length == 3) && nl->IsAtom( nl->Third( list ) ) &&
           (nl->AtomType( nl->Third( list )) == SymbolType) &&
            nl->IsEqual( nl->Second( list ), "type" ) )
      {
        if ( sys.IsDatabaseOpen() )
        {
          StartCommand();
          typeName = nl->SymbolValue( nl->Third( list ) );
          int message = ctlg.DeleteType( typeName );
          if ( message == 1 )
          {
            errorCode = ERR_TYPE_NAME_USED_BY_OBJ;   // Type used by an object
          }
          else if ( message == 2 )
          {
            // identifier not a known type name
            errorCode = ERR_IDENT_UNKNOWN_TYPE;
          }
          fc = FinishCommand( errorCode, errorMessage );
        }
        else
        {
          errorCode = ERR_NO_DATABASE_OPEN;      // no database open
        }
      }

      // delete object

      else if ( (length == 2) && nl->IsAtom( nl->Second( list )) &&
                (nl->AtomType( nl->Second( list )) == SymbolType) )
      {
        objName = nl->SymbolValue( nl->Second( list ) );

        if ( !sys.IsDatabaseOpen() )
        {
          errorCode = ERR_NO_DATABASE_OPEN;  // no database open
        }
        else if ( !errorCode && ctlg.IsSystemObject(objName) )
        {
           errorCode = ERR_IDENT_RESERVED;
        }

        StartCommand();
        if ( !errorCode )
        {
           if( !ctlg.DeleteObject( objName ) )
           {
             // identifier not a known object name
             errorCode = ERR_IDENT_UNKNOWN_OBJ;
           }
           else
           {
             // delete from derived objects table if necessary
             derivedObjPtr->deleteObj( objName );
           }
         }
         fc = FinishCommand( errorCode, errorMessage );

      }
      else
      { // no correct delete syntax
        errorCode = ERR_CMD_NOT_RECOGNIZED;  // Command not recognized
      }
    }

    // kill object

    else if ( length == 2 &&
              nl->IsEqual( first, "kill" ) &&
              nl->IsAtom( nl->Second( list )) &&
              nl->AtomType( nl->Second( list ) ) == SymbolType )
    {
      objName = nl->SymbolValue( nl->Second( list ) );

      if ( !sys.IsDatabaseOpen() )
      {
        errorCode = ERR_NO_DATABASE_OPEN;  // no database open
      }
      else if ( !errorCode && ctlg.IsSystemObject(objName) )
      {
         errorCode = ERR_IDENT_RESERVED;
      }

      StartCommand();
      if ( !errorCode )
      {
         if( !ctlg.KillObject( objName ) )
         {

           // identifier not a known object name
           errorCode = ERR_IDENT_UNKNOWN_OBJ;
         }
         else
         {
           // delete from derived objects table if necessary
           derivedObjPtr->deleteObj( objName );
         }
       }
       fc = FinishCommand( errorCode, errorMessage );
    }

    // --- Create object command

    else if ( nl->IsEqual( first, "create" ) && (length == 4) &&
              nl->IsAtom( nl->Second( list )) &&
             (nl->AtomType( nl->Second( list ) ) == SymbolType) &&
              nl->IsEqual( nl->Third( list ), ":" ) )
    {
      errorCode = Command_Create( list, resultList,
                                  errorList, errorMessage );
    }

    // --- Update object command

    else if ( nl->IsEqual( first, "update" ) && (length == 4) &&
              nl->IsAtom( nl->Second( list )) &&
             (nl->AtomType( nl->Second( list ) ) == SymbolType) &&
              nl->IsEqual( nl->Third( list ), ":=" ) )
    {
      errorCode = Command_Update( list, errorMessage );
    }

    // --- Let command

    else if ( nl->IsEqual( first, "let" ) && (length == 4) &&
              nl->IsAtom( nl->Second( list )) &&
             (nl->AtomType( nl->Second( list ) ) == SymbolType) &&
              nl->IsEqual( nl->Third( list ), "=" ) )
    {
      errorCode = Command_Let( list, errorMessage );
    }

    // --- derive command

    else if ( nl->IsEqual( first, "derive" ) && (length == 4) &&
              nl->IsAtom( nl->Second( list )) &&
             (nl->AtomType( nl->Second( list ) ) == SymbolType) &&
              nl->IsEqual( nl->Third( list ), "=" ) )
    {
      errorCode = Command_Derive( list, errorMessage );
    }

    // --- Query command

    else if ( nl->IsEqual( first, "query" ) && (length == 2) )
    {
      errorCode = Command_Query( list, resultList, errorMessage );
    }

    // --- Set command
    else if ( nl->IsEqual( first, "set" ) && (length == 4) &&
              nl->IsAtom( nl->Second( list )) &&
             (nl->AtomType( nl->Second( list ) ) == StringType) &&
              nl->IsEqual( nl->Third( list ), "=" ) &&
              nl->AtomType( nl->Fourth(list) ) == BoolType )
    {
      errorCode = Command_Set( list );
    }

    // --- conditional command: if <p> then <c1> [ else <c2> ] endif
    else if ( nl->IsEqual( first, "if" ) &&
              (
                (    (length == 7)
                  && nl->IsEqual(nl->Nth( 3, list ), "then")
                  && nl->IsEqual(nl->Nth( 5, list ), "else")
                  && nl->IsEqual(nl->Nth( 7, list ), "endif")
                ) ||
                (    (length == 5)
                  && nl->IsEqual(nl->Nth( 3, list ), "then")
                  && nl->IsEqual(nl->Nth( 5, list ), "endif")
                )
              )
            )
    {
      errorCode = SecondoInterface::Command_Conditional( list,
                                       resultList,
                                       errorMessage );
    }

    // --- command sequence: (beginseq (command*) endseq)
    else if ( nl->IsEqual( first, "beginseq" ) &&
                (length == 3) &&
                !nl->IsAtom( nl->Nth(2, list) ) &&
                nl->IsEqual( nl->Nth(3, list), "endseq" )
            )
    {
      errorCode =  SecondoInterface::Command_Sequence( list,
                                       resultList,
                                       errorMessage,
                                       true );
    }
    else if ( nl->IsEqual( first, "beginseq2" ) &&
                (length == 3) &&
                !nl->IsAtom( nl->Nth(2, list) ) &&
                nl->IsEqual( nl->Nth(3, list), "endseq2" )
            )
    {
      errorCode =  SecondoInterface::Command_Sequence( list,
                                       resultList,
                                       errorMessage,
                                       false );
    }

    // --- while-do loop: (while <p> do <c> endwhile)
    else if ( nl->IsEqual( first, "while" ) &&
              (length == 5) &&
              nl->IsEqual( nl->Nth(3,list), "do" ) &&
              nl->IsEqual( nl->Nth(5,list), "endwhile" )
            )
    {
      errorCode = Command_WhileDoLoop( list,
                                       resultList,
                                       errorMessage );
    }

    // --- ERROR: unrecognized command
    else
    {
      errorCode = ERR_CMD_NOT_RECOGNIZED;         // Command not recognized
    }

    // commit or abort transaction failed!
    if (!fc || (errorCode != ERR_NO_ERROR))
    {
      resultList=errorList;
    }
  }

  constructErrMsg(errorCode, errorMessage);

  //*** END COMMAND RECOGNITION ***//

  if ( resultAsText )
  {
    nl->WriteToFile( resultFileName, resultList );
  }

  bool showResult = RTFlag::isActive("SI:ResultList");
  if (showResult) {
    cmsg.info() << endl << "### Result List before copying: "
                << nl->ToString(resultList) << endl;
    cmsg.send();
  }

  // copy result into application specific list container.
  StopWatch copyTime;
  if (resultList) {
     if(isApplicationLevelCommand){
      resultList = nl->CopyList(resultList, al);
     }
     if (printQueryAnalysis)
     {
        cmsg.info() << padStr("Copying result ...",20)
                    << copyTime.diffTimes() << endl;
        cmsg.send();
     }
     copyReal = copyTime.diffSecondsReal();
  }

  if (showResult) {
    if(isApplicationLevelCommand){ // use external NL storage
        cmsg.info() << endl << "### Result after copying: "
                    << al->ToString(resultList) << endl;
    } else { // use internal NL storage
        cmsg.info() << endl << "### Result after copying: "
                    << nl->ToString(resultList) << endl;
    }
    cmsg.send();
  }
  if(isApplicationLevelCommand){ // clear NL storage after an ApplicationCommand
    nl->initializeListMemory();
  }

  cmdReal = cmdTime.diffSecondsReal();
  cmdCPU = cmdTime.diffSecondsCPU();

  // initialize and increment query counter (better: command counter)
  static int CmdNr = 0;
  CmdNr++;

  // handle command times
  bool printCmdTimes = RTFlag::isActive("SI:PrintCmdTimes");
  bool dumpCmdTimes = RTFlag::isActive("SI:DumpCmdTimes");

  CmdTimes* ctp = new CmdTimes( CmdNr, commandText,
                                cmdReal, cmdCPU, commitReal,
                                queryReal, queryCPU, outObjReal, copyReal );

  cmdTimesRel->append(ctp, dumpCmdTimes);

  if (printCmdTimes)
  {
    cmsg.info() << padStr("Total runtime ...",20)
                << cmdTime.diffTimes() << endl;
    cmsg.send();
  }

  // handle counters
  if (relStatistics) {
    Tuple::SetCounterValues();
    Attribute::SetCounterValues(relStatistics);
  }
  StdTypes::SetCounterValues(stdStatistics);

  bool printCtrs = RTFlag::isActive("SI:PrintCounters");
  bool dumpCtrs = RTFlag::isActive("SI:DumpCounters");
  Counter::Str2ValueMap cm = Counter::usedCounters();
  Counter::Str2ValueMap::iterator it = cm.begin();
  while ( it != cm.end() )
  {
    CmdCtr* cp = new CmdCtr(CmdNr, it->first, it->second);
    cmdCtrRel->append(cp, dumpCtrs);
    it++;
  }

  if (printCtrs)
  {
    Counter::reportValues(CmdNr);
  }
  Counter::resetAll();

  // handle cache and file info
  CacheInfoTuple* ci = new CacheInfoTuple();
  typedef vector<FileInfo*> FStatVec;
  FStatVec* fi = new FStatVec;

  // retrieve statistics
  if (SmiEnvironment::GetCacheStatistics(*ci, *fi)){
    ci->cstatNr = CmdNr;
    cacheInfoRel->append(ci, true);

    FStatVec::iterator fit = fi->begin();
    while (fit != fi->end())
    {
      (*fit)->fstatNr = CmdNr;
      FileInfoTuple* pfi = new FileInfoTuple(*fit);
      //SHOW(*pfi)
      fileInfoRel->append(pfi, true);
      delete *fit;
      fit++;
    }
  }
  delete fi;

  AlgebraManager& am = *SecondoSystem::GetAlgebraManager();
  am.UpdateOperatorUsage(operatorUsageRel);

  Flob::killAllNativeFlobs();

  return;
}


void
SecondoInterface::constructErrMsg(int& errorCode, string& errorMessage)
{

  // Check if there were SMI errors
  if ( SmiEnvironment::GetNumOfErrors() != 0) {

    if (errorCode == 0){
      errorCode = ERR_SYSTEM_ERROR;
    }
    string err;
    SmiEnvironment::GetLastErrorCode(err);
    errorMessage += err + "\n";
  }

  if ( errorCode != 0) {
    // translate error code into text
    // check if the queryprocessor reports errors
    string repMsg("");
    ErrorReporter::GetErrorMessage(repMsg);
    ErrorReporter::Reset();
    errorMessage += repMsg + "\n";

    cmsg.error() << "" ; // be sure to use the error channel
    cmsg.send(); // flush cmsg buffer

    if ( ServerInstance() )
    {
      // append all stacked error messages which were
      // send to cmsg.error
      errorMessage += cmsg.getErrorMsg();
    }

    errorMessage += GetErrorMessage(errorCode);
  }
}


/*
1.2 Implementation of commands

1.2.1 query

*/

SI_Error
SecondoInterface::Command_Query( const ListExpr list,
                                 ListExpr& resultList,
                                 string& errorMessage )
{
  QueryProcessor& qp = *SecondoSystem::GetQueryProcessor();
  SecondoCatalog& ctlg = *SecondoSystem::GetCatalog();
  SecondoSystem& sys = *SecondoSystem::GetInstance();
  NestedList& nl = *SecondoSystem::GetNestedList();

  SI_Error errorCode = ERR_NO_ERROR;
  bool correct = false;
  bool evaluable = false;
  bool defined = false;
  bool isFunction = false;

  Word result = SetWord( Address(0) );
  OpTree tree = 0;

  ListExpr resultType = nl.TheEmptyList();

  if ( !sys.IsDatabaseOpen() ) // no database open
  {
     return ERR_NO_DATABASE_OPEN;
  }

  StopWatch queryTime;
  StartCommand();

  try {

    qp.Construct( nl.Second( list ), correct, evaluable, defined,
                  isFunction, tree, resultType );

    if (printQueryAnalysis) {
      cmsg.info() << padStr("Analyze ...",20) << queryTime.diffTimes() << endl;
      cmsg.send();
      queryTime.start();
    }

    if ( evaluable )
    {
       if (printQueryAnalysis) {
	 cmsg.info() << padStr("Execute ...",20);
	 cmsg.send();
       }

       qp.ResetTimer();

       qp.EvalP( tree, result, 1 );

       queryReal = queryTime.diffSecondsReal();
       queryCPU = queryTime.diffSecondsCPU();
       if (printQueryAnalysis)
       {
	 showTimes(queryReal, queryCPU);
       }

       StopWatch outObj;
       ListExpr valueList = ctlg.OutObject( resultType, result );

       if (printQueryAnalysis)
       {
	 cmsg.info() << padStr("OutObject ...",20)
	             << outObj.diffTimes() << endl;
	 cmsg.send();
       }
       outObjReal = outObj.diffSecondsReal();

       resultList = nl.TwoElemList( resultType, valueList );

       StopWatch destroyTime;
       qp.Destroy( tree, true );
       if ( RTFlag::isActive("SI:DestroyOpTreeTime") ) {
	 cmsg.info() << "Destroy " << destroyTime.diffTimes() << endl;
	 cmsg.send();
       }

       if (RTFlag::isActive("NL:MemInfo"))
       {
	 cmsg.info() << nl.ReportTableSizes(true) << endl;
	 cmsg.send();
       }
       else
       {
	 nl.ReportTableSizes(false);
       }

    }

    if ( isFunction ) // abstraction or function object
    {
      ListExpr second = nl.Second( list );
      if ( nl.IsAtom( second ) )  // function object
      {
	ListExpr valueList = ctlg.GetObjectValue( nl.SymbolValue( second ) );
	resultList = nl.TwoElemList( resultType, valueList );
      }
      else
      {
	resultList = nl.TwoElemList( resultType, second );
      }
    }

  } catch (SI_Error err) {

    errorCode = err;
  }

  qp.Destroy( tree, true );
  SmiEnvironment::SetFlag_NOSYNC(true);
  FinishCommand( errorCode, errorMessage );
  SmiEnvironment::SetFlag_NOSYNC(false);

  return errorCode;
}

/*

1.2.2 derive

*/


SI_Error
SecondoInterface::Command_Derive( const ListExpr list, string& errorMessage )
{
  SecondoCatalog& ctlg = *SecondoSystem::GetCatalog();
  SecondoSystem& sys = *SecondoSystem::GetInstance();
  NestedList& nl = *SecondoSystem::GetNestedList();

  SI_Error errorCode = ERR_NO_ERROR;

  if ( !sys.IsDatabaseOpen() )  // no database open
  {
    return ERR_NO_DATABASE_OPEN;
  }

  if ( !errorCode ) { // if no errors ocurred continue

     StartCommand();
     string objName = nl.SymbolValue( nl.Second( list ) );
     ListExpr valueExpr = nl.Fourth( list );

    if ( ctlg.IsSystemObject(objName) ) // check if identifier is allowed
    {
      errorCode = ERR_IDENT_RESERVED;
    }
    else if ( ctlg.IsObjectName(objName) )  // identifier is already used
    {
      errorCode = ERR_IDENT_USED;
    }
    else // try to create object
    {
      errorCode = derivedObjPtr->createObj(objName, valueExpr);

      if ( !errorCode ) {
        derivedObjPtr->addObj( objName, valueExpr );
      }
    }
  }

  FinishCommand( errorCode, errorMessage );

  return errorCode;
}

/*

1.2.3 let

*/


SI_Error
SecondoInterface::Command_Let( const ListExpr list, string& errorMessage  )
{
  QueryProcessor& qp = *SecondoSystem::GetQueryProcessor();
  SecondoCatalog& ctlg = *SecondoSystem::GetCatalog();
  SecondoSystem& sys = *SecondoSystem::GetInstance();
  NestedList& nl = *SecondoSystem::GetNestedList();

  SI_Error errorCode = ERR_NO_ERROR;
  bool correct = false;
  bool evaluable = false;
  bool defined = false;
  bool isFunction = false;

  Word result = SetWord( Address(0) );
  OpTree tree = 0;

  ListExpr resultType = nl.TheEmptyList();


  if ( sys.IsDatabaseOpen() )
  {
      StartCommand();
      string objName = nl.SymbolValue( nl.Second( list ) );
      ListExpr valueExpr = nl.Fourth( list );

      if ( ctlg.IsSystemObject(objName) ) {
      errorCode = ERR_IDENT_RESERVED;

      }
      else if ( ctlg.IsObjectName(objName) ) // identifier is already used
      {
        errorCode = ERR_IDENT_USED;
      }
      else
      {
        try {

        qp.Construct( valueExpr, correct, evaluable, defined,
                      isFunction, tree, resultType );

	if ( evaluable || isFunction )
	{
	  string typeName = "";
	  ctlg.CreateObject(objName, typeName, resultType, 0);
	}
	if ( evaluable )
	{
	  qp.EvalP( tree, result, 1 );

	  if( IsRootObject( tree ) && !IsConstantObject( tree ) )
	  {
	    ctlg.CloneObject( objName, result );
	    qp.Destroy( tree, true );
	  }
	  else
	  {
	    ctlg.UpdateObject( objName, result );
	    qp.Destroy( tree, false );
	  }
	}
	else if ( isFunction ) // abstraction or function object
	{
	  if ( nl.IsAtom( valueExpr ) )  // function object
	  {
	     ListExpr functionList = ctlg.GetObjectValue(
					    nl.SymbolValue( valueExpr ) );
	     ctlg.UpdateObject( objName, SetWord( functionList ) );
	  }
	  else
	  {
	     ctlg.UpdateObject( objName, SetWord( valueExpr ) );
	  }
          qp.Destroy( tree, true );
	}
      } catch (SI_Error err) {

	 errorCode = err;
         qp.Destroy( tree, true );
      }
    }
    FinishCommand( errorCode, errorMessage );
  }
  else // no database open
  {
    errorCode = ERR_NO_DATABASE_OPEN;
  }

  return errorCode;
}


/*

1.2.4 update

*/

SI_Error
SecondoInterface::Command_Update( const ListExpr list, string& errorMessage )
{
  QueryProcessor& qp = *SecondoSystem::GetQueryProcessor();
  SecondoCatalog& ctlg = *SecondoSystem::GetCatalog();
  SecondoSystem& sys = *SecondoSystem::GetInstance();
  NestedList& nl = *SecondoSystem::GetNestedList();

  SI_Error errorCode = ERR_NO_ERROR;
  bool correct = false;
  bool evaluable = false;
  bool defined = false;
  bool isFunction = false;

  Word result = SetWord( Address(0) );
  OpTree tree = 0;

  ListExpr resultType = nl.TheEmptyList();

  if ( sys.IsDatabaseOpen() )
  {
    {
      StartCommand();
      string objName = nl.SymbolValue( nl.Second( list ) );
      ListExpr valueExpr = nl.Fourth( list );

      try {

      if ( ctlg.IsSystemObject(objName) ) {
	throw ERR_IDENT_RESERVED;
      }

      if ( derivedObjPtr && derivedObjPtr->isDerived(objName) ) {
	throw ERR_UPDATE_FOR_DERIVED_OBJ_UNSUPPORTED;
      }

      if ( !ctlg.IsObjectName( objName ) ) {
	// identifier not a known object name
	throw ERR_IDENT_UNKNOWN_OBJ;
      }

      qp.Construct( valueExpr, correct, evaluable, defined,
                    isFunction, tree, resultType );


      ListExpr typeExpr = ctlg.GetObjectTypeExpr( objName );
      if ( !nl.Equal( typeExpr, resultType ) ) {
	// types of object and expression do not agree
	throw ERR_EXPR_TYPE_NEQ_OBJ_TYPE;
      }

      if ( evaluable )
      {
	qp.EvalP( tree, result, 1 );

	if ( IsRootObject( tree ) && !IsConstantObject( tree ) )
	{
	   ctlg.CloneObject( objName, result );
	   qp.Destroy( tree, true );
	}
	else
	{
	   ctlg.UpdateObject( objName, result );
	   qp.Destroy( tree, false );
	}
      }
      else  // abstraction or function object
      {
	assert(isFunction);
	if ( nl.IsAtom( valueExpr ) )  // function object
	{
	  ListExpr functionList = ctlg.GetObjectValue(
					 nl.SymbolValue( valueExpr ) );
	  ctlg.UpdateObject( objName, SetWord( functionList ) );
	}
	else
	{
	  ctlg.UpdateObject( objName, SetWord( valueExpr ) );
	}
      }

      } catch (SI_Error err) {

	 errorCode = err;
         qp.Destroy( tree, true );
      }
      FinishCommand( errorCode, errorMessage );
    }
  }
  else // no database open
  {
    errorCode = ERR_NO_DATABASE_OPEN;
  }

  return errorCode;
}


/*

1.2.4 create

*/

SI_Error
SecondoInterface::Command_Create( const ListExpr list,
                                  ListExpr& resultList,
                                  ListExpr& errorList,
                                  string& errorMessage  )
{
  SecondoCatalog& ctlg = *SecondoSystem::GetCatalog();
  SecondoSystem& sys = *SecondoSystem::GetInstance();
  NestedList& nl = *SecondoSystem::GetNestedList();

  SI_Error errorCode = ERR_NO_ERROR;

  Word result = SetWord( Address(0) );

  if ( sys.IsDatabaseOpen() )
  {
    StartCommand();
    string objName = nl.SymbolValue( nl.Second( list ) );
    ListExpr typeExpr = nl.Fourth( list );
    ListExpr typeExpr2 = ctlg.ExpandedType( typeExpr );
    string typeName = "";

    if ( ctlg.KindCorrect( typeExpr2, errorList ) )
    {
      if ( nl.IsAtom( typeExpr ) &&
          ((nl.AtomType( typeExpr ) == SymbolType)) )
      {
        typeName = nl.SymbolValue( typeExpr );
        if ( !ctlg.MemberType( typeName ) )
        {
          typeName = "";
        }
      }
      if ( ctlg.IsSystemObject(objName) ) { // identifier not reserved?
        errorCode = ERR_IDENT_RESERVED;

      } else if ( !ctlg.CreateObject( objName, typeName, typeExpr2, 0 ) )
      {
        errorCode = ERR_IDENT_USED;  // identifier already used
      }
    }
    else // Wrong type expression
    {
      errorCode = ERR_NO_OBJ_CREATED;
      resultList = errorList;
    }
    FinishCommand( errorCode, errorMessage );
  }
  else // no database open
  {
    errorCode = ERR_NO_DATABASE_OPEN;
  }

  return errorCode;
}

/*

1.2.4 the set command

*/

SI_Error
SecondoInterface::Command_Set( const ListExpr list )
{
  NestedList& nl = *SecondoSystem::GetNestedList();

  string paramStr = nl.StringValue(nl.Second(list));
  bool value = nl.BoolValue(nl.Fourth(list));

  RTFlag::setFlag(paramStr, value);

  SI_Error errorCode = ERR_NO_ERROR;
  return errorCode;
}

/*
1.2.5 The if-then-endif and if-then-else-endif Command

~list~ must have one of the following two structures:

----
  (if <value-expression> then <command> endif)
  (if <value-expression> then <command> else <command> endif)
----

*/
SI_Error
SecondoInterface::Command_Conditional( const ListExpr list,
                                       ListExpr &resultList,
                                       string &errorMessage )
{
  SI_Error errorCode = ERR_NO_ERROR;
  bool pred_def      = false;
  bool pred_val      = false;
  int errorPos       = 0;
  ListExpr command   = nl->TheEmptyList();
  ListExpr presult   = nl->TheEmptyList();
  resultList         = nl->TheEmptyList();
  errorMessage       = "";

  command = nl->TwoElemList(nl->SymbolAtom("query"),nl->Second( list ));
  errorCode = SecondoInterface::Command_Query( command, presult, errorMessage );
  if(errorCode != ERR_NO_ERROR){
    return errorCode;
  } else { // check result type
    if(    (nl->ListLength(presult) != 2)
        || !nl->IsAtom(nl->First(presult))
        || !nl->IsEqual(nl->First(presult),"bool")
        || !nl->IsAtom(nl->Second(presult))
      ){
      errorMessage =
        "IF-THEN-ELSE-ENDIF: Expected a boolean expression as predicate.";
      errorCode = ERR_SYNTAX_ERROR;
      return errorCode;
    } else { // get result value
      pred_def = !nl->IsEqual(nl->Second(presult),"undef");
      if(pred_def){
        pred_val =  nl->BoolValue(nl->Second(presult));
      } // else pred_val = false
    }
  }
  if( pred_def && pred_val ){
    // execute 1st command (THEN-branch)
    command = nl->Nth( 4, list );
    const string command_text = "ELSE: " + (nl->ToString(command));
    SecondoInterface::Secondo( command_text,
                           command,
                           0,
                           false,
                           false,
                           resultList,
                           errorCode,
                           errorPos,
                           errorMessage,
                           "SecondoQPResult_tmp",
                           false);
  } else if( pred_def && !pred_val && (nl->ListLength(list) == 7) ){
    // execute 2nd command (ELSE-branch)
    command = nl->Nth( 6, list );
    const string command_text = "THEN: " + (nl->ToString(command));
    SecondoInterface::Secondo( command_text,
                           command,
                           0,
                           false,
                           false,
                           resultList,
                           errorCode,
                           errorPos,
                           errorMessage,
                           "SecondoQPResult_tmp",
                           false);
  }
  return errorCode;
}

/*
1.2.6 The command-sequence Command

~list~ must have the following structure:

----
  (beginseq (<command>*) endseq)
----

*/
SI_Error
SecondoInterface::Command_Sequence( const ListExpr  list,
                                    ListExpr       &resultList,
                                    string         &errorMessage,
                                    bool           ignoreError )
{
  SI_Error errorCode  = ERR_NO_ERROR;
  int errorPos        = 0;
  int no_commands     = nl->ListLength(nl->Second(list));
  int i               = 1;
  ListExpr command    = nl->TheEmptyList();
  ListExpr lastResult = nl->TheEmptyList();
  ListExpr cresult    = nl->TheEmptyList();
  resultList          = nl->TheEmptyList();
  errorMessage        = "";

  while( ((errorCode == ERR_NO_ERROR) || ignoreError) && (i<=no_commands) ){
    stringstream out;
    out << i;
    string i_s = out.str();
    command = nl->Nth(i, nl->Second(list)); // get nth command
    cresult = nl->TheEmptyList();
    const string command_text =   "SEQUENCE (" + i_s + "): "
                                + (nl->ToString(command));
    SecondoInterface::Secondo( command_text,
                           command,
                           0,
                           false,
                           false,
                           cresult,
                           errorCode,
                           errorPos,
                           errorMessage,
                           "SecondoQPResult_tmp",
                           false);         // execute ii
    // append result to result list
    if(nl->IsEmpty(resultList)){ // create OneElemList;
      resultList = nl->OneElemList(cresult);
      lastResult = resultList;
    } else { // append to existing list
      lastResult = nl->Append(lastResult,cresult);
    }
    i++;
  }
  resultList = nl->TwoElemList(nl->SymbolAtom("resultsequence"),resultList);
  return errorCode;
}

/*
1.2.t The While-Do Loop Command

~list~ must have the following structure:

----  (while <value-expression> do <command> endwhile)
----

*/
SI_Error
SecondoInterface::Command_WhileDoLoop( const ListExpr  list,
                                       ListExpr       &resultList,
                                       string         &errorMessage )
{
  SI_Error errorCode    = ERR_NO_ERROR;
  bool pred_def         = true;
  bool pred_val         = true;
  int errorPos          = 0;
  ListExpr pred_command = nl->TheEmptyList();
  ListExpr loop_command = nl->TheEmptyList();
  ListExpr presult      = nl->TheEmptyList();
  ListExpr cresult      = nl->TheEmptyList();
  ListExpr lastResult   = nl->TheEmptyList();
  int loop_cnt          = 0;
  resultList            = nl->TheEmptyList();
  errorMessage          = "";

  pred_command = nl->TwoElemList(nl->SymbolAtom("query"),nl->Second( list ));
  loop_command = nl->Fourth( list );
  while ( (errorCode == ERR_NO_ERROR) && pred_def && pred_val ){
    errorCode = SecondoInterface::Command_Query( pred_command,
                                                 presult,
                                                 errorMessage );
    if(errorCode == ERR_NO_ERROR){
      // check result type
      if(    (nl->ListLength(presult) != 2)
          || !nl->IsAtom(nl->First(presult))
          || !nl->IsEqual(nl->First(presult),"bool")
          || !nl->IsAtom(nl->Second(presult))
        ){
        errorMessage =
          "WHILE-DO-ENDWHILE: Expected a boolean expression as predicate.";
        errorCode = ERR_SYNTAX_ERROR;
      } else { // get result value
        pred_def = !nl->IsEqual(nl->Second(presult),"undef");
        if(pred_def){
          pred_val =  nl->BoolValue(nl->Second(presult));
        } else {
          pred_val = false;
        }
      }
    }
    if( (errorCode == ERR_NO_ERROR) && pred_def && pred_val ){
      loop_cnt++;
      stringstream out;
      out << loop_cnt;
      string loop_cnt_s = out.str();
      // execute command
      cresult = nl->TheEmptyList();
      const string command_text =   "DO (" + loop_cnt_s + "): "
                                  + (nl->ToString(loop_command));
      SecondoInterface::Secondo( command_text,
                            loop_command,
                            0,
                            false,
                            false,
                            cresult,
                            errorCode,
                            errorPos,
                            errorMessage,
                            "SecondoQPResult_tmp",
                            false);
      // print result
//       cmsg.info() << endl << "Result for loop " << loop_cnt << ": "
//                   << nl->ToString(cresult) << endl;
//       cmsg.send();
      // append result to result list
      if(nl->IsEmpty(resultList)){ // create OneElemList;
        resultList = nl->OneElemList(cresult);
        lastResult = resultList;
      } else { // append to existing list
        lastResult = nl->Append(lastResult,cresult);
      }
    }
  } // end while
  resultList = nl->TwoElemList(nl->SymbolAtom("resultsequence"),resultList);
  return errorCode;
}


/*
1.3 Procedure ~NumericTypeExpr~

*/
ListExpr
SecondoInterface::NumericTypeExpr( const ListExpr type )
{
  ListExpr list = nl->TheEmptyList();
  if ( SecondoSystem::GetInstance()->IsDatabaseOpen() )
  {
    // use application specific list memory
    list = SecondoSystem::GetCatalog()->NumericType( al->CopyList(type,nl) );
    list = nl->CopyList(list, al);
  }
  return (list);
}

bool
SecondoInterface::GetTypeId( const string& name,
                             int& algebraId, int& typeId )
{
  bool ok = SecondoSystem::GetCatalog()->
              GetTypeId( name, algebraId, typeId );
  return (ok);
}

bool
SecondoInterface::LookUpTypeExpr( ListExpr type, string& name,
                                  int& algebraId, int& typeId )
{
  bool ok = false;
  name = "";
  algebraId = 0;
  typeId = 0;
  al->ToString(type);

  if ( SecondoSystem::GetInstance()->IsDatabaseOpen() )
  {
    // use application specific list memory
    ok = SecondoSystem::GetCatalog()->
           LookUpTypeExpr( al->CopyList(type,nl), name, algebraId, typeId );
  }
  return (ok);
}

void
SecondoInterface::StartCommand()
{
  if ( !activeTransaction )
  {
    SecondoSystem::BeginTransaction();
  }
}

bool
SecondoInterface::FinishCommand( SI_Error& errorCode, string& errMsg )
{
  Flob::dropFiles();
  if ( !activeTransaction )
  {
    StopWatch commitTime;
    if ( errorCode == 0 )
    {
      if ( !SecondoSystem::CommitTransaction() )
      {
        errMsg = errMsg
               + "CommitTransaction() failed! "
               + "But the previous command was successful.";
        errorCode = ERR_COMMIT_OR_ABORT_FAILED;
        return false;
      }
    }
    else
    {
      if ( !SecondoSystem::AbortTransaction() )
      {
        errMsg = errMsg
               + "AbortTransaction() failed! An abort was necessary "
               + "since the previous command failed with: "
               + GetErrorMessage(errorCode);

        errorCode = ERR_COMMIT_OR_ABORT_FAILED;
        return false;
      }
    }
    if (printQueryAnalysis)
    {
      cmsg.info() << padStr("Committing ...", 20)
                  << commitTime.diffTimes() << endl;
      cmsg.send();
    }
    commitReal = commitTime.diffSecondsReal();
  }
  return true;
}


void
SecondoInterface::SetDebugLevel( const int level )
{
  SecondoSystem::GetQueryProcessor()->SetDebugLevel( level );
}


bool SecondoInterface::getOperatorIndexes(
                        const string opName,
                        const ListExpr argList,
                        ListExpr& resList,
                        int& algId,
                        int& opId,
                        int& funId,
                        NestedList* listStorage){

   ListExpr argList1;
   NestedList* nl = SecondoSystem::GetNestedList();
   if(listStorage!=nl){
      argList1 = listStorage->CopyList(argList, nl);
   } else {
      argList1 = argList;
   }

   AlgebraManager* am = SecondoSystem::GetAlgebraManager();
   ListExpr resList1;

   bool ok = am->findOperator(opName,argList1, resList1, algId,opId,funId);
   

   if(listStorage !=nl){
      resList = nl->CopyList(resList1,listStorage);
   }  else {
      resList = resList1;
   }
   
   return  ok;
}


/*
~getCosts~

The next functions return costs for a specified operator when number of tuples
and size of a single tuple is given. If the operator does not provide a
cost estimation function or the getCost function is not implemented,
the return value is false.

*/

bool SecondoInterface::getCosts(const int algId,
              const int opId,
              const int funId,
              const size_t noTuples,
              const size_t sizeOfTuple,
              const size_t noAttributes,
              const double selectivity,
              const size_t memoryMB,
              size_t& costs){
   return SecondoSystem::GetAlgebraManager()->getCosts(
                   algId,opId,funId,
                   noTuples,sizeOfTuple,noAttributes, 
                   selectivity,memoryMB,costs);
}


bool SecondoInterface::getCosts(const int algId,
              const int opId,
              const int funId,
              const size_t noTuples1,
              const size_t sizeOfTuple1,
              const size_t noAttributes1,
              const size_t noTuples2,
              const size_t sizeOfTuple2,
              const size_t noAttributes2,
              const double selectivity,
              const size_t memoryMB,
              size_t& costs) {
   return SecondoSystem::GetAlgebraManager()->getCosts(
                   algId, opId, funId,
                   noTuples1, sizeOfTuple2,noAttributes1,
                   noTuples2, sizeOfTuple2, noAttributes2,
                   selectivity, memoryMB, costs);

}

/*
~getLinearParams~

Retrieves the parameters for estimating the cost function of an operator
in a linear way.

*/
bool SecondoInterface::getLinearParams( const int algId,
                      const int opId,
                      const int funId,
                      const size_t noTuples1,
                      const size_t sizeOfTuple1,
                      const size_t noAttributes1,
                      const double selectivity,
                      double& sufficientMemory,
                      double& timeAtSuffMemory,
                      double& timeAt16MB) {
    return SecondoSystem::GetAlgebraManager()->getLinearParams(
                algId,opId,funId,
                noTuples1,sizeOfTuple1,noAttributes1,
                selectivity,
                sufficientMemory,timeAtSuffMemory,timeAt16MB);
}


bool SecondoInterface::getLinearParams( const int algId,
                      const int opId,
                      const int funId,
                      const size_t noTuples1,
                      const size_t sizeOfTuple1,
                      const size_t noAttributes1,
                      const size_t noTuples2,
                      const size_t sizeOfTuple2,
                      const size_t noAttributes2,
                      const double selectivity,
                      double& sufficientMemory,
                      double& timeAtSuffMemory,
                      double& timeAt16MB) {
    return SecondoSystem::GetAlgebraManager()->getLinearParams(
                algId, opId, funId,
                noTuples1, sizeOfTuple1, noAttributes1,
                noTuples2, sizeOfTuple2, noAttributes2,
                selectivity,
                sufficientMemory,timeAtSuffMemory,timeAt16MB);
}

/*
~getFunction~

Returns an approximation of the cost function of a specified value mapping as
a parametrized function.

*/
bool SecondoInterface::getFunction(const int algId,
                 const int opId,
                 const int funId,
                 const size_t noTuples,
                 const size_t sizeOfTuple,
                 const size_t noAttributes,
                 const double selectivity,
                 int& funType,
                 double& sufficientMemory,
                 double& timeAtSuffMemory,
                 double& timeAt16MB,
                 double& a, double& b, double&c, double& d) {
   return SecondoSystem::GetAlgebraManager()->getFunction(
           algId, opId, funId,
           noTuples, sizeOfTuple, noAttributes,
           selectivity,
           funType,sufficientMemory,timeAtSuffMemory,timeAt16MB,
           a,b,c,d);

}
                      


bool SecondoInterface::getFunction(const int algId,
                 const int opId,
                 const int funId,
                 const size_t noTuples1,
                 const size_t sizeOfTuple1,
                 const size_t noAttributes1,
                 const size_t noTuples2,
                 const size_t sizeOfTuple2,
                 const size_t noAttributes2,
                 const double selectivity,
                 int& funType,
                 double& sufficientMemory,
                 double& timeAtSuffMemory,
                 double& timeAt16MB,
                 double& a, double& b, double&c, double& d) {

   return SecondoSystem::GetAlgebraManager()->getFunction(
           algId, opId, funId,
           noTuples1, sizeOfTuple1, noAttributes1,
           noTuples2, sizeOfTuple2, noAttributes2,
           selectivity,
           funType, sufficientMemory, timeAtSuffMemory, timeAt16MB,
           a,b,c,d);
}

