#ifndef DBS_RECORD_MANAGER_H
#define DBS_RECORD_MANAGER_H

#include "Algebras/DBService2/DatabaseAdapter.hpp"
#include "Algebras/DBService2/Query.hpp"

#include <vector>
#include <sstream>

namespace DBService {
  /*
    The \textit{RecordManager} is a \textit{vector} wrapper for handling
    Records of a given RecordType (such as \textit{DBService::Node}, 
    \textit{DBService::Relation}, ...) in a single place.

    It provides methods for loading and saving a list of records.
  */
  template<class RecordType, class RecordAdapter>
  class RecordManager {

    protected:
    std::vector<std::shared_ptr<RecordType> > records;

    std::string database;

    std::shared_ptr<DatabaseAdapter> dbAdapter;  

    Query loadQuery;

    public:

    RecordManager(std::string newDatabase) : loadQuery(RecordType::findAllQuery(newDatabase)) {
      if (newDatabase == "")
        throw "Can't use the empty string as a database name!";
    
      database = newDatabase;

      setDatabaseAdapter(DatabaseAdapter::getInstance());
    }

    /*
      Sets the query used in ~load()~.
    */
    void setLoadQuery(Query newQuery) {
      loadQuery = newQuery;
    }

    bool add(std::shared_ptr<RecordType> record) {

      if(record->empty())
        throw "Cannot add empty record.";

      record->setDatabase(getDatabase());
      record->setDatabaseAdapter(dbAdapter);

      // TODO Add method exists or doesExist for improved readability
      if (find(record) == nullptr) {
        records.push_back(record);
      }

      return true;
    }

    /*
      Removes the given ~record~ from the vector of records.

      Do not use this function from a loop.
      Use erase-remove with remove_if.
      Do not modify a container while iterating it.

      See ~RelationManager::deleteRelationsByRelationDatabase~ for an example.
    */
    void remove(std::shared_ptr<RecordType> record) {
      if (records.empty())
      return;
    
      // erase-remove https://en.wikipedia.org/wiki/Erase%E2%80%93remove_idiom
      records.erase(std::remove(records.begin(), records.end(), record), records.end());
    }

    std::shared_ptr<RecordType> find(std::shared_ptr<RecordType> record) {
      typename std::vector<std::shared_ptr<RecordType>>::iterator it;
  
      for (it = records.begin(); it != records.end(); it++)
      {      
        // it is a pointer to a shared pointer so we need to dereference two times to get to the
        // the actual node.      
        if (**it == *record)
        {
          // *it dereferences the iterator pointer and thus points to a shared_ptr
          return *it;
        }
      }
      
      return nullptr;
    }

    std::string getDatabase() const {
      return database;
    }

    std::vector<std::shared_ptr<RecordType> > getAll() {
      return records;
    }
    
    bool empty() {
      return records.empty();
    }

    size_t size() {
      return records.size();
    }

    void setDatabaseAdapter(std::shared_ptr<DatabaseAdapter> newAdapter)
    {
      dbAdapter = std::move(newAdapter);
    }

    /*
      Loads records managed by the given ~RecordManager~ using the
      Query provided by ~loadQuery~ which may be customized in
      implementations using the ~RecordManager~ class template.
    */
    void load() { 
      // Ensure acting upon the right database
      dbAdapter->openDatabase(database);
      
      records = RecordType::findMany(loadQuery);      
    }

    void save() {
      // Ensure acting upon the right database
      dbAdapter->openDatabase(database);

      // Only dirty records will be saved to the db.
      for(auto& record : records)
        record->save();
    }

    std::shared_ptr<RecordType> back() {
      return records.back();
    }

    /*
      Provides a string description of the RecordManager's state.
    */
    std::string str() {
      std::stringstream msg;

      msg << "---------------------------------------" << std::endl;
      msg << "RecordManager - RecordType::getRelationName: "; 
      msg << RecordType::getRelationName() << std::endl;
      msg << "Size: " << records.size() << std::endl;
      msg << "Record database: " << getDatabase() << std::endl;
      msg << "Load query: " << loadQuery.str() << std::endl;
      msg << "Records:" << std::endl;

      for (auto& record : records) {
        msg << "\t" << *record << std::endl;
      }
      msg << "---------------------------------------" << std::endl;

      return msg.str();
    }

    /*
      Find a record by its given ~id~ performing an 
      in-memory search.
    */
    std::shared_ptr<RecordType> findById(int id) {
      for(auto& record : records) {
        if (record->getId() == id)
          return record;
      }        
      
      return nullptr;
    }
  };
}

#endif