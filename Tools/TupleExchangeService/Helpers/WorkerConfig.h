/*

*/
#ifndef SECONDO_TES_WORKERINFO_H
#define SECONDO_TES_WORKERINFO_H

#include <ostream>
#include "RemoteEndpoint.h"
#include "../typedefs.h"
#include <boost/log/trivial.hpp>

namespace distributed3 {
 struct WorkerConfig {

  public:

  std::ostream& print(std::ostream& os) const{
     os << "workernr: " << workernr;
     os << " endpoint: " << endpoint;
     os << " messageServerPort: " << messageServerPort;
     os << " configFilePath: " << configFilePath;
     os << " connection: " << connection;
     return os;
  }
  
  int workernr;
  RemoteEndpoint endpoint;  
  int messageServerPort;
  std::string configFilePath;
  WorkerConnection* connection;
  size_t noReferences; // as in ConnectionInfo

  WorkerConfig(int workernr, 
               const RemoteEndpoint endpoint, 
               int messageServerPort,
               const std::string configFilePath,
               WorkerConnection *connection) : 
                   workernr(workernr), endpoint(endpoint),
                   messageServerPort(
                   messageServerPort),
                   configFilePath(configFilePath),
                   connection(connection) {

     noReferences = 0;
     if(connection) noReferences = 1;
  }

  WorkerConfig(const WorkerConfig& a): 
    workernr(a.workernr), endpoint(a.endpoint),
                                       messageServerPort(a.messageServerPort),
                                       configFilePath(a.configFilePath),
                                       connection(a.connection),
                                       noReferences(a.noReferences) {
     ++noReferences;
  } 

  ~WorkerConfig() {
    connection = nullptr;
  }
 };

}

std::ostream& operator<<(std::ostream &os, 
                         const distributed3::WorkerConfig &config);


#endif //SECONDO_WORKERINFO_H
