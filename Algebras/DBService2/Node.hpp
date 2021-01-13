#ifndef DBS_NODE_H
#define DBS_NODE_H

#include <std::string>
#include <iostream>

#include "Algebras/DBService2/Record.hpp"
#include "Algebras/DBService2/Host.hpp"
#include "Algebras/DBService2/NodeConnection.hpp"
#include "Algebras/DBService2/SecondoNodeAdapter.hpp"
#include "Algebras/DBService2/Query.hpp"

namespace DBService {
  
  // Forward declaration
  class NodeConnection;
  class SecondoNodeAdapter;

  class Node : public Record<DBService::Node, SecondoNodeAdapter> {

    //TODO Make private
    protected:
    
    Host host;
    int port;
    std::string config;
    std::string diskPath;
    int comPort;
    int transferPort;

    std::string type;

    std::shared_ptr<NodeConnection> nodeConnection;

    public:
    Node();
    
    /*
      Constructor used during addNode assuming that node->connectAndConfigure
      is used to determine attributes such as comPort, transferPort and 
      diskPath.      
    */
    Node(const std::string newHost,
              const int newPort,
              std::string newConfig);

    Node(const std::string newHost,
              const int newPort,
              std::string newConfig, std::string diskPath, int comPort, int transferPort);

    Node(const Host newHost,
         const int newPort,
         std::string newConfig, std::string diskPath, int comPort, int transferPort);
    
    Node(const Node&);
    Node(Node&&);

    std::string getType() const;
    void setType(std::string newType);

    Host getHost() const;
    std::string getConfig() const;
    int getPort() const;
    std::string getDiskPath() const;
    int getComPort() const;
    int getTransferPort() const;
    std::shared_ptr<NodeConnection> getConnection();

    void setHost(std::string newHost);
    void setHost(Host newHost);

    void setConfig(std::string newConfig);
    void setPort(int newPort);
    void setDiskPath(std::string newDiskPath);
    void setComPort(int newComPort);
    void setTransferPort(int newTransferPort);
    void setConnection(std::shared_ptr<NodeConnection> newConnection);

    /*
      Returns host and port as a single line std::string.
      Indents the output according to the provided ~indentationLevel~ using 
      an according number of tabs.
    */
    std::std::string str(int indentationLevel = 0) const;

    /**
     * Connects to the node, creates & sets the corresponding nodeConnection and
     * configures the node with remote config params such as comPort, transferPort
     * and diskPath obtained from the remote node.
     */
    void connectAndConfigure();
    void startWorker();


    // For comparing worker nodes
    bool operator==(const Node &w2) const;
    bool operator!=(const Node &w2) const;

    bool empty() const;

    /*
      Checks if a nodeConnection is available.
      Does not tell whether the connection is alive.
    */
    bool isConnected();

    static std::string getRelationName();
    static std::string createRelationStatement();

    std::string createStatement() const;

    /*
      At this point Node's cannot be updated.
      The updateStatement returns a non-mutating query.
      
      Note that ~beforeSave~ and ~afterSave~ callbacks 
      are being triggered.

      TODO Implement updateStatement.
    */
    std::string updateStatement() const;
    
    //TODO Remove
    // void createRelation() override;

    //TODO Remove
    // Deprecatated. use findAllQuery instead.
    static std::string findAllStatement(std::std::string database);    

    // static std::string deleteAllStatement();    

    //TODO What is a better way to create those constants?
    static std::string nodeTypeDBService();
    static std::string nodeTypeOriginal();
    // static Query query(std::string database);

    // static std::vector<std::std::shared_ptr<Node> > findAll(std::string database);
    // static std::std::shared_ptr<Node> findByTid(std::string database, int tid);    
  };

  // Print node
  std::ostream &operator<<(std::ostream &os, Node const &node);
}

#endif