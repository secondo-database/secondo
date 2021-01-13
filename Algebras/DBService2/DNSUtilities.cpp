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

#include "Algebras/DBService2/DNSUtilities.hpp"

using namespace std;
using namespace boost::asio;

namespace DBService
{
  string DNSUtilities::resolveHostToIP(string hostname) {
    string resolvedIP = "";

    io_service io_service;

    ip::tcp::resolver::query resolver_query(hostname, "",
      ip::tcp::resolver::query::numeric_service);

    ip::tcp::resolver resolver(io_service);

    boost::system::error_code error_code;

    ip::tcp::resolver::iterator it =
        resolver.resolve(resolver_query, error_code);

    // TODO Raise exception
    if (error_code)
    {
      // TODO Raise exception
      //      << "Error code: " << error_code.value()
      //      << ".\nMessage: " << error_code.message();

      return "";
    }

    ip::tcp::resolver::iterator it_end;

    // TODO make ipv6 ready
    // Look for the first IPv4 address
    for (; it != it_end; it++)
    {
      ip::tcp::endpoint ep = it->endpoint();
      if (!ep.address().is_v4())
        continue;
      
      //  std::cout << "Endpoint Address: " << ep.address();
      resolvedIP = ep.address().to_string();
    }

    return resolvedIP;
  }
} // namespace DBService