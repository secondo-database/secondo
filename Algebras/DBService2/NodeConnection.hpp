#ifndef NODE_CONNECTION_H
#define NODE_CONNECTION_H

#include  "Algebras/Distributed2/ConnectionInfo.h"

#include <boost/thread/mutex.hpp>

namespace DBService {

  /**
The NodeConnection link a node with a ConnectionInfo.
This way a 
   */
  class NodeConnection {        
    
    private:
    std::shared_ptr<distributed2::ConnectionInfo> connection;    

    std::string host;
    int port;
    std::string config;

    //TODO a lock specific to a particular host & port connection would be
    //  more efficient. However, there could be several NodeConnection objects
    //  representing connections to a particular node (host, port).
    //  Locking all connections across all NodeConnections is wasteful but 
    //  simple.
    // Alternatively, a NodeConnection pool could be implemented ensuring that
    //  a node-connection to a particulat host, port is unique and whenever 
    //  threads want to share access, a shared pointer to the object is created
    //  secured by a lock on the level of the NodeConnection object.
    static boost::mutex connectionMutex;
    
    void createAndSelectRemoteDBServiceDatabase();

    /*
      Perform a remote query to receive a config param a string.
      Intended to be invoked by obtainRemoteConfigParamX methods.
    */
    void getRemoteConfigParam(std::string& result,
        distributed2::ConnectionInfo* connectionInfo, const char* section,
        const char* key);

    public:

    NodeConnection(std::string newHost, int newPort, std::string newConfig);

    std::shared_ptr<distributed2::ConnectionInfo> getConnectionInfo();

    void connect();
    void setConnectionInfo(std::shared_ptr<distributed2::ConnectionInfo> 
      newConnection);    

    int obtainRemoteConfigParamComPort();
    int obtainRemoteConfigParamTransferPort();
    std::string obtainRemoteConfigParamDiskPath();
    

    /*
      Checks if the connection member is not the nullptr.
    */
    bool isConnected();


    /*
      Starts the DBService worker on the given remote node.
    */
    void startDBServiceWorker();
  };
}
#endif