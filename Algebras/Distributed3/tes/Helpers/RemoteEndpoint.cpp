/*

*/
#include "RemoteEndpoint.h"
#include <ostream>

distributed3::RemoteEndpoint::RemoteEndpoint(
                                 const std::string &name, const int port)
 :
 host(name), port(port) {}


namespace distributed3 {
std::ostream &
operator<<(std::ostream &os, const RemoteEndpoint &endpoint) {
 os << "host: " << endpoint.host << " port: " << endpoint.port;
 return os;
}
}

bool
distributed3::RemoteEndpoint::operator==(
                                const distributed3::RemoteEndpoint &rhs) const {
 return host == rhs.host &&
        port == rhs.port;
}

bool
distributed3::RemoteEndpoint::operator!=(
                                const distributed3::RemoteEndpoint &rhs) const {
 return !(rhs == *this);
}
