/*

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

October 2003 M. Spiekermann made the command echo (printing out the command in NL format) configurable.
This is useful for server configuration, since the output of big lists consumes more time than processing
the command.

May 2004, M. Spiekermann. Support of derived objects (for further Information see DerivedObj.h) introduced.
A new command derive similar to let can be used by the user to create objects which are derived from other 
objects via a more or less complex value expression. The information about dependencies is stored in two 
system tables (relation objects). The save database command omits to save list expressions for those objects.
After restoring all saved objects the derived objects are rebuild in the restore database command.

August 2004, M. Spiekermann. The complex nesting of function ~Secondo~ has been reduced.

Sept 2004, M. Spiekermann. A bug in the error handling of restore databases has been fixed. 

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
#include "LogMsg.h"

#include "RelationAlgebra.h"
#include "StandardTypes.h"

#include "SecondoSMI.h"
#include "SecParser.h"
#include "StopWatch.h"
#include "LogMsg.h"
#include "Counter.h"
#include "DerivedObj.h"

extern AlgebraListEntry& GetAlgebraEntry( const int j );

static SecondoSystem* ss = 0;

/**************************************************************************

1 Implementation of class SecondoInterface

*/

int
DerivedObj::ObjRecord::id = 0;

/*

1.1 Constructor

*/

SecondoInterface::SecondoInterface()
  : initialized( false ), activeTransaction( false ), nl( 0 ), server( 0 ), derivedObjPtr(0)
{
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
                              string& parmFile, const bool multiUser )
{
  bool ok = false;
  bool found = false;
  cout << endl << "Initializing the SECONDO Interface ..." << endl;
  if ( parmFile.length() > 0 ) // try file specified by cmd switch -c
  {
    cout << "  Configuration file '" << parmFile;
    found = FileSystem::FileOrFolderExists( parmFile );
    if ( found )
    {
      cout << "':" << endl;
    }
    else
    {
      cout << "' not found!" << endl;
    }
  }

  if ( !found ) // try environment variable 
  {
    cout << "Searching environment for configuration file ..." << endl;
    char* home = getenv( "SECONDO_CONFIG" );
    if ( home != 0 )
    {
      parmFile = home;
      FileSystem::AppendSlash( parmFile );
      parmFile += "SecondoConfig.ini";
      cout << "  Configuration file '" << parmFile;
      found = FileSystem::FileOrFolderExists( parmFile );
      if ( found )
      {
  cout << "':" << endl;
      }
      else
      {
  cout << "' not found!" << endl;
      }
    }
    else
    {
      cout << "  Environment variable SECONDO_CONFIG not defined." << endl;
    }
  }

  if ( !found ) // try current directory
  {
    cout << "Searching current directory for configuration file ..." << endl;
    string cwd = FileSystem::GetCurrentFolder();
    FileSystem::AppendSlash( cwd );
    parmFile = cwd + "SecondoConfig.ini";
    cout << "  Configuration file '" << parmFile;
    found = FileSystem::FileOrFolderExists( parmFile );
    if ( found )
    {
      cout << "':" << endl;
    }
    else
    {
      cout << "' not found!" << endl;
    }
  }

  if ( !found ) // still no config file defined! 
  {
    cout << "Error: No configuration file specified." << endl;
    return (false);
  }

  // initialize runtime flags
  InitRTFlags(parmFile);

  string value, foundValue;
  if ( SmiProfile::GetParameter( "Environment", "SecondoHome", "", parmFile ) == "")
  {
    cout << "Error: Secondo home directory not specified." << endl;
  }
  else
  {
    ok = true;
  }

  if ( ok )
  {
    // --- Check storage management interface
    SmiEnvironment::RunMode mode =  SmiEnvironment::SingleUser;
    if ( RTFlag::isActive("SMI:NoTransactions") ) {
       
       mode = SmiEnvironment::SingleUserSimple;
       cout << "  Setting mode to SingleUserSimple" << endl;
       
    } else { // Transactions and logging are used
    
       mode = (multiUser) ? SmiEnvironment::MultiUser : SmiEnvironment::SingleUser;
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
  initialized = ok;
  return (ok);
}


/*

1.4 Terminate method

*/


void
SecondoInterface::Terminate()
{
  if ( initialized )
  {    
    if ( derivedObjPtr != 0 ) { // The destructor closes a relation object 
      delete derivedObjPtr;
      derivedObjPtr = 0;
    }    

    cout << "Terminating Secondo system ...";
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
    if ( SecondoSystem::ShutDown() )
    {
      cout << "completed." << endl;
    }
    else
    {
      cout << "failed." << endl;
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
      cout << "Error: Shutdown of the storage management interface failed." << endl;
      cout << "Error: " << errMsg << endl;
    }
    initialized = false;
    activeTransaction = false;
    nl = 0;
    al = 0;
    server = 0;
  }
  else
  {
    cout << "Error: Secondo system already terminated." << endl;
  }
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
                           const string& resultFileName /* = "SecondoResult" */ )
{
/*

~Secondo~ reads a command and executes it; it possibly returns a result.
The command is one of a set of SECONDO commands.

Error Codes: see definition module.

If value 0 is returned, the command was executed without error.

*/

  ListExpr first, list, typeExpr, resultType, modelList, valueExpr;
  first = list = typeExpr = resultType = modelList = valueExpr = nl->TheEmptyList();
  
  ListExpr typeExpr2, errorList,  errorInfo, functionList;
  typeExpr2 = errorList = errorInfo = functionList = nl->TheEmptyList();         
         
  string filename = "", dbName = "", objName = "", typeName = "";

  Word result = SetWord( Address(0) );
  OpTree tree = 0;
  
  int length = 0;
  bool correct      = false;
  bool evaluable    = false;
  bool defined      = false;
  bool isFunction   = false;
  int message = 0;                /* error code from called procedures */
  string listCommand = "";         /* buffer for command in list form */
  AlgebraLevel level = ExecutableLevel;

  StopWatch cmdTime;  // measure the time used for executing the command.

  SecParser sp;
  NestedList* nl = SecondoSystem::GetNestedList();
  NestedList* al = SecondoSystem::GetAppNestedList();


  // copy command list to internal NL memory
  ListExpr commandLE2 = nl->TheEmptyList();
  if (commandLE) {
     commandLE2 = al->CopyList(commandLE, nl);
  }


  errorMessage = "";
  errorCode    = 0;
  errorPos     = 0;
  errorList    = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );
  errorInfo    = errorList;
  resultList   = nl->TheEmptyList();

  switch (commandLevel)
  {
    case 0:  // executable, list form
    {
      level = ExecutableLevel;
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
      level = ExecutableLevel;
      if ( sp.Text2List( commandText, listCommand, errorMessage ) != 0 )
      {
        errorCode = ERR_SYNTAX_ERROR;  // syntax error in command/expression
      }
      else if ( !nl->ReadFromString( listCommand, list ) )
      {
        errorCode = ERR_SYNTAX_ERROR;  // syntax error in command/expression
      }
      break;
    }
    case 2:
    {
      level = DescriptiveLevel;
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
    case 3:
    {
      level = DescriptiveLevel;
      if ( sp.Text2List( commandText, listCommand, errorMessage ) != 0 )
      {
        errorCode = ERR_SYNTAX_ERROR;  // syntax error in command/expression
      }
      else if ( !nl->ReadFromString( listCommand, list ) )
      {
        errorCode = ERR_SYNTAX_ERROR;  // syntax error in command/expression
      }
      break;
    }
    default:
    {
      errorCode = ERR_CMD_LEVEL_NOT_YET_IMPL;  // Command level not implemented
      return;
    }
  } // switch
  if ( errorCode != 0 )
  {
    return;
  }
  SecondoSystem::SetAlgebraLevel( level );

  // local references of important objects
  QueryProcessor& qp = *SecondoSystem::GetQueryProcessor();
  SecondoCatalog& ctlg = *SecondoSystem::GetCatalog(level);
  SecondoSystem& sys = *SecondoSystem::GetInstance();


  if (!RTFlag::isActive("SI:NoCommandEcho")) {
     nl->WriteListExpr(list, cerr);
     cerr << endl;
  }
  length = nl->ListLength( list );
  if ( length > 1 )
  {
    first = nl->First( list );

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
            errorCode = ERR_IDENT_UNKNOWN_DB_NAME;  // identifier not a known database name
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
            errorCode = ERR_IDENT_UNKNOWN_DB_NAME;  // identifier not a known database name
      
          } else {
      if ( !derivedObjPtr )
         derivedObjPtr = new DerivedObj(); // create new instance if necessary
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
    if ( derivedObjPtr );
      delete derivedObjPtr;
    derivedObjPtr = 0;
        }
      }
      else if ( nl->IsEqual( first, "save" ) && (length == 4) &&
                nl->IsEqual( nl->Third( list ), "to" ) &&
                nl->IsAtom( nl->Fourth( list )) &&
               (nl->AtomType( nl->Fourth( list )) == SymbolType) )
      {
        if ( !sys.IsDatabaseOpen() )
        {
          errorCode = ERR_NO_DATABASE_OPEN;  // no database open
        }
        else
        {
          StartCommand();
          filename = nl->SymbolValue( nl->Fourth( list ) );
          if ( !sys.SaveDatabase( filename, *derivedObjPtr ) )
          {
            errorCode = ERR_PROBLEM_IN_WRITING_TO_FILE;  // Problem in writing to file
          }
          FinishCommand( errorCode );
        }
      }
      else if ( nl->IsEqual( first, "restore" ) &&
               (length == 5) && nl->IsAtom( nl->Third( list )) &&
               (nl->AtomType( nl->Third( list )) == SymbolType) &&
                nl->IsEqual( nl->Fourth( list ), "from" ) &&
                nl->IsAtom( nl->Fifth( list )) &&
               (nl->AtomType( nl->Fifth( list )) == SymbolType) )
      {
      
        if ( sys.IsDatabaseOpen() )
        {
          errorCode = ERR_DATABASE_OPEN;  // database open
        }
        else
        {
          cout << endl;
          dbName = nl->SymbolValue( nl->Third( list ) );
          filename = nl->SymbolValue( nl->Fifth( list ) );
          errorCode = sys.RestoreDatabase( dbName, filename, errorInfo );          
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
      cmsg.error() << endl << "Error during restore detected. Closing database " << dbName;
        
            if ( !sys.CloseDatabase() ) {
    cmsg.error() << " failed!" << endl;
      } else {
        cmsg.error() << " finished!" << endl;
            }
            cmsg.send();
            break;
          }
        } 
          
        if ( !errorCode ) { // rebuild derived objects if present
              
          if ( derivedObjPtr ) {
      delete derivedObjPtr;            
    }    
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

    else if ( nl->IsEqual( first, "save" ) &&
              nl->IsEqual( nl->Third( list ), "to" ) && (length == 4) &&
              nl->IsAtom( nl->Second( list )) &&
              nl->AtomType( nl->Second( list )) == SymbolType &&
        nl->IsAtom( nl->Fourth( list )) &&
              nl->AtomType( nl->Fourth( list )) == SymbolType )
    {
      filename = nl->SymbolValue( nl->Fourth( list ) );
      string objectname = nl->SymbolValue( nl->Second( list ) );

      if ( !sys.IsDatabaseOpen() )
      {
        errorCode = ERR_NO_DATABASE_OPEN;  // no database open
      }
      else
      {
        if ( !sys.IsDatabaseObject( objectname ) )
  {
    errorCode = ERR_IDENT_UNKNOWN_DB_OBJECT; // object is not known in the database
  }
        else
        {
          StartCommand();
          if ( !sys.SaveObject( objectname,
      filename ) )
          {
            errorCode = ERR_PROBLEM_IN_WRITING_TO_FILE;  // Problem in writing to file
          }
          FinishCommand( errorCode );
        }
      }
    }

    // --- Reading command for objects

    else if ( nl->IsEqual( first, "restore" ) &&
             (length == 4) && nl->IsAtom( nl->Second( list )) &&
             (nl->AtomType( nl->Second( list )) == SymbolType) &&
              nl->IsEqual( nl->Third( list ), "from" ) &&
              nl->IsAtom( nl->Fourth( list )) &&
             (nl->AtomType( nl->Fourth( list )) == SymbolType) )
    {
      if ( !sys.IsDatabaseOpen() )
      {
        errorCode = ERR_NO_DATABASE_OPEN;  // no database open
      }
      else
      {
        objName = nl->SymbolValue( nl->Second( list ) );
        filename = nl->SymbolValue( nl->Fourth( list ) );

        if ( sys.IsDatabaseObject( objName ) )
  {
    errorCode = ERR_IDENT_ALREADY_KNOWN_IN_DB; // object is already known in the database
  }
  else
  {

          message = sys.RestoreObjectFromFile( objName, filename, errorInfo );
          switch (message)
          {
            case 0:
              break;
            case 1:
              errorCode = ERR_OBJ_NAME_IN_FILE_NEQ_IDENT;  // object name in file different from identifier
              break;
            case 2:
              errorCode = ERR_PROBLEM_IN_READING_FILE;  // problem in reading the file
              break;
            case 3:
              errorCode = ERR_IN_LIST_STRUCTURE_IN_FILE;  // error in list structure in file
              break;
            case 4:
              errorCode = ERR_IN_DEFINITIONS_FILE;  // error in object definition in file
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
          resultList =
      nl->TwoElemList(nl->SymbolAtom("inquiry"),
          nl->TwoElemList(nl->SymbolAtom("algebras"),
            SecondoSystem::GetAlgebraManager( )->ListAlgebras() ));
      }

      else if (     nl->IsEqual( nl->Second( list ), "algebra" ) 
          && (length == 3) 
                &&  nl->IsAtom(nl->Third(list)) 
                &&  nl->AtomType( nl->Second( list ) ) == SymbolType )
      {
          if ( SecondoSystem::GetAlgebraManager( )->GetAlgebraId(
         nl->SymbolValue( nl->Third(list) )))
    {
      int aid = SecondoSystem::GetAlgebraManager( )->GetAlgebraId(
         nl->SymbolValue( nl->Third(list) ));
         
      ListExpr constOp =  nl->TwoElemList( ctlg.ListTypeConstructors( aid ),
             ctlg.ListOperators( aid ));
      resultList =
      nl->TwoElemList( nl->SymbolAtom("inquiry"),
           nl->TwoElemList( nl->SymbolAtom("algebra"),
                nl->TwoElemList( nl->Third(list), constOp )));
    }
    else errorCode = ERR_ALGEBRA_UNKNOWN;
      }


      else if (     nl->IsEqual( nl->Second( list ), "type" ) 
                && (length == 3) 
          &&  nl->IsEqual( nl->Third( list ), "constructors" ) )
      {
        resultList = nl->TwoElemList( nl->SymbolAtom("inquiry"),
               nl->TwoElemList( nl->SymbolAtom("constructors"),
                   ctlg.ListTypeConstructors() ));
      }
      else if ( nl->IsEqual( nl->Second(list), "operators" ) )
      {
        resultList = nl->TwoElemList( nl->SymbolAtom("inquiry"),
              nl->TwoElemList( nl->SymbolAtom("operators"),
                                                       ctlg.ListOperators() ));
      }
      else if ( nl->IsEqual( nl->Second( list ), "databases" ) )
      {
        resultList = nl->TwoElemList( nl->SymbolAtom("inquiry"),
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
          FinishCommand( errorCode );
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
          FinishCommand( errorCode );
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
          FinishCommand( errorCode );
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
        typeExpr = nl->Fourth( list );
        typeExpr2 = ctlg.ExpandedType( typeExpr );
  
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
        FinishCommand( errorCode );
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
          message = ctlg.DeleteType( typeName );
          if ( message == 1 )
          {
            errorCode = ERR_TYPE_NAME_USED_BY_OBJ;   // Type used by an object
          }
          else if ( message == 2 )
          {
            errorCode = ERR_IDENT_UNKNOWN_TYPE;   // identifier not a known type name
          }
          FinishCommand( errorCode );
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
     errorCode = ERR_NO_DATABASE_OPEN;  // no database open

        if ( !errorCode && ctlg.IsSystemObject(objName) )
           errorCode = ERR_IDENT_RESERVED;

        StartCommand();  
        if ( !errorCode ) 
        {
           message = ctlg.DeleteObject( objName );
           
           if ( message > 0 ) {
             errorCode = ERR_IDENT_UNKNOWN_OBJ;   // identifier not a known object name          
             
     } else {
       derivedObjPtr->deleteObj( objName ); // delete from derived objects table if necessary
           }
         }
        FinishCommand( errorCode );
  
     } else { // no correct delete syntax
     
       errorCode = ERR_CMD_NOT_RECOGNIZED;  // Command not recognized
     }
    }

    // --- Create object command

    else if ( nl->IsEqual( first, "create" ) && (length == 4) &&
              nl->IsAtom( nl->Second( list )) &&
             (nl->AtomType( nl->Second( list ) ) == SymbolType) &&
              nl->IsEqual( nl->Third( list ), ":" ) )
    {
      errorCode = Command_Create( level, list, resultList, errorList );   
    }

    // --- Update object command

    else if ( nl->IsEqual( first, "update" ) && (length == 4) &&
              nl->IsAtom( nl->Second( list )) &&
             (nl->AtomType( nl->Second( list ) ) == SymbolType) &&
              nl->IsEqual( nl->Third( list ), ":=" ) )
    {
          errorCode = Command_Update( level, list );    
    }

    // --- Let command

    else if ( nl->IsEqual( first, "let" ) && (length == 4) &&
              nl->IsAtom( nl->Second( list )) &&
             (nl->AtomType( nl->Second( list ) ) == SymbolType) &&
              nl->IsEqual( nl->Third( list ), "=" ) )
    {
      errorCode = Command_Let( level, list );       
    }

    // --- derive command

    else if ( nl->IsEqual( first, "derive" ) && (length == 4) &&
              nl->IsAtom( nl->Second( list )) &&
             (nl->AtomType( nl->Second( list ) ) == SymbolType) &&
              nl->IsEqual( nl->Third( list ), "=" ) )
    {     
      errorCode = Command_Derive( level, list );         
    }
 
    // --- Query command

    else if ( nl->IsEqual( first, "query" ) && (length == 2) )
    {
      errorCode = Command_Query( level, list, resultList, errorMessage );   
    }

    // --- Model command

    else if ( nl->IsEqual( first, "model" ) && (length == 2) )
    {
      if ( SecondoSystem::GetInstance()->IsDatabaseOpen() )
      {
        StartCommand();
        qp.Construct( level, nl->Second( list ), correct, evaluable, defined,
                      isFunction, tree, resultType );
          
        if ( !defined )
        {
          errorCode = ERR_UNDEF_OBJ_VALUE;         // Undefined object value
        }
        else if ( correct )
        {
          if ( evaluable )
          {
            qp.EvalModel( tree, result );
            modelList = ctlg.OutObjectModel( resultType, result );
            resultList = nl->TwoElemList( resultType, modelList );
            qp.Destroy( tree, true );
          }
          else
          {
            errorCode = ERR_EXPR_NOT_EVALUABLE;   // Query not evaluable
          }
        }
        else
        {
          errorCode = ERR_IN_QUERY_EXPR;     // Error in query
        }
        FinishCommand( errorCode );
      }
      else
      {
        errorCode = ERR_NO_DATABASE_OPEN;       // no database open
      }
    }
    else
    {
      errorCode = ERR_CMD_NOT_RECOGNIZED;         // Command not recognized
    }
  }
  if ( resultAsText )
  {
    nl->WriteToFile( resultFileName, resultList );
  }
  SecondoSystem::SetAlgebraLevel( UndefinedLevel );


  if (RTFlag::isActive("SI:PrintCounters")) {
    Counter::reportValues();
    Counter::resetAll();
  }

  // Creation and deletion of Objects
  LOGMSG( "SI:RelStatistics", 
    Tuple::ShowTupleStatistics( true, cmsg.info() ); 
    ShowStandardTypesStatistics( true, cmsg.info() ); 
    cmsg.send();
  ) 

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
  
  if ( RTFlag::isActive("SI:CommandTime") ){
   
    static int nr = 0;
    static bool writeHeadLine = true;
    const string logFile="cmd-times.csv"; 
    const string sep="|";
   
    if ( writeHeadLine ) {
      cmsg.file(logFile) << "#nr" << sep
                         << "command" << sep
                         << "realtime" << sep
                         << "cpu-time" << endl; 
      cmsg.send();
      writeHeadLine = false; 
    } 

    nr++;
    cmsg.file(logFile) << nr << sep << commandText
                       << sep << cmdTime.diffSecondsReal()
                       << sep << cmdTime.diffSecondsCPU() << endl;
    cmsg.send();                
    cmsg.info() << endl << "Command " << cmdTime.diffTimes() << endl;
    cmsg.send();
 } 
  
}


/*
1.2 Implementation of commands

1.2.1 query

*/

SI_Error 
SecondoInterface::Command_Query( const AlgebraLevel level,
                                 const ListExpr list, 
                                 ListExpr& resultList,
                                 string& errorMessage )
{
  QueryProcessor& qp = *SecondoSystem::GetQueryProcessor();
  SecondoCatalog& ctlg = *SecondoSystem::GetCatalog(level);
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
  
  if ( level == DescriptiveLevel ) // Command not yet implemented at this level
  {
     return ERR_CMD_NOT_IMPL_AT_THIS_LEVEL;  
  } 
              
  StartCommand();

  StopWatch queryTime;
  cmsg.info() << "Analyze query ..." << endl;
  cmsg.send();

  qp.Construct( level, nl.Second( list ), correct, evaluable, defined,
                isFunction, tree, resultType );


  if (!RTFlag::isActive("SI:NoQueryAnalysis")) {
    cmsg.info() << "Analyze " << queryTime.diffTimes() << endl;
    cmsg.send();
    queryTime.start();
    //cerr << nl->ReportTableSizes() << endl;
  }

  if ( !defined ) // Undefined object value
  {
    return ERR_UNDEF_OBJ_VALUE;  
  }
  
  if ( !correct ) // Error in query
  {
    ErrorReporter::GetErrorMessage(errorMessage); 
    ErrorReporter::Reset();
    return ERR_IN_QUERY_EXPR; 
  }

  if ( evaluable )
  {
     cmsg.info() << "Execute ..." << endl;
     cmsg.send();

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
       cmsg.info() << nl.ReportTableSizes() << endl;
       cmsg.send();
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
    ErrorReporter::GetErrorMessage(errorMessage);
    ErrorReporter::Reset();
    errorCode = ERR_EXPR_NOT_EVALUABLE;  // Query not evaluable   
  }
  
  SmiEnvironment::SetFlag_NOSYNC(true);
  FinishCommand( errorCode );
  SmiEnvironment::SetFlag_NOSYNC(false);
  
  return errorCode;
}

/*

1.2.2 derive

*/


SI_Error 
SecondoInterface::Command_Derive( const AlgebraLevel level, const ListExpr list )
{
  SecondoCatalog& ctlg = *SecondoSystem::GetCatalog(level);
  SecondoSystem& sys = *SecondoSystem::GetInstance();
  NestedList& nl = *SecondoSystem::GetNestedList();
  
  SI_Error errorCode = ERR_NO_ERROR;
  
  if ( !sys.IsDatabaseOpen() )  // no database open
  {
    return ERR_NO_DATABASE_OPEN;        
  }

  if ( !errorCode && (level == DescriptiveLevel) )  // Command not yet implemented at this level
  {
    return ERR_CMD_NOT_IMPL_AT_THIS_LEVEL; 
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

  FinishCommand( errorCode );
  
  return errorCode;
}

/*

1.2.3 let

*/


SI_Error 
SecondoInterface::Command_Let( const AlgebraLevel level,
                               const ListExpr list  )
{
  QueryProcessor& qp = *SecondoSystem::GetQueryProcessor();
  SecondoCatalog& ctlg = *SecondoSystem::GetCatalog(level);
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
    if ( level == DescriptiveLevel ) // Command not yet implemented at this level
    {
      errorCode = ERR_CMD_NOT_IMPL_AT_THIS_LEVEL;  
    }
    else
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
        qp.Construct( level, valueExpr, correct, evaluable, defined,
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
               ListExpr functionList = ctlg.GetObjectValue( nl.SymbolValue( valueExpr ) );
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
      FinishCommand( errorCode );
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
SecondoInterface::Command_Update( const AlgebraLevel level,
                                  const ListExpr list       )
{
  QueryProcessor& qp = *SecondoSystem::GetQueryProcessor();
  SecondoCatalog& ctlg = *SecondoSystem::GetCatalog(level);
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
    if ( level == DescriptiveLevel ) // Command not implemented at this level
    {
      errorCode = ERR_CMD_NOT_IMPL_AT_THIS_LEVEL;  
    }
    else
    {
      StartCommand();
      string objName = nl.SymbolValue( nl.Second( list ) );
      ListExpr valueExpr = nl.Fourth( list );
      qp.Construct( level, valueExpr, correct, evaluable, defined,
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
              ListExpr functionList = ctlg.GetObjectValue( nl.SymbolValue( valueExpr ) );
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
      FinishCommand( errorCode );
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
SecondoInterface::Command_Create( const AlgebraLevel level,
                                  const ListExpr list, 
                                  ListExpr& resultList,
                                  ListExpr& errorList   )
{
  SecondoCatalog& ctlg = *SecondoSystem::GetCatalog(level);
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
    FinishCommand( errorCode );
  }
  else // no database open
  {
    errorCode = ERR_NO_DATABASE_OPEN;       
  }

  return errorCode;
}


/*
1.3 Procedure ~NumericTypeExpr~

*/
ListExpr
SecondoInterface::NumericTypeExpr( const AlgebraLevel level, const ListExpr type )
{
  SecondoSystem::SetAlgebraLevel( level );
  ListExpr list = nl->TheEmptyList();
  if ( SecondoSystem::GetInstance()->IsDatabaseOpen() )
  {
    // use application specific list memory
    list = SecondoSystem::GetCatalog( level )->NumericType( al->CopyList(type,nl) );
    list = nl->CopyList(list, al);
  }
  SecondoSystem::SetAlgebraLevel( UndefinedLevel );
  return (list);
}

bool
SecondoInterface::GetTypeId( const AlgebraLevel level,
                             const string& name,
                             int& algebraId, int& typeId )
{
  SecondoSystem::SetAlgebraLevel( level );
  bool ok = SecondoSystem::GetCatalog( level )->
              GetTypeId( name, algebraId, typeId );
  SecondoSystem::SetAlgebraLevel( UndefinedLevel );
  return (ok);
}

bool
SecondoInterface::LookUpTypeExpr( const AlgebraLevel level,
                                  ListExpr type, string& name,
                                  int& algebraId, int& typeId )
{
  bool ok = false;
  SecondoSystem::SetAlgebraLevel( level );
  name = "";
  algebraId = 0;
  typeId = 0;
  //cout << al->reportTableStates() << endl;
  //cout << "typeExpr: " << 
  al->ToString(type); // without this an assertion in CTable fails ????

  if ( SecondoSystem::GetInstance()->IsDatabaseOpen() )
  {
    // use application specific list memory
    ok = SecondoSystem::GetCatalog( level )->
           LookUpTypeExpr( al->CopyList(type,nl), name, algebraId, typeId );

  //cout << al->reportTableStates() << endl;

  }
  SecondoSystem::SetAlgebraLevel( UndefinedLevel );
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

void
SecondoInterface::FinishCommand( SI_Error& errorCode )
{
  if ( !activeTransaction )
  {
    if ( errorCode == 0 )
    {
      if ( !SecondoSystem::CommitTransaction() )
      {
        errorCode = ERR_COMMIT_OR_ABORT_FAILED;
      }
    }
    else
    {
      if ( !SecondoSystem::AbortTransaction() )
      {
        cerr << "Error: " << GetErrorMessage(errorCode) << endl;          
        errorCode = ERR_COMMIT_OR_ABORT_FAILED;
      }
    }
  }
}



void
SecondoInterface::SetDebugLevel( const int level )
{
  SecondoSystem::GetQueryProcessor()->SetDebugLevel( level );
}

