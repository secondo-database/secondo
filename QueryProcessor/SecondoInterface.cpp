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

#include "SecondoSMI.h"
#include "SecParser.h"

extern AlgebraListEntry& GetAlgebraEntry( const int j );

static SecondoSystem* ss = 0;

/************************************************************************** 
3.1 The Secondo Procedure 

*/

SecondoInterface::SecondoInterface()
  : initialized( false ), activeTransaction( false ), nl( 0 ), server( 0 )
{
}

SecondoInterface::~SecondoInterface()
{
  if ( initialized )
  {
    Terminate();
  }
}

bool
SecondoInterface::Initialize( const string& user, const string& pswd,
                              const string& host, const string& port,
                              string& parmFile, const bool multiUser )
{
  bool ok = false;
  cout << "Checking configuration ..." << endl;
  if ( parmFile.length() > 0 )
  {
    bool found = false;
    cout << "Configuration file '" << parmFile;
    found = FileSystem::FileOrFolderExists( parmFile );
    if ( found )
    {
      cout << "':" << endl;
    }
    else
    {
      cout << "' not found!" << endl;
    }
    if ( !found )
    {
      cout << "Searching environment for configuration file ..." << endl;
      char* home = getenv( "SECONDO_HOME" );
      if ( home != 0 )
      {
        parmFile = home;
        FileSystem::AppendSlash( parmFile );
        parmFile += "SecondoConfig.ini";
        cout << "Configuration file '" << parmFile;
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
        cout << "Environment variable SECONDO_HOME not defined." << endl;
      }
      if ( !found )
      {
        cout << "Searching current directory for configuration file ..." << endl;
        string cwd = FileSystem::GetCurrentFolder();
        FileSystem::AppendSlash( cwd );
        parmFile = cwd + "SecondoConfig.ini";
        cout << "Configuration file '" << parmFile;
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
    }

    string value, foundValue;
    if ( SmiProfile::GetParameter( "Environment", "SecondoHome", "", parmFile ) == "")
    {
      cout << "Error: Secondo home directory not specified." << endl;
    }
    else
    {
      ok = true;
    }
  }
  else
  {
    cout << "Error: No configuration file specified." << endl;
    return (false);
  }

  if ( ok )
  {
    // --- Check storage management interface
    cout << "Initializing storage management interface ... ";
    if ( SmiEnvironment::StartUp( (multiUser) ? SmiEnvironment::MultiUser
                                              : SmiEnvironment::SingleUser,
                                  parmFile, cout ) )
    {
      SmiEnvironment::SetUser( user ); // TODO: Check for valid user/pswd
      cout << "completed." << endl;
      ok = true;
    }
    else
    {
      cout << "failed." << endl;
      string errMsg;
      SmiEnvironment::GetLastErrorCode( errMsg );
      cout << "Error: " << errMsg << endl;
      ok = false;
    }
  }
  if (ok)
  {
    cout << "Initializing the Secondo system ... ";
    ss = new SecondoSystem( &GetAlgebraEntry );
    nl = SecondoSystem::GetNestedList();
    ok = SecondoSystem::StartUp();
    if ( ok )
    {
      cout << "completed." << endl;
    }
    else
    {
      cout << "failed." << endl;
    }
  }
  initialized = ok;
  return (ok);
}

void
SecondoInterface::Terminate()
{
  if ( initialized )
  {
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
    server = 0;
  }
  else
  {
    cout << "Error: Secondo system already terminated." << endl;
  }
}

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

  ListExpr first,     list,       typeExpr,  resultType,
           valueList, modelList, valueExpr,
           typeExpr2, errorList,  errorInfo, functionList;
  string filename, dbName, objName, typeName;
  Word result, word, model;
  OpTree tree;
  int length;
  bool correct      = false;
  bool evaluable    = false;
  bool defined      = false;
  bool isFunction   = false;
  bool hasNamedType = false;
  int message;                /* error code from called procedures */ 
  string listCommand;         /* buffer for command in list form */
  AlgebraLevel level;

  SecParser sp;
  NestedList* nl = SecondoSystem::GetNestedList();

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
          errorCode = 9;  // syntax error in command/expression
        }
      }
      else
      {
        list = commandLE;
      }
      break;
    }
    case 1:  // executable, text form
    {
      level = ExecutableLevel;
      if ( sp.Text2List( commandText, listCommand, errorMessage ) != 0 )
      {
        errorCode = 9;  // syntax error in command/expression
      }
      else if ( !nl->ReadFromString( listCommand, list ) )
      {
        errorCode = 9;  // syntax error in command/expression
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
          errorCode = 9;  // syntax error in command/expression
        }
      }
      else
      {
        list = commandLE;
      }
      break;
    }
    case 3:
    {
      level = DescriptiveLevel;
      if ( sp.Text2List( commandText, listCommand, errorMessage ) != 0 )
      {
        errorCode = 9;  // syntax error in command/expression
      }
      else if ( !nl->ReadFromString( listCommand, list ) )
      {
        errorCode = 9;  // syntax error in command/expression
      }
      break;
    }
    default:
    {
      errorCode = 31;  // Command level not implemented
      return;
    }
  } // switch
  if ( errorCode != 0 )
  {
    return;
  }
  SecondoSystem::SetAlgebraLevel( level );

  nl->WriteListExpr(list);
  cout << endl;
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
            errorCode = 22;
          }
        }
        else
        {
          errorCode = 20;
        }
      }
      else if ( nl->IsEqual( first, "commit" ) )
      {
        if ( activeTransaction )
        {
          if ( !SecondoSystem::CommitTransaction() )
          {
            errorCode = 23;
          }
          activeTransaction = false;
        }
        else
        {
          errorCode = 21;
        }
      }
      else if ( nl->IsEqual( first, "abort" ) )
      {
        if ( activeTransaction )
        {
          if ( !SecondoSystem::AbortTransaction() )
          {
            errorCode = 23;
          }
          activeTransaction = false;
        }
        else
        {
          errorCode = 21;
        }
      }
      else
      {
        errorCode = 1;  // Command not recognized.
      }
    }

    // --- Database commands

    else if ( nl->IsEqual( nl->Second( list ), "database" ) )
    {
      if ( nl->IsEqual( first, "create" ) && (length == 3) &&
           nl->IsAtom( nl->Third( list ) ) && 
          (nl->AtomType( nl->Third( list ) ) == SymbolType) )
      {
        if ( SecondoSystem::GetInstance()->IsDatabaseOpen() )
        {
          errorCode = 7;  // a database is open
        }
        else
        {
          dbName = nl->SymbolValue( nl->Third( list ) ); 
          if ( SecondoSystem::GetInstance()->CreateDatabase( dbName ) )
          {
            SecondoSystem::GetInstance()->CloseDatabase();
          }
          else
          {
            errorCode = 10;   // identifier already used
          }
        }
      }
      else if ( nl->IsEqual( first, "delete" ) && 
               (length == 3) && nl->IsAtom( nl->Third( list ) ) && 
               (nl->AtomType( nl->Third( list ) ) == SymbolType) )
      {
        if ( SecondoSystem::GetInstance()->IsDatabaseOpen() )
        {
          errorCode = 7;  // a database is open
        }
        else
        {
          dbName = nl->SymbolValue( nl->Third( list ) ); 
          if ( !SecondoSystem::GetInstance()->DestroyDatabase( dbName ) )
          {
            errorCode = 25;  // identifier not a known database name
          }
        }
      }
      else if ( nl->IsEqual( first, "open" ) && 
               (length == 3) && nl->IsAtom( nl->Third( list ) ) && 
               (nl->AtomType( nl->Third( list ) ) == SymbolType) )
      {
        if ( SecondoSystem::GetInstance()->IsDatabaseOpen() )
        {
          errorCode = 7;  // a database is open
        }
        else
        {
          dbName = nl->SymbolValue( nl->Third( list ) ); 
          if ( !SecondoSystem::GetInstance()->OpenDatabase( dbName ) )
          {
            errorCode = 25;  // identifier not a known database name
          }
        }
      }
      else if ( nl->IsEqual( first, "close" ) )
      {
        if ( !SecondoSystem::GetInstance()->IsDatabaseOpen() )
        {
          errorCode = 6;  // no database open
        }
        else
        {
          if ( activeTransaction )
          {
            SecondoSystem::CommitTransaction();
            activeTransaction = false;
          }
          SecondoSystem::GetInstance()->CloseDatabase();
        }                    
      }
      else if ( nl->IsEqual( first, "save" ) && (length == 4) && 
                nl->IsEqual( nl->Third( list ), "to" ) &&
                nl->IsAtom( nl->Fourth( list )) && 
               (nl->AtomType( nl->Fourth( list )) == SymbolType) )
      {
        if ( !SecondoSystem::GetInstance()->IsDatabaseOpen() )
        {
          errorCode = 6;  // no database open
        }
        else
        {
          StartCommand();
          filename = nl->SymbolValue( nl->Fourth( list ) ); 
          if ( !SecondoSystem::GetInstance()->SaveDatabase( filename ) )
          {
            errorCode = 26;  // Problem in writing to file
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
        if ( SecondoSystem::GetInstance()->IsDatabaseOpen() )
        {
          errorCode = 7;  // a database is open
        }
        else
        {
          dbName = nl->SymbolValue( nl->Third( list ) );
          filename = nl->SymbolValue( nl->Fifth( list ) );
          message = SecondoSystem::GetInstance()->RestoreDatabase( dbName, filename, errorInfo );
          switch (message)
          {
            case 0:
              break;
            case 1:
              errorCode = 25;  // identifier not a known database name
              break;
            case 2:
              errorCode = 27;  // database name in file different from identifier
              break;
            case 3:
              errorCode = 28;  // problem in reading the file
              break;
            case 4:
              errorCode = 29;  // error in list structure in file
              break;
            case 5:
              errorCode = 24;  // error in type or object definitions in file
              resultList = errorList;
              break;
            default:
              errorCode = 1;   // Should never occur
              break;
          }
        }            
      }
      else
      {
        errorCode = 1;  // Command not recognized.
      }
    }

    // --- List commands

    else if ( nl->IsEqual( first, "list" ) )
    {
      if ( nl->IsEqual( nl->Second( list ), "type" ) && (length == 3) &&
           nl->IsEqual( nl->Third( list ), "constructors" ) )
      {
        resultList =
          SecondoSystem::GetCatalog( level )->ListTypeConstructors();
      }
      else if ( nl->IsEqual( nl->Second(list), "operators" ) )
      {
        resultList =
          SecondoSystem::GetCatalog( level )->ListOperators();
      }
      else if ( nl->IsEqual( nl->Second( list ), "databases" ) )
      {
        resultList = SecondoSystem::GetInstance()->ListDatabaseNames();
      }
      else if ( nl->IsEqual( nl->Second( list ), "types") )
      {
        if ( !SecondoSystem::GetInstance()->IsDatabaseOpen() )
        {
          errorCode = 6;  // no database open
        }
        else
        {
          StartCommand();
          resultList =
            SecondoSystem::GetCatalog( level )->ListTypes();
          FinishCommand( errorCode );
        }                    
      }
      else if ( nl->IsEqual( nl->Second( list ), "objects" ) )
      {
        if ( !SecondoSystem::GetInstance()->IsDatabaseOpen() )
        {
          errorCode = 6;  // no database open
        }
        else
        {
          StartCommand();
          resultList =
            SecondoSystem::GetCatalog( level )->ListObjects();
          FinishCommand( errorCode );
        }                    
      }
      else
      {
        errorCode = 1;  // Command not recognized.
      }
    }

    // --- Type definition

    else if ( nl->IsEqual( first, "type" ) && 
             (length == 4) && nl->IsAtom( nl->Second( list ) ) && 
             (nl->AtomType( nl->Second( list )) == SymbolType) && 
              nl->IsEqual( nl->Third( list ), "=" ) )
    {
      if ( SecondoSystem::GetInstance()->IsDatabaseOpen() )
      {
        StartCommand();
        typeName = nl->SymbolValue( nl->Second( list ) );
        typeExpr = nl->Fourth( list );
        typeExpr2 =
          SecondoSystem::GetCatalog( level )->ExpandedType( typeExpr );
        if ( SecondoSystem::GetCatalog( level )->
               KindCorrect( typeExpr2, errorInfo ) )
        { 
          if ( !SecondoSystem::GetCatalog( level )->
                 InsertType( typeName, typeExpr2 ) )
          {
            errorCode = 10;  // identifier already used
          }
        }
        else
        {
          errorCode = 5;     // Wrong type expression
          resultList = errorList;
        }
        FinishCommand( errorCode );
      }
      else
      {
        errorCode = 6;       // no database open
      } 
    }
    else if ( nl->IsEqual( first, "delete" ) )
    {
      if ( (length == 3) && nl->IsAtom( nl->Third( list ) ) && 
           (nl->AtomType( nl->Third( list )) == SymbolType) &&
            nl->IsEqual( nl->Second( list ), "type" ) )
      {
        if ( SecondoSystem::GetInstance()->IsDatabaseOpen() )
        {
          StartCommand();
          typeName = nl->SymbolValue( nl->Third( list ) ); 
          message =
            SecondoSystem::GetCatalog( level )->DeleteType( typeName );
          if ( message == 1 )
          {
            errorCode = 14;   // Type used by an object
          }
          else if ( message == 2 )
          {
            errorCode = 11;   // identifier not a known type name
          }
          FinishCommand( errorCode );
        }
        else
        {
          errorCode = 6;      // no database open
        } 
      }
      else if ( (length == 2) && nl->IsAtom( nl->Second( list )) && 
                (nl->AtomType( nl->Second( list )) == SymbolType) )
      {
        if ( SecondoSystem::GetInstance()->IsDatabaseOpen() )
        {
          StartCommand();
          objName = nl->SymbolValue( nl->Second( list ) ); 
          message =
            SecondoSystem::GetCatalog( level )->DeleteObject( objName );
          if ( message > 0 )
          {
            errorCode = 12;   // identifier not a known object name
          }
          FinishCommand( errorCode );
        }
        else
        {
          errorCode = 6;      // no database open
        } 
      }
      else
      {
        errorCode = 1;        // Command not recognized
      }
    }

    // --- Create object command

    else if ( nl->IsEqual( first, "create" ) && (length == 4) && 
              nl->IsAtom( nl->Second( list )) &&
             (nl->AtomType( nl->Second( list ) ) == SymbolType) && 
              nl->IsEqual( nl->Third( list ), ":" ) )
    {
      if ( SecondoSystem::GetInstance()->IsDatabaseOpen() )
      {
        StartCommand();
        objName = nl->SymbolValue( nl->Second( list ) );
        typeExpr = nl->Fourth( list );
        typeExpr2 =
          SecondoSystem::GetCatalog( level )->ExpandedType( typeExpr );
        typeName = "";
        if ( SecondoSystem::GetCatalog( level )->
               KindCorrect( typeExpr2, errorInfo ) )
        { 
          if ( nl->IsAtom( typeExpr ) &&
              (nl->AtomType( typeExpr ) == SymbolType) )
          {
            typeName = nl->SymbolValue( typeExpr );
            if ( !SecondoSystem::GetCatalog( level )->
                    MemberType( typeName ) )
            {
              typeName = "";
            }
          }
          if ( !SecondoSystem::GetCatalog( level )->
                  CreateObject( objName, typeName, typeExpr2, 0 ) )
          {
            errorCode = 10;  // identifier already used
          }
        }
        else
        {
          errorCode = 4;     // Wrong type expression
          resultList = errorList;
        }
        FinishCommand( errorCode );
      }
      else
      {
        errorCode = 6;       // no database open
      } 
    }

    // --- Update object command

    else if ( nl->IsEqual( first, "update" ) && (length == 4) && 
              nl->IsAtom( nl->Second( list )) &&
             (nl->AtomType( nl->Second( list ) ) == SymbolType) &&
              nl->IsEqual( nl->Third( list ), ":=" ) )
    {
      if ( SecondoSystem::GetInstance()->IsDatabaseOpen() )
      {
        if ( level == DescriptiveLevel )
        {
          errorCode = 32;  // Command not yet implemented at this level
        }
        else
        {
          StartCommand();
          objName = nl->SymbolValue( nl->Second( list ) );
          valueExpr = nl->Fourth( list );
          SecondoSystem::GetQueryProcessor()->
            Construct( level, valueExpr, correct, evaluable, defined,
                       isFunction, tree, resultType );
          if ( !defined )
          {
            errorCode = 8;      // Undefined object value in expression
          }
          else if ( correct )
          {
            if ( !SecondoSystem::GetCatalog( level )->
                    IsObjectName( objName ) )
            {
              errorCode = 12;   // identifier not a known object name
            }
            else
            {
              SecondoSystem::GetCatalog( level )->
                GetObjectExpr( objName, typeName, typeExpr,
                               word, defined, model, hasNamedType );
              if ( !nl->Equal( typeExpr, resultType ) )
              {
                errorCode = 13;   // types of object and expression do not agree
              }
              else if ( evaluable )
              {
                SecondoSystem::GetQueryProcessor()->
                  Eval( tree, result, 1 );
                SecondoSystem::GetCatalog( level )->
                  UpdateObject( objName, result );
                SecondoSystem::GetQueryProcessor()->
                  Destroy( tree, false );
              }
              else if ( isFunction )   // abstraction or function object
              {
                if ( nl->IsAtom( valueExpr ) )  // function object
                {
                  functionList = 
                    SecondoSystem::GetCatalog( level )->
                      GetObjectValue( nl->SymbolValue( valueExpr ) );
                  SecondoSystem::GetCatalog( level )->
                    UpdateObject( objName, SetWord( functionList ) );
                }
                else
                {
                  SecondoSystem::GetCatalog( level )->
                    UpdateObject( objName, SetWord( valueExpr ) );
                }
              }
              else
              {
                errorCode = 3;   // Expression not evaluable
              }
            }
          }
          else
          {
            errorCode = 2;    // Error in expression
          }
          FinishCommand( errorCode );
        }
      }
      else
      {
        errorCode = 6;        // no database open
      } 
    }


    // --- Let command

    else if ( nl->IsEqual( first, "let" ) && (length == 4) && 
              nl->IsAtom( nl->Second( list )) &&
             (nl->AtomType( nl->Second( list ) ) == SymbolType) &&
              nl->IsEqual( nl->Third( list ), "=" ) )
    {
      if ( SecondoSystem::GetInstance()->IsDatabaseOpen() )
      {
        if ( level == DescriptiveLevel )
        {
          errorCode = 32;  // Command not yet implemented at this level
        }
        else
        {
          StartCommand();
          objName = nl->SymbolValue( nl->Second( list ) );
          valueExpr = nl->Fourth( list );
          SecondoSystem::GetQueryProcessor()->
            Construct( level, valueExpr, correct, evaluable, defined,
                       isFunction, tree, resultType );
          if ( !defined )
          {
            errorCode = 8;      // Undefined object value in expression
          }
          else if ( correct )
          {

		if ( SecondoSystem::GetCatalog(level)->IsObjectName(objName) )
            {
              errorCode = 10;   // identifier is already used
            }
            else
            {
              if ( evaluable || isFunction )
		  {
		    typeName = "";
 		    bool b = SecondoSystem::GetCatalog(level)->
		        CreateObject(objName, typeName, resultType, 0);
		  }
		  if ( evaluable )
              {
                SecondoSystem::GetQueryProcessor()->
                  Eval( tree, result, 1 );
                SecondoSystem::GetCatalog( level )->
                  UpdateObject( objName, result );
                SecondoSystem::GetQueryProcessor()->
                  Destroy( tree, false );
              }
              else if ( isFunction )   // abstraction or function object
              {
                if ( nl->IsAtom( valueExpr ) )  // function object
                {
                  functionList = 
                    SecondoSystem::GetCatalog( level )->
                      GetObjectValue( nl->SymbolValue( valueExpr ) );
                  SecondoSystem::GetCatalog( level )->
                    UpdateObject( objName, SetWord( functionList ) );
                }
                else
                {
                  SecondoSystem::GetCatalog( level )->
                    UpdateObject( objName, SetWord( valueExpr ) );
                }
              }
              else
              {
                errorCode = 3;   // Expression not evaluable
              }
            }
          }
          else
          {
            errorCode = 2;    // Error in expression
          }
          FinishCommand( errorCode );
        }
      }
      else
      {
        errorCode = 6;        // no database open
      } 
    }


    // --- Query command

    else if ( nl->IsEqual( first, "query" ) && (length == 2) )
    {
      if ( SecondoSystem::GetInstance()->IsDatabaseOpen() )
      {
        if ( level == DescriptiveLevel )
        {
          errorCode = 32;  // Command not yet implemented at this level
        }
        else
        {
          StartCommand();

	  cout << "Analyze query ..." << endl;

          SecondoSystem::GetQueryProcessor()->
            Construct( level, nl->Second( list ), correct, evaluable, defined, 
                       isFunction, tree, resultType );
          if ( !defined )
          {
            errorCode = 8;         // Undefined object value
          }
          else if ( correct )
          {
            if ( evaluable )
            {
              cout << "Execute ..." << endl;

	      SecondoSystem::GetQueryProcessor()->
                Eval( tree, result, 1 );
              valueList = SecondoSystem::GetCatalog( level )->
                            OutObject( resultType, result );
              resultList = nl->TwoElemList( resultType, valueList );
              SecondoSystem::GetQueryProcessor()->
                Destroy( tree, true );
            }
            else if ( isFunction ) // abstraction or function object
            {
              if ( nl->IsAtom( nl->Second( list ) ) )  // function object
              {
                functionList = SecondoSystem::GetCatalog( level )->
                  GetObjectValue( nl->SymbolValue( nl->Second( list ) ) );
                resultList = nl->TwoElemList( resultType, functionList );
              }
              else
              {
                resultList = nl->TwoElemList( resultType, nl->Second( list ) );
              }
            }
            else
            {
              errorCode = 3;  // Query not evaluable
            }
          }
          else
          {
            errorCode = 2;    // Error in query
          }
          FinishCommand( errorCode );
        }
      }
      else
      {
        errorCode = 6;        // no database open
      } 
    }

    // --- Model command

    else if ( nl->IsEqual( first, "model" ) && (length == 2) )
    {
      if ( SecondoSystem::GetInstance()->IsDatabaseOpen() )
      {
        StartCommand();
        SecondoSystem::GetQueryProcessor()->
          Construct( level, nl->Second( list ), correct, evaluable, defined, 
                     isFunction, tree, resultType );
        if ( !defined )
        {
          errorCode = 8;         // Undefined object value
        }
        else if ( correct )
        {
          if ( evaluable )
          {
            SecondoSystem::GetQueryProcessor()->
              EvalModel( tree, result );
            modelList = SecondoSystem::GetCatalog( level )->
                          OutObjectModel( resultType, result );
            resultList = nl->TwoElemList( resultType, modelList ); 
            SecondoSystem::GetQueryProcessor()->
              Destroy( tree, true );
          }
          else
          {
            errorCode = 3;   // Query not evaluable
          }
        }
        else
        {
          errorCode = 2;     // Error in query
        }
        FinishCommand( errorCode );
      }
      else
      {
        errorCode = 6;       // no database open
      } 
    }
    else
    {
      errorCode = 1;         // Command not recognized
    }
  }
  if ( resultAsText )
  {
    nl->WriteToFile( resultFileName, resultList );
  }
  SecondoSystem::SetAlgebraLevel( UndefinedLevel );
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
    list = SecondoSystem::GetCatalog( level )->NumericType( type );
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
  if ( SecondoSystem::GetInstance()->IsDatabaseOpen() )
  {
    ok = SecondoSystem::GetCatalog( level )->
           LookUpTypeExpr( type, name, algebraId, typeId );
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
SecondoInterface::FinishCommand( int& errorCode )
{
  if ( !activeTransaction )
  {
    if ( errorCode == 0 )
    {
      if ( !SecondoSystem::CommitTransaction() )
      {
        errorCode = 23;
      }
    }
    else
    {
      if ( !SecondoSystem::AbortTransaction() )
      {
        errorCode = 23;
      }
    }
  }
}

void
SecondoInterface::SetDebugLevel( const int level )
{
  SecondoSystem::GetQueryProcessor()->SetDebugLevel( level );
}

