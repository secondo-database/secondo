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
#include "TriggerReplicaDeletionRunnable.hpp"

#include "StringUtils.h"

#include "Algebras/DBService/CommunicationClient.hpp"
#include "Algebras/DBService/DebugOutput.hpp"

using namespace std;

namespace DBService {

TriggerReplicaDeletionRunnable::TriggerReplicaDeletionRunnable(
                               string _dbServiceWorkerHost,
                               int _dbServiceWorkerCommPort,
                               const std::string& _database,
                               const std::string& _relation,
                               const std::string& _derivate)
:runner(0),
 dbServiceWorkerHost(_dbServiceWorkerHost),
 dbServiceWorkerCommPort(_dbServiceWorkerCommPort),
 database(_database),
 relation(_relation),
 derivate(_derivate)
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
            database, relation, derivate));
}


void TriggerReplicaDeletionRunnable::createClient(
        string dbServiceWorkerHost,
        int dbServiceWorkerCommPort,
        const std::string& database,
        const std::string& relationname,
        const std::string& derivate
        )
{
    printFunction("TriggerReplicaDeletionRunnable::createClient");
    CommunicationClient client(dbServiceWorkerHost, dbServiceWorkerCommPort, 0);
    client.triggerReplicaDeletion(database, relationname, derivate); 
    // TODO check return value
}

} /* namespace DBService */
