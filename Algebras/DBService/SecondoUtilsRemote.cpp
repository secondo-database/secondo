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

#include "CharTransform.h"
#include "NestedList.h"
#include "Profiles.h"
#include "SecParser.h"

#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/SecondoUtilsRemote.hpp"


using namespace std;
using namespace distributed2;

namespace DBService {

bool SecondoUtilsRemote::openDatabase(
        distributed2::ConnectionInfo* connectionInfo,
        const char* dbName)
{
    printFunction("SecondoUtilsRemote::openDatabase");
    return SecondoUtilsRemote::handleDatabase(connectionInfo,
                                                "open",
                                                dbName);
}

bool SecondoUtilsRemote::createDatabase(
        distributed2::ConnectionInfo* connectionInfo,
        const char* dbName)
{
    printFunction("SecondoUtilsRemote::createDatabase");
    return SecondoUtilsRemote::handleDatabase(connectionInfo,
                                                "create",
                                                dbName);
}

bool SecondoUtilsRemote::closeDatabase(
        distributed2::ConnectionInfo* connectionInfo)
{
    printFunction("SecondoUtilsRemote::closeDatabase");
    return SecondoUtilsRemote::handleDatabase(connectionInfo,
                                                "close",
                                                "");
}

bool SecondoUtilsRemote::handleDatabase(ConnectionInfo* connectionInfo,
                                          const string& action,
                                          const string& dbName)
{
    printFunction("SecondoUtilsRemote::handleDatabase");
    stringstream query;
    query << action << " database " << dbName;
    print(query.str());
    bool resultOk =
            SecondoUtilsRemote::executeQuery(connectionInfo,
                    query.str());
    if(!resultOk)
    {
        //throw new SecondoException("could not open database 'dbservice'");
        print("Boo");
    }
    return resultOk;
}

bool SecondoUtilsRemote::executeQuery(
        distributed2::ConnectionInfo* connectionInfo,
        const std::string& query)
{
    printFunction("SecondoUtilsRemote::executeQuery");
    string result;
    return executeQuery(connectionInfo,
                                      query,
                                      result);
}

bool SecondoUtilsRemote::executeQuery(
        distributed2::ConnectionInfo* connectionInfo,
        const std::string& query,
        std::string& result)
{
    printFunction("SecondoUtilsRemote::executeQuery");
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

} /* namespace DBService */
