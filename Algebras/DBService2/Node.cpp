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
#include "Algebras/DBService2/Node.hpp"
#include "Algebras/DBService2/Query.hpp"
#include "Algebras/DBService2/SecondoNodeAdapter.hpp"

#include <string>
#include <sstream>

using namespace std;

namespace DBService {

  /*
    TODO Implement Node updates.
  */
  Node::Node() : Record::Record()
  {    
    setHost("");
    setPort(0);
    setConfig("");
    setDiskPath("");
    setComPort(0);
    setTransferPort(0);
    setType(Node::nodeTypeDBService());
  }

  Node::Node(const string newHost,
             const int newPort,
             string newConfig) : Record::Record()
  {   
    setHost(newHost);
    setPort(newPort);
    setConfig(newConfig);
    setDiskPath("");
    setComPort(0);
    setTransferPort(0);
    setType(Node::nodeTypeDBService());
  }

  Node::Node(const string newHost,
             const int newPort,
             string newConfig, string newDiskPath, int newComPort, 
             int newTransferPort) : Record::Record()
  {   
    setHost(newHost);
    setPort(newPort);
    setConfig(newConfig);
    setDiskPath(newDiskPath);
    setComPort(newComPort);
    setTransferPort(newTransferPort);
    setType(Node::nodeTypeDBService());
  }

  Node::Node(const Host newHost,
             const int newPort,
             string newConfig, string newDiskPath, int newComPort, 
             int newTransferPort) : Record::Record()
  {
    setHost(newHost);
    setPort(newPort);
    setConfig(newConfig);
    setDiskPath(newDiskPath);
    setComPort(newComPort);
    setTransferPort(newTransferPort);
    setType(Node::nodeTypeDBService());
  }

  Node::Node(const Node &original) : Record(original) {
    setHost(original.getHost());
    setPort(original.getPort());
    setConfig(original.getConfig());
    setDiskPath(original.getDiskPath());
    setComPort(original.getComPort());
    setTransferPort(original.getTransferPort());    
    setType(Node::nodeTypeDBService());
    setDatabase(original.getDatabase());
  }

  Node::Node(Node &&original) : Record(original) {
    setHost(original.getHost());
    setPort(original.getPort());
    setConfig(original.getConfig());
    setDiskPath(original.getDiskPath());
    setComPort(original.getComPort());
    setTransferPort(original.getTransferPort());   
    setType(Node::nodeTypeDBService()); 
    setDatabase(original.getDatabase());
  }

  string Node::getRelationName() {
    return "dbs_nodes";
  }

  string Node::getType() const {
    return type;
  }

  void Node::setType(string newType) {
    if (newType != getType()) {
      if (getIsNew() == true) {
        type = newType;
      } else {
        throw "Can't change the type of a non-new record.";
      }
    }
  }

  Host Node::getHost() const {
    return host;
  }

  string Node::getConfig() const {
    return config;
  }

  int Node::getPort() const {
    return port;
  }

  string Node::getDiskPath() const
  {
    return diskPath;
  }

  int Node::getComPort() const
  {
    return comPort;
  }

  int Node::getTransferPort() const
  {
    return transferPort;
  }

  shared_ptr<NodeConnection> Node::getConnection() {
    return nodeConnection;
  }

  void Node::setHost(string newHostname)
  {
    Host newHost = Host(newHostname);
    return setHost(newHost);
  }

  void Node::setHost(Host newHost)
  {
    if (newHost != getHost())
    {
      host = newHost;
      setDirty();
    }
  }

  void Node::setConfig(string newConfig)
  {
    if (newConfig != getConfig()) {
      config = newConfig;
      setDirty();
    }
  }

  void Node::setPort(int newPort)
  {
    if (newPort != getPort()) {
      port = newPort;
      setDirty();
    }
  }
  
  void Node::setDiskPath(string newDiskPath) {
    if (newDiskPath != getDiskPath()) {
      diskPath = newDiskPath;
      setDirty();
    }
  }

  void Node::setComPort(int newComPort) {
    if (newComPort != getComPort()) {
      comPort = newComPort;
      setDirty();
    }
  }

  void Node::setTransferPort(int newTransferPort) {
    if (newTransferPort != getTransferPort()) {
      transferPort = newTransferPort;
      setDirty();
    }
  }

  void Node::setConnection(shared_ptr<NodeConnection> newConnection) {
    if (newConnection != nodeConnection)
      nodeConnection = newConnection;
  }

  string Node::str(int indentationLevel) const {
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
    ret << ind2 << "Id:" << getId() << endl;
    ret << ind2 << "Host:" << getHost().getHostname() << ", ";
    ret << "Port:" << getPort() << endl;
    ret << ind2 << "Config: " << getConfig() << endl;
    ret << ind2 << "DiskPath: " << getDiskPath() << endl;
    ret << ind2 << "ComPort: " << getComPort() << endl;
    ret << ind2 << "TransferPath: " << getTransferPort() << endl;
    ret << ind << "}";
    return ret.str();
  }

  void Node::connectAndConfigure() {    

    nodeConnection = make_shared<NodeConnection>(
      getHost().getHostname(),
      getPort(),
      getConfig()
    );
    
    nodeConnection->connect();
    
    setComPort(nodeConnection->obtainRemoteConfigParamComPort());
    setTransferPort(nodeConnection->obtainRemoteConfigParamTransferPort());
    setDiskPath(nodeConnection->obtainRemoteConfigParamDiskPath());
  }

  bool Node::isConnected() {
    return (nodeConnection != nullptr && nodeConnection->isConnected());
  }

  void Node::startWorker() {
    if (isConnected() == false) {
      throw "Can't start the DBService worker on a disconnected node.";
    }
    
    nodeConnection->startDBServiceWorker();
  }

  /** 
   * Two different node records - despite of their record id - will be 
   * considered equal as long as they have the same host.
   */
  bool Node::operator==(const Node &w2) const
  {    

    // TODO This does not compare disk paths as they had to be extracted from
    //  the config. Is this ok?

    // Implicit DNS resolution of hosts
    return (getHost() == w2.getHost() && getPort() == w2.getPort());
  }

  bool Node::operator!=(const Node &w2) const
  {
    return !(getHost() == w2.getHost() && getPort() == w2.getPort());
  }

  std::ostream &operator<<(std::ostream &os, Node const &worker) {
    os << "Host: " << worker.getHost().getHostname() << ", Port: ";
    os << worker.getPort() << ", Config: " << worker.getConfig();
    return os;
  }

  bool Node::empty() const {
    //TODO Should the other members be added, too? Or should the method be 
    // renamed to
    //"valid" to indicate if a node has passed its validation.
    return (getHost().getHostname() == "" && getPort() == 0 
      && getConfig() == "");
  }

  //TODO Refactor using the Query class.
  string Node::createStatement() const {
        
    //  inserttuple["sec-w-0.sec-ws.secondo.svc.cluster.local", 1234,
    //  '/database/config/SecondoConfig.ini', '/database/secondo-databases',
    //  9941, 9942, "dbservice"] consume
    stringstream createStatement;

    createStatement << "query " << Node::getRelationName() << " inserttuple";
    createStatement << "[";
    
    const string quote = "\"";
    const string to_text = "totext";

    // Hostname, e.g. totext("sec-w-0.sec-ws.secondo.svc.cluster.local")
    createStatement << to_text << "(" << quote << getHost().getHostname();
    createStatement << quote << ")" << ", ";

    // Secondo Port., e.g. 1234
    createStatement << std::to_string(getPort()) << ", ";

    // Config, e.g. totext("/database/config/SecondoConfig.ini")
    createStatement << to_text << "(" << quote << getConfig() << quote << ")"
                    << ", ";

    // DiskPath, e.g. totext("/database/secondo-databases")
    createStatement << to_text << "(" << quote << getDiskPath() << quote << ")"
                    << ", ";

    // ComPort, e.g. 9941
    createStatement << std::to_string(getComPort()) << ", ";

    // TransferPort, e.g. 9942
    createStatement << std::to_string(getTransferPort()) << ", ";

    createStatement << quote << getType() << quote;

    createStatement << "] consume";
    return createStatement.str();
  }

  string Node::updateStatement() const {

    //TODO Implement later.
    return "query 1";
  }

  //TODO Find an elegant way to use the Query interface to express the creation 
  //  of relations
  string Node::createRelationStatement() {
    //TODO this neglects the existence of schema migrations - the evolution of 
    //  database schemas over time.
    return "let " + getRelationName() + " = [const rel(tuple([Host: text, \
Port: int, Config: text, DiskPath: text, ComPort: int, TransferPort: \
int, Type: string])) value ()]";

  }

  //TODO Refactor using the Query class.
  string Node::findAllStatement(string database) {
    stringstream query;
    
    query << "query " << getRelationName() << " feed addid consume";

    /* Results should look like this:
     * Host : localhost
     * Port : 1245
     * Config :
     * DiskPath : /home/doesnt_exist/secondo
     * ComPort : 9941
     * TransferPort : 9942
     * TID : 4
     */

    return query.str();
  }

  string Node::nodeTypeDBService() {
    return "dbservice";
  }

  string Node::nodeTypeOriginal() {
    return "original";
  } 
}