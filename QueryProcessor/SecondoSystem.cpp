/*

1 The Implementation-Module *SecondoCatalog*

September 1996 Claudia Freundorfer

November 18, 1996 RHG Replaced all calls to ~CatalogManager~ by calls to ~CTable~.

December 20, 1996 Changed procedures ~OutObject~ and ~GetObjectValue~ and introduced ~NumericType~.

December 30, 1996 RHG Added procedures ~ExpandedType~ and ~KindCorrect~.

January 7-9, 1997 RHG Major revision for error handling.

January 13, 1997 RHG Corrected error in procedure ~OpenDatabase~.

December 23, 1997 RHG Corrected procedure ~LookUpTypeExpr~ to make it safe against wrong type expressions.

May 15, 1998 RHG Added treatment of models, especially functions ~InObjectModel~, ~OutObjectModel~, and ~ValueToObjectModel~.

October 13, 1998 Stefan Dieker ~NumericTypeExpr~ may now be called in database state ~dbClosed~, too.

September 9, 1998 Stefan Dieker Reimplemented functions ~CloseDatabase~ and
~OpenDatabase~ in such a way that all CTables and NameIndexes used for
storing object and type information are saved to files and loaded from those
files, respectively. Now the catalog is
semi-persistent, i.e. as persistent as provided by the underlying OS, without
access being save under transactions, logged, and locked.

This module implements the module *SecondoCatalog*. It consists of six parts: First it contains an interface to *Databases and Transactions* for loading the actual catalog of database types and objects and for managing transactions. Therefore it offers database functions to open, close, save, restore, create and destroy a named database as described in the SECONDO Programming Interface. Second it offers an interface for inquiries to give the database user information about the type system and the used algebras. Third it delivers functions for retrieving and manipulating database types. Fourth the module offers functions for retrieving, updating, deleting and inserting database objects. Fifth and Sixth it delivers functions to look up the type constructors and operators of the actually loaded algebras. These functions are implemented by applying the functions of the lower modules *CTable*, *NameIndex*, *AlgebraManager2*, *ObjectListManager* and *ObjectManager*. 
The names of existing databases are stored in a list ~DBTable~. 

\tableofcontents 

1.2 Implementation

*/

using namespace std;

#include "SecondoSystem.h"
#include "QueryProcessor.h"

SecondoSystem SecondoSystem::secondoSystem;

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
  list = nl->TheEmptyList();
  while (SmiEnvironment::ListDatabases( dbName ))
  {
    if ( list == nl->TheEmptyList() )
    {
      list = nl->OneElemList( nl->SymbolAtom( dbName ) ); 
      lastElem = list;
    }
    else
    {
      lastElem = nl->Append( lastElem, nl->SymbolAtom( dbName ) );
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
    cerr << " CreateDatabase: database is already open!" << endl;
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
        SmiEnvironment::CommitTransaction();
      }
      else
      {
        scDescriptive->Close();
        SmiEnvironment::AbortTransaction();
        SmiEnvironment::CloseDatabase();
      }
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
    exit( 0 );
  }
  scExecutable->Close();
  scDescriptive->Close();
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
SecondoSystem::SaveDatabase ( const string& filename )
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
  ListExpr objExprDesc = scDescriptive->ListObjectsFull();
  ListExpr objExprExec = scExecutable->ListObjectsFull();

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
SecondoSystem::RestoreDatabase( const string& dbname,
                                const string& filename,
                                ListExpr& errorInfo )
{
/*
Reads a database from a file named ~filename~ that has the same nested
list format as in the procedure ~SaveDatabase~ and fills the catalogs
for database types and objects. The database state changes to ~dbOpen~.
Returns 

  * error 1 if ~dbname~ is not a known database name, 

  * error 2, if the database name in the file is different from ~dbname~ here, 

  * error 3, if there was a problem in reading the file, 

  * error 4, if the list structure in the file was not correct,

  * error 5 if there are errors in type definitions and/or object list expressions.

    Furthermore, any errors found by kind checking and by ~In~ procedures are added to the list ~errorInfo~.

Precondition: dbState = dbClosed.

*/
  ListExpr list,        listFile;
  ListExpr typesDesc,   typesExec;
  ListExpr objectsDesc, objectsExec;
  int rc = 0;

  if ( testMode && SmiEnvironment::IsDatabaseOpen() )
  {
    cerr << " RestoreDatabase: database is not closed!" << endl;
    exit( 0 );
  }

  if ( !OpenDatabase( dbname ) )
  {
    rc = 1; // Database unknown
  }
  else if ( !nl->ReadFromFile( filename, list ) )
  {
    rc = 3; // Error reading file
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
        rc = 2; // Database name in file different
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
          rc = 4; // List structure invalid
        }
        else if ( nl->IsEmpty( typesDesc ) ||
                  nl->IsEmpty( typesExec ) ||
                  nl->IsEmpty( objectsDesc ) ||
                  nl->IsEmpty( objectsExec ) )
        {
          rc = 4; // List structure invalid
        }
        else if ( nl->IsEqual( nl->First( typesDesc ), "TYPES" ) &&
                  nl->IsEqual( nl->First( typesExec ), "TYPES" ) &&
                  nl->IsEqual( nl->First( objectsDesc ), "OBJECTS" ) &&
                  nl->IsEqual( nl->First( objectsExec ), "OBJECTS" ) )
        {
/*
Load database types and objects from file named ~filename~.

*/
          if ( RestoreCatalog( scDescriptive, typesDesc, objectsDesc, errorInfo ) &&
               RestoreCatalog( scExecutable, typesExec, objectsExec, errorInfo ) )
          {
            rc = 0; // Database successfully restored
          }
          else
          {
            rc = 5; // Error in types or objects
          }
        }
        else
        {
          rc = 4; // List structure invalid (Types or objects missing)
        }
      }
      else
      {
        rc = 4; // List structure invalid (Database info missing)
      }
    }
    else
    {
      rc = 4; // List structure invalid (List too short)
    }
    nl->Destroy( listFile );
    if ( rc != 0 )
    {
      CloseDatabase();
    }
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
    
  while ( !nl->IsEmpty( objects) )
  {
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

SecondoSystem::SecondoSystem()
{
  nl = new NestedList();
  algebraManager = new AlgebraManager( *nl );
  queryProcessor = new QueryProcessor( nl, algebraManager );
  scDescriptive  = 0;
  scExecutable   = 0;
  initialized    = false;
  testMode       = true; // At least during the programming and test phase
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
}

SecondoSystem*
SecondoSystem::GetInstance()
{
  return (&secondoSystem);
}

bool
SecondoSystem::StartUp()
{
  if ( !secondoSystem.initialized )
  {
    secondoSystem.algebraManager->LoadAlgebras();
    secondoSystem.scDescriptive =
      new SecondoCatalog( "Descriptive", DescriptiveLevel );
    secondoSystem.scExecutable  =
      new SecondoCatalog( "Executable",  ExecutableLevel );
    secondoSystem.initialized = true;
  }
  return (secondoSystem.initialized);
}

bool
SecondoSystem::ShutDown()
{
  if ( secondoSystem.initialized )
  {
    secondoSystem.algebraManager->UnloadAlgebras();
    delete secondoSystem.scDescriptive;
    secondoSystem.scDescriptive = 0;
    delete secondoSystem.scExecutable;
    secondoSystem.scExecutable  = 0;
    secondoSystem.initialized   = false;
  }
  return (!secondoSystem.initialized);
}

AlgebraManager*
SecondoSystem::GetAlgebraManager()
{
  return (secondoSystem.algebraManager);
}

QueryProcessor*
SecondoSystem::GetQueryProcessor()
{
  return (secondoSystem.queryProcessor);
}

SecondoCatalog*
SecondoSystem::GetCatalog( const AlgebraLevel level )
{
  if ( level == DescriptiveLevel )
  {
    return (secondoSystem.scDescriptive);
  }
  else
  {
    return (secondoSystem.scExecutable);
  }
}

NestedList*
SecondoSystem::GetNestedList()
{
  return (secondoSystem.nl);
}

