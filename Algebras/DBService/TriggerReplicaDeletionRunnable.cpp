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

*/
#include "TriggerReplicaDeletionRunnable.hpp"

#include "StringUtils.h"

#include "Algebras/DBService/CommunicationClient.hpp"
#include "Algebras/DBService/DebugOutput.hpp"

using namespace std;

namespace DBService {

TriggerReplicaDeletionRunnable::TriggerReplicaDeletionRunnable(
                               string dbServiceWorkerHost,
                               int dbServiceWorkerCommPort,
                               std::string relID)
:runner(0),
 dbServiceWorkerHost(dbServiceWorkerHost),
 dbServiceWorkerCommPort(dbServiceWorkerCommPort),
 relID(relID)
{
    printFunction(
            "TriggerReplicaDeletionRunnable::TriggerReplicaDeletionRunnable");
}

TriggerReplicaDeletionRunnable::~TriggerReplicaDeletionRunnable()
{
    printFunction(
            "TriggerReplicaDeletionRunnable::~TriggerReplicaDeletionRunnable");
}

void TriggerReplicaDeletionRunnable::run()
{
    printFunction("TriggerReplicaDeletionRunnable::run");
    if(runner){
        runner->join();
        delete runner;
    }
    runner = new boost::thread(boost::bind(
            &TriggerReplicaDeletionRunnable::createClient,
            this,
            dbServiceWorkerHost,
            dbServiceWorkerCommPort,
            relID));
}
void TriggerReplicaDeletionRunnable::createClient(
        string dbServiceWorkerHost,
        int dbServiceWorkerCommPort,
        std::string relID)
{
    printFunction("TriggerReplicaDeletionRunnable::createClient");
    CommunicationClient client(dbServiceWorkerHost, dbServiceWorkerCommPort, 0);
    client.triggerReplicaDeletion(relID); // TODO check return value
}

} /* namespace DBService */
