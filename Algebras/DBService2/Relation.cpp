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
#include "Algebras/DBService2/Relation.hpp"
#include "Algebras/DBService2/Query.hpp"
#include "Algebras/DBService2/SecondoRelationAdapter.hpp"
#include "Algebras/DBService2/Node.hpp"

#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

#include <boost/algorithm/string.hpp>

using namespace std;

namespace DBService {

  //class Node;

  Relation::Relation() : Record::Record() {
    setName("");
    setDatabase("");
  }

  Relation::Relation(string newRelationDatabase, string relationName) :
    Record::Record() {

    setRelationDatabase(newRelationDatabase);
    setName(relationName);
  }

  Relation::Relation(string newRelationDatabase, string relationName,
    shared_ptr<DBService::Node> newOriginalNode) : Record::Record() {

    setRelationDatabase(newRelationDatabase);
    setName(relationName);
    setOriginalNode(newOriginalNode);
  }

  Relation::Relation(string relationDatabase, string relationName,
    string originalNodeHost, int originalNodePort,
    string originalNodeDiskPath) : Record::Record() {

    // At this point we don't have a config -> ""
    shared_ptr<DBService::Node> newOriginalNode = make_shared<DBService::Node>(
      originalNodeHost, originalNodePort, "");

    newOriginalNode->setDiskPath(originalNodeDiskPath);

    setRelationDatabase(relationDatabase);
    setName(relationName);
    setOriginalNode(newOriginalNode);
  }

  shared_ptr<DBService::Relation> Relation::build() {
    shared_ptr<DBService::Relation> instance(new DBService::Relation());
    return instance;
  }

  shared_ptr<DBService::Relation> Relation::build(string relationDatabase,
    string relationName) {

    shared_ptr<DBService::Relation> instance(new DBService::Relation(
      relationDatabase, relationName));

    return instance;
  }

  shared_ptr<DBService::Relation> Relation::build(string relationDatabase,
    string relationName, shared_ptr<DBService::Node> originalNode) {

    shared_ptr<DBService::Relation> instance(new DBService::Relation(
      relationDatabase, relationName, originalNode));

    return instance;
  }

  shared_ptr<DBService::Relation> Relation::build(string relationDatabase,
    string relationName, string originalNodeHost, int originalNodePort,
    string originalNodeDisk) {

    shared_ptr<DBService::Relation> instance(new DBService::Relation(
      relationDatabase, relationName, originalNodeHost, originalNodePort,
      originalNodeDisk));

    instance->getOriginalNode()->setType(Node::nodeTypeOriginal());

    return instance;
  }

  string Relation::getName() const {
    return name;
  }

  void Relation::setName(string newName) {
    if(newName != getName()) {
      if(getIsNew() == true) {
        name = newName;
        setDirty();
      }
      else {
        throw "Can't set relation name for non-new relation!";
      }
    }
  }

  vector<shared_ptr<Replica> > Relation::getReplicas() const {
    return replicas;
  }

  int Relation::getReplicaCount() const {
    return replicas.size();
  }

  void Relation::addReplica(shared_ptr<Replica> newReplica) {
    newReplica->setDatabase(getDatabase());

    // https://stackoverflow.com/questions/11711034/stdshared-ptr-of-this
    newReplica->setRelation(shared_from_this());

    if(!doesReplicaExist(newReplica)) {
      replicas.push_back(newReplica);
      setDirty();
    }
    else {
      throw "Can't add replica. Replica already exists!";
    }
  }

  void Relation::addReplica(shared_ptr<DBService::Node> targetNode) {
    shared_ptr<Replica> replica = make_shared<Replica>();
    replica->setTargetNode(targetNode);

    // Also sets db and relation. Marks the relation as dirty.
    addReplica(replica);
  }

  bool Relation::doesReplicaExist(shared_ptr<Replica> newReplica) {
    //TODO
    // if (find(replicas.begin(), replicas.end(), newReplica) != replicas.end())

    for(const auto& replica : replicas) {
      if(replica == newReplica)
        return true;
    }

    return false;
  }

  shared_ptr<Replica> Relation::findReplica(string targetHost,
    int targetPort, std::string replicaType) {

    /*
      In order to compare nodes including DNS name resolution
      the comparison operator of Node is being used.
    */

    DBService::Node nodeToFind(
      as_const(targetHost), as_const(targetPort), "");
    for(auto& replica : replicas) {
      if(replica->getType() == replicaType &&
        *replica->getTargetNode() == nodeToFind) {
        return replica;
      }
    }

    return nullptr;
  }

  shared_ptr<Replica> Relation::findReplicaByDerivativeId(string targetHost,
    int targetPort, int derivativeId) {

    string replicaType = Replica::replicaTypeDerivative;

    /*
      In order to compare nodes including DNS name resolution
      the comparison operator of Node is being used.
    */

    DBService::Node nodeToFind(
      as_const(targetHost), as_const(targetPort), "");

    for(auto& replica : replicas) {
      if(replica->getType() == replicaType &&
        *replica->getTargetNode() == nodeToFind &&
        replica->getDerivativeId() == derivativeId) {

        return replica;
      }
    }

    return nullptr;
  }


  void Relation::updateReplicaStatus(string targetHost, int targetPort,
    std::string replicaStatus) {

    shared_ptr<Replica> replica = findReplica(
      targetHost, targetPort);

    if(replica == nullptr)
      throw "Couldn't find replica by targetHost and targetPort.";

    replica->setStatus(replicaStatus);
    replica->save();
  }

  void Relation::resetReplicas() {
    shared_ptr<DBService::Replica> replica;

    for(int i = 0; i < replicas.size(); i++) {
      replica = replicas[i];

      if(replica->getIsDirty() == true) {
        replicas.erase(replicas.begin() + i);
      }
    }
  }

  shared_ptr<Replica> Relation::getRandomReplica() const {
    // If the relation has no replicas
    if(getReplicaCount() < 1)
      return nullptr;

    auto replicas = getReplicas();

    // Selecting random Replica
    srand(time(0));
    int randomIndex = rand() % replicas.size();

    return replicas[randomIndex];
  }

  /*
    Set the id of the original Node and load the node.
    Does nothing if an original Node has already been loaded.
    => There is not reload functionality.
  */
  void Relation::loadOriginalNode(int originalNodeId) {
    if(originalNode == nullptr) {
      /*
        The node manager has already loaded nodes.
        It should be avoided that objects exist several times in memory.
        This may lead to inconsistencies.
        Marking a record as read-only could also prevent this.

        An original node may have to be created during replication along with
        its relation.
        > a new relation with a new node will be saved.

      */
      //using Record::findByTid;
      originalNode = DBService::Node::findByTid(database, originalNodeId);
      if(originalNode == nullptr)
        throw "Couldn't load the Relation's originalNode: " + str();
    }
  }

  /*
    Load the Relation's replicas.
    Does not load derivative-replicas into the ~replicas~ vector.
  */
  void Relation::loadReplicas() {
    if(replicas.empty()) {
      // Relation.cpp:113:16: error: no viable overloaded '='
      // replicas = Replica::findByRelationId(database, getId());

      vector<shared_ptr<Replica> > shrPtrReplicas = Replica::findByRelationId(
        database, getId());

      for(shared_ptr<Replica> shrdPtrReplica : shrPtrReplicas) {
        // true -> avoid dirty checking by forcefully setting the Relation
        shrdPtrReplica->setRelation(shared_from_this(), true);
        replicas.push_back(shrdPtrReplica);
      }
    }
  }

  void Relation::setOriginalNode(shared_ptr<DBService::Node> newOriginalNode) {
    if(newOriginalNode != getOriginalNode()) {
      if(getIsNew() == true) {
        originalNode = newOriginalNode;
        originalNode->setDatabase(database);
      }
      else {
        throw "Can't change Original Node for non-new Relation!";
      }
    }
  }

  shared_ptr<DBService::Node> Relation::getOriginalNode() const {
    return originalNode;
  }

  bool Relation::operator==(const Relation& other) const {
    return (boost::to_upper_copy(getRelationDatabase()) == boost::to_upper_copy(
      other.getRelationDatabase()) && getName() == other.getName());
  }

  bool Relation::operator!=(const Relation& other) const {
    return (boost::to_upper_copy(getRelationDatabase()) != boost::to_upper_copy(
      other.getRelationDatabase()) || getName() != other.getName());
  }

  string Relation::getRelationName() {
    return "dbs_relations";
  }

  string Relation::getRelationDatabase() const {
    return relationDatabase;
  }

  void Relation::setRelationDatabase(string newRelationDatabase) {
    if(newRelationDatabase != relationDatabase) {
      if(getIsNew() == true) {
        relationDatabase = newRelationDatabase;

        // Comparison of DB names is done in uppercase
        boost::to_upper(relationDatabase);
        setDirty();
      }
      else {
        throw "Can't change the relation-database for a non-new Relation!";
      }
    }
  }

  // Derivatives START

  shared_ptr<DBService::Derivative> Relation::addDerivative(
    string derivativeName, string function) {
    shared_ptr<Derivative> derivative = DBService::Derivative::build(
      derivativeName, function, shared_from_this());

    derivative->setDatabase(getDatabase());

    /*
      Add a derivative-replica per relation-replica using the corresponding
      target node.
    */
    derivative->syncReplicasWithRelation();

    derivatives.push_back(derivative);

    setDirty();

    return derivative;
  }

  int Relation::getDerivativeCount() const {
    return derivatives.size();
  }

  vector<shared_ptr<Derivative> > Relation::getDerivatives() {
    return derivatives;
  }

  void Relation::loadDerivatives() {
    /*
      TODO There is a strong similarity between Replicas and Derivatives
        as both are dependent (composite) objects. Thus there is a lot of code
        duplication which can surely be further reduced.
    */

    if(derivatives.empty()) {
      // Relation.cpp:113:16: error: no viable overloaded '='
      // replicas = Replica::findByRelationId(database, getId());

      vector<shared_ptr<Derivative> > loadedDerivatives = 
        Derivative::findByRelationId(database, getId());

      for(shared_ptr<Derivative> derivative : loadedDerivatives) {

        // true -> avoid dirty checking by forcefully setting the Relation
        derivative->setRelation(shared_from_this(), true);
        derivatives.push_back(derivative);
      }
    }
  }

  shared_ptr<Derivative> Relation::findDerivative(int derivativeId) {
    for(auto& derivative : derivatives) {
      if(derivative->getId() == derivativeId)
        return derivative;
    }
    return nullptr;
  }

  shared_ptr<Derivative> Relation::findDerivative(string derivativeName) {
    for(auto& derivative : derivatives) {
      if(derivative->getName() == derivativeName)
        return derivative;
    }
    return nullptr;
  }

  void Relation::updateDerivativeReplicaStatus(string derivativeName,
    string targetHost, int targetPort, string replicaStatus) {

    auto derivative = findDerivative(derivativeName);
    derivative->updateReplicaStatus(targetHost, targetPort, replicaStatus);
  }

  bool Relation::doesDerivativeExist(std::string derivativeName) {
    for(auto& derivative : derivatives) {
      if(derivative->getName() == derivativeName)
        return true;
    }
    return false;
  }

  // Derivatives END

  string Relation::str(int indentationLevel) const {
    string ind = std::string(2 * indentationLevel, ' ');
    string ind2 = ind + "  ";

    stringstream ret;

    ret << ind << "{" << endl;
    ret << ind2 << "RelationName: " << getName() << ", " << endl;
    ret << ind2 << "RelationDatabase:" << getRelationDatabase() << endl;
    ret << ind << "}";

    return ret.str();
  }

  string Relation::createRelationStatement() {
    //TODO this neglects the existence of schema migrations - 
    //  the evolution of database schemas over time.
    return "let " + getRelationName() + " = [const rel(tuple([Name: string, \
Database: string, OriginalNodeId: int])) value ()]";

  }

  string Relation::createStatement() const {
    const string quote = "\"";

    stringstream createStatement;

    if(getOriginalNode() == nullptr)
      throw "Can't execute Relation::createStatement() without original Node!";

    if(getOriginalNode()->getIsNew() == true)
      throw "Can't execute Relation::createStatement() with an unsaved \
original Node!";

    createStatement << "query " << Relation::getRelationName();
    createStatement << " inserttuple" << "[";

    createStatement << quote << getName() << quote << ", ";
    createStatement << quote << getRelationDatabase() << quote << ", ";
    createStatement << originalNode->getId();

    createStatement << "] consume";
    return createStatement.str();
  }

  string Relation::updateStatement() const {
    // A neutral, non-mutating query as an Relation can't be updated.
    return "query 1";
  }

  ostream& operator<<(ostream& os, Relation& relation) {
    int indentationLevel = 1;
    string ind = std::string(2 * indentationLevel, ' ');
    string ind2 = ind + "  ";

    os << ind << "{" << endl;
    os << ind2 << "Relation Name: " << relation.getName() << endl;
    os << ind2 << "Relation Database: " << relation.getRelationDatabase();
    os << endl;

    if(relation.getOriginalNode() != nullptr) {
      os << ind2 << "Target Node: " << endl;
      os << relation.getOriginalNode()->str(indentationLevel + 1) << endl;
    }

    os << ind2 << "Relation Replicas (" << relation.getReplicaCount();
    os << "):" << endl;

    if(relation.getReplicaCount() > 0) {
      for(const auto& replica : relation.getReplicas())
        os << replica->str(indentationLevel + 1) << endl;
    }

    os << ind2 << "Relation Derivatives (";
    os << relation.getDerivativeCount() << "):" << endl;

    if(relation.getDerivativeCount() > 0) {
      for(auto& derivative : relation.getDerivatives()) {
        os << derivative->str(indentationLevel + 1) << endl;
      }
    }

    os << "}" << endl;

    // os << "\tOriginal node id: " << relation.getOriginalNode
    return os;
  }

  Query Relation::query(string database) {
    stringstream query;

    query << "query" << " " << getRelationName();

    return Query(database, query.str());
  }

  Query Relation::queryByDatabaseAndName(std::string database,
    std::string relationDatabase, std::string relationName) {

    return DBService::Relation::query(database).feed().filter(".Database = ?", 
      boost::to_upper_copy(relationDatabase)).filter(".Name = ?", 
      relationName).addid().consume();
  }

  string Relation::findAllStatement(string database) {

    //TODO passing an empty query is dangerous as the resulting query is not 
    //  executable.
    //  Maybe there should be a database parameter and the function should be
    //  renamed to findAll instead of findAllStatement!
    return Relation::query(database).feed().addid().consume().str();
  }

  vector<shared_ptr<Relation> > Relation::findAll(string database) {
    Query query = Relation::query(database).feed().addid().consume();
    return query.retrieveVector<Relation, SecondoRelationAdapter>();
  }

  //TODO Remove database field as it is redundant to 
  //  the one contained in the query object.
  //TODO Establish findAll(...) and findOne(...) for all Record types.
  shared_ptr<Relation> Relation::findOne(string database, Query findQuery) {
    return findQuery.retrieveObject<Relation, SecondoRelationAdapter>();
  }

  void Relation::beforeSave() {
    if(originalNode != nullptr) {
      /*
        Ensure that the original Nodes' database is equal to the Relations'
        database which may get out of sync as setDatabase from the Record
        class template does not cascade setDatabase to the Relation's
        originalNode.
      */
      originalNode->setDatabase(getDatabase());
      originalNode->save();
    }
  }

  void Relation::afterSave() {
    saveReplicas();
    saveDerivatives();
  }

  void Relation::saveReplicas() {
    if(getReplicaCount() <= 0)
      return;

    for(auto& replica : getReplicas()) {
      if(replica->getIsDirty()) {
        replica->setDatabase(getDatabase());
        replica->setRelationId(getId());
        replica->save();
      }
    }
  }

  void Relation::saveDerivatives() {
    if(getDerivativeCount() <= 0)
      return;

    for(auto& derivative : getDerivatives()) {
      if(derivative->getIsDirty()) {
        derivative->setDatabase(getDatabase());
        derivative->save();
      }
    }
  }

  bool Relation::empty() const {
    return (getName() == "" && getRelationDatabase() == "");
  }

  void Relation::afterDestroy() {
    deleteReplicas();
    deleteDerivatives();
  }

  void Relation::deleteDerivatives() {
    for(auto& derivative : derivatives)
      derivative->destroy();
  }

  void Relation::deleteReplicas() {
    for(auto& replica : replicas)
      replica->destroy();
  }

  void Relation::deleteDerivative(std::string derivativeName) {
    auto derivative = findDerivative(derivativeName);

    // Also remove derivative from the derivatives vector
    if(derivative != nullptr) {
      derivative->destroy();
      derivatives.erase(std::remove(derivatives.begin(), derivatives.end(),
        derivative), derivatives.end());
    }
  }
}