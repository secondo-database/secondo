#ifndef DBS_RELATION_H
#define DBS_RELATION_H

#include "Algebras/DBService2/Record.hpp"
#include "Algebras/DBService2/Node.hpp"
#include "Algebras/DBService2/Query.hpp"
#include "Algebras/DBService2/Replica.hpp"
#include "Algebras/DBService2/SecondoRelationAdapter.hpp"
#include "Algebras/DBService2/Derivative.hpp"

#include <string>
#include <vector>

namespace DBService {

  // Forward declaration
  class Replica;
  class SecondoRelationAdapter;
  class Derivative;

  /*
    Represents Relations replicated using the DBService.

    Saves original Nodes before saving the Relation.
    Does not save target Nodes. They are assumed to already exist as the
    result of addNode operator invocations when setting up the DB Service 
    cluster.
  */
  class Relation : public Record<DBService::Relation, SecondoRelationAdapter>, public std::enable_shared_from_this<DBService::Relation>  {    

    private:
    std::string relationDatabase;
    std::string name;
    std::shared_ptr<DBService::Node> originalNode;    

    // https://stackoverflow.com/questions/11711034/stdshared-ptr-of-this
    std::vector<std::shared_ptr<DBService::Replica> > replicas;

    std::vector<std::shared_ptr<DBService::Derivative> > derivatives;

    void saveReplicas();
    void saveDerivatives();

    protected:

    /* 
      The use of enable_shared_from_this requires that prio to making
      a shared_ptr from this, another shared_ptr for this object must exist.
      The easiest way to ensure this is to declare the constructor proteced
      and force the usage of factory methods called ~build~ which in turn
      create a shared_ptr.
    */
    Relation();

    /*
      The relationDatabase is the database in which the relation with "name" is 
      stored. It is not the database of the Relation record.
    */
    Relation(std::string relationDatabase, std::string relationName);
    Relation(std::string relationDatabase, std::string relationName, 
      std::shared_ptr<DBService::Node> originalNode);    
    Relation(std::string relationDatabase, std::string relationName,
      std::string originalNodeHost, int originalNodePort, 
      std::string originalNodeDisk);    
    
    public:

    static std::shared_ptr<DBService::Relation> build();
    static std::shared_ptr<DBService::Relation> build(
      std::string relationDatabase, std::string relationName);
    static std::shared_ptr<DBService::Relation> build(
      std::string relationDatabase, std::string relationName, 
      std::shared_ptr<DBService::Node> originalNode);
    static std::shared_ptr<DBService::Relation> build(
      std::string relationDatabase, std::string relationName,
      std::string originalNodeHost, int originalNodePort, 
      std::string originalNodeDisk);

    /*
      In contrast to getDatabase() inherited from the Record superclass,
      getRelationDatabase() returns the Relation attribute relationDatabase
      which describes the database of the originalSystem holding the relation
      represented by the given Relation object.
    */
    std::string getRelationDatabase() const;

    /*
      Sets the Relation's relationDatabase property.
      Converts the name to uppercase before assignment as DB comparisons
      are done in uppercase.
    */
    void setRelationDatabase(std::string newRelationDatabase);

    /*
      Returns the Relation's name.
    */
    std::string getName() const;

    std::shared_ptr<DBService::Node> getOriginalNode() const;
    void setOriginalNode(std::shared_ptr<DBService::Node> newOriginalNode);

    // It is not allowed to change the name of a relation.
    void setName(std::string newName);

    /*
      Very short string description of the given Relation.

      Use ~str()~ over ~operator<<~ when printing a Relation as part of
      other Records that will also be part of Relation's ~operator<<~.
      
      The ~operator<<~ is meant to recursively print all dependent objects
      and these dependent objects will indicate their references to a relation
      by using the short ~str()~. This also presents indefinite loops.
    */
    std::string str(int indentationLevel = 0) const;


    // Replicas START

    std::vector<std::shared_ptr<Replica> > getReplicas() const;

    /*
      Returns the number of Replica objects held in-memory.
      Note that the number of persisted Replicas may be different.
    */
    int getReplicaCount() const;

    /*
      Adds the given replica and sets the database as well
      as the relation of the Replica to the parent Relation.

      The operation is performed in-memory, only.

      The Relation is considered "dirty" (unsaved) after
      the execution of ~addReplica~.
    */
    void addReplica(std::shared_ptr<Replica> newReplica);

    /*
      After a target Node for a Replica of the given Relation has been 
      determined, this function can be used to add the Replica and associate
      it with the Relation.

      The operation is performed in-memory, only.

      The Relation is considered "dirty" (unsaved) after
      the execution of ~addReplica~.

      The targetNode must be non-new so that it already has an id.
    */
    void addReplica(std::shared_ptr<DBService::Node> targetNode);

    /*
      Checks whether a given Replica already exists in-memory.
    */
    bool doesReplicaExist(std::shared_ptr<Replica> newReplica);

    /*
      Finds a Replica associated with the given Relation by 
      the host and port of the Replic's target Node.
      Returns ~nullptr~ in case none if found.
    */
    std::shared_ptr<Replica> findReplica(std::string targetHost, 
      int targetPort, std::string replicaType = "relation");

    
    /*
      Finds Replicas belonging to Derivatives.
      Returns ~nullptr~ if none are found.
    */
    std::shared_ptr<Replica> findReplicaByDerivativeId(std::string targetHost, 
      int targetPort, int derivativeId);
    

    /*
      Finds the Replica associated with the target Node specified by the 
      given ~targetHost~ and ~targetPort~, updates the Replica status to
      ~replicaStatus~ and saves the replica.
    */
    void updateReplicaStatus(std::string targetHost, int targetPort,
      std::string replicaStatus);

    /*      
      Removes unsaved (dirty) Replicas from the list of replicas.
            
      Does not change the Relations's dirty status.

      TODO Rename method to "removeDirtyReplicas". Describes the semantics 
        more accurately.
    */
    void resetReplicas();

    std::shared_ptr<Replica> getRandomReplica() const;

    /*
      Load all replicas belonging to the given Relation.
    */
    void loadReplicas();

    /*
      Destroy alls replicas by invoking ~destroy~.
    */
    void deleteReplicas();

    // Replicas END

    // Derivates START

    /*
      Builds a Derivative from the given ~derivativeName~ and ~function~ and
      associates the Derivative with the given Relation.

      This will also create a derivate Replica for each relation Replica of the 
      Relation.

      The relation is "dirty" afterwards.
    */
    std::shared_ptr<DBService::Derivative>
      addDerivative (std::string derivativeName, std::string function);

    /*
      Returns the number of in-memory Derivatives held by the Relation.
    */
    int getDerivativeCount() const;

    std::vector<std::shared_ptr<DBService::Derivative> > getDerivatives();

    /*
      Loads associated Derivatives.
    */
    void loadDerivatives();

    /*
      Finds the Replica associated with the target Node specified by the 
      given ~targetHost~ and ~targetPort~ of the given ~Derivative~, 
      updates the Replica status to
      ~replicaStatus~ and saves the replica.
    */
    void updateDerivativeReplicaStatus(std::string derivativeName,
      std::string targetHost, int targetPort, std::string replicaStatus);
    
    /*
      Find a Derivative by its id.
      Returns ~nullptr~ if none is found.
    */
    std::shared_ptr<Derivative> findDerivative(int derivativeId);

    /*
      Find a Derivative by its name.
      Returns ~nullptr~ if none is found.
    */
    std::shared_ptr<Derivative> findDerivative(std::string derivativeName);

    /*
      Returns ~true~ if a Derivative with the provided ~derivativeName~ exists.
      return ~false~ otherwise.
     */
    bool doesDerivativeExist(std::string derivativeName);

    /*
      Destroy all derivatives by invoking ~destroy~.
    */
    void deleteDerivatives();

    /*
      Destroy derivative and remove it from the relation.
    */
    void deleteDerivative(std::string derivativeName);

    // Derivatives END

    /*     
     * Record START
     */

    /*
      Returns the name of the Relation's record relation (as in database table).
    */
    static std::string getRelationName();
    static std::string createRelationStatement();

    std::string createStatement() const;

    /*
      It is not possible to update an existing Relation's attributes such as
      the ~relationName~ or ~relationDatabase~.

      Therefore the updateStatement() returns a non-mutating statement that
      can be executed by the Record's save implementation.

      The purpose of this is that still ~before_save~ and ~after_save~ methods
      will be triggered.
      ~Replicas~, for example, may very well be updated and their updates can
      be triggered by updating their ~Relation~.
    */
    std::string updateStatement() const;

    /*
      Trigger saving the originalNode before saving the relation.
    */
    void beforeSave();

    /*
      Trigger saving the Replicas after saving the relation.
    */
    void afterSave();

    void afterDestroy();

    bool empty() const;

    bool operator==(const DBService::Relation &other) const;
    bool operator!=(const DBService::Relation &other) const;

    void loadOriginalNode(int originalNodeId);

    static Query query(std::string database);

    static std::string findAllStatement(std::string database);
    static std::vector<std::shared_ptr<Relation> > findAll(
      std::string database);

    /*
      Returns the Relation if found or ~nullptr~ otherwise.
    */
    static std::shared_ptr<Relation> findOne(std::string database, Query query);

    /*     
     * Record END
     */

    static Query queryByDatabaseAndName(std::string database, 
      std::string relationDatabase, std::string relationName);
  };

  // Record
  // Print relation
  std::ostream &operator<<(std::ostream &os, 
    DBService::Relation &relation);
}

#endif