/*
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


//[$][\$]
//[_][\_]

*/
#ifndef ALGEBRAS_DBSERVICE_LOCATIONINFO_HPP_
#define ALGEBRAS_DBSERVICE_LOCATIONINFO_HPP_

#include <string>

namespace DBService {

/*

1 \textit{LocationInfo}

The \textit{LocationInfo} object represents a SECONDO instance and stores all
data that is relevant in the context of the \textit{DBService}.

*/

class LocationInfo {
public:

/*

1.1 Function Definitions

The \textit{LocationInfo} object provides several member functions.

1.1.1 Constructor

*/
    LocationInfo(const std::string& host,
                 const std::string& port,
                 const std::string& config,
                 const std::string& disk,
                 const std::string& commPort,
                 const std::string& transferPort);
/*

1.1.1 \textit{getHost}

This function returns the hostname of the represented instance.

*/
    const std::string& getHost() const;

/*

1.1.1 \textit{getPort}

This function returns the port of the represented instance.

*/
    const std::string& getPort() const;

/*

1.1.1 \textit{getConfig}

This function returns the path where the config file of the represented instance
is located.

*/
    const std::string& getConfig() const;

/*

1.1.1 \textit{getDisk}

This function returns the path where the represented instance stores its data.

*/
    const std::string& getDisk() const;

/*

1.1.1 \textit{getCommPort}

This function returns the port on which the \textit{CommunicationServer} is
listening on the represented instance.

*/
    const std::string& getCommPort() const;

/*

1.1.1 \textit{getTransferPort}

This function returns the port on which the \textit{ReplicationServer} is
listening on the represented instance.

*/
    const std::string& getTransferPort() const;

/*

1.1.1 \textit{isEqual}

This function returns whether the specified host and port is equal to the
stored ones.

*/
    bool isEqual(const std::string& cmpHost, const std::string& cmpPort) const;
/*

1.1 Member Definitions

The \textit{LocationInfo} object has several class members.

1.1.1 \textit{host}

Stores the hostname of the represented instance.

*/
private:
    std::string host;

/*

1.1.1 \textit{port}

Stores the port of the represented instance.

*/
    std::string port;

/*

1.1.1 \textit{config}

Stores the configuration file path of the represented instance.

*/
    std::string config;

/*

1.1.1 \textit{disk}

Stores the data path of the represented instance.

*/
    std::string disk;

/*

1.1.1 \textit{commPort}

Stores the port on which the \textit{CommunicationServer} is
listening on the represented instance.

*/
    std::string commPort;

/*

1.1.1 \textit{transferPort}

Stores the port on which the \textit{ReplicationServer} is
listening on the represented instance.

*/
    std::string transferPort;
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_LOCATIONINFO_HPP_ */
