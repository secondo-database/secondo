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
#include <cstdlib>

#include <boost/make_shared.hpp>

#include "SecondoException.h"

#include "Algebras/DBService/CommunicationClient.hpp"
#include "Algebras/DBService/DBServiceConnector.hpp"
#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/ReplicatorRunnable.hpp"
#include "SecondoUtilsLocal.hpp"

using namespace std;

namespace DBService {

DBServiceConnector::DBServiceConnector()
{
    printFunction("DBServiceConnector::DBServiceConnector");
}

DBServiceConnector* DBServiceConnector::getInstance()
{
    printFunction("DBServiceConnector::getInstance");
    if (!_instance)
    {
        _instance = new DBServiceConnector();
    }
    return _instance;
}

void DBServiceConnector::getNodesForReplication(
        string& host,
        int port,
        const string& relationName,
        vector<LocationInfo>& locations)
{
    CommunicationClient dbServiceMasterClient(host, port, 0);
    dbServiceMasterClient.start();

    dbServiceMasterClient.getNodesForReplication(relationName, locations);
}

bool DBServiceConnector::replicateRelation(const std::string& relationName)
{
    printFunction("DBServiceConnector::replicateRelation");
    print(relationName, "relationName");

    string dbServiceHost;
    SecondoUtilsLocal::readFromConfigFile(dbServiceHost,
                                           "DBService",
                                           "DBServiceHost",
                                           "");
    if(dbServiceHost.length() == 0)
    {
        print("could not find DBServiceHost in config file");
        throw new SecondoException("DBServiceHost not configured");
    }

    string dbServicePort;
    SecondoUtilsLocal::readFromConfigFile(dbServicePort,
                                       "DBService",
                                       "DBServicePort",
                                       "");
    if(dbServicePort.length() == 0)
    {
        print("could not find DBServicePort in config file");
        throw new SecondoException("DBServicePort not configured");
    }

    // connect to DBService master to find out location for replication
    vector<LocationInfo> locations;
    getNodesForReplication(
            dbServiceHost,
            atoi(dbServicePort.c_str()),
            relationName,
            locations);

    print("creating replication thread");
    ReplicatorRunnable replicationThread(
            SecondoSystem::GetInstance()->GetDatabaseName(),
            relationName,
            locations);
    replicationThread.run();
    return true;
}

DBServiceConnector* DBServiceConnector::_instance = NULL;

} /* namespace DBService */
