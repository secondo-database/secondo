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

#include "Algebras/DBService/LocationInfo.hpp"

using namespace std;

namespace DBService {

LocationInfo::LocationInfo(const string& host,
                           const string& port,
                           const string& config,
                           const string& disk,
                           const string& commPort,
                           const string& transferPort)
: MetadataObject(),
  host(host), port(port), config(config), disk(disk),
  commPort(commPort), transferPort(transferPort)
{}

const string& LocationInfo::getHost() const
{
    return host;
}

const string& LocationInfo::getPort() const
{
    return port;
}

const string& LocationInfo::getConfig() const
{
    return config;
}

const string& LocationInfo::getDisk() const
{
    return disk;
}

const string& LocationInfo::getCommPort() const
{
    return commPort;
}

const string& LocationInfo::getTransferPort() const
{
    return transferPort;
}

void LocationInfo::setTransferPort(string& port)
{
    transferPort = port;
}

bool LocationInfo::isSameWorker(
        const string& cmpHost,
        const string& cmpPort) const
{
    return isSameHost(cmpHost) && (port == cmpPort);
}

bool LocationInfo::isSameHost(
        const std::string& cmpHost) const
{
    bool isIdentical = false;

    string resolvedHost = resolveHostToIP(host);
    string resolvedHostToCompare = resolveHostToIP(cmpHost);

    // Only if both hosts can be resolved and are string-equal, they are 
    // considered equal.
    if (resolvedHost != "" && resolvedHostToCompare != ""
        && (resolvedHost == resolvedHostToCompare))
        isIdentical = true;        

    return isIdentical;
}

bool LocationInfo::isSameDisk(
        const std::string& cmpHost, const std::string& cmpDisk) const
{
    if(!isSameHost(cmpHost))
    {
        return false;
    }

    size_t slashPos = disk.find("/", 1);

    if(slashPos != cmpDisk.find("/", 1))
    {
        return false;
    }
    return disk.substr(0, slashPos).compare(cmpDisk.substr(0, slashPos)) == 0;
}

string LocationInfo::resolveHostToIP(std::string hostname) const
{
    string resolvedIP = "";

    boost::asio::io_service io_service;

    boost::asio::ip::tcp::resolver::query resolver_query(hostname, "", 
        boost::asio::ip::tcp::resolver::query::numeric_service);

    boost::asio::ip::tcp::resolver resolver(io_service);

    boost::system::error_code error_code;

    boost::asio::ip::tcp::resolver::iterator it = 
        resolver.resolve(resolver_query, error_code);

    // TODO Raise exception
    if (error_code)
    {
        // TODO Raise exception       
        //      << "Error code: " << error_code.value()
        //      << ".\nMessage: " << error_code.message();

        return "";
    }

    boost::asio::ip::tcp::resolver::iterator it_end;

    // TODO make ipv6 ready
    // Look for the first IPv4 address
    for (; it != it_end; it++) {    
        boost::asio::ip::tcp::endpoint ep = it->endpoint();
        if ( !ep.address().is_v4() )
            continue;
        // std::cout << "Endpoint Address: " << ep.address();
        resolvedIP = ep.address().to_string();
    }

    return resolvedIP;
}

} /* namespace DBService */

