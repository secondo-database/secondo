#ifndef DBS_DERIVATIVE_H
#define DBS_DERIVATIVE_H

#include "Algebras/DBService2/Record.hpp"
#include "Algebras/DBService2/Relation.hpp"
#include "Algebras/DBService2/Query.hpp"
#include "Algebras/DBService2/Replica.hpp"
#include "Algebras/DBService2/SecondoDerivativeAdapter.hpp"

#include <string>

namespace DBService {

  class SecondoDerivativeAdapter;
  class Relation;
  class Replica;

  /*
    A Derivative represents a function depending on a given 
    Relation. The Derivative is functionally dependent on the Relation which
    means that it can be created from the Relation by executing a user provided
    funtion.
  */
  class Derivative : 
    public Record<DBService::Derivative, SecondoDerivativeAdapter>, public std::enable_shared_from_this<DBService::Derivative> {

    private:

    /*
      Name of the Derivate (Secondo Object representing the Derivative).
    */
    std::string name;

    /*
      Definition of the 
    */
    std::string function;

    std::shared_ptr<DBService::Relation> relation;

    /*
      The Derivative's replicas - in constrast to the Relation's replicas.
    */
    std::vector<std::shared_ptr<Replica> > replicas;

    protected:

    /* 
      The use of ~enable_shared_from_this~ requires that prio to making
      a shared_ptr from this, another shared_ptr for this object must exist.
      The easiest way to ensure this is to declare the constructor proteced
      and force the usage of factory methods called ~build~ which in turn
      create a shared_ptr.
    */
    Derivative();
    Derivative(std::string newName, std::string newFunction);
    Derivative(std::string newName, std::string newFunction,
      std::shared_ptr<DBService::Relation> newRelation);

    public:

    static std::shared_ptr<Derivative> build();

    static std::shared_ptr<Derivative> build(std::string newName, 
      std::string newFunction);

    static std::shared_ptr<Derivative> build(std::string newName, 
      std::string newFunction,
      std::shared_ptr<DBService::Relation> newRelation);


    std::string getName() const;
    void setName(std::string newName);

    std::string getFunction() const;
    void setFunction(std::string newFunction);

    std::shared_ptr<DBService::Relation> getRelation() const;

    /*
      Sets a new Relation.
      If ~force = false~ then changing the Relation will only work for
      non-new records and also mark the record ~dirty~.
      If ~force = true~ dirty checking is disabled and the Relation can
      also be set for non-new records which is required during loading 
      Derivatives during the loading the superior Relation.
    */
    void setRelation(std::shared_ptr<DBService::Relation> newRelation,
      bool force=false);

    /*
     *  Replicas START
     */

    /* 
      If there are more relation replicas than derivative Replicas,
      attempt to create the missing derivative Replicas.
      Avoids duplicates.

      When is synchronization of replicas necessary?
      After the derivative has been created for the first time.
      In constrast to its creation during loading a previously persisted
      instance from the db.
    */
    void syncReplicasWithRelation();

    bool doesReplicaExist(shared_ptr<Replica> newReplica);

    /*
      Adds a derivative replica based on the provided ~relationReplica~.
    */
    void addDerivativeReplica(std::shared_ptr<Replica> relationReplica);

    /*
      Adds a derivative replica for each relation replica.
    */
    void addDerivativeReplicas(std::vector<std::shared_ptr<Replica> > 
      relationReplicas);
    
    int getReplicaCount() const;

    std::vector<std::shared_ptr<Replica> > getReplicas() const;

    void loadReplicas();
    void saveReplicas();

    std::shared_ptr<Replica> findReplica(std::string targetHost, 
      int targetPort);

    /*
      Analog to ~Relation::updateReplicaStatus~.
    */
    void updateReplicaStatus(std::string targetHost, int targetPort, 
      std::string replicaStatus);

    
    /*
      Deletes all replicas by invoking ~destroy~.
    */
    void deleteReplicas();

    /*
     *  Replicas END
     */


    /*
     * Record START      
     */
    std::string str(int indentationLevel = 0) const;

    static std::string getRelationName();
    static std::string createRelationStatement();

    std::string createStatement() const;
    std::string updateStatement() const;

    static Query query(std::string recordDatabase);

    void beforeSave();
    void afterSave();
    void afterDestroy();


    bool empty() const;

    bool operator==(const DBService::Derivative &other) const;
    bool operator!=(const DBService::Derivative &other) const;    

    static std::vector<std::shared_ptr<Derivative> > findAll(
      std::string database);

    /*
      Returns the Derivative if found or ~nullptr~ otherwise.
    */
    static std::shared_ptr<Derivative> findOne(string database, Query query);    

    /*
     * Record END
     */

    /*
      Retrieves Derivatives associated to the Relation specified by its relationId.
    */
    static std::vector<std::shared_ptr<Derivative> > findByRelationId(string database, int relationId);
  };

  // Print relation
  std::ostream &operator<<(std::ostream &os, 
  DBService::Derivative const &derivative);
}

#endif