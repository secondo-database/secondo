#ifndef SECONDO_RECORD_ADAPTER_H
#define SECONDO_RECORD_ADAPTER_H

#include "Algebras/DBService2/DatabaseAdapter.hpp"

#include "NestedList.h"

extern NestedList* nl;
extern boost::recursive_mutex nlparsemtx;

#include <loguru.hpp>

namespace DBService
{

  template<typename RecordType, typename RecordAdapterType>
  class Record;

  /*

    Similar to the type mapping of Secondo Algebra modules, within the ORM
    there is a need to map objects represented as nested list into
    data structures. In the ORM these data structures are substypes of the
    Record class.

    In order to achieve the transformation of the nested listed list 
    representation into the Record subclass the Adapter Pattern is used.
    See: https://refactoring.guru/design-patterns/adapter/cpp/example

    The Adapter implictly acts as a factory method 
    (https://refactoring.guru/design-patterns/factory-method) to construct
    Record objects such as Nodes, Relations, etc.


    Template classes do not have a cpp file as their definition needs
    to be done inside the header file.
    See: https://bit.ly/3fKw1IS
  */  
  template <class RecordType, class RecordAdapterType>
  class SecondoRecordAdapter {
    public:


      /*         
         Builds a vector of shared_ptr T (Nodes) from a resultList obtained 
         e.g. by executing a find statement. 

         The resultListOffset sets the offset from which relevant records
         within the nested list are to be expected.
         It is assumed that relevent entries are contained in the NL in a
         particular attribute order.
      */      
      static std::vector<std::shared_ptr<RecordType> > 
        buildVectorFromNestedList(
          std::string database, std::shared_ptr<DatabaseAdapter> dbAdapter, 
          ListExpr resultList, int resultListOffset = 0)  {

        std::vector<std::shared_ptr<RecordType> > records;

        /* The resultList contains a header which we are not interested in.     
        * Example:
        *  (
        *    (rel (tuple ((Host text) (Port int) (Config text) (DiskPath text) 
        *      (ComPort int) (TransferPort int) (TID tid)))) 
        *    (
        *      ('localhost' 1245 '' '/home/doesnt_exist/secondo' 9941 9942 1) 
        *      ('localhost' 1245 '' '/home/doesnt_exist/secondo' 9941 9942 2) 
        *      ('localhost' 1245 '' '/home/doesnt_exist/secondo' 9941 9942 3) 
        *      ('localhost' 1245 '' '/home/doesnt_exist/secondo' 9941 9942 4)
        *     )
        *   )
        * 
        *  The list of records is the 2nd element
        */

        LOG_F(INFO, "%s", "Acquiring lock for nlparsemtx...");
        boost::unique_lock<boost::recursive_mutex> nlLock(nlparsemtx);
        LOG_F(INFO, "%s", "Successfully acquired lock for nlparsemtx...");

        ListExpr recordList = nl->Second(resultList);    

        while ( !nl->IsEmpty(recordList) ) {
            
            // Obtain a record from the list of records.
            // ('localhost' 1245 '' '/home/doesnt_exist/secondo' 9941 9942 1) 
            ListExpr currentRecordNL = nl->First(recordList);
            
            std::shared_ptr<RecordType> currentRecord = 
              buildObjectFromNestedList(database, currentRecordNL, 
                resultListOffset);
                    
            currentRecord->setDatabase(database);
            currentRecord->setDatabaseAdapter(dbAdapter);

            records.push_back(currentRecord);

            // Set the recordList to the reduced List without the currentRow        
            recordList = nl->Rest(recordList);            
        }

      return records;
    }
    
    /* 
      Factory method constructing Record substypes from a given nested list
      acting as an Adapter.

      Input is a nested list.
      Returns a newly constructed Record object of the Type T (e.g. Node).
    */
    static std::shared_ptr<RecordType> buildObjectFromNestedList(
      std::string database, ListExpr recordAsNestedList, 
      int resultListOffset = 0) {

      /* 
        The static_cast makes the buildObjectFromNestedList implementation 
        "pluggable". When configuring specific SecondoRecordAdapter, e.g.
        the SecondoRelationAdapter the SecondoRelationAdapter can inherit 
        from the SecondoRecordAdapter and override the buildObjectFromNestedList
        function.

        See: https://www.youtube.com/watch?v=-WV9vWjhI3g&ab_channel=BoQian
        */
      
      return RecordAdapterType::buildObjectFromNestedList(
        database, recordAsNestedList, resultListOffset);
    }
  };

} // namespace DBService

#endif