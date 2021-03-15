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

