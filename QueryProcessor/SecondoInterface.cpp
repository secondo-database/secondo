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

\tableofcontents

*/

using namespace std;

#include "SecondoInterface.h"
#include "SecondoSystem.h"
#include "SecondoCatalog.h"
#include "QueryProcessor.h"

#include "SecondoSMI.h"
#include "SecParser.h"

/************************************************************************** 
3.1 The Secondo Procedure 

*/

SecondoInterface::SecondoInterface()
  : activeTransaction( false )
{
  nl = SecondoSystem::GetNestedList();
}

SecondoInterface::~SecondoInterface()
{
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
cout << "Interface Secondo Entry" << endl;
cout << "commandText  : " << commandText << endl;
cout << "commandLE    : " << commandLE << endl;
cout << "commandLevel : " << commandLevel << endl;
cout << "commandAsText: " << commandAsText << endl;
cout << "resultAsText : " << resultAsText << endl;
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
  bool correct, evaluable, defined, isFunction, hasNamedType;
  int message;                /* error code from called procedures */ 
  string listCommand;         /* buffer for command in list form */
  AlgebraLevel level;

  SecParser sp;
  NestedList* nl = SecondoSystem::GetNestedList();

  errorMessage = "";
  errorCode    = 0;
  errorList    = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );
  errorInfo    = errorList;

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
cout << "errorCode=" << errorCode << endl;
  if ( errorCode != 0 )
  {
    return;
  }
  length = nl->ListLength( list );
cout << "length=" << length << endl;
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
          if ( SmiEnvironment::BeginTransaction() )
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
          if ( !SmiEnvironment::CommitTransaction() )
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
          if ( !SmiEnvironment::AbortTransaction() )
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
          if ( !SecondoSystem::GetInstance()->CreateDatabase( dbName ) )
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
            SmiEnvironment::CommitTransaction();
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
          if ( !activeTransaction )
          {
            SmiEnvironment::BeginTransaction();
          }
          filename = nl->SymbolValue( nl->Fourth( list ) ); 
          if ( !SecondoSystem::GetInstance()->SaveDatabase( filename ) )
          {
            errorCode = 26;  // Problem in writing to file
          }
          if ( !activeTransaction )
          {
            if ( !SmiEnvironment::CommitTransaction() )
            {
              errorCode = 23;
            }
          }
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
cout << "List command" << endl;
      if ( nl->IsEqual( nl->Second( list ), "type" ) && (length == 3) &&
           nl->IsEqual( nl->Third( list ), "constructors" ) )
      {
cout << "list type constructors" << endl;
        resultList =
          SecondoSystem::GetCatalog( level )->ListTypeConstructors();
      }
      else if ( nl->IsEqual( nl->Second(list), "operators" ) )
      {
cout << "list operators" << endl;
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
          if ( !activeTransaction )
          {
            SmiEnvironment::BeginTransaction();
          }
          resultList =
            SecondoSystem::GetCatalog( level )->ListTypes();
          if ( !activeTransaction )
          {
            if ( !SmiEnvironment::CommitTransaction() )
            {
              errorCode = 23;
            }
          }
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
          if ( !activeTransaction )
          {
            SmiEnvironment::BeginTransaction();
          }
          resultList =
            SecondoSystem::GetCatalog( level )->ListObjects();
          if ( !activeTransaction )
          {
            if ( !SmiEnvironment::CommitTransaction() )
            {
              errorCode = 23;
            }
          }
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
        if ( !activeTransaction )
        {
          SmiEnvironment::BeginTransaction();
        }
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
        if ( !activeTransaction )
        {
          if ( !SmiEnvironment::CommitTransaction() )
          {
            errorCode = 23;
          }
        }
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
          if ( !activeTransaction )
          {
            SmiEnvironment::BeginTransaction();
          }
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
          if ( !activeTransaction )
          {
            if ( !SmiEnvironment::CommitTransaction() )
            {
              errorCode = 23;
            }
          }
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
          if ( !activeTransaction )
          {
            SmiEnvironment::BeginTransaction();
          }
          objName = nl->SymbolValue( nl->Second( list ) ); 
          message =
            SecondoSystem::GetCatalog( level )->DeleteObject( objName );
          if ( message > 0 )
          {
            errorCode = 12;   // identifier not a known object name
          }
          if ( !activeTransaction )
          {
            if ( !SmiEnvironment::CommitTransaction() )
            {
              errorCode = 23;
            }
          }
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
        if ( !activeTransaction )
        {
          SmiEnvironment::BeginTransaction();
        }
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
        if ( !activeTransaction )
        {
          if ( !SmiEnvironment::CommitTransaction() )
          {
            errorCode = 23;
          }
        }
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
          if ( !activeTransaction )
          {
            SmiEnvironment::BeginTransaction();
          }
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
          if ( !activeTransaction )
          {
            if ( !SmiEnvironment::CommitTransaction() )
            {
              errorCode = 23;
            }
          }
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
          if ( !activeTransaction )
          {
            SmiEnvironment::BeginTransaction();
          }
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
          if ( !activeTransaction )
          {
            if ( !SmiEnvironment::CommitTransaction() )
            {
              errorCode = 23;
            }
          }
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
        if ( !activeTransaction )
        {
          SmiEnvironment::BeginTransaction();
        }
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
        if ( !activeTransaction )
        {
          if ( !SmiEnvironment::CommitTransaction() )
          {
            errorCode = 23;
          }
        }
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
}

/*
1.3 Procedure ~NumericTypeExpr~

*/
ListExpr
SecondoInterface::NumericTypeExpr( const AlgebraLevel level, const ListExpr type )
{
  ListExpr list = nl->TheEmptyList();
  if ( SecondoSystem::GetInstance()->IsDatabaseOpen() )
  {
    list = SecondoSystem::GetCatalog( level )->NumericType( type );
  }
  return (list);
}

bool
SecondoInterface::GetTypeId( const AlgebraLevel level,
                             const string& name,
                             int& algebraId, int& typeId )
{
  return (SecondoSystem::GetCatalog( level )->
            GetTypeId( name, algebraId, typeId ));
}

bool
SecondoInterface::LookUpTypeExpr( const AlgebraLevel level,
                                  ListExpr type, string& name,
                                  int& algebraId, int& typeId )
{
  bool ok = false;
  name = "";
  algebraId = 0;
  typeId = 0;
  if ( SecondoSystem::GetInstance()->IsDatabaseOpen() )
  {
    ok = SecondoSystem::GetCatalog( level )->
           LookUpTypeExpr( type, name, algebraId, typeId );
  }
  return (ok);
}

