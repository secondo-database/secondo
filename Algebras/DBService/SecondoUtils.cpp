/*
----
This file is part of SECONDO.

Copyright (C) 2016,
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

#include "Profiles.h"
#include "CharTransform.h"
#include "SecParser.h"

#include "Algebras/DBService/SecondoUtils.hpp"
#include "Algebras/DBService/DebugOutput.hpp"


using namespace std;
using namespace distributed2;

namespace DBService {

void SecondoUtils::readFromConfigFile(std::string& resultValue,
        const char* section,
        const char* key,
        const char* defaultValue)
{
    string secondoConfig = expandVar("$(SECONDO_CONFIG)");
    resultValue = SmiProfile::GetParameter(section,
            key, defaultValue, secondoConfig);
}

bool SecondoUtils::openDatabaseOnRemoteServer(
        distributed2::ConnectionInfo* connectionInfo,
        const char* dbName)
{
    return SecondoUtils::handleRemoteDatabase(connectionInfo,
                                                "open",
                                                dbName);
}

bool SecondoUtils::createDatabaseOnRemoteServer(
        distributed2::ConnectionInfo* connectionInfo,
        const char* dbName)
{
    return SecondoUtils::handleRemoteDatabase(connectionInfo,
                                                "create",
                                                dbName);
}

bool SecondoUtils::closeDatabaseOnRemoteServer(
        distributed2::ConnectionInfo* connectionInfo)
{
    return SecondoUtils::handleRemoteDatabase(connectionInfo,
                                                "close",
                                                "");
}

bool SecondoUtils::handleRemoteDatabase(ConnectionInfo* connectionInfo,
                                          const string& action,
                                          const string& dbName)
{
    stringstream query;
    query << action << " database " << dbName;
    print(query.str());
    bool resultOk =
            SecondoUtils::executeQueryOnRemoteServer(connectionInfo,
                    query.str());
    if(!resultOk)
    {
        //throw new SecondoException("could not open database 'dbservice'");
        print("Boo");
    }
    return resultOk;
}

bool SecondoUtils::executeQueryOnRemoteServer(
        distributed2::ConnectionInfo* connectionInfo,
        const std::string& query)
{
    string result;
    return executeQueryOnRemoteServer(connectionInfo,
                                      query,
                                      result);
}

bool SecondoUtils::executeQueryOnRemoteServer(
        distributed2::ConnectionInfo* connectionInfo,
        const std::string& query,
        std::string& result)
{
    int errorCode;
    string errorMessage;
    double runtime;
    distributed2::CommandLog commandLog;
    connectionInfo->simpleCommand(query,
            errorCode, errorMessage, result, false,
            runtime, false, false, commandLog);
    //TODO better error handling
    print(errorCode);
    print(errorMessage.c_str());
    return errorCode == 0;
}

bool SecondoUtils::executeQueryOnCurrentNode(const string& query)
{
    SecParser secondoParser;
    string queryAsNestedList;
    if (secondoParser.Text2List(query, queryAsNestedList) != 0)
    {
        // TODO
        return false;
    } else
    {
        Word result;
        QueryProcessor::ExecuteQuery(queryAsNestedList, result, 1024);
    }
    return true;
}

bool SecondoUtils::adjustDatabaseOnCurrentNode(const std::string& databaseName)
{
    if(SecondoSystem::GetInstance()->GetDatabaseName()
            != databaseName)
    {
        string queryClose("close database");
        SecondoUtils::executeQueryOnCurrentNode(queryClose);
        stringstream queryCreate;

        queryCreate << "create database "
                    << databaseName;
        SecondoUtils::executeQueryOnCurrentNode(queryCreate.str());

        stringstream queryOpen;
        queryOpen << "open database "
                    << databaseName;
        SecondoUtils::executeQueryOnCurrentNode(queryOpen.str());
        return true;
    }
    return false;
}

} /* namespace DBService */
