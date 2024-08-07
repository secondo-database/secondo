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
#include <string>

#include "Algebras/DBService2/Host.hpp"
#include "Algebras/DBService2/DNSUtilities.hpp"

using namespace std;

namespace DBService
{
  Host::Host() {
    setHostname("");
  }
  
  Host::Host(string host) {
    setHostname(host);
  }

  Host::Host(Host &newHost) {
    setHostname(newHost.getHostname());
  }

  Host::Host(const Host &newHost)
  {
    setHostname(newHost.getHostname());
  }

  string Host::getHostname() const
  {
    return host;
  }

  void Host::setHostname(string newHost) {
    
    // TODO Host valididty checking
    host = newHost;
  }

  bool Host::operator==(const Host &h2) const
  {
    // Both hosts are empty
    if (getHostname() == "" && h2.getHostname() == "")
    {
      return true;
    }

    // One of two hosts is empty
    if (getHostname() == "" || h2.getHostname() == "")
    {
      return false;
    }

    // Both hosts are non-empty. We can try a DNS resolve...
    return (DNSUtilities::resolveHostToIP(getHostname()) == 
      DNSUtilities::resolveHostToIP(h2.getHostname()));
  }

  bool Host::operator!=(const Host &h2) const {
    return !(*this == h2);
  }
} // namespace DBService