/*

1.1 ~SecondoUtilsLocal~

This class allows interacting with the local SECONDO instance.

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

1.1.1 Class Definition

*/

class SecondoUtilsLocal {
public:

/*

1.1.1.1 ~readFromConfigFile~

This function retrieves a value from the local configuration file.

*/
    static void readFromConfigFile(std::string& resultValue,
            const char* section,
            const char* key,
            const char* defaultValue);

/*

1.1.1.1 ~executeQuery~

This function allows executing a query.

*/
    static bool executeQuery(const std::string& queryAsString);

/*

1.1.1.1 ~executeQuery2~

This function allows executing a query. The query is prepared before execution.

*/
    static bool executeQuery2(const std::string& queryAsString);

/*

1.1.1.1 ~adjustDatabase~

This function allows adjusting the currently opened database to the specified
value.

*/
    static bool adjustDatabase(const std::string& databaseName);

/*

1.1.1.1 ~createRelation~

This function allows creating a relation.

*/
    static bool createRelation(
            const std::string& queryAsString,
            std::string& errorMessage);

/*

1.1.1.1 ~executeQueryCommand~

This function allows executing a query.

*/
    static bool executeQueryCommand(
                const std::string& queryAsString);

/*

1.1.1.1 ~executeQueryCommand~

This function allows executing a query and accessing the result

*/
    static bool executeQueryCommand(
            const std::string& queryAsString,
            ListExpr& resultList,
            std::string& errorMessage);

/*

1.1.1.1 ~lookupDBServiceLocation~

This function retrieves the host name and the port number of the ~DBService~,
to be precise of the ~CommunicationServer~ that is running on the ~DBService~
master node.

*/
    static bool lookupDBServiceLocation(
            std::string& host,
            std::string& commPort);

/*

1.1.1.1 ~prepareQueryForProcessing~

This function prepares a query before it is executed by removing unnecessary
parts.

*/
private:
    static bool prepareQueryForProcessing(
            const std::string& queryAsString,
            std::string& queryAsPreparedNestedListString);

/*

1.1.1.1 ~executeQuery~

This function allows executing a query.

*/
    static bool executeQuery(const std::string& queryAsString,
                             Word& queryResult);

/*

1.1.1.1 ~utilsMutex~

This mutex ensures that only one nested list is processed at a time.

*/
    static boost::mutex utilsMutex;
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_SecondoUtilsLocal_HPP_ */
