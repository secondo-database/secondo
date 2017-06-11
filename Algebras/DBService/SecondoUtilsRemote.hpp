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
#ifndef ALGEBRAS_DBSERVICE_SecondoUtilsRemote_HPP_
#define ALGEBRAS_DBSERVICE_SecondoUtilsRemote_HPP_

#include <string>

#include "NestedList.h"

#include "Algebras/Distributed2/ConnectionInfo.h"

namespace DBService {

/*

1 \textit{}

\textit{DBService}
TODO

*/

class SecondoUtilsRemote {
public:
    static bool executeQuery(
            distributed2::ConnectionInfo* connectionInfo,
            const std::string& query);

    static bool executeQuery(
            distributed2::ConnectionInfo* connectionInfo,
            const std::string& query,
            std::string& result);

    static bool openDatabase(
            distributed2::ConnectionInfo* connectionInfo,
            const char* dbName);

    static bool createDatabase(
                distributed2::ConnectionInfo* connectionInfo,
                const char* dbName);

    static bool closeDatabase(
                distributed2::ConnectionInfo* connectionInfo);
private:
    static bool handleDatabase(
            distributed2::ConnectionInfo* connectionInfo,
            const std::string& action,
            const std::string& dbName);
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_SecondoUtilsRemote_HPP_ */
