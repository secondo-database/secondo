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

1 Header File: Secondo Catalog

September 1996 Claudia Freundorfer

December 20, 1996 RHG Changed definition of procedure ~OutObject~.

May 15, 1998 RHG Added treatment of models, especially functions
~InObjectModel~, ~OutObjectModel~, and ~ValueToObjectModel~. 

May 2002 Ulrich Telle Port to C++

May 2004 M. Spiekermann. Support for system reserved identifiers and derived objects added. 
The new private member ~sysObjNames~ and the methods ~IsSystemObj~ and ~AddSystemObjName~ were
introduced. To avoid saving derived objects (for further Information see DerivedObj.h) the
method ~ListObjectsFull~ was modified.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hybrid~). Only the executable
level remains. Models are also removed from type constructors.

April 2006, M. Spiekermann. New methods ~systemTable~ and ~createRelation~ added.
These will be used to check if a given object name is a system table and if a
relation should be created on the fly by calling ~InObject~.

1.1 Overview

This module defines the module ~SecondoCatalog~. It manages a set of
named types, a set of objects with given type name or type expressions
for a database. Persistency is implemented by the ~Storage Management 
Interface~.

Modifications to the catalog by the methods of this module are registered
in temporary data structures in memory and written to disk on completion
of the enclosing transaction.

1.2 Interface methods
    
The class ~SecondoCatalog~ provides the following methods:
  
[23]    Catalog and Types     & Object Values            & Type Constructors / Operators \\
        [--------]
        SecondoCatalog        & ListObjects              & ListTypeConstructors \\
        [tilde]SecondoCatalog & ListObjectsFull          & IsTypeName           \\
        Open                  & CreateObject             & GetTypeId            \\
        Close                 & InsertObject             & GetTypeName          \\
        CleanUp               & DeleteObject             & GetTypeDS            \\
                              & KillObject               &   \\  
                              & InObject                 &   \\
                              & GetObjectValue           &   \\
                              & OutObject                &   \\
                              & UpdateObject             &   \\
                              & CloneObject              &   \\
                              & ModifyObject             &   \\
        ListTypes             & IsObjectName             & ListOperators        \\
        InsertType            & GetObject                & IsOperatorName       \\
        DeleteType            & GetObjectExpr            & GetOperatorId        \\
        MemberType            & GetObjectType            & GetOperatorName      \\
        LookUpTypeExpr        & GetObjectTypeExpr        &   \\
        GetTypeExpr           & CloseObject              &   \\
        NumericType           &                          &   \\
        ExpandedType          &                          &   \\
        KindCorrect           &                          &   \\
                              &                          &   \\

1.4 Imports

*/

#ifndef SECONDO_CATALOG_H
#define SECONDO_CATALOG_H

#include <vector>
#include <set>
#include <iostream>

#include "AlgebraManager.h"
#include "NestedList.h"
#include "NameIndex.h"
#include "SecondoSMI.h"
#include "SystemInfoRel.h"
#include "SystemTables.h"


// forward declaration
class DerivedObj;

/*
1.3 Class "SecondoCatalog"[1]

This class implements all functionality of the
"Secondo"[3] catalog management.

All operations on types and objects are valid only, when the associated
database is open. Type constructors and operators may be accessed when
no database is open.

*/

class SecondoCatalog
{
 public:
  SecondoCatalog();
/*
Creates a new catalog.

*/
  virtual ~SecondoCatalog();
/*
Destroys a catalog.

*/
  bool Open();
/*
Opens the catalog for operation. Returns "true"[4] if the catalog could be
opened successfully, otherwise "false"[4].

*/
  bool Close();
/*
Closes the catalog. Returns "true"[4] if the catalog could be
closed successfully, otherwise "false"[4].

*/
  bool CleanUp( const bool revert );
/*
Cleans up the memory representation when a transaction is completed.
The switch ~revert~ has to be set to "true"[4] if the enclosing transaction
is aborted.

*/

/*
3.2.2 Database Types   

*/
  ListExpr ListTypes();
/*
Returns a list of types of the whole database in the following format:

----  (TYPES 
        (TYPE <type name><type expression>)* 
      )
----

*/
  bool InsertType( const string& typeName,
                   ListExpr typeExpr );
/*
Inserts a new type with identifier ~typeName~ defined by a list
~typeExpr~ of already existing types in the database. Returns "false"[4],
if the name was already defined. 

*/
  int DeleteType( const string& typeName );
/*
Deletes a type with identifier ~typename~ in the database. Returns error
1 if type is used by an object, error 2, if ~typename~ is not known. 

*/
  bool MemberType( const string& typeName );
/*                  
Returns "true"[4], if type with name ~typename~ is member of the actually
open database. 

*/                          
  bool LookUpTypeExpr( const ListExpr typeExpr,
                       string& typeName,
                       int& algebraId, int& typeId );
/*                        
Returns the algebra identifier ~algebraId~ and the type identifier
~opId~ and the name ~typeName~ of the outermost type constructor for a
given type expression ~typeExpr~, if it exists, otherwise an empty
string as ~typeName~ and value 0 for the identifiers, and the methods
return value is set to "false"[4].

*/
  ListExpr GetTypeExpr( const string& typeName );
/*                        
Returns a type expression for a given type name ~typename~,
if exists. 

*Precondition*: "MemberType( typeName ) == true"[4].

*/
  ListExpr NumericType( const ListExpr type );
/*
Transforms a given type expression into a list structure where each type
constructor has been replaced by the corresponding pair (algebraId,
typeId). For example, 

----  int -> (1 1)

      (rel (tuple ((name string) (age int)))

      ->  ((2 1) ((2 2) ((name (1 4)) (age (1 1))))
----

Identifiers such as ~name~, ~age~ are moved unchanged into the result
list. If a type expression contains other constants that are not
symbols, e.g. integer constants as in (array 10 real), they are also
moved unchanged into the result list. 

The resulting form of the type expression is useful for calling the type
specific ~In~ and ~Out~ procedures. 

*/
  ListExpr ExpandedType ( const ListExpr type );
/*
Transforms a given type definition (a type expression possibly
containing type names, or just a single type name) into the
corresponding type expression where all names have been replaced by
their defining expressions. 

3.1.1 Kind Checking

*/
  bool KindCorrect ( const ListExpr type, ListExpr& errorInfo );
/*
Here ~type~ is a type expression. ~KindCorrect~ does the kind checking;
if there are errors, they are reported in the list ~errorInfo~, and
"false"[4] is returned. ~errorInfo~ is a list whose entries are again
lists, the first element of an entry is an error code number. For
example, an entry 

----  (1 DATA (hello world))
----

says that kind ~DATA~ does not match the type expression ~(hello
world)~. This is the meaning of the general error code 1. The other
error codes are type-constructor specific. 

*/

/*
3.2.3 Database Objects 

*/                                
  ListExpr ListObjects();
/*
Returns a list of ~objects~ of the whole database in the same format that is used in the procedures ~SaveDatabase~ and ~RestoreDatabase~:

----  (OBJECTS 
        (OBJECT <object name>(<type name>) <type expression>)* 
      )
----

For each object the *value* component is missing, otherwise the whole database 
would be returned.

*/
  ListExpr ListObjectsFull(const DerivedObj& derivedObjs);
/*
Returns a list of ~objects~ of the whole database in the following format:

---- (OBJECTS 
       (OBJECT <object name>(<type name>) <type expression>
                                          <value>)*
     )
----
Derived objects (see class DerivedObj) are not contained in this list.

*/
  bool CreateObject( const string& objectName,
                     const string& typeName,
                     const ListExpr typeExpr,
                     const int sizeOfComponents );
/*
Creates a new object with identifier ~objectName~ defined with type name
~typeName~ (can be empty) and type ~typeExpr~. The value is not yet
defined, and no memory is allocated. Returns "false"[4], if the object name
is defined already. 

*/
  bool InsertObject( const string& objectName,
                     const string& typeName,
                     const ListExpr typeExpr,
                     const Word valueWord,
                     const bool defined );
/*
Inserts a new object with identifier ~objectName~ and value ~valueWord~
defined by type name ~typeName~ or by a list ~typeExpr~ of already
existing types (which always exists) into the database catalog.
Parameter ~defined~ tells, whether ~valueWord~ actually contains a defined
value. If the object name already exists, the procedure has no effect. 
Returns "false"[4] if the ~objectName~ is already in use.

When the given object has no type name, it is mandatory, that ~typeName~
is an empty string.

*/
  bool DeleteObject( const string& objectName );
/*
Deletes an object with identifier ~objectName~ in the database. Returns
"false"[4] if the object does not exist. 

*/
  bool KillObject( const string& objectName );
/*
Kills an object with identifier ~objectName~ in the database. Returns
"false"[4] if the object does not exist. This function differs from
~DeleteObject~ because it is more drastic, i.e., it only deletes the
entry for the object in the Secondo catalog. It can be used for 
objects in corrupted states that cannot be opened for destruction. 

*/

  Word InObject( const ListExpr typeExpr,
                 const ListExpr valueList,
                 const int errorPos,
                 ListExpr& errorInfo,
                 bool& correct );
/*
Converts an object of the type given by ~typeExpr~ and the value given
as a nested list into a ~Word~ representation which is returned. Any
errors found are returned together with the given ~errorPos~ in the list
~errorInfo~. ~correct~ is set to "true"[4] if a value was created (which
means that the input was at least partially correct). 

*/
  ListExpr GetObjectValue( const string& objectName );
/*
Returns the value of a locally stored database object with identifier
~objectName~ as list expression to show the value to the database
user. If the value is undefined, an empty list is returned. 

*/
  ListExpr OutObject( const ListExpr type,
                      const Word object );
/*
Returns for a given ~object~ of type ~type~ its value in nested list
representation. 

*/
  void CloseObject( const ListExpr type,
                    const Word object );
/*
Closes a given ~object~ of type ~type~.

*/
  bool IsObjectName( const string& objectName );
/*
Checks whether ~objectName~ is a valid object name.

*/
  bool GetObject( const string& objectName,
                  Word& word, bool& defined );
/*
Returns the value ~word~ of an object with identifier ~objectName~.
~defined~ tells whether the word contains a meaningful value. 

*Precondition*: "IsObjectName( objectName ) == true"[4].

*/
  bool GetObjectExpr( const string& objectName,
                      string& typeName,
                      ListExpr& typeExpr,
                      Word& value,
                      bool& defined,
                      bool& hasTypeName );
/*
Returns the value ~value~, the type name ~typeName~, and the type 
expression ~typeExpr~ of an object with identifier ~objectName~.
~defined~ tells whether ~value~ contains a defined value. If object has
no type name the variable ~hasTypeName~ is set to "false"[4] and the
procedure returns an empty string as ~typeName~.

*Precondition*: "IsObjectName(objectName) == true"[4].

*/
  bool GetObjectType( const string& objectName,
                      string& typeName );
/* 
Returns the type name ~typeName~ of an object with identifier
~objectName~, if the type name exists and an empty string otherwise. 

*Precondition*: "IsObjectName( objectName ) == true"[4].

*/
  ListExpr GetObjectTypeExpr( const string& objectName );
/* 
Returns the type expression of an object with identifier
~objectName~. 

*Precondition*: "IsObjectName( objectName ) == true"[4].

*/
  bool UpdateObject( const string& objectName,
                     const Word word );
/*
Overwrites the value of the object with identifier ~objectName~ with a
new value ~word~. Returns "false"[4] if object does not exist. 

*/
  bool CloneObject( const string& objectName,
                    const Word word );
/*
Overwrites the value of the object with identifier ~objectName~ with a
new value cloned from ~word~. Returns "false"[4] if object does not exist. 

*/
  bool ModifyObject( const string& objectName, const Word word );
/*
Overwrites the value of the object with identifier ~objectName~ with a
new value ~word~. Returns "false"[4] if object does not exist.
The difference between this function and ~UpdateObject~ is that the
second opens the old object for deletion. This one assumes that the
object is only modified, so that no deletion function is necessary.

3.2.4 Algebra Type Constructors

*/
  ListExpr ListTypeConstructors();
/*
Returns a list of type constructors of the actually load
algebras in the following format: 

----  (
        (<type constructor name> (<arg 1>..<arg n>) <result>) * 
      )
----

*/

  ListExpr ListTypeConstructors(int algebraId);
  void Initialize(TypeInfoRel* r);
  void Initialize(OperatorInfoRel* r);
/*
Returns a list of type constructors of the algebra with Id algebraId
in the following format: 

----  (
        (<type constructor name> (<arg 1>..<arg n>) <result>) * 
      )
----

*/

  bool IsTypeName( const string& typeName );
/*
Checks whether ~typeName~ is a valid name for an algebra type
constructor or a database type. 

*/
  bool GetTypeId( const string& typeName,
                  int& algebraId, int& typeId );
/*
Returns the algebra identifier ~algebraId~ and the type identifier
~opId~ of an existing type constructor or database type with name
~typeName~. 

*Precondition*: "IsTypeName( typeName ) == true"[4].

*/
  string GetTypeName( const int algebraId, const int typeId );
/*
Looks up the name of a type constructor defined by the algebra
identifier ~algebraId~ and the type identifier ~opId~. 

*/
  ListExpr GetTypeDS( const int algebraId, const int typeId );
/*
Looks up the properties of a type constructor defined by the
algebra identifier ~algebraId~ and the type identifier ~opId~. 

3.2.5 Algebra Operators

*/
  ListExpr ListOperators();
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
  ListExpr ListOperators( int algebraId );
/*
Returns a list of type constructors of the algebra with Id algebraId
in the following format: 

----  (
        (<type constructor name> (<arg 1>..<arg n>) <result>) * 
      )
----

*/

  bool IsOperatorName( const string& opName );
/*
Checks whether ~opName~ is a valid operator name.

*/
  ListExpr GetOperatorIds( const string& opName );

/*
Returns the algebra identifier ~algebraId~ and the operator identifier
~opId~ for all operators called ~opName~ in list format like below

( (alId1 opId1) ... (alIdN opIdN) )  

*Precondition*: "IsOperatorName( opName ) == true"[4].

*/
  string GetOperatorName( const int algebraId,
                          const int opId );
/*
Looks for the name of an operator defined by the algebra identifier ~algebraId~ and the operator ~opId~.

*/ 
  ListExpr GetOperatorSpec( const int algebraId,
                            const int opId );
/*
Returns the operator specification of an operator defined by the
algebra identifier ~algebraId~ and the operator identifier ~opId~ in the
following format: 

----  ( <operator name>   
        (<arg type spec 1>..<arg type spec n>)
        <result type spec>
        <syntax>
        <variable defs>
        <formula>
        <explaining text>
      )
----

The function below test if a name is reserved for system use.

*/
  
  inline bool IsSystemObject(const string& s) {
    return (GetSystemTable(s) != 0);
  }

 protected:
  bool TypeUsedByObject( const string& typeName );
 private:
  NestedList*      nl;
  AlgebraManager*  am;

  struct CatalogEntry
  {
    int      algebraId;
    int      entryId;
  };
  typedef vector<CatalogEntry> CatalogEntrySet;
  typedef map<string,CatalogEntry> LocalConstructorCatalog;
  typedef map<string,CatalogEntrySet*> LocalOperatorCatalog;
  LocalConstructorCatalog constructors;
  LocalOperatorCatalog operators;

  enum EntryState { EntryInsert, EntryUpdate, EntryDelete, Undefined };

  struct TypesCatalogEntry
  {
    int        algebraId;
    int        typeId;
    string     typeExpr;
    EntryState state;

    TypesCatalogEntry() : 
      algebraId(0), typeId(0), typeExpr(""), state(Undefined) {}

    ostream& print(ostream& os) const {
      os << "algId  :" << algebraId << endl;	    
      os << "typId  :" << typeId << endl;	    
      os << "typExpr:" << typeExpr << endl;	    
      os << "state  :" << state << endl;	    
      return os;
    }	    

  };
  typedef map<string,TypesCatalogEntry> TypesCatalog;
  TypesCatalog types;
  SmiKeyedFile typeCatalogFile;

  struct ObjectsCatalogEntry
  {
    int         algebraId;
    int         typeId;
    string      typeName;
    string      typeExpr;
    Word        value;
    bool        valueDefined;
    SmiRecordId valueRecordId;
    EntryState  state;

    ObjectsCatalogEntry() : 
      algebraId(0), typeId(0), typeName(""), typeExpr(""), 
      value(SetWord(0)), valueDefined(false), valueRecordId(0), 
      state(Undefined) {}

    ostream& print(ostream& os) const {
      os << "algId       :" << algebraId << endl;	    
      os << "typId       :" << typeId << endl;	    
      os << "typName     :" << typeName << endl;	    
      os << "typExpr     :" << typeExpr << endl;	    
      os << "value       :" << (void*) value.addr << endl;	    
      os << "valueDefined:" << valueDefined << endl;	    
      os << "valueRecId  :" << valueRecordId << endl;	    
      os << "state       :" << state << endl;	    
      return os;
    }	    
  };

  typedef map<string,ObjectsCatalogEntry> ObjectsCatalog;
  ObjectsCatalog objects;
  SmiKeyedFile   objCatalogFile;
  SmiRecordFile  objValueFile;

  bool testMode;
/*
If ~testMode~ is set some preconditions are tested. If an error occurs,
"exit"[4] is called.

*TODO*: "exit"[4] should never be called in the server version. In case of
an error it should always be reported to the client.



*/
  
  // check if name is a sytem table 
  const SystemInfoRel* GetSystemTable(const string& name) const;

  // create a ~trel~ object representing a system table.
  Word CreateRelation(const string& name);

  friend class SecondoSystem;
};

#endif
