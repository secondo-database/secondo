#ifndef NODE_CONNECTION_H
#define NODE_CONNECTION_H

#include  "Algebras/Distributed2/ConnectionInfo.h"

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
    
    void createAndSelectRemoteDBServiceDatabase();

    /*
      Perform a remote query to receive a config param a string.
      Intended to be invoked by obtainRemoteConfigParamX methods.
    */
    void getRemoteConfigParam(string& result,
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