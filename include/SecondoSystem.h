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

1 Header File: Secondo System

May 2002, Ulrich Telle. Port to C++.

August 2002, Ulrich Telle. Added methods to set and get the current algebra level.

April 29 2003, F. Hoffmann. Added save and restore commands for single objects.

April 2004 Hoffmann Changed some implementation details, so that the list databases 
command is available under Windows XP.

May 2004, M. Spiekermann. Function ~SaveDatabase~ was modified to left out derived objects.

Sept 15 2004, M. Spiekermann. Restore database returns now a value of type SI\_Error 
instead of an integer.

1.1 Overview

This module implements those parts of the "Secondo"[3] catalog which
are independent of the algebra level (descriptive or executable).

It manages a set of databases. A database consists of a set of named
types, a set of objects with given type name or type expressions and
a set of models for objects. Objects can be persistent or not.
Persistent objects are implemented by the ~Storage Management Interface~.
When a database is opened, for each algebra level a catalog with
informations about types, type constructors, operators, objects and
models of the database is loaded. Furthermore the catalog of each
algebra is loaded into memory by calling the procedures of the module
~Algebra Manager~.

1.2 Interface methods

The class ~SecondoSystem~ provides the following methods:

[23]    System Management & Database Management & Information    \\
        [--------]
        GetInstance       & CreateDatabase      & ListDatabaseNames \\
        StartUp           & DestroyDatabase     & IsDatabaseOpen \\
        ShutDown          & OpenDatabase        & GetDatabaseName \\
        GetAlgebraManager & CloseDatabase       &  \\
        GetQueryProcessor & SaveDatabase        &  \\
        GetCatalog        & RestoreDatabase     & SetAlgebraLevel \\
        GetNestedList     &                     & GetAlgebraLevel \\

1.4 Imports

*/

#ifndef SECONDO_SYSTEM_H
#define SECONDO_SYSTEM_H

#include "ErrorCodes.h"
#include "NestedList.h"
#include "AlgebraManager.h"
#include "SecondoCatalog.h"

/**************************************************************************
Forward declaration of several classes:

*/
class NestedList;
class AlgebraManager;
class QueryProcessor;
class SecondoCatalog;
class DerivedObj;

/*
1.3 Class "SecondoSystem"[1]

This class implements all algebra level independent functionality of the
"Secondo"[3] catalog management.

*/
class SecondoSystem
{
 public:
  SecondoSystem( GetAlgebraEntryFunction getAlgebraEntryFunc );
  virtual ~SecondoSystem();
  ListExpr ListDatabaseNames();
/*
Simply returns the names of existing databases in a list:

---- (<database name 1>..<database name n>)
----

*/
  bool CreateDatabase( const string& dbname );
/*
Creates a new database named ~dbname~ and loads the algebraic operators
and type constructors for each algebra level into the "Secondo"[3]
programming interface. Returns "false"[4] if a database under
this name already exists.

*Precondition*: No database is open.

*/
  bool DestroyDatabase( const string& dbname );
/*
Deletes a database named ~dbname~ and all data files belonging to it.
Returns "false"[4] if the database ~dbname~ is not known.

*Precondition*: No database is open.

*/
  bool OpenDatabase( const string& dbname );
/*
Opens a database with name ~dbname~.
Returns "false"[4] if database ~dbname~ is unknown.

*Precondition*: No database is open.

*/
  bool CloseDatabase();
/*
Closes the currently opened database.

*Precondition*: A database is open.

*/
  bool IsDatabaseOpen();
/*
Returns "true"[4] if a database is in open state, otherwise "false"[4].

*/
bool IsDatabaseObject( const string& objectName );
/*
Returns whether object with ~objectName~ is known in the currently opened
database.

*/
bool SaveObject ( const string& objectName,
                  const string& filename );
/*
Writes the currently open database called ~dbname~ to a file with name
~filename~ in nested list format. The format is the following:

---- (OBJECT <object name> (<type name>) <type expression> <value> <model>)*

----

Returns error 1 if there was a problem in writing the file.

Precondition: dbState = dbOpen.

*/
  bool SaveDatabase( const string& filename, const DerivedObj& derivedObjs );
/*
Writes the currently open database called ~dbname~ to a file with name
~filename~ in nested list format. The format is as follows:

---- (DATABASE <database name>
       (DESCRIPTIVE ALGEBRA)
         (TYPES
           (TYPE <type name> <type expression>)*
         )
         (OBJECTS
           (OBJECT <object name> (<type name>) <type expression>
                                               <value> <model>)*
         )
       (EXECUTABLE ALGEBRA)
         (TYPES
           (TYPE <type name> <type expression>)*
         )
         (OBJECTS
           (OBJECT <object name> (<type name>) <type expression>
                                               <value> <model>)*
         )
     )
----

Derived objects maintained in the DerivedObj instance are ignored.
Returns "false"[4] if there was a problem in writing the file.

*Precondition*: A database is open.

*/
  int RestoreObjectFromFile( const string& objectname,
                             const string& filename,
                             ListExpr& errorInfo );
/*
Reads an object from a file named ~filename~ and fills the catalog
with this object. The database remains in state ~dbOpen~.
Returns error 1, if object name in file is different from parameter ~objectname~,
error 2, if there was a problem in reading the file, error 3, if the list structure
in the file was not correct, error 4, if there is an error in object list expression,

Furthermore, any errors found by kind checking and by ~In~ procedures are added to
the list ~errorInfo~.

*Precondition*: Database is open and object is not known in the currently opened database.

*/

  SI_Error RestoreDatabase( const string& dbname,
                            const string& filename,
                            ListExpr& errorInfo );
/*
Reads a database from a file named ~filename~ that has the same nested
list format as described in the method ~SaveDatabase~ and fills the catalogs
for database types and objects. The database is in open state after
successful completion.
Returns an error if ~dbname~ is not a known database name, or
the database name in the file is different from ~dbname~ here, or
there was a problem in reading the file, or the list
structure in the file was not correct, or if there are
errors in type definitions and/or object list expressions.

Furthermore, any errors found by kind checking and by ~In~ procedures
are returned in the list ~errorInfo~.

*Precondition*: No database is open.

*/
  string                 GetDatabaseName();
/*
Returns the name of the currently open database. An empty string is
returned if no database is in open state.

*/
  static SecondoSystem*  GetInstance();
/*
Returns a reference to the single instance of the "Secondo"[3] system.

*/
  static bool            StartUp();
/*
Initializes the "Secondo"[3] system. The ~Storage Management Interface~
is started and the algebra modules are loaded into main memory.

*/
  static bool            ShutDown();
/*
Shuts down the "Secondo"[3] system and the ~Storage Management Interface~.
Dynamically loaded algebra modules are unloaded.

*/
  static AlgebraManager* GetAlgebraManager();
/*
Returns a reference to the associated algebra manager.

*/
  static QueryProcessor* GetQueryProcessor();
/*
Returns a reference to the associated query processor.

*/
  static void SetAlgebraLevel( const AlgebraLevel level );
/*
Sets the current algebra ~level~.

*/
  static AlgebraLevel GetAlgebraLevel();
/*
Returns the current algebra level.

*/
  static SecondoCatalog* GetCatalog( const AlgebraLevel level );
/*
Returns a reference to the "Secondo"[3] catalog of the specified
algebra ~level~.

*/
  static NestedList* GetNestedList();
  static NestedList* GetAppNestedList();
/*
Returns a reference to the associated nested list container.
The first one is used by the query processor and algebra modules and
the second is an application specific list container in which query
results are stored.

*/
  static bool BeginTransaction();
/*
Begins a transaction.

*/
  static bool CommitTransaction();
/*
Commits a transaction.

*/
  static bool AbortTransaction();
/*
Aborts a transaction.

*/
  static SmiRecordFile* GetFlobFile();
/*
Returns the file for FLOB objects.

*/
 protected:
  SecondoSystem( const SecondoSystem& );
  SecondoSystem& operator=( const SecondoSystem& );
 private:
  bool RestoreCatalog( SecondoCatalog* sc,
                       ListExpr types, ListExpr objects,
                       ListExpr& errorInfo );
  bool RestoreTypes( SecondoCatalog* sc,
                     ListExpr types, ListExpr& errorInfo );
  bool RestoreObjects( SecondoCatalog* sc,
                       ListExpr objects, ListExpr& errorInfo );
/*
Are internal methods for restoring a database.

*/
  static SecondoSystem* secondoSystem;

  NestedList*     nl;
  NestedList*     al;
  AlgebraManager* algebraManager;
  QueryProcessor* queryProcessor;
  SecondoCatalog* scDescriptive;
  SecondoCatalog* scExecutable;
  AlgebraLevel    currentLevel;
  SmiRecordFile*  flobFile;

  bool            testMode;
  bool            initialized;
};

#endif

