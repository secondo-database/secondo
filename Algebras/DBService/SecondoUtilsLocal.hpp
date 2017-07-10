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
#ifndef ALGEBRAS_DBSERVICE_SecondoUtilsLocal_HPP_
#define ALGEBRAS_DBSERVICE_SecondoUtilsLocal_HPP_

#include <string>

#include "NestedList.h"

#include "Algebras/Distributed2/ConnectionInfo.h"

namespace DBService {

/*

1 \textit{}

\textit{DBService}
TODO

*/

class SecondoUtilsLocal {
public:
    static void readFromConfigFile(std::string& resultValue,
            const char* section,
            const char* key,
            const char* defaultValue);

    static bool executeQuery(const std::string& queryAsString);
    static bool executeQuery(const std::string& queryAsString,
                             Word& queryResult);
    static bool executeQuery2(const std::string& queryAsString);

    static bool adjustDatabase(const std::string& databaseName);
    static bool createRelation(
            const std::string& queryAsString,
            std::string& errorMessage);
    static bool excuteQueryCommand(
                const std::string& queryAsString);
    static bool excuteQueryCommand(
            const std::string& queryAsString,
            ListExpr& resultList,
            std::string& errorMessage);
    static bool lookupDBServiceLocation(
            std::string& host,
            std::string& commPort);
private:
    static bool prepareQueryForProcessing(
            const std::string& queryAsString,
            std::string& queryAsPreparedNestedListString);
    static boost::mutex utilsMutex;
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_SecondoUtilsLocal_HPP_ */
