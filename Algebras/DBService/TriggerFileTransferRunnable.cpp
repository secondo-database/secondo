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
#include "TriggerFileTransferRunnable.hpp"

#include "StringUtils.h"

#include "Algebras/DBService/CommunicationClient.hpp"
#include "Algebras/DBService/DebugOutput.hpp"

using namespace std;

namespace DBService {

TriggerFileTransferRunnable::TriggerFileTransferRunnable(
                               string sourceSystemHost,
                               int sourceSystemTransferPort,
                               string dbServiceWorkerHost,
                               int dbServiceWorkerCommPort,
                               std::string databaseName,
                               std::string relationName)
:runner(0),
 sourceSystemHost(sourceSystemHost),
 sourceSystemTransferPort(sourceSystemTransferPort),
 dbServiceWorkerHost(dbServiceWorkerHost),
 dbServiceWorkerCommPort(dbServiceWorkerCommPort),
 databaseName(databaseName), relationName(relationName)
{
    printFunction("TriggerFileTransferRunnable::TriggerFileTransferRunnable",
                  std::cout);
}

TriggerFileTransferRunnable::~TriggerFileTransferRunnable()
{
    printFunction("TriggerFileTransferRunnable::~TriggerFileTransferRunnable",
                  std::cout);
}

void TriggerFileTransferRunnable::run()
{
    printFunction("TriggerFileTransferRunnable::run", std::cout);
    if(runner){
        runner->join();
        delete runner;
    }
    runner = new boost::thread(boost::bind(
            &TriggerFileTransferRunnable::createClient,
            this,
            sourceSystemHost,
            sourceSystemTransferPort,
            dbServiceWorkerHost,
            dbServiceWorkerCommPort,
            databaseName,
            relationName));
}
void TriggerFileTransferRunnable::createClient(
        string sourceSystemHost,
        int sourceSystemTransferPort,
        string dbServiceWorkerHost,
        int dbServiceWorkerCommPort,
        std::string databaseName,
        std::string relationName)
{
    printFunction("TriggerFileTransferRunnable::createClient", std::cout);
    CommunicationClient client(dbServiceWorkerHost, dbServiceWorkerCommPort, 0);
    client.triggerFileTransfer(sourceSystemHost,
                               stringutils::int2str(sourceSystemTransferPort),
                               databaseName,
                               relationName);
}

} /* namespace DBService */
