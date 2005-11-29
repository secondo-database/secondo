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

//paragraph    [10]    title:           [{\Large \bf ] [}]
//paragraph    [21]    table1column:    [\begin{quote}\begin{tabular}{l}]     [\end{tabular}\end{quote}]
//paragraph    [22]    table2columns:   [\begin{quote}\begin{tabular}{ll}]    [\end{tabular}\end{quote}]
//paragraph    [23]    table3columns:   [\begin{quote}\begin{tabular}{lll}]   [\end{tabular}\end{quote}]
//paragraph    [24]    table4columns:   [\begin{quote}\begin{tabular}{llll}]  [\end{tabular}\end{quote}]
//[--------]    [\hline]
//characters    [1]    verbatim:   [$]    [$]
//characters    [2]    formula:    [$]    [$]
//characters    [3]    capital:    [\textsc{]    [}]
//characters    [4]    teletype:   [\texttt{]    [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]
//[Contents] [\tableofcontents]

1 The Implementation-Module *SecondoSystem*

May 2002 Ulrich Telle Port to C++

August 2002 Ulrich Telle Added methods to set and get the current algebra level.

April 29 2003 Hoffmann Added methods for saving and restoring single objects.

April 2004 Hoffmann Changed some implementation details, so that the list databases 
command is available under Windows XP.

This module implements those parts of the "Secondo"[3] catalog which
are independent of the algebra level (descriptive or executable).

It consists of six parts: First it contains an interface to
 *Databases and Transactions* for loading the actual catalog of database
 types and objects and for managing transactions. Therefore it offers
 database functions to open, close, save, restore, create and destroy a
 named database as described in the SECONDO Programming Interface. Second
 it offers an interface for inquiries to give the database user information
 about the type system and the used algebras. Third it delivers functions
 for retrieving and manipulating database types. Fourth the module offers
 functions for retrieving, updating, deleting and inserting database objects.
 Fifth and Sixth it delivers functions to look up the type constructors and
 operators of the actually loaded algebras. These functions are implemented
 by applying the functions of the lower modules *CTable*, *NameIndex*,
 *AlgebraManager2*, *ObjectListManager* and *ObjectManager*.
The names of existing databases are stored in a list ~DBTable~.

\tableofcontents

1.2 Implementation

*/

using namespace std;

#include "ErrorCodes.h"
#include "SecondoSystem.h"
#include "QueryProcessor.h"

SecondoSystem* SecondoSystem::secondoSystem = 0;

/**************************************************************************
3 Functions and Procedures

3.1 Interface to DatabasesAndTransactions

*/

ListExpr
SecondoSystem::ListDatabaseNames ()
{
/*
Creates a scan on the table of database names and returns the names of existing databases in a list of the following format:

---- (<database name 1>..<database name n>)
----

*/
  ListExpr list, lastElem = 0;
  string dbName;
  char dbNametmp[1024];
  list = nl->TheEmptyList();
  unsigned int i=0, j=0;
  
  if (SmiEnvironment::ListDatabases( dbName ))
  {
    while ( i < dbName.length() )
    {
      if ( dbName[i] != '#' )
      {
        dbNametmp[j] = dbName[i];
        j++;
      }
      else
      {
        dbNametmp[j] = '\0';
        if ( list == nl->TheEmptyList() )
        {
          list = nl->OneElemList( nl->SymbolAtom( dbNametmp ) );
          lastElem = list;
        }
        else
        {
          lastElem = nl->Append( lastElem, nl->SymbolAtom( dbNametmp ) );
        }
        j = 0;
      }
      i++;
    }
  }
  return (list);
}

bool
SecondoSystem::CreateDatabase ( const string& dbname )
{
/*
Creates a new database named ~dbname~ and a logfile used by transaction
management  and loads the algebraic operators and type constructors into
the SECONDO programming interface. Returns error 1 if a database under
this name exists already.

Precondition: DBState = dbClosed.

*/
  bool ok = false;
  if ( testMode )
  {
    if ( SmiEnvironment::IsDatabaseOpen() )
    {
      cerr << " CreateDatabase: database is already open!" << endl;
      exit( 0 );
    }
  }
  if ( SmiEnvironment::CreateDatabase( dbname ) )
  {
    SmiEnvironment::BeginTransaction();
    if ( scDescriptive->Open() )
    {
      if ( scExecutable->Open() )
      {
        ok = true;
        SmiEnvironment::CommitTransaction();
      }
      else
      {
        scDescriptive->Close();
        SmiEnvironment::AbortTransaction();
        SmiEnvironment::CloseDatabase();
        SmiEnvironment::EraseDatabase( dbname );
      }
    } 
    else 
    {
      SmiEnvironment::AbortTransaction();
      // To do: Correct error handling
      cerr << "scDescriptive->Open() failed!" << endl;
    }
  }
  return (ok);
}

bool
SecondoSystem::DestroyDatabase ( const string& dbname )
{
/*
Deletes a database named ~dbname~ and the logfile that was created by
~CreateDatabase~. Returns error 1 if the ~dbname~ is not known.

Precondition: dbState = dbClosed.

*/
  bool ok = false;
  if ( SmiEnvironment::IsDatabaseOpen() )
  {
    cerr << " DestroyDatabase: database is already open!" << endl;
  }
  else
  {
    ok = SmiEnvironment::EraseDatabase( dbname );
  }
  return (ok);
}

bool
SecondoSystem::OpenDatabase( const string& dbname )
{
/*
Opens a database with name ~dbname~. Returns error 1 if ~dbname~ is unknown.

Precondition: dbState = dbClosed.

*/
  bool ok = false;
  if ( testMode && SmiEnvironment::IsDatabaseOpen() )
  {
    cerr << " OpenDatabase: database is already open!" << endl;
    exit( 0 );
  }
  if ( SmiEnvironment::OpenDatabase( dbname ) )
  {
    SmiEnvironment::BeginTransaction();
    if ( scDescriptive->Open() )
    {
      if ( scExecutable->Open() )
      {
        ok = true;
        ok = ok && flobFile->Open( "FLOB" );
        SmiEnvironment::CommitTransaction();
      }
      else
      {
        scDescriptive->Close();
        SmiEnvironment::AbortTransaction();
        SmiEnvironment::CloseDatabase();
      }
    }
    else 
    {
      SmiEnvironment::AbortTransaction();
      // To do: Correct error handling
      cerr << "scDescriptive->Open() failed!" << endl;
    }
  }
  return (ok);
}

bool
SecondoSystem::CloseDatabase()
{
/*
Closes the actually opened database. All open segments are closed and the allocated memory for internal data structures is returned.

Precondition: dbState = dbOpen.

*/
  if ( testMode && !SmiEnvironment::IsDatabaseOpen() )
  {
    cerr << " CloseDatabase: database is already closed!" << endl;
    assert( false );
  }
  scExecutable->Close();
  scDescriptive->Close();
  flobFile->Close();
  return (SmiEnvironment::CloseDatabase());
}

bool
SecondoSystem::IsDatabaseOpen()
{
/*
Returns the state of the database ~state~.

*/
  return (SmiEnvironment::IsDatabaseOpen());
}

bool
SecondoSystem::IsDatabaseObject( const string& objectName )
{
/*
Returns whether object with ~objectName~ is known in the currently opened
database.

*/
  if ( SecondoSystem::GetAlgebraLevel() == ExecutableLevel )
    return ( scExecutable->IsObjectName( objectName) );
  else if ( SecondoSystem::GetAlgebraLevel() == ExecutableLevel )
    return ( scDescriptive->IsObjectName( objectName) );
  return ( false );
}

bool
SecondoSystem::SaveObject ( const string& objectName,
                            const string& filename )
{
/*
Writes a secondo object called ~objectName~ of the currently opened database
to a file with name ~filename~ in nested list format. The format is the
following:

---- (OBJECT <object name> (<type name>) <type expression> <value> <model>)*

----

Returns false if there was a problem in writing the file.

Precondition: dbState = dbOpen.

*/
  ListExpr objectList, typeExpr, valueList, modelList;
  objectList = typeExpr = valueList = modelList = nl->TheEmptyList();

  Word value = SetWord(0);
  Word model = SetWord(0);

  bool defined = false;
  bool hasTypeName = false;

  string typeName = "";
  string typeExprString="";

  if ( testMode && !SmiEnvironment::IsDatabaseOpen() )
  {
    cerr << " SaveObject: database is not open!" << endl;
    exit( 0 );
  }

  if ( SecondoSystem::GetAlgebraLevel() == ExecutableLevel )
  {
    scExecutable->GetObjectExpr( objectName, typeName, typeExpr,
                       value, defined, model, hasTypeName );
    if ( defined )
    {
      valueList = scExecutable->OutObject( typeExpr, value );
      scExecutable->CloseObject( typeExpr, value );
    }
    else
    {
      valueList = nl->TheEmptyList();
    }

    if ( model.addr != 0 )
    {
      modelList = scExecutable->OutObjectModel( typeExpr, model );
    }
    else
    {
      modelList = nl->TheEmptyList();
    }
  }
  else
  {
    if ( SecondoSystem::GetAlgebraLevel() == DescriptiveLevel )
    {
      scDescriptive->GetObjectExpr( objectName, typeName, typeExpr,
                       value, defined, model, hasTypeName );

      if ( defined )
      {
        valueList = scDescriptive->OutObject( typeExpr, value );
	scExecutable->CloseObject( typeExpr, value );
      }
      else
      {
        valueList = nl->TheEmptyList();
      }

      if ( model.addr != 0 )
      {
        modelList = scDescriptive->OutObjectModel( typeExpr, model );
      }
      else
      {
        modelList = nl->TheEmptyList();
      }
    }
  }

  objectList = nl->SixElemList(
                   nl->SymbolAtom( "OBJECT" ),
                   nl->SymbolAtom( objectName ),
                   nl->OneElemList( nl->SymbolAtom( typeName ) ),
                   typeExpr,
                   valueList,
                   modelList );

  return (nl->WriteToFile( filename, objectList ));
}

bool
SecondoSystem::SaveDatabase ( const string& filename, const DerivedObj& derivedObjs )
{
/*
Writes the currently open database called ~dbname~ to a file with name
~filename~ in nested list format. The format is the following:

---- (DATABASE <database name>
       (TYPES
         (TYPE <type name> <type expression>)*
       )
       (OBJECTS
         (OBJECT <object name> (<type name>) <type expression> <value> <model>)*
       )
     )
----

Returns error 1 if there was a problem in writing the file.

Precondition: dbState = dbOpen.

*/
  if ( testMode && !SmiEnvironment::IsDatabaseOpen() )
  {
    cerr << " SaveDatabase: database is not open!" << endl;
    exit( 0 );
  }

  ListExpr typeExprDesc = scDescriptive->ListTypes();
  ListExpr typeExprExec = scExecutable->ListTypes();

  /* get the objects with value component !!*/
  ListExpr objExprDesc = scDescriptive->ListObjectsFull(derivedObjs);
  ListExpr objExprExec = scExecutable->ListObjectsFull(derivedObjs);

  ListExpr list;
  list = nl->SixElemList(
           nl->TwoElemList(
             nl->SymbolAtom( "DESCRIPTIVE" ),
             nl->SymbolAtom( "ALGEBRA" ) ),
           typeExprDesc,
           objExprDesc,
           nl->TwoElemList(
             nl->SymbolAtom( "EXECUTABLE" ),
             nl->SymbolAtom( "ALGEBRA" ) ),
           typeExprExec,
           objExprExec );
  list = nl->Cons( nl->SymbolAtom("DATABASE"),
	           nl->Cons( nl->SymbolAtom( GetDatabaseName() ), list ) );
  return (nl->WriteToFile( filename, list ));
}

int
SecondoSystem::RestoreObjectFromFile( const string& objectname,
                                      const string& filename,
                                      ListExpr& errorInfo )
{
/*
Reads an object from a file named ~filename~ and fills the catalog
with this object. The database remains in state ~dbOpen~.
Returns

  * error 1 object name in file is different from parameter ~objectname~,

  * error 2, if there was a problem in reading the file,

  * error 3, if the list structure in the file was not correct,

  * error 4, if there is an error in object list expression,

  Furthermore, any errors found by kind checking and by ~In~ procedures are added to the list ~errorInfo~.

Precondition: dbState = dbOpen.

*/
  ListExpr list;
  int rc = 0;

  if ( testMode && !SmiEnvironment::IsDatabaseOpen() )
  {
    cerr << " RestoreObjectFromFile: database is not open!" << endl;
    exit( 0 );
  }

  else if ( !nl->ReadFromFile( filename, list ) )
  {
    rc = 2; // Error reading file
  }

    else if ( !nl->IsEqual( nl->Second( list ), objectname, false ) )
    {
      rc = 1; // Object name in file different
    }

      else if ( nl->IsEmpty( list) )
      {
        rc = 3; // List structure invalid
      }

        else if ( SecondoSystem::GetAlgebraLevel() == ExecutableLevel )
        {
          if ( RestoreObjects( scExecutable, nl->TwoElemList( nl-> SymbolAtom
            ("OBJECTS"), list ), errorInfo ) )
	  {
            rc = 0; // object successfully restored
	  }
          else
	  {
	    rc = 4; // Error in reading object
	  }
        }
          else if ( SecondoSystem::GetAlgebraLevel() == DescriptiveLevel )
          {
            if ( RestoreObjects( scDescriptive, nl->TwoElemList
	      ( nl-> SymbolAtom("OBJECTS"), list ), errorInfo ) )
	    {
              rc = 0; // object successfully restored
	    }
            else
	    {
	      rc = 4; // Error in reading object
            }
	  }
  return ( rc );
}

SI_Error
SecondoSystem::RestoreDatabase( const string& dbname,
                                const string& filename,
                                ListExpr& errorInfo )
{
/*
Reads a database from a file named ~filename~ that has the same nested
list format as in the procedure ~SaveDatabase~ and fills the catalogs
for database types and objects. The database state changes to ~dbOpen~.
Returns an error if

  * ~dbname~ is not a known database name,

  * the database name in the file is different from ~dbname~ here,

  * there was a problem in reading the file,

  * the list structure in the file was not correct,

  * there are errors in type definitions and/or object list expressions.

    Furthermore, any errors found by kind checking and by ~In~ procedures are added to the list ~errorInfo~.

Precondition: dbState = dbClosed.

*/
  ListExpr list,        listFile;
  ListExpr typesDesc,   typesExec;
  ListExpr objectsDesc, objectsExec;
  SI_Error rc = ERR_NO_ERROR;

  if ( testMode && SmiEnvironment::IsDatabaseOpen() )
  {
    cerr << " RestoreDatabase: database is not closed!" << endl;
    assert( false );
  }

  if ( !OpenDatabase( dbname ) )
  {
    rc = ERR_IDENT_UNKNOWN_DB_NAME; // Database unknown
  }
  else if ( !nl->ReadFromFile( filename, list ) )
  {
    rc = ERR_PROBLEM_IN_READING_FILE; // Error reading file
  }
  else
  {
    listFile = list;
/*
Tests the syntax of the database file named ~filename~.

*/
    if ( nl->ExprLength( list ) >= 8 )
    {
      if ( !nl->IsEqual( nl->Second( list ), dbname, false ) )
      {
        rc = ERR_DB_NAME_NEQ_IDENT; // Database name in file different
      }
      else if ( nl->IsEqual( nl->First( list ), "DATABASE" ) )
      {
        list = nl->Rest( nl->Rest( list ) ); // reduce to remaining 6 elements

        typesDesc   = nl->Second( list );
        objectsDesc = nl->Third( list );
        typesExec   = nl->Fifth( list );
        objectsExec = nl->Sixth( list );

        // the first and fourth element are the headers
        // (DESCRIPTIVE ALGEBRA) and (EXECUTABLE ALGEBRA)

        if ( !nl->IsEqual( nl->First( nl->First( list ) ), "DESCRIPTIVE" ) ||
             !nl->IsEqual( nl->Second( nl->First( list ) ), "ALGEBRA" ) ||
             !nl->IsEqual( nl->First( nl->Fourth( list ) ), "EXECUTABLE" ) ||
             !nl->IsEqual( nl->Second( nl->Fourth( list ) ), "ALGEBRA" ) )
        {
          rc = ERR_IN_LIST_STRUCTURE_IN_FILE; // List structure invalid
        }
        else if ( nl->IsEmpty( typesDesc ) ||
                  nl->IsEmpty( typesExec ) ||
                  nl->IsEmpty( objectsDesc ) ||
                  nl->IsEmpty( objectsExec ) )
        {
          rc = ERR_IN_LIST_STRUCTURE_IN_FILE; // List structure invalid
        }
        else if ( nl->IsEqual( nl->First( typesDesc ), "TYPES" ) &&
                  nl->IsEqual( nl->First( typesExec ), "TYPES" ) &&
                  nl->IsEqual( nl->First( objectsDesc ), "OBJECTS" ) &&
                  nl->IsEqual( nl->First( objectsExec ), "OBJECTS" ) )
        {
/*
Before restoring the database we need to get a completely empty database.
This is done by closing, destroying and recreating the database.

*/
          CloseDatabase();
          DestroyDatabase ( dbname );
          CreateDatabase( dbname );

/*
Load database types and objects from file named ~filename~.

*/
//          SecondoSystem::BeginTransaction();
          if ( RestoreCatalog( scDescriptive, typesDesc, objectsDesc, errorInfo ) &&
               RestoreCatalog( scExecutable, typesExec, objectsExec, errorInfo ) )
          {
            rc = ERR_NO_ERROR; // Database successfully restored

            /*
            if ( !SecondoSystem::CommitTransaction() )
            {
              rc = 23;
            }
						*/

          }
          else
          {
            rc = ERR_IN_DEFINITIONS_FILE; // Error in types or objects
            
						/*
            if ( !SecondoSystem::AbortTransaction() )
            {
              rc = 23;
            }
            */
          }
        }
        else
        {
          rc = ERR_IN_LIST_STRUCTURE_IN_FILE; // List structure invalid (Types or objects missing)
        }
      }
      else
      {
        rc = ERR_IN_LIST_STRUCTURE_IN_FILE; // List structure invalid (Database info missing)
      }
    }
    else
    {
      rc = ERR_IN_LIST_STRUCTURE_IN_FILE; // List structure invalid (List too short)
    }
    nl->Destroy( listFile );
    //if ( rc != 0 )
    //{
    //  CloseDatabase();
    //}
  }
  return (rc);
}

/*
3.3 Local Functions and Procedures

*/

bool
SecondoSystem::RestoreCatalog( SecondoCatalog* sc,
                               ListExpr types,
                               ListExpr objects,
                               ListExpr& errorInfo )
{
  bool ok =  RestoreTypes( sc, types, errorInfo );
  ok = ok && RestoreObjects( sc, objects, errorInfo );
  return (ok);
}

bool
SecondoSystem::RestoreTypes( SecondoCatalog* sc,
                             ListExpr types,
                             ListExpr& errorInfo )
{
  ListExpr first, typeExpr;
  int typeno;
  string typeName;
  bool correct = true;

  types = nl->Rest( types );
  typeno = 0;
  while (!nl->IsEmpty( types ))
  {
    SecondoSystem::BeginTransaction();
    first = nl->First( types );
    types = nl->Rest( types );
    typeno++;
    if ( (nl->ExprLength( first ) == 3) &&
          nl->IsEqual( nl->First( first ), "TYPE" ) &&
          nl->IsAtom( nl->Second( first ) ) &&
         (nl->AtomType( nl->Second( first ) ) == SymbolType ) )
    {
      typeName = nl->SymbolValue( nl->Second( first ) );
      typeExpr = nl->Third( first );

      if ( sc->KindCorrect( typeExpr, errorInfo ) )
      {
        if ( !sc->InsertType( typeName, typeExpr ) )
        {
          // typename doubly defined
          correct = false;
          errorInfo = nl->Append( errorInfo,
                        nl->ThreeElemList(
                          nl->IntAtom( 41 ),
                          nl->IntAtom( typeno ),
                          nl->Second( first ) ) );
        }
      }
      else
      {
        // error in type expression
        correct = false;
        errorInfo = nl->Append( errorInfo,
                      nl->ThreeElemList(
                        nl->IntAtom( 42 ),
                        nl->IntAtom( typeno ),
                        nl->Second( first ) ) );
      }
    }
    else
    {
      // error in type definition
      correct = false;
      errorInfo = nl->Append( errorInfo,
                    nl->TwoElemList(
                      nl->IntAtom( 40 ),
                      nl->IntAtom( typeno ) ) );
    } // if
    SecondoSystem::CommitTransaction();
  } // while

  return (correct);
}

bool
SecondoSystem::RestoreObjects( SecondoCatalog* sc,
                               ListExpr objects,
                               ListExpr& errorInfo )
{
  ListExpr first, typeExpr, valueList, modelList = 0;
  int objno;
  string objectName, typeName;
  Word value, model;
  bool correctObj;
  bool correct = true;

  objects = nl->Rest( objects );
  objno = 0;

  cout << "Restoring objects of the " << sc->catalogName << " catalog ..." << endl;

  while ( !nl->IsEmpty( objects) )
  {
    SecondoSystem::BeginTransaction();
    first = nl->First( objects );
    objects = nl->Rest( objects );
    objno++;
    if ( (nl->ExprLength( first) >= 5) &&
          nl->IsEqual( nl->First( first ), "OBJECT" ) &&
          nl->IsAtom( nl->Second( first ) ) &&
         (nl->AtomType( nl->Second( first ) ) == SymbolType ) &&
         !nl->IsAtom( nl->Third( first ) ) )
    {
      objectName = nl->SymbolValue( nl->Second( first ) );
      cout << "  " << objectName << " ... ";
      if ( !nl->IsEmpty( nl->Third( first ) ) &&
            nl->IsAtom( nl->First( nl->Third( first ) ) ) &&
           (nl->AtomType( nl->First( nl->Third( first ) ) ) == SymbolType ) )
      {
        typeName = nl->SymbolValue( nl->First( nl->Third( first ) ) );
      }
      else
      {
        typeName = "";
      }
      typeExpr = nl->Fourth( first );
      valueList = nl->Fifth( first );

      // model does not have to exist
      if ( nl->ExprLength(first) == 6 )
      {
        modelList = nl->Sixth( first );
      }
      else
      {
        modelList = nl->TheEmptyList();
      }
      if ( sc->KindCorrect( typeExpr, errorInfo ) )
      {
        value = sc->InObject( typeExpr, valueList, objno, errorInfo, correctObj );
        model = sc->InObjectModel( typeExpr, modelList, objno );
        if ( correctObj )
        {
          if ( !sc->InsertObject( objectName, typeName, typeExpr,
                                  value, true, model ) )
          {
            // doubly defined object
            errorInfo = nl->Append( errorInfo,
                          nl->ThreeElemList(
                            nl->IntAtom( 51 ),
                            nl->IntAtom( objno ),
                            nl->Second( first ) ) );
          }
        }
        else
        {
          // wrong list representation
          correct = false;
          errorInfo = nl->Append( errorInfo,
                        nl->ThreeElemList(
                          nl->IntAtom( 53 ),
                          nl->IntAtom( objno ),
                          nl->Second( first ) ) );
        }
      }
      else
      {
        // wrong type expression
        correct = false;
        errorInfo = nl->Append( errorInfo,
                      nl->ThreeElemList(
                        nl->IntAtom( 52 ),
                        nl->IntAtom( objno ),
                        nl->Second( first ) ) );
      }
    }
    else
    {
      // error in object definition
      correct = false;
      errorInfo = nl->Append( errorInfo,
                    nl->TwoElemList(
                      nl->IntAtom( 50 ),
                      nl->IntAtom( objno ) ) );
    } // if
    
    SecondoSystem::CommitTransaction();
    cout << "processed." << endl;
     
  } // while

  return (correct);
}

string
SecondoSystem::GetDatabaseName()
{
  return (SmiEnvironment::CurrentDatabase());
}

/*
3.4 Initialization of Values and Test Procedures

*/

SecondoSystem::SecondoSystem( GetAlgebraEntryFunction getAlgebraEntryFunc )
{
  nl = new NestedList();
  al = new NestedList();

  //cout << "nl:" << (void*)nl << endl;
  //cout << "al:" << (void*)al << endl;

  algebraManager = new AlgebraManager( *nl, getAlgebraEntryFunc );
  queryProcessor = new QueryProcessor( nl, algebraManager );
  flobFile = new SmiRecordFile( false );
  scDescriptive  = 0;
  scExecutable   = 0;
  currentLevel   = UndefinedLevel;
  initialized    = false;
  testMode       = false; // Todo: Should be configurable in SecondoConfig.ini
  secondoSystem  = this;
}

SecondoSystem::~SecondoSystem()
{
  if ( initialized )
  {
    ShutDown();
  }
  if ( scDescriptive )
  {
    delete scDescriptive;
  }
  if ( scExecutable )
  {
    delete scExecutable;
  }
  delete queryProcessor;
  delete algebraManager;
  delete nl;
  delete al;
  delete flobFile;
  secondoSystem = 0;
}

SecondoSystem*
SecondoSystem::GetInstance()
{
  return (secondoSystem);
}

bool
SecondoSystem::StartUp()
{
  if ( !secondoSystem->initialized )
  {
    secondoSystem->algebraManager->LoadAlgebras();
    secondoSystem->scDescriptive =
      new SecondoCatalog( "Descriptive", DescriptiveLevel );
    secondoSystem->scExecutable  =
      new SecondoCatalog( "Executable",  ExecutableLevel );
    secondoSystem->initialized = true;
  }
  return (secondoSystem->initialized);
}

bool
SecondoSystem::ShutDown()
{
  if ( secondoSystem->initialized )
  {
    secondoSystem->algebraManager->UnloadAlgebras();
    delete secondoSystem->scDescriptive;
    secondoSystem->scDescriptive = 0;
    delete secondoSystem->scExecutable;
    secondoSystem->scExecutable  = 0;
    secondoSystem->initialized   = false;
  }
  return (!secondoSystem->initialized);
}

AlgebraManager*
SecondoSystem::GetAlgebraManager()
{
  return (secondoSystem->algebraManager);
}

QueryProcessor*
SecondoSystem::GetQueryProcessor()
{
  return (secondoSystem->queryProcessor);
}

void
SecondoSystem::SetAlgebraLevel( const AlgebraLevel level )
{
  secondoSystem->currentLevel = level;
}

AlgebraLevel
SecondoSystem::GetAlgebraLevel()
{
  return (secondoSystem->currentLevel);
}

SecondoCatalog*
SecondoSystem::GetCatalog( const AlgebraLevel level )
{
  if ( level == DescriptiveLevel )
  {
    return (secondoSystem->scDescriptive);
  }
  else
  {
    return (secondoSystem->scExecutable);
  }
}

NestedList*
SecondoSystem::GetNestedList()
{
  return (secondoSystem->nl);
}

NestedList*
SecondoSystem::GetAppNestedList()
{
  return (secondoSystem->al);
}

bool
SecondoSystem::BeginTransaction()
{
  return (SmiEnvironment::BeginTransaction());
}

bool
SecondoSystem::CommitTransaction()
{
  secondoSystem->scDescriptive->CleanUp( false );
  secondoSystem->scExecutable->CleanUp( false );
  return (SmiEnvironment::CommitTransaction());
}

bool
SecondoSystem::AbortTransaction()
{
  secondoSystem->scDescriptive->CleanUp( true );
  secondoSystem->scExecutable->CleanUp( true );
  return (SmiEnvironment::AbortTransaction());
}

SmiRecordFile *SecondoSystem::GetFlobFile()
{
  return (secondoSystem->flobFile);
}
