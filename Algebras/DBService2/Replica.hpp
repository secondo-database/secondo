#ifndef DBS_REPLICA_H
#define DBS_REPLICA_H

#include "Algebras/DBService2/Record.hpp"
#include "Algebras/DBService2/Node.hpp"
#include "Algebras/DBService2/Relation.hpp"
#include "Algebras/DBService2/SecondoReplicaAdapter.hpp"

#include <string>
#include <vector>

namespace DBService {

  // Forward declaration
  class Relation;
  class Derivative;
  class SecondoReplicaAdapter;

  /*
    A Replica - if of type ~relation~ represents a copy of a Relation.
    If a Replica is of type ~derivative~ it represents a "copy" of a Derivative.

    Similar to Single Table Inheritance (https://bit.ly/38JL3M0) attributes 
    of all Replica types are mapped to a single relation (table) containing.
    Instead of using Inheritance to distinguish Replica types, 
    a ~type~ attribute is used (similar to the ~type~ attribute in 
    ~DBService::Node~).

    In some cases, e.g. when loaded, Replica may not load the dependent objects
    such as their target Nodes. This is because these objects have already
    been loaded and therefore duplicate in-memory copies are to be avoided.

    A workaround would be to introduce a Record-cache holding a list of all
    in-memory objects that can be used when loading records to avoid in-memory
    duplicates. However, this creates unnecessary complexity as in-memory
    copies and copies loaded from the db may - in theory - be out of sync and,
    beside of this, is additional implementation effort.

    Instead, the Replica will provide ids to look dependent objects up, e.g.
    using the ~NodeManager~ to look up target nodes.
  */
  class Replica : public Record<Replica, SecondoReplicaAdapter> {
    
    private:

    /*
      The original Relation corresponding to the given replica.

      Maps to RelationId: int.
    */
    std::shared_ptr<DBService::Relation> relation;

    int relationId;

    std::shared_ptr<DBService::Derivative> derivative;

    int derivativeId;

    /* 
      Target location of the replication represented as node.
      Target nodes belong to the DBService Node group.

      Maps to TargetNodeId: int.
    */    
    std::shared_ptr<DBService::Node> targetNode;

    int targetNodeId;

    /*
      Status of the replication.
      
      Maps to: Status: int.

      Valid values are: waiting, replicated, failed.
    */
    std::string status;

    /*
      Replica type can either be ~Replica::replicaTypeRelation~ or
      ~Replica::replicaTypeDerivative~.
    */
    std::string type;

    public:

    //TODO Make usage of constants for config purposes uniform across 
    //  RecordTypes
    static std::string statusWaiting;
    static std::string statusReplicated;
    static std::string statusFailed;

    static std::string replicaTypeRelation;
    static std::string replicaTypeDerivative;

    Replica();

    std::string getStatus() const;    

    void setStatusWaiting();
    void setStatusReplicated();
    void setStatusFailed();
    void setStatus(std::string status);

    void setType(std::string newType);
    std::string getType() const;

    /*
      When a Replica is being loaded only the targetNodeId will be set.
      The targetNode will not be loaded eagerly.


      Note that there is no "dirty checking" for ~targetNode~.
      The ~targetId~ is used for loaded records, only.

      The idea is to use the ~NodeManager~ to load the ~targetNode~.
      This will ensure that all loaded nodes are centally managed by
      the ~NodeManager~. This way all references to nodes handed out by
      the ~NodeManager~ are shared_ptrs to unique ~Node~ objects.

      This avoids ~Node~ duplicates loaded several times from the 
      database which may lead to confusion.
    */  
    void setTargetNodeId(int newTargetNodeId);
    int getTargetNodeId() const;

    /*
      Analog to setTargetNodeId.
    */
    void setRelationId(int newRelationId);
    int getRelationId() const;    
    std::shared_ptr<DBService::Relation> getRelation() const;
    
    /*
      Sets a new Relation.
      If ~force = false~ then changing the Relation will 
      mark the record ~dirty~.

      If ~force = true~ dirty checking is disabled.
    */
    void setRelation(std::shared_ptr<DBService::Relation> newRelation, 
      bool force = false);

    void setDerivativeId(int newDerivativeId);
    int getDerivativeId() const;

    void setDerivative(std::shared_ptr<Derivative> newDerivative,
      bool force = false);
      
    std::shared_ptr<Derivative> getDerivative() const;

    /*
      One line string output.
    */
    std::string str(int indentationLevel = 0) const;

    std::shared_ptr<DBService::Node> getTargetNode() const;

    /*
      Sets the target Node for a new Replica.
      Once saved the target Node can't be changed as this may lead
      to an inconsistency with the actual replica location.
    */
    void setTargetNode(std::shared_ptr<DBService::Node> newTargetNode);



    // Record
    static std::string getRelationName();
    static std::string createRelationStatement();

    std::string createStatement() const;
    std::string updateStatement() const;

    static Query query(std::string database);

    bool operator==(const Replica &other) const;
    bool operator!=(const Replica &other) const;

    void beforeDestroy();

    /*
      Retrieves Replicas associated to the Relation specified by its relationId.
    */
    static std::vector<std::shared_ptr<Replica> > findByRelationId(
      std::string database, int relationId);

    static std::vector<std::shared_ptr<Replica> > findByRelationIdAndType(
      std::string database, int relationId, std::string replicaType);

    static std::vector<std::shared_ptr<Replica> > findByDerivativeId(
      std::string database, int derivativeId);
  };
  
  std::ostream &operator<<(std::ostream &os, Replica const &replica);
}

#endif