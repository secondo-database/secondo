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
#include "Algebras/DBService/ReplicationServer.hpp"
#include "Algebras/DBService/ReplicatorRunnable.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"
#include "Algebras/DBService/ServerRunnable.hpp"

using namespace std;

namespace DBService {

DBServiceConnector::DBServiceConnector()
{
    printFunction("DBServiceConnector::DBServiceConnector");
    string fileTransferPort;
    SecondoUtilsLocal::readFromConfigFile(fileTransferPort,
            "DBService",
            "FileTransferPort",
            "");
    print(fileTransferPort);
    ServerRunnable replicationServer(atoi(fileTransferPort.c_str()));
    replicationServer.run<ReplicationServer>();
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

void DBServiceConnector::triggerReplication(
        string& host,
        int port,
        const string& databaseName,
        const string& relationName,
        vector<LocationInfo>& locations)
{
    CommunicationClient dbServiceMasterClient(host, port, 0);
    dbServiceMasterClient.start();

    dbServiceMasterClient.triggerReplication(
            databaseName,
            relationName,
            locations);
}

bool DBServiceConnector::triggerReplication(const std::string databaseName,
                                            const std::string relationName)
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

    // connect to DBService master to determine location for replication
    vector<LocationInfo> locations;
    triggerReplication(
            dbServiceHost,
            atoi(dbServicePort.c_str()),
            databaseName,
            relationName,
            locations);
    return true;
}

void DBServiceConnector::createFileFromRelation()
{

}

DBServiceConnector* DBServiceConnector::_instance = NULL;

} /* namespace DBService */
