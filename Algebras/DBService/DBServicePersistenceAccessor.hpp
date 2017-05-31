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
#ifndef ALGEBRAS_DBSERVICE_DBSERVICEPERSISTENCEACCESSOR_HPP_
#define ALGEBRAS_DBSERVICE_DBSERVICEPERSISTENCEACCESSOR_HPP_

#include <map>
#include <queue>
#include <vector>

#include "NestedList.h"

#include "Algebras/DBService/CommandBuilder.hpp"
#include "Algebras/DBService/LocationInfo.hpp"
#include "Algebras/DBService/RelationInfo.hpp"

namespace DBService {

class DBServicePersistenceAccessor {
public:
    static bool persistLocationInfo(
            ConnectionID connID, LocationInfo& locationInfo);
    static bool persistRelationInfo(
            RelationInfo& relationInfo);

    static bool restoreLocationInfo(
            std::map<ConnectionID, LocationInfo>& locations);

    static bool restoreRelationInfo(
            std::map<std::string, RelationInfo>& relations);
    static bool restoreLocationMapping(
            std::queue<std::pair<std::string, ConnectionID> >& mapping);
private:
    static bool persistLocationMapping(
            std::string relationID,
            std::vector<ConnectionID>::const_iterator nodesBegin,
            std::vector<ConnectionID>::const_iterator nodesEnd);
    static bool createOrInsert(
            const std::string& relationName,
            const RelationDefinition& rel,
            const std::vector<std::string>& values);
    static RelationDefinition locations;
    static RelationDefinition relations;
    static RelationDefinition mapping;
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_DBSERVICEPERSISTENCEACCESSOR_HPP_ */
