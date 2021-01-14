/*

1.1.1 Class Implementation

----
This file is part of SECONDO.

Copyright (C) 2017,
Faculty of Mathematics and Computer Science,
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

*/
#include "SecondoException.h"

#include "NestedList.h"
#include "NList.h"

#include <boost/algorithm/string.hpp>

#include "Algebras/DBService2/SecondoDatabaseAdapter.hpp"
#include "Algebras/DBService2/SecondoUtilsLocal.hpp"
#include "Algebras/DBService2/DebugOutput.hpp"

#include <string>

using namespace std;

extern NestedList* nl;

namespace DBService
{

  shared_ptr<DatabaseAdapter> SecondoDatabaseAdapter::dbAdapter = nullptr;

  SecondoDatabaseAdapter::SecondoDatabaseAdapter() {
  }

  SecondoDatabaseAdapter::SecondoDatabaseAdapter(
    const SecondoDatabaseAdapter& original) {
    cout << "Copy SecondoDatabaseAdapter::SecondoDatabaseAdapter" << endl;
  }

  SecondoDatabaseAdapter::SecondoDatabaseAdapter(
    const SecondoDatabaseAdapter&& original) {
    cout << "Move SecondoDatabaseAdapter::SecondoDatabaseAdapter" << endl;
  }

  shared_ptr<DatabaseAdapter> SecondoDatabaseAdapter::getInstance() {

    if(SecondoDatabaseAdapter::dbAdapter == nullptr)

      // make shared can't be used because the constructor is private so that
      // the make function can't invoke it.
      SecondoDatabaseAdapter::dbAdapter = shared_ptr<DatabaseAdapter>(
        new SecondoDatabaseAdapter());

    return SecondoDatabaseAdapter::dbAdapter;
  }

  /*

   The insert query is insofar specials that it also retrieves the ID of the
   newly created record.
   Return value: ID of the newly created record.

  */
  int SecondoDatabaseAdapter::executeInsertQuery(
    string database, string insertQuery)
  {
    int recordId = -1;
    bool success = false;

    printFunction("SecondoDatabaseAdapter::executeInsertQuery", std::cout);

    print("Opening the database...", std::cout);

    // Ensures acting on the DBService database.
    openDatabase(database);

    print("Building the insert query...", std::cout);

    // Later is must be defined which column contains the "TID" attribute.
    // Therefore, the project operator is used to make TID the first attribute 
    // in the result.
    // E.g. query Cities inserttuple["Delhi", 28] consume feed project[TID] 
    // consume
    insertQuery += " feed project[TID] consume";

    string errorMessage;
    ListExpr resultList;

    print("Executing the insert query...", std::cout);

    try {

      SecondoCatalog* catalog = SecondoSystem::GetCatalog();

      // Without triggering the transaction, the relation creations would 
      // not be committed      
      SecondoSystem::BeginTransaction();

      success = SecondoUtilsLocal::executeQueryCommand(insertQuery, resultList,
        errorMessage);

      if(!SecondoSystem::CommitTransaction(true))
      {
        throw SecondoException("CommitTransaction() failed!");
      }

    }
    catch(const std::exception& e)
    {
      print("Couldn't execute create query. An Exception occured: ",
        string(e.what()), std::cout);
    }

    if(success) {
      print("resultList", resultList, std::cout);

      ListExpr resultData = nl->Second(resultList);
      print("resultData", resultData, std::cout);

      int resultCount = nl->ListLength(resultData);
      print("ResultCount", resultCount, std::cout);

      // Inserting a record should return exactly one result containing the 
      // record's id.      
      if(resultCount != 1) {
        print("Exception: Found a resultCount != 1. Expected exactly one \
result containing the record's id.", std::cout);

        throw SecondoException("Found a resultCount != 1. Expected exactly \
one result containing the record's id.");

      }

      if(nl->IsEmpty(resultData)) {
        print("Exception: Found empty result data. Should have found the \
inserted record's id.", std::cout);

        throw SecondoException("Found empty result data. Should have found \
the inserted record's id.");

      }

      ListExpr currentRow = nl->First(resultData);
      recordId = nl->IntValue(nl->First(currentRow));

    }
    else {
      print("Exception: The record couldn't be inserted. " + errorMessage, 
        std::cout);
      throw SecondoException("The record couldn't be inserted." + 
        errorMessage);
    }
    return recordId;
  }

  //TODO Although the query seems to be executed successfully, there is no 
  //  relation afterwards.
  // ALso all subsequent attempts to insert values fail.
  // Is this a problem in the implementation of the query or is it a general 
  //  problem with the
  // attempt to create an empty relation?
  // I don't think so. It is possible using the CLI so it must be possible 
  //  using code.
  // This means that the problem is - most likely - within the implementation 
  //  of this function.
  void SecondoDatabaseAdapter::executeCreateRelationQuery(string database, 
    string query) {

    printFunction("SecondoDatabaseAdapter::executeCreateRelationQuery", 
      std::cout);

    bool success = false;

    openDatabase(database);

    string errorMessage;

    print("Executing the query...", std::cout);

    try
    {
      // Without triggering the transaction, the relation creations would not 
      //  be committed      
      SecondoSystem::BeginTransaction();

      //TODO move implementation to this class and eliminate dependency to 
      // SecondoUtilsLocal.
      success = SecondoUtilsLocal::createRelation(query, errorMessage);

      if(!SecondoSystem::CommitTransaction(true))
      {
        throw SecondoException("CommitTransaction() failed!");
      }

    }
    catch(const std::exception& e)
    {
      print("Couldn't execute the query. An Exception occured: ", 
        string(e.what()), std::cout);
    }

    if(success)
    {
      print("\tThe query has been executed successfully.", std::cout);
    }
    else
    {
      print("Exception: The query couldn't be executed. " + errorMessage, 
        std::cout);
      throw SecondoException("Exception: The query couldn't be executed. " + 
        errorMessage);
    }
  }


  void SecondoDatabaseAdapter::executeQueryWithoutResult(
      string database, string query) {

    printFunction("SecondoDatabaseAdapter::executeCreateRelationQuery", 
      std::cout);

    bool success = false;

    openDatabase(database);

    string errorMessage;

    print("Executing the query...", std::cout);

    try
    {
      // Without triggering the transaction, the relation creations would not 
      // be committed      
      SecondoSystem::BeginTransaction();

      ListExpr resultList;

      //TODO move implementation to this class and eliminate dependency to 
      // SecondoUtilsLocal.
      success = SecondoUtilsLocal::executeQueryCommand(query, resultList, 
        errorMessage);

      if(!SecondoSystem::CommitTransaction(true))
      {
        throw SecondoException("CommitTransaction() failed!");
      }

    }
    catch(const std::exception& e)
    {
      print("Couldn't execute the query. An Exception occured: ", 
        string(e.what()), std::cout);
    }

    if(success)
    {
      print("\tThe query has been executed successfully.", std::cout);
    }
    else
    {
      print("Exception: The query couldn't be executed. " + errorMessage, 
        std::cout);
      throw SecondoException("Exception: The query couldn't be executed. " + 
        errorMessage);
    }
  }


  // /**
  //  * Executing arbitrary queries.
  //  * It is assumes that the query either does not return values or that 
  //  there is not interest in them.
  //  */
  ListExpr SecondoDatabaseAdapter::executeFindQuery(string database, 
    string query) {
    bool success = false;

    printFunction("SecondoDatabaseAdapter::executeQuery", std::cout);

    print("Opening the database...", std::cout);

    // Ensures acting on the DBService database.
    openDatabase(database);

    string errorMessage;
    ListExpr resultList;

    print("Executing the query...", std::cout);

    try {

      // Without triggering the transaction, the relation creations would 
      // not be committed      
      SecondoSystem::BeginTransaction();

      success = SecondoUtilsLocal::executeQueryCommand(query, resultList, 
        errorMessage);

      if(!SecondoSystem::CommitTransaction(true))
      {
        throw SecondoException("CommitTransaction() failed!");
      }

    }
    catch(const std::exception& e) {
      print("Couldn't execute query. An Exception occured: ", string(e.what()), 
        std::cout);
    }

    print("resultList", resultList, std::cout);

    if(success) {

      ListExpr resultData = nl->Second(resultList);
      print("resultData", resultData, std::cout);

      int resultCount = nl->ListLength(resultData);
      print("ResultCount", resultCount, std::cout);
    }
    else {
      print("Exception: The find statement couldn't be executed: " + 
        errorMessage, std::cout);
      throw SecondoException("The find statement couldn't be executed: " + 
        errorMessage);
    }

    //TODO Decide whether resultList or resultData should be returned
    return resultList;
  }

  /**
  * Return the name of the currently opened database.
  * Assumption: A database has already been opened.
  * TODO Add to header file.
  */
  string SecondoDatabaseAdapter::getCurrentDatabase() {
    string currentDatabase;

    if(!isDatabaseOpen() == true)
      throw SecondoException("Can't determine current database as no database \
is open.");

    currentDatabase = string(SecondoSystem::GetInstance()->GetDatabaseName());

    return currentDatabase;
  }

  bool SecondoDatabaseAdapter::doesDatabaseExist(string database)
  {
    //TODO Why use upper here? Does the catalog always return upper case names?
    // Comparing database names in upper case
    boost::to_upper(database);

    string databaseNameInList;

    SecondoSystem* secondoSystem = SecondoSystem::GetInstance();

    ListExpr databaseListType = secondoSystem->ListDatabaseNames();

    print("Database list", nl->ToString(databaseListType), std::cout);

    int resultCount = nl->ListLength(databaseListType);
    print("Number of databases", resultCount, std::cout);

    // For each database
    for(int i = 0; i < resultCount; i++) {

      // Get the database name of the current row
      databaseNameInList = nl->SymbolValue(nl->First(databaseListType));

      if(databaseNameInList == database) {
        return true;
      }

      // Next
      databaseListType = nl->Rest(databaseListType);
    }
  }

  void SecondoDatabaseAdapter::openDatabase(string database)
  {
    //TODO Make consistent. Check everywhere or nowhere.
    if(database == "")
      throw SecondoException("Can't open a database without a database name.");

    printFunction("SecondoDatabaseAdapter::openDatabase", std::cout);

    SecondoSystem* secondoSystem = SecondoSystem::GetInstance();

    if(isDatabaseOpen() == true) {

      // No need to open the db if it's already open.
      if(getCurrentDatabase() == database)
      {
        print("\tDatabase " + database + " already open.", std::cout);
        return;
      }
    }

    // A db is open and it's not the desired db...

    // Can't open a non-existent database.
    if(!doesDatabaseExist(database)) {
      print("\tDatabase " + database + " doesn't exist. Can't open it.", 
        std::cout);
      throw SecondoException("Database " + database + " doesn't exist. \
Thus, can't open it.");
    }

    // Before opening a new db, the current db has to be closed.
    closeDatabase();

    // Opening the db...
    SI_Error err = secondoSystem->OpenDatabase(database);

    if(err != 0) {
      throw SecondoException("Couldn't open database " + database
        + ". SecondoSystem::OpenDatbase has failed with error code. "
        + "Look up the SI_Error code in ErrorCodes.h. Error code: " 
        + std::to_string(static_cast<int>(err)));
    }

    print("Successfully opened database", database, std::cout);
  }

  void SecondoDatabaseAdapter::closeDatabase()
  {
    SecondoSystem* secondoSystem = SecondoSystem::GetInstance();

    // Only close if a db is open. Else do nothing.
    if(isDatabaseOpen() == true)
    {
      secondoSystem->CloseDatabase();
    }
  }

  void SecondoDatabaseAdapter::createDatabase(string database)
  {

    // Can't create an existing database.
    if(doesDatabaseExist(database)) {
      print("\tDatabase " + database + " already exists. Doing nothing.", 
        std::cout);
      return;
    }

    // No database may be open during the creation of a db.
    closeDatabase();

    SecondoSystem* secondoSystem = SecondoSystem::GetInstance();

    bool success = secondoSystem->CreateDatabase(database);

    if(!success)
      throw SecondoException("Couldn't create database " + database + ".");
  }

  void SecondoDatabaseAdapter::deleteDatabase(string database) {

    // No database may be open during the deletion of a db.
    closeDatabase();

    SecondoSystem* secondoSystem = SecondoSystem::GetInstance();

    bool success = secondoSystem->DestroyDatabase(database);

    if(!success)
      throw SecondoException("Couldn't delete database " + database
        + ". Please check if the database exists.");
  }

  bool SecondoDatabaseAdapter::doesRelationExist(string database, 
    string relationName) {

    openDatabase(database);
    SecondoCatalog* catalog = SecondoSystem::GetCatalog();

    return catalog->IsObjectName(relationName, true);
  }

  void SecondoDatabaseAdapter::createRelation(string database, 
    string relationName, string createRelationStatement) {
    openDatabase(database);

    if(doesRelationExist(database, relationName) == false)
      executeCreateRelationQuery(database, createRelationStatement);
    else
      print("Relation already exists: ", relationName, std::cout);
  }

  bool SecondoDatabaseAdapter::isDatabaseOpen()
  {
    SecondoSystem* secondoSystem = SecondoSystem::GetInstance();
    return secondoSystem->IsDatabaseOpen();
  }
}
