/*

1.1.1 Class Implementation

----
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
----

*/
#include "Algebras/DBService2/Replica.hpp"
#include "Algebras/DBService2/SecondoReplicaAdapter.hpp"
#include "Algebras/DBService2/TriggerReplicaDeletionRunnable.hpp"

#include <string>
#include <sstream>

using namespace std;

namespace DBService {

  string Replica::statusWaiting     = "waiting";
  string Replica::statusReplicated  = "replicated";
  string Replica::statusFailed      = "failed";

  string Replica::replicaTypeRelation   = "relation";
  string Replica::replicaTypeDerivative = "derivative";

  Replica::Replica() : Record::Record() {
    setStatusWaiting();
    setDatabase("");
    setType(Replica::replicaTypeRelation);
    setDerivativeId(-1);
  }

  void Replica::setStatusWaiting() {
    if (status != statusWaiting) {
      status = statusWaiting;
      setDirty();
    }
  }

  void Replica::setStatusReplicated() {
    if (status != statusReplicated) {
      status = statusReplicated;
      setDirty();
    }
  }

  void Replica::setStatusFailed() {
    if (status != statusFailed) {
      status = statusFailed;
      setDirty();
    }
  }

  void Replica::setStatus(string newStatus) {

    if (status != newStatus) {
      status = newStatus;
      setDirty();
    }
  }

  string Replica::getStatus() const {
    return status;
  }

  string Replica::getType() const {
    return type;
  }

  void Replica::setType(string newType) {
    type = newType;
  }

  shared_ptr<DBService::Node> Replica::getTargetNode() const {
    return targetNode;
  }

  int Replica::getTargetNodeId() const {
    return targetNodeId;
  }

  void Replica::setTargetNodeId(int newTargetNodeId) {
    targetNodeId = newTargetNodeId;
  }
  

  void Replica::setTargetNode(shared_ptr<DBService::Node> newTargetNode) {
    if (newTargetNode == nullptr)
      throw "Can't execute setTargetNode(nullptr)!";
        
    //TODO Remove code duplicity across Record subclasses;
    if (newTargetNode != targetNode) {
      
      targetNode = newTargetNode;        
      targetNodeId = newTargetNode->getId();
      
      if (getIsNew() == true)
        setDirty();      
    }
  }

  int Replica::getRelationId() const {
    return relationId;
  }

  void Replica::setRelationId(int newRelationId) {
    relationId = newRelationId;
  }

  int Replica::getDerivativeId() const {
    return derivativeId;
  }

  void Replica::setDerivativeId(int newDerivativeId) {
    derivativeId = newDerivativeId;
  }
  
  shared_ptr<DBService::Relation> Replica::getRelation() const {
    return relation;
  }

  void Replica::setRelation(shared_ptr<DBService::Relation> newRelation,
    bool force) {

    if (newRelation == nullptr)
      throw "Can't execute setRelation(nullptr)!";

    if (newRelation != relation) {
      if ((getIsNew() == true) || force == true) {
        relation = newRelation;
        relationId = relation->getId();
        
        if (force == false)
          setDirty();
      } else {
        throw "Can't change the relation of a non-new replica!";
      }
    }
  }

  void Replica::setDerivative(shared_ptr<Derivative> newDerivative,
    bool force) {

    if(newDerivative == nullptr)
      throw "Can't execute setDervivative(nullptr)!";

    if(newDerivative != derivative) {
      derivative = newDerivative;

      if(force == false)
        setDirty();
    }
  }

  shared_ptr<Derivative> Replica::getDerivative() const {
    return derivative;
  }

  string Replica::str(int indentationLevel) const {
    string ind = std::string(2 * indentationLevel, ' ');
    string ind2 = ind + "  ";

    /*
     With SecondoPLTTYCS
     These won't work:

      std::string(((4 * indentationLevel) - 2), ' ');
      std::string(--indentationLevel, '\t');

    With indentationLevel = 2

    */
    
    stringstream ret;

    ret << ind << "{" << endl;
    ret << ind2 << "Type: " << getType() << endl;
    ret << ind2 << "Status: " << getStatus() << endl;
    ret << ind2 << "isDirty: " << getIsDirty() << endl;

    ret << ind2 << "TargetNode: " << endl;

    if (targetNode != nullptr)
      ret << targetNode->str( indentationLevel + 1 );
    else
      ret << ind2 << targetNodeId;
    
    ret << endl;
        
    ret << ind2 << "Relation: " << endl;

    if (relation != nullptr)
      ret << relation->str( indentationLevel + 1);
    else
      ret << ind2 << relationId;

    ret << endl;

    ret << ind << "}";

    return ret.str();
  }

  string Replica::getRelationName() {
    return "dbs_replicas";
  }

  string Replica::createRelationStatement() {
    return "let " + getRelationName() + " = [const rel(tuple([RelationId: int, \
      TargetNodeId: int, Status: string, Type: string, DerivativeId: int])) \
      value ()]";    
  }

  string Replica::createStatement() const {
    const string quote = "\"";    
    stringstream createStatement;

    if (getType().empty())
      throw "Can't save Replica without ReplicaType!";

    if (getTargetNode() == nullptr) {
      stringstream msg;
      msg << "Can't execute Replica::createStatement() for " << str();
      msg <<  " without target node!"; 
      throw msg.str();
    }
    
    if (getRelation() == nullptr)
      throw "Can't execute Replica::createStatement() without relation";

    createStatement << "query " << Replica::getRelationName();
    createStatement << " inserttuple" << "[";

    // RelationId: int, TargetNodeId: int, Status: string
    createStatement << relationId << ", ";
    createStatement << targetNodeId << ", ";
    createStatement << quote << getStatus() << quote << ", ";
    createStatement << quote << getType() << quote << ", ";
    createStatement << derivativeId;

    createStatement << "] consume";
    return createStatement.str();
  }

  string Replica::updateStatement() const {
  
    // int relationId   = getRelation()->getId();
    // int targetNodeId = getTargetNode()->getId(); 

    stringstream updatedirect;
    updatedirect << "updatedirect[";
    updatedirect << "RelationId: " << relationId << ", ";
    updatedirect << "TargetNodeId: " << targetNodeId << ", ";
    updatedirect << "Status: \"" << getStatus() << "\"" << ", ";
    updatedirect << "Type: \"" << getType() << "\"" << ", ";
    updatedirect << "DerivativeId: " << derivativeId << "]";

    // query dbs_replicas feed addid filter[.TID = tid(1)] 
    //  dbs_replicas updatedirect [Status: "replicated"] consume
    // query dbs_replicas feed addid filter[.TID = tid(1)] 
    //  project[RelationId,TargetNodeId,Status] dbs_replicas 
    //  updatedirect[Status: "replicated"] consume
    // -> Crash
    // query dbs_replicas feed addid filter[.TID = tid(1)] 
    //  project[RelationId,TargetNodeId,Status] dbs_replicas 
    //  updatedirect[RelationId: 1, TargetNodeId: 1, Status: "replicated"] 
    //  consume
    // -> Crash
    // Without TID:
    // query dbs_replicas feed filter[(.RelationId=1) and (.TargetNodeId=1)] 
    //  dbs_replicas updatedirect[RelationId:1,TargetNodeId:1, 
    //  Status:"replicated"] consume
    // -> Works
    // query dbs_replicas feed filter[.RelationId=1] filter[.TargetNodeId=1] 
    //  dbs_replicas updatedirect[RelationId:1,TargetNodeId:1, 
    //  Status:"replicated"] consume
    // -> Works
    Query updateQuery = Replica::query(getDatabase()).feed().filter(
      ".Type = ?", getType()).filter(".RelationId = ?", relationId).filter(
      ".TargetNodeId = ?", targetNodeId).relation(
        Replica::getRelationName()).appendString(updatedirect.str()).consume();

    return updateQuery.str();
  }

  //TODO Remove code duplication across Record subclasses.
  Query Replica::query(string database) {
    stringstream query;

    query << "query" << " " << getRelationName();

    return Query(database, query.str());
  }

  /*
    Two Replicas are equal if they share the same target Node and
    represent the same Relation.
  */
  bool Replica::operator==(const Replica &other) const {
        
    // Dereference objects to avoid comparing their shared_ptrs.
    return (*getTargetNode() == *other.getTargetNode() 
      && *getRelation() == *other.getRelation()
      && getType() == other.getType()
      );
  }

  bool Replica::operator!=(const Replica &other) const {
   // Dereference objects to avoid comparing their shared_ptrs.
    return (*getTargetNode() != *other.getTargetNode() 
      || *getRelation() != *other.getRelation()
      || getType() != other.getType());
  }

  vector<shared_ptr<Replica> > Replica::findByRelationId(
      string database, int relationId) {
    string type(Replica::replicaTypeRelation);
    Query query = DBService::Replica::query(database).feed().filter(
      ".RelationId = ?", relationId).filter(
        ".Type = ?", type).addid().consume();

    return query.retrieveVector<Replica, SecondoReplicaAdapter>();
  }

  vector<shared_ptr<Replica> > Replica::findByRelationIdAndType(string database,
    int relationId, string replicaType) {

    Query query = DBService::Replica::query(database).feed().filter(
      ".RelationId = ?", relationId).filter(
        ".Type = ?", replicaType).addid().consume();

    return query.retrieveVector<Replica, SecondoReplicaAdapter>();
  }

  vector<shared_ptr<Replica> > Replica::findByDerivativeId(string database, 
    int derivativeId) {
    string replicaType = Replica::replicaTypeDerivative;

    Query query = DBService::Replica::query(database).feed().filter(
      ".DerivativeId = ?", derivativeId).filter(
        ".Type = ?", replicaType).addid().consume();
    return query.retrieveVector<Replica, SecondoReplicaAdapter>();
  }

  void Replica::beforeDestroy() {
    string derivativeName;
    string relationName;

    if (getRelation() == nullptr)
      throw "Can't destroy Replica without a pointer to a Relation";
    
    if (getType() == replicaTypeDerivative) {
      if (getDerivative() == nullptr) {
        throw "Can't destroy Derivative Replica without a ptr to a Derivative";
      } else {
        /* If no derivative is given, derivateName will be "" which will trigger
          the deletion of the entire replica. This unfortunate implicit behavor
          is the reason why it must be verified that the replicaType is truly
          "relation" to avoid accidentally deleting a relation if derivativeName
          accidentally is set to "".
        */
        derivativeName = getDerivative()->getName();
      } 
    }

    relationName = getRelation()->getName();    

    TriggerReplicaDeletionRunnable replicaEraser(
      targetNode->getHost().getHostname(),
      targetNode->getComPort(),
      database, relationName, derivativeName);

    replicaEraser.run();
  }

  ostream &operator<<(ostream &os, Replica const &replica) {
    
    os << replica.str();

    return os;
  }
}