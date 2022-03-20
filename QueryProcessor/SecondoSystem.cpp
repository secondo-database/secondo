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

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

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


//#define TRACE_ON 1
#undef TRACE_ON
#include "LogMsg.h"

#include "ErrorCodes.h"
#include "SecondoSystem.h"
#include "QueryProcessor.h"
#include "Profiles.h"
#include "RelationAlgebra.h"
#include "NestedList.h"
#include "NList.h"

#include "../Tools/Flob/Flob.h"

#include "DatabaseListener.h"

using namespace std;

SecondoSystem* SecondoSystem::instance = 0;

const string nlfolder = "temp_nested_list";

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
    if ( IsDatabaseOpen() )
    {
      cerr << " CreateDatabase: database is already open!" << endl;
      exit( 0 );
    }
  }
  if ( SmiEnvironment::CreateDatabase( dbname ) )
  {
    SmiEnvironment::BeginTransaction();
    if ( catalog->Open() )
    {
      ok = true;
      SmiEnvironment::CommitTransaction();
      std::vector<DatabaseListener*>::iterator it;
      for(it=dblisteners.begin(); it!=dblisteners.end(); it++){
         (*it)->openDatabase(dbname);
      }
    }
    else
    {
      SmiEnvironment::AbortTransaction();
      SmiEnvironment::CloseDatabase();
      SmiEnvironment::EraseDatabase( dbname );
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
  if ( IsDatabaseOpen() )
  {
    cerr << " DestroyDatabase: database is already open!" << endl;
  }
  else
  {
    ok = SmiEnvironment::EraseDatabase( dbname );
  }
  return (ok);
}

SI_Error
SecondoSystem::OpenDatabase( const string& dbname )
{
/*
Opens a database with name ~dbname~.

Precondition: dbState = dbClosed.

*/
  SI_Error rc = ERR_NO_ERROR;
  if ( testMode && IsDatabaseOpen() )
  {
    cerr << " OpenDatabase: database is already open!" << endl;
    exit( 0 );
  }

  rc = SmiEnvironment::OpenDatabase( dbname );
  if ( rc == ERR_NO_ERROR )
  {
    SmiEnvironment::BeginTransaction();
    if ( catalog->Open() )
    {
      SmiEnvironment::CommitTransaction();
    }
    else
    {
      SmiEnvironment::AbortTransaction();
      SmiEnvironment::CloseDatabase();
      rc = ERR_SYSTEM_ERROR;
    }
  }

  // inform registered listeners
  if(rc == ERR_NO_ERROR){
     std::vector<DatabaseListener*>::iterator it;
     for(it=dblisteners.begin(); it!=dblisteners.end(); it++){
        (*it)->openDatabase(dbname);
     }
  }
  return rc;
}

bool
SecondoSystem::CloseDatabase()
{
/*
Closes the actually opened database. All open segments are closed and the allocated memory for internal data structures is returned.

Precondition: dbState = dbOpen.

*/
  if ( testMode && !IsDatabaseOpen() )
  {
    cerr << " CloseDatabase: database is already closed!" << endl;
    assert( false );
  }
  if(IsDatabaseOpen() ){
     std::vector<DatabaseListener*>::iterator it;
     for(it=dblisteners.begin(); it!=dblisteners.end(); it++){
        (*it)->closeDatabase();
     }
  }
  catalog->Close();
  Flob::dropFiles();

  return (SmiEnvironment::CloseDatabase());
}

bool
SecondoSystem::IsDatabaseOpen()
{
/*
Returns the state of the database.

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
  return catalog->IsObjectName( objectName );
}

bool
SecondoSystem::SaveObject ( const string& objectName,
                            const string& filename )
{
/*
Writes a secondo object called ~objectName~ of the currently opened database
to a file with name ~filename~ in nested list format. The format is the
following:

---- (OBJECT <object name> (<type name>) <type expression> <value>)*

----

Returns false if there was a problem in writing the file.

Precondition: dbState = dbOpen.

*/
  ListExpr objectList, typeExpr, valueList;
  objectList = typeExpr = valueList = nl->TheEmptyList();

  Word value = SetWord(Address(0));

  bool defined = false;
  bool hasTypeName = false;

  string typeName = "";
  string typeExprString="";

  if ( testMode && !IsDatabaseOpen() )
  {
    cerr << " SaveObject: database is not open!" << endl;
    exit( 0 );
  }

  catalog->GetObjectExpr( objectName, typeName, typeExpr,
                          value, defined, hasTypeName );
  if ( defined )
  {
    valueList = catalog->OutObject( typeExpr, value );
    catalog->CloseObject( typeExpr, value );
  }
  else
  {
    valueList = nl->TheEmptyList();
  }

  objectList = nl->FiveElemList(
                   nl->SymbolAtom( "OBJECT" ),
                   nl->SymbolAtom( objectName ),
                   nl->OneElemList( nl->SymbolAtom( typeName ) ),
                   typeExpr,
                   valueList );

  return (nl->WriteToFile( filename, objectList ));
}

bool
SecondoSystem::SaveDatabase ( const string& filename,
                              const DerivedObj& derivedObjs )
{
/*
Writes the currently open database called ~dbname~ to a file with name
~filename~ in nested list format. The format is the following:

---- (DATABASE <database name>
       (TYPES
         (TYPE <type name> <type expression>)*
       )
       (OBJECTS
         (OBJECT <object name> (<type name>) <type expression> <value>)*
       )
     )
----

Returns error 1 if there was a problem in writing the file.

Precondition: dbState = dbOpen.

*/
  if ( testMode && !IsDatabaseOpen() )
  {
    cerr << " SaveDatabase: database is not open!" << endl;
    exit( 0 );
  }

  ListExpr typeExpr = catalog->ListTypes();

  /* get the objects with value component !!*/
  ListExpr objExpr = catalog->ListObjectsFull(derivedObjs);

  ListExpr list;
  list = nl->TwoElemList( typeExpr, objExpr );
  list = nl->Cons(
           nl->SymbolAtom("DATABASE"),
           nl->Cons( nl->SymbolAtom( GetDatabaseName() ),
                     list ) );
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

  if ( testMode && !IsDatabaseOpen() )
  {
    cerr << " RestoreObjectFromFile: database is not open!" << endl;
    exit( 0 );
  }
  else
  {
    cout << "Reading file ..." << endl;
    if ( !nl->ReadFromFile( filename, list ) )
    {
      return 2; // Error reading file
    }

    if(!nl->HasLength(list,2) && !nl->HasMinLength(list,5))
    {
      return 3; // invalid list structure 
    }
    if(nl->HasLength(list,2))
    {
       list = nl->FiveElemList(nl->SymbolAtom("OBJECT"),
		               nl->SymbolAtom(objectname),
			       nl->TheEmptyList(),
			       nl->First(list),
			       nl->Second(list));
    } 
    else  if ( !nl->IsEqual( nl->Second( list ), objectname ) )
    {
      if(nl->AtomType(nl->Second(list))!=SymbolType){
          return 3; // error in list structure
      }
      string name = nl->SymbolValue(nl->Second(list));
      cmsg.warning() << "Object name of the file '"
                     << name <<  "' changed to '"
                     << objectname << "'!" << endl;
      cmsg.send();
      nl->Replace( nl->Second(list), nl->SymbolAtom(objectname) );
    }

    if ( RestoreObjects(
                 nl->TwoElemList( nl->SymbolAtom("OBJECTS"), list ),
                 errorInfo ) )
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
  ListExpr list, listFile;
  ListExpr types, objects;
  SI_Error rc = ERR_NO_ERROR;

  if ( testMode && IsDatabaseOpen() )
  {
    cerr << " RestoreDatabase: database is not closed!" << endl;
    assert( false );
  }

  cout << "Reading file " << filename << " ... " << endl;
  if ( !nl->ReadFromFile( filename, list ) )
  {
    rc = ERR_PROBLEM_IN_READING_FILE; // Error reading file
  }
  else
  {
    listFile = list;
/*
Tests the syntax of the database file named ~filename~.

*/
    if ( nl->ExprLength( list ) == 4 ||
         nl->ExprLength( list ) == 8 /* Keep compatibility with old files */)
    {
      if ( !nl->IsEqual( nl->Second( list ), dbname, false ) )
      {
//         rc = ERR_DB_NAME_NEQ_IDENT; // Database name in file different
        string oldname;
        nl->WriteToString(oldname, nl->Second(list));
        cmsg.warning() << "Original database name '"
                       << oldname <<  "' was changed to '"
                       << dbname << "'!" << endl;
        cmsg.send();
      }
      if ( nl->IsEqual( nl->First( list ), "DATABASE" ) )
      {
        list = nl->Rest( nl->Rest( list ) );

        if( nl->ExprLength( list ) == 2 )
        {
          types   = nl->First( list );
          objects = nl->Second( list );
        }
        else
        {
          types   = nl->Fifth( list );
          objects = nl->Sixth( list );
        }

        if ( nl->IsEmpty( types ) ||
             nl->IsEmpty( objects ) )
        {
          rc = ERR_IN_LIST_STRUCTURE_IN_FILE; // List structure invalid
        }
        else if ( nl->IsEqual( nl->First( types ), "TYPES" ) &&
                  nl->IsEqual( nl->First( objects ), "OBJECTS" ) )
        {
/*
Before restoring the database we need to get a completely empty database.
This is done by closing, destroying and recreating the database.

*/
          DestroyDatabase ( dbname );

          if(CreateDatabase( dbname )) {
/*
Load database types and objects from file named ~filename~.

*/
            if ( RestoreCatalog( types, objects, errorInfo ) )
            {
              rc = ERR_NO_ERROR; // Database successfully restored
            }
            else
            {
              rc = ERR_IN_DEFINITIONS_FILE; // Error in types or objects
            }
             SmiEnvironment::UpdateCatalog();
          } else {
             rc = E_SMI_DB_INVALIDNAME;
          }
        }
        else
        {
          // List structure invalid (Types or objects missing)
          rc = ERR_IN_LIST_STRUCTURE_IN_FILE;
        }
      }
      else
      {
        // List structure invalid (Database info missing)
        rc = ERR_IN_LIST_STRUCTURE_IN_FILE;
      }
    }
    else
    {
      // List structure invalid (List too short)
      rc = ERR_IN_LIST_STRUCTURE_IN_FILE;
    }
    nl->Destroy( listFile );
  }
  return (rc);
}

/*
3.3 Local Functions and Procedures

*/

bool
SecondoSystem::RestoreCatalog( ListExpr types,
                               ListExpr objects,
                               ListExpr& errorInfo )
{
  bool ok =  RestoreTypes( types, errorInfo );
  ok = ok && RestoreObjects( objects, errorInfo );
  return (ok);
}

bool
SecondoSystem::RestoreTypes( ListExpr types,
                             ListExpr& errorInfo )
{
  ListExpr first, typeExpr;
  int typeno;
  string typeName;
  bool correct = true;

  cout << "Restoring types ..." << endl;

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

      if ( catalog->KindCorrect( typeExpr, errorInfo ) )
      {
        if ( !catalog->InsertType( typeName, typeExpr ) )
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
    SecondoSystem::CommitTransaction(true);
  } // while

  return (correct);
}

bool
SecondoSystem::RestoreObjects( ListExpr objects,
                               ListExpr& errorInfo ){
  ListExpr first = nl->Empty();
  ListExpr typeExpr = nl->Empty();
  ListExpr valueList = nl->Empty();
  int objno = 0;
  string objectName="", typeName="";
  Word value = SetWord(Address(0));
  bool correctObj = false;
  bool correct = true;

  objects = nl->Rest( objects );

  cout << "Restoring objects ..." << endl;

  while ( !nl->IsEmpty( objects) ) {
    SecondoSystem::BeginTransaction();
    first = nl->First( objects );
    objects = nl->Rest( objects );
    objno++;
    if ( (nl->ExprLength( first) >= 5) &&
          nl->IsEqual( nl->First( first ), "OBJECT" ) &&
          nl->IsAtom( nl->Second( first ) ) &&
         (nl->AtomType( nl->Second( first ) ) == SymbolType ) &&
         !nl->IsAtom( nl->Third( first ) ) ) {
      objectName = nl->SymbolValue( nl->Second( first ) );
      cout << "  " << objectName << " ... ";
      if ( !nl->IsEmpty( nl->Third( first ) ) &&
            nl->IsAtom( nl->First( nl->Third( first ) ) ) &&
           (nl->AtomType( nl->First( nl->Third( first ) ) ) == SymbolType ) ) {
        typeName = nl->SymbolValue( nl->First( nl->Third( first ) ) );
      } else {
        typeName = "";
      }
      typeExpr = nl->Fourth( first );
      valueList = nl->Fifth( first );

      if ( catalog->KindCorrect( typeExpr, errorInfo ) ) {
        value = catalog->InObject( typeExpr, valueList,
                                   objno, errorInfo, correctObj );
        if ( correctObj ) {
          if ( !catalog->InsertObject( objectName, typeName, typeExpr,
                                  value, true ) ){
            // doubly defined object
            errorInfo = nl->Append( errorInfo,
                          nl->ThreeElemList(
                            nl->IntAtom( 51 ),
                            nl->IntAtom( objno ),
                            nl->Second( first ) ) );
          }
        } else {
          // wrong list representation
          correct = false;
          errorInfo = nl->Append( errorInfo,
                        nl->ThreeElemList(
                          nl->IntAtom( 53 ),
                          nl->IntAtom( objno ),
                          nl->Second( first ) ) );
        }
      } else {
        // wrong type expression
        correct = false;
        correctObj = false;
        errorInfo = nl->Append( errorInfo,
                      nl->FourElemList(
                        nl->IntAtom( 52 ),
                        nl->IntAtom( objno ),
                        nl->Second( first ),
                        typeExpr ));
      }
    } else {
      // error in object definition
      correct = false;
      correctObj = false;
      errorInfo = nl->Append( errorInfo,
                    nl->TwoElemList(
                      nl->IntAtom( 50 ),
                      nl->IntAtom( objno ) ) );
    }

    SecondoSystem::CommitTransaction(true);
    cout << "processed";

    if(correctObj){
      cout << " and succeeded." << endl;
    } else {
      cout << " but FAILED!" << endl;
    }


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
  this->nl = new NestedList(nlfolder);
  this->al = new NestedList(nlfolder);
  NList::setNLRef(this->nl);

  algebraManager = new AlgebraManager( *(this->nl), getAlgebraEntryFunc );
  queryProcessor = new QueryProcessor( this->nl, algebraManager );
  catalog        = 0;
  initialized    = false;
  testMode       = false; // Todo: Should be configurable in SecondoConfig.ini
  heart1 = 0;
  heart2 = 0;
}

SecondoSystem::~SecondoSystem()
{

  bool shutdownOk = false;
  if ( initialized )
  {
    try
    {
      ShutDown();
      shutdownOk = true;
    }
    catch (SecondoException& e)
    {
      string smiErrors;
      SmiEnvironment::GetLastErrorCode( smiErrors );
      cmsg.error() << e.msg() << endl;
      cmsg.error() << smiErrors << endl;
      cmsg.send();
    }
  }

  delete algebraManager;
  algebraManager=0;

  delete nl;
  nl = 0;
  delete al;
  al = 0;
  delete catalog;
  delete queryProcessor;

  instance = 0;

  if (!shutdownOk)
    cerr << "shutdown of SecondoSystem failed" << endl;
}

bool
SecondoSystem::CreateInstance( GetAlgebraEntryFunction f)
{
  if (!instance) {
    instance = new SecondoSystem(f);
    return true;
  }
  return false;
}


SecondoSystem*
SecondoSystem::GetInstance()
{
  assert(instance);
  return (instance);
}

bool
SecondoSystem::StartUp()
{
  if ( !instance->initialized )
  {
    instance->algebraManager->LoadAlgebras();
    instance->catalog = new SecondoCatalog();
    instance->catalog->InitTypeConstructors();
    instance->initialized = true;
  }
  return (instance->initialized);
}

bool
SecondoSystem::ShutDown()
{
  if ( instance->IsDatabaseOpen() ) {
     instance->CloseDatabase();
  }

  if ( instance->initialized )
  {
    delete instance->catalog;
    instance->catalog = 0;
    instance->initialized  = false;
  }

  SmiEnvironment::ShutDown();

  return (!instance->initialized);
}

AlgebraManager*
SecondoSystem::GetAlgebraManager()
{
  return (instance->algebraManager);
}

QueryProcessor*
SecondoSystem::GetQueryProcessor()
{
  return (instance->queryProcessor);
}

SecondoCatalog*
SecondoSystem::GetCatalog()
{
  return instance->catalog;
}

NestedList*
SecondoSystem::GetNestedList()
{
  return (instance->nl);
}

NestedList*
SecondoSystem::GetAppNestedList()
{
  return (instance->al);
}

bool
SecondoSystem::BeginTransaction()
{
  TRACE_ENTER
  TRACE_LEAVE
  return (SmiEnvironment::BeginTransaction());
}

bool
SecondoSystem::CommitTransaction(const  bool closeObjects)
{
  TRACE_ENTER
  instance->catalog->CleanUp(closeObjects);
  TRACE_LEAVE
  return (SmiEnvironment::CommitTransaction());
}

bool
SecondoSystem::AbortTransaction(const bool closeObjects)
{
  TRACE_ENTER
  instance->catalog->ClearPendingChanges(closeObjects);
  TRACE_LEAVE
  return (SmiEnvironment::AbortTransaction());
}

bool
SecondoSystem::SetHeartbeat(const int _heart1, const int _heart2){
  if(_heart1<0 || _heart2 < 0){
     return false;
  }
  this->heart1 = _heart1;
  this->heart2 = _heart2;
  return true;
}

void SecondoSystem::addDBListener(DatabaseListener* dbl) {
   if(dbl!=0){
     for(size_t i=0;i<dblisteners.size();i++){
        if(dblisteners[i]==dbl){
           return;
        }
     }
     dblisteners.push_back(dbl);
   }
}

void SecondoSystem::removeDBListener(DatabaseListener* dbl) {
   if(dbl!=0){
      std::vector<DatabaseListener*>::iterator it;
      for(it = dblisteners.begin(); it!=dblisteners.end(); it++){
         if(*it == dbl){
            dblisteners.erase(it);
            return;
         }
      }
   }
}



