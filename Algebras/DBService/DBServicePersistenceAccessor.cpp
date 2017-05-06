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

#include "SecondoException.h"

#include "Algebras/DBService/DBServicePersistenceAccessor.hpp"
#include "Algebras/DBService/DebugOutput.hpp"
#include "SecondoUtilsLocal.hpp"

using namespace std;

namespace DBService {

bool DBServicePersistenceAccessor::createOrInsert(
        string& relationName,
        const string& createQuery,
        const string& insertQuery)
{
    printFunction("DBServicePersistenceAccessor::createOrInsert");
    string databaseName("dbservice");
    print(relationName);

    SecondoUtilsLocal::adjustDatabase(databaseName);

    bool resultOk = false;
    string errorMessage;

    if(!SecondoSystem::GetCatalog()->IsObjectName(relationName))
    {
        print("relation does not exist: ", relationName);
        resultOk = SecondoUtilsLocal::createRelation(
                createQuery, errorMessage);
        if(resultOk)
        {
            print("created relation: ", relationName);
            return true;
        }
        return false;
    }
    print("relation exists, trying insert command");

    ListExpr resultList;
    resultOk = SecondoUtilsLocal::excuteQueryCommand(
            insertQuery, resultList, errorMessage);

    if(!resultOk)
    {
        // TODO better error message (print everything)
        print("insert failed");
        //throw new SecondoException("Could not insert tuple");
        return false;
    }

    return true;
}

bool DBServicePersistenceAccessor::persistLocationInfo(
        ConnectionID connID, LocationInfo& locationInfo)
{
    printFunction("DBServicePersistenceAccessor::persistLocationInfo");

    string relationName("locations_DBSP");
    stringstream createQuery;
    createQuery << "let locations_DBSP = [const rel(tuple(["
            << "ConnectionID: int, "
            << "Host: string, "
            << "Port: string, "
            << "Config: string, "
            << "Disk: string, "
            << "CommPort: string, "
            << "TransferPort: string])) value(("
            << connID << " "
            << "\"" << locationInfo.getHost() << "\" "
            << "\"" << locationInfo.getPort() << "\" "
            << "\"" << locationInfo.getConfig() << "\" "
            << "\"" << locationInfo.getDisk() << "\" "
            << "\"" << locationInfo.getCommPort() << "\" "
            << "\"" << locationInfo.getTransferPort() << "\""
            << "))]";
    print("createQuery", createQuery.str());

    stringstream insertQuery;
    insertQuery << "query locations_DBSP inserttuple["
            << connID << ", "
            << "\"" << locationInfo.getHost() << "\", "
            << "\"" << locationInfo.getPort() << "\", "
            << "\"" << locationInfo.getConfig() << "\", "
            << "\"" << locationInfo.getDisk() << "\", "
            << "\"" << locationInfo.getCommPort() << "\", "
            << "\"" << locationInfo.getTransferPort() << "\""
            << "] consume";
    print("insertQuery", insertQuery.str());

    return createOrInsert(relationName, createQuery.str(), insertQuery.str());
}

bool DBServicePersistenceAccessor::persistRelationInfo(
        RelationInfo& relationInfo)
{
    printFunction("DBServicePersistenceAccessor::persistRelationInfo");

    string relationName("relations_DBSP");

    stringstream createQuery;
    createQuery << "let relations_DBSP = [const rel(tuple(["
            << "RelationID: string, "
            << "DatabaseName: string, "
            << "RelationName: string, "
            << "Host: string, "
            << "Port: string, "
            << "Disk: string])) value(("
            << "\"" << relationInfo.toString() << "\" "
            << "\"" << relationInfo.getDatabaseName() << "\" "
            << "\"" << relationInfo.getRelationName() << "\" "
            << "\"" << relationInfo.getOriginalLocation().getHost() << "\" "
            << "\"" << relationInfo.getOriginalLocation().getPort() << "\" "
            << "\"" << relationInfo.getOriginalLocation().getDisk() << "\""
            << "))]";
    print("createQuery", createQuery.str());

    stringstream insertQuery;
    insertQuery << "query relations_DBSP inserttuple["
            << "\"" << relationInfo.toString() << "\", "
            << "\"" << relationInfo.getDatabaseName() << "\", "
            << "\"" << relationInfo.getRelationName() << "\", "
            << "\"" << relationInfo.getOriginalLocation().getHost() << "\", "
            << "\"" << relationInfo.getOriginalLocation().getPort() << "\", "
            << "\"" << relationInfo.getOriginalLocation().getDisk() << "\""
            << "] consume";
    print("insertQuery", insertQuery.str());

    // TODO check return code
    createOrInsert(relationName, createQuery.str(), insertQuery.str());
    return persistLocationMapping(
            relationInfo.toString(),
            relationInfo.nodesBegin(),
            relationInfo.nodesEnd());
}

bool DBServicePersistenceAccessor::persistLocationMapping(
        std::string relationID,
        vector<ConnectionID>::const_iterator nodesBegin,
        vector<ConnectionID>::const_iterator nodesEnd)
{
    printFunction("DBServicePersistenceAccessor::persistLocationMapping");

    for(vector<ConnectionID>::const_iterator it = nodesBegin;
            it != nodesEnd; it++)
    {
        string relationName("mapping_DBSP");

        stringstream createQuery;
        createQuery << "let mapping_DBSP = [const rel(tuple(["
                << "RelationID: string, "
                << "ConnectionID: int])) value(("
                << "\"" << relationID << "\" "
                << *it
                << "))]";
        print("createQuery", createQuery.str());

        stringstream insertQuery;
        insertQuery << "query mapping_DBSP inserttuple["
                << "\"" << relationID << ", "
                << *it
                << "] consume";
        print("insertQuery", insertQuery.str());

        createOrInsert(relationName, createQuery.str(), insertQuery.str());
    }
    return true;
}


bool DBServicePersistenceAccessor::restoreLocationInfo(
        map<ConnectionID, LocationInfo>& locations)
{
    printFunction("DBServicePersistenceAccessor::restoreLocationInfo");

    string query("query locations_DBSP");
    string errorMessage;
    ListExpr resultList;
    bool resultOk = SecondoUtilsLocal::excuteQueryCommand(
            query, resultList, errorMessage);
    if(resultOk)
    {
        print("resultList", resultList);
        ListExpr resultData = nl->Second(resultList);
        print("resultData", resultData);

        int resultCount = nl->ListLength(resultData);
        print(resultCount);

        for(int i = 0; i < resultCount; i++)
        {
            if(!nl->IsEmpty(resultData))
            {
                print("resultData", resultData);
                ListExpr currentRow = nl->First(resultData);
                ConnectionID conn(nl->IntValue(nl->First(currentRow)));
                string host(nl->StringValue(nl->Second(currentRow)));
                string port(nl->StringValue(nl->Third(currentRow)));
                string config(nl->StringValue(nl->Fourth(currentRow)));
                string disk(nl->StringValue(nl->Fifth(currentRow)));
                string commPort(nl->StringValue(nl->Sixth(currentRow)));
                string transferPort(nl->StringValue(nl->Seventh(currentRow)));

                LocationInfo location(
                        host, port, config, disk, commPort, transferPort);
                print(location);
                locations.insert(
                        pair<ConnectionID, LocationInfo>(conn, location));
                // nl->Seventh(currentRow);

                resultData = nl->Rest(resultData);
            }
        }
    }else
    {
        print(errorMessage);
    }
    return resultOk;
}

bool DBServicePersistenceAccessor::restoreRelationInfo(
        vector<RelationInfo>& relations)
{
    printFunction("DBServicePersistenceAccessor::restoreRelationInfo");
    return true;
}

bool DBServicePersistenceAccessor::restoreLocationMapping(
        queue<pair<std::string, ConnectionID> >& mapping)
{
    printFunction("DBServicePersistenceAccessor::restoreLocationMapping");
    return true;
}

} /* namespace DBService */

