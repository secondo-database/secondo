#ifndef DBS_RELATION_MANAGER_H
#define DBS_RELATION_MANAGER_H

#include "Algebras/DBService2/Relation.hpp"
#include "Algebras/DBService2/RecordManager.hpp"
#include "Algebras/DBService2/SecondoNodeAdapter.hpp"

#include <string>

namespace DBService {
  class RelationManager : public RecordManager<DBService::Relation, SecondoRelationAdapter> {    

    public:
    RelationManager(std::string newDatabase);

    /*
      Finds a Relation by its given relation database and relation name
      performing an in-memory search.
      
      Returns ~nullptr~ if such a Relations does not exist.
    */
    std::shared_ptr<DBService::Relation> findByDatabaseAndName(
      std::string relationDatabase, 
      std::string relationName);

    /*
      Searches all Relations in-memory and returns the existing original Node
      given an new (non-persisted) original Node.

      If no such original Node exists ~nullptr~ is returned.
    */
    std::shared_ptr<DBService::Node> findOriginalNode(
      std::shared_ptr<DBService::Node> nodeToFind);

    /*
      Returns true if there's a Relation with a non-zero number of Replicas. 
    */
    bool doesRelationHaveReplicas(
      std::string relationDatabase, 
      std::string relationName);

    /*
      Returns a random replica associated with the relation specified 
      by the given ~relationDatabase~ and ~relationName~.

      Returns ~nullptr~ if no relation or replica could be found.
    */
    std::shared_ptr<Replica> getRandomReplica(std::string relationDatabase, 
      std::string relationName);
    

    /*
      Returns ~true~ if a Derivative with the provided ~derivativeName~ exists.
      return ~false~ otherwise.

      Searches for the Derivative by searching all Relations.
     */ 
    bool doesDerivativeExist(std::string derivativeName);

    /*
      Deletes all Relations belonging to the provided ~relationDatabase~.
    */
    void deleteRelationsByRelationDatabase(std::string relationDatabase);

    /*
      Destroys the relation specified by its ~relationDatabase~ and 
      ~relationName~.

      Does nothing if no relation is found.
    */
    void deleteRelationByDatabaseAndName(std::string relationDatabase,
      std::string relationName);


    /*
      Destroys the derivative specified by its ~derivativeName~, 
      ~relationDatabase~ and ~relationName~.

      Does nothing if no relation or derivative is found.
    */
    void deleteDerivativeByName(std::string relationDatabase,
      std::string relationName, std::string derivativeName);
  };
}

#endif