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
#include <sstream>
#include <utility>

#include "SecondoException.h"
#include "StringUtils.h"

#include "Algebras/DBService2/DBServicePersistenceAccessor.hpp"
#include "Algebras/DBService2/DebugOutput.hpp"
#include "Algebras/DBService2/SecondoUtilsLocal.hpp"

using namespace std;

namespace DBService {


//TODO This class is deprecated. Remove all dependencies to it. Then remove it.

bool DBServicePersistenceAccessor::createDBSchemaIfNotExists()
{
    printFunction("DBServicePersistenceAccessor::createDBSchemaIfNotExists", 
        std::cout);

    //TODO DBServicePersistenceAccessor must be instantiated ... 
    //  look for an example on how to do that.

    // Create table locations_DBSP    
    DBServicePersistenceAccessor::createSecondoRelation(string("dbs_nodes"), 
        nodes);

    print("Done creating the DBService database schema.", std::cout);
    return true;
}

bool DBServicePersistenceAccessor::deleteAndCreate(
        const string& relationName,
        const RelationDefinition& rel,
        const vector<vector <string> >& values)
{
    printFunction("DBServicePersistenceAccessor::deleteAndCreate", std::cout);
    print("Relation name:", relationName, std::cout);

    SecondoUtilsLocal::adjustDatabase(DBSERVICE_DATABASE_NAME);

    if(SecondoSystem::GetCatalog()->IsObjectName(relationName))
    {
        print("Deleting object", relationName, std::cout);
        
        // TODO Why delete the entire table?
        SecondoSystem::GetCatalog()->DeleteObject(relationName);
        print("Done deleting object", relationName, std::cout);
    } else {
        print("The relation can't be deleted as it doesn't exist", 
            relationName, std::cout);
        return false;
    }

    if(values.empty())
    {
        print("Nothing to persist", std::cout);
        return true;
    }

    print("Persisting values...", std::cout);
    return createOrInsert(relationName, rel, values);
}

//TODO Merge createSecondoRelation with createOrInsert
bool DBServicePersistenceAccessor::createSecondoRelation(
    const std::string &relationName,
    const RelationDefinition &rel)
{
    printFunction("DBServicePersistenceAccessor::createSecondoRelation", 
        std::cout);
    print(relationName, std::cout);

    SecondoUtilsLocal::adjustDatabase(DBSERVICE_DATABASE_NAME);

    bool resultOk = false;
    string errorMessage;

    // Create an empty vector for buildCreateCommand
    vector<vector<string>> values;

    if (!SecondoSystem::GetCatalog()->IsObjectName(relationName))
    {
        print("The secondo relation does not exist: ", relationName, std::cout);

        resultOk = SecondoUtilsLocal::createRelation(

            // Add buildCreateCommand without values
            CommandBuilder::buildCreateCommand(
                relationName,
                rel,
                {values}),
            errorMessage);
        if (resultOk)
        {            
            print("Created the secondo relation: ", relationName, std::cout);
        }
        else
        {            
            print("Failed to create the secondo relation: ", relationName, 
                std::cout);
            return false;
        }        
    }
    return resultOk;
}


    //TODO Rename to insert
    bool
    DBServicePersistenceAccessor::createOrInsert(
        const string &relationName,
        const RelationDefinition &rel,
        const vector<vector<string>> &values)
{
    printFunction("DBServicePersistenceAccessor::createOrInsert", std::cout);
    print(relationName, std::cout);

    SecondoUtilsLocal::adjustDatabase(DBSERVICE_DATABASE_NAME);

    bool resultOk = false;
    string errorMessage;

    if(!SecondoSystem::GetCatalog()->IsObjectName(relationName))
    {
        print("Relation does not exist. Can't insert. Aborting. Relationname:",
            relationName, std::cout);        

        return resultOk;
    }
    print("Relation exists. Trying insert command.", std::cout);

    if(values.size() != 1)
    {
        print("values has wrong format for insert, aborting", std::cout);
        return false;
    }

    string command = CommandBuilder::buildInsertCommand(
                    relationName,
                    rel,
                    values[0]);
    print("Command to be executed is: ", command, std::cout);

    resultOk = SecondoUtilsLocal::executeQuery2(
            command);

    if(resultOk) {
        print("Insert successful", std::cout);
    } else {
        print("Insert failed", std::cout);
    }

    return resultOk;
}

bool DBServicePersistenceAccessor::persistLocationInfo(
        ConnectionID connID, LocationInfo& locationInfo)
{
    printFunction("DBServicePersistenceAccessor::persistLocationInfo",
                  std::cout);

    string relationName("locations_DBSP");

    vector<string> value =
    {
        stringutils::int2str(connID),
        locationInfo.getHost(),
        locationInfo.getPort(),
        locationInfo.getConfig(),
        locationInfo.getDisk(),
        locationInfo.getCommPort(),
        locationInfo.getTransferPort(),
    };

    return createOrInsert(relationName, locations, {value});
}

bool DBServicePersistenceAccessor::persistRelationInfo(
        RelationInfo& relationInfo)
{
    printFunction("DBServicePersistenceAccessor::persistRelationInfo",
                  std::cout);

    string relationName("relations_DBSP");

    vector<string> value =
    {
        relationInfo.toString(),
        relationInfo.getDatabaseName(),
        relationInfo.getRelationName(),
        relationInfo.getOriginalLocation().getHost(),
        relationInfo.getOriginalLocation().getPort(),
        relationInfo.getOriginalLocation().getDisk()
    };

    /* TODO JF Change to insert if not exists. Currently this case is prevented 
     * as the CommunicationServer checks for the existence of the relation 
     * already
     * but it would be cleaner to express the intend of the insert semantic more
     * clearly where the persistency is about to happen.
     */
    bool resultOk =
            createOrInsert(relationName, relations, {value});
    if(resultOk)
    {
        print("RelationInfo persisted", std::cout);
        resultOk = persistLocationMapping(
                   relationInfo.toString(),
                   relationInfo.nodesBegin(),
                   relationInfo.nodesEnd());
    } else {
        print("Could not persist RelationInfo. Skipping mapping.", std::cout);
    }

    return resultOk;
}

bool DBServicePersistenceAccessor::persistLocationMapping(
        string relationID,
        ReplicaLocations::const_iterator nodesBegin,
        ReplicaLocations::const_iterator nodesEnd)
{
    printFunction("DBServicePersistenceAccessor::persistLocationMapping",
                  std::cout);

    bool resultOk = true;
    for(ReplicaLocations::const_iterator it = nodesBegin;
            it != nodesEnd; it++)
    {
        string relationName("mapping_DBSP");

        vector<string> value =
        {
            relationID ,
            stringutils::int2str(it->first) ,
            it->second ? string("TRUE") : string("FALSE") ,
        };

        resultOk = resultOk &&
                createOrInsert(
                        relationName, mapping, {value});
        if(!resultOk)
        {
            print("failed to persist location mapping", std::cout);
        }
    }
    if(resultOk)
    {
        print("location mapping persisted successfully", std::cout);
    }
    return resultOk;
}


bool DBServicePersistenceAccessor::persistDerivateInfo(
        DerivateInfo& derivateInfo)
{
    printFunction(__PRETTY_FUNCTION__, std::cout);

    string relationName("derivate_DBSP");

    vector<string> value =
    {
       derivateInfo.getName(),
       derivateInfo.getSource(),
       derivateInfo.getFun()
    };

    bool resultOk =
            createOrInsert(relationName, derivates, {value});
    if(resultOk)
    {
        print("RelationInfo persisted", std::cout);
        resultOk = persistLocationMapping(
                   derivateInfo.toString(),
                   derivateInfo.nodesBegin(),
                   derivateInfo.nodesEnd());
    }else
    {
        print("Could not persist DerivateInfo. Skipping mapping.", std::cout);
    }
    return resultOk;
}

bool DBServicePersistenceAccessor::restoreLocationInfo(
        map<ConnectionID, LocationInfo>& locations)
{
    printFunction("DBServicePersistenceAccessor::restoreLocationInfo",
                   std::cout);
    bool resultOk = true;
    if(SecondoSystem::GetCatalog()->IsObjectName(string("locations_DBSP")))
    {
        print("The relation locations_DBSP exists. Trying to retrieve \
locations...", std::cout);

         string query("query locations_DBSP");
         string errorMessage;
         ListExpr resultList;
         resultOk = SecondoUtilsLocal::executeQueryCommand(
                 query, resultList, errorMessage);
         if(resultOk)
         {
             print("Successfully executed the locations_DBSP query command", 
                std::cout);
             print("resultList", resultList, std::cout);
             ListExpr resultData = nl->Second(resultList);
             print("resultData", resultData, std::cout);

            int resultCount = nl->ListLength(resultData);
            print("Number of locations", resultCount, std::cout);

            for(int i = 0; i < resultCount; i++)
            {
                if(!nl->IsEmpty(resultData))
                {
                    print("resultData", resultData, std::cout);
                    ListExpr currentRow = nl->First(resultData);
                    ConnectionID conn(nl->IntValue(nl->First(currentRow)));
                    string host(nl->StringValue(nl->Second(currentRow)));
                    string port(nl->StringValue(nl->Third(currentRow)));
                    string config(nl->Text2String(nl->Fourth(currentRow)));
                    string disk(nl->Text2String(nl->Fifth(currentRow)));
                    string commPort(nl->StringValue(nl->Sixth(currentRow)));
                    string transferPort(
                            nl->StringValue(nl->Seventh(currentRow)));

                    LocationInfo location(
                            host, port, config, disk, commPort, transferPort);
                    print(location, std::cout);
                    locations.insert(
                            pair<ConnectionID, LocationInfo>(conn, location));
                    resultData = nl->Rest(resultData);
                } else {
                    print("ResultData was empty.", std::cout);
                }
            }
        } else
        {
            print("Couldn't query locations from locations_DBSP.", std::cout);
            print(errorMessage, std::cout);
        }
    } else
    {
        print("locations_DBSP not found -> nothing to restore", std::cout);
    }
    return resultOk;
}

bool DBServicePersistenceAccessor::restoreRelationInfo(
        map<string, RelationInfo>& relations)
{
    printFunction("DBServicePersistenceAccessor::restoreRelationInfo",
                  std::cout);

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
            print("resultList", resultList, std::cout);
            ListExpr resultData = nl->Second(resultList);
            print("resultData", resultData, std::cout);

            int resultCount = nl->ListLength(resultData);
            print(resultCount, std::cout);

            for(int i = 0; i < resultCount; i++)
            {
                if(!nl->IsEmpty(resultData))
                {
                    print("resultData", resultData, std::cout);
                    ListExpr currentRow = nl->First(resultData);
                    string relID(nl->StringValue(nl->First(currentRow)));
                    string dbName(nl->StringValue(nl->Second(currentRow)));
                    string relName(nl->StringValue(nl->Third(currentRow)));
                    string host(nl->StringValue(nl->Fourth(currentRow)));
                    string port(nl->StringValue(nl->Fifth(currentRow)));
                    string disk(nl->Text2String(nl->Sixth(currentRow)));
                    RelationInfo relationInfo(
                            dbName, relName, host, port, disk);
                    print(relationInfo, std::cout);
                    relations.insert(
                            pair<string, RelationInfo>(relID, relationInfo));
                    resultData = nl->Rest(resultData);
                }
            }
        }else
        {
            print(errorMessage, std::cout);
        }
    }else
    {
        print("relations_DBSP not found -> nothing to restore", std::cout);
    }
    return resultOk;
}


bool DBServicePersistenceAccessor::restoreDerivateInfo(
        DBServiceDerivates& derivates)
{
    printFunction(__PRETTY_FUNCTION__, std::cout);

    bool resultOk = true;
    string relName ("derivates_DBSP");
    if(SecondoSystem::GetCatalog()->IsObjectName(relName))
    {
        string query("query " + relName);
        string errorMessage;
        ListExpr resultList;
        resultOk = SecondoUtilsLocal::executeQueryCommand(
                query, resultList, errorMessage);
        if(resultOk)
        {
            print("resultList", resultList, std::cout);
            ListExpr resultData = nl->Second(resultList);
            print("resultData", resultData, std::cout);

            int resultCount = nl->ListLength(resultData);
            print(resultCount, std::cout);

            for(int i = 0; i < resultCount; i++)
            {
                if(!nl->IsEmpty(resultData))
                {
                    print("resultData", resultData, std::cout);
                    ListExpr currentRow = nl->First(resultData);
                    string objectName(nl->StringValue(nl->First(currentRow)));
                    string source(nl->StringValue(nl->Second(currentRow)));
                    string fun(nl->TextValue(nl->Third(currentRow)));
                    DerivateInfo derivateInfo(objectName, source, fun);
                    print(derivateInfo, std::cout);
                    string did = derivateInfo.toString();
                    derivates.insert(
                            pair<string, DerivateInfo>(did, derivateInfo));
                    resultData = nl->Rest(resultData);
                }
            }
        }else
        {
            print(errorMessage, std::cout);
        }
    }else
    {
        print(relName + " not found -> nothing to restore", std::cout);
    }
    return resultOk;
}



bool DBServicePersistenceAccessor::restoreLocationMapping(
        queue<pair<string, pair<ConnectionID, bool> > >& mapping)
{
    printFunction("DBServicePersistenceAccessor::restoreLocationMapping",
                   std::cout);
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
            print("resultList", resultList, std::cout);
            ListExpr resultData = nl->Second(resultList);
            print("resultData", resultData, std::cout);

            int resultCount = nl->ListLength(resultData);
            print(resultCount, std::cout);

            for(int i = 0; i < resultCount; i++)
            {
                if(!nl->IsEmpty(resultData))
                {
                    print("resultData", resultData, std::cout);
                    ListExpr currentRow = nl->First(resultData);
                    string objectID(nl->StringValue(nl->First(currentRow)));
                    int conn = nl->IntValue(nl->Second(currentRow));
                    bool replicated = nl->BoolValue(nl->Third(currentRow));
                    print("ObjectID: ", objectID, std::cout);
                    print("ConnectionID: ", conn, std::cout);
                    print("Replicated: ", replicated, std::cout);
                    mapping.push(
                            pair<string, pair<ConnectionID, bool> >(
                                    objectID, make_pair(conn, replicated)));
                    resultData = nl->Rest(resultData);
                }
            }
        }else
        {
            print(errorMessage, std::cout);
        }
    }else
    {
        print("mapping_DBSP not found -> nothing to restore", std::cout);
    }
    return resultOk;
}

bool DBServicePersistenceAccessor::updateLocationMapping(
        string objectID,
        ConnectionID connID,
        bool replicated)
{
    printFunction("DBServicePersistenceAccessor::updateLocationMapping",
                  std::cout);

    SecondoUtilsLocal::adjustDatabase(DBSERVICE_DATABASE_NAME);

    FilterConditions filterConditions =
    {
        { {AttributeType2::STRING, string("ObjectID")}, objectID },
        { {AttributeType2::INT, string("ConnectionID")},
                stringutils::int2str(connID) }
    };
    AttributeInfoWithValue valueToUpdate =
    {{AttributeType2::BOOL, string("Replicated")},
            (replicated ? string("TRUE") : string("FALSE")) };

    // return SecondoUtilsLocal::executeQuery2(
    //return SecondoUtilsLocal::executeQuery(
    return SecondoUtilsLocal::executeQueryCommand(
        CommandBuilder::buildUpdateCommand(
            string("mapping_DBSP"),
            filterConditions,
            valueToUpdate));
}

// TODO Remove, obsolete.
bool DBServicePersistenceAccessor::deleteRelationInfo(
        RelationInfo& relationInfo)
{
    printFunction("DBServicePersistenceAccessor::deleteRelationInfo",
                   std::cout);
    string relationName("relations_DBSP");

    string relationID = relationInfo.toString();
    FilterConditions filterConditions =
    {
        { { AttributeType2::STRING, string("ObjectID")}, relationID },
    };

    bool resultOk = SecondoUtilsLocal::executeQuery2(
            CommandBuilder::buildDeleteCommand(
                    relationName,
                    filterConditions));

    if(!resultOk)
    {
        print("Could not delete relation metadata", std::cout);
        print("ObjectID: ", relationID, std::cout);
    }

    return resultOk && deleteLocationMapping(
            relationID,
            relationInfo.nodesBegin(),
            relationInfo.nodesEnd());
}

//TODO Remove. Obsolete
bool DBServicePersistenceAccessor::deleteDerivateInfo(
        DerivateInfo& derivateInfo,
        RelationInfo& source)
{
    printFunction("DBServicePersistenceAccessor::deleteDerivateInfo",
                  std::cout);

    if(derivateInfo.getSource() != source.toString()){
        print("relationInfo is not the source of the derivate.", std::cout);
        print(derivateInfo, std::cout);
        print(source, std::cout);
        return false;
    }

    string relationName("derivate_DBSP");
    SecondoUtilsLocal::adjustDatabase(source.getDatabaseName());

    string objectName = derivateInfo.getName();
    string sourceName = derivateInfo.getSource();
    FilterConditions filterConditions =
    {
        { { AttributeType2::STRING, string("ObjectName")}, objectName },
        { { AttributeType2::STRING, string("Source")}, sourceName }

    };

    bool resultOk = SecondoUtilsLocal::executeQuery2(
            CommandBuilder::buildDeleteCommand(
                    relationName,
                    filterConditions));

    if(!resultOk)
    {
        print("Could not delete  metadata", std::cout);
        print("ObjectName: ", objectName, std::cout);
    }
    string objectId = derivateInfo.toString();

    return resultOk && deleteLocationMapping(
            objectId , 
            derivateInfo.nodesBegin(),
            derivateInfo.nodesEnd());
}


//TODO Remove. Obsolete
bool DBServicePersistenceAccessor::deleteLocationMapping(
        string objectID,
        ReplicaLocations::const_iterator nodesBegin,
        ReplicaLocations::const_iterator nodesEnd)
{
    printFunction("DBServicePersistenceAccessor::deleteLocationMapping",
                  std::cout);
    string relationName("mapping_DBSP");
    bool resultOk = true;
    for(ReplicaLocations::const_iterator it = nodesBegin;
            it != nodesEnd; it++)
    {
        FilterConditions filterConditions =
        {
            { {AttributeType2::STRING, string("ObjectID") }, objectID },
            { {AttributeType2::INT, string("ConnectionID") },
                    stringutils::int2str(it->first) }
        };

        resultOk = resultOk && SecondoUtilsLocal::executeQuery2(
                CommandBuilder::buildDeleteCommand(
                        relationName,
                        filterConditions));
        if(!resultOk)
        {
            print("Could not delete mapping", std::cout);
            print("ObjectID: ", objectID, std::cout);
            print("ConnectionID: ", it->first, std::cout);
        }
    }
    return resultOk;
}

//TODO Remove. Obsolete
bool DBServicePersistenceAccessor::persistAllLocations(
        DBServiceLocations dbsLocations)
{
    printFunction("DBServicePersistenceAccessor::persistAllLocations",
                  std::cout);
    vector<vector<string> > values;
    if(!dbsLocations.empty())
    {
        for(const auto& location : dbsLocations)
        {
            const LocationInfo& locationInfo = location.second.first;
            vector<string> value = {
                    stringutils::int2str(location.first),
                    locationInfo.getHost(),
                    locationInfo.getPort(),
                    locationInfo.getConfig(),
                    locationInfo.getDisk(),
                    locationInfo.getCommPort(),
                    locationInfo.getTransferPort(),
            };
            values.push_back(value);
        }
    }

    //TODO Delete and create is basically updateOrCreate. So why not update 
    //  existing records and create if no record exists?.
    return deleteAndCreate(
            string("locations_DBSP"),
            locations,
            values);
    return true;
}


//TODO Refactore. Remove the deleteAndCreateLogic.
bool DBServicePersistenceAccessor::persistAllReplicas(
        DBServiceRelations dbsRelations,
        DBServiceDerivates dbsDerivates)
{

    /*
     */

    printFunction("DBServicePersistenceAccessor::persistAllReplicas",
                  std::cout);
    vector<vector<string> > relationValues;
    vector<vector<string> > mappingValues;

    // If there are Relations, for each relation build a relationInfo object.
    // Build a vector of relations
    // For each replica locations create a mapping (relation <> location)
    // Build a vectore of mappings
    if(!dbsRelations.empty())
    {
        for(const auto& relation : dbsRelations)
        {
            const RelationInfo& relationInfo = relation.second;
            vector<string> value = {
                relation.first,
                relationInfo.getDatabaseName(),
                relationInfo.getRelationName(),
                relationInfo.getOriginalLocation().getHost(),
                relationInfo.getOriginalLocation().getPort(),
                relationInfo.getOriginalLocation().getDisk()
            };
            relationValues.push_back(value);
            for(ReplicaLocations::const_iterator it =
                    relationInfo.nodesBegin();
                    it != relationInfo.nodesEnd(); it++)
            {
                vector<string> mapping = {
                        relation.first,
                        stringutils::int2str(it->first),
                        it->second ? string("TRUE") : string("FALSE")
                };
                mappingValues.push_back(mapping);
            }
        }
    }

    // Build a vectore of derivates
    vector<vector<string> > derivateValues;

    // Build a vector of derivate mappings
    if(!dbsDerivates.empty())
    {
        for(const auto& derivate : dbsDerivates)
        {
            const DerivateInfo& derivateInfo = derivate.second;
            vector<string> value = {
                derivateInfo.getName(),
                derivateInfo.getSource(),
                derivateInfo.getFun()
            };
            derivateValues.push_back(value);
            string id = derivateInfo.toString();
            for(ReplicaLocations::const_iterator it =
                    derivateInfo.nodesBegin();
                    it != derivateInfo.nodesEnd(); it++)
            {
                vector<string> mapping = {
                        id,
                        stringutils::int2str(it->first),
                        it->second ? string("TRUE") : string("FALSE")
                };
                mappingValues.push_back(mapping);
            }
        }
    }

    /* Delete the DBService relations (tables) and recreate them.
    *  While recreating them insert the relation, derivate and
    *  mapping records.
    */

    return     deleteAndCreate(
                 string("relations_DBSP"),
                 relations,
                 relationValues)
           && deleteAndCreate(
                 string("derivates_DBSP"),
                 derivates,
                 derivateValues)
           && deleteAndCreate(
                    string("mapping_DBSP"),
                    mapping,
                    mappingValues);
}

size_t getRecordCount(const string& databaseName, const string& relationName)
{
    printFunction("DBServicePersistenceAccessor::getRecordCount", std::cout);
    print(databaseName, std::cout);
    print(relationName, std::cout);

   // seems to be wrong, waiting for answer
   // SecondoUtilsLocal::adjustDatabase(databaseName);
    return 0;
}

RelationDefinition DBServicePersistenceAccessor::locations =
{
     { AttributeType2::INT, "ConnectionID" },
     { AttributeType2::STRING, "Host" },
     { AttributeType2::STRING, "Port" },
     { AttributeType2::TEXT, "Config" },
     { AttributeType2::TEXT, "Disk" },
     { AttributeType2::STRING, "CommPort" },
     { AttributeType2::STRING, "TransferPort" }
};

RelationDefinition DBServicePersistenceAccessor::nodes =
{
        {AttributeType2::TEXT, "Host"},
        {AttributeType2::INT, "Port"},
        {AttributeType2::TEXT, "Config"},
        {AttributeType2::TEXT, "DiskPath"},
        {AttributeType2::INT, "ComPort"},
        {AttributeType2::INT, "TransferPort"}
};

RelationDefinition DBServicePersistenceAccessor::relations =
{
    { AttributeType2::STRING, "RelationID" },
    { AttributeType2::STRING, "DatabaseName" },
    { AttributeType2::STRING, "RelationName" },
    { AttributeType2::STRING, "Host" },
    { AttributeType2::STRING, "Port" },
    { AttributeType2::TEXT, "Disk" }
};

RelationDefinition DBServicePersistenceAccessor::mapping =
{
    { AttributeType2::STRING, "ObjectID" },
    { AttributeType2::INT, "ConnectionID" },
    { AttributeType2::BOOL, "Replicated" }
};


RelationDefinition DBServicePersistenceAccessor::derivates =
{
    { AttributeType2::STRING, "ObjectName" },
    { AttributeType2::STRING, "DependsOn" },
    { AttributeType2::TEXT, "FunDef" }
};

const string DBServicePersistenceAccessor::DBSERVICE_DATABASE_NAME = 
    "dbservice";

} /* namespace DBService */

