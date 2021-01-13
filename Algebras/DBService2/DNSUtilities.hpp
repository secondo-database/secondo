#include <string>

#include <boost/asio.hpp>

namespace DBService {
  class DNSUtilities {
    public:
    static std::string resolveHostToIP(std::string hostname);
  };
}