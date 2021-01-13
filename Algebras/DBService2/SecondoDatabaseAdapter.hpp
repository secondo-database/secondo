#ifndef SECONDO_DATABASE_ADAPTER_H
#define SECONDO_DATABASE_ADAPTER_H

#include "Algebras/DBService2/DatabaseAdapter.hpp"

#include <string>
#include <vector>

namespace DBService
{

  /**
   * TODO Make class thread-safe
   * 
   */
  class SecondoDatabaseAdapter : public DatabaseAdapter
  {    

    protected:

    static std::shared_ptr<DatabaseAdapter> dbAdapter;

    SecondoDatabaseAdapter();
    SecondoDatabaseAdapter(const SecondoDatabaseAdapter&);
    SecondoDatabaseAdapter(const SecondoDatabaseAdapter&&);

    public:

    static std::shared_ptr<DatabaseAdapter> getInstance();

    /*TODO how to make the execute methods generic?
     *  Objective: Avoid code duplication.
     *  Idea: Separate the execution of the query from the processing of the
     *    query results. Especially when nested lists are being processed, 
     *    there is no need to pass arguments as nl is a global variable.
     */

    int executeInsertQuery(std::string database, std::string query) override;

    void executeQueryWithoutResult(
      std::string database, std::string query) override;

    void executeCreateRelationQuery(std::string database, 
      std::string query) override;
    
    ListExpr executeFindQuery(std::string database, std::string query) override;
    

    // TODO executeUpdateQuery

    bool doesDatabaseExist(std::string database) override;
    void openDatabase(std::string database) override;
    void closeDatabase() override;
    
    void createDatabase(std::string database) override;
    void deleteDatabase(std::string database) override;
    
    std::string getCurrentDatabase() override;

    bool isDatabaseOpen() override;

    void createRelation(std::string database, std::string relationName, 
      std::string createStatement) override;

    bool doesRelationExist(std::string database, 
      std::string relationName) override;
      
    // // deleteRelation
  };

} // namespace DBService

#endif