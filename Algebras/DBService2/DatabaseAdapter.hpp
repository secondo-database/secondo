#ifndef DATABASE_ADAPTER_H
#define DATABASE_ADAPTER_H

#include "NestedList.h"

#include <string>

namespace DBService
{

  /*
    Defines the interface to access the database.
    The main purpose is not to abstract from Secondo - which could be a future 
    design goal for a major refactoring - but to allow a MockAdapter that
    will allow performing tests without using the database to focus
    on testing the Record logic, for example.
  */
  class DatabaseAdapter
  {    

    protected:    

    /* 
      In order to create subclasses with dynamic binding, the database adapter 
      does not use static methods although it does not maintain any state.
      The idea is to have multiple DatabaseAdapter implementations such as
      a MockDatabaseAdapter that doesn't execute commands in a database but is
      used to test the logic handling records, instead.
      
      TODO All execute methods share a significant part of code. Find a way
      to reduce code duplicity.
     */
    DatabaseAdapter();
    DatabaseAdapter(const DatabaseAdapter&);
    DatabaseAdapter(const DatabaseAdapter&&);

    public:

    static std::shared_ptr<DatabaseAdapter> getInstance();

    virtual bool isDatabaseOpen();

    virtual int executeInsertQuery(std::string database, std::string query);


    /*
      Find query may return application specific results.
      The DatabaseAdapter should not be SecondoSpecific.
      Nor should it contain application specific logic, e.g.
      define how a Node-Record is being constructed. 
      
      However, to get the job done, a Secondo-specific return type
      is being used.    
      
      Preconditions:
      Ensure that the database exists and has been opened.
      Ensure that the relation does not exist before using this method.      
     */
    virtual ListExpr executeFindQuery(std::string database, std::string query);

    /*
      Execute a query where there is no need to process the result.

      Preconditions:
      Ensure that the database exists and has been opened.      
    */
    virtual void executeQueryWithoutResult(std::string database, 
      std::string query);

    /*
      Executes the statement to create a relation.
      Does not check whether the relation already exists.

      Preconditions:
      Ensure that the database exists and has been opened.
      Ensure that the relation does not exist before using this method.      
    */
    virtual void executeCreateRelationQuery(std::string database, 
      std::string query);

    virtual bool doesDatabaseExist(std::string database);

    /*
      Opens the given database.
      Closes a database if a database has been open.
      Throws an exception if the database doesn't exist.
    */
    virtual void openDatabase(std::string database);
    virtual void closeDatabase();

    /*
      Creates the given database.
      Does nothing if the database already exists.
    */
    virtual void createDatabase(std::string database);

    virtual void deleteDatabase(std::string database);
    //TODO deleteDatabase

    /*
      Creates the given relation.
      Does nothing if the relation already exists.      
    */
    virtual void createRelation(std::string database, std::string relationName, 
      std::string createStatement);

    virtual bool doesRelationExist(std::string database, 
      std::string relationName);

    // //TODO deleteRelation    

    virtual std::string getCurrentDatabase();
  };
} // namespace DBService

#endif