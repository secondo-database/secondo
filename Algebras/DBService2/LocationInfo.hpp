/*

1.1 ~LocationInfo~

This object is mainly used to store metadata about the ~DBService~ worker nodes
that are available in the system. It is also used to store the original location
of a relation.

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
#ifndef ALGEBRAS_DBSERVICE_LOCATIONINFO_HPP_
#define ALGEBRAS_DBSERVICE_LOCATIONINFO_HPP_

#include <string>
#include <boost/asio.hpp>

#include "MetadataObject.hpp"

namespace DBService {

/*

1.1.1 Class Definition

*/

class LocationInfo : public MetadataObject
{
public:

/*

1.1.1.1 Constructor

*/
    LocationInfo(const std::string& host,
                 const std::string& port,
                 const std::string& config,
                 const std::string& disk,
                 const std::string& commPort,
                 const std::string& transferPort);
/*

1.1.1.1 \textit{getHost}

This function returns the hostname of the represented instance.

*/
    const std::string& getHost() const;

/*

1.1.1.1 \textit{getPort}

This function returns the port of the represented instance.

*/
    const std::string& getPort() const;

/*

1.1.1.1 \textit{getConfig}

This function returns the path where the config file of the represented instance
is located.

*/
    const std::string& getConfig() const;

/*

1.1.1.1 \textit{getDisk}

This function returns the path where the represented instance stores its data.

*/
    const std::string& getDisk() const;

/*

1.1.1.1 \textit{getCommPort}

This function returns the port on which the \textit{CommunicationServer} is
listening on the represented instance.

*/
    const std::string& getCommPort() const;

/*

1.1.1.1 \textit{getTransferPort}

This function returns the port on which the \textit{ReplicationServer} is
listening on the represented instance.

*/
    const std::string& getTransferPort() const;

/*

1.1.1.1 ~setTransferPort~

This function allows to set the value of member ~transferPort~.

*/
    void setTransferPort(std::string& port);

/*

1.1.1.1 \textit{isSameWorker}

This function returns whether the specified host and port are equal to the
stored ones.

*/
    bool isSameWorker(
            const std::string& cmpHost,
            const std::string& cmpPort) const;


/*

1.1.1.1 \textit{isSameHost}

This function returns whether the specified host is equal to the stored ones.

*/
    bool isSameHost(
            const std::string& cmpHost) const;

    /*

1.1.1.1 \textit{resolveHostToIP}

Resolve the given hostname to be sure to get to an IP address instead of a domain name.
If the host can't be resolved the empty string "" is returned.

*/
    std::string resolveHostToIP(std::string hostname) const;

    /*

1.1.1.1 \textit{isSameDisk}

This function returns whether the data of the specified worker is located on the
same disk as the data of the stored one.

*/
    bool isSameDisk(
        const std::string &cmpHost, const std::string &cmpDisk) const;

    inline static std::string getIdentifier(const std::string& host,
                                            const std::string& disk)
    {
       return host + separator + disk;
    }


/*

1.1.1.1 \textit{host}

Stores the hostname of the represented instance.

*/
private:
    std::string host;

/*

1.1.1.1 \textit{port}

Stores the port of the represented instance.

*/
    std::string port;

/*

1.1.1.1 \textit{config}

Stores the configuration file path of the represented instance.

*/
    std::string config;

/*

1.1.1.1 \textit{disk}

Stores the data path of the represented instance.

*/
    std::string disk;

/*

1.1.1.1 \textit{commPort}

Stores the port on which the \textit{CommunicationServer} is
listening on the represented instance.

*/
    std::string commPort;

/*

1.1.1.1 \textit{transferPort}

Stores the port on which the \textit{ReplicationServer} is
listening on the represented instance.

*/
    std::string transferPort;

};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_LOCATIONINFO_HPP_ */
