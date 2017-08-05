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

#include "CharTransform.h"
#include "NestedList.h"
#include "Profiles.h"
#include "SecParser.h"

#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/SecondoUtilsRemote.hpp"


using namespace std;
using namespace distributed2;

namespace DBService {

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
