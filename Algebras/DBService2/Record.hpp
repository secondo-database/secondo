#ifndef RECORD_H
#define RECORD_H

#include "Algebras/DBService2/DatabaseAdapter.hpp"
#include "Algebras/DBService2/SecondoDatabaseAdapter.hpp"
#include "Algebras/DBService2/Query.hpp"

#include <loguru.hpp>

#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <memory>

//TODO Replace cout with debug lib or logger
namespace DBService
{
  /*

    Record is the base class for all entities to be stored in the database.
    It's a simple ORM (https://bit.ly/3n4XcjO) implementation and is
    - although much more basic - inspired by Ruby on Rails' ActiveRecord.

    The design goal is not to implement a generic representation for all
    possible Secondo types across all existing and future algebras.

    Instead it aims to provide an ORM framework to simplify creating
    applications with basic persistency needs such as simple relational
    database models.

    As such the usage of the ~Record~ class will imply restrictions on the
    application design but also make simple applications much more lightweigt
    and more clear in structure.

    Comments about the implementation:
    As my first C++ application, the code shows a learning curve and many C++
    best practices may be violated.

    The test coverage is not complete and should be improved with future
    refactorings.
  */
  template <class RecordType, class RecordAdapter>
  class Record {
  protected:

    bool isDirty;
    bool isNew;
    int id;
    std::string database;
    
    std::shared_ptr<DatabaseAdapter> dbAdapter;
    
    static std::string getRelationName();

    // Inline > the map will be specific to each RecordType
    static inline std::map<int, std::shared_ptr<RecordType> > cache;
    static inline bool cacheValidity = false;

  public:
    Record()
    {    
      // Default setting
      LOG_F(INFO, "Constructing the Record...");  
      setClean();
      setNew();
      setId(-1);

      // The use of this-> avoids the error "use of undeclared identifier"
      this->setDatabaseAdapter(DatabaseAdapter::getInstance());
      this->setDatabase("dbservicedefaultdb");
    }

    Record(const Record &original) {
      LOG_F(INFO, "Copying the Record...");
      original.getIsDirty() ? setDirty() : setClean();
      original.getIsNew() ? setNew() : setNotNew();

      setId(original.getId());    

      // Create a new adapter as we "copy"
      this->setDatabaseAdapter(DatabaseAdapter::getInstance());
      this->setDatabase(original.getDatabase());
    }

    Record(Record &&original) {
      LOG_F(INFO, "Moving the Record...");
      original.getIsDirty() ? setDirty() : setClean();
      original.getIsNew() ? setNew() : setNotNew();

      setId(original.getId());    

      // Use the existing adapter by assigning its address as we "move"
      this->setDatabaseAdapter(DatabaseAdapter::getInstance());    
      this->setDatabase(original.getDatabase());
    }

    void setCacheValidity(bool newCacheValidity) {
      cacheValidity = newCacheValidity;
    }

    bool getCacheValidity() {
      return cacheValidity;
    }

    void setDirty()  {
      isDirty = true;
    }

    void setClean()  {
      isDirty = false;
    }

    void setNew() {
      isNew = true;
    }

    void setNotNew() {
      isNew = false;
    }

    int getId() const{
      return id;
    }

    void setId(int newId) {

      // As the id is set during save() there is no need for 
      // marking the record "dirty".
      id = newId;
    }    

    bool save() {
      bool ret = false;

      LOG_F(INFO, "Saving Record...");          

      if (isDirty == true)
      {

        // Triggering the afterSave callback.
        beforeSave();      
        
        if (isNew == true)      
          ret = create();
        else
          ret = update();

        // Triggering the afterSave callback.
        afterSave();
      }
      else
      {
        LOG_F(INFO, "Record is clean. Nothing to do.");
      }

      LOG_F(INFO, "Done saving.");

      return ret;
    }       

    virtual bool empty() const {
      return true;
    }

    virtual bool create() {
      std::string insertQuery = createStatement();

      LOG_F(INFO, "Record::create()\n\t%s", insertQuery.c_str());          

      int id = dbAdapter->executeInsertQuery(this->getDatabase(), insertQuery);
      setId(id);

      setClean();
      setNotNew();

      //TODO Add to cache. Requires shared_from_this().

      return true;
    }

    virtual std::string createStatement() const {
      throw "createStatement is not implemented in Record";
    }

    virtual bool update()   {
      LOG_F(INFO, "Record::update()");          

      std::string updateQuery = updateStatement();

      dbAdapter->executeQueryWithoutResult(this->getDatabase(), updateQuery,
        true, // use transaction,
        false // do not delete operator tree DestroyRootValue 
        // (see Queryprocessor.cpp for more details on this)
        );

      setClean();
      setNotNew();
      return true;
    }

    virtual std::string updateStatement() const {
      throw "updateStatement IS NOT IMPLEMENTED in Record.";
    }

    /*
      Triggers the deletion of the Record.
      Invokes ~beforeDestroy~ before executing ~deleteRecord~ and
      invokes ~afterDestroy~ afterwards.
    */
    virtual bool destroy() {      //TODO Remove virtual 
      LOG_F(INFO, "Record::destroy()");

      beforeDestroy();
      
      deleteRecord();

      afterDestroy();

      LOG_F(INFO, "Record successfully destroyed.");

      return true;
    };

    /*
      Delete performs the deletion of the Record in the recordDatabase.
      In constrast to ~destroy~ no callbacks such as ~beforeDestroy~
      or ~afterDestroy~ are invoked.

      Note: The function cannot be called ~delete~ as this is a reserved 
      keyword in C++.
    */
    virtual void deleteRecord() {

      std::string query = deleteRecordQuery(database).str();
      LOG_F(INFO, "Record::delete(): %s", query.c_str());

      std::shared_ptr<DatabaseAdapter> dbAdapter = 
        DatabaseAdapter::getInstance();

      dbAdapter->executeQueryWithoutResult(
        database,
        query
      );

      LOG_F(INFO, "%s", "Removing Record from cache...");
      
      // Remove object from cache
      eraseFromCache(getId());
    }

    virtual std::string destroyStatement() {
      throw "destroyStatement IS NOT IMPLEMENTED in Record.";
    }

    /*
      Override afterSave to trigger actions after the save method has been
      executed.
    */
    virtual void afterSave() { }
    virtual void beforeSave() { }

    virtual void beforeDestroy() { }
    virtual void afterDestroy() { }

    //virtual std::string createRelationStatement() const;
    // virtual void createRelation() const;

    // Virtual methods can't be static and static methods can't be virtual.
    static std::string createRelationStatement() {
      throw "Record::createRelationStatement() is not implemented in Record";
    }

    //TODO Deprecated. Refactor to remove.
    static std::string findAllStatement(std::string database) {      
      return findAllQuery(database).str();
    }

    /*
      Creates a Query to retrieve all record of the given RecordType.
    */
    static Query findAllQuery(std::string database) {
      return RecordType::query(database).feed().addid().consume();
    }

    /*
      Finds a vector of records of the given RecordType.

      ~findMany~ does not require a ~database~ argument as this information
      is already contained in the ~Query~ parameter.
    */
    static std::vector<std::shared_ptr<RecordType> > findMany(Query findQuery) {
      
      LOG_SCOPE_FUNCTION(INFO);

      std::vector<std::shared_ptr<RecordType> > manyRecords = 
        findQuery.retrieveVector<RecordType, RecordAdapter>();

      //TODO Currently there is no use of the cache for repeated calls to 
      //  findMany. This could be added when needed.
      syncToCache(manyRecords);

      return manyRecords;
    }

    /*
      Adds the given record to the cache if it hasn't been cached before.
    */
    static void syncToCache(std::vector<std::shared_ptr<RecordType> >       
      recordsToSync) {

        LOG_SCOPE_FUNCTION(INFO);
        
        for(auto& record : recordsToSync) {
          syncToCache(record);
        }

        cacheValidity = true;
    }

    /*
      Adds the given record to the cache if it doesn't exist, yet.
    */
    static void syncToCache(std::shared_ptr<RecordType> recordToSync) {
        LOG_SCOPE_FUNCTION(INFO);
        
        // If the record doesn't exist in the cache, add it.
        if (cache.find(recordToSync->getId()) == cache.end()) {
          LOG_F(INFO, "Adding Record to the cache...");
          cache[recordToSync->getId()] = recordToSync;
        }
    }

    /*
      Removes the record with the given recordId from the cache.
    */
    static void eraseFromCache(int recordId) {
      LOG_SCOPE_FUNCTION(INFO);

      cache.erase(recordId);
    }


    /*
      Find a record by recordId.
      Complexity: ~ O(LOG(n)).
    */
    static std::shared_ptr<RecordType> findOneInCache(int recordId) {
      LOG_SCOPE_FUNCTION(INFO);

      if(cache.find(recordId) != cache.end()) {
        // Cache hit!
        LOG_F(INFO, "%s", "Record cache hit!");
        return cache[recordId];
      }

      return std::shared_ptr<RecordType>(nullptr);
    }

    /*
      This is a more generic find method to find a single record.
      It is less efficient but allows to pass a predicate to match records
      according to the needs of a particular context.
      
      Complexity: ~ O(n).

      Functor specifies the predicate used for comparison.

      Examplary Functor: [=](int relationId, 
        std::pair<int, std::shared_ptr<Derivative>> const& recordPair) 
          { return recordPair.second->getRelationId() == relationId; }
    */
    template<typename Functor>
    static std::shared_ptr<RecordType> findOneInCache(Functor& matchPredicate) {
      std::shared_ptr<RecordType> record;

      LOG_SCOPE_FUNCTION(INFO);

      // foundIt: Iterator pointing to the found record. Pun intended.
      auto foundIt = std::find_if(cache.begin(), cache.end(), matchPredicate);

      if (foundIt != cache.end())
        record = foundIt->second;
      

      // if no record is found, the shared_ptr compares with nullptr
      return record;
    }

    template<typename Functor>
    static std::vector<std::shared_ptr<RecordType> > 
      findManyInCache(Functor& matchPredicate) {

      LOG_SCOPE_FUNCTION(INFO);

      std::vector<std::shared_ptr<RecordType> > records;
      std::shared_ptr<RecordType> record;

      // TODO Find more concise way to express the for loop.
      /*
        Move the foundIt iterator forward while finding more records for which
        the matchPredicate is evaluated to true.
      */
      for(auto foundIt = 
        std::find_if(cache.begin(), cache.end(), matchPredicate); 
        foundIt != cache.end(); 
        foundIt = std::find_if(++foundIt, cache.end(), matchPredicate)) {
        
        // Each find_if may set foundIt to cache.end() 
        // if no further records match the predicate.
        if (foundIt != cache.end()) {
          records.push_back(foundIt->second);
        }
      }

      // if no record is found, the shared_ptr compares with nullptr
      return records;
    }

    /*
      Retrieves all records of the given RecordType.
    */
    static std::vector<std::shared_ptr<RecordType> > findAll(
      std::string database) {

      LOG_SCOPE_FUNCTION(INFO);

      Query query = findAllQuery(database);
      return findMany(query);
    }

    //TODO Deprecated. Refactor to remove.
    static std::string deleteAllStatement() {
      throw "Record::deleteAllStatement is not implemented in Record";
    }

    static Query deleteAllQuery(std::string database) {
      return RecordType::query(database).feed().relation(
        RecordType::getRelationName()).deletedirect().consume();
    }

    /*
      Generates the Query to delete the given record.
    */
    Query deleteRecordQuery(std::string database) {
    
      // query dbs_relations deletebyid[tid(11)] consume
      return RecordType::query(database).deletebyid(getId()).consume();
    }

    static void deleteAll(std::string database) {
      std::shared_ptr<DatabaseAdapter> dbAdapter 
        = DatabaseAdapter::getInstance();
        
      dbAdapter->executeQueryWithoutResult(
        database,
        RecordType::deleteAllQuery(database).str()
      );
    }

    //TODO static or move to manager
    //virtual vector<std::shared_ptr<Record>> findAll();

    bool getIsDirty() const {
      return isDirty;
    }

    bool getIsNew() const {
      return isNew;
    }

    std::string getDatabase() const {
      return database;
    }    

    void setDatabase(std::string newDatabase) {
      if (newDatabase != database) {
        if (getIsNew() == true) {
          database = newDatabase;
        } else {
          throw SecondoException("Can't change the database of a non-new record.");
        }      
      }
    }
    
    //TODO With the database adapter being a singleton, the member could be removed.
    //  Maybe for the selection of db environments it makes sense to have a member so that 
    //  some records can be in a test database (testdbservice) and others can be in 
    //  the production database (other operators.)
    // Returning a shared_ptr is problematic as it doesn't have a copy constructor.
    // shared_ptr<DatabaseAdapter> getDatabaseAdapter() const;
    void setDatabaseAdapter(std::shared_ptr<DatabaseAdapter> newAdapter) {
      dbAdapter = std::move(newAdapter);
    }

    std::shared_ptr<DatabaseAdapter> getDatabaseAdapter() {
      return dbAdapter;
    }

    /* 
      TODO Find a generic way to implement query for all subclasses.
      C++ doesn't support runtime polymorphism for static functions
      Record::query() uses Record::getRelationName()
      When a subclass inherits - such as Relation::query() 
      Record::getRelationName() would be used as there is no dynamic binding
      of static methods. Consequently, query() would fail to generate the
      correct string as getRelationName() should be "dbs_relations", the
      implementation provided by Relation:: instead of Record::.
    */
    // static Query query();

    static Query query(std::string database) {
      std::stringstream query;

      // Ducktyping - expecting RecordType to implement a static method called
      //  getRelationName
      query << "query" << " " << RecordType::getRelationName();

      return Query(database, query.str());
    }

    static std::shared_ptr<RecordType> findByTid(
      std::string database, int tid) {

      LOG_SCOPE_FUNCTION(INFO);
      
      std::shared_ptr<RecordType> record = findOneInCache(tid);

      if (record != nullptr)
        return record;

      LOG_F(INFO, "%s", "Record cache miss! Retrieving from DB...");
      
      Query findQuery = RecordType::query(database).feed().addid().filterByTid(
        tid).consume();

      record = findQuery.retrieveObject<RecordType, RecordAdapter>();      
      syncToCache(record);

      return record;
    }
  };
}
#endif