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
#include <boost/thread/mutex.hpp>

#include <loguru.hpp>

#include "Algebras/DBService2/SecondoDatabaseAdapter.hpp"
#include "Algebras/DBService2/SecondoUtilsLocal.hpp"
#include "Algebras/DBService2/DebugOutput.hpp"

#include <string>

using namespace std;

extern NestedList* nl;
extern boost::mutex nlparsemtx;

namespace DBService
{

  shared_ptr<DatabaseAdapter> SecondoDatabaseAdapter::dbAdapter = nullptr;
  boost::recursive_mutex SecondoDatabaseAdapter::utilsMutex;

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
    LOG_SCOPE_FUNCTION(INFO);

    int recordId = -1;
    bool success = false;

    boost::lock_guard<boost::recursive_mutex> lock(utilsMutex);

    print("Opening the database...", std::cout);
    LOG_F(INFO, "Opening the database: %s", database.c_str());

    // Ensures acting on the DBService database.
    openDatabase(database);

    print("Building the insert query...", std::cout);
    LOG_F(INFO, "%s", "Building the insert query...");

    // Later is must be defined which column contains the "TID" attribute.
    // Therefore, the project operator is used to make TID the first attribute 
    // in the result.
    // E.g. query Cities inserttuple["Delhi", 28] consume feed project[TID] 
    // consume
    insertQuery += " feed project[TID] consume";

    string errorMessage;
    ListExpr resultList;

    print("Executing the insert query...", std::cout);
    LOG_F(INFO, "Executing the insert query: %s", insertQuery.c_str());

    try {
      
      // Without triggering the transaction, the relation creations would 
      // not be committed      
      SecondoSystem::BeginTransaction();

      success = SecondoUtilsLocal::executeQueryCommand(insertQuery, resultList,
        errorMessage, true);

      if(!SecondoSystem::CommitTransaction(true))
      {
        throw SecondoException("CommitTransaction() failed!");
      }

    }
    catch(const std::exception& e)
    {
      print("Couldn't execute create query. An Exception occured: ",
        string(e.what()), std::cout);
      
      LOG_F(ERROR, "Couldn't execute create query. An Exception occured: %s",
        e.what());
    }

    if(success) {

      boost::lock_guard<boost::mutex> guard(nlparsemtx);
      //print("resultList", resultList, std::cout);
      //TODO Add LOG_F

      ListExpr resultData = nl->Second(resultList);
      print("resultData", resultData, std::cout);
      //TODO Add LOG_F

      int resultCount = nl->ListLength(resultData);
      print("ResultCount", resultCount, std::cout);
      //TODO Add LOG_F

      // Inserting a record should return exactly one result containing the 
      // record's id.      
      if(resultCount != 1) {
        print("Exception: Found a resultCount != 1. Expected exactly one \
result containing the record's id.", std::cout);

        LOG_F(ERROR, "%s", "Exception: Found a resultCount != 1. "
        "Expected exactly one result containing the record's id.");

        throw SecondoException("Found a resultCount != 1. Expected exactly \
one result containing the record's id.");

      }

      if(nl->IsEmpty(resultData)) {
        print("Exception: Found empty result data. Should have found the \
inserted record's id.", std::cout);

        LOG_F(ERROR, "%s", "Exception: Found empty result data. "
        "Should have found the inserted record's id.");

        throw SecondoException("Found empty result data. Should have found \
the inserted record's id.");

      }

      ListExpr currentRow = nl->First(resultData);
      recordId = nl->IntValue(nl->First(currentRow));

    }
    else {
      print("Exception: The record couldn't be inserted. " + errorMessage, 
        std::cout);
      LOG_F(ERROR, "Exception: The record couldn't be inserted: %s ", 
        errorMessage.c_str());

      throw SecondoException("The record couldn't be inserted." + 
        errorMessage);
    }
    return recordId;
  }

  void SecondoDatabaseAdapter::executeCreateRelationQuery(string database, 
    string query) {

    LOG_SCOPE_FUNCTION(INFO);
    printFunction("SecondoDatabaseAdapter::executeCreateRelationQuery", 
      std::cout);

    //boost::lock_guard<boost::recursive_mutex> lock(utilsMutex);    

    bool success = false;

    openDatabase(database);

    string errorMessage;

    print("Executing the query...", std::cout);
    LOG_F(INFO, "Executing the query: %s", query.c_str());

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
        LOG_F(ERROR, "%s", "CommitTransaction() failed!");
        throw SecondoException("CommitTransaction() failed!");
      }

    }
    catch(const std::exception& e)
    {
      print("Couldn't execute the query. An Exception occurred: ",
        string(e.what()), std::cout);
      LOG_F(ERROR, "Couldn't execute the query. An Exception occurred: %s",
            e.what());
    }

    if(success)
    {
      print("\tThe query has been executed successfully.", std::cout);
      LOG_F(INFO, "%s", "The query has been executed successfully.");
    }
    else
    {
      print("Exception: The query couldn't be executed. " + errorMessage, 
        std::cout);
      LOG_F(ERROR, "Couldn't execute the query. An Exception occurred: %s",
        errorMessage.c_str());

      throw SecondoException("Exception: The query couldn't be executed. " + 
        errorMessage);
    }
  }


  void SecondoDatabaseAdapter::executeQueryWithoutResult(
    string database, string query, bool useTransaction, 
    bool destroyRootValue) {

    LOG_SCOPE_FUNCTION(INFO);
    printFunction("SecondoDatabaseAdapter::executeCreateRelationQuery", 
      std::cout);

    boost::lock_guard<boost::recursive_mutex> lock(utilsMutex);

    bool success = false;

    openDatabase(database);

    string errorMessage;

    print("Executing the query...", std::cout);
    LOG_F(INFO, "Executing the query: %s", query.c_str());

    try
    {
      if (useTransaction) {
        // Without triggering the transaction, the relation creations would not 
        // be committed      
        SecondoSystem::BeginTransaction();
      }

      ListExpr resultList;

      //TODO move implementation to this class and eliminate dependency to 
      // SecondoUtilsLocal.
      success = SecondoUtilsLocal::executeQueryCommand(query, resultList, 
        errorMessage);

      if(useTransaction && !SecondoSystem::CommitTransaction(true))
      {
        LOG_F(ERROR, "%s", "CommitTransaction() failed!");
        throw SecondoException("CommitTransaction() failed!");
      }

    }
    catch(const std::exception& e)
    {
      print("Couldn't execute the query. An Exception occurred: ",
        string(e.what()), std::cout);
      LOG_F(ERROR, "Couldn't execute the query. An Exception occurred: %s",
        e.what());
    }

    if(success)
    {
      print("\tThe query has been executed successfully.", std::cout);
      LOG_F(INFO, "%s", "The query has been executed successfully.");
    }
    else
    {
      print("Exception: The query couldn't be executed. " + errorMessage, 
        std::cout);
      LOG_F(ERROR, "Couldn't execute the query. An Exception occurred: %s",
              errorMessage.c_str());
      throw SecondoException("Exception: The query couldn't be executed. " + 
        errorMessage);
    }
  }


  /*
    Executing arbitrary queries.
    It is assumes that the query either does not return values or that 
    there is not interest in them.
   */
  ListExpr SecondoDatabaseAdapter::executeFindQuery(string database, 
    string query) {
    bool success = false;

    printFunction("SecondoDatabaseAdapter::executeFindQuery", std::cout);
    LOG_SCOPE_FUNCTION(INFO);

    boost::lock_guard<boost::recursive_mutex> lock(utilsMutex);

    print("Opening the database...", std::cout);

    // Ensures acting on the DBService database.
    openDatabase(database);

    string errorMessage;
    ListExpr resultList;

    print("Executing the query...", std::cout);
    LOG_F(INFO, "Executing the query: %s", query.c_str());

    try {

      // Without triggering the transaction, the relation creations would 
      // not be committed      
      SecondoSystem::BeginTransaction();

      success = SecondoUtilsLocal::executeQueryCommand(query, resultList, 
        errorMessage);

      if(!SecondoSystem::CommitTransaction(true))
      {
        LOG_F(ERROR, "%s", "CommitTransaction() failed!");
        throw SecondoException("CommitTransaction() failed!");
      }

    }
    catch(const std::exception& e) {
      print("Couldn't execute query. An Exception occurred: ", string(e.what()),
        std::cout);
      LOG_F(ERROR, "Couldn't execute the query. An Exception occurred: %s",
        e.what());
    }

    LOG_F(INFO, "%s", "The query has been executed. Parsing the result...");
    

    if(success) {

      boost::lock_guard<boost::mutex> guard(nlparsemtx);
      
      ListExpr resultData = nl->Second(resultList);
      print("resultData", resultData, std::cout);

      int resultCount = nl->ListLength(resultData);
      print("ResultCount", resultCount, std::cout);
    }
    else {
      print("Exception: The find statement couldn't be executed: " + 
        errorMessage, std::cout);
      LOG_F(ERROR, "Couldn't execute the find query. An Exception occurred: %s",
            errorMessage.c_str());

      throw SecondoException("The find statement couldn't be executed: " + 
        errorMessage);
    }

    //TODO Decide whether resultList or resultData should be returned
    return resultList;
  }

  /*
    Return the name of the currently opened database.
    Assumption: A database has already been opened.    
  */
  string SecondoDatabaseAdapter::getCurrentDatabase() {
    string currentDatabase;

    LOG_SCOPE_FUNCTION(INFO);
    //boost::lock_guard<boost::recursive_mutex> lock(utilsMutex);

    if(!isDatabaseOpen() == true)
      throw SecondoException("Can't determine current database as no database \
is open.");

    currentDatabase = string(SecondoSystem::GetInstance()->GetDatabaseName());

    return currentDatabase;
  }

  bool SecondoDatabaseAdapter::doesDatabaseExist(string database)
  {
    LOG_SCOPE_FUNCTION(INFO);
    //boost::lock_guard<boost::recursive_mutex> lock(utilsMutex);
    boost::lock_guard<boost::mutex> guard(nlparsemtx);

    // Uppercase is also for getting the filenames right later as 
    // they are derived from the relation's db name.
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

  //TODO Continue to use LOG_F in the remainder of this class

  void SecondoDatabaseAdapter::openDatabase(string database)
  {
    LOG_SCOPE_FUNCTION(INFO);

    // Deadlock
    //boost::lock_guard<boost::recursive_mutex> lock(utilsMutex);

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
    LOG_SCOPE_FUNCTION(INFO);
    SecondoSystem* secondoSystem = SecondoSystem::GetInstance();

    // Only close if a db is open. Else do nothing.
    if(isDatabaseOpen() == true)
    {
      secondoSystem->CloseDatabase();
    }
  }

  void SecondoDatabaseAdapter::createDatabase(string database)
  {
    LOG_SCOPE_FUNCTION(INFO);

    boost::lock_guard<boost::recursive_mutex> lock(utilsMutex);

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
    LOG_SCOPE_FUNCTION(INFO);

    boost::lock_guard<boost::recursive_mutex> lock(utilsMutex);

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

    LOG_SCOPE_FUNCTION(INFO);

    openDatabase(database);
    SecondoCatalog* catalog = SecondoSystem::GetCatalog();

    return catalog->IsObjectName(relationName, true);
  }

  void SecondoDatabaseAdapter::createRelation(string database, 
    string relationName, string createRelationStatement) {

    LOG_SCOPE_FUNCTION(INFO);

    boost::lock_guard<boost::recursive_mutex> lock(utilsMutex);

    openDatabase(database);

    if(doesRelationExist(database, relationName) == false)
      executeCreateRelationQuery(database, createRelationStatement);
    else
      print("Relation already exists: ", relationName, std::cout);
  }

  bool SecondoDatabaseAdapter::isDatabaseOpen()
  {
    LOG_SCOPE_FUNCTION(INFO);

    //boost::lock_guard<boost::recursive_mutex> lock(utilsMutex);

    SecondoSystem* secondoSystem = SecondoSystem::GetInstance();
    return secondoSystem->IsDatabaseOpen();
  }
}
