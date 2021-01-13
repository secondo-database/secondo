#ifndef DBS_QUERY_H
#define DBS_QUERY_H

#include "Algebras/DBService2/DatabaseAdapter.hpp"

#include <sstream>

namespace DBService {

  /*
    The Query class is an object oriented interface to provide a C++ 
    flavored domain specific language to generate database queries.

    TODO If the database is extensible in both data types and operations, 
    the Query interface will have to be extensible by future database 
    extensions such as provided by Secondo Algebras.
    Each algebra may introduce new operators. For each operator there
    must be an extension to Query.
    >Find a way to extend the Query class when implementing a new Algebra.

  */
  class Query {
    private:

    std::string database;

    // The shared pointer fascilitates moving the query
    std::shared_ptr<std::stringstream> query;

    public:
    
    Query(std::string database, std::string query);    

    /*
     Copy constructor
    */
    Query(const Query&);    

    void setDatabase(std::string newDatabase);

    std::string getDatabase() const;

    /*
      TODO Make template function or add signatures to support more data types.
      filter(".Name = ?", "Paul") -> filter[".Name = \"Paul\""]
    */
    Query filter(std::string filterCondition, std::string parameter);
    Query filter(std::string filterCondition, int parameter);

    /*
      filter[.TID = tid(7)]
    */
    Query filterByTid(int tid);

    /*
      query.feed() -> "query some_relation feed"
    */
    Query feed();

    /*
      query.addid() -> "... addid ..."

      Results will then include the additional field "TID".
    */
   Query addid();

  /*
      query.deletedirect() -> "query ... deletedirect"
    */
  Query deletedirect();

  /*
    deletebyid[tid(11)]
  */
  Query deletebyid(int tid);

    /*
      query.consume() -> "query ... consume"
    */
    Query consume();

    /*
      Appends the given string to the query.
    */
    Query appendString(std::string stringToAppend);

    /*
      Appends tthe given relation name to the query;
      Alias for appendString.
    */
    Query relation(std::string relationName);

    /*
      Returns the query as string.
    */
    std::string str() const;

    std::shared_ptr<std::stringstream> getQuery() const;    


    template<typename RecordType, typename SecondoRecordAdapter>
    std::shared_ptr<RecordType> retrieveObject() {
      std::vector<std::shared_ptr<RecordType> > records = retrieveVector<RecordType, SecondoRecordAdapter>();
      if (records.empty())
        return nullptr;
      return records.back();
    }

    /*
      Execute a find query and retrieves a vector of RecordType objects by
      using the SecondoRecordAdapter to convert nested list records into
      their OO counterparts.

      TODO Is it possible to deduce the SecondoRecordAdapter from a mapping?
        RecordType -> SecondoRecordAdapter?
    */ 
    template<typename RecordType, typename SecondoRecordAdapter>
    std::vector<std::shared_ptr<RecordType> > retrieveVector() {
      std::shared_ptr<DatabaseAdapter> dbAdapter = DatabaseAdapter::getInstance();

      if (getDatabase().empty())
        throw "Can't execute Query::retrieveVector for an empty database string!";

      dbAdapter->openDatabase(getDatabase());

      std::string query = str();
      ListExpr resultList = dbAdapter->executeFindQuery(getDatabase(), query);     

      std::vector<std::shared_ptr<RecordType> > records = SecondoRecordAdapter::buildVectorFromNestedList(getDatabase(), dbAdapter, resultList);

      return records;
    }
  };
}
#endif