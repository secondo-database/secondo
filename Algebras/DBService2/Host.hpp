#ifndef HOST_H
#define HOST_H

#include <std::string>

namespace DBService {
  class Host {

    protected:
    std::string host;

    public:
    Host();
    Host(std::string host);
    Host(Host &host);
    Host(const Host &host);

    std::string getHostname() const;
    void setHostname(std::string newHost);

    bool operator==(const Host &h2) const;
    bool operator!=(const Host &h2) const;
  };
}
#endif