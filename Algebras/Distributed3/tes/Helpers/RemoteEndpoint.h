/*

*/

#ifndef SECONDO_TES_REMOTEHOST_H
#define SECONDO_TES_REMOTEHOST_H


#include <string>
#include <ostream>

namespace distributed3 {
 struct RemoteEndpoint {
  RemoteEndpoint(const std::string &host, const int port);

  friend std::ostream& operator<<(std::ostream& os, 
                                const distributed3::RemoteEndpoint& endpoint);
  
  bool operator==(const RemoteEndpoint &rhs) const;

  bool operator!=(const RemoteEndpoint &rhs) const;

  const std::string host;
  const int port;
 };
}


#endif //SECONDO_TES_REMOTEHOST_H
