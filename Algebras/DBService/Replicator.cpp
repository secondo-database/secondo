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
#include "Algebras/Distributed2/FileRelations.h"

#include "Algebras/DBService/CommunicationClient.hpp"
#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/ReplicationServer.hpp"
#include "Algebras/DBService/ReplicationUtils.hpp"
#include "Algebras/DBService/Replicator.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"

using namespace std;

namespace DBService {

Replicator::Replicator(
        std::string& dbName,
        std::string& relName,
        ListExpr type)
: runner(0), databaseName(dbName), relationName(relName), relType(type)
{
    printFunction("Replicator::Replicator");
}

Replicator::~Replicator()
{
    printFunction("Replicator::~Replicator");
 /*   if(runner){
        runner->join();
        delete runner;
    }*/
}

void Replicator::createReplica(
        const std::string databaseName,
        const std::string relationName,
        const ListExpr relType)
{
    printFunction("Replicator::createReplica");
    print("databaseName", databaseName);
    print("relationName", relationName);

    const string fileName = ReplicationUtils::getFileName(
            databaseName,
            relationName);

    SecondoCatalog* catalog = SecondoSystem::GetCatalog();
    if(!catalog->IsObjectName(relationName))
    {
        print("not an object");
        return;
    }

    Word value;
    bool defined;
    do
    {
        print("object not yet defined");
        catalog->GetObject(relationName, value, defined);
    }while(!defined);

    Relation* rel = (Relation*)value.addr;

    if(BinRelWriter::writeRelationToFile(rel, relType,
            fileName))
    {
        string dbServiceHost;
        string dbServiceCommPort;
        SecondoUtilsLocal::lookupDBServiceLocation(
                dbServiceHost, dbServiceCommPort);

        print("Relation written to file, triggering replication");
        CommunicationClient dbServiceMasterClient(dbServiceHost,
                atoi(dbServiceCommPort.c_str()),
                0);

        dbServiceMasterClient.giveStartingSignalForReplication(
                databaseName,
                relationName);
    }else
    {
        print("Could not write relation to file");
    }
}

void Replicator::run()
{
    printFunction("Replicator::run");
    if(runner){
       runner->join();
       delete runner;
    }
    runner = new boost::thread(boost::bind(
            &Replicator::createReplica,
            this,
            databaseName,
            relationName,
            relType));
}

} /* namespace DBService */
