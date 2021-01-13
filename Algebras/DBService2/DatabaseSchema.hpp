#ifndef DBS_DATABASE_SCHEMA_H
#define DBS_DATABASE_SCHEMA_H

#include "Algebras/DBService2/DatabaseAdapter.hpp"

#include <string>

namespace DBService {
  class DatabaseSchema {
    public:
    
    /*
      Creates the DBService database schema.
      Creates the given database if it doesn't exist already.
      Currently does not support database schema changes.

      TODO Add support for schema changes.
    */
    static void migrate(std::string database); 

    /*
      Truncate all Relations of the given Database.
      This method is part of DatabaseSchema as the operation requires
      knowledge about which relations to be truncated.
      All Records of all RecordTypes will be deleted but the Relations (Tables)
      remain intact.
    */
    static void truncate(std::string database);

    private:

    /*
      Idempotent creation of relations.
      Creates a relation if it doesn't exist. Does nothing otherwise.
    */
    template<typename RecordType>
    static void createRelation(std::string database, 
      std::shared_ptr<DatabaseAdapter> adapter) {
      
      // Open the database and create the relation if it doesn't exist, yet.
      adapter->createRelation(
          database,
          RecordType::getRelationName(),
          RecordType::createRelationStatement()
      );
    }
  };
}

#endif