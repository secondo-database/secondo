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
        }else
        {
            print("failed to create relation: ", relationName);
        }
        return resultOk;
    }
    print("relation exists, trying insert command");

    resultOk = SecondoUtilsLocal::excuteQueryCommand(insertQuery);

    if(resultOk)
    {
        print("insert successful");
    }else
    {
        print("insert failed");
    }
    return resultOk;
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

    bool resultOk =
            createOrInsert(relationName, createQuery.str(), insertQuery.str());
    if(resultOk)
    {
        print("RelationInfo persisted");
        resultOk = persistLocationMapping(
                   relationInfo.toString(),
                   relationInfo.nodesBegin(),
                   relationInfo.nodesEnd());
    }else
    {
        print("Could not persist RelationInfo. Skipping mapping.");
    }
    return resultOk;
}

bool DBServicePersistenceAccessor::persistLocationMapping(
        std::string relationID,
        vector<ConnectionID>::const_iterator nodesBegin,
        vector<ConnectionID>::const_iterator nodesEnd)
{
    printFunction("DBServicePersistenceAccessor::persistLocationMapping");

    bool resultOk = true;
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
                << "\"" << relationID << "\", "
                << *it
                << "] consume";
        print("insertQuery", insertQuery.str());

        resultOk = resultOk &&
                createOrInsert(
                        relationName, createQuery.str(), insertQuery.str());
        if(!resultOk)
        {
            print("failed to persist location mapping");
        }
    }
    if(resultOk)
    {
        print("location mapping persisted successfully");
    }
    return resultOk;
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
        map<string, RelationInfo>& relations)
{
    printFunction("DBServicePersistenceAccessor::restoreRelationInfo");

    string query("query relations_DBSP");
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
                string relID(nl->StringValue(nl->First(currentRow)));
                string dbName(nl->StringValue(nl->Second(currentRow)));
                string relName(nl->StringValue(nl->Third(currentRow)));
                string host(nl->StringValue(nl->Fourth(currentRow)));
                string port(nl->StringValue(nl->Fifth(currentRow)));
                string disk(nl->StringValue(nl->Sixth(currentRow)));
                RelationInfo relationInfo(dbName, relName, host, port, disk);
                print(relationInfo);
                relations.insert(
                        pair<string, RelationInfo>(relID, relationInfo));
                resultData = nl->Rest(resultData);
            }
        }
    }else
    {
        print(errorMessage);
    }
    return resultOk;
}

bool DBServicePersistenceAccessor::restoreLocationMapping(
        queue<pair<std::string, ConnectionID> >& mapping)
{
    printFunction("DBServicePersistenceAccessor::restoreLocationMapping");
    string query("query mapping_DBSP");
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
                    string relID(nl->StringValue(nl->First(currentRow)));
                    ConnectionID conn(nl->IntValue(nl->Second(currentRow)));
                    print("RelationID: ", relID);
                    print("ConnectionID: ", conn);
                    mapping.push(
                            pair<string, ConnectionID>(relID, conn));
                    resultData = nl->Rest(resultData);
                }
            }
        }else
        {
            print(errorMessage);
        }
        return resultOk;
}

} /* namespace DBService */

