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

February 2006, M. Spiekermann. A proper handling of errors when commit or abort transaction 
fails after command execution was implemented. Further, the scope of variables in function
Secondo was limited to a minimum, e.g. the declarations were moved nearer to the usage. This
gives more encapsulation and is easier to understand and maintain.

April 2006, M. Spiekermann. Implementation of system tables SEC\_COUNTERS and SEC\_COMMANDS.

\tableofcontents

*/

using namespace std;

#include <iostream>
#include <fstream>

#include "SecondoInterface.h"
#include "SecondoSystem.h"
#include "SecondoCatalog.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "Profiles.h"
#include "FileSystem.h"

#include "RelationAlgebra.h"
#include "StandardTypes.h"

#include "SecondoSMI.h"
#include "SecParser.h"

#include "StopWatch.h"
#include "LogMsg.h"
#include "Counter.h"
#include "DerivedObj.h"
#include "NList.h"

#include "CharTransform.h"

/* 
The System Tables

Below there are implementations of system tables. More information
about them can be found in SystemInfoRel.h

*/

#include "SystemInfoRel.h"
SystemTables* SystemTables::instance = 0;

class CmdTimes : public InfoTuple 
{
   int nr;
   string cmdStr;
   double elapsedTime;
   double cpuTime;

   public: 
   CmdTimes(int num, const string& cmd, double realT, double cpuT) :
     nr(num),
     cmdStr(cmd),
     elapsedTime(realT),
     cpuTime(cpuT)
   {
   }
   virtual ~CmdTimes() {}
  
   virtual NList valueList() const
   {
     NList value;
     value.makeHead( NList(nr) );
     value.append( NList().textAtom(cmdStr) );
     value.append( NList(elapsedTime) );
     value.append( NList(cpuTime) );
     return value;
   } 
   
   virtual ostream& print(ostream& os) const
   {
      os << nr << sep << cmdStr << sep << elapsedTime << sep << cpuTime;
      return os;
   } 

}; 


class CmdTimesRel : public SystemInfoRel 
{
   public:
   CmdTimesRel(const string& name) : SystemInfoRel(name, initSchema()) 
   {} 

   ~CmdTimesRel() {}
   
   private:
   RelSchema* initSchema()
   { 
     RelSchema*  attrList = new RelSchema();
     attrList->push_back( make_pair("CmdNr", "int") );
     attrList->push_back( make_pair("CmdStr", "text") );
     attrList->push_back( make_pair("ElapsedTime", "real") );
     attrList->push_back( make_pair("CpuTime", "real") );
     return attrList;  
   } 

}; 

class CmdCtr : public InfoTuple 
{
   int nr;
   string ctrStr;
   long value;

   public: 
   CmdCtr(int num, const string& cmd, long ctrVal) :
     nr(num),
     ctrStr(cmd),
     value(ctrVal)
   {}
   virtual ~CmdCtr() {}
   
   virtual NList valueList() const
   {
     NList list;
     list.makeHead( NList(nr) );
     list.append( NList().stringAtom(ctrStr) );
     list.append( NList((int) value) );
     return list;
   } 

   
   virtual ostream& print(ostream& os) const
   {
      os << nr << sep << ctrStr << sep << value;
      return os;
   } 
}; 


class CmdCtrRel : public SystemInfoRel 
{
   public:
   CmdCtrRel(const string& name) : SystemInfoRel(name, initSchema()) 
   {}
   virtual ~CmdCtrRel() {}
   
   private:
   RelSchema* initSchema()
   { 
     RelSchema* attrList = new RelSchema();
     attrList->push_back( make_pair("CtrNr", "int") );
     attrList->push_back( make_pair("CtrStr", "string") );
     attrList->push_back( make_pair("Value", "int") );
     return attrList;
   } 
}; 

class DerivedObjInfo : public InfoTuple 
{
   string name;
   string value;
   string usedObjs;

   public: 
   DerivedObjInfo(const string& n, const string& v, const string&u) :
     name(n),
     value(v),
     usedObjs(u)
   {}
   virtual ~DerivedObjInfo() {}
   
   virtual NList valueList() const
   {
     NList list;
     list.makeHead( NList().stringAtom(name) );
     list.append( NList().textAtom(value) );
     list.append( NList().textAtom(usedObjs) );
     return list;
   } 

   
   virtual ostream& print(ostream& os) const
   {
      os << name << sep << value << sep << usedObjs;
      return os;
   } 
}; 



class DerivedObjRel : public SystemInfoRel 
{
   public:
   DerivedObjRel(const string& name) : SystemInfoRel(name, initSchema(), true) 
   {}
   virtual ~DerivedObjRel() {}
   
   private:
   RelSchema* initSchema()
   { 
     RelSchema* attrList = new RelSchema();
     attrList->push_back( make_pair("name", "string") );
     attrList->push_back( make_pair("value", "text") );
     attrList->push_back( make_pair("usedObjs", "text") );
     return attrList;
   } 
}; 



  
ostream& operator<<(ostream& os, const InfoTuple& si) 
{
  return si.print(os);
} 

// currently we will have 3 tables 

CmdTimesRel cmdTimesRel("SEC_COMMANDS");
CmdCtrRel cmdCtrRel("SEC_COUNTERS");

// The next table is currently only a dummy. This is necessary
// that the catalog recognizes it as a system table. In the future
// I try to change the implementation of class ~DerivedObj~ in order
// to make it to a subclass of SystemInfoRel
DerivedObjRel devObjRel("SEC_DERIVED_OBJ");


extern AlgebraListEntry& GetAlgebraEntry( const int j );

static SecondoSystem* ss = 0;

Symbols sym;

/**************************************************************************

1 Implementation of class SecondoInterface

*/

int
DerivedObj::ObjRecord::id = 0;

/*

1.1 Constructor

*/

SecondoInterface::SecondoInterface() : 
  initialized( false ), 
  activeTransaction( false ), 
  isCSImpl( false ), 
  nl( 0 ), 
  server( 0 ), 
  derivedObjPtr(0)
{}


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
                              string& parmFile, const bool multiUser )
{
  bool ok = false;
  cout << endl << "Initializing the SECONDO Interface ..." << endl;

  // initialize runtime flags
  InitRTFlags(parmFile);

  string value, foundValue;
  if ( SmiProfile::GetParameter( "Environment", 
                                 "SecondoHome", "", parmFile ) == "" )
  {
    cout << "Error: Secondo home directory not specified." << endl;
  }
  else
  {
    ok = true;
  }


  // create directory tmp below current dir
  string tempDir("tmp");
  if (! FileSystem::FileOrFolderExists(tempDir) ) 
  {
    if (! FileSystem::CreateFolder(tempDir) )
    {
      cmsg.error() << "Could not create directory " << tempDir << endl;
      cmsg.send();
      ok = false;
    } 
  }

  if ( ok )
  {
    // --- Check storage management interface
    SmiEnvironment::RunMode mode =  SmiEnvironment::SingleUser;
    if ( RTFlag::isActive("SMI:NoTransactions") ) {
       
       mode = SmiEnvironment::SingleUserSimple;
       cout << "  Setting mode to SingleUserSimple" << endl;
       
    } else { // Transactions and logging are used
    
       mode = (multiUser) ? SmiEnvironment::MultiUser 
                          : SmiEnvironment::SingleUser;
    }                   
    
    if ( SmiEnvironment::StartUp( mode, parmFile, cout ) )
    {
      SmiEnvironment::SetUser( user ); // TODO: Check for valid user/pswd
      ok = true;
    }
    else
    {
      string errMsg;
      SmiEnvironment::GetLastErrorCode( errMsg );
      cout << "Error: " << errMsg << endl;
      ok = false;
    }
  }
  if (ok)
  {
    cout << "Initializing the SECONDO System ... " << endl;
    ss = new SecondoSystem( &GetAlgebraEntry );
    nl = SecondoSystem::GetNestedList();
    al = SecondoSystem::GetAppNestedList();
    ok = SecondoSystem::StartUp();
  }

  // set the maximum memory which may be allocated by operators
  QueryProcessor& qp = *SecondoSystem::GetQueryProcessor();

  long keyVal = 
    SmiProfile::GetParameter("QueryProcessor", "MaxMemPerOperator", 
                             16 * 1024, parmFile);
  qp.SetMaxMemPerOperator(keyVal*1024);
  cmsg.info() << "Memory usage per operator limited by " 
              << keyVal << "kb" << endl;

  keyVal = 
    SmiProfile::GetParameter("System", "FLOBCacheSize", 
                             16*1024, parmFile);
  qp.InitializeFLOBCache( keyVal*1024 );
  
  cmsg.info() << "FLOB Cache size " 
              << keyVal << "kb" << endl;
  cmsg.send();

  SystemTables& st = SystemTables::getInstance();
  st.insert(&cmdCtrRel);
  st.insert(&cmdTimesRel);
  st.insert(&devObjRel);
  
  initialized = ok;
  return (ok);
}


/*

1.4 Terminate method

*/


void
SecondoInterface::Terminate()
{
  const string bullet(" - ");
  if ( initialized )
  {    
    cmsg.info() << "Terminating the secondo interface instance ..." << endl;
    cmsg.send();
    // --- Abort open transaction, if there is an open transaction
    if ( activeTransaction )
    {
      SecondoSystem::AbortTransaction();
      activeTransaction = false;
    }
    // --- Close database, if one is open
    if ( SecondoSystem::GetInstance()->IsDatabaseOpen() )
    {
      SecondoSystem::GetInstance()->CloseDatabase();
    }
    if ( derivedObjPtr != 0 ) { // The destructor closes a relation object 
      cmsg.info() << "Closing system tables ..." << endl;
      cmsg.send();
      delete derivedObjPtr;
      derivedObjPtr = 0;
    }    
    if ( !SecondoSystem::ShutDown() )
    {
      cmsg.error() << bullet << "Error: SecondoSytem::Shutdown() failed." 
                   << endl;
      cmsg.send();
    }
    if ( ss != 0 )
    {
      delete ss;
      ss = 0;
    }

    if ( !SmiEnvironment::ShutDown() )
    {
      string errMsg;
      SmiEnvironment::GetLastErrorCode( errMsg );
      cmsg.error() << bullet << "Error: SmiEnvironment::ShutDown() failed." 
                   << endl;
      cmsg.error() << bullet << "Error: " << errMsg << endl;
      cmsg.send();
    }
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
                           const string& resultFileName /*="SecondoResult"*/ )
{
/*

~Secondo~ reads a command and executes it; it possibly returns a result.
The command is one of a set of SECONDO commands.

Error Codes: see definition module.

If value 0 is returned, the command was executed without error. 

To avoid redundant maintainance of error messages in the C++ code and in the
Java-GUI, the errorcode will not longer be used and code -1 will be returned
in case of an error. All error messages shoold be stored in the ErrorReporter
as a formatted string with linebreaks and this string will be returned.

Messages send to ~cerr~ or ~cout~ should be avoided, since they can not be
transferred to the client. If necessary please use the ~cmsg~ object explained
in the file "include/LogMsg.h"

Please create new functions for implementation of new commands, since this
function conatins already many lines of code. Some commands have been moved to
separate functions which should be named Command\_<name>.
 

*/

  // initialize variable parameters
  errorMessage = "";
  errorCode    = 0;
  errorPos     = 0;
  resultList   = nl->TheEmptyList();

  StopWatch cmdTime;  // measure the time used for executing the command.

  NestedList* nl = SecondoSystem::GetNestedList();
  NestedList* al = SecondoSystem::GetAppNestedList();

  // copy command list to internal NL memory
  ListExpr commandLE2 = nl->TheEmptyList();
  if (commandLE) 
  {
     commandLE2 = al->CopyList(commandLE, nl);
  }

  // The error list which may be extended by some catalog commands
  ListExpr errorList    = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );
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
     //abort command execution
     constructErrMsg(errorCode, errorMessage);
     return;
  }

  // Initialize Counters for the creation and deletion of objects
  LOGMSG( "SI:RelStatistics", 
    Tuple::SetCounterReport( true ); 
    ShowStandardTypesStatistics( true ); 
  ) 


  //*** START COMMAND RECOGNITION ***//
  
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
        else
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
            errorCode = ERR_IDENT_USED;   // identifier already used
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
            errorCode = ERR_IDENT_UNKNOWN_DB_NAME; 
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
          if ( !sys.OpenDatabase( dbName ) )
          {
           // identifier not a known database name
            errorCode = ERR_IDENT_UNKNOWN_DB_NAME; 
          } 
          else if ( !derivedObjPtr )
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
          case ERR_IDENT_UNKNOWN_DB_NAME: 
          {
            break;
          }         
          case ERR_IN_DEFINITIONS_FILE:
          {
            resultList = errorList;
          }
          case ERR_DB_NAME_NEQ_IDENT:
          case ERR_PROBLEM_IN_READING_FILE:
          case ERR_IN_LIST_STRUCTURE_IN_FILE:

          default: 
          {       
            cmsg.info() << endl 
                         << "Error during restore detected. Closing database " 
                         << dbName;
            cmsg.send();
        
            if ( !sys.CloseDatabase() ) 
            {
              cmsg.info() << " failed!" << endl;
            } 
            else 
            {
              cmsg.info() << " finished!" << endl;
            }
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

  //*** END COMMAND RECOGNITION ***//
  
  if ( resultAsText )
  {
    nl->WriteToFile( resultFileName, resultList );
  }

  LOGMSG( "SI:ResultList",
    cmsg.info() << endl << "### Result List before copying: " 
          << nl->ToString(resultList) << endl;
    cmsg.send();
  )

  // copy result into application specific list container.
  StopWatch copyTime;
  if (resultList) {
     resultList = nl->CopyList(resultList, al);
     LOGMSG( "SI:CopyListTime",
        cmsg.info() << "CopyList " << copyTime.diffTimes() << endl;
        cmsg.send();
     )
  }
  LOGMSG( "SI:ResultList",
    cmsg.info() << endl << "### Result after copying: " 
         << al->ToString(resultList) << endl;
    cmsg.send();           
  )
  nl->initializeListMemory();
 
  // initialize and increment query counter 
  static int CmdNr = 0;
  CmdNr++;

  // handle command times
  bool printCmdTimes = RTFlag::isActive("SI:PrintCmdTimes");
  bool dumpCmdTimes = RTFlag::isActive("SI:DumpCmdTimes");

  double treal = cmdTime.diffSecondsReal();
  double tcpu = cmdTime.diffSecondsCPU();
  CmdTimes* ctp = new CmdTimes(CmdNr, commandText, treal, tcpu);
  cmdTimesRel.append(ctp, dumpCmdTimes);
  
  if (printCmdTimes) 
  {
    cmsg.info() << endl << "Command " << cmdTime.diffTimes() << endl;
    cmsg.send();
  } 
    
  // handle counters
  bool printCtrs = RTFlag::isActive("SI:PrintCounters");
  bool dumpCtrs = RTFlag::isActive("SI:DumpCounters");
  Counter::Str2ValueMap cm = Counter::usedCounters();
  Counter::Str2ValueMap::iterator it = cm.begin();
  while ( it != cm.end() )
  {
    CmdCtr* cp = new CmdCtr(CmdNr, it->first, it->second);
    cmdCtrRel.append(cp, dumpCtrs);
    it++;
  }

  if (printCtrs)
  {  
    Counter::reportValues(CmdNr);
  }  
  Counter::resetAll();
   
  constructErrMsg(errorCode, errorMessage);
  
  return;
}


void
SecondoInterface::constructErrMsg(int errorCode, string& errorMessage)
{
  if ( errorCode != 0) {
    // translate error code into text
    // check if the queryprocessor reports errors
    string repMsg("");
    ErrorReporter::GetErrorMessage(repMsg);
    ErrorReporter::Reset();
    errorMessage += repMsg + "\n";

    cmsg.send(); // flush cmsg buffer
    if ( isCSImpl ) {
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
  
  StartCommand();

  StopWatch queryTime;
  cmsg.info() << "Analyze query ..." << endl;
  cmsg.send();

  qp.Construct( nl.Second( list ), correct, evaluable, defined,
                isFunction, tree, resultType );


  if (!RTFlag::isActive("SI:NoQueryAnalysis")) {
    cmsg.info() << "Analyze " << queryTime.diffTimes() << endl;
    cmsg.send();
    queryTime.start();
  }

  if ( !defined ) // Undefined object value
  {
    return ERR_UNDEF_OBJ_VALUE;  
  }
  
  if ( !correct ) // Error in query
  {
    return ERR_IN_QUERY_EXPR; 
  }

  if ( evaluable )
  {
     cmsg.info() << "Execute ..." << endl;
     cmsg.send();

     qp.ResetTimer();
     qp.Eval( tree, result, 1 );
     
     ListExpr valueList = ctlg.OutObject( resultType, result );
     resultList = nl.TwoElemList( resultType, valueList );
     
     StopWatch destroyTime;
     qp.Destroy( tree, true );
     if ( RTFlag::isActive("SI:DestroyOpTreeTime") ) {
       cmsg.info() << "Destroy " << destroyTime.diffTimes() << endl;
       cmsg.send();
     }

     if (!RTFlag::isActive("SI:NoQueryAnalysis")) 
     {
       cmsg.info() << "Execute " << queryTime.diffTimes() << endl;
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
  else if ( isFunction ) // abstraction or function object
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
  else
  {
    errorCode = ERR_EXPR_NOT_EVALUABLE;  // Query not evaluable   
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
        qp.Construct( valueExpr, correct, evaluable, defined,
                      isFunction, tree, resultType );
                      
        if ( !defined ) // Undefined object value in expression
        {
          errorCode = ERR_UNDEF_OBJ_VALUE;      
        }
        else if ( correct )
        {
          if ( evaluable || isFunction )
          {
            string typeName = "";
            ctlg.CreateObject(objName, typeName, resultType, 0);
          }
          if ( evaluable )
          {
            qp.Eval( tree, result, 1 );

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
          }
          else // Expression not evaluable
          {
            errorCode = ERR_EXPR_NOT_EVALUABLE;   
          }
        }
        else // Error in expression
        {
          errorCode = ERR_IN_QUERY_EXPR;    
        }
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
      qp.Construct( valueExpr, correct, evaluable, defined,
                    isFunction, tree, resultType );
                     
      if ( !defined ) // Undefined object value in expression
      {
        errorCode = ERR_UNDEF_OBJ_VALUE;      
      }
      else if ( correct )
      {
        if ( ctlg.IsSystemObject(objName) ) 
        {
          errorCode = ERR_IDENT_RESERVED;      
        } 
        else if ( derivedObjPtr && derivedObjPtr->isDerived(objName) ) 
        {
          errorCode = ERR_UPDATE_FOR_DERIVED_OBJ_UNSUPPORTED;
        } 
        else if ( !ctlg.IsObjectName( objName ) )
        {
          // identifier not a known object name
          errorCode = ERR_IDENT_UNKNOWN_OBJ;   
        }
        else
        {       
          ListExpr typeExpr = ctlg.GetObjectTypeExpr( objName );

          if ( !nl.Equal( typeExpr, resultType ) )
          {
            // types of object and expression do not agree
            errorCode = ERR_EXPR_TYPE_NEQ_OBJ_TYPE;   
          }
          else if ( evaluable )
          {
            qp.Eval( tree, result, 1 );
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
          else if ( isFunction )   // abstraction or function object
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
          }
          else // Expression not evaluable
          {
            errorCode = ERR_EXPR_NOT_EVALUABLE;   
          }
        }
      }
      else // Error in expression
      {
        errorCode = ERR_IN_QUERY_EXPR;    
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
  if ( !activeTransaction )
  {
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
  }
  return true;
}


void
SecondoInterface::SetDebugLevel( const int level )
{
  SecondoSystem::GetQueryProcessor()->SetDebugLevel( level );
}
