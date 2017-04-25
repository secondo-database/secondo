/*
----
This file is part of SECONDO.

Copyright (C) 2016,
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
#include <cstdlib>
#include <boost/make_shared.hpp>

#include "SecondoException.h"

#include "Algebras/DBService/DBServiceConnector.hpp"
#include "Algebras/DBService/CommunicationClient.hpp"
#include "Algebras/DBService/SecondoUtils.hpp"

using namespace std;

namespace DBService {

DBServiceConnector::DBServiceConnector()
{}

DBServiceConnector* DBServiceConnector::getInstance()
{
    if (!_instance)
    {
        _instance = new DBServiceConnector();
    }
    return _instance;
}

bool DBServiceConnector::replicateRelation(const std::string& relationName)
{
    string dbServiceHost;
    SecondoUtils::readFromConfigFile(dbServiceHost,
                                           "DBService",
                                           "DBServiceHost",
                                           "");
    if(dbServiceHost.length() == 0)
    {
        throw new SecondoException("DBServiceHost not configured");
    }

    string dbServicePort;
    SecondoUtils::readFromConfigFile(dbServicePort,
                                       "DBService",
                                       "DBServicePort",
                                       "");
    if(dbServicePort.length() == 0)
    {
        throw new SecondoException("DBServicePort not configured");
    }

    // connect to DBService master to find out location for replication
    CommunicationClient masterClient(dbServiceHost,
                                              atoi(dbServicePort.c_str()), 0);
    masterClient.start();

    vector<LocationInfo> locations;
    masterClient.getNodesForReplication(relationName, locations);

    replicator.replicateRelation(relationName, locations);
    return true;
}

DBServiceConnector* DBServiceConnector::_instance = NULL;

} /* namespace DBService */
