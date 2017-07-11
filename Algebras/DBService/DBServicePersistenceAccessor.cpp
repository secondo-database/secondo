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
#include <sstream>

#include "SecondoException.h"
#include "StringUtils.h"

#include "Algebras/DBService/DBServicePersistenceAccessor.hpp"
#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"

using namespace std;

namespace DBService {

bool DBServicePersistenceAccessor::createOrInsert(
        const string& relationName,
        const RelationDefinition& rel,
        const vector<string>& values)
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
                CommandBuilder::buildCreateCommand(
                        relationName,
                        rel,
                        values),
                errorMessage);
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

    resultOk = SecondoUtilsLocal::executeQuery2(
            CommandBuilder::buildInsertCommand(
                    relationName,
                    rel,
                    values));

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

    vector<string> values =
    {
        { stringutils::int2str(connID) },
        { locationInfo.getHost() },
        { locationInfo.getPort() },
        { locationInfo.getConfig() },
        { locationInfo.getDisk() },
        { locationInfo.getCommPort() },
        { locationInfo.getTransferPort() },
    };

    return createOrInsert(relationName, locations, values);
}

bool DBServicePersistenceAccessor::persistRelationInfo(
        RelationInfo& relationInfo)
{
    printFunction("DBServicePersistenceAccessor::persistRelationInfo");

    string relationName("relations_DBSP");

    vector<string> values =
    {
        { relationInfo.toString() },
        { relationInfo.getDatabaseName() },
        { relationInfo.getRelationName() },
        { relationInfo.getOriginalLocation().getHost() },
        { relationInfo.getOriginalLocation().getPort() },
        { relationInfo.getOriginalLocation().getDisk() },
    };

    bool resultOk =
            createOrInsert(relationName, relations, values);
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
        string relationID,
        map<ConnectionID, bool>::const_iterator nodesBegin,
        map<ConnectionID, bool>::const_iterator nodesEnd)
{
    printFunction("DBServicePersistenceAccessor::persistLocationMapping");

    bool resultOk = true;
    for(map<ConnectionID, bool>::const_iterator it = nodesBegin;
            it != nodesEnd; it++)
    {
        string relationName("mapping_DBSP");

        vector<string> values =
        {
            { relationID },
            { stringutils::int2str(it->first) },
            { it->second ? string("TRUE") : string("FALSE") },
        };

        resultOk = resultOk &&
                createOrInsert(
                        relationName, mapping, values);
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
    bool resultOk = true;
    if(SecondoSystem::GetCatalog()->IsObjectName(string("locations_DBSP")))
    {
        string query("query locations_DBSP");
        string errorMessage;
        ListExpr resultList;
        resultOk = SecondoUtilsLocal::executeQueryCommand(
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
                    string transferPort(
                            nl->StringValue(nl->Seventh(currentRow)));

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
    }else
    {
        print("locations_DBSP not found -> nothing to restore");
    }
    return resultOk;
}

bool DBServicePersistenceAccessor::restoreRelationInfo(
        map<string, RelationInfo>& relations)
{
    printFunction("DBServicePersistenceAccessor::restoreRelationInfo");

    bool resultOk = true;
    if(SecondoSystem::GetCatalog()->IsObjectName(string("relations_DBSP")))
    {
        string query("query relations_DBSP");
        string errorMessage;
        ListExpr resultList;
        resultOk = SecondoUtilsLocal::executeQueryCommand(
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
                    RelationInfo relationInfo(
                            dbName, relName, host, port, disk);
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
    }else
    {
        print("relations_DBSP not found -> nothing to restore");
    }
    return resultOk;
}

bool DBServicePersistenceAccessor::restoreLocationMapping(
        queue<pair<string, ConnectionID> >& mapping)
{
    printFunction("DBServicePersistenceAccessor::restoreLocationMapping");
    bool resultOk = true;
    if(SecondoSystem::GetCatalog()->IsObjectName(string("mapping_DBSP")))
    {
        string query("query mapping_DBSP");
        string errorMessage;
        ListExpr resultList;
        resultOk = SecondoUtilsLocal::executeQueryCommand(
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
                    string conn(nl->StringValue(nl->Second(currentRow)));
                    print("RelationID: ", relID);
                    print("ConnectionID: ", conn);
                    mapping.push(
                            pair<string, ConnectionID>(
                                    relID, atoi(conn.c_str())));
                    resultData = nl->Rest(resultData);
                }
            }
        }else
        {
            print(errorMessage);
        }
    }else
    {
        print("mapping_DBSP not found -> nothing to restore");
    }
    return resultOk;
}

bool DBServicePersistenceAccessor::updateLocationMapping(
        string relationID,
        ConnectionID connID,
        bool replicated)
{
    printFunction("DBServicePersistenceAccessor::updateLocationMapping");

    string databaseName;
    string relationName;
    RelationInfo::parseIdentifier(
            relationID,
            databaseName,
            relationName);

    SecondoUtilsLocal::adjustDatabase(string("dbservice"));

    FilterConditions filterConditions =
    {
        { {AttributeType::STRING, string("RelationID") }, relationID },
        { {AttributeType::INT, string("ConnectionID") },
                stringutils::int2str(connID) }
    };
    AttributeInfoWithValue valueToUpdate =
    { {AttributeType::BOOL, string("Replicated") },
            replicated ? string("TRUE") : string("FALSE") };

    return SecondoUtilsLocal::executeQuery2(
            CommandBuilder::buildUpdateCommand(
                    relationName,
                    filterConditions,
                    valueToUpdate));
}

bool DBServicePersistenceAccessor::deleteRelationInfo(
        RelationInfo& relationInfo)
{
    printFunction("DBServicePersistenceAccessor::deleteRelationInfo");
    string relationName("relations_DBSP");
    SecondoUtilsLocal::adjustDatabase(relationInfo.getDatabaseName());

    string relationID = relationInfo.toString();
    FilterConditions filterConditions =
    {
        { {AttributeType::STRING, string("RelationID") }, relationID },
    };

    bool resultOk = SecondoUtilsLocal::executeQuery2(
            CommandBuilder::buildDeleteCommand(
                    relationName,
                    filterConditions));

    if(!resultOk)
    {
        print("Could not delete relation metadata");
        print("RelationID: ", relationID);
    }

    return resultOk && deleteLocationMapping(
            relationID,
            relationInfo.nodesBegin(),
            relationInfo.nodesEnd());
}

bool DBServicePersistenceAccessor::deleteLocationMapping(
        string relID,
        map<ConnectionID, bool>::const_iterator nodesBegin,
        map<ConnectionID, bool>::const_iterator nodesEnd)
{
    printFunction("DBServicePersistenceAccessor::deleteLocationMapping");
    string dbName;
    string relName;
    RelationInfo::parseIdentifier(
            relID,
            dbName,
            relName);

    SecondoUtilsLocal::adjustDatabase(dbName);

    string relationName("mapping_DBSP");
    bool resultOk = true;
    for(map<ConnectionID, bool>::const_iterator it = nodesBegin;
            it != nodesEnd; it++)
    {
        FilterConditions filterConditions =
        {
            { {AttributeType::STRING, string("RelationID") }, relID },
            { {AttributeType::INT, string("ConnectionID") },
                    stringutils::int2str(it->first) }
        };

        resultOk = resultOk && SecondoUtilsLocal::executeQuery2(
                CommandBuilder::buildDeleteCommand(
                        relationName,
                        filterConditions));
        if(!resultOk)
        {
            print("Could not delete mapping");
            print("RelationID: ", relID);
            print("ConnectionID: ", it->first);
        }
    }
    return resultOk;
}

RelationDefinition DBServicePersistenceAccessor::locations =
{
    { AttributeType::INT, "ConnectionID" },
    { AttributeType::STRING, "Host" },
    { AttributeType::STRING, "Port" },
    { AttributeType::STRING, "Config" },
    { AttributeType::STRING, "Disk" },
    { AttributeType::STRING, "CommPort" },
    { AttributeType::STRING, "TransferPort" }
};

RelationDefinition DBServicePersistenceAccessor::relations =
{
    { AttributeType::STRING, "RelationID" },
    { AttributeType::STRING, "DatabaseName" },
    { AttributeType::STRING, "RelationName" },
    { AttributeType::STRING, "Host" },
    { AttributeType::STRING, "Port" },
    { AttributeType::STRING, "Disk" },
};

RelationDefinition DBServicePersistenceAccessor::mapping =
{
    { AttributeType::STRING, "RelationID" },
    { AttributeType::STRING, "ConnectionID" },
    { AttributeType::BOOL, "Replicated" },
};

} /* namespace DBService */

