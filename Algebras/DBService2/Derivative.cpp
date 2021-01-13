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
#include "Algebras/DBService2/Derivative.hpp"

#include <sstream>

using namespace std;

namespace DBService {
  Derivative::Derivative() : Record::Record() {
    setName("");
    setDatabase("");
    setFunction("");    
  }

  Derivative::Derivative(string newName, string newFunction) : 
    Record::Record() { 
  
    setName(newName);
    setFunction(newFunction);
  } 

  Derivative::Derivative(string newName, 
    string newFunction, shared_ptr<DBService::Relation> newRelation) : 
    Record::Record() {

    setName(newName); 
    setFunction(newFunction);
    setRelation(newRelation);
  } 
  
  shared_ptr<Derivative> Derivative::build(string newName, string newFunction) {
    shared_ptr<Derivative> instance(new Derivative(newName, newFunction));
    return instance;
  }

  shared_ptr<Derivative> Derivative::build(string newName, 
    string newFunction, shared_ptr<DBService::Relation> newRelation) {
    
    shared_ptr<Derivative> instance(
      new Derivative(newName, newFunction, newRelation));

    return instance;
  }

  string Derivative::getName() const {
    return name;
  }

  void Derivative::setName(std::string newName) {
    if (newName != name) {
      name = newName;
      setDirty();
    }
  }

  string Derivative::getFunction() const {
    return function;
  }

  void Derivative::setFunction(std::string newFunction) {
    if (newFunction != function) {
      function = newFunction;
      setDirty();
    }
  }

  std::shared_ptr<DBService::Relation> Derivative::getRelation() const {
    return relation;
  }
  
  void Derivative::setRelation(
    std::shared_ptr<DBService::Relation> newRelation, bool force) {

    if (newRelation == nullptr)
      throw "Can't execute setRelation(nullptr)!";

    if (newRelation != relation) {
      relation = newRelation;

      if (force == false)
        setDirty();
    }
  }

  void Derivative::syncReplicasWithRelation() { 
    if (relation->getReplicaCount() > getReplicaCount()) {
      addDerivativeReplicas(relation->getReplicas());
    }
  }

  /*
    Creates a derivative Replica based on the provided relation Replica.
    Prevents adding Replica duplicates.
  */  
  void Derivative::addDerivativeReplica(
    std::shared_ptr<Replica> relationReplica) {

    shared_ptr<Replica> derivativeReplica = make_shared<Replica>();

    derivativeReplica->setDatabase(getDatabase());
    derivativeReplica->setType(Replica::replicaTypeDerivative);
    derivativeReplica->setDerivativeId(getId());
    derivativeReplica->setTargetNode(relationReplica->getTargetNode());
    derivativeReplica->setRelation(relationReplica->getRelation());
    derivativeReplica->setDerivative(shared_from_this());

    if (!doesReplicaExist(derivativeReplica)) {  
      replicas.push_back(derivativeReplica);
      setDirty();
    }
  }

  //TODO Remove code duplicity with Relation
  bool Derivative::doesReplicaExist(shared_ptr<Replica> newReplica) {    
    for( const auto& replica : replicas) {
      if (replica == newReplica)
        return true;
    }

    return false;
  }

  /*
    This function is meant to be called once the Derivative has been created.
    It is assumed that this happens after relation-Replicas have been added to 
    the Relation.

    Furthermore the amount of Replicas is assumed to be a system-wide static 
    setting that won't change over time.
  */
  void Derivative::addDerivativeReplicas(std::vector<std::shared_ptr<Replica> > 
      relationReplicas) {
      
      for(auto& relationReplica : relationReplicas) {
        addDerivativeReplica(relationReplica);
      }
    }

  int Derivative::getReplicaCount() const {
    return replicas.size();
  }

  void Derivative::loadReplicas() {
    if (replicas.empty()) {
      vector<shared_ptr<Replica> > shrPtrReplicas = 
        Replica::findByDerivativeId(
          database, getId());      
      
      for( shared_ptr<Replica> shrdPtrReplica : shrPtrReplicas) {        
        
        // true -> avoid dirty checking by forcefully setting the Relation
        // shrdPtrReplica->setDerivative(shared_from_this(), true);
        
        shrdPtrReplica->setDerivativeId(getId());
        shrdPtrReplica->setDerivative(shared_from_this());

        replicas.push_back(shrdPtrReplica);
      }
    }
  }

  vector<shared_ptr<Replica> > Derivative::getReplicas() const {
    return replicas;
  }

  void Derivative::saveReplicas() {
    if (getReplicaCount() <= 0)
      return;
    
    for( auto& replica : getReplicas()) {
      if (replica->getIsDirty()) {
        replica->setDatabase(getDatabase());

        // If the derivative has been saved in the meanwhile, there is now an
        // id != -1 which will be needed to retrieve the replica later.
        replica->setDerivativeId(getId());

        replica->save();
      }
    }
  }

  shared_ptr<Replica> Derivative::findReplica(string targetHost, 
    int targetPort) {
    
    DBService::Node nodeToFind(
      as_const(targetHost), as_const(targetPort), "");

    for (auto& replica : replicas) {        
      if (*replica->getTargetNode() == nodeToFind) {
        return replica;
      }
    }

    return nullptr;
  }

  void Derivative::updateReplicaStatus(string targetHost, int targetPort, 
      string replicaStatus) {

    shared_ptr<Replica> replica = findReplica(
      targetHost, targetPort);

    if (replica == nullptr)
      throw "Couldn't find replica by targetHost and targetPort.";
    
    replica->setStatus(replicaStatus);
    replica->save();
  }

  string Derivative::str(int indentationLevel) const {
    string ind = std::string(2 * indentationLevel, ' ');
    string ind2 = ind + "  ";

    stringstream msg;

    msg << ind << "{" << endl;
    msg << ind2 << "Name: " << getName() << endl;
    msg << ind2 << "Function: " << getFunction() << endl;

    if (relation != nullptr) {
      msg << ind2 << "Relation: " << endl;
      msg << relation->str(indentationLevel + 1) << endl;
    }

    msg << ind2 << "Derivative Replicas: (";
    msg << getReplicaCount() << ");" << endl;

    if (getReplicaCount() > 0) {
      for (const auto& replica : replicas) {
        msg << replica->str((indentationLevel + 1)) << endl;
      }
    }
    
    msg << ind << "}";

    return msg.str();
  }

  string Derivative::getRelationName() {
    return "dbs_derivatives";
  }

  string Derivative::createRelationStatement() {
    return "let " + getRelationName() + " = [const rel(tuple([Name: string, \
      Function: string, RelationId: int])) value ()]";
  }

  string Derivative::createStatement() const {
    const string quote = "\"";    

    stringstream createStatement;

    if (relation == nullptr)
      throw "Can't execute Derivative::createStatement() without relation!";

    createStatement << "query " << Derivative::getRelationName();
    createStatement << " inserttuple" << "[";

    createStatement << quote << getName() << quote << ", ";
    createStatement << quote << getFunction() << quote << ", ";
    createStatement << relation->getId();

    createStatement << "] consume";
    return createStatement.str();
  }

  string Derivative::updateStatement() const {

    // A neutral, non-mutating query as a Derivative can't be updated.
    return "query 1";
  }

  Query Derivative::query(string recordDatabase) {
    stringstream query;

    query << "query" << " " << getRelationName();

    return Query(recordDatabase, query.str());
  }

  void Derivative::beforeSave() { }
  void Derivative::afterSave() {
    saveReplicas();
  }

  bool Derivative::empty() const {
    return replicas.size();
  }

  shared_ptr<Derivative> Derivative::findOne(string database, Query query) {
    return query.retrieveObject<Derivative, SecondoDerivativeAdapter>();
  }

  vector<shared_ptr<Derivative> > Derivative::findAll(string database) {
    Query query = Derivative::query(database).feed().addid().consume();
    return query.retrieveVector<Derivative, SecondoDerivativeAdapter>();
  }

  bool Derivative::operator==(const DBService::Derivative &other) const {
    return (getName() == other.getName() && 
      getFunction() == other.getFunction() &&
      *relation == *other.getRelation());
  }
  bool Derivative::operator!=(const DBService::Derivative &other) const {
    return !(getName() == other.getName() && 
      getFunction() == other.getFunction() &&
      *relation == *other.getRelation());
  }

  vector<shared_ptr<Derivative> > Derivative::findByRelationId(
    string database, int relationId) {

    Query query = DBService::Derivative::query(database).feed().filter(
      ".RelationId = ?", relationId).addid().consume();
    return query.retrieveVector<Derivative, SecondoDerivativeAdapter>();
  }

  void Derivative::afterDestroy() {
    deleteReplicas();
  }

  void Derivative::deleteReplicas() {
    for(auto& replica : replicas)
      replica->destroy();
  }

  ostream &operator<<(ostream &os, Derivative const &derivative) {
    os << derivative.str();
  }
}