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
#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/ReplicationClient.hpp"
#include "Algebras/DBService/ReplicationClientRunnable.hpp"
#include "Algebras/DBService/ReplicationUtils.hpp"

using namespace std;

namespace DBService {

ReplicationClientRunnable::ReplicationClientRunnable(
        string targetHost,
        int targetTransferPort,
        string databaseName,
        string relationName)
: targetHost(targetHost), targetTransferPort(targetTransferPort),
  databaseName(databaseName), relationName(relationName),
  runner(0)
{
    print("ReplicationClientRunnable::ReplicationClientRunnable", std::cout);
}

ReplicationClientRunnable::~ReplicationClientRunnable()
{
    print("ReplicationClientRunnable::~ReplicationClientRunnable", std::cout);
}

void ReplicationClientRunnable::run()
{
    print("ReplicationClientRunnable::run", std::cout);
    if(runner){
        runner->join();
        delete runner;
    }
    runner = new boost::thread(boost::bind(
            &ReplicationClientRunnable::create,
            this,
            targetHost,
            targetTransferPort,
            databaseName,
            relationName));
}

void ReplicationClientRunnable::create(
        string& targetHost,
        int targetTransferPort,
        string& databaseName,
        string& relationName)
{
    print("ReplicationClientRunnable::create", std::cout);

    const string fileNameDBS =
            ReplicationUtils::getFileNameOnDBServiceWorker(
                    databaseName,
                    relationName);
    const string fileNameOrigin =
            ReplicationUtils::getFileName(
                    databaseName,
                    relationName);

    ReplicationClient client(targetHost,
                             targetTransferPort,
                             fileNameDBS,
                             fileNameOrigin,
                             databaseName,
                             relationName);
    client.receiveReplica();
}

} /* namespace DBService */
