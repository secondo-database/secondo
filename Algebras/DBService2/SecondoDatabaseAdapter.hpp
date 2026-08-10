/*

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

*/

#ifndef SECONDO_DATABASE_ADAPTER_H
#define SECONDO_DATABASE_ADAPTER_H

#include "Algebras/DBService2/DatabaseAdapter.hpp"

#include <boost/thread/mutex.hpp>
#include <mutex>

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
    static std::recursive_mutex utilsMutex;

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
      std::string database, std::string query, 
      bool useTransaction = true, bool destroyRootValue = true) override;

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