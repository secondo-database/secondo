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
#include "CreateDerivateRunnable.hpp"

#include "StringUtils.h"

#include "Algebras/DBService/CommunicationClient.hpp"
#include "Algebras/DBService/DebugOutput.hpp"

using namespace std;

namespace DBService {

CreateDerivateRunnable::CreateDerivateRunnable(
                               string dbServiceWorkerHost,
                               int dbServiceWorkerCommPort,
                               const std::string& _database,
                               const std::string& _targetname,
                               const std::string& _relname,
                               const std::string& _fundef)
:runner(0),
 dbServiceWorkerHost(dbServiceWorkerHost),
 dbServiceWorkerCommPort(dbServiceWorkerCommPort),
 dbname(_database),
 targetname(_targetname),
 relname(_relname),
 fundef(_fundef)
{
    printFunction(__FUNCTION__);
}

CreateDerivateRunnable::~CreateDerivateRunnable()
{
    printFunction(__FUNCTION__);
}

void CreateDerivateRunnable::run()
{
    printFunction(__FUNCTION__);
    if(runner){
        runner->join();
        delete runner;
    }
    runner = new boost::thread(boost::bind(
            &CreateDerivateRunnable::createClient,
            this,
            dbServiceWorkerHost,
            dbServiceWorkerCommPort,
            dbname,
            targetname,
            relname,
            fundef));
}
void CreateDerivateRunnable::createClient(
        string dbServiceWorkerHost,
        int dbServiceWorkerCommPort,
        std::string database,
        std::string targetname,
        std::string relname,
        std::string fundef)
{
    printFunction(__FUNCTION__);
    CommunicationClient client(dbServiceWorkerHost, dbServiceWorkerCommPort, 0);
    client.createDerivation(database, targetname, relname,fundef); 
    // TODO check return value
}

} /* namespace DBService */
