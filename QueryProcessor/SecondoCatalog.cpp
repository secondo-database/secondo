/*

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

using namespace std;

#include "SecondoCatalog.h"
#include "SecondoSystem.h"

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
const SmiSize CE_OBJS_MODEL          = sizeof( SmiRecordId ) + CE_OBJS_VALUE_RECID;
const SmiSize CE_OBJS_MODEL_RECID    = sizeof( Word ) + CE_OBJS_MODEL;
const SmiSize CE_OBJS_ALGEBRA_ID     = sizeof( SmiRecordId ) + CE_OBJS_MODEL_RECID;
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
been assigned to the object. Field ~model~ contains the model data
structure for this object. 

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
      if ( !typeRecord.Read( &exprSize, sizeof( int ), CE_TYPES_EXPR_SIZE ) ) continue;
      typeBuffer = new char[exprSize];
      if ( !typeRecord.Read( typeBuffer, exprSize, CE_TYPES_EXPR_START ) ) continue;
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
Deletes a type with name ~typeName~ in the database. This is only possible if the type with name ~typeName~ is not used by an object. Returns error 1 if type is used by an object, error 2, if type name is not known.

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
        if ( tRec.Read( &tEntry.algebraId, sizeof( int ), CE_TYPES_ALGEBRA_ID ) &&
             tRec.Read( &tEntry.typeId, sizeof( int ), CE_TYPES_TYPE_ID ) &&
             tRec.Read( &exprSize, sizeof( int ), CE_TYPES_EXPR_SIZE ) )
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
      cerr << " GetTypeExpr: " << typeName << " is not a valid type name!" << endl;
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
    return ((am->TypeCheck( algebraId, typeId ))( typeExpr, errorInfo ));
  }
}
 
/************************************************************************
3.1.2 Database Objects

*/                                

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
  ListExpr objectsList, typeExpr, lastElem = 0;
  string objectName, typeName, typeExprString;
  char*  oBuffer;
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
      if ( !oRec.Read( &nameSize, sizeof( int ), CE_OBJS_TYPENAME_SIZE ) ) continue;
      if ( !oRec.Read( &exprSize, sizeof( int ), CE_OBJS_TYPEEXPR_SIZE ) ) continue;
      int bufSize = (nameSize > exprSize) ? nameSize : exprSize;
      oBuffer = new char[bufSize];
      if ( nameSize > 0 )
      {
        if ( !oRec.Read( oBuffer, nameSize, CE_OBJS_TYPEINFO_START ) ) continue;
        typeName.assign( oBuffer, nameSize );
      }
      else
      {
        typeName = "";
      }
      if ( !oRec.Read( oBuffer, exprSize, CE_OBJS_TYPEINFO_START+nameSize ) ) continue;
      typeExprString.assign( oBuffer, exprSize );
      delete []oBuffer;
      nl->ReadFromString( typeExprString, typeExpr ); 
      if ( objectsList == nl->TheEmptyList() )
      {
        objectsList = nl->Cons( nl->FourElemList(
                                  nl->SymbolAtom( "OBJECT" ),
                                  nl->SymbolAtom( objectName ),
                                  nl->OneElemList( nl->SymbolAtom( typeName ) ),
                                  typeExpr ),
                                nl->TheEmptyList() );
        lastElem = objectsList; 
      }
      else
      {
        lastElem = nl->Append( lastElem,
                     nl->FourElemList(
                       nl->SymbolAtom( "OBJECT" ),
                       nl->SymbolAtom( objectName ),
                       nl->OneElemList( nl->SymbolAtom( typeName ) ),
                       typeExpr ) );
      }
    }
  }
  oIterator.Finish();
  for ( oPos = objects.begin(); oPos != objects.end(); oPos++ )
  {
    if ( oPos->second.state == EntryInsert ||
         oPos->second.state == EntryUpdate )
    {
      nl->ReadFromString( oPos->second.typeExpr, typeExpr );
      if ( objectsList == nl->TheEmptyList() )
      {
        objectsList = nl->Cons( nl->FourElemList(
                                  nl->SymbolAtom( "OBJECT" ),
                                  nl->SymbolAtom( oPos->first ),
                                  nl->OneElemList( nl->SymbolAtom( oPos->second.typeName ) ),
                                  typeExpr ),
                                nl->TheEmptyList() );
        lastElem = objectsList; 
      }
      else
      {
        lastElem = nl->Append( lastElem,
                     nl->FourElemList(
                       nl->SymbolAtom( "OBJECT" ),
                       nl->SymbolAtom( oPos->first ),
                       nl->OneElemList( nl->SymbolAtom( oPos->second.typeName ) ),
                       typeExpr ) );
      }
    }
  }
  objectsList = nl->Cons( nl->SymbolAtom( "OBJECTS" ), objectsList );
  return (objectsList);
}

ListExpr
SecondoCatalog::ListObjectsFull()
{
/*
Returns a list of ~objects~ of the whole database in the following format:

---- (OBJECTS 
       (OBJECT <object name>(<type name>) <type expression> <value> <model>)*
     )
----

For each object the *value* component is missing, otherwise the whole database will be returned.

Precondition: dbState = dbOpen.

*/
  ListExpr objectsList, typeExpr, valueList, modelList, lastElem = 0;
  Word value, model;
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

      GetObjectExpr( objectName, typeName, typeExpr,
                     value, defined, model, hasTypeName );
      if ( defined )
      {
        valueList = OutObject( typeExpr, value );
      }
      else
      {
        valueList = nl->TheEmptyList();
      }
      if ( model.addr != 0 )
      {
        modelList = OutObjectModel( typeExpr, model );
      }
      else
      {
        modelList = nl->TheEmptyList();
      }
      if ( objectsList == nl->TheEmptyList() )
      {
        objectsList = nl->Cons( nl->SixElemList(
                                  nl->SymbolAtom( "OBJECT" ),
                                  nl->SymbolAtom( objectName ),
                                  nl->OneElemList( nl->SymbolAtom( typeName ) ),
                                  typeExpr,
                                  valueList,
                                  modelList ),
                                nl->TheEmptyList() );
        lastElem = objectsList; 
      }
      else
      {
        lastElem = nl->Append( lastElem,
                     nl->SixElemList(
                       nl->SymbolAtom( "OBJECT" ),
                       nl->SymbolAtom( objectName ),
                       nl->OneElemList( nl->SymbolAtom( typeName ) ),
                       typeExpr,
                       valueList,
                       modelList ) );
      }
    }
  }
  oIterator.Finish();
  for ( oPos = objects.begin(); oPos != objects.end(); oPos++ )
  {
    if ( oPos->second.state == EntryInsert ||
         oPos->second.state == EntryUpdate )
    {
      nl->ReadFromString( oPos->second.typeExpr, typeExpr );
      if ( oPos->second.valueDefined )
      {
        valueList = OutObject( typeExpr, oPos->second.value );
      }
      else
      {
        valueList = nl->TheEmptyList();
      }
      if ( oPos->second.model.addr != 0 )
      {
        modelList = OutObjectModel( typeExpr, oPos->second.model );
      }
      else
      {
        modelList = nl->TheEmptyList();
      }
      if ( objectsList == nl->TheEmptyList() )
      {
        objectsList = nl->Cons( nl->SixElemList(
                                  nl->SymbolAtom( "OBJECT" ),
                                  nl->SymbolAtom( oPos->first ),
                                  nl->OneElemList( nl->SymbolAtom( oPos->second.typeName ) ),
                                  typeExpr,
                                  valueList,
                                  modelList ),
                                nl->TheEmptyList() );
        lastElem = objectsList; 
      }
      else
      {
        lastElem = nl->Append( lastElem,
                     nl->SixElemList(
                       nl->SymbolAtom( "OBJECT" ),
                       nl->SymbolAtom( oPos->first ),
                       nl->OneElemList( nl->SymbolAtom( oPos->second.typeName ) ),
                       typeExpr,
                       valueList,
                       modelList ) );
      }
    }
  }
  objectsList = nl->Cons( nl->SymbolAtom( "OBJECTS" ), objectsList );
  return (objectsList);
}

bool
SecondoCatalog::CreateObject( const string& objectName, const string& typeName,
                              const ListExpr typeExpr, const int sizeOfComponents )
{
/*
Creates a new object with identifier ~objectName~ defined with type name ~typeName~ (can be empty) and type ~typeExpr~. The value is not yet defined, and no memory is allocated. The model is also undefined. Returns error 1, if the object name is defined already.

Precondition: dbState = dbOpen.

*/
  int alId = 7, typeId = 7;
  Word value;
  string typecon;
  Word model;

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
  model = InObjectModel( typeExpr, nl->TheEmptyList(), 1 );    
            /* generates the undefined model for this type */
  return (InsertObject( objectName, typeName, typeExpr, value, false, model ));
}

bool
SecondoCatalog::InsertObject( const string& objectName, const string& typeName,
                              const ListExpr typeExpr, const Word valueWord,
                              const bool defined, const Word modelWord )
{
/*
Inserts a new object with identifier ~objectName~ and value ~valueWord~ defined by type name ~typeName~ or by a list ~typeExpr~ of already existing types (which always exists) into the database catalog. Parameter ~defined~ tells, whether ~wordvalueWord~ actually contains a defined value. Further, ~modelWord~ contains a model for this value, possibly 0, the undefined model. If the object name already exists, the procedure has no effect. Returns error 1 if the ~objectName~ is used already.

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
      oPos->second.model = modelWord;
      oPos->second.modelRecordId = 0;
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
    objEntry.model = modelWord;
    objEntry.modelRecordId = 0;
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
Deletes an object with identifier ~objectName~ in the database calatog and deallocates the used memory. Returns error 1 if the object does not exist.

Precondition: dbState = dbOpen.

*/
  bool ok = false;
  string typeName, typecon;
  Word value, model;
  bool defined, hasNamedType;
  ListExpr typeExpr;

  if ( testMode && !SmiEnvironment::IsDatabaseOpen() )
  {
    cerr << " DeleteObject: database is closed!" << endl;
    return (false);
  }

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
          (am->DeleteObj( oPos->second.algebraId, oPos->second.typeId ))( oPos->second.value );
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
                           value, defined, model, hasNamedType ) )
  {
    ObjectsCatalogEntry oEntry;
    oEntry.state        = EntryDelete;
    oEntry.value        = value;
    oEntry.valueDefined = defined;
    oEntry.model        = model;
    oEntry.typeName     = typeName;
    nl->WriteToString( oEntry.typeExpr, typeExpr );
    LookUpTypeExpr( typeExpr, typecon, oEntry.algebraId, oEntry.typeId );
    objects.insert( make_pair( objectName, oEntry ) );
  }
  return (ok);
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
  Word value, model;
  bool hasNamedType;
  bool defined;

  if ( testMode && !SmiEnvironment::IsDatabaseOpen() )
  {
    cerr << " GetObjectValue: database is closed!" << endl;
    exit( 0 );
  }
  GetObjectExpr( objectName, typeName, typeExpr, value, defined, model, 
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

Word
SecondoCatalog::InObjectModel( const ListExpr typeExpr,
                               const ListExpr modelList,
                               const int objNo )
{
/*
Converts a model of the type given by ~typeExpr~ and the value given as a nested list into a WORD representation which is returned.

Precondition: dbState = dbOpen.

*/
  ListExpr pair, numtype;
  int algebraId, typeId;

  numtype = NumericType( typeExpr );

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
  return ((am->InModel( algebraId, typeId ))( typeExpr, modelList, objNo ));
}

ListExpr
SecondoCatalog::OutObjectModel( const ListExpr typeExpr, const Word model )
{
/*
Returns for a given ~model~ of type ~typeExpr~ its description in nested list representation.

*/
  ListExpr pair, numtype;
  int alId, typeId;

  numtype = NumericType( typeExpr );
  if ( nl->IsEmpty( numtype ) )
  { /* do nothing */
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
    return ((am->OutModel( alId, typeId ))( typeExpr, model ));
  }
}

Word
SecondoCatalog::ValueToObjectModel( const ListExpr typeExpr, const Word value )
{
/*
Returns for a given ~value~ of type ~typeExpr~ its model.

*/
  ListExpr pair, numtype;
  int alId, typeId;

  numtype = NumericType( typeExpr );
  if ( nl->IsEmpty( numtype ) )
  { /* do nothing */
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
    alId = nl->IntValue( nl->First( pair ) );
    typeId = nl->IntValue( nl->Second( pair ) );
    return ((am->ValueToModel( alId, typeId ))( typeExpr, value ));
  }
}

Word
SecondoCatalog::ValueListToObjectModel( const ListExpr typeExpr,
                                        const ListExpr valueList,
                                        int& errorPos, 
                                        ListExpr& errorInfo, 
                                        bool& correct )
{
/*
Returns for a given ~value~ of type ~typeExpr~ its model.

*/
  ListExpr pair, numtype;
  int alId, typeId;

  numtype = NumericType( typeExpr );
  if ( nl->IsEmpty( numtype ) )
  { /* do nothing */
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
    alId = nl->IntValue( nl->First( pair ) );
    typeId = nl->IntValue( nl->Second( pair ) );
    return ((am->ValueListToModel( alId, typeId ))
            (  typeExpr, valueList, errorPos, errorInfo, correct ));
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
        if ( objValueFile.SelectRecord( valueRecId, vRec ) )
        {
          ListExpr typeExpr, typeInfo;
          nl->ReadFromString( typeExprString, typeExpr );  
          typeInfo = NumericType( typeExpr );
          am->PersistValue( algebraId, typeId, ReadFrom, vRec, typeInfo, value );
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
                               Word& model,
                               bool& hasTypeName )
{
/*
Returns the value ~value~, the type name ~typeName~, the type expression ~typeExpr~, and the ~model~ of an object with identifier ~objectName~. ~defined~ tells whether ~value~ contains a defined value. If object has no type name the variable  ~hasTypeName~ is set to FALSE and the procedure returns an empty string as ~typeName~.

Precondition: ~IsObjectName(objectName)~ delivers TRUE.  

*/
  bool ok = false;
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
      typeExpr = nl->First( typeExpr ); //???
      value       = oPos->second.value;
      defined     = oPos->second.valueDefined;
      model       = oPos->second.model;
      hasTypeName = (typeName != "");
      ok = true;
    }
    else
    {
      typeName    = "";
      typeExpr    = nl->TheEmptyList();
      value.addr  = 0;
      defined     = false;
      model.addr  = 0;
      hasTypeName = false;
    }
  }
  else
  {
    SmiRecord oRec;
    if ( objCatalogFile.SelectRecord( SmiKey( objectName ), oRec ) )
    {
      SmiRecordId valueRecId, modelRecId;
      oRec.Read( &value, sizeof( Word ), CE_OBJS_VALUE );
      oRec.Read( &defined, sizeof( bool ), CE_OBJS_VALUE_DEF );
      oRec.Read( &valueRecId, sizeof( SmiRecordId ), CE_OBJS_VALUE_RECID );
      oRec.Read( &model, sizeof( Word ), CE_OBJS_MODEL );
      oRec.Read( &modelRecId, sizeof( int ), CE_OBJS_MODEL_RECID );
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
        if ( objValueFile.SelectRecord( valueRecId, vRec ) )
        {
          ListExpr typeExpr, typeInfo;
          nl->ReadFromString( typeExprString, typeExpr );  
          typeInfo = NumericType( typeExpr );
          am->PersistValue( algebraId, typeId, ReadFrom, vRec, typeInfo, value );
          nl->Destroy( typeInfo );
          nl->Destroy( typeExpr );
        }
      }
      else
      {
        value.addr = 0;
      }
      if ( model.addr != 0 )
      {
        SmiRecord mRec;
        if ( objModelFile.SelectRecord( modelRecId, mRec ) )
        {
          ListExpr typeExpr;
          nl->ReadFromString( typeExprString, typeExpr );  
          am->PersistModel( algebraId, typeId, ReadFrom, mRec, typeExpr, model );
          nl->Destroy( typeExpr );
        }
      }
      ok = true;
    }
    else
    {
      typeName    = "";
      typeExpr    = nl->TheEmptyList();
      value.addr  = 0;
      defined     = false;
      model.addr  = 0;
      hasTypeName = false;
    }
  }
  return (ok);
}

bool
SecondoCatalog::GetObjectType( const string& objectName, string& typeName )
{
/* 
Returns the type name ~typeName~ of an object with identifier ~objectName~, if the type name exists and an empty string otherwise.

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
      oEntry.value = value;
      oEntry.valueDefined = true;
      oRec.Read( &oEntry.valueRecordId, sizeof( SmiRecordId ), CE_OBJS_VALUE_RECID );
      oRec.Read( &oEntry.model, sizeof( Word ), CE_OBJS_MODEL );
      oRec.Read( &oEntry.modelRecordId, sizeof( int ), CE_OBJS_MODEL_RECID );
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
      objects.insert( make_pair( objectName, oEntry ) );
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
  LocalCatalog::iterator pos;
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
    cerr << "  GetTypeId: " << typeName << " is not a valid type name!" << endl;
    exit( 0 );
  }

  LocalCatalog::iterator pos = constructors.find( typeName );
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
    if ( am->IsAlgebraLoaded( algebraId, catalogLevel ) || 
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

void
SecondoCatalog::GetOperatorId( const string& opName, int& algebraId, int& opId )
{
/*
Returns the algebra identifier ~algebraId~ and the operator identifier
~opId~ of an existing ~opName~. 

Precondition: ~IsOperatorName( opName)~ delivers TRUE.  

*/

  LocalCatalog::iterator pos = operators.find( opName );
 
  if (  pos != operators.end() )
  {
    algebraId = pos->second.algebraId;
    opId      = pos->second.entryId;
  }
  else
  { 
    cerr << "  GetOperatorId: " << opName << " is not a valid operator name!" << endl;
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
    if ( am->IsAlgebraLoaded( algebraId, catalogLevel ) || 
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
    if ( am->IsAlgebraLoaded( algebraId, catalogLevel ) || 
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
  LocalCatalog::iterator pos;
  ListExpr last = 0, list;
  ListExpr opList = nl->TheEmptyList();

  for ( pos = operators.begin(); pos != operators.end(); pos++ )
  {
    list = am->Specs( pos->second.algebraId, pos->second.entryId );
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
  return (opList);
}

/*
3.4 Initialization of Values and Test Procedures

*/

SecondoCatalog::SecondoCatalog( const string& name, 
                                const AlgebraLevel level )
  : typeCatalogFile( SmiKey::String ), objCatalogFile( SmiKey::String ),
    objValueFile( false ), objModelFile( false ), testMode( false )
{
  catalogName  = name;
  catalogLevel = level;
  nl = SecondoSystem::GetNestedList();
  am = SecondoSystem::GetAlgebraManager();

  CatalogEntry newEntry;
  int algebraId = 0;

  while ( am->NextAlgebraId( level, algebraId ) )
  {
    newEntry.algebraId = algebraId;
/* 
Defines a dictionary for algebra type constructors.

*/
    int j;
    for ( j = 0; j < am->ConstrNumber( algebraId ); j++ )
    {
      newEntry.entryId = j;
      constructors.insert( make_pair( am->Constrs( algebraId, j ), newEntry ) );
    }
/* 
Defines a dictionary for algebra operators.

*/
    for ( j = 0; j < am->OperatorNumber( algebraId ); j++ )
    {
      newEntry.entryId = j;
      operators.insert( make_pair( am->Ops( algebraId, j ), newEntry ) );
    }
  }
}

SecondoCatalog::~SecondoCatalog()
{
  constructors.clear();
  operators.clear();
  types.clear();
  objects.clear();
}

bool
SecondoCatalog::Open()
{
  bool ok = true;
  ok = ok && typeCatalogFile.Open( catalogName + "Types", "SecondoCatalog" );
  ok = ok && objCatalogFile.Open( catalogName + "Objects", "SecondoCatalog" );
  ok = ok && objValueFile.Open( catalogName + "ObjValues", "SecondoCatalog" );
  ok = ok && objModelFile.Open( catalogName + "ObjModels", "SecondoCatalog" );
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
  if ( objModelFile.IsOpen() )
  {
    ok = ok && objModelFile.Close();
  }
  types.clear();
  objects.clear();
  return (ok);
}

bool
SecondoCatalog::CleanUp( const bool revert )
{
  bool ok = true;
  TypesCatalog::iterator tPos;
  if ( !revert )
  {
    SmiRecord tRec;
    for ( tPos = types.begin(); tPos != types.end(); tPos++ )
    {
      switch (tPos->second.state)
      {
        case EntryInsert:
        {
          if ( typeCatalogFile.InsertRecord( SmiKey( tPos->first ), tRec ) )
          {
            tRec.Write( &tPos->second.algebraId, sizeof( int ), CE_TYPES_ALGEBRA_ID );
            tRec.Write( &tPos->second.typeId, sizeof( int ), CE_TYPES_TYPE_ID );
            int exprSize = tPos->second.typeExpr.length();
            tRec.Write( &exprSize, sizeof( int ), CE_TYPES_EXPR_SIZE );
            tRec.Write( tPos->second.typeExpr.data(), exprSize, CE_TYPES_EXPR_START );
          }
          else
          {
            ok = false;
          }
          break;
        }
        case EntryUpdate:
        {
          if ( typeCatalogFile.SelectRecord( SmiKey( tPos->first ), tRec, SmiFile::Update ) )
          {
            tRec.Write( &tPos->second.algebraId, sizeof( int ), CE_TYPES_ALGEBRA_ID );
            tRec.Write( &tPos->second.typeId, sizeof( int ), CE_TYPES_TYPE_ID );
            int exprSize = tPos->second.typeExpr.length();
            tRec.Write( &exprSize, sizeof( int ), CE_TYPES_EXPR_SIZE );
            tRec.Write( tPos->second.typeExpr.data(), exprSize, CE_TYPES_EXPR_START );
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
  ObjectsCatalog::iterator oPos;
  SmiRecord oRec;
  for ( oPos = objects.begin(); oPos != objects.end(); oPos++ )
  {
    switch (oPos->second.state)
    {
      case EntryInsert:
      {
        if ( !revert )
        {
          if ( objCatalogFile.InsertRecord( SmiKey( oPos->first ), oRec ) )
          {
            oRec.Write( &oPos->second.value, sizeof( Word ), CE_OBJS_VALUE );
            oRec.Write( &oPos->second.valueDefined, sizeof( bool ), CE_OBJS_VALUE_DEF );
            if ( oPos->second.valueDefined )
            {
              SmiRecord vRec;
              ok = objValueFile.AppendRecord( oPos->second.valueRecordId, vRec );
              if ( ok )
              {
                ListExpr typeExpr, typeInfo;
                nl->ReadFromString( oPos->second.typeExpr, typeExpr );  
                typeInfo = NumericType( typeExpr );
                am->PersistValue( oPos->second.algebraId, oPos->second.typeId,
                                  WriteTo, vRec,
                                  typeInfo, oPos->second.value );
                nl->Destroy( typeInfo );
                nl->Destroy( typeExpr );
              }
            }
            oRec.Write( &oPos->second.valueRecordId, sizeof( SmiRecordId ), CE_OBJS_VALUE_RECID );
            oRec.Write( &oPos->second.model, sizeof( Word ), CE_OBJS_MODEL );
            if ( oPos->second.model.addr != 0 )
            {
              SmiRecord mRec;
              ok = objModelFile.AppendRecord( oPos->second.modelRecordId, mRec );
              if ( ok )
              {
                ListExpr typeExpr;
                nl->ReadFromString( oPos->second.typeExpr, typeExpr );  
                am->PersistModel( oPos->second.algebraId, oPos->second.typeId,
                                  WriteTo, mRec,
                                  typeExpr, oPos->second.model );
                nl->Destroy( typeExpr );
              }
            }
            oRec.Write( &oPos->second.modelRecordId, sizeof( SmiRecordId ), CE_OBJS_MODEL_RECID );
            oRec.Write( &oPos->second.algebraId, sizeof( int ), CE_OBJS_ALGEBRA_ID );
            oRec.Write( &oPos->second.typeId, sizeof( int ), CE_OBJS_TYPE_ID );
            int nameSize = oPos->second.typeName.length();
            int exprSize = oPos->second.typeExpr.length();
            oRec.Write( &nameSize, sizeof( int ), CE_OBJS_TYPENAME_SIZE );
            oRec.Write( &exprSize, sizeof( int ), CE_OBJS_TYPEEXPR_SIZE );
            oRec.Write( oPos->second.typeName.data(), nameSize, CE_OBJS_TYPEINFO_START );
            oRec.Write( oPos->second.typeExpr.data(), exprSize, CE_OBJS_TYPEINFO_START + nameSize );
          }
          else
          {
            ok = false;
          }
        }
        if ( oPos->second.valueDefined )
        {
          (am->DeleteObj( oPos->second.algebraId, oPos->second.typeId ))( oPos->second.value );
        }
        break;
      }
      case EntryUpdate:
      {
        if ( !revert )
        {
          if ( objCatalogFile.SelectRecord( SmiKey( oPos->first ), oRec, SmiFile::Update ) )
          {
            bool ok2;
            oRec.Write( &oPos->second.value, sizeof( int ), CE_OBJS_VALUE );
            oRec.Write( &oPos->second.valueDefined, sizeof( bool ), CE_OBJS_VALUE_DEF );
            if ( oPos->second.valueDefined )
            {
              SmiRecord vRec;
              if ( oPos->second.valueRecordId == 0 )
              {
                ok2 = objValueFile.AppendRecord( oPos->second.valueRecordId, vRec );
              }
              else
              {
                ok2 = objValueFile.SelectRecord( oPos->second.valueRecordId, vRec, SmiFile::Update );
              }
              if ( ok2 )
              {
                ListExpr typeExpr, typeInfo;
                nl->ReadFromString( oPos->second.typeExpr, typeExpr );  
                typeInfo = NumericType( typeExpr );
                am->PersistValue( oPos->second.algebraId, oPos->second.typeId,
                                  WriteTo, vRec,
                                  typeInfo, oPos->second.value );
                nl->Destroy( typeInfo );
                nl->Destroy( typeExpr );
              }
            }
            oRec.Write( &oPos->second.valueRecordId, sizeof( int ), CE_OBJS_VALUE_RECID );
            oRec.Write( &oPos->second.model, sizeof( int ), CE_OBJS_MODEL );
            if ( oPos->second.model.addr != 0 )
            {
              SmiRecord mRec;
              if ( oPos->second.modelRecordId == 0 )
              {
                ok2 = objModelFile.AppendRecord( oPos->second.modelRecordId, mRec );
              }
              else
              {
                ok2 = objModelFile.SelectRecord( oPos->second.modelRecordId, mRec, SmiFile::Update );
              }
              if ( ok2 )
              {
                ListExpr typeExpr;
                nl->ReadFromString( oPos->second.typeExpr, typeExpr );  
                am->PersistModel( oPos->second.algebraId, oPos->second.typeId,
                                  WriteTo, mRec,
                                  typeExpr, oPos->second.model );
                nl->Destroy( typeExpr );
              }
            }
            oRec.Write( &oPos->second.modelRecordId, sizeof( int ), CE_OBJS_MODEL_RECID );
            oRec.Write( &oPos->second.algebraId, sizeof( int ), CE_OBJS_ALGEBRA_ID );
            oRec.Write( &oPos->second.typeId, sizeof( int ), CE_OBJS_TYPE_ID );
            int nameSize = oPos->second.typeName.length();
            int exprSize = oPos->second.typeExpr.length();
            oRec.Write( &nameSize, sizeof( int ), CE_OBJS_TYPENAME_SIZE );
            oRec.Write( &exprSize, sizeof( int ), CE_OBJS_TYPEEXPR_SIZE );
            oRec.Write( oPos->second.typeName.data(), nameSize, CE_OBJS_TYPEINFO_START );
            oRec.Write( oPos->second.typeExpr.data(), exprSize, CE_OBJS_TYPEINFO_START + nameSize );
          }
          else
          {
            ok = false;
          }
        }
        if ( oPos->second.valueDefined )
        {
          (am->DeleteObj( oPos->second.algebraId, oPos->second.typeId ))( oPos->second.value );
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
            if ( oPos->second.modelRecordId != 0 )
            {
              objModelFile.DeleteRecord( oPos->second.modelRecordId );
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
  types.clear();
  objects.clear();
  return (ok);
}

