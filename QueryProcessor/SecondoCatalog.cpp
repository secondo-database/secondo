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

1 The Implementation-Module *SecondoCatalog*

September 1996 Claudia Freundorfer

November 18, 1996 RHG Replaced all calls to ~CatalogManager~ by calls to
~CTable~.

December 20, 1996 Changed procedures ~OutObject~ and ~GetObjectValue~
and introduced ~NumericType~.

December 30, 1996 RHG Added procedures ~ExpandedType~ and ~KindCorrect~.

January 7-9, 1997 RHG Major revision for error handling.

January 13, 1997 RHG Corrected error in procedure ~OpenDatabase~.

December 23, 1997 RHG Corrected procedure ~LookUpTypeExpr~ to make it
safe against wrong type expressions.

May 15, 1998 RHG Added treatment of models, especially functions
~InObjectModel~, ~OutObjectModel~, and ~ValueToObjectModel~.

October 13, 1998 Stefan Dieker ~NumericTypeExpr~ may now be called in
database state ~dbClosed~, too.

September 9, 1998 Stefan Dieker Reimplemented functions ~CloseDatabase~
and ~OpenDatabase~ in such a way that all CTables and NameIndexes used
for storing object and type information are saved to files and loaded
from those files, respectively. Now the catalog is semi-persistent, i.e.
as persistent as provided by the underlying OS, without access being
save under transactions, logged, and locked.

October 2002 Ulrich Telle, testMode flag initialization added

September 2003 Frank Hoffmann, added (overloaded) ~ListTypeConstructors~
and ~ListOperators~

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

April 2006, M. Spiekermann. New methods ~systemTable~ and ~createRelation~ added.
These will be used to check if a given object name is a system table and if a
relation should be created on the fly by calling ~InObject~.


This module implements the module *SecondoCatalog*. It consists of six
parts: First it contains an interface to *Databases and Transactions*
for loading the actual catalog of database types and objects and for
managing transactions. Therefore it offers database functions to open,
close, save, restore, create and destroy a named database as described
in the SECONDO Programming Interface. Second it offers an interface for
inquiries to give the database user information about the type system
and the used algebras. Third it delivers functions for retrieving and
manipulating database types. Fourth the module offers functions for
retrieving, updating, deleting and inserting database objects. Fifth and
Sixth it delivers functions to look up the type constructors and
operators of the actually loaded algebras. These functions are
implemented by applying the functions of the lower modules *CTable*,
*NameIndex*, *AlgebraManager2*, *ObjectListManager* and *ObjectManager*.
The names of existing databases are stored in a list ~DBTable~.

\tableofcontents

1.2 Implementation

*/

#include <string>

#include "SecondoSystem.h"
#include "DerivedObj.h"
#include "SecondoCatalog.h"
#include "NList.h"
#include "SystemTables.h"
#include "ExampleREader.h"

using namespace std;

/**************************************************************************
2.2 Types

*/

const SmiSize CE_TYPES_ALGEBRA_ID = 0;
const SmiSize CE_TYPES_TYPE_ID    = sizeof( int ) + CE_TYPES_ALGEBRA_ID;
const SmiSize CE_TYPES_EXPR_SIZE  = sizeof( int ) + CE_TYPES_TYPE_ID;
const SmiSize CE_TYPES_EXPR_START = sizeof( int ) + CE_TYPES_EXPR_SIZE;

const SmiSize CE_OBJS_VALUE          = 0;
const SmiSize CE_OBJS_VALUE_DEF      = sizeof( Word ) + CE_OBJS_VALUE;
const SmiSize CE_OBJS_VALUE_RECID    = sizeof( bool ) + CE_OBJS_VALUE_DEF;
const SmiSize CE_OBJS_ALGEBRA_ID = sizeof( SmiRecordId ) + CE_OBJS_VALUE_RECID;
const SmiSize CE_OBJS_TYPE_ID        = sizeof( int ) + CE_OBJS_ALGEBRA_ID;
const SmiSize CE_OBJS_TYPENAME_SIZE  = sizeof( int ) + CE_OBJS_TYPE_ID;
const SmiSize CE_OBJS_TYPEEXPR_SIZE  = sizeof( int ) + CE_OBJS_TYPENAME_SIZE;
const SmiSize CE_OBJS_TYPEINFO_START = sizeof( int ) + CE_OBJS_TYPEEXPR_SIZE;

/*
~InfoPointer~ is a reference to a record structure ~Info~ that is used
to store the information about type constructors, operators, database
objects and types. The same fields can be used for the different kinds
of stored information:

  * type constructors: For every type constructor, the fields ~algebraId,
typeId~ contain the type numbers and algebra numbers of the type name,
~props~ contains the functionality of the type constructor.

  * operators: For every operator the fields ~algebraId, typeId~ contain
the operator numbers and algebra numbers of the operator name and
~props~ contains the corresponding operator specification as defined in
the *SECONDO Project*.

  * database objects: The fields ~algebraId, typeId~ contain identifiers
for the type name of the stored object, ~props~ contains the type name
and type expression in nested list format and ~value~ delivers the
object value as a word. Field ~valueDefined~ tells whether a value has
been assigned to the object. 

  * database types: For every database type, the fields ~algebraId,
typeId~ contain the type numbers and algebra numbers of the type name,
~props~ contains the corresponding type expression.

*/

/**************************************************************************
3 Functions and Procedures

3.2 Catalog Management Operations

3.2.1 Database Types

*/

ListExpr
SecondoCatalog::ListTypes()
{
/*
Returns a list of ~types~ of the whole database in the following format:

---- (TYPES
       (TYPE <type name><type expression>)*
     )
----

Precondition: dbState = dbOpen.

*/
  ListExpr typesList, typeExpr, lastElem = 0;
  string typeName, typeExprString;
  char*  typeBuffer;
  SmiKeyedFileIterator typeIterator;
  SmiKey typeKey;
  SmiRecord typeRecord;
  TypesCatalog::iterator tPos;

  if ( testMode && !SmiEnvironment::IsDatabaseOpen() )
  {
    cerr << " ListTypes: database is closed!" << endl;
    return (nl->TheEmptyList());
  }

  typesList = nl->TheEmptyList();
  if ( typeCatalogFile.SelectAll( typeIterator ) )
  {
    while (typeIterator.Next( typeKey, typeRecord ))
    {
      typeKey.GetKey( typeName );
      tPos = types.find( typeName );
      if ( tPos != types.end() )
      {
        if ( tPos->second.state == EntryDelete ||
             tPos->second.state == EntryUpdate ) continue;
      }
      int exprSize;
      if ( !typeRecord.Read( &exprSize, 
	   sizeof( int ), CE_TYPES_EXPR_SIZE ) ) continue;
      typeBuffer = new char[exprSize];
      if ( !typeRecord.Read( typeBuffer, 
	                     exprSize, CE_TYPES_EXPR_START ) ) continue;
      typeExprString.assign( typeBuffer, exprSize );
      delete []typeBuffer;
      nl->ReadFromString( typeExprString, typeExpr );
      typeExpr = nl->First( typeExpr );
      if ( typesList == nl->TheEmptyList() )
      {
        typesList = nl->Cons(
                      nl->ThreeElemList(
                        nl->SymbolAtom( "TYPE" ),
                        nl->SymbolAtom( typeName ),
                        typeExpr ),
                      nl->TheEmptyList() );
        lastElem = typesList;
      }
      else
      {
        lastElem = nl->Append(
                     lastElem,
                     nl->ThreeElemList(
                       nl->SymbolAtom( "TYPE" ),
                       nl->SymbolAtom( typeName ),
                       typeExpr ) );
      }
    }
  }
  typeIterator.Finish();
  for ( tPos = types.begin(); tPos != types.end(); tPos++ )
  {
    if ( tPos->second.state == EntryInsert ||
         tPos->second.state == EntryUpdate )
    {
      nl->ReadFromString( tPos->second.typeExpr, typeExpr );
      typeExpr = nl->First( typeExpr );
      if ( typesList == nl->TheEmptyList() )
      {
        typesList = nl->Cons(
                      nl->ThreeElemList(
                        nl->SymbolAtom( "TYPE" ),
                        nl->SymbolAtom( tPos->first ),
                        typeExpr ),
                      nl->TheEmptyList() );
        lastElem = typesList;
      }
      else
      {
        lastElem = nl->Append(
                     lastElem,
                     nl->ThreeElemList(
                       nl->SymbolAtom( "TYPE" ),
                       nl->SymbolAtom( tPos->first ),
                       typeExpr ) );
      }
    }
  }
  typesList = nl->Cons( nl->SymbolAtom( "TYPES" ), typesList );
  return (typesList);
}

bool
SecondoCatalog::InsertType( const string& typeName, ListExpr typeExpr )
{
/*
Inserts a new type named ~typeName~ defined by a list ~typeExpr~ of
already existing types in the database. Returns ~false~, if the name was
already defined. If the type name already exists, the procedure has no
effect.

Precondition: dbState = dbOpen.

*/
  bool ok = false;
  string name;
  SmiRecord tRecord;

  if ( testMode && !SmiEnvironment::IsDatabaseOpen() )
  {
    cerr << " InsertType: database is closed!" << endl;
    return (false);
  }
  TypesCatalog::iterator tPos = types.find( typeName );
  if ( tPos != types.end() )
  {
    if ( tPos->second.state == EntryDelete )
    {
      tPos->second.state = EntryUpdate;
      LookUpTypeExpr( typeExpr, name,
                      tPos->second.algebraId, tPos->second.typeId );
      nl->WriteToString( tPos->second.typeExpr, nl->OneElemList( typeExpr ) );
      ok = true;
    }
  }
  else if ( !typeCatalogFile.SelectRecord( SmiKey( typeName ), tRecord ) )
  {
    TypesCatalogEntry typeEntry;
    typeEntry.state = EntryInsert;
    LookUpTypeExpr( typeExpr, name, typeEntry.algebraId, typeEntry.typeId );
    nl->WriteToString( typeEntry.typeExpr, nl->OneElemList( typeExpr ) );
    types.insert( make_pair( typeName, typeEntry ) );
    ok = true;
  }
  return (ok);
}

int
SecondoCatalog::DeleteType( const string& typeName )
{
/*
Deletes a type with name ~typeName~ in the database. This is only possible if the type 
with name ~typeName~ is not used by an object. Returns error 1 if type is used by an 
object, error 2, if type name is not known.

Precondition: dbState = dbOpen.

*/
  int rc = 0;
  SmiRecord tRec;

  if ( SmiEnvironment::IsDatabaseOpen() )
  {
    TypesCatalog::iterator tPos = types.find( typeName );
    if ( tPos != types.end() )
    {
      if ( tPos->second.state != EntryDelete )
      {
        if ( TypeUsedByObject( typeName) )
        {
          rc = 1;
        }
        else
        {
          if ( tPos->second.state == EntryUpdate )
          {
            tPos->second.state = EntryDelete;
          }
          else
          {
            types.erase( tPos );
          }
          rc = 0;
        }
      }
      else
      {
        rc = 2;
      }
    }
    else if ( typeCatalogFile.SelectRecord( SmiKey( typeName ), tRec ) )
    {
      if ( TypeUsedByObject( typeName) )
      {
        rc = 1;
      }
      else
      {
        TypesCatalogEntry tEntry;
        int exprSize;
        tEntry.state = EntryDelete;
        if ( tRec.Read( &tEntry.algebraId, 
	     sizeof( int ), CE_TYPES_ALGEBRA_ID ) &&
             tRec.Read( &tEntry.typeId, 
	     sizeof( int ), CE_TYPES_TYPE_ID ) &&
             tRec.Read( &exprSize, 
	     sizeof( int ), CE_TYPES_EXPR_SIZE ) )
        {
          char* tBuffer = new char[exprSize];
          tRec.Read( tBuffer, exprSize, CE_TYPES_EXPR_START );
          tEntry.typeExpr.assign( tBuffer, exprSize );
          delete []tBuffer;
          types.insert( make_pair( typeName, tEntry ) );
          rc = 0;
        }
        else
        {
          rc = 2;
        }
      }
    }
    else
    {
      rc = 2;
    }
  }
  else if ( testMode )
  {
    cerr << " DeleteType: database is closed!" << endl;
    exit( 0 );
  }
  return (rc);
}

bool
SecondoCatalog::TypeUsedByObject( const string& typeName )
{
  bool used = false;
  ObjectsCatalog::iterator oPos;
  for ( oPos = objects.begin(); !used && oPos != objects.end(); oPos++ )
  {
    used = oPos->second.state != EntryDelete &&
           oPos->second.typeName == typeName;
  }
  if ( !used )
  {
    SmiKeyedFileIterator oIterator;
    SmiKey oKey;
    SmiRecord oRec;
    if ( objCatalogFile.SelectAll( oIterator ) )
    {
      int nameSize;
      char* buffer;
      string typeName2;
      while (!used && oIterator.Next( oRec ))
      {
        oRec.Read( &nameSize, sizeof( int ), CE_OBJS_TYPENAME_SIZE );
        if ( nameSize > 0 )
        {
          buffer = new char[nameSize];
          oRec.Read( buffer, nameSize, CE_OBJS_TYPEINFO_START );
          typeName2.assign( buffer, nameSize );
          delete []buffer;
          used = (typeName == typeName2);
        }
      }
    }
  }
  return (used);
}

bool
SecondoCatalog::MemberType( const string& typeName )
{
/*
Returns true iff type with name ~typeName~ is member of the actually opened database.

Precondition: dbState = dbOpen.

*/
  bool found = false;
  if ( SmiEnvironment::IsDatabaseOpen() )
  {
    TypesCatalog::iterator tPos = types.find( typeName );
    if ( tPos != types.end() )
    {
      found = (tPos->second.state != EntryDelete);
    }
    else
    {
      SmiRecord tRecord;
      found = typeCatalogFile.SelectRecord( SmiKey( typeName ), tRecord );
    }
  }
  else if ( testMode )
  {
    cerr << " MemberType: database is closed!" << endl;
    exit( 0 );
  }
  return (found);
}

bool
SecondoCatalog::LookUpTypeExpr( const ListExpr typeExpr,
                                string& typeName,
                                int& algebraId, int& typeId )
{
/*
Returns the algebra identifier ~algebraId~ and the type identifier ~opId~
and the name ~typeName~ of the outermost type constructor for a given type
expression ~typeExpr~, if it exists, otherwise an empty string as ~typeName~
and value 0 for the identifiers.

Precondition: dbState = dbOpen.

*/
  bool ok = false;
  ListExpr first = 0;

  if ( testMode && !SmiEnvironment::IsDatabaseOpen() )
  {
    cerr << " LookUpTypeExpr: database is closed!" << endl;
    return (false);
  }
  
  if ( nl->IsAtom( typeExpr ) )
  {
    first = typeExpr;
    ok = true;
  }
  else if ( !nl->IsEmpty( typeExpr ) )
  {
    first = nl->First( typeExpr );
    ok = nl->IsAtom( first );
  }
  if ( ok )
  {
    ok = false;
    if ( nl->AtomType( first ) == SymbolType )
    {
      typeName = nl->SymbolValue( first );
      if ( typeName != "" && IsTypeName( typeName ) )
      {
        ok = GetTypeId( typeName, algebraId, typeId );
      }
    }
  }
  if ( !ok )
  {
    algebraId = 0;
    typeId    = 0;
    typeName  = "";
  }
  return (ok);
}

ListExpr
SecondoCatalog::GetTypeExpr( const string& typeName )
{
/*
Returns a type expression ~typeExpr~ for a given type name ~typeName~, if exists.

Precondition: dbState = dbOpen and ~MemberType(typeName)~ delivers TRUE.

*/
  ListExpr typeExpr = nl->TheEmptyList();

  TypesCatalog::iterator tPos = types.find( typeName );
  if ( tPos != types.end() )
  {
    if ( tPos->second.state != EntryDelete )
    {
      nl->ReadFromString( tPos->second.typeExpr, typeExpr );
      typeExpr = nl->First( typeExpr );
    }
  }
  else
  {
    SmiRecord tRec;
    if ( typeCatalogFile.SelectRecord( SmiKey( typeName ), tRec ) )
    {
      int exprSize;
      tRec.Read( &exprSize, sizeof( int ), CE_TYPES_EXPR_SIZE );
      char* tBuffer = new char[exprSize];
      tRec.Read( tBuffer, exprSize, CE_TYPES_EXPR_START );
      string typeExprString;
      typeExprString.assign( tBuffer, exprSize );
      delete []tBuffer;
      nl->ReadFromString( typeExprString, typeExpr );
      typeExpr = nl->First( typeExpr );
    }
    else if ( testMode )
    {
      cerr << " GetTypeExpr: " << typeName 
           << " is not a valid type name!" << endl;
      exit( 0 );
    }
  }
  return (typeExpr);
}

/*
3.1.1 Transform type Expressions

*/

ListExpr
SecondoCatalog::NumericType( const ListExpr type )
{
/*
Transforms a given type expression into a list structure where each type constructor has been replaced by the corresponding pair (algebraId, typeId). For example,

----    int    ->    (1 1)

    (rel (tuple ((name string) (age int)))

    ->     ((2 1) ((2 2) ((name (1 4)) (age (1 1))))
----

Identifiers such as ~name~, ~age~ are moved unchanged into the result list. If a type expression contains other constants that are not symbols, e.g. integer constants as in (array 10 real), they are also moved unchanged into the result list.

The resulting form of the type expression is useful for calling the type specific ~In~ and ~Out~ procedures.

*/
  string name;
  int alId, typeId;

  if ( nl->IsEmpty( type ) )
  {
    return (type);
  }
  else if ( nl->IsAtom( type ) )
  {
    if ( nl->AtomType( type ) == SymbolType )
    {
      name = nl->SymbolValue( type );
      if ( IsTypeName( name ) &&
          (!SmiEnvironment::IsDatabaseOpen() || !MemberType( name )) )
      {
        GetTypeId( name, alId, typeId );
        return (nl->TwoElemList(
                  nl->IntAtom( alId ),
                  nl->IntAtom( typeId ) ));
      }
      else
      {
         return (type);  /* return identifier */
      }
    }
    else
    {
      return (type);  /* return other constants */
    }
  }
  else
  { /* is a nonempty list */
    return (nl->Cons( NumericType( nl->First( type ) ),
                      NumericType( nl->Rest( type ) ) ));
  }
}

ListExpr
SecondoCatalog::ExpandedType ( const ListExpr type )
{
/*
Transforms a given type definition (a type expression possibly containing type names, or just a single type name) into the corresponding type expression where all names have been replaced by their defining expressions.

*/
  string name;
  ListExpr typeExpr;

  if ( nl->IsEmpty( type ) )
  {
    return (type);
  }
  else if ( nl->IsAtom( type ) )
  {
    if ( nl->AtomType( type ) == SymbolType )
    {
      name = nl->SymbolValue( type );
      if ( MemberType( name ) )
      {
        typeExpr = GetTypeExpr( name );
        return (typeExpr);
      }
      else
      {
        return (type);  /* return type constructor */
      }
    }
    else
    {
      return (type);  /* return other constants */
    }
  }
  else
  { /* is a nonempty list */
    return (nl->Cons( ExpandedType( nl->First( type ) ),
                      ExpandedType( nl->Rest( type ) ) ));
  }
}

/****************************************************************************
3.1.1 Kind Checking

*/
bool
SecondoCatalog::KindCorrect( const ListExpr typeExpr, ListExpr& errorInfo )
{
/*
Here ~type~ is a type expression. ~KindCorrect~ does the kind checking; if there are errors, they are reported in the list ~errorInfo~, and ~FALSE~ is returned. ~errorInfo~ is a list whose entries are again lists, the first element of an entry is an error code number. For example, an entry

----    (1 DATA (hello world))
----

says that kind ~DATA~ does not match the type expression ~(hello world)~. This is the meaning of the general error code 1. The other error codes are type-constructor specific.

*/
  string typeName;
  int algebraId, typeId;

  LookUpTypeExpr( typeExpr, typeName, algebraId, typeId );
  if ( algebraId == 0 )
  {
    return (false);
  }
  else
  {
    return am->TypeCheck( algebraId, typeId, typeExpr, errorInfo );
  }
}

/************************************************************************
3.1.2 Database Objects

The function below creates a four element list which is used as object
description.

*/

static ListExpr objEntry( const string& name, 
                          const string& type, ListExpr typeExpr ) 
{   
  return nl->FourElemList(
               nl->SymbolAtom( "OBJECT" ),
               nl->SymbolAtom( name ),
               nl->OneElemList( nl->SymbolAtom( type ) ),
               typeExpr );
} 


static void appendEntry( ListExpr& start, 
                         ListExpr& last, const ListExpr elem )
{ 
  if ( start == nl->Empty() )
  {
    start = nl->Cons( elem, nl->Empty() ); 
    last = start;
  }
  else
  {
    last = nl->Append( last, elem );
  }
}



ListExpr
SecondoCatalog::ListObjects()
{
/*
Returns a list of ~objects~ of the whole database in the following format:

---- (OBJECTS
       (OBJECT <object name>(<type name>) <type expression>)*
     )
----

For each object the *value* component is missing, otherwise the whole database will be returned.

Precondition: dbState = dbOpen.

*/


  ListExpr objectsList = nl->Empty(); 
  ListExpr typeExpr = nl->Empty(); 
  ListExpr lastElem = nl->Empty();
    
  string objectName="", typeName="", typeExprString="";
  char*  oBuffer=0;
  SmiKeyedFileIterator oIterator;
  SmiKey oKey;
  SmiRecord oRec;
  ObjectsCatalog::iterator oPos;
  if ( testMode && !SmiEnvironment::IsDatabaseOpen() )
  {
    cerr << " ListObjects: database is closed!" << endl;
    exit( 0 );
  }

  objectsList = nl->TheEmptyList();
  if ( objCatalogFile.SelectAll( oIterator ) )
  {
    while (oIterator.Next( oKey, oRec ))
    {
      oKey.GetKey( objectName );
      oPos = objects.find( objectName );
      if ( oPos != objects.end() )
      {
        if ( oPos->second.state == EntryDelete ||
             oPos->second.state == EntryUpdate ) continue;
      }
      int nameSize, exprSize;
      if ( !oRec.Read( &nameSize, 
	   sizeof( int ), CE_OBJS_TYPENAME_SIZE ) ) continue;
      if ( !oRec.Read( &exprSize, 
	   sizeof( int ), CE_OBJS_TYPEEXPR_SIZE ) ) continue;
      int bufSize = (nameSize > exprSize) ? nameSize : exprSize;
      oBuffer = new char[bufSize];
      if ( nameSize > 0 )
      {
        if ( !oRec.Read( oBuffer, 
	     nameSize, CE_OBJS_TYPEINFO_START ) ) continue;
        typeName.assign( oBuffer, nameSize );
      }
      else
      {
        typeName = "";
      }
      if ( !oRec.Read( oBuffer, 
	   exprSize, CE_OBJS_TYPEINFO_START+nameSize ) ) continue;
      typeExprString.assign( oBuffer, exprSize );
      delete []oBuffer;
      nl->ReadFromString( typeExprString, typeExpr );
//VTA - This line must be added
//      typeExpr = nl->First( typeExpr );
      appendEntry( objectsList, lastElem, 
                   objEntry(objectName, typeName, typeExpr) );
    }
  }
  oIterator.Finish();
  for ( oPos = objects.begin(); oPos != objects.end(); oPos++ )
  {
    if ( oPos->second.state == EntryInsert ||
         oPos->second.state == EntryUpdate )
    {
      objectName = oPos->first;
      typeName = oPos->second.typeName;
      nl->ReadFromString( oPos->second.typeExpr, typeExpr );
//VTA - This line must be added
//      typeExpr = nl->First( typeExpr );
      appendEntry( objectsList, lastElem, 
                   objEntry(objectName, typeName, typeExpr) );
    }
  }
  // Append system tables
  SystemTables& st = SystemTables::getInstance();
  SystemTables::iterator it = st.begin();
  while ( it != st.end() )
  {
    objectName = it->first;
    typeName = nl->Empty();
    if (!it->second->isPersistent)
    { 
      typeExpr = it->second->relSchema().enclose().listExpr();
      appendEntry( objectsList, lastElem, 
                   objEntry(objectName, typeName, typeExpr) );
    }  
    it++;
  } 

  objectsList = nl->Cons( nl->SymbolAtom( "OBJECTS" ), objectsList );
  return (objectsList);
}

ListExpr
SecondoCatalog::ListObjectsFull(const DerivedObj& derivedObjs)
{
/*
Returns a list of ~objects~ of the whole database in the following format:

---- (OBJECTS
       (OBJECT <object name>(<type name>) <type expression> <value>)*
     )
----

Precondition: dbState = dbOpen.

*/
  ListExpr objectsList, typeExpr, valueList, lastElem = 0;
  Word value;
  bool defined, hasTypeName;
  string objectName, typeName, typeExprString;
  SmiKeyedFileIterator oIterator;
  SmiKey oKey;
  SmiRecord oRec;
  ObjectsCatalog::iterator oPos;

  if ( testMode && !SmiEnvironment::IsDatabaseOpen() )
  {
    cerr << " ListObjects: database is closed!" << endl;
    exit( 0 );
  }
  cout << endl << "Saving objects ..." << endl;
  const string msgOmitted = "omitted (derived object).";
  const string msgSaved = "saved.";
  
  objectsList = nl->TheEmptyList();
  if ( objCatalogFile.SelectAll( oIterator ) )
  {
    while (oIterator.Next( oKey, oRec ))
    {
      oKey.GetKey( objectName );
      oPos = objects.find( objectName );
      
      if ( oPos != objects.end() )
      {
        if ( oPos->second.state == EntryDelete ||
             oPos->second.state == EntryUpdate ) continue;
      }
      
      // check for derived objects
      cout << "  " << objectName << " ... ";
      if ( derivedObjs.isDerived(objectName) ) {
         cout <<  msgOmitted << endl;
         continue;
      }	 
      

      GetObjectExpr( objectName, typeName, typeExpr,
                     value, defined, hasTypeName );
      if ( defined )
      {
        valueList = OutObject( typeExpr, value );
      }
      else
      {
        valueList = nl->TheEmptyList();
      }
      if ( objectsList == nl->TheEmptyList() )
      {
        objectsList = 
	   nl->Cons( nl->FiveElemList(
                           nl->SymbolAtom( "OBJECT" ),
                           nl->SymbolAtom( objectName ),
                           nl->OneElemList( nl->SymbolAtom( typeName ) ),
                           typeExpr,
                           valueList ),
                      nl->TheEmptyList() );
        lastElem = objectsList;
      }
      else
      {
        lastElem = nl->Append( lastElem,
                     nl->FiveElemList(
                       nl->SymbolAtom( "OBJECT" ),
                       nl->SymbolAtom( objectName ),
                       nl->OneElemList( nl->SymbolAtom( typeName ) ),
                       typeExpr,
                       valueList ) );
      }
      cout << msgSaved << endl;
    }
  }
  oIterator.Finish();
  for ( oPos = objects.begin(); oPos != objects.end(); oPos++ )
  {    
    if ( oPos->second.state == EntryInsert ||
         oPos->second.state == EntryUpdate )
    {
      
      // check for derived objects
      cout << "  " << oPos->first << " ... ";
      if ( derivedObjs.isDerived(oPos->first) ) {
         cout <<  msgOmitted << endl;
         continue;
      }	 

      nl->ReadFromString( oPos->second.typeExpr, typeExpr );
//VTA - This line must be added
//      typeExpr = nl->First( typeExpr );
      if ( oPos->second.valueDefined )
      {
        valueList = OutObject( typeExpr, oPos->second.value );
      }
      else
      {
        valueList = nl->TheEmptyList();
      }
      if ( objectsList == nl->TheEmptyList() )
      {
        objectsList = 
	   nl->Cons( nl->FiveElemList(
                       nl->SymbolAtom( "OBJECT" ),
                       nl->SymbolAtom( oPos->first ),
                       nl->OneElemList( 
			     nl->SymbolAtom( oPos->second.typeName ) ),
                       typeExpr,
                       valueList ),
                     nl->TheEmptyList() );
        lastElem = objectsList;
      }
      else
      {
        lastElem = 
	   nl->Append( lastElem,
                       nl->FiveElemList(
                         nl->SymbolAtom( "OBJECT" ),
                         nl->SymbolAtom( oPos->first ),
                         nl->OneElemList( 
			       nl->SymbolAtom( oPos->second.typeName ) ),
                         typeExpr,
                       valueList ) );
      }
    cout << msgSaved << endl;
    }
  }

  objectsList = nl->Cons( nl->SymbolAtom( "OBJECTS" ), objectsList );
  return (objectsList);
}

bool
SecondoCatalog::CreateObject( const string& objectName, 
                              const string& typeName,
                              const ListExpr typeExpr, 
			      const int sizeOfComponents )
{
/*
Creates a new object with identifier ~objectName~ defined with type name ~typeName~ 
(can be empty) and type ~typeExpr~. The value is not yet defined, and no memory is 
allocated. Returns error 1, if the object name is already defined.

Precondition: dbState = dbOpen.

*/
  int alId = 7, typeId = 7;
  Word value;
  string typecon;

  if ( testMode && !SmiEnvironment::IsDatabaseOpen() )
  {
    cerr << " CreateObject: database is closed!" << endl;
    exit( 0 );
  }
  if ( typeName != "" )
  {
    if ( IsTypeName( typeName ) )
    {
      GetTypeId( typeName, alId, typeId );
    }
    else
    {
      LookUpTypeExpr( typeExpr, typecon, alId, typeId );
    }
  }
  else
  {
    LookUpTypeExpr( typeExpr, typecon, alId, typeId );
  }
  return (InsertObject( objectName, typeName, typeExpr, value, false ));
}

bool
SecondoCatalog::InsertObject( const string& objectName, const string& typeName,
                              const ListExpr typeExpr, const Word valueWord,
                              const bool defined )
{
/*
Inserts a new object with identifier ~objectName~ and value ~valueWord~ defined by 
type name ~typeName~ or by a list ~typeExpr~ of already existing types (which always 
exists) into the database catalog. Parameter ~defined~ tells, whether ~wordvalueWord~ 
actually contains a defined value. If the object name already exists, the procedure 
has no effect. Returns error 1 if the ~objectName~ is used already.

When the given object has no type name, it is mandatory, that ~typeName~
is an empty string.

Precondition: dbState = dbOpen.

*/
  bool ok = false;
  string name;
  SmiRecord oRecord;
  if ( testMode && !SmiEnvironment::IsDatabaseOpen() )
  {
    cerr << " InsertObject: database is closed!" << endl;
    exit( 0 );
  }

  ObjectsCatalog::iterator oPos = objects.find( objectName );
  if ( oPos != objects.end() )
  {
    if ( oPos->second.state == EntryDelete )
    {
      oPos->second.state = EntryUpdate;
      oPos->second.value = valueWord;
      oPos->second.valueDefined = defined;
      oPos->second.valueRecordId = 0;
      oPos->second.typeName = typeName;
      LookUpTypeExpr( typeExpr, name,
                      oPos->second.algebraId, oPos->second.typeId );
      nl->WriteToString( oPos->second.typeExpr, nl->OneElemList( typeExpr ) );
      ok = true;
    }
  }
  else if ( !objCatalogFile.SelectRecord( SmiKey( objectName ), oRecord ) )
  {
    ObjectsCatalogEntry objEntry;
    objEntry.state = EntryInsert;
    objEntry.value = valueWord;
    objEntry.valueDefined = defined;
    objEntry.valueRecordId = 0;
    objEntry.typeName = typeName;
    LookUpTypeExpr( typeExpr, name, objEntry.algebraId, objEntry.typeId );
    nl->WriteToString( objEntry.typeExpr, nl->OneElemList( typeExpr ) );
    objects.insert( make_pair( objectName, objEntry ) );
    ok = true;
  }
  return (ok);
}

bool
SecondoCatalog::DeleteObject( const string& objectName )
{
/*
Deletes an object with identifier ~objectName~ in the database calatog and deallocates the 
used memory. Returns error 1 if the object does not exist.

Precondition: dbState = dbOpen.

*/
  bool ok = false;
  string typeName, typecon;
  Word value;
  bool defined, hasNamedType;
  ListExpr typeExpr;

  ObjectsCatalog::iterator oPos = objects.find( objectName );
  if ( oPos != objects.end() )
  {
    if ( oPos->second.state != EntryDelete )
    {
      ok = true;
      if ( oPos->second.state == EntryInsert )
      {
        if ( oPos->second.valueDefined )
        {
          ListExpr typeInfo, typeExpr;
          nl->ReadFromString( oPos->second.typeExpr, typeExpr );
          typeInfo = NumericType( nl->First( typeExpr ) ); 
          (am->DeleteObj( oPos->second.algebraId, oPos->second.typeId ))
            ( typeInfo, oPos->second.value );
          nl->Destroy( typeExpr );
          nl->Destroy( typeInfo );
        }
        objects.erase( oPos );
      }
      else
      {
        oPos->second.state = EntryDelete;
      }
    }
  }
  else if ( GetObjectExpr( objectName, typeName, typeExpr,
                           value, defined, hasNamedType ) )
  {
    ObjectsCatalogEntry oEntry;
    oEntry.state        = EntryDelete;
    oEntry.value        = value;
    oEntry.valueDefined = defined;
    oEntry.typeName     = typeName;
    nl->WriteToString( oEntry.typeExpr, nl->OneElemList( typeExpr ) );
    LookUpTypeExpr( typeExpr, typecon, oEntry.algebraId, oEntry.typeId );
    objects.insert( make_pair( objectName, oEntry ) );
    ok = true;
  }
  return (ok);
}

bool
SecondoCatalog::KillObject( const string& objectName )
{
/*
Kills an object with identifier ~objectName~ in the database calatog and deallocates the used memory. 
Returns false if the object does not exist.

Precondition: dbState = dbOpen.

*/
  bool ok = false;

  ObjectsCatalog::iterator oPos = objects.find( objectName );
  if ( oPos != objects.end() )
  {
    if ( oPos->second.state != EntryDelete )
    {
      ok = true;
      if ( oPos->second.state == EntryInsert )
      {
        objects.erase( oPos );
      }
      else
      {
        oPos->second.state = EntryDelete;
      }
    }
  }
  else if( IsObjectName( objectName ) )
  {
    SmiRecordId valueRecId;
    {
      SmiRecord oRec;
      if( ok = objCatalogFile.SelectRecord( SmiKey( objectName ), oRec ) )
      {
        oRec.Read( &valueRecId, sizeof( SmiRecordId ), CE_OBJS_VALUE_RECID );
      }
    }
 
    if( ok && (ok = objCatalogFile.DeleteRecord( SmiKey( objectName ) )) )
    {
      if( ok && valueRecId != 0 )
      {
        ok = objValueFile.DeleteRecord( valueRecId );
      }
    }
  } 

  return ok;
}

Word
SecondoCatalog::InObject(  const ListExpr typeExpr,
                           const ListExpr valueList,
                           const int errorPos,
                           ListExpr& errorInfo,
                           bool& correct )
{
/*
Converts an object of the type given by ~typeExpr~ and the value given as a nested list into a WORD representation which is returned. Any errors found are returned together with the given ~errorPos~ in the list ~errorInfo~. ~Correct~ is set to TRUE if a value was created (which means that the input was at least partially correct).

Precondition: dbState = dbOpen.

*/
   ListExpr pair, numtype;
   int algebraId, typeId;

   numtype = NumericType( typeExpr );

  if ( nl->IsEmpty( numtype ) )
  {
    correct = false;
    return (SetWord( Address( 0 ) ));
  }
  else
  {
    if ( nl->IsAtom( nl->First( numtype ) ) )
    {
      pair = numtype;
    }
    else
    {
      pair = nl->First( numtype );
    }
    algebraId = nl->IntValue( nl->First( pair ) );
    typeId = nl->IntValue( nl->Second( pair ) );
    return ((am->InObj( algebraId, typeId ))
            ( numtype, valueList, errorPos, errorInfo, correct ));
  }
}

ListExpr
SecondoCatalog::GetObjectValue( const string& objectName )
{
/*
Returns the value of a locally stored database object with identifier ~objectName~ as list expression ~list~ to show the value to the database user. If the value is undefined, an empty list is returned.

Precondition: dbState = dbOpen.

*/
  ListExpr list, typeExpr;
  string typeName;
  Word value;
  bool hasNamedType;
  bool defined;

  if ( testMode && !SmiEnvironment::IsDatabaseOpen() )
  {
    cerr << " GetObjectValue: database is closed!" << endl;
    exit( 0 );
  }
  GetObjectExpr( objectName, typeName, typeExpr, value, defined,
                 hasNamedType );
  if ( defined )
  {
    list = OutObject( typeExpr, value );
  }
  else
  {
    list = nl->TheEmptyList();
  }
  return (list);
}

ListExpr
SecondoCatalog::OutObject( const ListExpr type, const Word object )
{
/*
Returns for a given ~object~ of type ~type~ its value in nested list representation.

*/
  ListExpr pair, numtype;
  int alId, typeId;

  numtype = NumericType( type );
  if ( nl->IsEmpty( numtype ) )
  { // do nothing
    return (nl->TheEmptyList());
  }
  else
  {
    if ( nl->IsAtom( nl->First( numtype ) ) )
    {
      pair = numtype;
    }
    else
    {
      pair = nl->First( numtype );
    }
    alId = nl->IntValue( nl->First( pair ) );
    typeId = nl->IntValue( nl->Second( pair ) );
    return ((am->OutObj( alId, typeId ))( numtype, object ));
  }
}

void
SecondoCatalog::CloseObject( const ListExpr type, Word object )
{
/*
Closes a given ~object~ of type ~type~.

*/
  ListExpr pair, numtype;
  int alId, typeId;

  numtype = NumericType( type );
  if ( nl->IsEmpty( numtype ) )
  { // do nothing
    ;
  }
  else
  {
    if ( nl->IsAtom( nl->First( numtype ) ) )
    {
      pair = numtype;
    }
    else
    {
      pair = nl->First( numtype );
    }
    alId = nl->IntValue( nl->First( pair ) );
    typeId = nl->IntValue( nl->Second( pair ) );
    ( am->CloseObj( alId, typeId ))( numtype, object );
  }
}

bool
SecondoCatalog::IsObjectName( const string& objectName )
{
/*
Checks whether ~objectName~ is a valid object name.

Precondition: dbState = dbOpen.

*/
  bool found = false;

  if ( systemTable(objectName) != 0 )
    return true;
  
  if ( testMode && !SmiEnvironment::IsDatabaseOpen() )
  {
    cerr << " IsObjectName: database is closed!" << endl;
    exit( 0 );
  }
  ObjectsCatalog::iterator oPos = objects.find( objectName );
  if ( oPos != objects.end() )
  {
    found = (oPos->second.state != EntryDelete);
  }
  else
  {
    SmiRecord oRecord;
    found = objCatalogFile.SelectRecord( SmiKey( objectName ), oRecord );
  }
  return (found);
}

bool
SecondoCatalog::GetObject( const string& objectName,
                           Word& value, bool& defined )
{
/*
Returns the value ~value~ of an object with identifier ~objectName~. ~defined~ tells whether the word contains a meaningful value.

Precondition: ~IsObjectName(objectName)~ delivers TRUE.

*/
 
  const SystemInfoRel* table = systemTable(objectName);
  if (  table != 0 )
  { 
    value = createRelation(objectName);
    defined = true;
    return true;
  }  
    
 
 
  if ( testMode && !SmiEnvironment::IsDatabaseOpen() )
  {
    cerr << " GetObject: database is closed!" << endl;
    exit( 0 );
  }
  ObjectsCatalog::iterator oPos = objects.find( objectName );
  if ( oPos != objects.end() )
  {
    if ( oPos->second.state != EntryDelete )
    {
      value  = oPos->second.value;
      defined = oPos->second.valueDefined;
    }
    else
    {
      value.addr = 0;
      defined    = false;
    }
  }
  else
  {
    SmiRecord oRec;
    if ( objCatalogFile.SelectRecord( SmiKey( objectName ), oRec ) )
    {
      SmiRecordId valueRecId;
      oRec.Read( &value, sizeof( Word ), CE_OBJS_VALUE );
      oRec.Read( &defined, sizeof( bool ), CE_OBJS_VALUE_DEF );
      if ( defined )
      {
        oRec.Read( &valueRecId, sizeof( SmiRecordId ), CE_OBJS_VALUE_RECID );
        int algebraId, typeId;
        oRec.Read( &algebraId, sizeof( int ), CE_OBJS_ALGEBRA_ID );
        oRec.Read( &typeId, sizeof( int ), CE_OBJS_TYPE_ID );
        int nameSize, exprSize;
        oRec.Read( &nameSize, sizeof( int ), CE_OBJS_TYPENAME_SIZE );
        oRec.Read( &exprSize, sizeof( int ), CE_OBJS_TYPEEXPR_SIZE );
        char* buffer = new char[exprSize];
        oRec.Read( buffer, exprSize, CE_OBJS_TYPEINFO_START + nameSize );
        string typeExprString;
        typeExprString.assign( buffer, exprSize );
        delete []buffer;
        SmiRecord vRec;
        size_t offset = 0;
        if ( objValueFile.SelectRecord( valueRecId, vRec ) )
        {
          ListExpr typeExpr, typeInfo;
          nl->ReadFromString( typeExprString, typeExpr );
          typeInfo = NumericType( nl->First( typeExpr ) );
          am->OpenObj( algebraId, typeId, vRec, offset, typeInfo, value );
          nl->Destroy( typeInfo );
          nl->Destroy( typeExpr );
        }
      }
      else
      {
        value.addr = 0;
      }
    }
    else
    {
      value.addr = 0;
      defined    = false;
    }
  }
  return (defined);
}

bool
SecondoCatalog::GetObjectExpr( const string& objectName,
                               string& typeName,
                               ListExpr& typeExpr,
                               Word& value,
                               bool& defined,
                               bool& hasTypeName )
{
/*
Returns the value ~value~, the type name ~typeName~, and the type expression ~typeExpr~, 
of an object with identifier ~objectName~. ~defined~ tells whether ~value~ contains a 
defined value. If object has no type name the variable  ~hasTypeName~ is set to FALSE 
and the procedure returns an empty string as ~typeName~.

Precondition: ~IsObjectName(objectName)~ delivers TRUE.

*/
  bool ok = false;

  const SystemInfoRel* table = systemTable(objectName);
  if (  table != 0 )
  { 
    value = createRelation(objectName);
    typeExpr = table->relSchema().listExpr();
    typeName= "rel";
    hasTypeName = true;
    defined = true;
    return true;
  }  

  
  if ( testMode && !SmiEnvironment::IsDatabaseOpen() )
  {
    cerr << " GetObjectExpr: database is closed!" << endl;
    return (false);
  }
  ObjectsCatalog::iterator oPos = objects.find( objectName );
  if ( oPos != objects.end() )
  {
    if ( oPos->second.state != EntryDelete )
    {
      typeName    = oPos->second.typeName;
      nl->ReadFromString( oPos->second.typeExpr, typeExpr );
      typeExpr = nl->First( typeExpr );
      value       = oPos->second.value;
      defined     = oPos->second.valueDefined;
      hasTypeName = (typeName != "");
      ok = true;
    }
    else
    {
      typeName    = "";
      typeExpr    = nl->TheEmptyList();
      value.addr  = 0;
      defined     = false;
      hasTypeName = false;
    }
  }
  else
  {
    SmiRecord oRec;
    if ( objCatalogFile.SelectRecord( SmiKey( objectName ), oRec ) )
    {
      SmiRecordId valueRecId;
      oRec.Read( &value, sizeof( Word ), CE_OBJS_VALUE );
      oRec.Read( &defined, sizeof( bool ), CE_OBJS_VALUE_DEF );
      oRec.Read( &valueRecId, sizeof( SmiRecordId ), CE_OBJS_VALUE_RECID );
      int algebraId, typeId;
      oRec.Read( &algebraId, sizeof( int ), CE_OBJS_ALGEBRA_ID );
      oRec.Read( &typeId, sizeof( int ), CE_OBJS_TYPE_ID );
      int nameSize, exprSize, bufSize;
      oRec.Read( &nameSize, sizeof( int ), CE_OBJS_TYPENAME_SIZE );
      oRec.Read( &exprSize, sizeof( int ), CE_OBJS_TYPEEXPR_SIZE );
      bufSize = (nameSize > exprSize) ? nameSize : exprSize;
      char* buffer = new char[bufSize];
      oRec.Read( buffer, nameSize, CE_OBJS_TYPEINFO_START );
      typeName.assign( buffer, nameSize );
      oRec.Read( buffer, exprSize, CE_OBJS_TYPEINFO_START + nameSize );
      string typeExprString;
      typeExprString.assign( buffer, exprSize );
      delete []buffer;
      nl->ReadFromString( typeExprString, typeExpr );
      typeExpr = nl->First( typeExpr );
      hasTypeName = (typeName != "");
      if ( defined )
      {
        SmiRecord vRec;
        size_t offset = 0;
        if ( objValueFile.SelectRecord( valueRecId, vRec ) )
        {
          ListExpr typeInfo = NumericType( typeExpr );
          am->OpenObj( algebraId, typeId, vRec, offset, typeInfo, value );
          nl->Destroy( typeInfo );
          nl->Destroy( typeExpr );
        }
      }
      else
      {
        value.addr = 0;
      }
      ok = true;
    }
    else
    {
      typeName    = "";
      typeExpr    = nl->TheEmptyList();
      value.addr  = 0;
      defined     = false;
      hasTypeName = false;
    }
  }
  return (ok);
}

bool
SecondoCatalog::GetObjectType( const string& objectName, string& typeName )
{
/*
Returns the type name ~typeName~ of an object with identifier ~objectName~, if the type 
name exists and an empty string otherwise.

Precondition: ~IsObjectName(objectName)~ delivers TRUE.

*/
  if ( testMode && !SmiEnvironment::IsDatabaseOpen() )
  {
    cerr << " GetObjectType: database is closed!" << endl;
    exit( 0 );
  }
  ObjectsCatalog::iterator oPos = objects.find( objectName );
  if ( oPos != objects.end() )
  {
    if ( oPos->second.state != EntryDelete )
    {
      typeName  = oPos->second.typeName;
    }
    else
    {
      typeName  = "";
    }
  }
  else
  {
    SmiRecord oRec;
    if ( objCatalogFile.SelectRecord( SmiKey( objectName ), oRec ) )
    {
      int nameSize;
      oRec.Read( &nameSize, sizeof( int ), CE_OBJS_TYPENAME_SIZE );
      char* buffer = new char[nameSize];
      oRec.Read( buffer, nameSize, CE_OBJS_TYPEINFO_START );
      typeName.assign( buffer, nameSize );
      delete []buffer;
    }
    else
    {
      typeName  = "";
    }
  }
  return (typeName == "");
}

ListExpr
SecondoCatalog::GetObjectTypeExpr( const string& objectName )
{
/*

Returns the type expression of an object with identifier ~objectName~.

*Precondition*: "IsObjectName( objectName ) == true"[4].

*/
  ListExpr typeExpr = nl->Empty();

  const SystemInfoRel* table = systemTable(objectName);
  if (  table != 0 )
  {
    return table->relSchema().listExpr();
  } 
   
  if ( testMode && !SmiEnvironment::IsDatabaseOpen() )
  {
    cerr << " GetObjectTypeExpr: database is closed!" << endl;
    return (nl->TheEmptyList());
  }
  ObjectsCatalog::iterator oPos = objects.find( objectName );
  if ( oPos != objects.end() )
  {
    if ( oPos->second.state != EntryDelete )
    {
      nl->ReadFromString( oPos->second.typeExpr, typeExpr );
      typeExpr = nl->First( typeExpr );
    }
    else
    {
      typeExpr    = nl->TheEmptyList();
    }
  }
  else
  {
    SmiRecord oRec;
    if ( objCatalogFile.SelectRecord( SmiKey( objectName ), oRec ) )
    {
      int nameSize, exprSize, bufSize;
      string typeName;

      oRec.Read( &nameSize, sizeof( int ), CE_OBJS_TYPENAME_SIZE );
      oRec.Read( &exprSize, sizeof( int ), CE_OBJS_TYPEEXPR_SIZE );
      bufSize = (nameSize > exprSize) ? nameSize : exprSize;
      char* buffer = new char[bufSize];
      oRec.Read( buffer, nameSize, CE_OBJS_TYPEINFO_START );
      typeName.assign( buffer, nameSize );
      oRec.Read( buffer, exprSize, CE_OBJS_TYPEINFO_START + nameSize );
      string typeExprString;
      typeExprString.assign( buffer, exprSize );
      delete []buffer;
      nl->ReadFromString( typeExprString, typeExpr );
      typeExpr = nl->First( typeExpr );
    }
    else
    {
      typeExpr    = nl->TheEmptyList();
    }
  }
  return typeExpr;
}

bool
SecondoCatalog::UpdateObject( const string& objectName, const Word value )
{
/*
Overwrites the value of the object with identifier ~objectName~ with a
new value ~value~. Returns error 1 if object does not exist.

*/
  bool found = false;
  ObjectsCatalog::iterator oPos = objects.find( objectName );
  if ( oPos != objects.end() )
  {
    if ( oPos->second.state != EntryDelete )
    {
      if( oPos->second.valueDefined )
      {
        ObjectDeletion del = am->DeleteObj( oPos->second.algebraId, 
	                                    oPos->second.typeId     );
        ListExpr typeExpr, typeInfo;
        nl->ReadFromString( oPos->second.typeExpr, typeExpr );
        typeInfo = NumericType( nl->First( typeExpr ) );
        del( typeInfo, oPos->second.value );
        nl->Destroy( typeInfo );
        nl->Destroy( typeExpr );
      }
      oPos->second.value = value;
      oPos->second.valueDefined = true;
      found = true;
    }
  }
  else
  {
    ObjectsCatalogEntry oEntry;
    SmiRecord oRec;
    found = objCatalogFile.SelectRecord( SmiKey( objectName ), oRec );
    if ( found )
    {
      oRec.Read( &oEntry.valueDefined, sizeof( bool ), CE_OBJS_VALUE_DEF );
      oRec.Read( &oEntry.valueRecordId, 
	         sizeof( SmiRecordId ), CE_OBJS_VALUE_RECID );
      oRec.Read( &oEntry.algebraId, sizeof( int ), CE_OBJS_ALGEBRA_ID );
      oRec.Read( &oEntry.typeId, sizeof( int ), CE_OBJS_TYPE_ID );
      int nameSize, exprSize, bufSize;
      oRec.Read( &nameSize, sizeof( int ), CE_OBJS_TYPENAME_SIZE );
      oRec.Read( &exprSize, sizeof( int ), CE_OBJS_TYPEEXPR_SIZE );
      bufSize = (nameSize > exprSize) ? nameSize : exprSize;
      char* buffer = new char[bufSize];
      oRec.Read( buffer, nameSize, CE_OBJS_TYPEINFO_START );
      oEntry.typeName.assign( buffer, nameSize );
      oRec.Read( buffer, exprSize, CE_OBJS_TYPEINFO_START + nameSize );
      oEntry.typeExpr.assign( buffer, exprSize );
      delete []buffer;
      oEntry.state = EntryUpdate;
      if( oEntry.valueDefined )
      {
        SmiRecord vRec;
        size_t offset = 0;
        if ( objValueFile.SelectRecord( oEntry.valueRecordId, vRec ) )
        {
          Word oldvalue;
          ListExpr typeExpr, typeInfo;
          ObjectDeletion del = am->DeleteObj( oEntry.algebraId, 
	                                      oEntry.typeId     );

          nl->ReadFromString( oEntry.typeExpr, typeExpr );
          typeInfo = NumericType( nl->First( typeExpr ) );

          if( am->OpenObj( oEntry.algebraId, 
	                   oEntry.typeId, vRec, offset, typeInfo, oldvalue ) )
            del( typeInfo, oldvalue );

          nl->Destroy( typeInfo );
          nl->Destroy( typeExpr );
        }
      }
      oEntry.value = value;
      oEntry.valueDefined = true;
      objects.insert( make_pair( objectName, oEntry ) );
    }
  }
  return (found);
}

bool
SecondoCatalog::ModifyObject( const string& objectName, const Word value )
{
/*
Overwrites the value of the object with identifier ~objectName~ with a
new value ~word~. Returns "false"[4] if object does not exist.
The difference between this function and ~UpdateObject~ is that the
second opens the old object for deletion. This one assumes that the
object is only modified, so that no deletion function is necessary.

*/
  bool found = false;
  ObjectsCatalog::iterator oPos = objects.find( objectName );
  if ( oPos != objects.end() )
  {
    assert( oPos->second.state != EntryDelete );

    if( oPos->second.value.addr != value.addr )
    {
      ListExpr typeExpr, typeInfo;
      nl->ReadFromString( oPos->second.typeExpr, typeExpr );
      typeInfo = NumericType( nl->First( typeExpr ) );
      (am->CloseObj( oPos->second.algebraId, oPos->second.typeId ))
        ( typeInfo, oPos->second.value );
      nl->Destroy( typeInfo );
      nl->Destroy( typeExpr );
    }
    oPos->second.value = value;
    oPos->second.valueDefined = true;
    found = true;
  }
  else
  {
    ObjectsCatalogEntry oEntry;
    SmiRecord oRec;
    found = objCatalogFile.SelectRecord( SmiKey( objectName ), oRec );
    if ( found )
    {
      oRec.Read( &oEntry.valueDefined, 
	         sizeof( bool ), CE_OBJS_VALUE_DEF );
      oRec.Read( &oEntry.valueRecordId, 
	         sizeof( SmiRecordId ), CE_OBJS_VALUE_RECID );
      oRec.Read( &oEntry.algebraId, sizeof( int ), CE_OBJS_ALGEBRA_ID );
      oRec.Read( &oEntry.typeId, sizeof( int ), CE_OBJS_TYPE_ID );
      int nameSize, exprSize, bufSize;
      oRec.Read( &nameSize, sizeof( int ), CE_OBJS_TYPENAME_SIZE );
      oRec.Read( &exprSize, sizeof( int ), CE_OBJS_TYPEEXPR_SIZE );
      bufSize = (nameSize > exprSize) ? nameSize : exprSize;
      char* buffer = new char[bufSize];
      oRec.Read( buffer, nameSize, CE_OBJS_TYPEINFO_START );
      oEntry.typeName.assign( buffer, nameSize );
      oRec.Read( buffer, exprSize, CE_OBJS_TYPEINFO_START + nameSize );
      oEntry.typeExpr.assign( buffer, exprSize );
      delete []buffer;
      oEntry.state = EntryUpdate;
      oEntry.value = value;
      oEntry.valueDefined = true;
      objects.insert( make_pair( objectName, oEntry ) );
    }
  }
  return (found);
}


bool
SecondoCatalog::CloneObject( const string& objectName, const Word value )
{
/*
Overwrites the value of the object with identifier ~objectName~ with a
new value cloned from ~value~. Returns error 1 if object does not exist.

*/
  bool found = false;
  ObjectsCatalog::iterator oPos = objects.find( objectName );
  if ( oPos != objects.end() )
  {
    if ( oPos->second.state != EntryDelete )
    {
      ListExpr typeExpr, typeInfo;
      nl->ReadFromString( oPos->second.typeExpr, typeExpr );
      typeInfo = NumericType( nl->First( typeExpr ) );
      if( oPos->second.valueDefined )
      {
        ObjectDeletion del = am->DeleteObj( oPos->second.algebraId, 
	                                    oPos->second.typeId     );
        del( typeInfo, oPos->second.value );
      }
      oPos->second.value = (am->CloneObj( oPos->second.algebraId, 
	                                  oPos->second.typeId     ))
        ( typeInfo, value );
      oPos->second.valueDefined = true;
      found = true;
      nl->Destroy( typeInfo );
      nl->Destroy( typeExpr );
    }
  }
  else
  {
    ObjectsCatalogEntry oEntry;
    SmiRecord oRec;
    found = objCatalogFile.SelectRecord( SmiKey( objectName ), oRec );
    if ( found )
    {
      oRec.Read( &oEntry.valueDefined, sizeof( bool ), CE_OBJS_VALUE_DEF );
      oRec.Read( &oEntry.valueRecordId, 
	         sizeof( SmiRecordId ), CE_OBJS_VALUE_RECID );
      oRec.Read( &oEntry.algebraId, sizeof( int ), CE_OBJS_ALGEBRA_ID );
      oRec.Read( &oEntry.typeId, sizeof( int ), CE_OBJS_TYPE_ID );
      int nameSize, exprSize, bufSize;
      oRec.Read( &nameSize, sizeof( int ), CE_OBJS_TYPENAME_SIZE );
      oRec.Read( &exprSize, sizeof( int ), CE_OBJS_TYPEEXPR_SIZE );
      bufSize = (nameSize > exprSize) ? nameSize : exprSize;
      char* buffer = new char[bufSize];
      oRec.Read( buffer, nameSize, CE_OBJS_TYPEINFO_START );
      oEntry.typeName.assign( buffer, nameSize );
      oRec.Read( buffer, exprSize, CE_OBJS_TYPEINFO_START + nameSize );
      oEntry.typeExpr.assign( buffer, exprSize );
      delete []buffer;
      oEntry.state = EntryUpdate;

      ListExpr typeExpr, typeInfo;
      nl->ReadFromString( oEntry.typeExpr, typeExpr );
      typeInfo = NumericType( nl->First( typeExpr ) );

      if( oEntry.valueDefined )
      {
        SmiRecord vRec;
        size_t offset = 0;
        if ( objValueFile.SelectRecord( oEntry.valueRecordId, vRec ) )
        {
          Word oldvalue;
          ObjectDeletion del = am->DeleteObj( oEntry.algebraId, 
	                                      oEntry.typeId     );

          if( am->OpenObj( oEntry.algebraId, 
	                   oEntry.typeId, vRec, offset, typeInfo, oldvalue ) )
            del( typeInfo, oldvalue );
        }
      }
      ObjectClone clone = am->CloneObj( oEntry.algebraId, oEntry.typeId );
      oEntry.value = clone( typeInfo, value );
      oEntry.valueDefined = true;
      objects.insert( make_pair( objectName, oEntry ) );
      nl->Destroy( typeInfo );
      nl->Destroy( typeExpr );
    }
  }
  return (found);
}

/************************************************************************
3.1.2 Algebra Type Constructors

*/

ListExpr
SecondoCatalog::ListTypeConstructors()
{
/*
Returns a list of type constructors ~typecons~ of the actually load algebras in the following format:

---- (
      (<type constructor name> (<arg 1>..<arg n>) <result>) *
     )
----

*/
  LocalConstructorCatalog::iterator pos;
  ListExpr last = 0, list;
  ListExpr tcList = nl->TheEmptyList();

  for ( pos = constructors.begin(); pos != constructors.end(); pos++ )
  {
    list = am->Props( pos->second.algebraId, pos->second.entryId );
    if ( tcList == nl->TheEmptyList() )
    {
      tcList = nl->Cons( nl->Cons( nl->SymbolAtom( pos->first ), list ),
                         nl->TheEmptyList() );
      last = tcList;
    }
    else
    {
      last = nl->Append( last,
                         nl->Cons( nl->SymbolAtom( pos->first ), list ) );
    }
  }
  return (tcList);
}


void
SecondoCatalog::Initialize(TypeInfoRel* r)
{
  LocalConstructorCatalog::iterator pos = constructors.begin();
  
  while ( pos != constructors.end() )
  {
    //cout << pos->first << endl;
    int algId = pos->second.algebraId;
    int typeId = pos->second.entryId;
    NList typeInfo( am->Props( algId, typeId ));
    typeInfo = typeInfo.second();
    int size = (*(am->SizeOfObj( algId, typeId )))();
    //cout << "  " << typeInfo << " : " << size << endl;
    
    TypeInfoTuple& t = *(new TypeInfoTuple());
    t.type = pos->first;
    t.size = size;
    t.algebra = am->GetAlgebraName(algId);
    if (typeInfo.length() >= 1)
    t.signature = typeInfo.elem(1).str();
    if (typeInfo.length() >= 2)
    t.typeListExample = typeInfo.elem(2).str();
    if (typeInfo.length() >= 3)
    t.listRep = typeInfo.elem(3).str();
    if (typeInfo.length() >= 4)
    t.valueListExample = typeInfo.elem(4).str();
    if (typeInfo.length() >= 5)
      t.remark = typeInfo.elem(5).str();
    r->append(&t, false);
    pos++;
  }
} 

void
SecondoCatalog::Initialize(OperatorInfoRel* r)
{
  LocalOperatorCatalog::iterator pos = operators.begin();

  while ( pos != operators.end())
  {
    CatalogEntrySet* entrySet = pos->second;
    CatalogEntrySet::iterator i = entrySet->begin();
    
    const int algId = i->algebraId;
    const string algName =  am->GetAlgebraName(algId);
    ExampleReader examples("tmp/"+algName+".examples");
    while (  i != entrySet->end())
    {
      NList list( am->Specs( algId, i->entryId ));
      assert( list.hasLength(2) );
      list = list.second();
      //cout << pos->first << ":  " << endl 
      //     << list << endl << endl;
      
      OperatorInfoTuple& t = *(new OperatorInfoTuple());
      t.name = pos->first;
      t.algebra = algName;
      if (list.length() >= 1)
      t.signature = list.elem(1).str();
      if (list.length() >= 2)
      t.syntax = list.elem(2).str();
      if (list.length() >= 3)
      t.meaning = list.elem(3).str();
      if (list.length() >= 4)
      t.example = list.elem(4).str();
      if (list.length() >= 5)
        t.remark = list.elem(5).str();
      r->append(&t, false);

      // copy to Example Info
      ExampleInfo ex;
      ex.opName = t.name;
      ex.number = 1; 
      ex.signature = t.signature;
      ex.example = t.example;
      
      examples.add(t.name, 1, ex);
      i++;
    }  
    examples.write();
    pos++;
  }
}


ListExpr
SecondoCatalog::ListTypeConstructors( int algebraId )
{
  int k;
  ListExpr last = 0, list;
  ListExpr tcList = nl->TheEmptyList();
  string tcname;

  for ( k = 0; k < am->ConstrNumber( algebraId ); k++ )
  {
    tcname = am->Constrs( algebraId, k );
    list = am->Props( algebraId, k );
    if ( tcList == nl->TheEmptyList() )
    {
      tcList = nl->Cons( nl->Cons( nl->SymbolAtom( tcname ), list ),
                         nl->TheEmptyList() );
      last = tcList;
    }
    else
    {
      last = nl->Append( last,
                         nl->Cons( nl->SymbolAtom( tcname ), list ) );
    }
  }
  return (tcList);
}

bool
SecondoCatalog::IsTypeName( const string& typeName )
{
/*
Checks whether ~typeName~ is a valid name for an algebra type constructor
or a database type.

*/
  return (constructors.find( typeName ) != constructors.end() ||
          (SmiEnvironment::IsDatabaseOpen() && MemberType( typeName )));
}

bool
SecondoCatalog::GetTypeId( const string& typeName,
                           int& algebraId, int& typeId )
{
/*
Returns the algebra identifier ~algebraId~ and the type identifier
~typeId~ of an existing type constructor or database type with name
~typeName~.

Precondition: ~IsTypeName(typeName)~ delivers TRUE.

*/
  bool found = false;

  if ( testMode && !IsTypeName( typeName ) )
  {
    cerr << "  GetTypeId: " << typeName 
         << " is not a valid type name!" << endl;
    exit( 0 );
  }

  LocalConstructorCatalog::iterator pos = constructors.find( typeName );
  if ( pos != constructors.end() )
  {
    algebraId = pos->second.algebraId;
    typeId    = pos->second.entryId;
    found     = true;
  }
  else if ( SmiEnvironment::IsDatabaseOpen() )
  {
    TypesCatalog::iterator tPos = types.find( typeName );
    if ( tPos != types.end() )
    {
      algebraId = tPos->second.algebraId;
      typeId    = tPos->second.typeId;
      found     = true;
    }
    else
    {
      SmiRecord tRec;
      found = typeCatalogFile.SelectRecord( SmiKey( typeName ), tRec );
      if ( found )
      {
        tRec.Read( &algebraId, sizeof( int ), CE_TYPES_ALGEBRA_ID );
        tRec.Read( &typeId, sizeof( int ), CE_TYPES_TYPE_ID );
        found = true;
      }
    }
  }
  if ( !found )
  {
    algebraId = 0;
    typeId    = 0;
  }
  return (found);
}

string
SecondoCatalog::GetTypeName( const int algebraId, const int typeId )
{
/*
Looks up the ~typeName~ of a type constructor defined by the algebra identifier ~algebraId~ and the type identifier ~opId~.

*/

  if ( testMode )
  {
    if ( am->IsAlgebraLoaded( algebraId ) ||
        (typeId >= am->ConstrNumber( algebraId )) )
    {
      cerr << "  GetTypeName: No valid type name exists!" << endl;
      exit( 0 );
    }
  }
  return (am->Constrs( algebraId, typeId ));
}

ListExpr
SecondoCatalog::GetTypeDS( const int algebraId, const int typeId )
{
/*
Looks up the properties ~props~ of a type constructor defined by the
algebra identifier ~algebraId~ and the type identifier ~opId~.

*/
  return (am->Props( algebraId, typeId ));
}

/************************************************************************
3.1.2 Algebra Operators

*/

bool
SecondoCatalog::IsOperatorName( const string& opName )
{
/*
Checks whether ~opName~ is a valid operator name.

*/
  return (operators.find( opName ) != operators.end());
}

ListExpr
SecondoCatalog::GetOperatorIds( const string& opName )
{
/*
Returns the algebra identifier ~algebraId~ and the operator identifier
~opId~ of an existing ~opName~.

Precondition: ~IsOperatorName( opName)~ delivers TRUE.

*/
  ListExpr opList;
  LocalOperatorCatalog::iterator pos = operators.find( opName );

  if (  pos != operators.end() )
  {
    CatalogEntrySet *operatorSet = pos->second;
    CatalogEntrySet::iterator operatorSetIterator = operatorSet->begin();

    opList = nl->OneElemList(
               nl->TwoElemList( 
		 nl->IntAtom( operatorSetIterator->algebraId ), 
		   nl->IntAtom( operatorSetIterator->entryId ) ) );
    ListExpr last = opList;

    while ( ++operatorSetIterator != operatorSet->end() )
    {
      last = nl->Append( last,
               nl->TwoElemList( 
		 nl->IntAtom( operatorSetIterator->algebraId ), 
		    nl->IntAtom( operatorSetIterator->entryId ) ) );
    }
    return opList;
  }
  else
  {
    cerr << "  GetOperatorIds: " << opName 
         << " is not a valid operator name!" << endl;
    exit( 0 );
  }
}

string
SecondoCatalog::GetOperatorName( const int algebraId, const int opId )
{
/*
Looks for the name of an operator defined by the algebra identifier ~algebraId~
and the operator identifier ~opId~.

*/
  if ( testMode )
  {
    if ( am->IsAlgebraLoaded( algebraId ) ||
        (opId >= am->OperatorNumber( algebraId )) )
    {
      cerr << "   GetOperatorName: No valid operator name exists!" << endl;
      exit( 0 );
    }
  }
  return (am->Ops( algebraId, opId ));
}

ListExpr
SecondoCatalog::GetOperatorSpec( const int algebraId, const int opId )
{
/*
Returns the operator specification ~specs~ of an operator defined by the algebra identifier ~algebraId~ and the operator identifier ~opId~ in the following format:

----
  ( <operator name>
    (<arg type spec 1>..<arg type spec n>)
    <result type spec>
    <syntax>
    <variable defs>
    <formula>
    <explaining text>
  )
----

*/
  if ( testMode )
  {
    if ( am->IsAlgebraLoaded( algebraId ) ||
        (opId >= am->OperatorNumber( algebraId )) )
    {
      cerr << " GetOperatorSpec : No valid operator name exists!" << endl;
      exit( 0 );
    }
  }

  return (am->Specs( algebraId, opId ));
}

ListExpr
SecondoCatalog::ListOperators()
{
/*
The list format for a single operator is (opname (a)(b)). 
Where (a) and (b) are flat list of string or text atoms. This is the 
result returned  by NestedList::ReadFromString of the operator's 
specification string.

Note: The list structure below is not implemented! This seems to be an
old attempt for describing an operator which is not pratical.

Returns a list of operators specifications in the following format:

----
(
  ( <operator name>
    (<arg type spec 1>..<arg type spec n>)
    <result type spec>
    <syntax>
    <variable defs>
    <formula>
    <explaining text>
  )*
)
----
This format is based on the formal definition of the syntax of operator
specifications from [BeG95b, Section3.1].

*/
  LocalOperatorCatalog::iterator pos;
  ListExpr last = 0, list;
  ListExpr opList = nl->TheEmptyList();

  for ( pos = operators.begin(); pos != operators.end(); pos++ )
  {
    CatalogEntrySet* entrySet = pos->second;
    CatalogEntrySet::iterator i;

    for( i = entrySet->begin(); i != entrySet->end(); i++ )
    {
      list = am->Specs( i->algebraId, i->entryId );
      assert( nl->HasLength(list,2) );
      if ( opList == nl->TheEmptyList() )
      {
        opList = nl->Cons( nl->Cons( nl->SymbolAtom( pos->first ), list ),
                           nl->TheEmptyList() );
        last = opList;
      }
      else
      {
        last = nl->Append( last,
                           nl->Cons( nl->SymbolAtom( pos->first ), list ) );
      }
    }
  }
  return (opList);
}

ListExpr
SecondoCatalog::ListOperators( int algebraId )
{
  int k;
  ListExpr last = 0, list;
  ListExpr opList = nl->TheEmptyList();
  string opname;

  for ( k = 0; k < am->OperatorNumber( algebraId ); k++ )
  {
    opname = am->Ops( algebraId, k );
    list = am->Specs( algebraId, k );
    if ( opList == nl->TheEmptyList() )
    {
      opList = nl->Cons( nl->Cons( nl->SymbolAtom( opname ), list ),
                         nl->TheEmptyList() );
      last = opList;
    }
    else
    {
      last = nl->Append( last,
                         nl->Cons( nl->SymbolAtom( opname ), list ) );
    }
  }
  return (opList);
}

/*
3.4 Initialization of Values and Test Procedures

*/

SecondoCatalog::SecondoCatalog()
  : typeCatalogFile( SmiKey::String ), objCatalogFile( SmiKey::String ),
    objValueFile( false ), testMode( false )
{
  nl = SecondoSystem::GetNestedList();
  am = SecondoSystem::GetAlgebraManager();

  CatalogEntry newEntry;
  int algebraId = 0;

  while ( am->NextAlgebraId( algebraId ) )
  {
    newEntry.algebraId = algebraId;
/*
Defines a dictionary for algebra type constructors.

*/
    int j;
    for ( j = 0; j < am->ConstrNumber( algebraId ); j++ )
    {
      newEntry.entryId = j;
      constructors.insert( make_pair( 
	                          am->Constrs( algebraId, j ), newEntry ) );
    }
/*
Defines a dictionary for algebra operators.

*/
    for ( j = 0; j < am->OperatorNumber( algebraId ); j++ )
    {
      newEntry.entryId = j;
      LocalOperatorCatalog::iterator 
        pos = operators.find( am->Ops( algebraId, j ) );
      CatalogEntrySet* entrySet;

      if (  pos != operators.end() )
      {
        entrySet = pos->second;
        entrySet->push_back( newEntry );
      }
      else
      {
        entrySet = new CatalogEntrySet();
        entrySet->push_back( newEntry );
        operators.insert( make_pair( am->Ops( algebraId, j ), entrySet ) );
      }
    }
  }
}

SecondoCatalog::~SecondoCatalog()
{
  constructors.clear();

  for( LocalOperatorCatalog::iterator i = operators.begin();
       i != operators.end();
       i++ )
    delete i->second;
  operators.clear();

  types.clear();
  objects.clear();
}

bool
SecondoCatalog::Open()
{
  bool ok = true;
  ok = ok && typeCatalogFile.Open( "Types", "SecondoCatalog" );
  ok = ok && objCatalogFile.Open( "Objects", "SecondoCatalog" );
  ok = ok && objValueFile.Open( "ObjValues", "SecondoCatalog" );
  
  if ( !ok )
  {
    Close();
  }
  return (ok);
}

bool
SecondoCatalog::Close()
{
  bool ok = true;
  if ( typeCatalogFile.IsOpen() )
  {
    ok = ok && typeCatalogFile.Close();
  }
  if ( objCatalogFile.IsOpen() )
  {
    ok = ok && objCatalogFile.Close();
  }
  if ( objValueFile.IsOpen() )
  {
    ok = ok && objValueFile.Close();
  }
  types.clear();
  objects.clear();
  return (ok);
}

bool
SecondoCatalog::CleanUp( const bool revert )
{
  bool ok = true;
  if ( !revert )
  {
    SmiRecord tRec;
    for ( TypesCatalog::iterator tPos = types.begin();
          tPos != types.end(); 
	  tPos++ )
    {
      switch (tPos->second.state)
      {
        case EntryInsert:
        {
          if ( typeCatalogFile.InsertRecord( SmiKey( tPos->first ), tRec ) )
          {
            tRec.Write( &tPos->second.algebraId, 
	                sizeof( int ), CE_TYPES_ALGEBRA_ID );
            tRec.Write( &tPos->second.typeId, 
	                sizeof( int ), CE_TYPES_TYPE_ID );
            int exprSize = tPos->second.typeExpr.length();
            tRec.Write( &exprSize, sizeof( int ), CE_TYPES_EXPR_SIZE );
            tRec.Write( tPos->second.typeExpr.data(), 
	                exprSize, CE_TYPES_EXPR_START );
          }
          else
          {
            ok = false;
          }
          break;
        }
        case EntryUpdate:
        {
          if ( typeCatalogFile.SelectRecord( SmiKey( tPos->first ), 
	                                     tRec, SmiFile::Update ) )
          {
            tRec.Write( &tPos->second.algebraId, 
	                sizeof( int ), CE_TYPES_ALGEBRA_ID );
            tRec.Write( &tPos->second.typeId, 
	                sizeof( int ), CE_TYPES_TYPE_ID );
            int exprSize = tPos->second.typeExpr.length();
            tRec.Write( &exprSize, sizeof( int ), CE_TYPES_EXPR_SIZE );
            tRec.Write( tPos->second.typeExpr.data(), 
	                exprSize, CE_TYPES_EXPR_START );
          }
          else
          {
            ok = false;
          }
          break;
        }
        case EntryDelete:
        {
          ok = typeCatalogFile.DeleteRecord( SmiKey( tPos->first ) );
          break;
        }
      }
    }
  }

/*
In this first iteration:

 * Objects are created and updated with undefined values.

 * In the deletion, only the catalog part of the object is deleted.

*/
  for ( ObjectsCatalog::iterator oPos = objects.begin(); 
        oPos != objects.end(); 
	oPos++ )
  {
    switch (oPos->second.state)
    {
      case EntryInsert:
      {
        if ( !revert )
        {
          SmiRecord oRec;
          if ( objCatalogFile.InsertRecord( SmiKey( oPos->first ), oRec ) )
          {
            bool f = false;
            oRec.Write( &oPos->second.value, 
	                sizeof( Word ), CE_OBJS_VALUE );
            oRec.Write( &f, sizeof( bool ), 
	                CE_OBJS_VALUE_DEF );
            oRec.Write( &oPos->second.algebraId, 
	                sizeof( int ), CE_OBJS_ALGEBRA_ID );
            oRec.Write( &oPos->second.typeId, 
	                sizeof( int ), CE_OBJS_TYPE_ID );
            int nameSize = oPos->second.typeName.length();
            int exprSize = oPos->second.typeExpr.length();
            oRec.Write( &nameSize, sizeof( int ), CE_OBJS_TYPENAME_SIZE );
            oRec.Write( &exprSize, sizeof( int ), CE_OBJS_TYPEEXPR_SIZE );
            oRec.Write( oPos->second.typeName.data(), 
	                nameSize, CE_OBJS_TYPEINFO_START );
            oRec.Write( oPos->second.typeExpr.data(), 
	                exprSize, CE_OBJS_TYPEINFO_START + nameSize );
            oRec.Finish();
          }
          else
          {
            ok = false;
          }
        }
        break;
      }
      case EntryUpdate:
      {
        if ( !revert )
        {
          SmiRecord oRec;
          if ( objCatalogFile.SelectRecord( SmiKey( oPos->first ), 
	                                    oRec, SmiFile::Update ) )
          {
            bool f = false;
            oRec.Write( &oPos->second.value, sizeof( int ), CE_OBJS_VALUE );
            oRec.Write( &f, sizeof( bool ), CE_OBJS_VALUE_DEF );
            oRec.Write( &oPos->second.algebraId, 
	                sizeof( int ), CE_OBJS_ALGEBRA_ID );
            oRec.Write( &oPos->second.typeId, sizeof( int ), CE_OBJS_TYPE_ID );
            int nameSize = oPos->second.typeName.length();
            int exprSize = oPos->second.typeExpr.length();
            oRec.Write( &nameSize, sizeof( int ), CE_OBJS_TYPENAME_SIZE );
            oRec.Write( &exprSize, sizeof( int ), CE_OBJS_TYPEEXPR_SIZE );
            oRec.Write( oPos->second.typeName.data(), 
	                nameSize, CE_OBJS_TYPEINFO_START );
            oRec.Write( oPos->second.typeExpr.data(), 
	                exprSize, CE_OBJS_TYPEINFO_START + nameSize );
            oRec.Finish();
          }
          else
          {
            ok = false;
          }
        }
        break;
      }
      case EntryDelete:
      {
        if ( !revert )
        {
          if ( objCatalogFile.DeleteRecord( SmiKey( oPos->first ) ) )
          {
            if ( oPos->second.valueRecordId != 0 )
            {
              objValueFile.DeleteRecord( oPos->second.valueRecordId );
            }
          }
          else
          {
            ok = false;
          }
        }
        break;
      }
    }
  }

/*
In this second iteration:

 * Objects previously created and updated with undefined values are
updated to express their real value. Then they are saved and closed.

 * In the deletion, the object is now deleted.

This two iteration process is used because some objects have big
representations and the database sometimes get out of locks in the
save process. Then, if it occurs, the database state will be
preserved and the objects are created with undefined values.

*/
  for ( ObjectsCatalog::iterator oPos = objects.begin();
        oPos != objects.end(); 
	oPos++ )
  {
    switch (oPos->second.state)
    {
      case EntryInsert:
      {
        if ( !revert )
        {
          SmiRecord oRec;
          if ( objCatalogFile.SelectRecord( SmiKey( oPos->first ), 
	                                    oRec, SmiFile::Update ) )
          {
            if ( oPos->second.valueDefined )
            {
              oRec.Write( &oPos->second.valueDefined, 
		          sizeof( bool ), CE_OBJS_VALUE_DEF );
              SmiRecord vRec;
              size_t offset = 0;
              ok = objValueFile.AppendRecord( oPos->second.valueRecordId, 
		                              vRec                        );
              if ( ok )
              {
                oRec.Write( &oPos->second.valueRecordId, 
		            sizeof( SmiRecordId ), CE_OBJS_VALUE_RECID );
                ListExpr typeExpr, typeInfo;
                nl->ReadFromString( oPos->second.typeExpr, typeExpr );
                typeInfo = NumericType( nl->First( typeExpr ) );
                am->SaveObj( oPos->second.algebraId, oPos->second.typeId,
                             vRec, offset, typeInfo, oPos->second.value );
                (am->CloseObj( oPos->second.algebraId, oPos->second.typeId ))
                  ( typeInfo, oPos->second.value );
                nl->Destroy( typeInfo );
                nl->Destroy( typeExpr );
              }
              oRec.Finish();
            }
          }
          else
          {
            ok = false;
          }
        }
        break;
      }
      case EntryUpdate:
      {
        if ( !revert )
        {
          SmiRecord oRec;
          if ( objCatalogFile.SelectRecord( SmiKey( oPos->first ), 
	                                    oRec, SmiFile::Update ) )
          {
            oRec.Write( &oPos->second.valueDefined, 
	                sizeof( bool ), CE_OBJS_VALUE_DEF );
            bool ok2;
            if ( oPos->second.valueDefined )
            {
              SmiRecord vRec;
              size_t offset = 0;
              if ( oPos->second.valueRecordId == 0 )
              {
                ok2 = objValueFile.AppendRecord(
		        oPos->second.valueRecordId, vRec );
                oRec.Write( &oPos->second.valueRecordId, 
		            sizeof( int ), CE_OBJS_VALUE_RECID );
              }
              else
              {
                ok2 = objValueFile.SelectRecord( 
		        oPos->second.valueRecordId, vRec, SmiFile::Update );
              }
              if ( ok2 )
              {
                ListExpr typeExpr, typeInfo;
                nl->ReadFromString( oPos->second.typeExpr, typeExpr );
                typeInfo = NumericType( nl->First( typeExpr ) );
                am->SaveObj( oPos->second.algebraId, oPos->second.typeId,
                             vRec, offset, typeInfo, oPos->second.value );
                (am->CloseObj( oPos->second.algebraId, oPos->second.typeId ))
                  ( typeInfo, oPos->second.value );
                nl->Destroy( typeInfo );
                nl->Destroy( typeExpr );
              }
              oRec.Finish();
            }
          }
          else
          {
            ok = false;
          }
        }
        break;
      }
      case EntryDelete:
      {
        if ( !revert )
        {
          if ( oPos->second.valueDefined )
          {
            ListExpr typeExpr, typeInfo;
            nl->ReadFromString( oPos->second.typeExpr, typeExpr );
            typeInfo = NumericType( nl->First( typeExpr ) );
            (am->DeleteObj( oPos->second.algebraId, oPos->second.typeId ))
              ( typeInfo, oPos->second.value );
            nl->Destroy( typeInfo );
            nl->Destroy( typeExpr );
          }
        }
        break;
      }
    }
  }

  types.clear();
  objects.clear();
  return (ok);
}

