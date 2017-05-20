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
#include <sstream>
#include <stdlib.h>
#include <vector>

#include "Algebra.h"

//#include "Algebras/DBService/CommunicationClientRunnable.hpp"
#include "Algebras/DBService/CommunicationClient.hpp"
#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/ReplicationServer.hpp"
#include "Algebras/DBService/ReplicationUtils.hpp"
#include "Algebras/DBService/Replicator.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"
#include "Algebras/DBService/ServerRunnable.hpp"

using namespace std;

namespace DBService
{

Replicator::Replicator(
        std::string& databaseName,
        std::string& relationName)
: databaseName(databaseName), relationName(relationName)
{
    printFunction("Replicator::Replicator");

    string fileTransferPort;
    SecondoUtilsLocal::readFromConfigFile(fileTransferPort,
            "DBService",
            "FileTransferPort",
            "");
    print(fileTransferPort);
    transferPort = atoi(fileTransferPort.c_str());

    SecondoUtilsLocal::readFromConfigFile(host,
            "Environment",
            "SecondoHost",
            "");
    print(host);
}


void Replicator::replicateRelation(const vector<LocationInfo>& locations) const
{
    printFunction("Replicator::replicateRelation");
    createFileOnCurrentNode();
//    runReplication(locations);
}

void Replicator::createFileOnCurrentNode() const
{
    printFunction("Replicator::createFileOnCurrentNode");

    // wait for "let" command to finish
    sleep(1000);
    print("starting replication");
    // TODO adjust database if necessary

    stringstream query;
    query << "query "
          << relationName
          << " saveObjectToFile[\""
          << ReplicationUtils::getFileName(
                  *(const_cast<string*>(&databaseName)),
                  *(const_cast<string*>(&relationName)))
          << "\"]";
    print("query", query.str());

    ListExpr resultList;
    string errorMessage;
    SecondoUtilsLocal::excuteQueryCommand(
            query.str(),
            resultList,
            errorMessage);
    print("resultList", resultList);
    print("errorMessage", errorMessage);
}

//void Replicator::runReplication(const vector<LocationInfo>& locations) const
//{
//    printFunction("Replicator::runReplication");
//    for(vector<LocationInfo>::const_iterator it = locations.begin();
//            it != locations.end(); it++)
//    {
//        printFunction("Replicator::runReplication");
////        CommunicationClientRunnable commClient(
////                           host,
////                           transferPort,
////                           it->getHost(),
////                           atoi(it->getCommPort().c_str()),
////                           getFileName(),
////                           databaseName,
////                           relationName);
////        commClient.run();
//
//        CommunicationClient client(*(const_cast<string*>(&(it->getHost()))),
//                                   atoi(it->getCommPort().c_str()),
//                                   0);
//        client.start();
//        client.triggerFileTransfer(host,
//                                   stringutils::int2str(transferPort),
//                                   databaseName,
//                                   relationName);
//    }
//}

} /* namespace DBService */
