/*

1 Header File: SecondoCatalog

September 1996 Claudia Freundorfer

December 20, 1996 RHG Changed definition of procedure ~OutObject~.

May 15, 1998 RHG Added treatment of models, especially functions
~InObjectModel~, ~OutObjectModel~, and ~ValueToObjectModel~. 

This module defines the module ~SecondoCatalog~. It manages a set of
databases. A database consists of a set of named types, a set of objects
with given type name or type expressions and a set of models for
objects. Objects can be persistent or not. Persistent objects are
implemented by the ~StorageManager~. When a database is opened, the
catalog with informations about types, type constructors, operators,
objects and models of the database is loaded as *Secondo Catalog* into
main memory. The implementation of the ~SecondoCatalog~ is based on the
concept of ~compact tables~ (module ~Catalog Manager~) and thereby safe
under transactions. Furthermore the catalog of each algebra is loaded
into memory by calling the procedures of the module ~Algebra Manager2~. 

1.2 Interface
    
*/

#ifndef SECONDO_SYSTEM_H
#define SECONDO_SYSTEM_H

#include "NestedList.h"
#include "AlgebraManager.h"
#include "SecondoCatalog.h"

/************************************************************************** 
2.1 Export 

*/ 

/**************************************************************************

3.1 Types, Variables

*/

/*
The type ~dbState~ has two valid states : only one database can be
opened concurrently at any time, then the SECONDO database system has
the state ~dbOpen~, otherwise it has the state ~dbClosed~. 

*/

/**************************************************************************
3.2 Exported Functions and Procedures 

3.2.1 Interface to DatabasesAndTransactions 

*/

class NestedList;
class AlgebraManager;
class QueryProcessor;
class SecondoCatalog;

class SecondoSystem
{
 public:
  ListExpr ListDatabaseNames();
/*
Simply returns the names of existing databases in a list:

---- (<database name 1>..<database name n>)
----

*/
  bool CreateDatabase( const string& dbname );
/*
Creates a new database named ~dbname~ and a logfile used by transaction
management and loads the algebraic operators and type constructors into
the SECONDO programming interface. Returns error 1 if a database under
this name exists already. 

Precondition: DBState = dbClosed.

*/

  bool DestroyDatabase( const string& dbname );
/*
Deletes a database named ~dbname~ and the logfile that was created by
~CreateDatabase~. Returns error 1 if the ~dbname~ is not known. 

Precondition: dbState = dbClosed.

*/

  bool OpenDatabase( const string& dbname );
/*
Opens a database with name ~dbname~. Especially opens a logfile for
managing the transactions of the database system. Returns error 1 if
~dbname~ is unknown. 

Precondition: dbState = dbClosed.

*/
  bool CloseDatabase();
/*
Closes the actually opened database. All open segments are closed and
the allocated memory for internal data structures is returned. 

Precondition: dbState = dbOpen.

*/
  bool IsDatabaseOpen();
/* 
Returns ~true~ if the database is open state, otherwise ~false~.

*/
  bool SaveDatabase( const string& filename );
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
  int RestoreDatabase( const string& dbname, const string& filename, ListExpr& errorInfo );
/*
Reads a database from a file named ~filename~ that has the same nested
list format as in the procedure ~SaveDatabase~ and fills the catalogs
for database types and objects. The database state changes to ~dbOpen~.
Returns error 1 if ~dbname~ is not a known database name, error 2, if
the database name in the file is different from ~dbname~ here, error 3,
if there was a problem in reading the file, and error 4, if the list
structure in the file was not correct. Returns error 5 if there are
errors in type definitions and/or object list expressions. 

Furthermore, any errors found by kind checking and by ~In~ procedures
are returned in the list ~errorInfo~. 

Precondition: dbState = dbClosed.

*/
  string                 GetDatabaseName();
  static SecondoSystem*  GetInstance();
  static bool            StartUp();
  static bool            ShutDown();
  static AlgebraManager* GetAlgebraManager();
  static QueryProcessor* GetQueryProcessor();
  static SecondoCatalog* GetCatalog( const AlgebraLevel level );
  static NestedList*     GetNestedList();
 protected:
  SecondoSystem();
  SecondoSystem( const SecondoSystem& );
  SecondoSystem& operator=( const SecondoSystem& );
  virtual ~SecondoSystem();
 private:
  bool RestoreCatalog( SecondoCatalog* sc,
                       ListExpr types, ListExpr objects,
                       ListExpr& errorInfo );
  bool RestoreTypes( SecondoCatalog* sc,
                     ListExpr types, ListExpr& errorInfo );
  bool RestoreObjects( SecondoCatalog* sc,
                       ListExpr objects, ListExpr& errorInfo );

  static SecondoSystem secondoSystem;

  NestedList*     nl;
  AlgebraManager* algebraManager;
  QueryProcessor* queryProcessor;
  SecondoCatalog* scDescriptive;
  SecondoCatalog* scExecutable;

  bool testMode;
  bool initialized;
};

#endif

